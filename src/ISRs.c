///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Common.h"
#include "Record.h"
#include "Menu.h"
#include "InitDataBuffers.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "SysEvents.h"
#include "Uart.h"
#include "Keypad.h"
#include "RealTimeClock.h"
#include "SoftTimer.h"
#include "RemoteHandler.h"
#include "PowerManagement.h"
#include "Analog.h"
#include "gpio.h"
#include "intc.h"
#include "SysEvents.h"
#include "M23018.h"
#include "rtc.h"
#include "tc.h"
#include "twi.h"
#include "spi.h"
#include "usart.h"
#include "string.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define DUMMY_READ(x) if ((volatile int)x == 0) {}

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern void rtc_clear_interrupt(volatile avr32_rtc_t *rtc);
extern BOOLEAN processCraftCmd;
extern uint8 craft_g_input_buffer[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
// Flags and variables to statically defined to prevent creating and clearing stack variables every ISR data process call
static uint16 s_R_channelReading, s_V_channelReading, s_T_channelReading, s_A_channelReading;
static uint16 s_channelConfigReadBack;
static uint16* s_pretrigPtr;
static uint16* s_samplePtr;
static uint16* s_calPtr[3] = {NULL, NULL, NULL};
static uint32 s_pretrigCount = 0;
static uint32 s_sampleCount = 0;
static uint32 s_calSampleCount = 0;
static uint32 s_pendingCalCount = 0;
static uint32 s_pretriggerCount = 0;
static uint16 s_consecSeismicTriggerCount = 0;
static uint16 s_consecAirTriggerCount = 0;
static uint16 s_sensorCalSampleCount = CALIBRATION_FIXED_SAMPLE_RATE;

static uint8 s_pretriggerFull = NO;
static uint8 s_checkForTempDrift = NO;
static uint8 s_channelConfig = CHANNELS_R_AND_V_SWAPPED;
static uint8 s_seismicTriggerSample = NO;
static uint8 s_airTriggerSample = NO;
static uint8 s_recordingEvent = NO;
static uint8 s_calPulse = NO;
static uint8 s_consecEventsWithoutCal = 0;
static uint8 s_channelSyncError = NO;
static uint8 s_channelReadsToSync = 0;
static int16 s_temperatureDelta = 0;
static uint8 s_consecutiveEventsWithoutCalThreshold = CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Tc_sample_irq(void);
__attribute__((__interrupt__))
void Tc_typematic_irq(void);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Eic_external_rtc_irq(void)
{
	static uint32 updateCounter = 0;

	// Print test for verification of operation
	//debugRaw("`");

	updateCounter++;
	
	if (updateCounter % 1024 == 0)
	{
		debugRaw("#");
	}

	// clear the interrupt flag in the processor
	AVR32_EIC.ICR.int1 = 1;

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Eic_low_battery_irq(void)
{
	debugRaw("-LowBatt-");

	raiseSystemEventFlag(LOW_BATTERY_WARNING_EVENT);

	// Clear the interrupt flag in the processor
	AVR32_EIC.ICR.int0 = 1;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Eic_keypad_irq(void)
{
	// Print test for verification of operation
	//debugRaw("^");

	if (g_kpadProcessingFlag == DEACTIVATED)
	{
		raiseSystemEventFlag(KEYPAD_EVENT);
	}
	else
	{
		g_kpadInterruptWhileProcessing = YES;
	}

#if 0 // Not necessary to clear the keypad interrupt just the processor interrupt so the ISR isn't called again
	ReadMcp23018(IO_ADDRESS_KPD, INTFB);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
#endif

	// Clear the interrupt flag in the processor
	AVR32_EIC.ICR.int5 = 1;

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Eic_system_irq(void)
{
	static uint8 onKeyCount = 0;
	static uint8 powerOffAttempted = NO;
	uint8 keyFlag;
	uint8 keyScan;

	// Print test for verification of operation
	//debugRaw("&");

#if 1
	keyFlag = ReadMcp23018(IO_ADDRESS_KPD, INTFA);
	keyScan = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);

	//-----------------------------------------------------------------------------------
	// Check if the On key was pressed
	if ((keyFlag & keyScan) == ON_KEY)
	{
		if (g_factorySetupSequence != PROCESS_FACTORY_SETUP)
		{
			//debug("Factory Setup: Stage 1\r\n");
			g_factorySetupSequence = STAGE_1;
		}

		if (powerOffAttempted == YES)
		{ 
			onKeyCount++;
		}
	}

	//-----------------------------------------------------------------------------------
	// Check if the Off key was pressed
	if (keyScan & OFF_KEY)
	{
		g_powerOffActivated = YES;

		if ((powerOffAttempted == YES) && (onKeyCount == 3))
		{
			// Jumping off the ledge.. Buh bye! No returning from this
			//debugRaw("\n--> BOOM <--");
			PowerControl(POWER_OFF_PROTECTION_ENABLE, OFF);
			PowerControl(POWER_OFF, ON);
		}

		powerOffAttempted = YES;
		onKeyCount = 0;
	}

	//-----------------------------------------------------------------------------------
	// Check of the external trigger signal has been found
	if (keyFlag & 0x08)
	{
		// Clear trigger out signal in case it was self generated (Active high control)
		PowerControl(TRIGGER_OUT, OFF);
		
		//debugRaw("-ET-");
		
		// Check if monitoring and not bargraph and not processing an event
		if (((g_sampleProcessing == ACTIVE_STATE)) && (g_triggerRecord.opMode != BARGRAPH_MODE) && (g_busyProcessingEvent == NO))
		{
			// Signal the start of an event
			g_externalTrigger = YES;
		}
	}
#else
	ReadMcp23018(IO_ADDRESS_KPD, INTFA);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
#endif

	// Clear the interrupt flag in the processor
	AVR32_EIC.ICR.int4 = 1;

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Usart_1_rs232_irq(void)
{
	// Test print to verify the interrupt is running
	//debugRaw("`");

	uint32 usart_1_status;
	uint8 recieveData;

	// Read control/status register to clear flags
	usart_1_status = AVR32_USART1.csr;

	// Read the data received (in combo with control/status read clears the interrupt)
	recieveData = AVR32_USART1.rhr;

	// Check if receive operation was successful
	if (usart_1_status & AVR32_USART_CSR_RXRDY_MASK)
	{
		// Write the received data into the buffer
		*g_isrMessageBufferPtr->writePtr = recieveData;

		// Advance the buffer pointer
		g_isrMessageBufferPtr->writePtr++;

		// Check if buffer pointer goes beyond the end
		if (g_isrMessageBufferPtr->writePtr >= (g_isrMessageBufferPtr->msg + CMD_BUFFER_SIZE))
		{
			// Reset the buffer pointer to the beginning of the buffer
			g_isrMessageBufferPtr->writePtr = g_isrMessageBufferPtr->msg;
		}

		// Raise the Craft Data flag
		g_modemStatus.craftPortRcvFlag = YES;
	}
	else if (usart_1_status & (AVR32_USART_CSR_OVRE_MASK | AVR32_USART_CSR_FRAME_MASK | AVR32_USART_CSR_PARE_MASK))
	{
		//g_isrMessageBufferPtr->status = CMD_MSG_OVERFLOW_ERR;

		AVR32_USART1.cr = AVR32_USART_CR_RSTSTA_MASK;
		usart_1_status = AVR32_USART1.csr;
	}

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 1 // ET test
static uint32 s_testForForeverLoop = 0;
static uint32 s_lastExecCycles = 0;
#endif

__attribute__((__interrupt__))
void Soft_timer_tick_irq(void)
{
	g_testAfterSleepISR = Get_system_register(AVR32_COUNT);

	// Test print to verify the interrupt is running
	//debugRaw("`");

	// Increment the lifetime soft timer tick count
	g_rtcSoftTimerTickCount++;

	// Every tick raise the flag to check soft timers
	raiseTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

	// Every 8 ticks (4 secs) trigger the cyclic event flag
	if (++g_cyclicEventDelay == CYCLIC_EVENT_TIME_THRESHOLD)
	{
		g_cyclicEventDelay = 0;
		raiseSystemEventFlag(CYCLIC_EVENT);

#if 1 // Test
		g_sampleCountHold = g_sampleCount;
		g_sampleCount = 0;
#endif
	}

	// Every so often flag for updating to the External RTC time.
	if (++g_rtcCurrentTickCount >= UPDATE_TIME_EVENT_THRESHOLD)
	{
		raiseSystemEventFlag(UPDATE_TIME_EVENT);
	}

#if 1 // ET test
	// Check if the exec cycles if the same meaning that the main loop isn't running
	if (g_execCycles == s_lastExecCycles)
	{
		s_testForForeverLoop++;

		if (s_testForForeverLoop > (10 * TICKS_PER_MIN))
		{
			// Signal error condition
			//OverlayMessage(getLangText(ERROR_TEXT), "UNIT STUCK IN NON ISR LOOP", (3 * SOFT_SECS));
			if (g_debugBufferCount == 1) { g_breakpointCause = BP_MB_LOOP; }
			else { g_breakpointCause = BP_SOFT_LOOP; }

			__asm__ __volatile__ ("breakpoint");
		}
	}
	else // Capture current exec cycles and clear loop count
	{
		s_lastExecCycles = g_execCycles;
		s_testForForeverLoop = 0;
	}
#endif

	// clear the interrupt flag
	rtc_clear_interrupt(&AVR32_RTC);

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Start_Data_Clock(TC_CHANNEL_NUM channel)
{
	//volatile avr32_tc_t *tc = &AVR32_TC;

	// Start the timer/counter.
	tc_start(&AVR32_TC, channel); // And start the timer/counter.

	switch (channel)
	{
		case TC_SAMPLE_TIMER_CHANNEL:
			g_tcSampleTimerActive = YES;
			break;
			
		case TC_CALIBRATION_TIMER_CHANNEL:
			break;
			
		case TC_TYPEMATIC_TIMER_CHANNEL:
			g_tcTypematicTimerActive = YES;
			break;
	}
}	

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Stop_Data_Clock(TC_CHANNEL_NUM channel)
{
	//volatile avr32_tc_t *tc = &AVR32_TC;

	// Stop the timer/counter.
	tc_stop(&AVR32_TC, channel); // And start the timer/counter.

	switch (channel)
	{
		case TC_SAMPLE_TIMER_CHANNEL:
			g_tcSampleTimerActive = NO;
			break;
			
		case TC_CALIBRATION_TIMER_CHANNEL:
			break;
			
		case TC_TYPEMATIC_TIMER_CHANNEL:
			g_tcTypematicTimerActive = NO;
			break;
	}
}	

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Tc_typematic_irq(void)
{
	// Increment the ms seconds counter
	g_keypadTimerTicks++;

	// clear the interrupt flag
	DUMMY_READ(AVR32_TC.channel[TC_TYPEMATIC_TIMER_CHANNEL].sr);

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void fillPretriggerBufferUntilFull_ISR_Inline(void)
{
	s_pretriggerCount++;

	// Check if the Pretrigger count has accumulated the full number of samples (variable Pretrigger size)
	if (s_pretriggerCount >= (g_triggerRecord.trec.sample_rate / g_unitConfig.pretrigBufferDivider))
	{ 
		s_pretriggerFull = YES;
		s_pretriggerCount = 0;
		
		// Save the current temperature
		g_storedTempReading = g_previousTempReading = g_currentTempReading;

		// Check if setup to monitor temp drift and not 16K sample rate
		if ((g_triggerRecord.trec.adjustForTempDrift == YES) && (g_triggerRecord.trec.sample_rate != SAMPLE_RATE_16K))
		{
			s_checkForTempDrift = YES;
		}
	}					
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void normalizeSampleData_ISR_Inline(void)
{
	if (s_R_channelReading < ACCURACY_16_BIT_MIDPOINT) { s_R_channelReading = ACCURACY_16_BIT_MIDPOINT - s_R_channelReading; }
	else { s_R_channelReading -= ACCURACY_16_BIT_MIDPOINT; }

	if (s_V_channelReading < ACCURACY_16_BIT_MIDPOINT) { s_V_channelReading = ACCURACY_16_BIT_MIDPOINT - s_V_channelReading; }
	else { s_V_channelReading -= ACCURACY_16_BIT_MIDPOINT; }

	if (s_T_channelReading < ACCURACY_16_BIT_MIDPOINT) { s_T_channelReading = ACCURACY_16_BIT_MIDPOINT - s_T_channelReading; }
	else { s_T_channelReading -= ACCURACY_16_BIT_MIDPOINT; }

	if (s_A_channelReading < ACCURACY_16_BIT_MIDPOINT) { s_A_channelReading = ACCURACY_16_BIT_MIDPOINT - s_A_channelReading; }
	else { s_A_channelReading -= ACCURACY_16_BIT_MIDPOINT; }
}
	
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void checkAlarms_ISR_Inline(void)
{
	// Check if Alarm 1 mode is active
	if (g_unitConfig.alarmOneMode != ALARM_MODE_OFF)
	{
		// Check if seismic is enabled for Alarm 1 (bitwise operation)
		if (g_unitConfig.alarmOneMode & ALARM_MODE_SEISMIC)
		{
			if ((s_R_channelReading > g_unitConfig.alarmOneSeismicLevel) || (s_V_channelReading > g_unitConfig.alarmOneSeismicLevel) ||
				(s_T_channelReading > g_unitConfig.alarmOneSeismicLevel))
			{
				raiseSystemEventFlag(WARNING1_EVENT);
			}
		}

		// Check if air is enabled for Alarm 1 (bitwise operation)
		if (g_unitConfig.alarmOneMode & ALARM_MODE_AIR)
		{
			if (s_A_channelReading > g_unitConfig.alarmOneAirLevel)
			{
				raiseSystemEventFlag(WARNING1_EVENT);
			}
		}
	}
						
	if (g_unitConfig.alarmTwoMode != ALARM_MODE_OFF)
	{
		// Check if seismic is enabled for Alarm 2 (bitwise operation)
		if (g_unitConfig.alarmTwoMode & ALARM_MODE_SEISMIC)
		{
			if ((s_R_channelReading > g_unitConfig.alarmTwoSeismicLevel) || (s_V_channelReading > g_unitConfig.alarmTwoSeismicLevel) ||
				(s_T_channelReading > g_unitConfig.alarmTwoSeismicLevel))
			{
				raiseSystemEventFlag(WARNING2_EVENT);
			}
		}

		// Check if air is enabled for Alarm 2 (bitwise operation)
		if (g_unitConfig.alarmTwoMode & ALARM_MODE_AIR)
		{
			if (s_A_channelReading > g_unitConfig.alarmTwoAirLevel)
			{
				raiseSystemEventFlag(WARNING2_EVENT);
			}
		}
	}				
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void processAndMoveManualCalData_ISR_Inline(void)
{
	// Wait 5 samples to start cal signaling (~5 ms)
	if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_FIRST_TRANSITION_HIGH) { AdSetCalSignalHigh(); }	// (~10 ms)
	if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_SECOND_TRANSITION_LOW) { AdSetCalSignalLow(); }	// (~20 ms)
	if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_THIRD_TRANSITION_HIGH) { AdSetCalSignalHigh(); }	// (~10 ms)
	if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_FOURTH_TRANSITION_OFF) { AdSetCalSignalOff(); }	// (~55 ms)

	// Check if the first time through
	if (g_manualCalSampleCount == MAX_CAL_SAMPLES)
	{
		// Setup the sample pointer to the start of the event buffer
		s_samplePtr = g_startOfEventBufferPtr;
	}

	// Move the samples into the event buffer
	*(SAMPLE_DATA_STRUCT*)s_samplePtr = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;

	s_samplePtr += NUMBER_OF_CHANNELS_DEFAULT;

	g_manualCalSampleCount--;

	// Check if done with the Manual Cal pulse
	if (g_manualCalSampleCount == 0)
	{
		// Signal the end of the Cal pulse
		raiseSystemEventFlag(MANUAL_CAL_EVENT);

		// Signal an event buffer has been used (gimmick to make processing work)
		g_freeEventBuffers--;

		g_manualCalFlag = FALSE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void processAndMoveWaveformData_ISR_Inline(void)
{
	//_____________________________________________________________________________________
	//___Three main stages to processing:
	//___1) Look for a trigger, either for the first time or within the window to process a consecutive event which postpones a cal pulse
	//___2) Capture event data since a trigger has been found
	//___3) Process the cal pulse to accompany the captured event data

	//_____________________________________________________________________________________
	//___Check if not recording _and_ no cal pulse, or if pending cal, and only if there are free event buffers
	if (((((s_recordingEvent == NO) && (s_calPulse == NO)) || (s_pendingCalCount))) && (g_freeEventBuffers))
	{
		//_____________________________________________________________________________________
		//___Check for triggers
		if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
		{
			if (s_R_channelReading > g_triggerRecord.trec.seismicTriggerLevel) { s_seismicTriggerSample = YES; }
			else if (s_V_channelReading > g_triggerRecord.trec.seismicTriggerLevel) { s_seismicTriggerSample = YES; }
			else if (s_T_channelReading > g_triggerRecord.trec.seismicTriggerLevel) { s_seismicTriggerSample = YES; }
					
			if (s_seismicTriggerSample == YES) { s_consecSeismicTriggerCount++; s_seismicTriggerSample = NO; }
			else {s_consecSeismicTriggerCount = 0; }
		}

		if (g_triggerRecord.trec.airTriggerLevel != NO_TRIGGER_CHAR)
		{
			if (s_A_channelReading > g_triggerRecord.trec.airTriggerLevel) { s_airTriggerSample = YES; }
						
			if (s_airTriggerSample == YES) { s_consecAirTriggerCount++; s_airTriggerSample = NO; }
			else { s_consecAirTriggerCount = 0; }
		}				

		//___________________________________________________________________________________________
		//___Check if either a seismic or acoustic trigger threshold condition was achieved or an external trigger was found
		if ((s_consecSeismicTriggerCount == CONSECUTIVE_TRIGGERS_THRESHOLD) || 
			(s_consecAirTriggerCount == CONSECUTIVE_TRIGGERS_THRESHOLD) || (g_externalTrigger == YES))
		{
			//debug("--> Trigger Found! %x %x %x %x\r\n", s_R_channelReading, s_V_channelReading, s_T_channelReading, s_A_channelReading);
			//usart_write_char(&AVR32_USART1, '$');
			g_testTimeSinceLastTrigger = g_rtcSoftTimerTickCount;
			
			// Check if this event was triggered by an external trigger signal
			if (g_externalTrigger == YES)
			{
				// Flag as an External Trigger for handling the event
				raiseSystemEventFlag(EXT_TRIGGER_EVENT);

				// Reset the external trigger flag
				g_externalTrigger = NO;
			}
			// Check if the source was not an external trigger
			else // (g_externalTrigger == NO)
			{
				//debugRaw("+ET+");

				// Signal a trigger found for any external unit connected (Active high control)
				PowerControl(TRIGGER_OUT, ON);
					
				// Trigger out will be cleared when our own ISR catches the active high signal
			}

			s_consecSeismicTriggerCount = 0;
			s_consecAirTriggerCount = 0;

#if 0 // Need better management of check
			// Don't worry about temp drift during an event
			s_checkForTempDrift = NO;
#endif

			//___________________________________________________________________________________________
			//___Check if there is an event buffer available and not marked done for taking events (stop trigger)
			if ((g_freeEventBuffers != 0) && (g_doneTakingEvents == NO))
			{
				//___________________________________________________________________________________________
				//__Save date and timestamp of new trigger
				g_startOfEventDateTimestampBufferPtr[g_eventBufferWriteIndex] = GetCurrentTime();

				//___________________________________________________________________________________________
				//__Setup new event buffer pointers, counts, and flags
				s_pretrigPtr = g_startOfEventBufferPtr + (g_eventBufferWriteIndex * g_wordSizeInEvent);
				s_samplePtr = s_pretrigPtr + g_wordSizeInPretrig;

				s_pretrigCount = g_samplesInPretrig;
				s_sampleCount = g_samplesInBody;

				// Check if a cal pulse was already pending
				if (s_calPulse == PENDING)
				{
					// An event has already been captured without a cal, so inc the consecutive count
					s_consecEventsWithoutCal++;
				}

				// Set the cal pointer in the array corresponding to the appropriate event cal section buffer section
				s_calPtr[s_consecEventsWithoutCal] = s_pretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;
									
				s_recordingEvent = YES;
				s_calPulse = PENDING;
				
				// Global flag to signal handling an event
				g_busyProcessingEvent = YES;
				
				// Clear count (also needs to remain zero until event recording is done to prevent reentry into this 'if' section)
				s_pendingCalCount = 0;
									
				//___________________________________________________________________________________________
				//___Copy current Pretrigger buffer sample to event
				*(SAMPLE_DATA_STRUCT*)s_samplePtr = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;

				//___________________________________________________________________________________________
				//___Copy oldest Pretrigger buffer sample to Pretrigger
				if ((g_tailOfPretriggerBuff + NUMBER_OF_CHANNELS_DEFAULT) >= g_endOfPretriggerBuff)
				{
					// Copy first (which is currently the oldest) Pretrigger buffer sample to Pretrigger buffer
					*(SAMPLE_DATA_STRUCT*)s_pretrigPtr = *(SAMPLE_DATA_STRUCT*)g_startOfPretriggerBuff;
				}										
				else // Copy oldest Pretrigger buffer sample to Pretrigger
				{
					*(SAMPLE_DATA_STRUCT*)s_pretrigPtr = *(SAMPLE_DATA_STRUCT*)(g_tailOfPretriggerBuff + NUMBER_OF_CHANNELS_DEFAULT);
				}										

				//___________________________________________________________________________________________
				//___Advance data pointers and decrement counts
				s_samplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				s_sampleCount--;
									
				s_pretrigPtr += NUMBER_OF_CHANNELS_DEFAULT;
				s_pretrigCount--;
			}
		}
		//___________________________________________________________________________________________
		//___Check if pending for a cal pulse if no consecutive trigger was found
		else if (s_pendingCalCount)
		{
			s_pendingCalCount--;
						
			// Check if done pending for a Cal pulse
			if (s_pendingCalCount == 0)
			{
				// Time to handle the Cal pulse
				s_calPulse = YES;
				s_calSampleCount = START_CAL_SIGNAL;
			}
		}
		//___________________________________________________________________________________________
		//___Check if waiting to finish monitoring
		else if (g_doneTakingEvents == PENDING)
		{
			g_doneTakingEvents = YES;
		}
	}
	//___________________________________________________________________________________________
	//___Check if handling event samples
	else if ((s_recordingEvent == YES) && (s_sampleCount))
	{
		//___________________________________________________________________________________________
		//___Check if pretrig data to copy
		if (s_pretrigCount)
		{
			// Check if the end of the Pretrigger buffer has been reached
			if ((g_tailOfPretriggerBuff + NUMBER_OF_CHANNELS_DEFAULT) >= g_endOfPretriggerBuff)
			{
				// Copy oldest (which is currently the first) Pretrigger buffer samples to event Pretrigger
				*(SAMPLE_DATA_STRUCT*)s_pretrigPtr = *(SAMPLE_DATA_STRUCT*)g_startOfPretriggerBuff;
			}										
			else // Copy oldest Pretrigger buffer samples to event Pretrigger
			{
				*(SAMPLE_DATA_STRUCT*)s_pretrigPtr = *(SAMPLE_DATA_STRUCT*)(g_tailOfPretriggerBuff + NUMBER_OF_CHANNELS_DEFAULT);
			}										

			s_pretrigPtr += NUMBER_OF_CHANNELS_DEFAULT;
			s_pretrigCount--;
		}

		//___________________________________________________________________________________________
		//___Copy data samples to event buffer
		*(SAMPLE_DATA_STRUCT*)s_samplePtr = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;

		s_samplePtr += NUMBER_OF_CHANNELS_DEFAULT;
		s_sampleCount--;
					
		//___________________________________________________________________________________________
		//___Check if all the event samples have been handled
		if (s_sampleCount == 0)
		{
			//debug("--> Recording done\r\n");
			//usart_write_char(&AVR32_USART1, '%');

			s_recordingEvent = NO;

			// Mark one less event buffer available and inc the event buffer index for next event usage
			g_freeEventBuffers--;
			g_eventBufferWriteIndex++;

			// Check if the event buffer index matches the max
			if (g_eventBufferWriteIndex == g_maxEventBuffers)
			{
				// Reset event buffer index to the start
				g_eventBufferWriteIndex = 0;
			}

			// Check if maximum consecutive events have not been captured, therefore pend Cal pulse
			if (s_consecEventsWithoutCal < s_consecutiveEventsWithoutCalThreshold)
			{
				// Setup delay for Cal pulse (based on Pretrigger size)
				s_pendingCalCount = g_triggerRecord.trec.sample_rate / g_unitConfig.pretrigBufferDivider;
			}
			else // Max number of events have been captured, force a Cal pulse
			{
				// Time to handle the Cal pulse
				s_calPulse = YES;
				s_calSampleCount = START_CAL_SIGNAL;
			}
		}
	}
	//___________________________________________________________________________________________
	//___Check if handling cal pulse samples
	else if ((s_calPulse == YES) && (s_calSampleCount))
	{
		// Check if the SPI is available to access or already locked to handle a Cal pulse
		if ((g_spi1AccessLock == AVAILABLE) || (g_spi1AccessLock == CAL_PULSE_LOCK))
		{
			// If SPI lock is available, grab it and flag for cal pulse
			if (g_spi1AccessLock == AVAILABLE) { g_spi1AccessLock = CAL_PULSE_LOCK;	}
							
			// Check for the start of the Cal pulse and set low sensitivity and swap clock source for 1024 sample rate
			if (s_calSampleCount == START_CAL_SIGNAL)
			{
				// Check if on high sensitivity and if so set to low sensitivity for Cal pulse
				if (g_triggerRecord.srec.sensitivity == HIGH) { SetSeismicGainSelect(SEISMIC_GAIN_LOW); }

				g_testTimeSinceLastCalPulse = g_rtcSoftTimerTickCount;

				// Swap to alternate timer/counter for default 1024 sample rate for Cal
#if INTERNAL_SAMPLING_SOURCE
				DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
				tc_stop(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
				tc_start(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
				StartExternalRtcClock(SAMPLE_RATE_1K);
				AVR32_EIC.ICR.int1 = 1;
#endif

				s_calSampleCount = MAX_CAL_SAMPLES;
			}
			else // Cal pulse started
			{
				// Wait 5 samples to start cal signaling (~5 ms)
				if (s_calSampleCount == CAL_SAMPLE_COUNT_FIRST_TRANSITION_HIGH) { AdSetCalSignalHigh();	}	// (~10 ms)
				if (s_calSampleCount == CAL_SAMPLE_COUNT_SECOND_TRANSITION_LOW) { AdSetCalSignalLow(); }	// (~20 ms)
				if (s_calSampleCount == CAL_SAMPLE_COUNT_THIRD_TRANSITION_HIGH) { AdSetCalSignalHigh(); }	// (~10 ms)
				if (s_calSampleCount == CAL_SAMPLE_COUNT_FOURTH_TRANSITION_OFF) { AdSetCalSignalOff(); }	// (~55 ms)

				// Copy cal data to the event buffer cal section
				*(SAMPLE_DATA_STRUCT*)(s_calPtr[DEFAULT_CAL_BUFFER_INDEX]) = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;

				s_calPtr[DEFAULT_CAL_BUFFER_INDEX] += NUMBER_OF_CHANNELS_DEFAULT;
									
				// Check if a delayed cal pointer has been established
				if (s_calPtr[ONCE_DELAYED_CAL_BUFFER_INDEX] != NULL)
				{
					// Copy delayed cal data to event buffer cal section
					*(SAMPLE_DATA_STRUCT*)(s_calPtr[ONCE_DELAYED_CAL_BUFFER_INDEX]) = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;
					s_calPtr[ONCE_DELAYED_CAL_BUFFER_INDEX] += NUMBER_OF_CHANNELS_DEFAULT;
				}

				// Check if a second delayed cal pointer has been established
				if (s_calPtr[TWICE_DELAYED_CAL_BUFFER_INDEX] != NULL)
				{
					// Copy delayed cal data to event buffer cal section
					*(SAMPLE_DATA_STRUCT*)(s_calPtr[TWICE_DELAYED_CAL_BUFFER_INDEX]) = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;
					s_calPtr[TWICE_DELAYED_CAL_BUFFER_INDEX] += NUMBER_OF_CHANNELS_DEFAULT;
				}

				s_calSampleCount--;

				if (s_calSampleCount == 0)
				{
					//debug("\n--> Cal done\r\n");
					//usart_write_char(&AVR32_USART1, '&');
						
					// Reset all states and counters (that haven't already)
					s_calPulse = NO;
					s_consecSeismicTriggerCount = 0;
					s_consecAirTriggerCount = 0;
					s_consecEventsWithoutCal = 0;
					
					// Reset cal pointers to null
					s_calPtr[DEFAULT_CAL_BUFFER_INDEX] = NULL;
					s_calPtr[ONCE_DELAYED_CAL_BUFFER_INDEX] = NULL;
					s_calPtr[TWICE_DELAYED_CAL_BUFFER_INDEX] = NULL;

					// Signal for an event to be processed
					raiseSystemEventFlag(TRIGGER_EVENT);

					// Global flag to signal done handling an event
					g_busyProcessingEvent = NO;

					if (g_doneTakingEvents == PENDING) { g_doneTakingEvents = YES; }
					
					// Check if on high sensitivity and if so reset to high sensitivity after Cal pulse (done on low)
					if (g_triggerRecord.srec.sensitivity == HIGH) { SetSeismicGainSelect(SEISMIC_GAIN_HIGH); }

					// Swap back to original sampling rate
#if INTERNAL_SAMPLING_SOURCE
					DUMMY_READ(AVR32_TC.channel[TC_CALIBRATION_TIMER_CHANNEL].sr);
					tc_stop(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
					tc_start(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
					StartExternalRtcClock(g_triggerRecord.trec.sample_rate);
					AVR32_EIC.ICR.int1 = 1;
#endif

					// Invalidate the Pretrigger buffer until it's filled again
					s_pretriggerFull = NO;

					g_spi1AccessLock = AVAILABLE;
				}																
			}
		}
	}
	else
	{
#if 1 // Test
		if (g_freeEventBuffers)
		{
			// Should _never_ get here
			debugErr("Error in ISR processing\r\n");
		}
#else // Normal
		// Should _never_ get here
		debugErr("Error in ISR processing\r\n");
#endif
	}
}

#if 0 // Test (First attempt to buffer waveform data for processing outside of the ISR)
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void moveWaveformData_ISR_Inline(void)
{
	// Copy sample over to bargraph buffer
	*(SAMPLE_DATA_STRUCT*) = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;

	// Increment the write pointer
	 += NUMBER_OF_CHANNELS_DEFAULT;
	
	// Check if write pointer is beyond the end of the circular bounds
	// add code

	// Alert system that we have data in ram buffer, raise flag to calculate and move data to flash.
	raiseSystemEventFlag();
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void moveBargraphData_ISR_Inline(void)
{
	// Copy sample over to bargraph buffer
	*(SAMPLE_DATA_STRUCT*)g_bargraphDataWritePtr = *(SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff;

	// Increment the write pointer
	g_bargraphDataWritePtr += NUMBER_OF_CHANNELS_DEFAULT;
	
	// Check if write pointer is beyond the end of the circular bounds
	if (g_bargraphDataWritePtr >= g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

	// Alert system that we have data in ram buffer, raise flag to calculate and move data to flash.
	raiseSystemEventFlag(BARGRAPH_EVENT);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void checkForTemperatureDrift_ISR_Inline(void)
{
	// Get the delta between the current and previous temperature reading
	s_temperatureDelta = Abs((int16)(g_currentTempReading - g_previousTempReading));

	// Check if the delta in consecutive temp readings is greater than the max jump signifying a bogus temp reading that plagues this A/D part
	if (s_temperatureDelta < MAX_TEMPERATURE_JUMP_PER_SAMPLE)
	{
		// Update the previous temperature reading
		g_previousTempReading = g_currentTempReading;

		// Check if the stored temp reading is higher than the current reading
		if (g_storedTempReading > g_currentTempReading)
		{
			// Check if the delta is less than the margin of change required to signal an adjustment
			if ((g_storedTempReading - g_currentTempReading) > AD_TEMP_COUNT_FOR_ADJUSTMENT)
			{
				// Updated stored temp reading for future comparison
				g_storedTempReading = g_currentTempReading;

				// Re-init the update to get new offsets
				g_updateOffsetCount = 0;

				raiseSystemEventFlag(UPDATE_OFFSET_EVENT);
			}
		}
		// The current temp reading is equal or higher than the stored (result needs to be positive)
		else if ((g_currentTempReading - g_storedTempReading) > AD_TEMP_COUNT_FOR_ADJUSTMENT)
		{
			// Updated stored temp reading for future comparison
			g_storedTempReading = g_currentTempReading;

			// Re-init the update to get new offsets
			g_updateOffsetCount = 0;

			raiseSystemEventFlag(UPDATE_OFFSET_EVENT);
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void saveMaxPeaksLastThreeSecondsForSensorCal_ISR_Inline(void)
{
	s_sensorCalSampleCount--;

	if (s_R_channelReading > sensorCalPeaks[0].r) { sensorCalPeaks[0].r = s_R_channelReading; }
	if (s_V_channelReading > sensorCalPeaks[0].v) { sensorCalPeaks[0].v = s_V_channelReading; }
	if (s_T_channelReading > sensorCalPeaks[0].t) { sensorCalPeaks[0].t = s_T_channelReading; }
	if (s_A_channelReading > sensorCalPeaks[0].a) { sensorCalPeaks[0].a = s_A_channelReading; }

	if (s_sensorCalSampleCount == 0)
	{
		// Reset the counter
		s_sensorCalSampleCount = CALIBRATION_FIXED_SAMPLE_RATE;

		// Shift the Peaks out after a second (index 1 being the current peak for the last second, index 2 for the peak the second before, etc)
		sensorCalPeaks[3] = sensorCalPeaks[2];
		sensorCalPeaks[2] = sensorCalPeaks[1];
		sensorCalPeaks[1] = sensorCalPeaks[0];

		// Clear out the first Peak hold values
		memset(&sensorCalPeaks[0], 0, sizeof(SAMPLE_DATA_STRUCT));
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void applyOffsetAndCacheSampleData_ISR_Inline(void)
{
	// Apply channel zero offset
#if 1 // Normal
	s_R_channelReading -= g_channelOffset.r_offset;
	s_V_channelReading -= g_channelOffset.v_offset;
	s_T_channelReading -= g_channelOffset.t_offset;
	s_A_channelReading -= g_channelOffset.a_offset;
#else // Test (Mark cal pulse with identifable data when no sensor connected)
	if ((s_calPulse == YES) && (s_calSampleCount))
	{
		s_R_channelReading = (0x2000 + s_calSampleCount - 100);
		s_V_channelReading = (0x3000 + s_calSampleCount - 100);
		s_T_channelReading = (0x4000 + s_calSampleCount - 100);
		s_A_channelReading = (0x1000 + s_calSampleCount - 100);
	}
	else
	{
		s_R_channelReading -= g_channelOffset.r_offset;
		s_V_channelReading -= g_channelOffset.v_offset;
		s_T_channelReading -= g_channelOffset.t_offset;
		s_A_channelReading -= g_channelOffset.a_offset;
	}
#endif

	// Store the raw A/D data (adjusted for zero offset) into the Pretrigger buffer
	((SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff)->r = s_R_channelReading;
	((SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff)->v = s_V_channelReading;
	((SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff)->t = s_T_channelReading;
	((SAMPLE_DATA_STRUCT*)g_tailOfPretriggerBuff)->a = s_A_channelReading;

#if 0 // Test
	// Store samples in event buffer
	testSamples[samplesCollected++] = s_R_channelReading;
	testSamples[samplesCollected++] = s_V_channelReading;
	testSamples[samplesCollected++] = s_T_channelReading;
	testSamples[samplesCollected++] = s_A_channelReading;
	testSamples[samplesCollected++] = g_currentTempReading;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void getChannelDataWithReadbackWithTemp_ISR_Inline(void)
{
	//___________________________________________________________________________________________
	//___Sample Output with return config words for reference
	// R: 7f26 (e0d0) | V: 7f10 (e2d0) | T: 7f15 (e4d0) | A: 7f13 (e6d0) | Temp: dbe (b6d0)
	
	if (s_channelConfig == CHANNELS_R_AND_V_SCHEMATIC)
	{
		// Chan 0 - R
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_R_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe0d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe150) { s_channelSyncError = YES; }

		// Chan 1 - T
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_T_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe2d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe350) { s_channelSyncError = YES; }

		// Chan 2 - V
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_V_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe4d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe550) { s_channelSyncError = YES; }

		// Chan 3 - A
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_A_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe6d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe750) { s_channelSyncError = YES; }

		// Temperature
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, (uint16*)&g_currentTempReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xb6d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xb750) { s_channelSyncError = YES; }
	}
	else // (s_channelConfig == CHANNELS_R_AND_V_SWAPPED)
	{
		// Chan 0 - V
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_V_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe0d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe150) { s_channelSyncError = YES; }

		// Chan 1 - T
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_T_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe2d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe350) { s_channelSyncError = YES; }

		// Chan 2 - R
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_R_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe4d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe550) { s_channelSyncError = YES; }

		// Chan 3 - A
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_A_channelReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xe6d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xe750) { s_channelSyncError = YES; }

		// Temperature
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, (uint16*)&g_currentTempReading);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		//if (s_channelConfigReadBack != 0xb6d0) { s_channelSyncError = YES; }
		if (s_channelConfigReadBack != 0xb750) { s_channelSyncError = YES; }
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void getChannelDataNoReadbackWithTemp_ISR_Inline(void)
{
	if (s_channelConfig == CHANNELS_R_AND_V_SCHEMATIC)
	{
		// Chan 0 - R
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_R_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 1 - T
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_T_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 2 - V
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_V_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 3 - A
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_A_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Temperature
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, (uint16*)&g_currentTempReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	}
	else // (s_channelConfig == CHANNELS_R_AND_V_SWAPPED)
	{
		// Chan 0 - V
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_V_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 1 - T
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_T_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 2 - R
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_R_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 3 - A
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_A_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Temperature
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, (uint16*)&g_currentTempReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void getChannelDataNoReadbackNoTemp_ISR_Inline(void)
{
	//___________________________________________________________________________________________
	//___Sample Output with return config words for reference
	// R: 7f26 (e0d0) | V: 7f10 (e2d0) | T: 7f15 (e4d0) | A: 7f13 (e6d0) | Temp: dbe (b6d0)
	
	if (s_channelConfig == CHANNELS_R_AND_V_SCHEMATIC)
	{
		// Chan 0 - R
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_R_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 1 - T
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_T_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 2 - V
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_V_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 3 - A
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_A_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	}
	else // (s_channelConfig == CHANNELS_R_AND_V_SWAPPED)
	{
		// Chan 0 - V
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_V_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 1 - T
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_T_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 2 - R
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_R_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);

		// Chan 3 - A
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_A_channelReading);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static inline void HandleChannelSyncError_ISR_Inline(void)
{
	debugErr("AD Channel Sync Error\r\n");

	// Attempt channel recovery (with a channel read to get the config read back value)
	spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
	spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack); // Data insignificant
	spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack); // Config read back
	spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
		
	switch (s_channelConfigReadBack)
	{
#if 0 // Old channel config
		case 0xe0d0: s_channelReadsToSync = 4; break; // R Chan
		case 0xe2d0: s_channelReadsToSync = 3; break; // T Chan
		case 0xe4d0: s_channelReadsToSync = 2; break; // V Chan
		case 0xe6d0: s_channelReadsToSync = 1; break; // A Chan
		case 0xb6d0: s_channelReadsToSync = 0; break; // Temp Chan
#else
		case 0xe150: s_channelReadsToSync = 4; break; // R Chan
		case 0xe350: s_channelReadsToSync = 3; break; // T Chan
		case 0xe550: s_channelReadsToSync = 2; break; // V Chan
		case 0xe750: s_channelReadsToSync = 1; break; // A Chan
		case 0xb750: s_channelReadsToSync = 0; break; // Temp Chan
#endif

		default: 
			s_channelReadsToSync = 0; // Houston, we have a problem... all channels read and unable to match
			debugErr("Error: ISR Processing AD --> Unable to Sync channels, data collection broken\r\n");
			break;
	}
		
	while (s_channelReadsToSync--)
	{
		// Dummy reads to realign channel processing
		spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack); // Data insignificant
		spi_write(&AVR32_SPI0, 0x0000); spi_read(&AVR32_SPI0, &s_channelConfigReadBack); // Config read back
		spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DataIsrInit(void)
{
	s_pretriggerFull = NO;
	s_checkForTempDrift = NO;

	if ((g_maxEventBuffers - 1) < CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD)
	{
		s_consecutiveEventsWithoutCalThreshold = (g_maxEventBuffers - 1);
	}
	else { s_consecutiveEventsWithoutCalThreshold = CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD; }

	if (g_factorySetupRecord.analogChannelConfig == CHANNELS_R_AND_V_SWAPPED) { s_channelConfig = CHANNELS_R_AND_V_SWAPPED; }
	else { s_channelConfig = CHANNELS_R_AND_V_SCHEMATIC; }
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
__attribute__((__interrupt__))
void Tc_sample_irq(void)
{
	g_testAfterSleepISR = Get_system_register(AVR32_COUNT);

#if 0 // Test
	static uint32 isrCounter = 0;

	isrCounter++;
	debugRaw("I(%d)", isrCounter);
#endif

#if EXTERNAL_SAMPLING_SOURCE
	static uint8 skipProcessingFor512 = 0;
	
	//___________________________________________________________________________________________
	//___Test timing (throw away at some point)
	if (g_triggerRecord.trec.sample_rate == 512)
	{
		// Toggle the bit to toss away half the samples for a 512 sample rate clocked at 1024
		skipProcessingFor512 ^= 1;
		
		if (skipProcessingFor512 == 0)
		{
			AVR32_EIC.ICR.int1 = 1;
			return;
		}
	}
#endif

#if 1 // Test
	//___________________________________________________________________________________________
	//___Revert power savings for sleep
	if (g_powerSavingsForSleepEnabled == YES)
	{
extern inline void RevertPowerSavingsAfterSleeping(void);
		RevertPowerSavingsAfterSleeping();
	}
#endif

	//___________________________________________________________________________________________
	//___Test timing (throw away at some point)
	g_sampleCount++;

	//___________________________________________________________________________________________
	//___AD raw data read all 4 channels without config read back and no temp
	if (g_adChannelConfig == FOUR_AD_CHANNELS_NO_READBACK_NO_TEMP)
	{
		getChannelDataNoReadbackNoTemp_ISR_Inline();
	}
	//___________________________________________________________________________________________
	//___AD raw data read all 4 channels without config read back and temp
	else if (g_adChannelConfig == FOUR_AD_CHANNELS_NO_READBACK_WITH_TEMP)
	{
		getChannelDataNoReadbackWithTemp_ISR_Inline();

		//___________________________________________________________________________________________
		//___Check for temperature drift (only will start after Pretrigger buffer is full initially)
		if (s_checkForTempDrift == YES)
		{
			checkForTemperatureDrift_ISR_Inline();
		}
	}
	//___________________________________________________________________________________________
	//___AD raw data read all 4 channels with config read back and temp
	else // (g_adChannelConfig == FOUR_AD_CHANNELS_WITH_READBACK_WITH_TEMP)
	{
		getChannelDataWithReadbackWithTemp_ISR_Inline();
		
		//___________________________________________________________________________________________
		//___Check for channel sync error
		if (s_channelSyncError == YES)
		{
			g_breakpointCause = BP_AD_CHAN_SYNC_ERR;
			__asm__ __volatile__ ("breakpoint");

			HandleChannelSyncError_ISR_Inline();

			// clear the interrupt flags and bail
#if INTERNAL_SAMPLING_SOURCE
			DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
			DUMMY_READ(AVR32_TC.channel[TC_CALIBRATION_TIMER_CHANNEL].sr);
#elif EXTERNAL_SAMPLING_SOURCE
			AVR32_EIC.ICR.int1 = 1;
#endif
			return;
		}

		//___________________________________________________________________________________________
		//___Check for temperature drift (only will start after Pretrigger buffer is full initially)
		if (s_checkForTempDrift == YES)
		{
			checkForTemperatureDrift_ISR_Inline();
		}
	}		

	//___________________________________________________________________________________________
	//___AD data read successfully, Normal operation
	applyOffsetAndCacheSampleData_ISR_Inline();

	//___________________________________________________________________________________________
	//___Check if not actively sampling
	if (g_sampleProcessing == IDLE_STATE)
	{
		// Idle and not processing, could adjust the channel offsets

		// Isolate processing for just the sensor calibration, testing feature (remove reference and check at some point)
		if (g_activeMenu == CAL_SETUP_MENU)
		{
			normalizeSampleData_ISR_Inline();
			saveMaxPeaksLastThreeSecondsForSensorCal_ISR_Inline();
		}
	}
	//___________________________________________________________________________________________
	//___Check if the Pretrigger buffer is not full, which is necessary for an event
	else if (s_pretriggerFull == NO)
	{
		fillPretriggerBufferUntilFull_ISR_Inline();
	}
	//___________________________________________________________________________________________
	//___Check if handling a Manual Cal
	else if (g_manualCalFlag == TRUE)
	{
		processAndMoveManualCalData_ISR_Inline();
	}
	//___________________________________________________________________________________________
	//___Handle all modes not manual cal
	else
	{
		// Normalize sample data to zero (positive) for comparison
		normalizeSampleData_ISR_Inline();
	
		// Alarm checking section
		checkAlarms_ISR_Inline();

		//___________________________________________________________________________________________
		//___Process and move the sample data for triggers in waveform or combo mode
		if ((g_triggerRecord.opMode == WAVEFORM_MODE) || (g_triggerRecord.opMode == COMBO_MODE))
		{
			processAndMoveWaveformData_ISR_Inline();
		}

		//___________________________________________________________________________________________
		//___Move the sample data for bar calculations in bargraph or combo mode (when combo mode isn't handling a cal pulse)
		if ((g_triggerRecord.opMode == BARGRAPH_MODE) || ((g_triggerRecord.opMode == COMBO_MODE) && (s_calPulse != YES)))
		{
			moveBargraphData_ISR_Inline();
		}
	}			

	//___________________________________________________________________________________________
	//___Advance to the next sample in the buffer
	g_tailOfPretriggerBuff += g_sensorInfo.numOfChannels;

	// Check if the end of the Pretrigger buffer has been reached
	if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;

#if 0 // Test
	gpio_clr_gpio_pin(AVR32_PIN_PB20);
#endif

	// clear the interrupt flag
#if INTERNAL_SAMPLING_SOURCE
	DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
	DUMMY_READ(AVR32_TC.channel[TC_CALIBRATION_TIMER_CHANNEL].sr);
#elif EXTERNAL_SAMPLING_SOURCE
	AVR32_EIC.ICR.int1 = 1;
#endif

#if 1 // Test reads to clear the bus
	PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif
}
