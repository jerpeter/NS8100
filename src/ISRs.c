///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: ISRs.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:49 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ISRs.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Ispi.h"
#include "Common.h"
#include "Record.h"
#include "Menu.h"
#include "InitDataBuffers.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "SysEvents.h"
#include "Uart.h"
#include "Old_Board.h"
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
#include "ad_test_menu.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define DUMMY_READ(x) if ((volatile int)x == 0) {}

#if SUPERGRAPH_UNIT
#define ESC_KEY_ROW			0x02
#define ESC_KEY_POSITION	0x20
#else // MINIGRAPH_UNIT
#define ESC_KEY_ROW			0x04
#define ESC_KEY_POSITION	0x01
#endif

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern void rtc_clear_interrupt(volatile avr32_rtc_t *rtc);
extern void rtc_enable_interrupt(volatile avr32_rtc_t *rtc);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

#if 0 // ns7100
/*******************************************************************************
* Function: isr_PowerOffKey
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_PowerOffKey(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '@';

	if (g_sampleProcessing != ACTIVE_STATE)
	{
		// Check if timer mode is enabled and power off enable has been turned off
		if ((g_helpRecord.timer_mode == ENABLED) && (getPowerControlState(POWER_SHUTDOWN_ENABLE) == OFF))
		{
			// Signal a Power Off event
			raiseSystemEventFlag(POWER_OFF_EVENT);
		}
		else if (getPowerControlState(POWER_SHUTDOWN_ENABLE) == ON)
		{
			// Unit is already starting to power down, stop interrupts to prevent the proc from waking back up
			RTC_ENABLES.bit.periodicIntEnable = OFF;
			RTC_ENABLES.bit.powerFailIntEnable = OFF;
		}
	}
	else
	{
		// Check if the ESC key is being pressed as well
		if (~(*(uint8*)KEYPAD_ADDRESS) & ESC_KEY_POSITION)
		{
			// Stop watchdog timer from waking up processor
			RTC_ENABLES.bit.periodicIntEnable = OFF;
			RTC_ENABLES.bit.powerFailIntEnable = OFF;

			// Enable the power off Control
			powerControl(POWER_SHUTDOWN_ENABLE, ON);
		}

		// Clear the processor interrupt flag
		mmc_clear_EPF4_int;
	}

	// Clear the processor interrupt flag
	mmc_clear_EPF1_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_SCI1
* Purpose: Handle serial data coming into the processor
*******************************************************************************/
//#pragma interrupt on
extern uint8 messageBoxActiveFlag;
extern uint8 messageBoxKeyInput;
void isr_SCI1(void)
{
	uint8 statusReg = 0;
	uint8 dataReg = 0;

    MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '_';

	// Read the Status register
	statusReg = imm->Sci1.SCISR1;

	// Read out the data register to clear the interrupt
	dataReg = imm->Sci1.SCIDRL;

#if 1
	{
		auto INPUT_MSG_STRUCT msg;
		auto uint8 keyPress = 0;

		switch(dataReg)
		{
			case 'u' : keyPress = KEY_UPARROW; break;
			case 'd' : keyPress = KEY_DOWNARROW; break;
			case 'e' : keyPress = KEY_ENTER; break;
			case '-' : keyPress = KEY_MINUS; break;
			case '+' : keyPress = KEY_PLUS; break;
			case 'h' : keyPress = KEY_HELP; break;
			case 'b' : keyPress = KEY_BACKLIGHT; break;
			case 'x' : keyPress = KEY_ESCAPE; break;
		}

		if (messageBoxActiveFlag == YES)
		{
			messageBoxKeyInput = keyPress;
		}
		else
		{
			messageBoxKeyInput = 0;

			if (dataReg == 'o')
			{
				g_factorySetupSequence = STAGE_1;
			}
			else if ((g_factorySetupSequence == STAGE_1) && (dataReg == 'u'))
			{
				g_factorySetupSequence = STAGE_2;
			}
			else if ((g_factorySetupSequence == STAGE_2) && (dataReg == 'd'))
			{
				// Check if actively in Monitor mode
				if (g_sampleProcessing == ACTIVE_STATE)
				{
					// Don't allow access to the factory setup
					g_factorySetupSequence = SEQ_NOT_STARTED;
				}
				else // Not in Monitor mode
				{
					// Allow access to factory setup
					g_factorySetupSequence = ENTER_FACTORY_SETUP;
				}
			}
			else
			{
				msg.length = 1;
				msg.cmd = KEYPRESS_MENU_CMD;
				msg.data[0] = keyPress;
				sendInputMsg(&msg);
			}
		}
	}
#endif

#if 0
	// Check if the recieve data interrupt bit is set (if not, we have a problem)
	if (statusReg & MMC2114_SCI_SCISR1_RDRF)
	{
		if (statusReg & MMC2114_SCI_SCISR1_OR)
		{
			// If the overrun bit is set for an error.
			g_isrMessageBufferPtr->status = CMD_MSG_OVERFLOW_ERR;
		}
		else
		{
			// Write the received data into the buffer (clears the interrupt as well)
			*(g_isrMessageBufferPtr->writePtr) = dataReg;

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
	}
#endif
}
//#pragma interrupt off
#endif

// ============================================================================
// eic_keypad_irq
// ============================================================================
__attribute__((__interrupt__))
void eic_keypad_irq(void)
{
	// Print test for verification of operation
	//debugRaw("^");

#if 1
	if (g_kpadProcessingFlag == DEACTIVATED)
	{
		raiseSystemEventFlag(KEYPAD_EVENT);

		// Found a new key, reset last stored key
		g_kpadLastKeyPressed = 0;
	}
#endif

#if 0
	uint8 keyScan;
	keyScan = read_mcp23018(IO_ADDRESS_KPD, INTFB);
	{
		debug("Keypad IRQ: Interrupt Flags: %x\n", keyScan);
	}

	keyScan = read_mcp23018(IO_ADDRESS_KPD, GPIOB);
	{
		debug("Kaypad IRQ: Key Pressed: %x\n", keyScan);
	}
#endif

	read_mcp23018(IO_ADDRESS_KPD, INTFB);
	read_mcp23018(IO_ADDRESS_KPD, GPIOB);

	// clear the interrupt flag in the processor
	AVR32_EIC.ICR.int5 = 1;
}

// ============================================================================
// eic_keypad_irq
// ============================================================================
__attribute__((__interrupt__))
void eic_system_irq(void)
{
	static uint8 onKeyCount = 0;
	static uint8 powerOffAttempted = NO;
	uint8 keyFlag;
	uint8 keyScan;

	// Print test for verification of operation
	//debugRaw("&");

#if 0
	if (g_kpadProcessingFlag == DEACTIVATED)
	{
		raiseSystemEventFlag(KEYPAD_EVENT);

		// Found a new key, reset last stored key
		g_kpadLastKeyPressed = 0;
	}
#endif

#if 0
	keyFlag = read_mcp23018(IO_ADDRESS_KPD, INTFA);
	keyScan = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	debug("System IRQ: Interrupt Flags: %x\n", keyFlag);
	debug("System IRQ: Key Pressed: %x\n", keyScan);
#endif

#if 1
	keyFlag = read_mcp23018(IO_ADDRESS_KPD, INTFA);
	keyScan = read_mcp23018(IO_ADDRESS_KPD, GPIOA);

	// Check if the On key was pressed
	if ((keyFlag & keyScan) == ON_KEY)
	{
		if (g_factorySetupSequence != PROCESS_FACTORY_SETUP)
		{
			//debug("Factory Setup: Stage 1\n");
			g_factorySetupSequence = STAGE_1;
		}

		if (powerOffAttempted == YES)
		{ 
			onKeyCount++;
		}

#if 0 // Test - seemingly doesn't want to work, only shows 1 key being pressed
		// Check if the Off key is also being pressed - silent ability to power off the unit in the case of a monitor failure
		if (keyScan & OFF_KEY)
		{
			// Jumping off the ledge.. Buh bye! No returning from this
			powerControl(POWER_OFF, ON);
		}
#endif
	}

	if (keyScan & OFF_KEY)
	{
		if ((powerOffAttempted == YES) && (onKeyCount == 3))
		{
			// Jumping off the ledge.. Buh bye! No returning from this
			//debugRaw("\n--> BOOM <--");
			powerControl(POWER_OFF, ON);
		}

		powerOffAttempted = YES;
		onKeyCount = 0;
	}
#endif

	read_mcp23018(IO_ADDRESS_KPD, INTFA);
	read_mcp23018(IO_ADDRESS_KPD, GPIOA);

	// clear the interrupt flag in the processor
	AVR32_EIC.ICR.int4 = 1;
}

// ============================================================================
// usart_1_irq
// ============================================================================
#include "usart.h"
extern BOOLEAN processCraftCmd;
extern uint8 craft_g_input_buffer[];
__attribute__((__interrupt__))
void usart_1_rs232_irq(void)
{
	// Test print to verify the interrupt is running
	//debugRaw("`");

	uint32 usart_1_status;
	uint8 recieveData;

#if 1	
	usart_1_status = AVR32_USART1.csr;
	recieveData = AVR32_USART1.rhr;

	if (usart_1_status & AVR32_USART_CSR_RXRDY_MASK)
	{
#if 0 // Craft buffer fill
static uint8 craftBufferCount = 0;

		if (processCraftCmd == NO)
		{
			switch (recieveData)
			{
				case '\r':
				case '\n':
					g_input_buffer[craftBufferCount] = recieveData;
					craftBufferCount = 0;
					processCraftCmd = YES;
					break;

				default:
					g_input_buffer[craftBufferCount++] = recieveData;
					break;
			}
		}
#endif
		// Write the received data into the buffer
		*(g_isrMessageBufferPtr->writePtr) = recieveData;

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
		g_isrMessageBufferPtr->status = CMD_MSG_OVERFLOW_ERR;
	}		
	//else // USART_RX_EMPTY
#endif

#if 0 // Raw
	// Check if USART_RX_ERROR
	if (AVR32_USART1.csr & (AVR32_USART_CSR_OVRE_MASK | AVR32_USART_CSR_FRAME_MASK | AVR32_USART_CSR_PARE_MASK))
		g_isrMessageBufferPtr->status = CMD_MSG_OVERFLOW_ERR;

	// Check if we received a char
	if ((AVR32_USART1.csr & AVR32_USART_CSR_RXRDY_MASK) != 0)
	{
		*(g_isrMessageBufferPtr->writePtr) = ((AVR32_USART1.rhr & AVR32_USART_RHR_RXCHR_MASK) >> AVR32_USART_RHR_RXCHR_OFFSET);
	}
	//else USART_RX_EMPTY;
#endif

#if 0
	{
		auto INPUT_MSG_STRUCT msg;
		auto uint8 keyPress = 0;

		switch(dataReg)
		{
			case 'u' : keyPress = KEY_UPARROW; break;
			case 'd' : keyPress = KEY_DOWNARROW; break;
			case 'e' : keyPress = KEY_ENTER; break;
			case '-' : keyPress = KEY_MINUS; break;
			case '+' : keyPress = KEY_PLUS; break;
			case 'h' : keyPress = KEY_HELP; break;
			case 'b' : keyPress = KEY_BACKLIGHT; break;
			case 'x' : keyPress = KEY_ESCAPE; break;
		}

		if (messageBoxActiveFlag == YES)
		{
			messageBoxKeyInput = keyPress;
		}
		else
		{
			messageBoxKeyInput = 0;

			if (dataReg == 'o')
			{
				g_factorySetupSequence = STAGE_1;
			}
			else if ((g_factorySetupSequence == STAGE_1) && (dataReg == 'u'))
			{
				g_factorySetupSequence = STAGE_2;
			}
			else if ((g_factorySetupSequence == STAGE_2) && (dataReg == 'd'))
			{
				// Check if actively in Monitor mode
				if (g_sampleProcessing == ACTIVE_STATE)
				{
					// Don't allow access to the factory setup
					g_factorySetupSequence = SEQ_NOT_STARTED;
				}
				else // Not in Monitor mode
				{
					// Allow access to factory setup
					g_factorySetupSequence = ENTER_FACTORY_SETUP;
				}
			}
			else
			{
				msg.length = 1;
				msg.cmd = KEYPRESS_MENU_CMD;
				msg.data[0] = keyPress;
				sendInputMsg(&msg);
			}
		}
	}
#endif
}

// ============================================================================
// soft_timer_tick_irq
// ============================================================================
__attribute__((__interrupt__))
void soft_timer_tick_irq(void)
{
	// Test print to verify the interrupt is running
	//debugRaw("`");

	// Increment the lifetime soft timer tick count
	g_rtcSoftTimerTickCount++;

	// Every tick raise the flag to check soft timers
	raiseTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

	// Every 8 ticks (4 secs) trigger the cyclic event flag
	if (++g_cyclicEventDelay == 8)
	{
		g_cyclicEventDelay = 0;
		raiseSystemEventFlag(CYCLIC_EVENT);

#if 1 // Test
		g_sampleCountHold = g_sampleCount;
		g_sampleCount = 0;
#endif
	}

	// Every 60 ticks (30 secs) get the rtc time.
	if (++g_rtcCurrentTickCount >= 60)
	{
		raiseSystemEventFlag(UPDATE_TIME_EVENT);
	}

	// clear the interrupt flag
	rtc_clear_interrupt(&AVR32_RTC);
}

// ============================================================================
// Setup_8100_EIC_Keypad_ISR
// ============================================================================
void Setup_8100_EIC_Keypad_ISR(void)
{
	// External Interrupt Controller setup
	AVR32_EIC.IER.int5 = 1;
	AVR32_EIC.MODE.int5 = 0;
	AVR32_EIC.EDGE.int5 = 0;
	AVR32_EIC.LEVEL.int5 = 0;
	AVR32_EIC.FILTER.int5 = 0;
	AVR32_EIC.ASYNC.int5 = 1;
	AVR32_EIC.EN.int5 = 1;

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&eic_keypad_irq, AVR32_EIC_IRQ_5, 0);

	// Enable the interrupt
	rtc_enable_interrupt(&AVR32_RTC);

#if 0
	// Test for int enable
	if(AVR32_EIC.IMR.int5 == 0x01) debug("\nKeypad Interrupt Enabled\n");
	else debug("\nKeypad Interrupt Not Enabled\n");
#endif
}

