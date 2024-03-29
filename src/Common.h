///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _COMMON_H_
#define _COMMON_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "board.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define SOFT_VERSION	"1.11"
#define SOFT_DATE		"6-22-2004"
#define SOFT_TIME		"08:35pm"

#define SYSTEM_PATH		"A:\\System\\"
#define EVENTS_PATH		"A:\\Events\\"
#define ER_DATA_PATH	"A:\\ERData\\"
#define LANGUAGE_PATH	"A:\\Language\\"
#define LOGS_PATH		"A:\\Logs\\"

#define EVT_FILE		"Evt"
#define EVTS_SUB_DIR	"Evts"

#define SUMMARY_LIST_FILE			"SummaryList.bin"
#define MONITOR_LOG_BIN_FILE		"MonitorLog.ns8"
#define MONITOR_LOG_READABLE_FILE	"MonitorLogReadable.txt"
#define ON_OFF_READABLE_FILE		"OnOffLogReadable.txt"
#define EXCEPTION_REPORT_FILE		"ExceptionReport.txt"

#define MAX_FILE_NAME_CHARS		255
#define MAX_TEXT_LINE_CHARS		255
#define MAX_BASE_PATH_CHARS		20

enum {
	EVENT_FILE_TYPE = 1,
	ER_DATA_FILE_TYPE
};

// Define core clock rate
#if 1 // Normal
#define FOSC0	66000000
#else
#define FOSC0	12000000
#endif

#define INTERNAL_SAMPLING_SOURCE	NO
// Make sure choice is mutually exclusive
#if (!INTERNAL_SAMPLING_SOURCE)
#define EXTERNAL_SAMPLING_SOURCE	YES
#else
#define EXTERNAL_SAMPLING_SOURCE	NO
#endif

#define VT_FEATURE_DISABLED			NO

#define FLASH_USER_PAGE_BASE_ADDRESS	(0x80800000)
#define SHADOW_FACTORY_SETUP_CLEARED	(*(uint16*)FLASH_USER_PAGE_BASE_ADDRESS)
#define GET_HARDWARE_ID					(*(uint8*)(0x8080000C)) // Factory setup location of Hardware ID
#define GET_BUILD_ID					(*(uint8*)(0x8080000D)) // Factory setup location of Build ID

enum {
	HARDWARE_ID_REV_8_NORMAL = 0x08,
	HARDWARE_ID_REV_8_WITH_GPS_MOD = 0x28,
	HARDWARE_ID_REV_8_WITH_USART = 0x18,
};

#define PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP	if ((AVR32_FLASHC.fsr + *(uint16*)0xD0000000 + AVR32_PM.gplp[0] == 0)) { UNUSED(AVR32_PM.gplp[0]); } else { UNUSED(AVR32_PM.gplp[0]); }

#define DISABLED_BUT_FIX_FOR_NS8100	0

typedef enum
{
	KEYPAD_TIMER,
	SAMPLE_TIMER
} PIT_TIMER;

typedef struct
{
	uint8 year;
	uint8 month;
	uint8 day;
	uint8 weekday;
	uint8 unused0;
	uint8 hour;
	uint8 min;
	uint8 sec;
	uint8 hundredths;
	// Additional fields added to make the structure even and long bounded (even div by 4)
	uint8 unused1;
	uint8 unused2;
	uint8 valid;
} DATE_TIME_STRUCT;

typedef struct {
	uint16 year;
	uint8 month;
	uint8 day;
} CALIBRATION_DATE_STRUCT;

typedef union {
	uint32 epochDate;
	CALIBRATION_DATE_STRUCT normalDate;
	uint8 rawDate[4];
} CALIBRATION_DATE_UNIVERSAL_STRUCT;

typedef struct
{
	CALIBRATION_DATE_STRUCT date;
	uint8 source;
	uint8 unused;
} WORKING_CAL_DATE_STRUCT;

typedef struct
{
	uint32 cmd;
	uint32 length;
	uint32 data[6];
} INPUT_MSG_STRUCT;

