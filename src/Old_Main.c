///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Main.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/Main.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Mmc2114.h"
#include "Mmc2114_InitVals.h"
#include "Old_Board.h"
#include "Common.h"
#include "Display.h"
#include "Menu.h"
#include "Uart.h"
#include "Ispi.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "SysEvents.h"
#include "Rec.h"
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
extern void initProcessor(void);
extern SUMMARY_DATA *results_summtable_ptr;
extern int8 g_kpadCheckForKeyFlag;				// Flag to indicate key stroke in the ISR
extern uint32 g_kpadLookForKeyTickCount;		// Timer to count number of ticks for a key stroke.
extern volatile uint32 g_keypadTimerTicks;		//
extern POWER_MANAGEMENT_STRUCT powerManagement;	// Struct containing power management parameters
extern REC_HELP_MN_STRUCT help_rec;				// Struct containing system parameters
extern uint32 g_rtcSoftTimerTickCount;			// A global counter to count system ticks
extern uint8 g_factorySetupSequence;			//
extern uint32 isTriggered;
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
extern uint8 g_autoDialoutState;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
// Sensor information and constants.
#if 0
SENSOR_PARAMETERS_STRUCT g_SensorInfoStruct;
SENSOR_PARAMETERS_STRUCT* gp_SensorInfo = &g_SensorInfoStruct;

// Contains the event record in ram.
EVT_RECORD g_RamEventRecord;

// Factory Setup record.
FACTORY_SETUP_STRUCT factory_setup_rec;

// Structure to contain system paramters and system settings.
REC_EVENT_MN_STRUCT trig_rec;				// Contains trigger specific information.

// Menu specific structures
int active_menu;							// For active menu number/enum.
MN_EVENT_STRUCT mn_event_flags;				// Menu event flags, for main loop processing.
MN_TIMER_STRUCT mn_timer;					// Menu timer strucutre.

// System Event Flags, for main loopo processing.
SYS_EVENT_STRUCT SysEvents_flags;

MODEM_SETUP_STRUCT modem_setup_rec;			// Record for user input data.
MODEM_STATUS_STRUCT g_ModemStatus;			// Record for modem data processing.

// Used as a circular buffer to continually caputer incomming data from the craft port.
CMD_BUFFER_STRUCT isrMessageBufferStruct;
CMD_BUFFER_STRUCT* gp_ISRMessageBuffer = &isrMessageBufferStruct;

void (*menufunc_ptrs[TOTAL_NUMBER_OF_MENUS]) (INPUT_MSG_STRUCT) = {
		mainMn, loadRecMn, summaryMn, monitorMn, resultsMn,
		overWriteMn, batteryMn, dateTimeMn, lcdContrastMn, timerModeTimeMn,
		timerModeDateMn, calSetupMn, userMn, monitorLogMn
};
#else
extern SENSOR_PARAMETERS_STRUCT g_SensorInfoStruct;
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;

// Contains the event record in ram.
extern EVT_RECORD g_RamEventRecord;

// Factory Setup record.
extern FACTORY_SETUP_STRUCT factory_setup_rec;

// Structure to contain system paramters and system settings.
extern REC_EVENT_MN_STRUCT trig_rec;				// Contains trigger specific information.

// Menu specific structures
extern int active_menu;							// For active menu number/enum.
extern MN_EVENT_STRUCT mn_event_flags;				// Menu event flags, for main loop processing.
extern MN_TIMER_STRUCT mn_timer;					// Menu timer strucutre.

// System Event Flags, for main loopo processing.
extern SYS_EVENT_STRUCT SysEvents_flags;

extern MODEM_SETUP_STRUCT modem_setup_rec;			// Record for user input data.
extern MODEM_STATUS_STRUCT g_ModemStatus;			// Record for modem data processing.

// Used as a circular buffer to continually caputer incomming data from the craft port.
extern CMD_BUFFER_STRUCT isrMessageBufferStruct;
extern CMD_BUFFER_STRUCT* gp_ISRMessageBuffer;

extern void (*menufunc_ptrs[TOTAL_NUMBER_OF_MENUS]) (INPUT_MSG_STRUCT);
#endif

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitSystemHardware(void);
void InitInterrupts(void);
void InitSoftwareSettings(void);
#if 0
static inline void SystemEventManager(void);
static inline void MenuEventManager(void);
static inline void CraftManager(void);
static inline void MessageManager(void);
static inline void FactorySetupManager(void);
#endif

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
	InitRtc();
#endif
}

