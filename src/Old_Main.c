///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Old_Main.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:55 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Old_Main.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Old_Board.h"
#include "Common.h"
#include "Display.h"
#include "Menu.h"
#include "Uart.h"
#include "Ispi.h"
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

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitSystemHardware(void);
void InitInterrupts(void);
void InitSoftwareSettings(void);

/****************************************
*	Function:	initSystem
*	Purpose:	Initialize all the system components
****************************************/
void InitSystemHardware(void)
{
#if 0 // Old system
	// Initialize MMC2114 registers
	initProcessor();

	// Initialize the keypad IO device
	initKeypad();

	// Printer serial port setup
	uart_init(RS485_BAUDRATE, RS485_COM_PORT);

	// Craft serial port setup
	uart_init(CRAFT_BAUDRATE, CRAFT_COM_PORT);

	// SPI initialization
	ISPI_Init();

	// Set default power options
	setMcorePwMgntDefaults();

	// Set default contrast
	setLcdContrast(DEFUALT_CONTRAST);

	// Turn LCD on
	powerControl(LCD_POWER_ENABLE, ON);

	// Setup LCD segments and clear display buffer
	initLcdDisplay();

	// Set backlight state
	setLcdBacklightState(BACKLIGHT_DIM);

	// Init the real time clock
	debug("Setting up RTC\n");
	InitExternalRtc();
#endif
}

/****************************************
*	Function:	initInterrupts
*	Purpose:	Initialize the system interrupts to be used
****************************************/
void InitInterrupts(void)
{
#if 0 // ns7100
	debug("Init Interrupt Buffers\n");
	initCraftInterruptBuffers();

	debug("Enable Interrupts\n");
	// Clear possible interrupt flags
	// Edge Port Interrupt flags
    mmc_clear_EPF0_int;				// Clear On key processor interrupt flag
    mmc_clear_EPF1_int;				// Clear Off key processor interrupt flag
    mmc_clear_EPF2_int;				// Clear Lan processor interrupt flag
    mmc_clear_EPF3_int;				// Clear Keypad processor interrupt flag
    mmc_clear_EPF4_int;				// Clear USB Host processor interrupt flag
    mmc_clear_EPF5_int;				// Clear USB Device processor interrupt flag
    mmc_clear_EPF6_int;				// Clear RTC processor interrupt flag
    mmc_clear_EPF7_int;				// Clear Ext. Trig processor interrupt flag

	// Enable use of Normal Interrupts
	// Edge Port Interrupt Enables
    mmc_enNormalInterrupt4;  		// Power On key interrupt (Edge Port Pin 0)
    mmc_enNormalInterrupt5;  		// Power Off key interrupt (Edge Port Pin 1)
    //mmc_enNormalInterrupt6;  		// Lan interrupt (Edge Port Pin 2)
    mmc_enNormalInterrupt0;  		// Keypad interrupt (Edge Port Pin 3)
    //mmc_enNormalInterrupt7;  		// USB Host interrupt (Edge Port Pin 4)
    //mmc_enNormalInterrupt8;  		// USB Device interrupt (Edge Port Pin 5)
    mmc_enNormalInterrupt3;  		// RTC Periodic interrupt (Edge Port Pin 6)
    //mmc_enNormalInterrupt30; 		// Ext. Trig interrupt (Edge Port Pin 7)

	// System Interrupt Enables
	mmc_enNormalInterrupt21;		// Craft receive data interrupt (SCI1)
	mmc_enNormalInterrupt29; 		// PIT timer tick interrupt (PIT1)

	// Enable use of Fast Interrupts
	//ISPI_IRQEnable(TRUE);			// low level enable at module level

	// Enable interrupts at the core processor level
    EnableAllInterrupts;
#endif
}