typedef struct
{
	uint8 hour;
	uint8 min;
	uint8 sec;
} TM_TIME_STRUCT;

typedef struct
{
	uint8 day;
	uint8 month;
	uint8 year;
} TM_DATE_STRUCT;

#define TEN_MSEC				1					// # of counts per msec
#define SECND 					(10 * TEN_MSEC)		// # of ten_msec per second

#define SOFT_MSECS				1000 				// Scale usecs to msecs
#define SOFT_SECS				(1000 * 1000) 		// Scale usecs to secs

#define EXCEPTION_HANDLING_USE_SOFT_DELAY_KEY	(0xFFFFFF00)
#define EXCEPTION_MSG_DISPLAY_TIME				(8)	// Seconds

#define SECS_PER_DAY			86400
#define SECS_PER_HOUR			3600
#define SECS_PER_MIN			60

#define PI	3.14159

enum {
	JUMP_TO_BOOT = 2,
	OFF_EXCEPTION,
	FORCED_OFF
};

#define LANGUAGE_TABLE_MAX_SIZE		16384

// To eliminate C warning when the variable is not used.
#define UNUSED(p) ((void)p)

#define	DB_CONVERSION_VALUE			5000000
#define MB_CONVERSION_VALUE			400
#define ADJUSTED_MB_TO_HEX_VALUE		1.5625 // Was 25 @ 12-bit, new value @ 16-bit
#define ADJUSTED_MB_IN_PSI_TO_HEX_VALUE	105.20565

enum {
	INPUT_BUFFER_EMPTY = 0,
	INPUT_BUFFER_NOT_EMPTY
};

#define INPUT_BUFFER_SIZE	3

enum {
	DATA_NORMALIZED = 0,
	DATA_NOT_NORMALIZED
};

enum {
	SAVE_EXTRA_FILE_COMPRESSED_DATA = 1,
	DO_NOT_SAVE_EXTRA_FILE_COMPRESSED_DATA
};

enum {
	QUICK_BOOT_ENTRY_FROM_MENU = 1,
	QUICK_BOOT_ENTRY_FROM_SERIAL
};

// Channel type definitions
#define RADIAL_CHANNEL_TYPE 	10
#define VERTICAL_CHANNEL_TYPE 	11
#define TRANSVERSE_CHANNEL_TYPE 12
#define ACOUSTIC_CHANNEL_TYPE 	13

#define CAL_PULSE_FIXED_SAMPLE_RATE		SAMPLE_RATE_1K
#define CALIBRATION_FIXED_SAMPLE_RATE	SAMPLE_RATE_1K

#if INTERNAL_SAMPLING_SOURCE
typedef enum {
	TC_SAMPLE_TIMER_CHANNEL = 0,
	TC_CALIBRATION_TIMER_CHANNEL = 1,
	TC_TYPEMATIC_TIMER_CHANNEL = 2
} TC_CHANNEL_NUM;
#else // EXTERNAL_SAMPLING_SOURCE
typedef enum {
	TC_SAMPLE_TIMER_CHANNEL = 0,
	TC_MILLISECOND_TIMER_CHANNEL = 1,
	TC_TYPEMATIC_TIMER_CHANNEL = 2
} TC_CHANNEL_NUM;
#endif

enum {
	SEISMIC_GROUP_1 = 1,
	SEISMIC_GROUP_2
};

enum {
	SYSTEM_EVENT = 0,
	TIMER_EVENT,
	MENU_EVENT,
	TOTAL_EVENTS
};

enum {
	RAISE_FLAG = 1,
	CLEAR_FLAG
};

#define USBM_RI_8507_DRYWALL_STANDARD	1
#define USBM_RI_8507_PLASTER_STANDARD	2
#define OSM_REGULATIONS_STANDARD		3
#define END_OF_VIBRATION_STANDARDS_LIST	4
#define START_OF_CUSTOM_CURVES_LIST		9
#define CUSTOM_STEP_THRESHOLD			10
#define CUSTOM_STEP_LIMITING			11
#define END_OF_VIBRATION_CURVES_LIST	12

