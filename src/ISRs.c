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
#include "Mmc2114_Registers.h"
#include "Mmc2114_InitVals.h"
#include "Ispi.h"
#include "Common.h"
#include "Rec.h"
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
#if SUPERGRAPH_UNIT
#define ESC_KEY_ROW			0x02
#define ESC_KEY_POSITION	0x20
#else // MINIGRAPH_UNIT
#define ESC_KEY_ROW			0x04
#define ESC_KEY_POSITION	0x01
#endif

#define TC_CHANNEL 0

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern SYS_EVENT_STRUCT SysEvents_flags;
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;
extern uint8* _SPIDR;
extern uint8* _SPISR;
extern uint16* tailOfPreTrigBuff;
extern ISPI_STATE_E  _ISPI_State;
extern uint16 manual_cal_flag;
extern volatile uint32 g_keypadTimerTicks;
extern volatile uint32 g_rtcCurrentTickCount;
extern uint32 g_rtcSoftTimerTickCount;
extern int8 g_kpadProcessingFlag;
extern uint8 g_kpadLastKeyPressed;
extern uint8 g_factorySetupSequence;
extern REC_HELP_MN_STRUCT help_rec;
extern REC_EVENT_MN_STRUCT trig_rec;
extern uint8 g_monitorEscapeCheck;
extern MN_TIMER_STRUCT mn_timer;
extern CMD_BUFFER_STRUCT* gp_ISRMessageBuffer;		// Craft port receive buffer.
extern MODEM_STATUS_STRUCT g_ModemStatus;			// Craft port status information flag.
extern void rtc_clear_interrupt(volatile avr32_rtc_t *rtc);
extern int rtc_init(volatile avr32_rtc_t *rtc, unsigned char osc_type, unsigned char psel);
extern void rtc_set_top_value(volatile avr32_rtc_t *rtc, unsigned long top);
extern void rtc_enable_interrupt(volatile avr32_rtc_t *rtc);
extern void rtc_enable(volatile avr32_rtc_t *rtc);
extern uint16 manualCalSampleCount;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
uint32 cyclicEventDelay = 0;
uint8 g_sampleProcessing = IDLE_STATE;
uint8 g_modemConnected = NO;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void isr_PowerOnKey(void);
void isr_PowerOffKey(void);
void isr_Lan(void);
void isr_Keypad(void);
void isr_UsbHost(void);
void isr_UsbDevice(void);
void isr_RTC(void);
void isr_Trig(void);
void isr_PIT1(void);
void isr_PIT2(void);
void isr_SCI1(void);
void isr_SCI2(void);
void isr_SPI_Transfer_Complete(void);
void isr_MSP430WakeupMsg(void);

// Test exception handlers
void isr_misaligned_access(void);
void isr_access_error(void);
void isr_divide_by_zero(void);
void isr_illegal_instruction(void);
void isr_privilege_violation(void);
void isr_trace_exception(void);
void isr_breakpoint_exception(void);
void isr_unrecoverable_error(void);

extern  void __misaligned_access(void);
extern  void __access_error(void);
extern  void __divide_by_zero(void);
extern  void __illegal_instruction(void);
extern  void __privilege_violation(void);
extern  void __trace_exception(void);
extern  void __breakpoint_exception(void);
extern  void __unrecoverable_error(void);

#if 0 // fix_ns8100

// =================
// Misaligned Access
// =================
void isr_misaligned_access(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '1';

	soft_usecWait(1 * SOFT_SECS);
#else
	__misaligned_access();
#endif
}

// ============
// Access Error
// ============
void isr_access_error(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '2';

	soft_usecWait(1 * SOFT_SECS);
#else
	__access_error();
#endif
}

// ==============
// Divide by Zero
// ==============
void isr_divide_by_zero(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '3';

	soft_usecWait(1 * SOFT_SECS);
#else
	__divide_by_zero();
#endif
}

// ===================
// Illegal Instruction
// ===================
void isr_illegal_instruction(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '4';

	soft_usecWait(1 * SOFT_SECS);
#else
	__illegal_instruction();
#endif
}