// ============================================================================
// Setup_8100_EIC_System_ISR
// ============================================================================
void Setup_8100_EIC_System_ISR(void)
{
	// External Interrupt Controller setup
	AVR32_EIC.IER.int4 = 1;
	AVR32_EIC.MODE.int4 = 0;
	AVR32_EIC.EDGE.int4 = 0;
	AVR32_EIC.LEVEL.int4 = 0;
	AVR32_EIC.FILTER.int4 = 0;
	AVR32_EIC.ASYNC.int4 = 1;
	AVR32_EIC.EN.int4 = 1;

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&eic_system_irq, AVR32_EIC_IRQ_4, 0);

	// Enable the interrupt
	rtc_enable_interrupt(&AVR32_RTC);

#if 0 
	// Test for int enable
	if(AVR32_EIC.IMR.int4 == 0x01) debug("\nSystem Interrupt Enabled\n");
	else debug("\nSystem Interrupt Not Enabled\n");
#endif
}

// ============================================================================
// Setup_8100_Usart_ISR
// ============================================================================
void Setup_8100_Usart_RS232_ISR(void)
{
	INTC_register_interrupt(&usart_1_rs232_irq, AVR32_USART1_IRQ, 1);

	// Enable Receive Ready, Overrun, Parity and Framing error interrupts
	AVR32_USART1.ier = (AVR32_USART_IER_RXRDY_MASK | AVR32_USART_IER_OVRE_MASK |
						AVR32_USART_IER_PARE_MASK | AVR32_USART_IER_FRAME_MASK);
}