enum {
	EXTERNAL_TRIGGER_EVENT = 1,
	VARIABLE_TRIGGER_EVENT
};

enum {
	EXT_CHARGE_VOLTAGE,
	BATTERY_VOLTAGE
};

enum {
	USB_SYNC_NORMAL = 1,
	USB_SYNC_FROM_SHELL
};

#define VIN_CHANNEL		2
#define VBAT_CHANNEL	3

#define LOW_VOLTAGE_THRESHOLD		5.4
#define EXTERNAL_VOLTAGE_PRESENT	5.0

#define CYCLIC_EVENT_TIME_THRESHOLD		(4 * 2)
#define UPDATE_TIME_EVENT_THRESHOLD		(60 * 2)

/* Uart Info */
#define CRAFT_BAUDRATE	115200 //14400 //38400
#define CRAFT_COM_PORT	0

#define RS485_BAUDRATE	2000000 //19200
#define RS485_COM_PORT	1

#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
// Define Debug port
#define DEBUG_BAUDRATE	115200
#define DEBUG_COM_PORT	2

// Define Project Debug Port
#define GLOBAL_DEBUG_PRINT_PORT	DEBUG_COM_PORT //CRAFT_COM_PORT
#endif

#define TOTAL_UNIQUE_EVENT_NUMBERS		(65535)
#define EVENT_NUMBER_CACHE_MAX_ENTRIES	(TOTAL_UNIQUE_EVENT_NUMBERS + 1)
#define DER_CACHE_SIZE					(65536)

#define SPARE_BUFFER_SIZE				8192

/* Battery Level defines */
#define BATT_MIN_VOLTS 			4.0

#if NS8100_ORIGINAL_PROTOTYPE
#define REFERENCE_VOLTAGE		(float)3.3
#else // (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
#define REFERENCE_VOLTAGE		(float)2.5
#endif
#define BATT_RESOLUTION			(float)1024 // 10-bit resolution

#define VOLTAGE_RATIO_BATT			(float)3
#define VOLTAGE_RATIO_EXT_CHARGE	(float)16.05

enum {
	KEYPAD_LED_STATE_UNKNOWN = 0,
	KEYPAD_LED_STATE_BOTH_OFF,
	KEYPAD_LED_STATE_IDLE_GREEN_ON,
	KEYPAD_LED_STATE_CHARGE_RED_ON,
	KEYPAD_LED_STATE_ACTIVE_GREEN_ON,
	KEYPAD_LED_STATE_ACTIVE_GREEN_OFF,
	KEYPAD_LED_STATE_ACTIVE_CHARGE_GREEN_ON,
	KEYPAD_LED_STATE_ACTIVE_CHARGE_RED_ON
};

enum {
	POWER_SAVINGS_NONE = 0,
	POWER_SAVINGS_MINIMUM,
	POWER_SAVINGS_NORMAL,
	POWER_SAVINGS_MOST,
	POWER_SAVINGS_MAX
};

enum USB_STATES {
	USB_INIT_DRIVER,
	USB_NOT_CONNECTED,
	USB_READY,
	USB_CONNECTED_AND_PROCESSING,
	USB_HOST_MODE_WAITING_FOR_DEVICE,
	USB_DISABLED_FOR_OTHER_PROCESSING,
	USB_DEVICE_MODE_SELECTED,
	USB_HOST_MODE_SELECTED
};

enum USB_SYNC_FILE_EXISTS_ACTIONS {
	PROMPT_OPTION = 0,
	SKIP_OPTION,
	REPLACE_OPTION,
	DUPLICATE_OPTION,
	SKIP_ALL_OPTION,
	REPLACE_ALL_OPTION,
	DUPLICATE_ALL_OPTION
};

