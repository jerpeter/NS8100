///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Menu.h"
#include "Uart.h"
#include "Display.h"
#include "Common.h"
#include "InitDataBuffers.h"
#include "Summary.h"
#include "SysEvents.h"
#include "Board.h"
#include "PowerManagement.h"
#include "Record.h"
#include "SoftTimer.h"
#include "Keypad.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "TextTypes.h"
#include "EventProcessing.h"
#include "Analog.h"
#include "M23018.h"
#include "Globals.h"
#include "RealTimeClock.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern void Stop_Data_Clock(TC_CHANNEL_NUM);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void DataIsrInit(void);
void StartDataCollection(uint32 sampleRate);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StartMonitoring(TRIGGER_EVENT_DATA_STRUCT trig_mn, uint8 op_mode)
{ 
	// Check if any events are still stored in buffers and need to be stored into flash
	if (getSystemEventState(TRIGGER_EVENT))
	{
		while (getSystemEventState(TRIGGER_EVENT))
		{
			MoveWaveformEventToFlash();
		}
	}

	// Display a message to be patient while the software disables the power off key and setups up parameters
	OverlayMessage(getLangText(STATUS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT), 0);

	if (GetPowerControlState(POWER_OFF_PROTECTION_ENABLE) == OFF)
	{
		debug("Start Trigger: Enabling Power Off Protection\n");

		// Enable power off protection
		PowerControl(POWER_OFF_PROTECTION_ENABLE, ON);
	}

	// Assign a one second menu update timer
	AssignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, MenuUpdateTimerCallBack);

#if 0 	// Moved to system init
	// Assign a one second menu keypad led update timer
	//AssignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, KeypadLedUpdateTimerCallBack);
