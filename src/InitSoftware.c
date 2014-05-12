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
#include "craft.h"
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
#include "Old_Board.h"
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

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

extern void InitKeypad(void);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

/****************************************
*	Function:	setupMnDef
*	Purpose:
****************************************/
void setupMnDef(void)
{
	char buff[50];
	#if 0 // Moved this init to the hardware section to allow for the saved Baud rate to be established from the start
	usart_options_t rs232Options =
	{ .charlength = 8, .paritytype = USART_NO_PARITY, .stopbits = USART_1_STOPBIT, .channelmode = USART_NORMAL_CHMODE };
	#endif

	debug("Init Power on LCD...\n");

	// Turn the LCD on
	powerControl(LCD_POWER_ENABLE, ON);

	debug("Init Load Trig Rec...\n");

	// Load Trig Record 0 to get the last settings
	debug("Trigger Record: Loading stored settings into cache\n");
	getRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

	debug("Init Trig Rec Defaults...\n");

	// Check if the Default Trig Record is uninitialized
	if ((g_triggerRecord.op_mode != WAVEFORM_MODE) && (g_triggerRecord.op_mode != BARGRAPH_MODE) &&
	(g_triggerRecord.op_mode != COMBO_MODE))
	{
		debugWarn("Trigger Record: Operation Mode not set\n");
		debug("Trigger Record: Loading defaults and setting mode to Waveform\n");
		loadTrigRecordDefaults((REC_EVENT_MN_STRUCT*)&g_triggerRecord, WAVEFORM_MODE);
	}
	else
	{
		// Make sure record is marked valid
		g_triggerRecord.validRecord = YES;
	}

	debug("Init Load Help Rec...\n");

	// Load the Help Record
	getRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

	debug("Init Help Rec Defaults...\n");

	// Check if the Help Record is uninitialized
	if (g_helpRecord.encode_ln != 0xA5A5)
	{
		// Set defaults in Help Record
		debugWarn("Help record: Not found.\n");
		debug("Loading Help Menu Defaults\n");
		loadHelpRecordDefaults((REC_HELP_MN_STRUCT*)&g_helpRecord);
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}
	else
	{
		// Help Record is valid
		debug("Help record: Found.\n");

		if ((g_helpRecord.pretrig_buffer_div != PRETRIGGER_BUFFER_QUARTER_SEC_DIV) && (g_helpRecord.pretrig_buffer_div != PRETRIGGER_BUFFER_HALF_SEC_DIV) &&
		(g_helpRecord.pretrig_buffer_div != PRETRIGGER_BUFFER_FULL_SEC_DIV))
		{
			g_helpRecord.pretrig_buffer_div = PRETRIGGER_BUFFER_QUARTER_SEC_DIV;
			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
		}

		#if 0 // Moved this init to the hardware section to allow for the saved Baud rate to be established from the start
		// Set the baud rate to the user stored baud rate setting (initialized to 115200)
		switch (g_helpRecord.baud_rate)
		{
			case BAUD_RATE_57600: rs232Options.baudrate = 57600; break;
			case BAUD_RATE_38400: rs232Options.baudrate = 38400; break;
			case BAUD_RATE_19200: rs232Options.baudrate = 19200; break;
			case BAUD_RATE_9600: rs232Options.baudrate = 9600; break;
		}

		if (g_helpRecord.baud_rate != BAUD_RATE_115200)
		{
			// Re-Initialize the RS232 with the stored baud rate
			usart_init_rs232(&AVR32_USART1, &rs232Options, FOSC0);
		}
		#endif
	}

	debug("Init Build Language Table...\n");

	// Build the language table based on the user's last language choice
	build_languageLinkTable(g_helpRecord.lang_mode);

	debug("Init Activate Help Rec Options...\n");

	debug("Activate help record items\n");
	// Initialize Help Record items such as contrast, timers
	activateHelpRecordOptions();

	// Wait 1/2 second for the LCD power to settle
	soft_usecWait(500 * SOFT_MSECS);

	debug("Display Splash screen\n");
	// Display the Splash screen
	displaySplashScreen();

	// Wait at least 3 seconds for the main screen to be displayed
	soft_usecWait(3 * SOFT_SECS);

	// Check if the unit is set for Mini and auto print is enabled
	if ((MINIGRAPH_UNIT) && (g_helpRecord.auto_print == YES))
	{
		// Disable Auto printing
		g_helpRecord.auto_print = NO;
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}

	debug("Init Load Factory Setup Rec...\n");

	debug("Load Factory setup record\n");
	// Load the Factory Setup Record
	getRecData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);

	// Check if the Factory Setup Record is valid
	if (!g_factorySetupRecord.invalid)
	{
		// Print the Factory Setup Record to the console
		byteSet(&buff[0], 0, sizeof(buff));
		convertTimeStampToString(buff, &g_factorySetupRecord.cal_date, REC_DATE_TIME_TYPE);

		debug("Factory Setup: Serial #: %s\n", g_factorySetupRecord.serial_num);
		debug("Factory Setup: Cal Date: %s\n", buff);
		debug("Factory Setup: Sensor Type: %d %s\n", (g_factorySetupRecord.sensor_type),
		((g_factorySetupRecord.sensor_type == SENSOR_ACC) ? "(Acc)" : ""));
		debug("Factory Setup: A-Weighting: %s\n", (g_factorySetupRecord.aweight_option == YES) ? "Enabled" : "Disabled");
	}
	else // Factory Setup Record is not found or invalid
	{
		// Warn the user
		debugWarn("Factory setup record not found.\n");
		//messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
	}

	debug("Init Load Modem Setup Rec...\n");

	debug("Load Modem Setup record\n");
	// Load the Modem Setup Record
	getRecData(&g_modemSetupRecord, 0, REC_MODEM_SETUP_TYPE);

	// Check if the Modem Setup Record is invalid
	if (g_modemSetupRecord.invalid)
	{
		// Warn the user
		debugWarn("Modem setup record not found.\n");

		// Initialize the Modem Setup Record
		loadModemSetupRecordDefaults();

		// Save the Modem Setup Record
		saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
	}
	else
	{
		validateModemSetupParameters();
	}

	debug("Init Current Event Number...\n");

	// Init Global Unique Event Number
	initCurrentEventNumber();

	debug("Init Auto Dialout...\n");

	// Init AutoDialout
	initAutoDialout();

	debug("Init Monitor Log...\n");

	// Init Monitor Log
	initMonitorLog();

	debug("Init Flash Buffers...\n");

	// Init Flash Buffers
	InitFlashBuffs();

	debug("Init Sensor Parameters...\n");

	initSensorParameters(g_factorySetupRecord.sensor_type, (uint8)g_triggerRecord.srec.sensitivity);
}

