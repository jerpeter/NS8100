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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void setupMnDef(void)
{
	char buff[50];

	debug("Init Power on LCD...\n");

	// Turn the LCD on
	PowerControl(LCD_POWER_ENABLE, ON);

	debug("Init Load Trig Rec...\n");

	// Load Trig Record 0 to get the last settings
	debug("Trigger Record: Loading stored settings into cache\n");
	GetRecordData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

	debug("Init Trig Rec Defaults...\n");

	// Check if the Default Trig Record is uninitialized
	if ((g_triggerRecord.op_mode != WAVEFORM_MODE) && (g_triggerRecord.op_mode != BARGRAPH_MODE) &&
	(g_triggerRecord.op_mode != COMBO_MODE))
	{
		debugWarn("Trigger Record: Operation Mode not set\n");
		debug("Trigger Record: Loading defaults and setting mode to Waveform\n");
		LoadTrigRecordDefaults((REC_EVENT_MN_STRUCT*)&g_triggerRecord, WAVEFORM_MODE);
	}
	else
	{
		// Make sure record is marked valid
		g_triggerRecord.validRecord = YES;
	}

	debug("Init Load Help Rec...\n");

	// Load the Help Record
	GetRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

	debug("Init Help Rec Defaults...\n");

	// Check if the Help Record is uninitialized
	if (g_helpRecord.validationKey != 0xA5A5)
	{
		// Set defaults in Help Record
		debugWarn("Help record: Not found.\n");
		debug("Loading Help Menu Defaults\n");
		LoadHelpRecordDefaults((REC_HELP_MN_STRUCT*)&g_helpRecord);
		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}
	else
	{
		// Help Record is valid
		debug("Help record: Found.\n");

		if ((g_helpRecord.pretrigBufferDivider != PRETRIGGER_BUFFER_QUARTER_SEC_DIV) && (g_helpRecord.pretrigBufferDivider != PRETRIGGER_BUFFER_HALF_SEC_DIV) &&
		(g_helpRecord.pretrigBufferDivider != PRETRIGGER_BUFFER_FULL_SEC_DIV))
		{
			g_helpRecord.pretrigBufferDivider = PRETRIGGER_BUFFER_QUARTER_SEC_DIV;
			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
		}

		#if 0 // Moved this init to the hardware section to allow for the saved Baud rate to be established from the start
		// Set the baud rate to the user stored baud rate setting (initialized to 115200)
		switch (g_helpRecord.baudRate)
		{
			case BAUD_RATE_57600: rs232Options.baudrate = 57600; break;
			case BAUD_RATE_38400: rs232Options.baudrate = 38400; break;
			case BAUD_RATE_19200: rs232Options.baudrate = 19200; break;
			case BAUD_RATE_9600: rs232Options.baudrate = 9600; break;
		}

		if (g_helpRecord.baudRate != BAUD_RATE_115200)
		{
			// Re-Initialize the RS232 with the stored baud rate
			usart_init_rs232(&AVR32_USART1, &rs232Options, FOSC0);
		}
		#endif
	}

	debug("Init Build Language Table...\n");

	// Build the language table based on the user's last language choice
	BuildLanguageLinkTable(g_helpRecord.languageMode);

	debug("Init Activate Help Rec Options...\n");

	debug("Activate help record items\n");
	// Initialize Help Record items such as contrast, timers
	ActivateHelpRecordOptions();

	// Wait 1/2 second for the LCD power to settle
	SoftUsecWait(500 * SOFT_MSECS);

	debug("Display Splash screen\n");
	// Display the Splash screen
	DisplaySplashScreen();

	// Wait at least 3 seconds for the main screen to be displayed
	SoftUsecWait(3 * SOFT_SECS);

#if 0 // Test - Failed
	ClearControlLinesLcdDisplay();
	LcdClearPortReg();
#endif

	// Check if the unit is set for Mini and auto print is enabled
	if ((MINIGRAPH_UNIT) && (g_helpRecord.autoPrint == YES))
	{
		// Disable Auto printing
		g_helpRecord.autoPrint = NO;
		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}

	debug("Init Load Factory Setup Rec...\n");

	debug("Load Factory setup record\n");
	// Load the Factory Setup Record
	GetRecordData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);

	// Check if the Factory Setup Record is valid
	if (!g_factorySetupRecord.invalid)
	{
		// Print the Factory Setup Record to the console
		ByteSet(&buff[0], 0, sizeof(buff));
		ConvertTimeStampToString(buff, &g_factorySetupRecord.cal_date, REC_DATE_TIME_TYPE);

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
		//MessageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
	}

	debug("Init Load Modem Setup Rec...\n");

	debug("Load Modem Setup record\n");
	// Load the Modem Setup Record
	GetRecordData(&g_modemSetupRecord, 0, REC_MODEM_SETUP_TYPE);

	// Check if the Modem Setup Record is invalid
	if (g_modemSetupRecord.invalid)
	{
		// Warn the user
		debugWarn("Modem setup record not found.\n");

		// Initialize the Modem Setup Record
		LoadModemSetupRecordDefaults();

		// Save the Modem Setup Record
		SaveRecordData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
	}
	else
	{
		ValidateModemSetupParameters();
	}

	debug("Init Current Event Number...\n");

	// Init Global Unique Event Number
	InitCurrentEventNumber();

	debug("Init Auto Dialout...\n");

	// Init AutoDialout
	InitAutoDialout();

	debug("Init Monitor Log...\n");

	// Init Monitor Log
	InitMonitorLog();

	debug("Init Flash Buffers...\n");

	// Init Flash Buffers
	InitFlashBuffs();

	debug("Init Sensor Parameters...\n");

	InitSensorParameters(g_factorySetupRecord.sensor_type, (uint8)g_triggerRecord.srec.sensitivity);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitSensorParameters(uint16 sensor_type, uint8 sensitivity)
{
	uint8 gainFactor = (uint8)((sensitivity == LOW) ? 2 : 4);

	// Sensor type information
	g_sensorInfoPtr->numOfChannels = NUMBER_OF_CHANNELS_DEFAULT;		// The number of channels from a sensor.
	g_sensorInfoPtr->unitsFlag = g_helpRecord.unitsOfMeasure;			// 0 = SAE; 1 = Metric

	g_sensorInfoPtr->sensorAccuracy = SENSOR_ACCURACY_100X_SHIFT;		// 100, sensor values are X 100 for accuaracy.
	g_sensorInfoPtr->ADCResolution = ADC_RESOLUTION;				// Raw data Input Range, unless ADC is changed

	// Get the shift value
	g_sensorInfoPtr->shiftVal = 1;

	g_sensorInfoPtr->sensorTypeNormalized = (float)(sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_100X_SHIFT);

	if((IMPERIAL_TYPE == g_helpRecord.unitsOfMeasure) || (sensor_type == SENSOR_ACC))
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "navigation.h"
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
	InitVersionMsg();

	//-------------------------------------------------------------------------
	// Init time msg
	//-------------------------------------------------------------------------
	debug("Init Time Msg...\n");
	InitTimeMsg();

	//-------------------------------------------------------------------------
	// Load current time into cache
	//-------------------------------------------------------------------------
	UpdateCurrentTime();

	//-------------------------------------------------------------------------
	// Get the function address passed by the bootloader
	//-------------------------------------------------------------------------
	debug("Init Get Boot Function Addr...\n");
	GetBootFunctionAddress();

#if NS8100_ALPHA
	//-------------------------------------------------------------------------
	// Init the NAV and select the SD MMC Card
	//-------------------------------------------------------------------------
	nav_reset();
	nav_select(0);
	nav_drive_set(0);
#endif

	//-------------------------------------------------------------------------
	// Setup defaults, load records, init the language table
	//-------------------------------------------------------------------------
	debug("Init Setup Menu Defaults...\n");
	setupMnDef();

	//-------------------------------------------------------------------------
	// Check for Timer mode activation
	//-------------------------------------------------------------------------
	debug("Init Timer Mode Check...\n");
	if (TimerModeActiveCheck() == TRUE)
	{
		debug("--- Timer Mode Startup ---\n");

		ProcessTimerMode();

		// If here, the unit is in Timer mode, but did not power itself off yet
		// Enabling power off protection
		debug("Timer Mode: Enabling Power Off Protection\n");
		PowerControl(POWER_OFF_PROTECTION_ENABLE, ON);
	}
	else
	{
		debug("--- Normal Startup ---\n");
	}

	//-------------------------------------------------------------------------
	// Init the cmd message handling buffers before initialization of the ports.
	//-------------------------------------------------------------------------
	debug("Init Cmd Msg Handler...\n");
	RemoteCmdMessageHandlerInit();

	//-------------------------------------------------------------------------
	// Init the input buffers and status flags for input craft data.
	//-------------------------------------------------------------------------
	debug("Init Craft Init Status Flags...\n");
	CraftInitStatusFlags();

#if 0 // ns7100
	// Overlay a message to alert the user that the system is checking the sensors
	debug("Init LCD Message...\n");
	char buff[50];
	ByteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	OverlayMessage(getLangText(STATUS_TEXT), buff, 0);
	#endif

	//-------------------------------------------------------------------------
	// Signal remote end that RS232 Comm is available
	//-------------------------------------------------------------------------
	SET_RTS; SET_DTR; // Init signals for ready to send and terminal ready

	//-------------------------------------------------------------------------
	// Reset LCD timers
	//-------------------------------------------------------------------------
	ResetSoftTimer(DISPLAY_ON_OFF_TIMER_NUM);
	ResetSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM);

	//-------------------------------------------------------------------------
	// Turn on the Green keypad LED when system init complete
	//-------------------------------------------------------------------------
	debug("Init complete, turning Kepypad LED Green...\n");
	WriteMcp23018(IO_ADDRESS_KPD, GPIOA, ((ReadMcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | GREEN_LED_PIN));

	//-------------------------------------------------------------------------
	// Assign a one second keypad led update timer
	//-------------------------------------------------------------------------
	AssignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, KeypadLedUpdateTimerCallBack);

	//-------------------------------------------------------------------------
	// Jump to the true main menu
	//-------------------------------------------------------------------------
	debug("Jump to Main Menu\n");
	SETUP_MENU_MSG(MAIN_MENU);
	JUMP_TO_ACTIVE_MENU();
}