// ====================
// Priviledge Violation
// ====================
void isr_privilege_violation(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '5';

	soft_usecWait(1 * SOFT_SECS);
#else
	__privilege_violation();
#endif
}

// ===============
// Trace Exception
// ===============
void isr_trace_exception(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '6';

	soft_usecWait(1 * SOFT_SECS);
#else
	__trace_exception();
#endif
}

// ====================
// Breakpoint Exception
// ====================
void isr_breakpoint_exception(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '7';

	soft_usecWait(1 * SOFT_SECS);
#else
	__breakpoint_exception();
#endif
}

// ===================
// Unrecoverable Error
// ===================
void isr_unrecoverable_error(void)
{
#if TEST_EXCEPTION_HANDLING
    MMC2114_IMM *imm = mmc2114_get_immp();

	while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '8';

	soft_usecWait(1 * SOFT_SECS);
#else
	__unrecoverable_error();
#endif
}
#endif // big block

#if 0 // fix_ns8100
/*******************************************************************************
* Function: isr_PowerOnKey
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_PowerOnKey(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '!';

	if (g_factorySetupSequence != PROCESS_FACTORY_SETUP)
	{
		g_factorySetupSequence = STAGE_1;
	}

	// Clear the processor interrupt flag
	mmc_clear_EPF0_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_PowerOffKey
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_PowerOffKey(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '@';

	if (g_sampleProcessing != SAMPLING_STATE)
	{
		// Check if timer mode is enabled and power off enable has been turned off
		if ((help_rec.timer_mode == ENABLED) && (getPowerControlState(POWER_SHUTDOWN_ENABLE) == OFF))
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
* Function: isr_Lan
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_Lan(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '#';

	// Clear the processor interrupt flag
	mmc_clear_EPF2_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_Keypad
* Purpose:                                                                     *
*******************************************************************************/
//#pragma interrupt on
void isr_Keypad(void)
{
	//MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '$';

	if (g_kpadProcessingFlag == DEACTIVATED)
	{
		raiseSystemEventFlag(KEYPAD_EVENT);

		// Found a new key, reset last stored key
		g_kpadLastKeyPressed = 0;
	}

	// Clear the processor interrupt flag
	mmc_clear_EPF3_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_UsbHost
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_UsbHost(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '%';

	// Clear the processor interrupt flag
	mmc_clear_EPF4_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_UsbDevice
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_UsbDevice(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '^';

	// Clear the processor interrupt flag
	mmc_clear_EPF5_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_RTC
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_RTC(void)
{
	RTC_FLAGS_STRUCT rtcFlags;
	static uint32 samplingCounter = 0;

	//MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '&';

	// Reading the flags register clears all the flags
	rtcFlags.reg = RTC_FLAGS.reg;

	// Check which interrupt source has signaled the interrupt
	if (rtcFlags.bit.alarmIntFlag)
	{

	}

	if (rtcFlags.bit.periodicIntFlag)
	{
		if(g_sampleProcessing == SAMPLING_STATE)
		{
			GatherSampleData();

			if (manual_cal_flag)
			{
				ProcessManuelCalPulse();
			}
			else if (trig_rec.op_mode == WAVEFORM_MODE)
			{
				ProcessWaveformData();
			}
			else //if (trig_rec.op_mode == BARGRAPH_MODE)
			{
				ProcessBargraphData();
			}

			// Check if a half seconds worth of samples have occurred
			samplingCounter++;
			if ((samplingCounter % (trig_rec.trec.sample_rate >> 1)) == 0)
			{
				 samplingCounter = 0;

				// Increment the lifetime soft timer tick count
				g_rtcSoftTimerTickCount++;

				// Every tick raise the flag to check soft timers
				raiseTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

				// Every 8 ticks (4 secs) trigger the cyclic event flag
				if (++cyclicEventDelay >= 8)
				{
					cyclicEventDelay = 0;
					raiseSystemEventFlag(CYCLIC_EVENT);
				}

				// Every 60 ticks (30 secs) get the rtc time.
				if (++g_rtcCurrentTickCount >= 60)
				{
					raiseSystemEventFlag(UPDATE_TIME_EVENT);
				}
			}
		}
		else
		{
			// Reset the sampling counter if isn't not zero
			if (samplingCounter) samplingCounter = 0;

			// Increment the lifetime soft timer tick count
			g_rtcSoftTimerTickCount++;

			// Every tick raise the flag to check soft timers
			raiseTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

			// Every 8 ticks (4 secs) trigger the cyclic event flag
			if (++cyclicEventDelay >= 8)
			{
				cyclicEventDelay = 0;
				raiseSystemEventFlag(CYCLIC_EVENT);
			}

			// Every 60 ticks (30 secs) get the rtc time.
			if (++g_rtcCurrentTickCount >= 60)
			{
				raiseSystemEventFlag(UPDATE_TIME_EVENT);
			}
		}
	}

	if (rtcFlags.bit.powerFailIntFlag)
	{

	}

	// Clear the processor interrupt flag
	mmc_clear_EPF6_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_Trig
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_Trig(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '*';

	// Clear the processor interrupt flag
	mmc_clear_EPF7_int;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_PIT1
* Purpose: Provides an normal interrupt service routine for PIT interrupts
*******************************************************************************/
//#pragma interrupt on	    				   /* PIT1 interrupt service routine */
void isr_PIT1(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '(';

	// Clear the processor interrupt flag
	mmc_clear_PIT1_int;
	reg_PCSR1.reg = reg_PCSR1.reg;
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_PIT2
* Purpose: Provides an normal interrupt service routine for PIT interrupts
*******************************************************************************/
//#pragma interrupt on	    				   /* PIT1 interrupt service routine */
void isr_PIT2(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = ')';

	// Clear the processor interrupt flag
	mmc_clear_PIT2_int;
	reg_PCSR2.reg = reg_PCSR2.reg;
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
				if (g_sampleProcessing == SAMPLING_STATE)
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
			gp_ISRMessageBuffer->status = CMD_MSG_OVERFLOW_ERR;
		}
		else
		{
			// Write the received data into the buffer (clears the interrupt as well)
			*(gp_ISRMessageBuffer->writePtr) = dataReg;

			gp_ISRMessageBuffer->writePtr++;

			// Check if buffer pointer goes beyond the end
			if (gp_ISRMessageBuffer->writePtr >= (gp_ISRMessageBuffer->msg + CMD_BUFFER_SIZE))
			{
				// Reset the buffer pointer to the beginning of the buffer
				gp_ISRMessageBuffer->writePtr = gp_ISRMessageBuffer->msg;
			}

			// Raise the Craft Data flag
			g_ModemStatus.craftPortRcvFlag = YES;
		}
	}
#endif
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_SCI2
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_SCI2(void)
{
	uint8 statusReg = 0;
	uint8 dataReg = 0;

    MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '-';

	// Read the Status register
	statusReg = imm->Sci2.SCISR1;

	// Read out the data register to clear the interrupt
	dataReg = imm->Sci2.SCIDRL;
}
//#pragma interrupt off


/*******************************************************************************
* Function: isr_SPI_Transfer_Complete
* Purpose:
*******************************************************************************/
//#pragma interrupt on
void isr_SPI_Transfer_Complete(void)
{
    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '+';
}
//#pragma interrupt off

/*******************************************************************************
* Function: isr_MSP430WakeupMsg
* Purpose:
*******************************************************************************/
//#pragma stack_regs off
//#pragma fast_interrupt on
//#pragma interrupt on
void isr_MSP430WakeupMsg(void)
{
	extern uint32 gTotalSamples;
	static uint32 numOfSampleCnt = 0;
	uint32 numBytes = (uint32)(gp_SensorInfo->numOfChannels * 2);
	volatile uint8* spiDR = _SPIDR;
	volatile uint8* spiSR = _SPISR;
	ISPI_PACKET* rTailOfPreTrigBuff = (ISPI_PACKET*)tailOfPreTrigBuff;
	uint32 i = 0;
	uint32 j;
	uint8 tempByte;
	//uint8 dcdStatus = (uint8)(reg_TIM1PORT.reg & 0x08);
	//uint8 SS_Bit; //uncomment if we want to handshake every byte
	//uint8 temp;   //uncomment if we what to handshake every byte

    //MMC2114_IMM *imm = mmc2114_get_immp();
	//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = '=';

	if (READ_DCD == CONNECTION_ESTABLISHED)
	{
		// Clear RTS to hold the remote end from sending any more data
		CLEAR_RTS;
	}

	// Clear the processor interrupt flag
  	mmc_clear_EPF7_int;

	if (ISPI_GetISPI_State() == ISPI_RECEIVE)
	{
	    // uncomment if we want to handshake every byte
		//SS_Bit = (uint8)((*(uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT);

		//used i so I would not have to create another variable (Its about proformance)
		// make a comment if we want to handshake every byte
		i = (uint32)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT);

		*spiDR = IDLE_DATA;
		while ((*spiSR & ISPI_IRQ_BIT) != ISPI_IRQ_BIT){;}
		rTailOfPreTrigBuff->sampleByte[0] = *spiDR;

        // comment if we want to handshake every byte
		while (i == ((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT)){;}

		for (i = 2, j = 1; i < numBytes; i++, j++)
		{
		    // uncomment if we want to handshake every byte
			//while (SS_Bit == (temp = (uint8)((*(uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT))){;}
            //SS_Bit = temp;

			*spiDR = IDLE_DATA;
			while ((*spiSR & ISPI_IRQ_BIT) != ISPI_IRQ_BIT){;}
			rTailOfPreTrigBuff->sampleByte[j] = *spiDR;
		}

        // uncomment if we want to handshake every byte
		//while (SS_Bit == (temp = (uint8)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT))){;}
		*spiDR = IDLE_DATA;
		while ((*spiSR & ISPI_IRQ_BIT) != ISPI_IRQ_BIT){;}
		rTailOfPreTrigBuff->sampleByte[j] = *spiDR;

		if (manual_cal_flag)
		{
			ProcessManuelCalPulse();
		}

		else if (trig_rec.op_mode == WAVEFORM_MODE)
		{
			ProcessWaveformData();
		}

		else // (trig_rec.op_mode == BARGRAPH_MODE)
		{
			ProcessBargraphData();
		}

		numOfSampleCnt = numOfSampleCnt + gp_SensorInfo->numOfChannels;
		if (numOfSampleCnt >= gTotalSamples)
		{
			//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = 'I';
			ISPI_SetISPI_State(ISPI_IDLE);
			numOfSampleCnt = 0;
		}
	}

	else if (ISPI_GetISPI_State() == ISPI_IDLE)
	{
	    // uncomment if we want to handshake every btye
		//SS_Bit = (uint8)((*(uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT);

	    //used j so I would not have to create another variable (Its about proformance)
		// comment if we want to handshake every byte
		j = (uint32)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT);

		*spiDR = IDLE_DATA;
		while ((*spiSR & ISPI_IRQ_BIT) != ISPI_IRQ_BIT)
		{
			i++;
		}
		tempByte = *spiDR;

		if (tempByte == SAMPLING_CMD_ID)
		{
		    // comment out if we want to handshake every byte
			while (j == ((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & (uint8)ISPI_DDRSP3_BIT)){;}
			for (i = 0; i < numBytes; i++)
			{
			    // uncomment if we want to handshake every byte
				//while (SS_Bit == (temp = (uint8)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT))){;}
                //SS_Bit = temp;
				*spiDR = IDLE_DATA;
				while ((*spiSR & ISPI_IRQ_BIT) != ISPI_IRQ_BIT){;}
				rTailOfPreTrigBuff->sampleByte[i] = *spiDR;
			}

			if (manual_cal_flag)
			{
				ProcessManuelCalPulse();
			}

			else if (trig_rec.op_mode == WAVEFORM_MODE)
			{
				ProcessWaveformData();
			}

			else //if (trig_rec.op_mode == BARGRAPH_MODE)
			{
				ProcessBargraphData();
			}

			numOfSampleCnt = numOfSampleCnt + gp_SensorInfo->numOfChannels;
			//while (!(imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_TC)) {;} imm->Sci1.SCIDRL = 'R';
			ISPI_SetISPI_State(ISPI_RECEIVE);
		}
	}

	else
	{
		// TODO: This is an error!!

		// Since we "hang", print the error to the craft
		// Note: If the while (1) condition is replaced (and operation is set to continue),
		//       remove the print statement to prevent the ISR from running to long
		debugErr("430 Wakeup ISR: Received an invalid message. Halting!\n");
		while (1){;}
	}

	if (READ_DCD == CONNECTION_ESTABLISHED)
	{
		// Set RTS to allow the remote end to send data again
		SET_RTS;
	}
}
//#pragma stack_regs reset
//#pragma fast_interrupt off
//#pragma interrupt off
#endif // huge block

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
	// Print test for verification of operation
	debugRaw("&");

#if 0
	if (g_kpadProcessingFlag == DEACTIVATED)
	{
		raiseSystemEventFlag(KEYPAD_EVENT);

		// Found a new key, reset last stored key
		g_kpadLastKeyPressed = 0;
	}
#endif

#if 1
	uint8 keyScan;
	keyScan = read_mcp23018(IO_ADDRESS_KPD, INTFA);
	{
		debug("System IRQ: Interrupt Flags: %x\n", keyScan);
	}

	keyScan = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	{
		debug("System IRQ: Key Pressed: %x\n", keyScan);
	}
#endif

	read_mcp23018(IO_ADDRESS_KPD, INTFA);
	read_mcp23018(IO_ADDRESS_KPD, GPIOA);

	// clear the interrupt flag in the processor
	AVR32_EIC.ICR.int4 = 1;
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
	if (++cyclicEventDelay >= 8)
	{
		cyclicEventDelay = 0;
		raiseSystemEventFlag(CYCLIC_EVENT);
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
// tc_irq
// ============================================================================
#if 1
extern uint16* startOfPreTrigBuff;
extern uint16* tailOfPreTrigBuff;
extern uint16* endOfPreTrigBuff;
#endif

__attribute__((__interrupt__))
void tc_irq(void)
{
	static uint16 r_chan_read, v_chan_read, t_chan_read, a_chan_read;
	static uint16 temp_chan_read, configReadBack;

#if 1
	static uint32 sampleCount = 0;
	static uint32 calSampleCount = 0;
	static uint8 recording = NO;
	static uint8 calPulse = NO;
	//static uint8 calPulseDelay = 0;
	static uint8 trigFound = NO, alarm1Found = NO, alarm2Found = NO;
#endif

	// Sample Output with return config words
	// R: 7f26 (e0d0) | V: 7f10 (e2d0) | T: 7f15 (e4d0) | A: 7f13 (e6d0) | Temp:  dbe (b6d0)

	// Data time!
    // Chan 0 - R
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &r_chan_read);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &configReadBack);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);
	if(configReadBack != 0xe0d0) debugErr("AD Channel Sync Error!\n");
    //debugRaw("R: %4x (%4x)", r_chan_read, configReadBack);

    // Chan 1 - T
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &t_chan_read);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &configReadBack);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);
	if(configReadBack != 0xe2d0) debugErr("AD Channel Sync Error!\n");
    //debugRaw(" | V: %4x (%4x)", v_chan_read, configReadBack);

    // Chan 2 - V
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &v_chan_read);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &configReadBack);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);
	if(configReadBack != 0xe4d0) debugErr("AD Channel Sync Error!\n");
    //debugRaw(" | T: %4x (%4x)", t_chan_read, configReadBack);

    // Chan 3 - A
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &a_chan_read);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &configReadBack);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);
	if(configReadBack != 0xe6d0) debugErr("AD Channel Sync Error!\n");
    //debugRaw(" | A: %4x (%4x)", a_chan_read, configReadBack);

    // Temp
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &temp_chan_read);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &configReadBack);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);
	if(configReadBack != 0xb6d0) debugErr("AD Channel Sync Error!\n");
    //debugRaw(" | Temp: %4x (%4x)\n", temp_chan_read, configReadBack);
	// End Data Time!

	// Store Data in Pretrigger
	// Check for Triggers and Alarms
	
	// Fit into legacy design
	r_chan_read >>= 4;
	v_chan_read >>= 4;
	t_chan_read >>= 4;
	a_chan_read >>= 4;
	
	((SAMPLE_DATA_STRUCT*)tailOfPreTrigBuff)->r = r_chan_read;
	((SAMPLE_DATA_STRUCT*)tailOfPreTrigBuff)->v = v_chan_read;
	((SAMPLE_DATA_STRUCT*)tailOfPreTrigBuff)->t = t_chan_read;
	((SAMPLE_DATA_STRUCT*)tailOfPreTrigBuff)->a = a_chan_read;

	// Check to see if collecting data to fill the pretrigger buffer
	if ((g_sampleProcessing != SAMPLING_STATE) && (trig_rec.op_mode == WAVEFORM_MODE || trig_rec.op_mode == COMBO_MODE))
	{
		tailOfPreTrigBuff += gp_SensorInfo->numOfChannels;

		// Check if the end of the PreTrigger buffer has been reached
		if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;
	}

	// Processing Data in real time
	if (g_sampleProcessing == SAMPLING_STATE)
	{
		// Normalize
		if(r_chan_read > 0x800) r_chan_read = r_chan_read - 0x800; 
		else r_chan_read = 0x800 - r_chan_read;

		if(v_chan_read > 0x800) v_chan_read = v_chan_read - 0x800; 
		else v_chan_read = 0x800 - v_chan_read;

		if(t_chan_read > 0x800) t_chan_read = t_chan_read - 0x800; 
		else t_chan_read = 0x800 - t_chan_read;

		if(a_chan_read > 0x800) a_chan_read = a_chan_read - 0x800; 
		else a_chan_read = 0x800 - a_chan_read;
	
		// Check if handling a manual calibration
		if (manual_cal_flag == FALSE)
		{
			// Check if not recording an event and not handling a cal pulse
			if ((recording == NO) && (calPulse == NO))
			{
				if (trig_rec.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
				{
					if (r_chan_read > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
					if (v_chan_read > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
					if (t_chan_read > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
				}

				if (trig_rec.trec.soundTriggerLevel != NO_TRIGGER_CHAR)
					if (a_chan_read > trig_rec.trec.soundTriggerLevel) trigFound = YES;

				if ((trigFound == YES) && (recording == NO) && (calPulse == NO))
				{
					// Add command nibble to signal a tigger
					*(tailOfPreTrigBuff + 0) |= TRIG_ONE;
					*(tailOfPreTrigBuff + 1) |= TRIG_ONE;
					*(tailOfPreTrigBuff + 2) |= TRIG_ONE;
					*(tailOfPreTrigBuff + 3) |= TRIG_ONE;

					//debug("--> Trigger Found! %x %x %x %x\n", r_chan_read, v_chan_read, t_chan_read, a_chan_read);
					
					recording = YES;
					sampleCount = trig_rec.trec.record_time * trig_rec.trec.sample_rate;
				}
			}
			// Else check if we are still recording
			else if ((recording == YES) && (sampleCount))
			{
				sampleCount--;

				// Check if seismic is enabled for Alarm 1
				if ((help_rec.alarm_one_mode == ALARM_MODE_BOTH) || (help_rec.alarm_one_mode == ALARM_MODE_SEISMIC))
				{
					if (r_chan_read > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
					if (v_chan_read > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
					if (t_chan_read > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}

				// Check if air is enabled for Alarm 1
				if ((help_rec.alarm_one_mode == ALARM_MODE_BOTH) || (help_rec.alarm_one_mode == ALARM_MODE_AIR))
				{
					if (a_chan_read > help_rec.alarm_one_air_lvl) alarm1Found = YES;
				}

				// Check if seismic is enabled for Alarm 2
				if ((help_rec.alarm_two_mode == ALARM_MODE_BOTH) || (help_rec.alarm_two_mode == ALARM_MODE_SEISMIC))
				{
					if (r_chan_read > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
					if (v_chan_read > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
					if (t_chan_read > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}

				// Check if air is enabled for Alarm 2
				if ((help_rec.alarm_two_mode == ALARM_MODE_BOTH) || (help_rec.alarm_two_mode == ALARM_MODE_SEISMIC))
				{
					if (a_chan_read > help_rec.alarm_two_air_lvl) alarm2Found = YES;
				}

				// Check if Alarm 2 condition was met (Alarm 2 overrides Alarm 1)
				if (alarm2Found == YES)
				{
					// Add command nibble to signal a tigger
					*(tailOfPreTrigBuff + 0) |= TRIG_THREE;
					*(tailOfPreTrigBuff + 1) |= TRIG_THREE;
					*(tailOfPreTrigBuff + 2) |= TRIG_THREE;
					*(tailOfPreTrigBuff + 3) |= TRIG_THREE;
				}
				// Else check if Alarm 1 condition was met
				else if (alarm1Found == YES)
				{
					// Add command nibble to signal a tigger
					*(tailOfPreTrigBuff + 0) |= TRIG_TWO;
					*(tailOfPreTrigBuff + 1) |= TRIG_TWO;
					*(tailOfPreTrigBuff + 2) |= TRIG_TWO;
					*(tailOfPreTrigBuff + 3) |= TRIG_TWO;
				}
			}
			// Else check if we are still recording but handling the last sample
			else if ((recording == YES) && (sampleCount == 0))
			{
				//debug("--> Recording done!\n");

				recording = NO;
				trigFound = NO;
				calPulse = YES;
				calSampleCount = 100 * (trig_rec.trec.sample_rate / 1024);

				// Start Cal
				*(tailOfPreTrigBuff + 0) |= CAL_START;
				*(tailOfPreTrigBuff + 1) |= CAL_START;
				*(tailOfPreTrigBuff + 2) |= CAL_START;
				*(tailOfPreTrigBuff + 3) |= CAL_START;

				SetCalSignalEnable(ON);
			}
			// Else check if a cal pulse is enabled
			else if ((calPulse == YES) && (calSampleCount))
			{
				if (calSampleCount == (100 * (trig_rec.trec.sample_rate / 1024)))
				{
					SetCalSignalEnable(ON);
					SetCalSignal(ON);
				}

				if (calSampleCount == (91 * (trig_rec.trec.sample_rate / 1024)))
					SetCalSignal(OFF);

				if (calSampleCount == (73 * (trig_rec.trec.sample_rate / 1024)))
					SetCalSignal(ON);

				if (calSampleCount == (64 * (trig_rec.trec.sample_rate / 1024)))
					SetCalSignalEnable(OFF);

				calSampleCount--;
			}
			else if ((calPulse == YES) && (calSampleCount == 0))
			{
				//debug("--> Cal done!\n");

				calPulse = NO;
			}
		}
		else //manual_cal_flag == TRUE
		{
			if (manualCalSampleCount == 0)
			{
				// Start Cal
				*(tailOfPreTrigBuff + 0) |= CAL_START;
				*(tailOfPreTrigBuff + 1) |= CAL_START;
				*(tailOfPreTrigBuff + 2) |= CAL_START;
				*(tailOfPreTrigBuff + 3) |= CAL_START;
			}

			if (manualCalSampleCount == 99)
			{
				// Stop Cal
				*(tailOfPreTrigBuff + 0) |= CAL_END;
				*(tailOfPreTrigBuff + 1) |= CAL_END;
				*(tailOfPreTrigBuff + 2) |= CAL_END;
				*(tailOfPreTrigBuff + 3) |= CAL_END;
			}

			manualCalSampleCount++;
		}

		if (manual_cal_flag)
		{
			ProcessManuelCalPulse();
		}
		else if (trig_rec.op_mode == WAVEFORM_MODE)
		{
			ProcessWaveformData();
		}
		else if (trig_rec.op_mode == BARGRAPH_MODE)
		{
			ProcessBargraphData();
		}
		else if (trig_rec.op_mode == COMBO_MODE)
		{
			ProcessComboData();
		}
#if 0 // Temp code used to fill pretrigger buffer if not processing
		tailOfPreTrigBuff += gp_SensorInfo->numOfChannels;

		// Check if the end of the PreTrigger buffer has been reached
		if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;
#endif
	}	

	// clear the interrupt flag
	AVR32_TC.channel[TC_CHANNEL].sr;

#if 0
	static volatile uint32 testCounterDataClock = 0;

	// Increment the ms seconds counter
	testCounterDataClock++;

	// specify that an interrupt has been raised
	if(testCounterDataClock % 1000 == 0)
	{
		debugRaw("*");
	}
#endif
}

// ============================================================================
// Setup_EIC_Keypad_ISR
// ============================================================================
void Setup_EIC_Keypad_ISR(void)
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
	if(AVR32_EIC.IMR.int5 == 0x01)
	{
		print_dbg("\r\nKeypad Interrupt Enabled\n");
	}
	else
	{
		print_dbg("\r\nKeypad Interrupt Not Enabled\n");
	}
#endif
}

// ============================================================================
// Setup_EIC_System_ISR
// ============================================================================
void Setup_EIC_System_ISR(void)
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
	if(AVR32_EIC.IMR.int4 == 0x01)
	{
		debug("\r\nSystem Interrupt Enabled\n");
	}
	else
	{
		debug("\r\nSystem Interrupt Not Enabled\n");
	}
#endif
}

// ============================================================================
// Setup_Soft_Timer_Tick_ISR
// ============================================================================
void Setup_Soft_Timer_Tick_ISR(void)
{
	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&soft_timer_tick_irq, AVR32_RTC_IRQ, 0);
}

// ============================================================================
// Setup_Soft_Timer_Tick_ISR
// ============================================================================
#define FOSC0	66000000 // 66 MHz
void Setup_Data_Clock_ISR(uint32 sampleRate)
{
	volatile avr32_tc_t *tc = &AVR32_TC;

	// Options for waveform generation.
	tc_waveform_opt_t WAVEFORM_OPT =
	{
		.channel  = TC_CHANNEL,                        // Channel selection.
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
		.tcclks   = TC_CLOCK_SOURCE_TC2                // Internal source clock 2 - connected to PBA/4
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

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&tc_irq, AVR32_TC_IRQ0, 0);

	// Initialize the timer/counter.
	tc_init_waveform(tc, &WAVEFORM_OPT);         // Initialize the timer/counter waveform.

	// Set the compare triggers.
	// Remember TC counter is 16-bits, so counting second is not possible.
	// We configure it to count ms.
	// We want: (1/(FOSC0/4)) * RC = 1000 Hz => RC = (FOSC0/4) / 1000 = 3000 to get an interrupt every 1ms
	//tc_write_rc(tc, TC_CHANNEL, (FOSC0/2)/1000);  // Set RC value.
	tc_write_rc(tc, TC_CHANNEL, (FOSC0 / (sampleRate * 2)));
	
	tc_configure_interrupts(tc, TC_CHANNEL, &TC_INTERRUPT);
}

// ============================================================================
// Start_Data_Clock
// ============================================================================
void Start_Data_Clock(void)
{
	volatile avr32_tc_t *tc = &AVR32_TC;

	// Start the timer/counter.
	tc_start(tc, TC_CHANNEL);                    // And start the timer/counter.
}	

// ============================================================================
// Stop_Data_Clock
// ============================================================================
void Stop_Data_Clock(void)
{
	volatile avr32_tc_t *tc = &AVR32_TC;

	// Stop the timer/counter.
	tc_stop(tc, TC_CHANNEL);                    // And start the timer/counter.
}	
