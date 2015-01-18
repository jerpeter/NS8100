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
#include "adc.h"
#include "usb_task.h"
#include "device_mass_storage_task.h"
#include "usb_drv.h"
#include "srec.h"
#include "flashc.h"
#if 0 // Port fat driver
#include "FAT32_Disk.h"
#include "FAT32_Access.h"
#include "FAT32_FileLib.h"
#endif

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

	//-------------------------------------------------------------------------
	// Turn the LCD on
	debug("Init Power on LCD...\r\n");
	PowerControl(LCD_POWER_ENABLE, ON);
	SoftUsecWait(LCD_ACCESS_DELAY);

	//-------------------------------------------------------------------------
	// Load Trig Record 0 to get the last settings
	debug("Loading Monitor & Trigger record\r\n");
	GetRecordData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

	// Check if the Default Trig Record is uninitialized
	if ((g_triggerRecord.op_mode != WAVEFORM_MODE) && (g_triggerRecord.op_mode != BARGRAPH_MODE) &&
		(g_triggerRecord.op_mode != COMBO_MODE))
	{
		debugWarn("Monitor & Trigger Record: Operation Mode not set\r\n");
		debug("Monitor & Trigger Record: Loading defaults and setting mode to Waveform\r\n");
		LoadTrigRecordDefaults((REC_EVENT_MN_STRUCT*)&g_triggerRecord, WAVEFORM_MODE);
	}
	else
	{
		debug("Loading Monitor & Trigger record is valid\r\n");

		// Make sure record is marked valid
		g_triggerRecord.validRecord = YES;
	}

	//-------------------------------------------------------------------------
	// Load the Unit Config
	debug("Loading Unit Config record\r\n");
	GetRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);

	// Check if the Unit Config is uninitialized
	if (g_unitConfig.validationKey != 0xA5A5)
	{
		// Set defaults in Unit Config
		debugWarn("Unit Config: Not found.\r\n");
		debug("Loading Unit Config Defaults\r\n");
		LoadUnitConfigDefaults((UNIT_CONFIG_STRUCT*)&g_unitConfig);
		SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
	}
	else
	{
		// Unit Config is valid
		debug("Unit Config record is valid\r\n");

		if ((g_unitConfig.pretrigBufferDivider != PRETRIGGER_BUFFER_QUARTER_SEC_DIV) && (g_unitConfig.pretrigBufferDivider != PRETRIGGER_BUFFER_HALF_SEC_DIV) &&
		(g_unitConfig.pretrigBufferDivider != PRETRIGGER_BUFFER_FULL_SEC_DIV))
		{
			g_unitConfig.pretrigBufferDivider = PRETRIGGER_BUFFER_QUARTER_SEC_DIV;
			SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
		}

#if 0 // Moved this init to the hardware section to allow for the saved Baud rate to be established from the start
		// Set the baud rate to the user stored baud rate setting (initialized to 115200)
		switch (g_unitConfig.baudRate)
		{
			case BAUD_RATE_57600: rs232Options.baudrate = 57600; break;
			case BAUD_RATE_38400: rs232Options.baudrate = 38400; break;
			case BAUD_RATE_19200: rs232Options.baudrate = 19200; break;
			case BAUD_RATE_9600: rs232Options.baudrate = 9600; break;
		}

		if (g_unitConfig.baudRate != BAUD_RATE_115200)
		{
			// Re-Initialize the RS232 with the stored baud rate
			usart_init_rs232(&AVR32_USART1, &rs232Options, FOSC0);
		}
#endif
	}

	//-------------------------------------------------------------------------
	// Build the language table based on the user's last language choice
	debug("Init Build Language Table...\r\n");
	BuildLanguageLinkTable(g_unitConfig.languageMode);

	//-------------------------------------------------------------------------
	// Initialize Unit Config items such as contrast, timers
	debug("Init Activate Unit Config Options...\r\n");
	ActivateUnitConfigOptions();

	//-------------------------------------------------------------------------
	// Wait 1/2 second for the LCD power to settle
	SoftUsecWait(500 * SOFT_MSECS);

	//-------------------------------------------------------------------------
	// Display the Splash screen
	debug("Display Splash screen\r\n");
	DisplaySplashScreen();

	//-------------------------------------------------------------------------
	// Wait at least 3 seconds for the main screen to be displayed
	SoftUsecWait(3 * SOFT_SECS);

	//-------------------------------------------------------------------------
	// Check if auto print is enabled
	if (g_unitConfig.autoPrint == YES)
	{
		// Disable Auto printing
		g_unitConfig.autoPrint = NO;
	}

	//-------------------------------------------------------------------------
	// Load the Factory Setup Record
	debug("Loading Factory setup record\r\n");
	GetRecordData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);

	// Check if the Factory Setup Record is valid
	if (!g_factorySetupRecord.invalid)
	{
		// Print the Factory Setup Record to the console
		memset(&buff[0], 0, sizeof(buff));
		ConvertTimeStampToString(buff, &g_factorySetupRecord.cal_date, REC_DATE_TIME_TYPE);

		if (g_factorySetupRecord.sensor_type == SENSOR_ACC) { strcpy((char*)&g_spareBuffer, "Acc"); }
		else { sprintf((char*)&g_spareBuffer, "%3.1f in", (float)g_factorySetupRecord.sensor_type / (float)204.8); }

		// Check if an older unit doesn't have the Analog Channel Config set
		if ((g_factorySetupRecord.analogChannelConfig != CHANNELS_R_AND_V_SCHEMATIC) && (g_factorySetupRecord.analogChannelConfig != CHANNELS_R_AND_V_SWAPPED))
		{
			// Set the default
			g_factorySetupRecord.analogChannelConfig = CHANNELS_R_AND_V_SCHEMATIC;
		}

		debug("Factory Setup: Serial #: %s\r\n", g_factorySetupRecord.serial_num);
		debug("Factory Setup: Cal Date: %s\r\n", buff);
		debug("Factory Setup: Sensor Type: %s\r\n", (char*)g_spareBuffer);
		debug("Factory Setup: A-Weighting: %s\r\n", (g_factorySetupRecord.aweight_option == YES) ? "Enabled" : "Disabled");
		debug("Factory Setup: Analog Channel Config: %s\r\n", (g_factorySetupRecord.analogChannelConfig == CHANNELS_R_AND_V_SCHEMATIC) ? "Schematic" : "Swapped");
	}
	else // Factory Setup Record is not found or invalid
	{
		// Warn the user
		debugWarn("Factory setup record not found.\r\n");
		OverlayMessage(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), (2 * SOFT_SECS));
	}

	//-------------------------------------------------------------------------
	// Load the Modem Setup Record
	debug("Load Modem Setup record\r\n");
	GetRecordData(&g_modemSetupRecord, 0, REC_MODEM_SETUP_TYPE);

	// Check if the Modem Setup Record is invalid
	if (g_modemSetupRecord.invalid)
	{
		// Warn the user
		debugWarn("Modem setup record not found.\r\n");

		// Initialize the Modem Setup Record
		LoadModemSetupRecordDefaults();

		// Save the Modem Setup Record
		SaveRecordData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
	}
	else
	{
		debug("Modem Setup record is valid\r\n");

		ValidateModemSetupParameters();
	}

	//-------------------------------------------------------------------------
	// Add OnOff Log Timestamp
	debug("Adding On/Off Log timestamp\r\n");
	AddOnOffLogTimestamp(ON);
	