/****************************************
*	Function:	InitSoftwareSettings
*	Purpose:	Initialize software variables and settings
****************************************/
void InitSoftwareSettings(void)
{
	INPUT_MSG_STRUCT mn_msg;

	debug("Init Software Settings\n");

	// Init version strings
    debug("Init Version Strings...\n");
	initVersionStrings();

	// Init version msg
    debug("Init Version Msg...\n");
	initVersionMsg();

	// Init time msg
    debug("Init Time Msg...\n");
	initTimeMsg();

	// Get the function address passed by the bootloader
    debug("Init Get Boot Function Addr...\n");
	getBootFunctionAddress();

	// Setup defaults, load records, init the language table
    debug("Init Setup Menu Defaults...\n");
	setupMnDef();

    debug("Init Timer Mode Check...\n");
	// Check for Timer mode activation
	if (timerModeActiveCheck() == TRUE)
	{
		debug("--- Timer Mode Startup ---\n");
		processTimerMode();

		// If here, the unit is in Timer mode, but did not power itself off yet
		// Disable the Power Off key
		debug("Timer Mode: Disabling Power Off key\n");
		powerControl(POWER_SHUTDOWN_ENABLE, OFF);
	}
	else
	{
		debug("--- Normal Startup ---\n");
	}

	// Init the cmd message handling buffers before initialization of the ports.
    debug("Init Cmd Msg Handler...\n");
	cmdMessageHandlerInit();

	// Init the input buffers and status flags for input craft data.
    debug("Init Craft Init Status Flags...\n");
	craftInitStatusFlags();

#if 0 // ns7100
	// Overlay a message to alert the user that the system is checking the sensors
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	overlayMessage(getLangText(STATUS_TEXT), buff, 0);
#endif

    debug("Init Jump to Main Menu...\n");

	debug("Jump to Main Menu\n");
	// Jump to the true main menu
	g_activeMenu = MAIN_MENU;
	ACTIVATE_MENU_MSG();
	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

/****************************************
*	Function:	SystemEventManager
*	Purpose:	Handle the system events as they arise
****************************************/
//static inline void SystemEventManager(void)
void SystemEventManager(void)
{
	//debug("System Event Manager called\n");

	if (getSystemEventState(TRIGGER_EVENT))
	{
		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			debug("Trigger Event (Wave)\n");
			MoveWaveformEventToFlash();
		}		
		else if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			debug("Trigger Event (Combo)\n");
			MoveComboWaveformEventToFile();
		}		
	}

	if (getSystemEventState(MANUEL_CAL_EVENT))
	{
	    debug("Cal Pulse Event\n");
	    MoveManuelCalToFlash();
	}

	if ((getSystemEventState(KEYPAD_EVENT)) ||
		(g_kpadCheckForKeyFlag && (g_kpadLookForKeyTickCount < g_keypadTimerTicks)))
	{
		if (getSystemEventState(KEYPAD_EVENT))
		{
			debug("Keypad Event\n");
			clearSystemEventFlag(KEYPAD_EVENT);
		}

		keypad();
	}

	if (getSystemEventState(POWER_OFF_EVENT))
	{
		debug("Power Off Event\n");
		clearSystemEventFlag(POWER_OFF_EVENT);

		handleUserPowerOffDuringTimerMode();
	}

	if (getSystemEventState(CYCLIC_EVENT))
	{
		debug("Cyclic Event\n");
		clearSystemEventFlag(CYCLIC_EVENT);

#if 1 // Test
		//sprintf((char*)&g_spareBuffer[0], "%d (%s) %d", (int)g_execCycles, ((g_channelSyncError == YES) ? "YES" : "NO"), (int)(g_sampleCount / 4));
		//overlayMessage("EXEC CYCLES", (char*)&g_spareBuffer[0], 500 * SOFT_MSECS);
		debugRaw("ISR Ticks/sec: %d (E:%s), Exec: %d\n", (g_sampleCountHold / 4), ((g_channelSyncError == YES) ? "YES" : "NO"), g_execCycles);
		g_sampleCountHold = 0;
		g_execCycles = 0;
		g_channelSyncError = NO;
#endif

		procTimerEvents();
	}

	if (getSystemEventState(MIDNIGHT_EVENT))
	{
		debug("Midnight Event\n");
		clearSystemEventFlag(MIDNIGHT_EVENT);

		handleMidnightEvent();
	}

	if (getSystemEventState(UPDATE_TIME_EVENT))
	{
		clearSystemEventFlag(UPDATE_TIME_EVENT);
		updateCurrentTime();
	}

	if (getSystemEventState(BARGRAPH_EVENT))
	{
		clearSystemEventFlag(BARGRAPH_EVENT);

		if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			CalculateBargraphData();
		}		
		else if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			CalculateComboData();
		}		
	}

#if 0
	if (getSystemEventState(COMBO_EVENT))
	{
		clearSystemEventFlag(COMBO_EVENT);
		CalculateComboData();
	}