/****************************************
*	Function:  initSensorParameters()
*	Purpose:
****************************************/
void initSensorParameters(uint16 sensor_type, uint8 sensitivity)
{
	uint8 gainFactor = (uint8)((sensitivity == LOW) ? 2 : 4);

	// Sensor type information
	g_sensorInfoPtr->numOfChannels = NUMBER_OF_CHANNELS_DEFAULT;		// The number of channels from a sensor.
	g_sensorInfoPtr->unitsFlag = g_helpRecord.units_of_measure;			// 0 = SAE; 1 = Metric

	g_sensorInfoPtr->sensorAccuracy = SENSOR_ACCURACY_100X_SHIFT;		// 100, sensor values are X 100 for accuaracy.
	g_sensorInfoPtr->ADCResolution = ADC_RESOLUTION;				// Raw data Input Range, unless ADC is changed

	// Get the shift value
	g_sensorInfoPtr->shiftVal = 1;

	g_sensorInfoPtr->sensorTypeNormalized = (float)(sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_100X_SHIFT);

	if((IMPERIAL_TYPE == g_helpRecord.units_of_measure) || (sensor_type == SENSOR_ACC))
	{
		g_sensorInfoPtr->measurementRatio = (float)IMPERIAL; 				// 1 = SAE; 25.4 = Metric
	}
	else
	{
		g_sensorInfoPtr->measurementRatio = (float)METRIC; 				// 1 = SAE; 25.4 = Metric
	}

	// Get the sensor type in terms of the measurement units.
	g_sensorInfoPtr->sensorTypeNormalized = (float)(g_sensorInfoPtr->sensorTypeNormalized) * (float)(g_sensorInfoPtr->measurementRatio);

	// the conversion is length(in or mm) = hexValue * (sensor scale/ADC Max Value)
	g_sensorInfoPtr->hexToLengthConversion = (float)( (float)ADC_RESOLUTION / (float)g_sensorInfoPtr->sensorTypeNormalized );

	g_sensorInfoPtr->sensorValue = (uint16)(g_factorySetupRecord.sensor_type / gainFactor); // sensor value X 100.
}

