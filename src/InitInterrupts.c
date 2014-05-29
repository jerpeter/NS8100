///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "sdramc.h"
#include "intc.h"
#include "usart.h"
#include "print_funcs.h"
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
#include "Common.h"
#include "Display.h"
#include "Menu.h"
#include "Uart.h"
#include "spi.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "SysEvents.h"
#include "Record.h"
#include "InitDataBuffers.h"
#include "PowerManagement.h"
#include "SoftTimer.h"
#include "Keypad.h"
#include "RealTimeClock.h"
#include "RemoteHandler.h"
#include "TextTypes.h"
#include "RemoteCommon.h"
#include "twi.h"
#include "M23018.h"
#include "sd_mmc_spi.h"
#include "FAT32_Disk.h"
#include "FAT32_Access.h"
#include "adc.h"
#include "usb_task.h"
#include "device_mass_storage_task.h"
#include "usb_drv.h"
#include "FAT32_FileLib.h"
#include "srec.h"
#include "flashc.h"
#include "rtc.h"
#include "tc.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define ONE_MS_RESOLUTION	1000

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

extern void rtc_enable_interrupt(volatile avr32_rtc_t *rtc);
extern void Eic_keypad_irq(void);
extern void Eic_system_irq(void);
extern void Eic_external_rtc_irq(void);
extern void Tc_sample_irq(void);
extern void Usart_1_rs232_irq(void);
extern void Soft_timer_tick_irq(void);
extern void Tc_sample_irq(void);
extern void Tc_typematic_irq(void);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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
	INTC_register_interrupt(&Eic_keypad_irq, AVR32_EIC_IRQ_5, 0);

#if 0 // Some residual wrongly executed code
	// Enable the interrupt
	rtc_enable_interrupt(&AVR32_RTC);
#endif

	#if 0
	// Test for int enable
	if(AVR32_EIC.IMR.int5 == 0x01) debug("\nKeypad Interrupt Enabled\n");
	else debug("\nKeypad Interrupt Not Enabled\n");
	#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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
	INTC_register_interrupt(&Eic_system_irq, AVR32_EIC_IRQ_4, 0);

#if 0 // Some residual wrongly executed code
	// Enable the interrupt
	rtc_enable_interrupt(&AVR32_RTC);
#endif

	#if 0
	// Test for int enable
	if(AVR32_EIC.IMR.int4 == 0x01) debug("\nSystem Interrupt Enabled\n");
	else debug("\nSystem Interrupt Not Enabled\n");
	#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Setup_8100_EIC_External_RTC_ISR(void)
{
	// External Interrupt Controller setup
	AVR32_EIC.IER.int1 = 1;
	AVR32_EIC.MODE.int1 = 0;
	AVR32_EIC.EDGE.int1 = 0;
	AVR32_EIC.LEVEL.int1 = 0;
	AVR32_EIC.FILTER.int1 = 0;
	AVR32_EIC.ASYNC.int1 = 1;
	AVR32_EIC.EN.int1 = 1;

	// Register the RTC interrupt handler to the interrupt controller.
	#if 0 // Test
	INTC_register_interrupt(&Eic_external_rtc_irq, AVR32_EIC_IRQ_1, 0);
	#else // Hook in the External RTC interrupt to the actual sample processing interrupt handler
	INTC_register_interrupt(&Tc_sample_irq, AVR32_EIC_IRQ_1, 0);
	#endif

	#if 1
	// Test for int enable
	if(AVR32_EIC.IMR.int1 == 0x01) { debug("External RTC Interrupt Enabled\n"); }
	else { debug("External RTC Interrupt Not Enabled\n"); }
	#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Setup_8100_Usart_RS232_ISR(void)
{
	INTC_register_interrupt(&Usart_1_rs232_irq, AVR32_USART1_IRQ, 1);

	// Enable Receive Ready, Overrun, Parity and Framing error interrupts
	AVR32_USART1.ier = (AVR32_USART_IER_RXRDY_MASK | AVR32_USART_IER_OVRE_MASK |
	AVR32_USART_IER_PARE_MASK | AVR32_USART_IER_FRAME_MASK);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Setup_8100_Soft_Timer_Tick_ISR(void)
{
	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&Soft_timer_tick_irq, AVR32_RTC_IRQ, 0);
	
	// Enable half second tick
	rtc_enable_interrupt(&AVR32_RTC);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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
		INTC_register_interrupt(&Tc_sample_irq, AVR32_TC_IRQ0, 3);
		break;
		
		case TC_CALIBRATION_TIMER_CHANNEL:
		// Register the RTC interrupt handler to the interrupt controller.
		INTC_register_interrupt(&Tc_sample_irq, AVR32_TC_IRQ1, 3);
		break;
		
		case TC_TYPEMATIC_TIMER_CHANNEL:
		// Register the RTC interrupt handler to the interrupt controller.
		INTC_register_interrupt(&Tc_typematic_irq, AVR32_TC_IRQ2, 0);
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitInterrupts_NS8100(void)
{
	Disable_global_interrupt();

	//Setup interrupt vectors
	INTC_init_interrupts();

	// Hook in and enable half second tick
	Setup_8100_Soft_Timer_Tick_ISR();
	
	// Setup typematic timer for repeat key interrupt
	Setup_8100_TC_Clock_ISR(ONE_MS_RESOLUTION, TC_TYPEMATIC_TIMER_CHANNEL);

	// Setup keypad for interrupts
	Setup_8100_EIC_Keypad_ISR();
	Setup_8100_EIC_System_ISR();

	#if 0 // Moved interrupt setup to the end of the BootloaderManager to allow for searching for Ctrl-B
	InitCraftInterruptBuffers();
	Setup_8100_Usart_RS232_ISR();
	#endif
	
	Enable_global_interrupt();
}