#if NS8100_BETA_PROTOTYPE
enum SDMMC_CARD_DETECT_STATE {
	SDMMC_CARD_DETECTED = 0,
	SDMMC_CARD_NOT_PRESENT
};
#elif NS8100_ALPHA_PROTOTYPE
enum SDMMC_CARD_DETECT_STATE {
	SDMMC_CARD_NOT_PRESENT = 0,
	SDMMC_CARD_DETECTED
};
#endif

enum {
	BP_UNHANDLED_INT = 1,
	BP_SOFT_LOOP,
	BP_MB_LOOP,
	BP_INT_MEM_CORRUPTED,
	BP_AD_CHAN_SYNC_ERR,
	BP_END
};

typedef enum {
	AVAILABLE = 1,
	EVENT_LOCK,
	CAL_PULSE_LOCK,
	SDMMC_LOCK,
	RTC_TIME_LOCK,
	EEPROM_LOCK
} SPI1_LOCK_TYPE;

typedef struct
{
	uint16 freq_count;
	uint16 peak;
	uint16* peakSamplePtr;
} VARIABLE_TRIGGER_FREQ_CHANNEL_BUFFER;

typedef struct
{
	VARIABLE_TRIGGER_FREQ_CHANNEL_BUFFER r[2];
	VARIABLE_TRIGGER_FREQ_CHANNEL_BUFFER v[2];
	VARIABLE_TRIGGER_FREQ_CHANNEL_BUFFER t[2];
	uint16 r_sign;
	uint16 v_sign;
	uint16 t_sign;
} VARIABLE_TRIGGER_FREQ_CALC_BUFFER;

enum {
	GPS_MSG_START = 1,
	GPS_MSG_BODY,
	GPS_MSG_CHECKSUM_FIRST_NIBBLE,
	GPS_MSG_CHECKSUM_SECOND_NIBBLE,
	GPS_MSG_END,
	GPS_BINARY_MSG_START,
	GPS_BINARY_MSG_START_END,
	GPS_BINARY_MSG_PAYLOAD_START,
	GPS_BINARY_MSG_PAYLOAD,
	GPS_BINARY_MSG_BODY,
	GPS_BINARY_MSG_CHECKSUM,
	GPS_BINARY_MSG_END
};

#define GPS_SERIAL_BUFFER_SIZE		500
#define TOTAL_GPS_MESSAGES	5
#define TOTAL_GPS_BINARY_MESSAGES	10
#define GPS_MESSAGE_SIZE	100
#define GPS_THRESHOLD_TOTAL_FIXES_FOR_BEST_LOCATION		(2 * 30) // 30 seconds of GGA and GLL location fixes
#define GPS_ACTIVE_LOCATION_SEARCH_TIME		10
#define GPS_REACTIVATION_TIME_SEARCH_FAIL	50
#define GPS_REACTIVATION_TIME_NORMAL		60

#define GPS_POWER_NORMAL_SAVE_POWER		0
#define GPS_POWER_ALWAYS_ON_ACQUIRING	1

typedef struct {
	uint8* readPtr;
	uint8* writePtr;
	uint8* endPtr;
	uint8 buffer[GPS_SERIAL_BUFFER_SIZE];
	uint8 ready;
	uint8 state;
	uint8 binaryState;
} GPS_SERIAL_DATA;

typedef struct {
	uint8 data[GPS_MESSAGE_SIZE];
	uint8 checksum;
} GPS_MESSAGE;

typedef struct {
	uint8 binMsgValid;
	uint8 binMsgSize;
	uint8 data[GPS_MESSAGE_SIZE];
} GPS_BINARY_MESSAGE;

typedef struct {
	uint8 readIndex;
	uint8 writeIndex;
	uint8 endIndex;
	uint8 messageReady;
	GPS_MESSAGE message[TOTAL_GPS_MESSAGES];
} GPS_QUEUE;