#if 0 // Removed debug log file due to inducing system problems
	//-------------------------------------------------------------------------
	// Switch Debug Log file
	SwitchDebugLogFile();
#endif
	
	//-------------------------------------------------------------------------
	// Init Global Unique Event Number
	debug("Init Current Event Number...\r\n");
	InitCurrentEventNumber();

	//-------------------------------------------------------------------------
	// Init AutoDialout
	debug("Init Auto Dialout...\r\n");
	InitAutoDialout();

	//-------------------------------------------------------------------------
	// Init Monitor Log
	debug("Init Monitor Log...\r\n");
	InitMonitorLog();

	//-------------------------------------------------------------------------
	// Init Flash Buffers
	debug("Init Flash Buffers...\r\n");
	InitFlashBuffs();

	//-------------------------------------------------------------------------
	// Init the sensor parameters
	debug("Init Sensor Parameters...\r\n");
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
	g_sensorInfoPtr->unitsFlag = g_unitConfig.unitsOfMeasure;			// 0 = SAE; 1 = Metric

	g_sensorInfoPtr->sensorAccuracy = SENSOR_ACCURACY_100X_SHIFT;		// 100, sensor values are X 100 for accuaracy.
	g_sensorInfoPtr->ADCResolution = ADC_RESOLUTION;				// Raw data Input Range, unless ADC is changed

	// Get the shift value
	g_sensorInfoPtr->shiftVal = 1;

	g_sensorInfoPtr->sensorTypeNormalized = (float)(sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_100X_SHIFT);

	if((IMPERIAL_TYPE == g_unitConfig.unitsOfMeasure) || (sensor_type == SENSOR_ACC))
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

	//debug("Address of local variable: 0x%x\r\n", &mn_msg);

	debug("Init Software Settings\r\n");

	//-------------------------------------------------------------------------
	// Init version msg
	//-------------------------------------------------------------------------
	InitVersionMsg();

	//-------------------------------------------------------------------------
	// Init time msg and update current time
	//-------------------------------------------------------------------------
	InitTimeMsg();
	UpdateCurrentTime();

	//-------------------------------------------------------------------------
	// Get the function address passed by the bootloader
	//-------------------------------------------------------------------------
	CheckBootloaderAppPresent();