#endif

	// This is for error checking, If these checks are true, the defaults are not being set.
	if ((op_mode == WAVEFORM_MODE) || (op_mode == COMBO_MODE))
	{
		if ((g_helpRecord.alarmOneMode == ALARM_MODE_SEISMIC) || (g_helpRecord.alarmOneMode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarmOneSeismicLevel < trig_mn.seismicTriggerLevel)
			{
				g_helpRecord.alarmOneSeismicLevel = trig_mn.seismicTriggerLevel;
			}
		}

		if ((g_helpRecord.alarmTwoMode == ALARM_MODE_SEISMIC) || (g_helpRecord.alarmTwoMode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarmTwoSeismicLevel < trig_mn.seismicTriggerLevel)
			{
				g_helpRecord.alarmTwoSeismicLevel = trig_mn.seismicTriggerLevel;
			}
		}

		if ((g_helpRecord.alarmOneMode == ALARM_MODE_AIR) || (g_helpRecord.alarmOneMode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarmOneAirLevel < (uint32)trig_mn.airTriggerLevel)
			{
				g_helpRecord.alarmOneAirLevel = (uint32)trig_mn.airTriggerLevel;
			}
		}

		if ((g_helpRecord.alarmTwoMode == ALARM_MODE_AIR) || (g_helpRecord.alarmTwoMode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarmTwoAirLevel < (uint32)trig_mn.airTriggerLevel)
			{
				g_helpRecord.alarmTwoAirLevel = (uint32)trig_mn.airTriggerLevel;
			}
		}
	}

	//-------------------------
	// Debug Information
	//-------------------------

	if (op_mode == WAVEFORM_MODE)
	{
		debug("--- Waveform Mode Settings ---\n");
		debug("\tRecord Time: %d, Sample Rate: %d, Channels: %d\n", g_triggerRecord.trec.record_time, trig_mn.sample_rate, g_sensorInfoPtr->numOfChannels);

		if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
		{
			debug("\tSeismic Trigger Count: 0x%x, Air Level: %d dB, Air Trigger Count: 0x%x\n", trig_mn.seismicTriggerLevel, AirTriggerConvertToUnits(trig_mn.airTriggerLevel), trig_mn.airTriggerLevel);
		}
		else // (g_helpRecord.unitsOfAir == MILLIBAR_TYPE)
		{
			debug("\tSeismic Trigger Count: 0x%x, Air Level: %0.3f mb, Air Trigger Count: 0x%x\n", trig_mn.seismicTriggerLevel,
					((float)AirTriggerConvertToUnits(trig_mn.airTriggerLevel) / (float)10000), trig_mn.airTriggerLevel);
		}
	}
	else if (op_mode == BARGRAPH_MODE)
	{
		debug("--- Bargraph Mode Settings ---\n");
		debug("\tSample Rate: %d, Bar Interval: %d secs, Summary Interval: %d mins\n", trig_mn.sample_rate, g_triggerRecord.bgrec.barInterval,
				(g_triggerRecord.bgrec.summaryInterval / 60));
	}
	else if (op_mode == COMBO_MODE)
	{
		debug("--- Combo Mode Settings ---\n");
		debug("\tRecord Time: %d, Sample Rate: %d, Channels: %d\n", g_triggerRecord.trec.record_time, trig_mn.sample_rate, g_sensorInfoPtr->numOfChannels);

		if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
		{
			debug("\tSeismic Trigger Count: 0x%x, Air Level: %d dB, Air Trigger Count: 0x%x\n", trig_mn.seismicTriggerLevel, AirTriggerConvertToUnits(trig_mn.airTriggerLevel), trig_mn.airTriggerLevel);
		}
		else // (g_helpRecord.unitsOfAir == MILLIBAR_TYPE)
		{
			debug("\tSeismic Trigger Count: 0x%x, Air Level: %0.3f mb, Air Trigger Count: 0x%x\n", trig_mn.seismicTriggerLevel,
					((float)AirTriggerConvertToUnits(trig_mn.airTriggerLevel) / (float)10000), trig_mn.airTriggerLevel);
		}

		debug("\tBar Interval: %d secs, Summary Interval: %d mins\n", g_triggerRecord.bgrec.barInterval, (g_triggerRecord.bgrec.summaryInterval / 60));
	}
	else if (op_mode == MANUAL_TRIGGER_MODE)
	{
		debug("--- Manual Trigger Mode Settings ---\n");
	}
	else if (op_mode == MANUAL_CAL_MODE)
	{
		debug("--- Manual Cal Mode Settings ---\n");
		debug("\tSample Rate: %d, Channels: %d\n", trig_mn.sample_rate, g_sensorInfoPtr->numOfChannels);
	}

	debug("---------------------------\n");

	// Check if mode is Manual Cal
	if (op_mode == MANUAL_CAL_MODE)
	{
		// Raise flag
		g_manualCalFlag = TRUE;
		g_manualCalSampleCount = MAX_CAL_SAMPLES;
	}
	else // Waveform, Bargraph, Combo
	{
		// Clear flag
		g_manualCalFlag = FALSE;
		g_manualCalSampleCount = 0;

		// Create a new monitor log entry
		NewMonitorLogEntry(op_mode);
	}

	if ((op_mode == BARGRAPH_MODE) || (op_mode == COMBO_MODE))
	{
		g_fileProcessActiveUsbLockout = ON;
		BargraphForcedCalibration();
	}

	// Initialize buffers and settings and gp_ramEventRecord
	debug("Init data buffers\n");
	InitDataBuffs(op_mode);

	// Setup Analog controls
	debug("Setup Analog controls\n");

	// Set the cutoff frequency based on sample rate
	switch (trig_mn.sample_rate)
	{
		case SAMPLE_RATE_512: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_LOW); break;
		case SAMPLE_RATE_1K: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1); break;
		case SAMPLE_RATE_2K: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_2); break;
		case SAMPLE_RATE_4K: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_3); break;

		// Handle 8K, 16K and 32K the same
		case SAMPLE_RATE_8K: case SAMPLE_RATE_16K: case 32768: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_4); break;
			
		// Default just in case it's a custom frequency
		default: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1); break;
	}
	
	// Set the sensitivity based on the current settings
	if (g_triggerRecord.srec.sensitivity == LOW) { SetSeismicGainSelect(SEISMIC_GAIN_LOW); }
	else { SetSeismicGainSelect(SEISMIC_GAIN_HIGH); }
	
	if (g_factorySetupRecord.aweight_option == DISABLED) { SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL); }
	else { SetAcousticGainSelect(ACOUSTIC_GAIN_A_WEIGHTED);	}