typedef struct {
	uint8 readIndex;
	uint8 writeIndex;
	uint8 endIndex;
	uint8 binaryMessageReady;
	GPS_BINARY_MESSAGE message[TOTAL_GPS_BINARY_MESSAGES];
} GPS_BINARY_QUEUE;

typedef struct {
	uint32 acquisitionStartTicks;
	uint32 acqTickAccumulation;
	uint16 totalAcquisitions;
	uint16 failedAcquisitions;
	uint16 retryForPositionTimeoutInSeconds;
	uint8 positionFound;
} GPS_INFO;

enum {
	GGA = 0,
	GLL,
	ZDA,
	TOTAL_GPS_COMMANDS
};

enum {
	GPS_BIN_MSG_ACK = 0,
	GPS_BIN_MSG_NACK,
	VERSION_QUERY,
	SOFT_CRC_QUERY,
	NMEA_MSG_INTERVAL_QUERY,
	GPS_TIME_QUERY,
	TOTAL_GPS_BINARY_COMMANDS
};

typedef struct
{
	int     year;       /**< Years since 1900 */
	int     mon;        /**< Months since January - [0,11] */
	int     day;        /**< Day of the month - [1,31] */
	int     hour;       /**< Hours since midnight - [0,23] */
	int     min;        /**< Minutes after the hour - [0,59] */
	int     sec;        /**< Seconds after the minute - [0,59] */
	int     hsec;       /**< Hundredth part of second - [0,99] */
} nmeaTIME;

typedef struct
{
	uint8 latDegrees;
	uint8 latMinutes;
	uint16 latSeconds;
	uint8 longDegrees;
	uint8 longMinutes;
	uint16 longSeconds;
	char northSouth;
	char eastWest;
	uint8 utcHour;
	uint8 utcMin;
	uint8 utcSec;
	uint8 validLocationCount;
	uint8 locationFoundWhileMonitoring;
	uint8 positionFix;
	int16 altitude;
	uint16 utcYear;
	uint8 utcMonth;
	uint8 utcDay;
} GPS_POSITION;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
// Gps routines
void InitGpsBuffers(void);
void EnableGps(void);
void DisableGps(void);
uint8 GpsChecksum(uint8* message);
void ProcessGpsSerialData(void);
void ProcessGpsMessage(void);
void ProcessGpsBinaryMessage(void);
void GpsQueryVersion(void);
void GpsQueryTime(void);
//void GpsQueryUTCDate(void);
void GpsQuerySoftCrc(void);
void GpsQueryNmeaMsgInterval(void);
void GpsSendBinaryMessage(uint8* binaryMessage, uint16 messageLength);
uint8 GpsCalcBinaryChecksum(uint8* binaryPayload, uint16 payloadLength);
void HandleVersionQuery(void);
void HandleSoftCrcQuery(void);
void HandleNmeaMsgIntervalQuery(void);
void HandleGPSQueryTime(void);
void HandleUTCDateQuery(void);
void GpsChangeNmeaMsgInterval(uint8 GGAInt, uint8 GSAInt, uint8 GSVInt, uint8 GLLInt, uint8 RMCInt, uint8 VTGInt, uint8 ZDAInt);
void HandleBinaryMsgAck(void);
void HandleBinaryMsgNack(void);
void GpsChangeSerialBaud(void);

// Battery routines
float GetExternalVoltageLevelAveraged(uint8 type);
BOOLEAN CheckExternalChargeVoltagePresent(void);

// Power Savings
void AdjustPowerSavings(void);

// Math routines
uint16 Isqrt (uint32 x);

// Input message routines
uint16 CheckInputMsg(INPUT_MSG_STRUCT *);
void ProcessInputMsg (INPUT_MSG_STRUCT);
uint16 SendInputMsg(INPUT_MSG_STRUCT *);

// Delay timing
void SoftUsecWait(uint32 usecs);
void SpinBar(void);

// Conversion routines
uint16 SwapInt(uint16);
float HexToDB(uint16, uint8, uint16, uint8);
float HexToMB(uint16, uint8, uint16, uint8);
float HexToPSI(uint16, uint8, uint16, uint8);
uint16 DbToHex(uint32, uint8);
uint16 MbToHex(uint32, uint8);
uint16 PsiToHex(uint32, uint8);