// ============================================================================
// Setup_8100_Soft_Timer_Tick_ISR
// ============================================================================
void Setup_8100_Soft_Timer_Tick_ISR(void)
{
	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&soft_timer_tick_irq, AVR32_RTC_IRQ, 0);
}

// ============================================================================
// Setup_8100_TC_Clock_ISR
// ============================================================================
__attribute__((__interrupt__))
void tc_sample_irq(void);
void tc_typematic_irq(void);

void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM channel)
{
	volatile avr32_tc_t *tc = &AVR32_TC;

	// Options for waveform generation.
	tc_waveform_opt_t WAVEFORM_OPT =
	{
		.channel  = channel,						   // Channel selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,                // RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOA: toggle.
		.acpa     = TC_EVT_EFFECT_NOOP,                // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
		.enetrg   = FALSE,                             // External event trigger enable.
		.eevt     = 0,                                 // External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,                    // External event edge selection.
		.cpcdis   = FALSE,                             // Counter disable when RC compare.
		.cpcstop  = FALSE,                             // Counter clock stopped with RC compare.
		.burst    = FALSE,                             // Burst signal selection.
		.clki     = FALSE,                             // Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC2                // Internal source clock 2 - connected to PBA/2
	};

	// Options for interrupt
	tc_interrupt_t TC_INTERRUPT =
	{
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1,
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};

	switch (channel)
	{
		case TC_SAMPLE_TIMER_CHANNEL:
			// Register the RTC interrupt handler to the interrupt controller.
			INTC_register_interrupt(&tc_sample_irq, AVR32_TC_IRQ0, 3);
			break;
			
		case TC_CALIBRATION_TIMER_CHANNEL:
			// Register the RTC interrupt handler to the interrupt controller.
			INTC_register_interrupt(&tc_sample_irq, AVR32_TC_IRQ1, 3);
			break;
			
		case TC_TYPEMATIC_TIMER_CHANNEL:
			// Register the RTC interrupt handler to the interrupt controller.
			INTC_register_interrupt(&tc_typematic_irq, AVR32_TC_IRQ2, 0);
			break;
	}

	// Initialize the timer/counter.
	tc_init_waveform(tc, &WAVEFORM_OPT);         // Initialize the timer/counter waveform.

	// Set the compare triggers.
	// Remember TC counter is 16-bits, so counting second is not possible.
	// We configure it to count ms.
	// We want: (1/(FOSC0/4)) * RC = 1000 Hz => RC = (FOSC0/4) / 1000 = 3000 to get an interrupt every 1ms
	//tc_write_rc(tc, TC_CHANNEL_0, (FOSC0/2)/1000);  // Set RC value.
	//tc_write_rc(tc, channel, (FOSC0 / (sampleRate * 2)));
	tc_write_rc(tc, channel, (32900000 / sampleRate));
	
	tc_configure_interrupts(tc, channel, &TC_INTERRUPT);
}