#if 0 // Moved to hardware init
#if NS8100_ALPHA
	//-------------------------------------------------------------------------
	// Init the NAV and select the SD MMC Card
	//-------------------------------------------------------------------------
	nav_reset();
	nav_select(FS_NAV_ID_DEFAULT);
	nav_drive_set(0);
	nav_partition_mount();
#endif
#endif
	//-------------------------------------------------------------------------
	// Setup defaults, load records, init the language table
	//-------------------------------------------------------------------------
	debug("Setup Menu Defaults...\r\n");
	setupMnDef();

	//-------------------------------------------------------------------------
	// Update Flash Usage Stats
	//-------------------------------------------------------------------------
	debug("Updating Flash Usage Stats...\r\n");
	OverlayMessage(getLangText(STATUS_TEXT), "CALCULATING EVENT STORAGE SPACE FREE", 0);
	GetSDCardUsageStats();

	//-------------------------------------------------------------------------
	// Check for Timer mode activation
	//-------------------------------------------------------------------------
	if (TimerModeActiveCheck() == TRUE)
	{
		debug("--- Timer Mode Startup ---\r\n");

		ProcessTimerMode();

		// If here, the unit is in Timer mode, but did not power itself off yet
		// Enabling power off protection
#if 0 // Test with power off protection always enabled
		debug("Timer Mode: Enabling Power Off Protection\r\n");
		PowerControl(POWER_OFF_PROTECTION_ENABLE, ON);
#endif
	}
	else
	{
		debug("--- Normal Startup ---\r\n");
	}

	//-------------------------------------------------------------------------
	// Init the cmd message handling buffers before initialization of the ports.
	//-------------------------------------------------------------------------
	debug("Init Cmd Msg Handler...\r\n");
	RemoteCmdMessageHandlerInit();

	//-------------------------------------------------------------------------
	// Init the input buffers and status flags for input craft data.
	//-------------------------------------------------------------------------
	debug("Init Craft Init Status Flags...\r\n");
	CraftInitStatusFlags();

#if 0 // Add ability to check sensor if one is available
	//-------------------------------------------------------------------------
	// Overlay a message to alert the user that the system is checking the sensors
	//-------------------------------------------------------------------------
	debug("Init LCD Message...\r\n");
	sprintf((char*)g_spareBuffer, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	OverlayMessage(getLangText(STATUS_TEXT), g_spareBuffer, 0);
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
	debug("Init complete, turning Kepypad LED Green...\r\n");
	WriteMcp23018(IO_ADDRESS_KPD, GPIOA, ((ReadMcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | GREEN_LED_PIN));

	//-------------------------------------------------------------------------
	// Assign a one second keypad led update timer
	//-------------------------------------------------------------------------
	AssignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, KeypadLedUpdateTimerCallBack);

	//-------------------------------------------------------------------------
	// Jump to the true main menu
	//-------------------------------------------------------------------------
	debug("Jump to Main Menu\r\n");
	SETUP_MENU_MSG(MAIN_MENU);
	JUMP_TO_ACTIVE_MENU();
}

