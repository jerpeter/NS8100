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
#define FOSC0	66000000

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

#define LOW_VOLTAGE_THRESHOLD	5.0

/* Uart Info */
#define CRAFT_BAUDRATE	115200 //14400 //38400
#define CRAFT_COM_PORT	0
#define RS485_BAUDRATE	2000000 //19200
#define RS485_COM_PORT	1

/* Battery Level defines */
#define BATT_MIN_VOLTS 			4.0

#define REFERENCE_VOLTAGE       (float)3.3
#define BATT_RESOLUTION         (float)1024  // 10-bit resolution

#if 0 // ns7100
#define BATT_RESISTOR_RATIO     	((604 + 301)/301)
#define EXT_CHARGE_RESISTOR_RATIO	((3000 + 200)/200)
#else // ns8100
#define VOLTAGE_RATIO_BATT			(float)3
#define VOLTAGE_RATIO_EXT_CHARGE    (float)16.05
#endif

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

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
// Battery routines
float getExternalVoltageLevelAveraged(uint8 type);
BOOLEAN checkExternalChargeVoltagePresent(void);
//uint8 adjustedRawBatteryLevel(void);

// Math routines
uint16 isqrt (uint32 x);

// Input message routines
uint16 ckInputMsg(INPUT_MSG_STRUCT *);
void procInputMsg (INPUT_MSG_STRUCT);
uint16 sendInputMsg(INPUT_MSG_STRUCT *);

// Delay timing
void soft_usecWait(uint32 usecs);
void spinBar(void);

// Conversion routines
uint16 swapInt(uint16);
float hexToDB(uint16, uint8, uint16);
float hexToMB(uint16, uint8, uint16);
float hexToPsi(uint16, uint8, uint16);
uint16 dbToHex(uint16);
uint16 mbToHex(float);
uint32 convertDBtoMB(uint32);
uint32 convertMBtoDB(uint32);

// PIT timers
void startPitTimer(PIT_TIMER timer);
void stopPitTimer(PIT_TIMER timer);
BOOLEAN checkPitTimer(PIT_TIMER timer);
void configPitTimer(PIT_TIMER timer, uint16 clockDivider, uint16 modulus);

// Language translation
void build_languageLinkTable(uint8 languageSelection);

// Version routine
//void initVersionStrings(void);
void initVersionMsg(void);

// Bootloader Function
void getBootFunctionAddress(void);
void jumpToBootFunction(void);
void byteCpy(void* dest, void* src, uint32 size);
void byteSet(void* dest, uint8 value, uint32 size);

// Main menu prototype extensions
void handleSystemEvents(void);

// Time routines
uint8 getDayOfWeek(uint8 year, uint8 month, uint8 day);
uint16 getTotalDaysFromReference(TM_DATE_STRUCT date);
void getDateString(char*, uint8, uint8);
uint8 getDaysPerMonth(uint8, uint16);
void initTimeMsg(void);

#endif // _COMMON_H_