#endif

	if (getSystemEventState(WARNING1_EVENT))
	{
		debug("Warning Event 1\n");
		clearSystemEventFlag(WARNING1_EVENT);

#if 0 // ns7100
		// Activate alarm 1 signal
		reg_TIM2PORT.reg |= 0x04;
#else //ns8100
		powerControl(ALARM_1_ENABLE, ON);
#endif
		// Assign soft timer to turn the Alarm 1 signal off
		assignSoftTimer(ALARM_ONE_OUTPUT_TIMER_NUM, (uint32)(g_helpRecord.alarm_one_time * 2), alarmOneOutputTimerCallback);
	}

	if (getSystemEventState(WARNING2_EVENT))
	{
		debug("Warning Event 2\n");
		clearSystemEventFlag(WARNING2_EVENT);

#if 0 // ns7100
		// Activate alarm 2 signal
		reg_TIM2PORT.reg |= 0x08;
#else //ns8100
		powerControl(ALARM_2_ENABLE, ON);
#endif
		// Assign soft timer to turn the Alarm 2 signal off
		assignSoftTimer(ALARM_TWO_OUTPUT_TIMER_NUM, (uint32)(g_helpRecord.alarm_two_time * 2), alarmTwoOutputTimerCallback);
	}

	if (getSystemEventState(AUTO_DIALOUT_EVENT))
	{
		clearSystemEventFlag(AUTO_DIALOUT_EVENT);

		startAutoDialoutProcess();
	}
}

/****************************************
*	Function:	MenuEventManager
*	Purpose:	Handle the menu events as they arise
****************************************/
//static inline void MenuEventManager(void)
void MenuEventManager(void)
{
	//debug("Menu Event Manager called\n");

	INPUT_MSG_STRUCT mn_msg;

	if (getMenuEventState(RESULTS_MENU_EVENT))
	{
		clearMenuEventFlag(RESULTS_MENU_EVENT);

		g_activeMenu = RESULTS_MENU;

		if (g_triggerRecord.op_mode == MANUAL_CAL_MODE)
		{
			RESULTS_MENU_MANUEL_CAL_MSG();
		}
		else
		{
			RESULTS_MENU_MONITORING_MSG();
		}
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
	}

	if (getTimerEventState(SOFT_TIMER_CHECK_EVENT))
	{
		clearTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

		// Handle and process software timers
	    checkSoftTimers();
	}

	if (getTimerEventState(TIMER_MODE_TIMER_EVENT))
	{
		clearTimerEventFlag(TIMER_MODE_TIMER_EVENT);

		debug("Timer mode: Start monitoring...\n");
		g_activeMenu = MONITOR_MENU;
		ACTIVATE_MENU_WITH_DATA_MSG(g_triggerRecord.op_mode);
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
	}
}

/****************************************
*	Function:	CraftManager
*	Purpose:	Handle craft input and events
****************************************/
//static inline void CraftManager(void)
void CraftManager(void)
{
	//debug("Craft Manager called\n");

	// Check if there is an modem present and if the connection state is connected
	if ((MODEM_CONNECTED == READ_DSR) && (CONNECTED == g_modemStatus.connectionState))
	{
		// Check if the connection has been lost
		if (NO_CONNECTION == READ_DCD)
		{
			// Check if a ring indicator has been received
			if ((g_modemStatus.ringIndicator != READ_RI) && (READ_RI == RING))
			{
				// Check if nine rings have been received
				if (g_modemStatus.numberOfRings++ > 9)
				{
					// Reset flag
					g_modemStatus.numberOfRings = 0;

					// Assign timer to process modem init
					assignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_QUICK_DELAY, modemDelayTimerCallback);
				}
			}

			// Assign ring indicator to the status of the ring indicator line
			g_modemStatus.ringIndicator = (uint8)READ_RI;

			// Relock the system
			g_modemStatus.systemIsLockedFlag = YES;

			if (CONNECTED == g_modemStatus.firstConnection)
			{
				g_modemStatus.firstConnection = NOP_CMD;
				assignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_QUICK_DELAY, modemDelayTimerCallback);
			}
		}
		else
		{
			g_modemStatus.firstConnection = CONNECTED;
		}
	}
	else
	{
		if ((YES == g_modemSetupRecord.modemStatus) && (CONNECTED == g_modemStatus.connectionState))
		{
			g_modemStatus.connectionState = NOP_CMD;
			g_modemStatus.systemIsLockedFlag = YES;
			assignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_DELAY, modemDelayTimerCallback);
		}
	}

	// Check if the Auto Dialout state is not idle
	if (g_autoDialoutState != AUTO_DIAL_IDLE)
	{
		// Run the Auto Dialout State machine
		autoDialoutStateMachine();
	}

	// Are we transfering data, if so process the correct command.
	if (NOP_CMD != g_modemStatus.xferState)
	{
		if (DEMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDEMData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.auto_print = g_modemStatus.xferPrintState;
			}
		}
		else if (DSMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDSMData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.auto_print = g_modemStatus.xferPrintState;
			}
		}
		else if (DQMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDQMData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.auto_print = g_modemStatus.xferPrintState;
			}
		}
		else if (VMLx_CMD == g_modemStatus.xferState)
		{
			sendVMLData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.auto_print = g_modemStatus.xferPrintState;
			}
		}
	}

	// Data from the craft port is independent of the modem.
	if (YES == g_modemStatus.craftPortRcvFlag)
	{
		g_modemStatus.craftPortRcvFlag = NO;
		processCraftData();
	}

	// Did we raise the craft data port flag, if so process the incomming data.
	if (getSystemEventState(CRAFT_PORT_EVENT))
	{
		clearSystemEventFlag(CRAFT_PORT_EVENT);
		cmdMessageProcessing();
	}
}