/****************************************
*	Function:	initInterrupts
*	Purpose:	Initialize the system interrupts to be used
****************************************/
void InitInterrupts(void)
{
#if 0 // fix_ns8100 // Old system
	debug("Init Interrupt Buffers\n");
	initInterruptBuffers();

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
	char buff[50];
	//uint16 i = 0;

	debug("Init Software Settings\n");

#if 0 // fix_ns8100
	GetParameterMemory((uint8*)&buff[0], 0, 50);
	for (i=0;i<50;i++)
	{
		debug("Parameter Mem at: 0x%x is: 0x%x\n", i, buff[i]);
	}
	for (i=0;i<50;i++)
	{
		buff[i] = (char)(50 - i);
	}
	SaveParameterMemory((uint8*)&buff[0], 0, 50);
	GetParameterMemory((uint8*)&buff[0], 0, 50);
	for (i=0;i<50;i++)
	{
		debug("Parameter Mem at: 0x%x is: 0x%x\n", i, buff[i]);
	}
#endif

    debug("Init Version Strings...\n");

	// Init version strings
	initVersionStrings();

    debug("Init Version Msg...\n");

	// Init version msg
	initVersionMsg();

    debug("Init Time Msg...\n");

	// Init time msg
	initTimeMsg();

    debug("Init Get Boot Function Addr...\n");

	// Get the function address passed by the bootloader
	getBootFunctionAddress();

    debug("Init Setup Menu Defaults...\n");

	debug("Setup Menu Defaults\n");
	// Setup defaults, load records, init the language table
	setupMnDef();

    debug("Init Timer Mode Check...\n");

	debug("Check Timer Mode\n");
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

    debug("Init Cmd Msg Handler...\n");

	debug("Cmd/Craft Init\n");
	// Init the cmd message handling buffers before initialization of the ports.
	cmdMessageHandlerInit();

    debug("Init Craft Init Status Flags...\n");

	// Init the input buffers and status flags for input craft data.
	craftInitStatusFlags();

    debug("Init LCD Message...\n");

	debug("LCD message\n");
	// Overlay a message to alert the user that the system is checking the sensors
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	overlayMessage(getLangText(STATUS_TEXT), buff, 0);

    debug("Init Jump to Main Menu...\n");

	debug("Jump to Main Menu\n");
	// Jump to the true main menu
	active_menu = MAIN_MENU;
	ACTIVATE_MENU_MSG();
	(*menufunc_ptrs[active_menu]) (mn_msg);
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
		debug("Trigger Event 1\n");
#if 0 // fix_ns8100
		if (trig_rec.op_mode == WAVEFORM_MODE)
			MoveWaveformEventToFlash();
		else if (trig_rec.op_mode == COMBO_MODE)
			MoveComboWaveformEventToFlash();
#endif
	}

	if (getSystemEventState(MANUEL_CAL_EVENT))
	{
	    debug("Cal Pulse Event\n");
#if 0 // fix_ns8100
	    MoveManuelCalToFlash();
#endif
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
		CalculateBargraphData();
	}

	if (getSystemEventState(COMBO_EVENT))
	{
		clearSystemEventFlag(COMBO_EVENT);
		CalculateComboData();
	}

	if (getSystemEventState(WARNING1_EVENT))
	{
		debug("Warning Event 1\n");
		clearSystemEventFlag(WARNING1_EVENT);

#if 0 // fix_ns8100
		// Activate alarm 1 signal
		reg_TIM2PORT.reg |= 0x04;
#endif
		// Assign soft timer to turn the Alarm 1 signal off
		assignSoftTimer(ALARM_ONE_OUTPUT_TIMER_NUM, (uint32)(help_rec.alarm_one_time * 2), alarmOneOutputTimerCallback);
	}

	if (getSystemEventState(WARNING2_EVENT))
	{
		debug("Warning Event 2\n");
		clearSystemEventFlag(WARNING2_EVENT);

#if 0 // fix_ns8100
		// Activate alarm 2 signal
		reg_TIM2PORT.reg |= 0x08;
#endif
		// Assign soft timer to turn the Alarm 2 signal off
		assignSoftTimer(ALARM_TWO_OUTPUT_TIMER_NUM, (uint32)(help_rec.alarm_two_time * 2), alarmTwoOutputTimerCallback);
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

		active_menu = RESULTS_MENU;

		if (trig_rec.op_mode == MANUAL_CAL_MODE)
		{
			RESULTS_MENU_MANUEL_CAL_MSG();
		}
		else
		{
			RESULTS_MENU_MONITORING_MSG();
		}
		(*menufunc_ptrs[active_menu]) (mn_msg);
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
		active_menu = MONITOR_MENU;
		ACTIVATE_MENU_WITH_DATA_MSG(trig_rec.op_mode);
		(*menufunc_ptrs[active_menu]) (mn_msg);
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

#if 0 // fix_ns8100
	// Check if there is an modem present and if the connection state is connected
	if ((MODEM_CONNECTED == READ_DSR) && (CONNECTED == g_ModemStatus.connectionState))
	{
		// Check if the connection has been lost
		if (NO_CONNECTION == READ_DCD)
		{
			// Check if a ring indicator has been received
			if ((g_ModemStatus.ringIndicator != READ_RI) && (READ_RI == RING))
			{
				// Check if nine rings have been received
				if (g_ModemStatus.numberOfRings++ > 9)
				{
					// Reset flag
					g_ModemStatus.numberOfRings = 0;

					// Assign timer to process modem init
					assignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_QUICK_DELAY, modemDelayTimerCallback);
				}
			}

			// Assign ring indicator to the status of the ring indicator line
			g_ModemStatus.ringIndicator = (uint8)READ_RI;

			// Relock the system
			g_ModemStatus.systemIsLockedFlag = YES;

			if (CONNECTED == g_ModemStatus.firstConnection)
			{
				g_ModemStatus.firstConnection = NOP_CMD;
				assignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_QUICK_DELAY, modemDelayTimerCallback);
			}
		}
		else
		{
			g_ModemStatus.firstConnection = CONNECTED;
		}
	}
	else
	{
		if (	(YES == modem_setup_rec.modemStatus) && (CONNECTED == g_ModemStatus.connectionState))
		{
			g_ModemStatus.connectionState = NOP_CMD;
			g_ModemStatus.systemIsLockedFlag = YES;
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
	if (NOP_CMD != g_ModemStatus.xferState)
	{
		if (DEMx_CMD == g_ModemStatus.xferState)
		{
			//debug("CraftManager:sendDEMData\n");
			g_ModemStatus.xferState = sendDEMData();

			if (NOP_CMD == g_ModemStatus.xferState)
			{
				help_rec.auto_print = g_ModemStatus.xferPrintState;
			}
		}
		else if (DSMx_CMD == g_ModemStatus.xferState)
		{
			//debug("CraftManager:sendDSMData\n");
			g_ModemStatus.xferState = sendDSMData();

			if (NOP_CMD == g_ModemStatus.xferState)
			{
				help_rec.auto_print = g_ModemStatus.xferPrintState;
			}
		}
		else if (DQMx_CMD == g_ModemStatus.xferState)
		{
			//debug("CraftManager:sendDQMData\n");
			g_ModemStatus.xferState = sendDQMData();

			if (NOP_CMD == g_ModemStatus.xferState)
			{
				help_rec.auto_print = g_ModemStatus.xferPrintState;
			}
		}
		else if (VMLx_CMD == g_ModemStatus.xferState)
		{
			sendVMLData();

			if (NOP_CMD == g_ModemStatus.xferState)
			{
				help_rec.auto_print = g_ModemStatus.xferPrintState;
			}
		}
	}

	// Data from the craft port is independent of the modem.
	if (YES == g_ModemStatus.craftPortRcvFlag)
	{
		g_ModemStatus.craftPortRcvFlag = NO;
		processCraftData();
	}

	// Did we raise the craft data port flag, if so process the incomming data.
	if (getSystemEventState(CRAFT_PORT_EVENT))
	{
		clearSystemEventFlag(CRAFT_PORT_EVENT);
		cmdMessageProcessing();
	}
#endif
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

		active_menu = DATE_TIME_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[active_menu]) (mn_msg);
	}
}

/****************************************
*	Function:	handleSystemEvents
*	Purpose:	Handles system events when the main loop can't run (thread locked)
****************************************/
void handleSystemEvents(void)
{
	if (SysEvents_flags.wrd)
	    SystemEventManager();

	if (mn_event_flags.wrd)
	    MenuEventManager();

	MessageManager();
}

/****************************************
*	Function:	main
*	Purpose:	Starting point for the application
****************************************/
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
		if ((SysEvents_flags.wrd == 0x0000) && !(powerManagement.reg & 0x60) &&
			(g_ModemStatus.xferState == NOP_CMD))
		{
			// Sleep
			Wait();
		}
    }

	return (0);
}