// ============================================================================
// Start_Data_Clock
// ============================================================================
void Start_Data_Clock(TC_CHANNEL_NUM channel)
{
	//volatile avr32_tc_t *tc = &AVR32_TC;

	// Start the timer/counter.
	tc_start(&AVR32_TC, channel);                    // And start the timer/counter.

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

// ============================================================================
// Stop_Data_Clock
// ============================================================================
void Stop_Data_Clock(TC_CHANNEL_NUM channel)
{
	//volatile avr32_tc_t *tc = &AVR32_TC;

	// Stop the timer/counter.
	tc_stop(&AVR32_TC, channel);                    // And start the timer/counter.

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

// ============================================================================
// tc_typematic_irq
// ============================================================================
__attribute__((__interrupt__))
void tc_typematic_irq(void)
{
	// Increment the ms seconds counter
	g_keypadTimerTicks++;

	// clear the interrupt flag
	DUMMY_READ(AVR32_TC.channel[TC_TYPEMATIC_TIMER_CHANNEL].sr);
}

// ============================================================================
// tc_sample_irq
// ============================================================================
#if 1
extern uint16* g_startOfPreTrigBuff;
extern uint16* g_tailOfPreTrigBuff;
extern uint16* g_endOfPreTrigBuff;
extern OFFSET_DATA_STRUCT g_channelOffset;
#define CONSECUTIVE_TRIGGERS_THRESHOLD 2
#define CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD 3
#define PENDING	2 // Anything above 1
#endif

__attribute__((__interrupt__))
void tc_sample_irq(void)
{
	// Don't even bother with creating and clearing stack variables every time
	static uint16 s_R_channelReading, s_V_channelReading, s_T_channelReading, s_A_channelReading;
	static uint16 s_temperatureReading, s_channelConfigReadBack;
	static uint32 s_sampleCount = 0;
	static uint32 s_calSampleCount = 0;
	static uint32 s_pendingCalCount = 0;
	static uint32 s_pretriggerCount = 0;

	//static uint32 fakeDataIncrement = 0;

	static uint16 s_consecSeismicTriggerCount = 0;
	static uint16 s_consecAirTriggerCount = 0;
	static uint8 s_pretriggerFull = NO;
	static uint8 s_seismicTriggerSample = NO;
	static uint8 s_airTriggerSample = NO;
	static uint8 s_recording = NO;
	static uint8 s_calPulse = NO;
	static uint8 s_consecEventsWithoutCal = 0;
	static uint8 s_channelSyncError = NO;
	static uint8 s_channelReadsToSync = 0;

	//___________________________________________________________________________________________
	//___Sample Output with return config words for reference
	// R: 7f26 (e0d0) | V: 7f10 (e2d0) | T: 7f15 (e4d0) | A: 7f13 (e6d0) | Temp:  dbe (b6d0)
	
#if 1 // Test
	//___________________________________________________________________________________________
	//___AD raw data read all channels
    // Chan 0 - R
	spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
    spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_R_channelReading);
    //spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
    spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	//if(s_channelConfigReadBack != 0xe0d0) { s_channelSyncError = YES; }
	//if(s_channelConfigReadBack != 0xe0d0) { g_channelSyncError = YES; }	

    // Chan 1 - T
    spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
    spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_T_channelReading);
    //spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
    spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	//if(s_channelConfigReadBack != 0xe2d0) { s_channelSyncError = YES; }
	//if(s_channelConfigReadBack != 0xe2d0) { g_channelSyncError = YES; }

    // Chan 2 - V
    spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
    spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_V_channelReading);
    //spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
    spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	//if(s_channelConfigReadBack != 0xe4d0) { s_channelSyncError = YES; }
	//if(s_channelConfigReadBack != 0xe4d0) { g_channelSyncError = YES; }

    // Chan 3 - A
    spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
    spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_A_channelReading);
    //spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
    spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	//if(s_channelConfigReadBack != 0xe6d0) { s_channelSyncError = YES; }
	//if(s_channelConfigReadBack != 0xe6d0) { g_channelSyncError = YES; }

    // Temp
    //spi_selectChip(&AVR32_SPI0, AD_SPI_NPCS);
    //spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_temperatureReading);
    //spi_write(&AVR32_SPI0, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
    //spi_unselectChip(&AVR32_SPI0, AD_SPI_NPCS);
	//if(s_channelConfigReadBack != 0xb6d0) { s_channelSyncError = YES; }
	//if(s_channelConfigReadBack != 0xb6d0) { g_channelSyncError = YES; }