/****************************************
*	Function:	MessageManager
*	Purpose:	Handle messages that need to be processed
****************************************/
//static inline void MessageManager(void)
void MessageManager(void)
{
	INPUT_MSG_STRUCT input_msg;

	// Check if there is a message in the queue
	if (ckInputMsg(&input_msg) != INPUT_BUFFER_EMPTY)
	{
		debug("Processing message...\n");

		// Process message
		procInputMsg(input_msg);
	}
}

/****************************************
*	Function:	FactorySetupManager
*	Purpose:	Handle the factory setup operation
****************************************/
//static inline void FactorySetupManager(void)
void FactorySetupManager(void)
{
	//debug("Factory Setup Manager called\n");

	INPUT_MSG_STRUCT mn_msg;

	if (g_factorySetupSequence == ENTER_FACTORY_SETUP)
	{
		g_factorySetupSequence = PROCESS_FACTORY_SETUP;

		keypressEventMgr();
		messageBox(getLangText(STATUS_TEXT), getLangText(YOU_HAVE_ENTERED_THE_FACTORY_SETUP_TEXT), MB_OK);

		g_activeMenu = DATE_TIME_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
	}
#if 0 // Added to work around On Key IRQ problem
	else if (g_factorySetupRecord.invalid && g_factorySetupSequence != PROCESS_FACTORY_SETUP)
	{
		g_factorySetupSequence = PROCESS_FACTORY_SETUP;

		keypressEventMgr();
		messageBox(getLangText(STATUS_TEXT), getLangText(YOU_HAVE_ENTERED_THE_FACTORY_SETUP_TEXT), MB_OK);

		g_activeMenu = DATE_TIME_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
	}
#endif
}

/****************************************
*	Function:	handleSystemEvents
*	Purpose:	Handles system events when the main loop can't run (thread locked)
****************************************/
void handleSystemEvents(void)
{
	if (g_systemEventFlags.wrd)
	    SystemEventManager();

	if (g_menuEventFlags.wrd)
	    MenuEventManager();

	MessageManager();
}

/****************************************
*	Function:	main
*	Purpose:	Starting point for the application
****************************************/
#if 0 // ns7100
int old_main(void)
{
	InitSystemHardware();
	InitInterrupts();
	InitSoftwareSettings();

	debug("Unit Type: %s\n", SUPERGRAPH_UNIT ? "Supergraph" : "Minigraph");

	debug("--- System Init complete ---\n");

 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
		// Handle system events
	    SystemEventManager();

		// Handle menu events
	    MenuEventManager();

		// Handle craft processing
		CraftManager();

		// Handle messages to be processed
		MessageManager();

		// Handle processing the factory setup
		FactorySetupManager();

		// Check if no System Event and Lcd and Printer power are off
		if ((g_systemEventFlags.wrd == 0x0000) && !(powerManagement.reg & 0x60) &&
			(g_modemStatus.xferState == NOP_CMD))
		{
			// Sleep
			Wait();
		}
    }

	return (0);
}
#endif
