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
///	Unit Type
///----------------------------------------------------------------------------
// Change the value of SUPERGRAPH_UNIT to select type
// 1 = Supergraph unit
// 0 = Minigraph unit
#define SUPERGRAPH_UNIT		0

// This will auto-select the Minigraph setting based on the Supergraph
// Do not modify these lines!!!
#if SUPERGRAPH_UNIT
#define MINIGRAPH_UNIT		0
#else
#define MINIGRAPH_UNIT		1
#endif

// Print Specific Flags
#define DISABLE_DEBUG_PRINTING	0
#define MODEM_DEBUG_TEST_FLAG	0

// Test Exception Handling
#define TEST_EXCEPTION_HANDLING	1

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define SOFT_VERSION  		"1.11"
#define SOFT_DATE     		"6-22-2004"
#define SOFT_TIME     		"08:35pm"

// Define core clock rate
#if 1 // Normal
#define FOSC0	66000000
#else
#define FOSC0	12000000
#endif

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
	uint8 clk_12_24;
	uint8 hour;
	uint8 min;
	uint8 sec;
	uint8 hundredths;
	// Additional fields added to make the structure even and long bounded (even div by 4)
	uint8 unused1;
	uint8 unused2;
	uint8 valid;
} DATE_TIME_STRUCT;

typedef struct
{
	uint16 upper;
	uint16 lower;
} HALFS;

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

#define SECS_PER_DAY			86400
#define SECS_PER_HOUR			3600
#define SECS_PER_MIN			60

#define PI	3.14159

#define JUMP_TO_BOOT	2

#define LANGUAGE_TABLE_MAX_SIZE		8192

// To eliminate C warning when the variable is not used.
#define UNUSED(p) ((void)p)

#define	DB_CONVERSION_VALUE			5000000
#if 0 // Port mistake?
#define MB_CONVERSION_VALUE			1
#else // Corrected value
#define MB_CONVERSION_VALUE			400
#define ADJUSTED_MB_TO_HEX_VALUE	25
#endif

enum {
	INPUT_BUFFER_EMPTY = 0,
	INPUT_BUFFER_NOT_EMPTY
};

#define INPUT_BUFFER_SIZE  		3

enum {
	DATA_NORMALIZED = 0,
	DATA_NOT_NORMALIZED
};

// Channel type definitions
#define RADIAL_CHANNEL_TYPE 	10
#define VERTICAL_CHANNEL_TYPE 	11
#define TRANSVERSE_CHANNEL_TYPE 12
#define ACOUSTIC_CHANNEL_TYPE 	13

#define CAL_PULSE_FIXED_SAMPLE_RATE		SAMPLE_RATE_1K
#define CALIBRATION_FIXED_SAMPLE_RATE	SAMPLE_RATE_1K

typedef enum {
	TC_SAMPLE_TIMER_CHANNEL = 0,
	TC_CALIBRATION_TIMER_CHANNEL = 1,
	TC_TYPEMATIC_TIMER_CHANNEL = 2
} TC_CHANNEL_NUM;

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

enum {
	EXT_CHARGE_VOLTAGE,
	BATTERY_VOLTAGE
};

#define VIN_CHANNEL     2
#define VBAT_CHANNEL    3

#define LOW_VOLTAGE_THRESHOLD		5.4
#define EXTERNAL_VOLTAGE_PRESENT	5.0

/* Uart Info */
#define CRAFT_BAUDRATE	115200 //14400 //38400
#define CRAFT_COM_PORT	0

#define RS485_BAUDRATE	2000000 //19200
#define RS485_COM_PORT	1

#if NS8100_ALPHA
// Define Debug port
#define DEBUG_BAUDRATE	115200
#define DEBUG_COM_PORT	2

// Define Project Debug Port
#define GLOBAL_DEBUG_PRINT_PORT	DEBUG_COM_PORT
#endif

#define GLOBAL_DEBUG_BUFFER_THRESHOLD	8000

/* Battery Level defines */
#define BATT_MIN_VOLTS 			4.0

#if NS8100_ORIGINAL
#define REFERENCE_VOLTAGE       (float)3.3
#else // NS8100_ALPHA
#define REFERENCE_VOLTAGE       (float)2.5
#endif
#define BATT_RESOLUTION         (float)1024  // 10-bit resolution

#define VOLTAGE_RATIO_BATT			(float)3
#define VOLTAGE_RATIO_EXT_CHARGE    (float)16.05

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
	POWER_SAVINGS_MOST,
	POWER_SAVINGS_MAX
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
// Battery routines
float GetExternalVoltageLevelAveraged(uint8 type);
BOOLEAN CheckExternalChargeVoltagePresent(void);
//uint8 AdjustedRawBatteryLevel(void);

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
float HexToDB(uint16, uint8, uint16);
float HexToMB(uint16, uint8, uint16);
float HexToPsi(uint16, uint8, uint16);
uint16 DbToHex(uint16);
uint16 MbToHex(float);
uint32 ConvertDBtoMB(uint32);
uint32 ConvertMBtoDB(uint32);

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
void ByteCpy(void* dest, void* src, uint32 size);
void ByteSet(void* dest, uint8 value, uint32 size);

// Main menu prototype extensions
void HandleSystemEvents(void);

// Time routines
uint8 GetDayOfWeek(uint8 year, uint8 month, uint8 day);
uint16 GetTotalDaysFromReference(TM_DATE_STRUCT date);
void GetDateString(char*, uint8, uint8);
uint8 GetDaysPerMonth(uint8, uint16);
void InitTimeMsg(void);

#endif // _COMMON_H_