// PIT timers
void startPitTimer(PIT_TIMER timer);
void stopPitTimer(PIT_TIMER timer);
BOOLEAN checkPitTimer(PIT_TIMER timer);
void configPitTimer(PIT_TIMER timer, uint16 clockDivider, uint16 modulus);

// Language translation
void BuildLanguageLinkTable(uint8 languageSelection);

// Version routine
//void initVersionStrings(void);
void InitVersionMsg(void);

// Bootloader Function
void CheckBootloaderAppPresent(void);

// Main menu prototype extensions
void HandleSystemEvents(void);
void BootLoadManager(void);
void SystemEventManager(void);
void MenuEventManager(void);
void CraftManager(void);
void GpsManager(void);
void MessageManager(void);
void FactorySetupManager(void);
void UsbDeviceManager(void);
void UsbDisableIfActive(void);
void CheckExceptionReportLogExists(void);

// Init Hardware prototype extensions
void InitSystemHardware_NS8100(void);
void InitGps232(void);

// Init Interrupts prototype extensions
void InitInterrupts_NS8100(void);
void Setup_8100_EIC_External_RTC_ISR(void);
void Setup_8100_EIC_Keypad_ISR(void);
void Setup_8100_EIC_System_ISR(void);
void Setup_8100_Soft_Timer_Tick_ISR(void);
void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM);
void Setup_8100_Usart1_RS232_ISR(void);
void Setup_8100_Usart0_RS232_ISR(void);

// Init Software prototype extensions
void InitSoftwareSettings_NS8100(void);

// ISRs prototype extensions
void DataIsrInit(uint16 sampleRate);
void Eic_low_battery_irq(void);
void Eic_keypad_irq(void);
void Eic_system_irq(void);
void Eic_external_rtc_irq(void);
void Gps_status_irq(void);
void Tc_sample_irq(void);
void Usart_1_rs232_irq(void);
void Usart_0_rs232_irq(void);
void Soft_timer_tick_irq(void);
void Tc_typematic_irq(void);
void Start_Data_Clock(TC_CHANNEL_NUM);
void Stop_Data_Clock(TC_CHANNEL_NUM);
void HandleActiveAlarmExtension(void);
void SensorCalibrationDataInit(void);
void ProcessSensorCalibrationData(void);

#if EXTERNAL_SAMPLING_SOURCE
void Tc_ms_timer_irq(void);
#endif

// Process Handler prototype extensions
void StartDataCollection(uint32 sampleRate);
void GetManualCalibration(void);
void HandleCycleChangeEvent(void);

// Keypad prototype extensions
void InitKeypad(void);

// Time routines
uint8 GetDayOfWeek(uint8 year, uint8 month, uint8 day);
uint16 GetTotalDaysFromReference(TM_DATE_STRUCT date);
void GetDateString(char*, uint8, uint8);
uint8 GetDaysPerMonth(uint8, uint16);
void InitTimeMsg(void);
void CheckForCycleChange(void);

// Error routines
void ReportFileSystemAccessProblem(char*);
void ReportFileAccessProblem(char* attemptedFile);

// CalDate/DateTime conversions
void ConvertDateTimeToCalDate(CALIBRATION_DATE_STRUCT* calDate, DATE_TIME_STRUCT* dateTime);
void ConvertCalDatetoDateTime(DATE_TIME_STRUCT* dateTime, CALIBRATION_DATE_STRUCT* calDate);

// Spi 1 Mutex Access
void GetSpi1MutexLock(SPI1_LOCK_TYPE spi1LockType);
void ReleaseSpi1MutexLock(void);

// Validate trigger source
uint8 CheckTriggerSourceExists(void);

// Process the USB Core routines
void ProcessUsbCoreHandling(void);

#endif // _COMMON_H_