#endif // Test

	//___________________________________________________________________________________________
	//___Test timing (throw away)
	// clear the interrupt flag
	g_sampleCount++;
	//tc_read_sr(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
	//tc_read_sr(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
	//DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
	//DUMMY_READ(AVR32_TC.channel[TC_CALIBRATION_TIMER_CHANNEL].sr);
	//return;

	//___________________________________________________________________________________________
	//___Check for channel sync error
	if (s_channelSyncError == YES)
	{
		debugErr("AD Channel Sync Error!\n");
		
		// Attempt channel recovery
		spi_selectChip(AD_SPI, AD_SPI_NPCS);
		spi_write(AD_SPI, 0x0000); spi_read(AD_SPI, &s_temperatureReading);
		spi_write(AD_SPI, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
		spi_unselectChip(AD_SPI, AD_SPI_NPCS);
		
		switch (s_channelConfigReadBack)
		{
			case 0xe0d0: s_channelReadsToSync = 4; break; // R Chan
			case 0xe2d0: s_channelReadsToSync = 3; break; // T Chan
			case 0xe4d0: s_channelReadsToSync = 2; break; // V Chan
			case 0xe6d0: s_channelReadsToSync = 1; break; // A Chan
			case 0xb6d0: s_channelReadsToSync = 0; break; // Temp Chan

			default: 
				s_channelReadsToSync = 0; // Houston, we have a problem... all channels read and unable to match
				debugErr("Error: ISR Processing AD --> Unable to Sync channels, data collection broken!\n");
				break;
		}
		
		while (s_channelReadsToSync--)
		{
			// Dummy reads to realign channel processing
			spi_selectChip(AD_SPI, AD_SPI_NPCS); 
			spi_write(AD_SPI, 0x0000); spi_read(AD_SPI, &s_temperatureReading);
			spi_write(AD_SPI, 0x0000); spi_read(AD_SPI, &s_channelConfigReadBack);
			spi_unselectChip(AD_SPI, AD_SPI_NPCS);
		}
	}
	//___________________________________________________________________________________________
	//___AD data read successfully, Normal operation
	else // Analog data ready to be processed
	{
		// Fit into legacy design, throw away 4 bits
		s_R_channelReading = s_R_channelReading / 16;
		s_V_channelReading = s_V_channelReading / 16;
		s_T_channelReading = s_T_channelReading / 16;
		s_A_channelReading = s_A_channelReading / 16;
	
		// Apply channel offset
		s_R_channelReading -= g_channelOffset.r_12bit;
		s_V_channelReading -= g_channelOffset.v_12bit;
		s_T_channelReading -= g_channelOffset.t_12bit;
		s_A_channelReading -= g_channelOffset.a_12bit;

#if 1 // Normal
		// Store the data into the pretrigger buffer
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->r = s_R_channelReading;
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->v = s_V_channelReading;
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->t = s_T_channelReading;
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->a = s_A_channelReading;
#else // Test
		// Store the fakedata into the pretrigger buffer
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->r = (((fakeDataIncrement + 1) & 0x0FFF) | 0x0000);
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->v = (((fakeDataIncrement + 2) & 0x0FFF) | 0x0000);
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->t = (((fakeDataIncrement + 3) & 0x0FFF) | 0x0000);
		((SAMPLE_DATA_STRUCT*)g_tailOfPreTrigBuff)->a = (((fakeDataIncrement + 0) & 0x0FFF) | 0x0000);
		fakeDataIncrement++;
#endif

		//___________________________________________________________________________________________
		//___Check if the system flag is idle
		if (g_sampleProcessing == IDLE_STATE)
		{
			// Idle and not processing, might as well adjust the channel offsets
			// fix_ns8100

			// Advance to the next pretrigger sample in the buffer
			g_tailOfPreTrigBuff += g_sensorInfoPtr->numOfChannels;

			// Check if the end of the PreTrigger buffer has been reached
			if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;
		}
		//___________________________________________________________________________________________
		//___Processing Data in real time for Alarms, Triggers and Cal pulse
		else // (g_sampleProcessing == ACTIVE_STATE)
		{
			//_____________________________________________________________________________________
			//___Check if the pretrigger is not full, which is necessary for an event
			if (s_pretriggerFull == NO)
			{
				s_pretriggerCount++;

				// Check if the pretrigger count has accumulated to 1/4 of a second
				if (s_pretriggerCount >= (g_triggerRecord.trec.sample_rate / 4)) 
				{ 
					s_pretriggerFull = YES;
					s_pretriggerCount = 0;
				}					
					
				// Advance to the next pretrigger sample in the buffer
				g_tailOfPreTrigBuff += g_sensorInfoPtr->numOfChannels;

				// Check if the end of the PreTrigger buffer has been reached
				if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;
			}
			//___________________________________________________________________________________________
			//___Pretrigger is full and ready
			else // (s_pretriggerFull == YES)
			{
				//___________________________________________________________________________________________
				//___If handling a Manual Cal
				if (g_manualCalFlag == TRUE)
				{
					// Wait 5 samples to start cal signaling (~5 ms)
					if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_FIRST_TRANSITION_HIGH) { adSetCalSignalHigh(); }	// (~10 ms)
					if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_SECOND_TRANSITION_LOW) { adSetCalSignalLow(); }	// (~20 ms)
					if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_THIRD_TRANSITION_HIGH) { adSetCalSignalHigh(); }	// (~10 ms)
					if (g_manualCalSampleCount == CAL_SAMPLE_COUNT_FOURTH_TRANSITION_OFF) { adSetCalSignalOff(); }	// (~55 ms)

					// Mark the start of the Manual Cal pulse
					if (g_manualCalSampleCount == MAX_CAL_SAMPLES)
					{
#if 1 // Normal
						// Signal the start of the Cal pulse
						*(g_tailOfPreTrigBuff + 0) |= CAL_START;
						*(g_tailOfPreTrigBuff + 1) |= CAL_START;
						*(g_tailOfPreTrigBuff + 2) |= CAL_START;
						*(g_tailOfPreTrigBuff + 3) |= CAL_START;
#else // Test
						// Signal the start of the Cal pulse
						*(g_tailOfPreTrigBuff + 0) &= 0x0FFF; *(g_tailOfPreTrigBuff + 0) |= CAL_START;
						*(g_tailOfPreTrigBuff + 1) &= 0x0FFF; *(g_tailOfPreTrigBuff + 1) |= CAL_START;
						*(g_tailOfPreTrigBuff + 2) &= 0x0FFF; *(g_tailOfPreTrigBuff + 2) |= CAL_START;
						*(g_tailOfPreTrigBuff + 3) &= 0x0FFF; *(g_tailOfPreTrigBuff + 3) |= CAL_START;
#endif
					}

					if (g_manualCalSampleCount)
					{
						g_manualCalSampleCount--;

						// Check if done with the Manual Cal pulse
						if (g_manualCalSampleCount == 0)
						{
#if 1 // Normal
							// Mark the end of the Cal pulse
							*(g_tailOfPreTrigBuff + 0) |= CAL_END;
							*(g_tailOfPreTrigBuff + 1) |= CAL_END;
							*(g_tailOfPreTrigBuff + 2) |= CAL_END;
							*(g_tailOfPreTrigBuff + 3) |= CAL_END;
#else // Test
							// Signal the start of the Cal pulse
							*(g_tailOfPreTrigBuff + 0) &= 0x0FFF; *(g_tailOfPreTrigBuff + 0) |= CAL_END;
							*(g_tailOfPreTrigBuff + 1) &= 0x0FFF; *(g_tailOfPreTrigBuff + 1) |= CAL_END;
							*(g_tailOfPreTrigBuff + 2) &= 0x0FFF; *(g_tailOfPreTrigBuff + 2) |= CAL_END;
							*(g_tailOfPreTrigBuff + 3) &= 0x0FFF; *(g_tailOfPreTrigBuff + 3) |= CAL_END;
#endif
						}
					}

					// Process the samples
					ProcessManuelCalPulse();
				}
				//___________________________________________________________________________________________
				//___For all other modes check for Alarms and mode specific triggers and cal
				else // (g_manualCalFlag == FALSE)
				{
					//___________________________________________________________________________________________
					//___Normalize to zero (positive) for comparison
					if (s_R_channelReading < 0x800) { s_R_channelReading = 0x800 - s_R_channelReading; } else { s_R_channelReading -= 0x800; }
					if (s_V_channelReading < 0x800) { s_V_channelReading = 0x800 - s_V_channelReading; } else { s_V_channelReading -= 0x800; }
					if (s_T_channelReading < 0x800) { s_T_channelReading = 0x800 - s_T_channelReading; } else { s_T_channelReading -= 0x800; }
					if (s_A_channelReading < 0x800) { s_A_channelReading = 0x800 - s_A_channelReading; } else { s_A_channelReading -= 0x800; }
	
					//___________________________________________________________________________________________
					//___Start of Alarm section
					// Collecting data for any mode (other than calibration), signal alarm if active
					if (g_helpRecord.alarm_one_mode != ALARM_MODE_OFF)
					{
						// Check if seismic is enabled for Alarm 1
						if (g_helpRecord.alarm_one_mode & ALARM_MODE_SEISMIC)
						{
							if (s_R_channelReading > (g_helpRecord.alarm_one_seismic_lvl * 16)) { raiseSystemEventFlag(WARNING1_EVENT); }
							else if (s_V_channelReading > (g_helpRecord.alarm_one_seismic_lvl * 16)) { raiseSystemEventFlag(WARNING1_EVENT); }
							else if (s_T_channelReading > (g_helpRecord.alarm_one_seismic_lvl * 16)) { raiseSystemEventFlag(WARNING1_EVENT); }
						}

						// Check if air is enabled for Alarm 1
						if (g_helpRecord.alarm_one_mode & ALARM_MODE_AIR)
							if (s_A_channelReading > (g_helpRecord.alarm_one_air_lvl * 16)) { raiseSystemEventFlag(WARNING1_EVENT); }
					}
						
					if (g_helpRecord.alarm_two_mode != ALARM_MODE_OFF)
					{
						// Check if seismic is enabled for Alarm 2
						if (g_helpRecord.alarm_two_mode & ALARM_MODE_SEISMIC)
						{
							if (s_R_channelReading > (g_helpRecord.alarm_two_seismic_lvl * 16)) { raiseSystemEventFlag(WARNING2_EVENT); }
							else if (s_V_channelReading > (g_helpRecord.alarm_two_seismic_lvl * 16)) { raiseSystemEventFlag(WARNING2_EVENT); }
							else if (s_T_channelReading > (g_helpRecord.alarm_two_seismic_lvl * 16)) { raiseSystemEventFlag(WARNING2_EVENT); }
						}

						// Check if air is enabled for Alarm 2
						if (g_helpRecord.alarm_two_mode & ALARM_MODE_AIR)
							if (s_A_channelReading > (g_helpRecord.alarm_two_air_lvl * 16)) { raiseSystemEventFlag(WARNING2_EVENT); }
					}				

					//_____________________________________________________________________________________
					//___If processing Waveform or Combo mode, look for triggers and manage the cal pulse
					if ((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
					{
						//_____________________________________________________________________________________
						//___Check if not recording _and_ no cal pulse, or if pending cal and all below consec threshold
						if ((((s_recording == NO) && (s_calPulse == NO)) || (s_pendingCalCount)) && 
							(s_consecEventsWithoutCal < CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD))
						{
							//_____________________________________________________________________________________
							//___Check for triggers
							if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
							{
								if (s_R_channelReading > (g_triggerRecord.trec.seismicTriggerLevel * 16)) { s_seismicTriggerSample = YES; }
								else if (s_V_channelReading > (g_triggerRecord.trec.seismicTriggerLevel * 16)) { s_seismicTriggerSample = YES; }
								else if (s_T_channelReading > (g_triggerRecord.trec.seismicTriggerLevel * 16)) { s_seismicTriggerSample = YES; }
					
								if (s_seismicTriggerSample == YES) { s_consecSeismicTriggerCount++; s_seismicTriggerSample = NO; }
								else {s_consecSeismicTriggerCount = 0; }
							}

							if (g_triggerRecord.trec.soundTriggerLevel != NO_TRIGGER_CHAR)
							{
								if (s_A_channelReading > (g_triggerRecord.trec.soundTriggerLevel * 16)) { s_airTriggerSample = YES; }
						
								if (s_airTriggerSample == YES) { s_consecAirTriggerCount++; s_airTriggerSample = NO; }
								else { s_consecAirTriggerCount = 0; }
							}				

							//___________________________________________________________________________________________
							//___Check if either a seismic or acoustic trigger threshold condition was achieved
#if 1 // Normal
							if ((s_consecSeismicTriggerCount == CONSECUTIVE_TRIGGERS_THRESHOLD) || 
								(s_consecAirTriggerCount == CONSECUTIVE_TRIGGERS_THRESHOLD))
							{
#else // Test
							if (g_testTrigger)
							{
								g_testTrigger = NO;
#endif
								
#if 1 // Normal
								// Signal the start of a Trigger
								*(g_tailOfPreTrigBuff + 0) |= TRIG_ONE;
								*(g_tailOfPreTrigBuff + 1) |= TRIG_ONE;
								*(g_tailOfPreTrigBuff + 2) |= TRIG_ONE;
								*(g_tailOfPreTrigBuff + 3) |= TRIG_ONE;
#else // Test
								// Signal the start of a Trigger
								*(g_tailOfPreTrigBuff + 0) &= 0x0FFF; *(g_tailOfPreTrigBuff + 0) |= TRIG_ONE;
								*(g_tailOfPreTrigBuff + 1) &= 0x0FFF; *(g_tailOfPreTrigBuff + 1) |= TRIG_ONE;
								*(g_tailOfPreTrigBuff + 2) &= 0x0FFF; *(g_tailOfPreTrigBuff + 2) |= TRIG_ONE;
								*(g_tailOfPreTrigBuff + 3) &= 0x0FFF; *(g_tailOfPreTrigBuff + 3) |= TRIG_ONE;
#endif
								//debug("--> Trigger Found! %x %x %x %x\n", s_R_channelReading, s_V_channelReading, s_T_channelReading, s_A_channelReading);
								//usart_write_char(&AVR32_USART1, '$');
					
								s_consecSeismicTriggerCount = 0;
								s_consecAirTriggerCount = 0;
								s_recording = YES;
								s_calPulse = PENDING;
								s_pendingCalCount = 0;
								s_consecEventsWithoutCal++;

								// Already handled trigger sample, so now record 1 sample less than the total
								s_sampleCount = (g_triggerRecord.trec.record_time * g_triggerRecord.trec.sample_rate - 1);
							}
							//___________________________________________________________________________________________
							//___Check if pending for a cal pulse since no trigger was found
							else if (s_pendingCalCount)
							{
								s_pendingCalCount--;
						
								// Check if done pending for a Cal pulse
								if (s_pendingCalCount == 0)
								{
									// Time to handle the Cal pulse
									s_calPulse = YES;
									s_calSampleCount = START_CAL_SIGNAL;
					
#if 0 // Moved to Cal pulse for easier protection logic
									// Check if on high sensitivity and if so set to low sensitivity for Cal pulse
									if (g_triggerRecord.srec.sensitivity == HIGH) { SetSeismicGainSelect(SEISMIC_GAIN_LOW); }

									// Swap to alternate timer/counter for default 1024 sample rate for Cal
									DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
									tc_stop(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
									tc_start(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
#endif
								}
							}
						}
						//___________________________________________________________________________________________
						//___Check if handling event samples
						else if ((s_recording == YES) && (s_sampleCount))
						{
							s_sampleCount--;
					
							// Check if all the event samples have been handled
							if (s_sampleCount == 0)
							{
								//debug("--> Recording done!\n");
								//usart_write_char(&AVR32_USART1, '%');

								s_recording = NO;

								// Check if maximum consecutive events have not been captured, therefore pend Cal pulse
								if (s_consecEventsWithoutCal < CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD)
								{
									// Setup delay for Cal pulse (1/4 sample rate)
									s_pendingCalCount = g_triggerRecord.trec.sample_rate / 4;
								}
								else // Max number of events have been captured, force a Cal pulse
								{
									// Time to handle the Cal pulse
									s_calPulse = YES;
									s_calSampleCount = START_CAL_SIGNAL;
					
#if 0 // Moved to Cal pulse for easier protection logic
									// Check if on high sensitivity and if so set to low sensitivity for Cal pulse
									if (g_triggerRecord.srec.sensitivity == HIGH) { SetSeismicGainSelect(SEISMIC_GAIN_LOW); }

									// Swap to alternate timer/counter for default 1024 sample rate for Cal
									DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
									tc_stop(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
									tc_start(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
#endif
								}
							}
						}
						//___________________________________________________________________________________________
						//___Check if handling cal pulse samples
						else if ((s_calPulse == YES) && (s_calSampleCount))
						{
							if (g_spi1AccessLock != EVENT_LOCK)
							{
								if (g_spi1AccessLock == AVAILABLE) { g_spi1AccessLock = CAL_PULSE_LOCK;	}
							
								// Check for the start of the Cal pulse and set low sensitivity and swap clock source for 1024 sample rate
								if (s_calSampleCount == START_CAL_SIGNAL)
								{
									// Check if on high sensitivity and if so set to low sensitivity for Cal pulse
									if (g_triggerRecord.srec.sensitivity == HIGH) { SetSeismicGainSelect(SEISMIC_GAIN_LOW); }

									// Swap to alternate timer/counter for default 1024 sample rate for Cal
									DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
									tc_stop(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
									tc_start(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);

									s_calSampleCount = MAX_CAL_SAMPLES;
								}
								else // Cal pulse started
								{
									// Wait 5 samples to start cal signaling (~5 ms)
									if (s_calSampleCount == CAL_SAMPLE_COUNT_FIRST_TRANSITION_HIGH) { adSetCalSignalHigh();	}	// (~10 ms)
									if (s_calSampleCount == CAL_SAMPLE_COUNT_SECOND_TRANSITION_LOW) { adSetCalSignalLow(); }	// (~20 ms)
									if (s_calSampleCount == CAL_SAMPLE_COUNT_THIRD_TRANSITION_HIGH) { adSetCalSignalHigh(); }	// (~10 ms)
									if (s_calSampleCount == CAL_SAMPLE_COUNT_FOURTH_TRANSITION_OFF) { adSetCalSignalOff(); }	// (~55 ms)

									// Signal the start of a Cal
									if (s_calSampleCount == MAX_CAL_SAMPLES)
									{
#if 1 // Normal
										*(g_tailOfPreTrigBuff + 0) |= CAL_START;
										*(g_tailOfPreTrigBuff + 1) |= CAL_START;
										*(g_tailOfPreTrigBuff + 2) |= CAL_START;
										*(g_tailOfPreTrigBuff + 3) |= CAL_START;
#else // Test
										*(g_tailOfPreTrigBuff + 0) &= 0x0FFF; *(g_tailOfPreTrigBuff + 0) |= CAL_START;
										*(g_tailOfPreTrigBuff + 1) &= 0x0FFF; *(g_tailOfPreTrigBuff + 1) |= CAL_START;
										*(g_tailOfPreTrigBuff + 2) &= 0x0FFF; *(g_tailOfPreTrigBuff + 2) |= CAL_START;
										*(g_tailOfPreTrigBuff + 3) &= 0x0FFF; *(g_tailOfPreTrigBuff + 3) |= CAL_START;
#endif
									}

									s_calSampleCount--;

									if (s_calSampleCount == 0)
									{
										//debug("\n--> Cal done!\n");
										//usart_write_char(&AVR32_USART1, '&');
						
										// Reset all states and counters (that haven't already)
										s_calPulse = NO;
										s_consecSeismicTriggerCount = 0;
										s_consecAirTriggerCount = 0;
										s_consecEventsWithoutCal = 0;
					
										// Check if on high sensitivity and if so reset to high sensitivity after Cal pulse (done on low)
										if (g_triggerRecord.srec.sensitivity == HIGH) { SetSeismicGainSelect(SEISMIC_GAIN_HIGH); }

										// Swap back to original sampling rate
										DUMMY_READ(AVR32_TC.channel[TC_CALIBRATION_TIMER_CHANNEL].sr);
										tc_stop(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
										tc_start(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);

										// Invalidate the pretrigger until it's filled again
										s_pretriggerFull = NO;

										g_spi1AccessLock = AVAILABLE;
									}																
								}
							}
						}
						else
						{
							// Should _never_ get here
							debugErr("Error in ISR processing!\n");
						}
					} // End of processing waveform and combo trigger and cal pulse			

					//___________________________________________________________________________________________
					//___Process the data with the mode specific handler
					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						ProcessWaveformData();
					}
					else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
					{
						ProcessBargraphData();
					}
					else if (g_triggerRecord.op_mode == COMBO_MODE)
					{
						if (s_calPulse == NO)
						{
							ProcessComboData();
						}
						else // (s_calPulse == YES)
						{
							ProcessComboDataSkipBargraphDuringCal();
						}							
					}

				} // End of process all modes except for manual cal
			} // End of pretrigger full
		} // End of (g_sampleProcessing == ACTIVE_STATE)
	} // End of data processing

	// clear the interrupt flag
	DUMMY_READ(AVR32_TC.channel[TC_SAMPLE_TIMER_CHANNEL].sr);
	DUMMY_READ(AVR32_TC.channel[TC_CALIBRATION_TIMER_CHANNEL].sr);
}