//=================================================================================================
//	Function:	InitSoftwareSettings_NS8100
//=================================================================================================
void InitSoftwareSettings_NS8100(void)
{
	INPUT_MSG_STRUCT mn_msg;

	//debug("Address of local variable: 0x%x\n", &mn_msg);

	debug("Init Software Settings\n");
	debug("Init Version Strings...\n");

	//-------------------------------------------------------------------------
	// Init version strings
	//-------------------------------------------------------------------------
	#if 0 // ns7100
	initVersionStrings();
	#endif

	//-------------------------------------------------------------------------
	// Init version msg
	//-------------------------------------------------------------------------
	debug("Init Version Msg...\n");
	initVersionMsg();

	//-------------------------------------------------------------------------
	// Init time msg
	//-------------------------------------------------------------------------
	debug("Init Time Msg...\n");
	initTimeMsg();

	//-------------------------------------------------------------------------
	// Load current time into cache
	//-------------------------------------------------------------------------
	updateCurrentTime();

	//-------------------------------------------------------------------------
	// Get the function address passed by the bootloader
	//-------------------------------------------------------------------------
	debug("Init Get Boot Function Addr...\n");
	getBootFunctionAddress();

	//-------------------------------------------------------------------------
	// Setup defaults, load records, init the language table
	//-------------------------------------------------------------------------
	debug("Init Setup Menu Defaults...\n");
	setupMnDef();

	#if 1 // fix_ns8100
	// Have to recall Keypad init otherwise interrupt hangs (Following code needs keypad access)
	InitKeypad();
	#endif

	//-------------------------------------------------------------------------
	// Check for Timer mode activation
	//-------------------------------------------------------------------------
	debug("Init Timer Mode Check...\n");
	if (timerModeActiveCheck() == TRUE)
	{
		debug("--- Timer Mode Startup ---\n");
		processTimerMode();

		// If here, the unit is in Timer mode, but did not power itself off yet
		// Enabling power off protection
		debug("Timer Mode: Enabling Power Off Protection\n");
		powerControl(POWER_OFF_PROTECTION_ENABLE, ON);
	}
	else
	{
		debug("--- Normal Startup ---\n");
	}

	//-------------------------------------------------------------------------
	// Init the cmd message handling buffers before initialization of the ports.
	//-------------------------------------------------------------------------
	debug("Init Cmd Msg Handler...\n");
	cmdMessageHandlerInit();

	//-------------------------------------------------------------------------
	// Init the input buffers and status flags for input craft data.
	//-------------------------------------------------------------------------
	debug("Init Craft Init Status Flags...\n");
	craftInitStatusFlags();

	#if 0 // ns7100
	// Overlay a message to alert the user that the system is checking the sensors
	debug("Init LCD Message...\n");
	char buff[50];
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	overlayMessage(getLangText(STATUS_TEXT), buff, 0);
	#endif

	//-------------------------------------------------------------------------
	// Signal remote end that RS232 Comm is available
	//-------------------------------------------------------------------------
	SET_RTS; SET_DTR; // Init signals for ready to send and terminal ready

	//-------------------------------------------------------------------------
	// Reset LCD timers
	//-------------------------------------------------------------------------
	resetSoftTimer(DISPLAY_ON_OFF_TIMER_NUM);
	resetSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM);

	//-------------------------------------------------------------------------
	// Turn on the Green keypad LED when system init complete
	//-------------------------------------------------------------------------
	write_mcp23018(IO_ADDRESS_KPD, GPIOA, ((read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | GREEN_LED_PIN));

	//-------------------------------------------------------------------------
	// Assign a one second keypad led update timer
	//-------------------------------------------------------------------------
	assignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, keypadLedUpdateTimerCallBack);

	//-------------------------------------------------------------------------
	// Jump to the true main menu
	//-------------------------------------------------------------------------
	debug("Jump to Main Menu\n");
	SETUP_MENU_MSG(MAIN_MENU);
	JUMP_TO_ACTIVE_MENU();
}