#if 0 // Necessary? Probably need 1 sec for changes, however 1 sec worth of samples thrown away with getting channel offsets 
	// Delay for Analog cutoff and gain select changes to propagate
	SoftUsecWait(500 * SOFT_MSECS);
#endif

	// Monitor some data.. oh yeah
	StartDataCollection(trig_mn.sample_rate);
}

#if 1
extern void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM channel);
extern void Start_Data_Clock(TC_CHANNEL_NUM channel);
extern void SetupADChannelConfig(uint32 sampleRate);
extern void Setup_8100_EIC_External_RTC_ISR(void);
#endif
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StartDataCollection(uint32 sampleRate)
{
	// Enable the A/D
	debug("Enable the A/D\n");
	PowerControl(ANALOG_SLEEP_ENABLE, OFF);

	// Delay to allow AD to power up/stabilize
	SoftUsecWait(50 * SOFT_MSECS);

	// Setup the A/D Channel configuration
	SetupADChannelConfig(sampleRate);
	
	// Get current A/D offsets for normalization
	debug("Getting channel offsets...\n");
	//OverlayMessage(getLangText(STATUS_TEXT), getLangText(CALIBRATING_TEXT), 0);
	GetChannelOffsets(sampleRate);

	debug("Setup TC clocks...\n");
	// Setup ISR to clock the data sampling

#if INTERNAL_SAMPLING_SOURCE
	Setup_8100_TC_Clock_ISR(sampleRate, TC_SAMPLE_TIMER_CHANNEL);
	Setup_8100_TC_Clock_ISR(CAL_PULSE_FIXED_SAMPLE_RATE, TC_CALIBRATION_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
	Setup_8100_EIC_External_RTC_ISR();
#endif

	// Init a few key values for data collection
	DataIsrInit();

	debug("Start sampling...\n");
	// Start the timer for collecting data
#if INTERNAL_SAMPLING_SOURCE
	Start_Data_Clock(TC_SAMPLE_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
#if 1 // Normal
	StartExternalRtcClock(sampleRate);
#else
	StartExternalRtcClock(1);
#endif
#endif

	// Change state to start processing the samples
	debug("Raise signal to start sampling\n");
	g_sampleProcessing = ACTIVE_STATE;

	// Test - Throw away at some point
	//debugRaw("\nA1M (%d)\n", g_helpRecord.alarmOneMode);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StopMonitoring(uint8 mode, uint8 operation)
{
	OverlayMessage(getLangText(STATUS_TEXT), "CLOSING MONITOR SESSION...", 0);

	// Check if the unit is currently monitoring
	if (g_sampleProcessing == ACTIVE_STATE)
	{
		if ((mode == WAVEFORM_MODE) || (mode == COMBO_MODE))
		{
			// Set flag to prevent any more incoming events from being processed
			g_doneTakingEvents = PENDING;

			// Wait for any triggered events to finish sending
			WaitForEventProcessingToFinish();
		}
		
#if 0 // ns7100 (Serves no useful purpose)
		if (((mode == BARGRAPH_MODE) || (mode == COMBO_MODE)) && (operation == EVENT_PROCESSING))
		{
			raiseSystemEventFlag(BARGRAPH_EVENT);
		}
#endif

		// Stop the data transfers
		StopDataCollection();

		// Reset the Waveform flag
		g_doneTakingEvents = NO;

		if ((mode == WAVEFORM_MODE) && (operation == FINISH_PROCESSING))
		{
			while (getSystemEventState(TRIGGER_EVENT))
			{
				debug("Handle Waveform Trigger Event Completion\n");
				MoveWaveformEventToFlash();
			}
		}

		if ((mode == COMBO_MODE) && (operation == FINISH_PROCESSING))
		{
			while (getSystemEventState(TRIGGER_EVENT))
			{
				debug("Handle Combo - Waveform Trigger Event Completion\n");
				MoveComboWaveformEventToFile();
			}
		}

		if (mode == BARGRAPH_MODE)
		{
			// Handle the end of a Bargraph event
			debug("Handle End of Bargraph event\n");
			EndBargraph();
		}
		
		if (mode == COMBO_MODE)
		{
			// Handle the end of a Combo Bargraph event
			debug("Handle End of Combo - Bargraph event\n");
			EndCombo();
		}
		
		// Check if any events are waiting to still be processed
		if (!getSystemEventState(TRIGGER_EVENT))
		{
			CloseMonitorLogEntry();
		}
		
#if 0 // Test (Display LCD message with mode for timer mode ending)
		char modeBuff[10];
		char msgBuff[50];
		if (mode == WAVEFORM_MODE)
			sprintf(&modeBuff[0], "%s", getLangText(WAVEFORM_MODE_TEXT));
		else if (mode == BARGRAPH_MODE)
			sprintf(&modeBuff[0], "%s", getLangText(BARGRAPH_MODE_TEXT));
		else if (mode == COMBO_MODE)
			sprintf(&modeBuff[0], "%s", getLangText(COMBO_MODE_TEXT));
		sprintf(&msgBuff[0], "%s MONITORING AND EVENT CLOSURE COMPLETE", modeBuff);
		OverlayMessage(getLangText(TIMER_MODE_TEXT), msgBuff, 3 * SOFT_SECS);
#endif		
	}
	
	// Turn on the Green keypad LED
	WriteMcp23018(IO_ADDRESS_KPD, GPIOA, ((ReadMcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | GREEN_LED_PIN));

	// Check if Auto Monitor is active and not in monitor mode
	if ((g_helpRecord.autoMonitorMode != AUTO_NO_TIMEOUT) && (operation == EVENT_PROCESSING))
	{
		AssignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_helpRecord.autoMonitorMode * TICKS_PER_MIN), AutoMonitorTimerCallBack);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StopDataCollection(void)
{
	g_sampleProcessing = IDLE_STATE;

#if INTERNAL_SAMPLING_SOURCE
	Stop_Data_Clock(TC_SAMPLE_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
	StopExternalRtcClock();
#endif

	PowerControl(ANALOG_SLEEP_ENABLE, ON);

	ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
	
#if 0 // Moved to system init
	//ClearSoftTimer(KEYPAD_LED_TIMER_NUM);
#endif

	// Check if not in Timer Mode and if the Power Off protection is enabled
	if ((g_helpRecord.timerMode != ENABLED) && (GetPowerControlState(POWER_OFF_PROTECTION_ENABLE) == ON))
	{
		// Disable power off protection
		debug("Stop Trigger: Disabling Power Off Protection\n");
		PowerControl(POWER_OFF_PROTECTION_ENABLE, OFF);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StopDataClock(void)
{
	g_sampleProcessing = IDLE_STATE;

#if INTERNAL_SAMPLING_SOURCE
	Stop_Data_Clock(TC_SAMPLE_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
	StopExternalRtcClock();
#endif

	PowerControl(ANALOG_SLEEP_ENABLE, ON);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WaitForEventProcessingToFinish(void)
{
	if (g_doneTakingEvents == PENDING)
	{
		debug("ISR Monitor process is still pending\n");
		
		OverlayMessage(getLangText(STATUS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT), 0);

		while (g_doneTakingEvents == PENDING)
		{
#if 0 // Test (? Throw away at some point)
			if (getSystemEventState())
			{
				clearSystemEventFlag();

extern void processAndMoveWaveformData_ISR_Inline(void);
				processAndMoveWaveformData_ISR_Inline();		
			}		
#else // Normal
			// Just wait for the cal and end immediately afterwards
			SoftUsecWait(250);
#endif
		}
	}
}

#if 0 // ns7100 (Used to convert trigger level for the 430)
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 SeismicTriggerConvert(float seismicTriggerLevel)
{
    uint16 seisTriggerVal;
    uint8 gainFactor = (uint8)((g_triggerRecord.srec.sensitivity == LOW) ? 2 : 4);
    float convertToHex = (float)(g_factorySetupRecord.sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_100X_SHIFT);
    
    convertToHex = (float)ADC_RESOLUTION / (float)convertToHex;
  
	if ((seismicTriggerLevel != NO_TRIGGER_CHAR) && (seismicTriggerLevel != MANUAL_TRIGGER_CHAR))
    {
		// Convert the trigger level into a hex value for the 430 processor board.
		seisTriggerVal = (uint16)(((float)convertToHex * (float)seismicTriggerLevel) + (float)0.5);	
			
		//debug("SeismicTriggerConvert: seismicTriggerLevel=%f seisTriggerVal = 0x%x convertToHex=%d\n",
		//	seismicTriggerLevel, seisTriggerVal, convertToHex);
	}
	else
	{
		seisTriggerVal = (uint16)seismicTriggerLevel;
	}

	return (SwapInt(seisTriggerVal));
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 AirTriggerConvert(uint32 airTriggerToConvert)
{
	// Check if the air trigger level is not no trigger and not manual trigger
	if ((airTriggerToConvert != NO_TRIGGER_CHAR) && (airTriggerToConvert != MANUAL_TRIGGER_CHAR) && (airTriggerToConvert != EXTERNAL_TRIGGER_CHAR))
	{
#if 0 // Port lost change
		// Convert the float db to an offset from 0 to 2048 and upscale to 16-bit
		airTriggerToConvert = (DbToHex(airTriggerToConvert) * 16);
#else // Updated
		if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
		{
			// Convert dB to an offset from 0 to 2048 and upscale to 16-bit
			airTriggerToConvert = (uint32)(DbToHex(airTriggerToConvert) * 16);
		}
		else
		{
			// Convert mb to an offset from 0 to 2048 and upscale to 16-bit
			airTriggerToConvert = (uint32)(MbToHex(airTriggerToConvert) * 16);
		}
#endif
	}

	return (airTriggerToConvert);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint32 AirTriggerConvertToUnits(uint32 airTriggerToConvert)
{
	// Check if the air trigger level is not no trigger and not manual trigger
	if ((airTriggerToConvert != NO_TRIGGER_CHAR) && (airTriggerToConvert != MANUAL_TRIGGER_CHAR) && (airTriggerToConvert != EXTERNAL_TRIGGER_CHAR))
	{
		if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
		{
			airTriggerToConvert = HexToDB(airTriggerToConvert, DATA_NORMALIZED, ACCURACY_16_BIT_MIDPOINT);
		}
		else // (g_helpRecord.unitsOfAir == MILLIBAR_TYPE)
		{
			airTriggerToConvert = (HexToMB(airTriggerToConvert, DATA_NORMALIZED, ACCURACY_16_BIT_MIDPOINT) * 10000);
		}
	}

	return (airTriggerToConvert);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleManualCalibration(void)
{
	FLASH_USAGE_STRUCT flashStats;
	//uint32 manualCalProgressCheck = MAX_CAL_SAMPLES;
	//uint8 holdOpMode;

	// Check if currently monitoring
	if (g_sampleProcessing == ACTIVE_STATE)
	{
		// Check if Waveform or Combo mode and not handling a cached event
		if (((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE)) && 
			(getSystemEventState(TRIGGER_EVENT) == NO))
		{
			// Check if still waiting for an event and not processing a cal and not waiting for a cal
			if (g_busyProcessingEvent == NO)
			{
				GetFlashUsageStats(&flashStats);
				
				// fix_ns8100
				if ((g_helpRecord.flashWrapping == NO) && (flashStats.manualCalsLeft == 0))
				{
					OverlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
				}
				else
				{
					// Stop data transfer
					StopDataClock();

					// Perform Cal while in monitor mode
					OverlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 0);

					InitDataBuffs(MANUAL_CAL_MODE);
					g_manualCalFlag = TRUE;
					g_manualCalSampleCount = MAX_CAL_SAMPLES;

					// Make sure Cal pulse is generated at low sensitivity
					SetSeismicGainSelect(SEISMIC_GAIN_LOW);

					StartDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);

					// Wait for the Cal Pulse to complete, 250ms + 100ms
					SoftUsecWait(350 * SOFT_MSECS);

					// Just make absolutely sure we are done with the Cal pulse
					while ((volatile uint32)g_manualCalSampleCount != 0) { }

					// Stop data transfer
					StopDataClock();

					if (getSystemEventState(MANUEL_CAL_EVENT))
						MoveManuelCalToFlash();

					InitDataBuffs(g_triggerRecord.op_mode);
					g_manualCalFlag = FALSE;
					g_manualCalSampleCount = 0;

					// Make sure to reset the gain after the Cal pulse (which is generated at low sensitivity)
					if (g_triggerRecord.srec.sensitivity == HIGH)
					{
						SetSeismicGainSelect(SEISMIC_GAIN_HIGH);
					}

					StartDataCollection(g_triggerRecord.trec.sample_rate);
				}
			}
		}					
	}
	else // Performing Cal outside of monitor mode
	{
		GetFlashUsageStats(&flashStats);
		
		// fix_ns8100
		if ((g_helpRecord.flashWrapping == NO) && (flashStats.manualCalsLeft == 0))
		{
			OverlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
		}
		else
		{
			OverlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 0);

			InitDataBuffs(MANUAL_CAL_MODE);
			g_manualCalFlag = TRUE;
			g_manualCalSampleCount = MAX_CAL_SAMPLES;

			// Make sure Cal pulse is generated at low sensitivity
			SetSeismicGainSelect(SEISMIC_GAIN_LOW);

			StartDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
			
			// Wait for the Cal Pulse to complete, 250ms + 100ms
			SoftUsecWait(350 * SOFT_MSECS);

			// Just make absolutely sure we are done with the Cal pulse
			while ((volatile uint32)g_manualCalSampleCount != 0) { }

			// Stop data transfer
			StopDataClock();

			if (getSystemEventState(MANUEL_CAL_EVENT))
				MoveManuelCalToFlash();

			g_manualCalFlag = FALSE;
			g_manualCalSampleCount = 0;
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void BargraphForcedCalibration(void)
{
	INPUT_MSG_STRUCT mn_msg;
	uint8 pendingMode = g_triggerRecord.op_mode;
	
	OverlayMessage(getLangText(STATUS_TEXT), "PERFORMING CALIBRATION", 0);

	g_bargraphForcedCal = YES;

	InitDataBuffs(MANUAL_CAL_MODE);
	g_manualCalFlag = TRUE;
	g_manualCalSampleCount = MAX_CAL_SAMPLES;

	// Make sure Cal pulse is generated at low sensitivity
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	
#if 0 // ns7100
	// Temp set mode to waveform to force the 430 ISR to call ProcessWaveformData instead of ProcessBargraphData 
	// after the calibration finishes to prevent a lockup when bargraph references globals that are not inited yet
	g_triggerRecord.op_mode = WAVEFORM_MODE;

	// Set flag to Sampling, we are about to begin to sample.
	g_sampleProcessing = ACTIVE_STATE;

	// Send message to 430
	//ISPI_SendMsg(MANUAL_CAL_PULSE_CMD);
#else // ns8100
	StartDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
#endif

	// No longer needed, handled in the ISR for Cal
	//GenerateCalSignal();

	// Wait for the Cal Pulse to complete, 250ms + 100ms
	SoftUsecWait(350 * SOFT_MSECS);

	// Just make absolutely sure we are done with the Cal pulse
	while ((volatile uint32)g_manualCalSampleCount != 0) { /* spin */ }

	// Stop data transfer
	StopDataClock();

	g_manualCalFlag = FALSE;
	g_manualCalSampleCount = 0;

	if (getSystemEventState(MANUEL_CAL_EVENT))
		MoveManuelCalToFlash();

	if (getMenuEventState(RESULTS_MENU_EVENT)) 
	{
		clearMenuEventFlag(RESULTS_MENU_EVENT);

#if 0 // fix_ns8100
		SETUP_RESULTS_MENU_MANUEL_CAL_MSG(RESULTS_MENU);

		JUMP_TO_ACTIVE_MENU();
#else
		UNUSED(mn_msg);
#endif
	}

	// Wait until after the Cal Pulse has completed
	SoftUsecWait(1 * SOFT_SECS);

	g_activeMenu = MONITOR_MENU;

	g_bargraphForcedCal = NO;

	// Reset the mode back to Bargraph
	g_triggerRecord.op_mode = pendingMode;

	UpdateMonitorLogEntry();
}
