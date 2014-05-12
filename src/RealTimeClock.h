///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RealTimeClock.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:57 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RealTimeClock.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _RTC_H_
#define _RTC_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Common.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define TOTAL_MONTHS	12

// Months
enum {
	JAN = 1,
	FEB, //	2
	MAR, //	3
	APR, //	4
	MAY, //	5
	JUN, //	6
	JUL, //	7
	AUG, //	8
	SEP, //	9
	OCT, //	10
	NOV, //	11
	DEC //	12
};

// Days
enum {
	SUN = 0,
	MON, // 1
	TUE, // 2
	WED, // 3
	THU, // 4
	FRI, // 5
	SAT  // 6
};

typedef struct
{
	uint32 monthNumber;
	uint8 name[4];
	uint32 days;
} MONTH_TABLE_STRUCT;

// Alarm Frequency
enum {
	ONCE_PER_SECOND = 0,
	ONCE_PER_MINUTE_WHEN_SECONDS_MATCH,
	ONCE_PER_HOUR_WHEN_MINUTES_AND_SECONDS_MATCH,
	ONCE_PER_DAY_WHEN_HOURS_MINUTES_AND_SECONDS_MATCH,
	WHEN_DAY_HOURS_MINUTES_AND_SECONDS_MATCH
};

#if 0 // ns7100
// Periodic Interrupt frequency
enum {
	NO_PERIODIC_INT = 0,	// 0x00, Disabled
	PULSE_32K_PER_SEC,		// 0x01, 30.5175us
	PULSE_16K_PER_SEC,		// 0x02, 61.035us
	PULSE_8K_PER_SEC,		// 0x03, 122.070us
	PULSE_4K_PER_SEC,		// 0x04, 244.141us
	PULSE_2K_PER_SEC,		// 0x05, 488.281us
	PULSE_1K_PER_SEC,		// 0x06, 976.5625us
	PULSE_512_PER_SEC,		// 0x07, 1.95315ms
	PULSE_256_PER_SEC,		// 0x08, 3.90625ms
	PULSE_128_PER_SEC,		// 0x09, 7.8125ms
	PULSE_64_PER_SEC,		// 0x0A, 16.625ms
	PULSE_32_PER_SEC,		// 0x0B, 31.25ms
	PULSE_16_PER_SEC,		// 0x0C, 62.5ms
	PULSE_8_PER_SEC,		// 0x0D, 125ms
	PULSE_4_PER_SEC,		// 0x0E, 250ms
	PULSE_2_PER_SEC			// 0x0F, 500ms
};

// Watchdog Timeout rate
enum {
	WATCHDOG_TIMOUT_1 = 0,	// 0x00, 1.5s (watchdog), 0.25s (reset)
	WATCHDOG_TIMOUT_2,		// 0x01, 23.4375ms (watchdog), 3.9063ms (reset)
	WATCHDOG_TIMOUT_3,		// 0x02, 46.875ms (watchdog), 7.8125ms (reset)
	WATCHDOG_TIMOUT_4,		// 0x03, 93.750ms (watchdog), 15.625ms (reset)
	WATCHDOG_TIMOUT_5,		// 0x04, 175.5ms (watchdog), 31.25ms (reset)
	WATCHDOG_TIMOUT_6,		// 0x05, 375ms (watchdog), 62.5ms (reset)
	WATCHDOG_TIMOUT_7,		// 0x06, 750ms (watchdog), 125ms (reset)
	WATCHDOG_TIMOUT_8		// 0x07, 3.0s (watchdog), 0.5s (reset)
};

typedef struct
{
	uint8 mode;
	uint8 secondsAlarmMask;
	uint8 minutesAlarmMask;
	uint8 hoursAlarmMask;
	uint8 dayAlarmMask;
} RTC_ALARM_FREQ_STRUCT;

// Set Update Transfer Inhibit bit in Control register
#define RTC_FREEZE_UPDATES (RTC_CONTROL.bit.transferInhibitEnable = ON)

// Clear Update Transfer Inhibit bit in Control register
#define RTC_UNFREEZE_UPDATES (RTC_CONTROL.bit.transferInhibitEnable = OFF)

// ===========================
// RTC Registers
// ===========================

// ---------------------------
// Seconds register
// Address: 0x0
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:   	1;
		bitfield tenSecond:	3;
		bitfield oneSecond:	4;
	} bit;

	uint8 reg;
} RTC_SECONDS_STRUCT;

// ---------------------------
// Seconds Alarm register
// RTC Address: 0x1
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:   	1;
		bitfield tenSecond:	3;
		bitfield oneSecond:	4;
	} bit;

	struct
	{
		bitfield alarmMask:	2;
		bitfield unused:	6;
	} alarmBit;

   uint8 reg;
} RTC_SECONDS_ALARM_STRUCT;

// ---------------------------
// Minutes register
// RTC Address: 0x2
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:   	1;
		bitfield tenMinute:	3;
		bitfield oneMinute:	4;
	} bit; 

	uint8 reg;
} RTC_MINUTES_STRUCT;

// ---------------------------
// Minutes Alarm register
// RTC Address: 0x3
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:   	1;
		bitfield tenMinute:	3;
		bitfield oneMinute:	4;
	} bit; 

	struct
	{
		bitfield alarmMask:	2;
		bitfield unused:	6;
	} alarmBit;

	uint8 reg;
} RTC_MINUTES_ALARM_STRUCT;

// ---------------------------
// Hours register
// RTC Address: 0x4
// ---------------------------
typedef union
{
	struct
	{
		bitfield am_pm:  	1;
		bitfield unused: 	1;
		bitfield tenHour:	2;
		bitfield oneHour:	4;
	} bit; 

	uint8 reg;
} RTC_HOURS_STRUCT;

// ---------------------------
// Hours Alarm register
// RTC Address: 0x5
// ---------------------------
typedef union
{
	struct
	{
		bitfield am_pm:  	1;
		bitfield unused: 	1;
		bitfield tenHour:	2;
		bitfield oneHour:	4;
	} bit; 

	struct
	{
		bitfield alarmMask:	2;
		bitfield unused:	6;
	} alarmBit;

	uint8 reg;
} RTC_HOURS_ALARM_STRUCT;

// ---------------------------
// Day register
// RTC Address: 0x6
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:	2;
		bitfield tenDay:	2;
		bitfield oneDay:	4;
	} bit; 

	uint8 reg;
} RTC_DAY_STRUCT;

// ---------------------------
// Day Alarm register
// RTC Address: 0x7
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:   	2;
		bitfield tenDay:	2;
		bitfield oneDay:	4;
	} bit; 

	struct
	{
		bitfield alarmMask:	2;
		bitfield unused:	6;
	} alarmBit;

	uint8 reg;
} RTC_DAY_ALARM_STRUCT;

// ---------------------------
// Day of Week register
// RTC Address: 0x8
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:	5;
		bitfield dayOfWeek:	3;
	} bit; 

	uint8 reg;
} RTC_DAY_OF_WEEK_STRUCT;

// ---------------------------
// Month register
// RTC Address: 0x9
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:	3;
		bitfield tenMonth:	1;
		bitfield oneMonth:	4;
	} bit; 

	uint8 reg;
} RTC_MONTH_STRUCT;

// ---------------------------
// Year register
// RTC Address: 0xA
// ---------------------------
typedef union
{
	struct
	{
		bitfield tenYear:	4;
		bitfield oneYear:	4;
	} bit; 

	uint8 reg;
} RTC_YEAR_STRUCT;

// ---------------------------
// Rates register
// RTC Address: 0xB
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:				1;
		bitfield watchdogTimeoutRate:	3;	// WD0-2
		bitfield periodicIntRate:		4;	// RS0-3
	} bit; 

	uint8 reg;
} RTC_RATES_STRUCT;

// ---------------------------
// Enables register
// RTC Address: 0xC
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:					4;
		bitfield alarmIntEnable:			1;	// AIE
		bitfield periodicIntEnable:			1;	// PIE
		bitfield powerFailIntEnable:		1;	// PWRIE
		bitfield batteryBackAlarmIntEnable:	1;	// ABE
	} bit; 

	uint8 reg;
} RTC_ENABLES_STRUCT;

// ---------------------------
// Flags register
// RTC Address: 0xD
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:				4;
		bitfield alarmIntFlag:			1;	// AF
		bitfield periodicIntFlag:		1;	// PF
		bitfield powerFailIntFlag:		1;	// PWRF
		bitfield batteryValidFlag:		1;	// BVF
	} bit; 

	uint8 reg;
} RTC_FLAGS_STRUCT;

// ---------------------------
// Control register
// RTC Address: 0xE
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:					4;
		bitfield transferInhibitEnable:		1;	// UTI
		bitfield oscillatorEnable:			1;	// STOP (Active Low)
		bitfield _24_hourEnable:			1;	// 24/12
		bitfield daylightSavingsEnable:		1;	// DSE
	} bit; 

	uint8 reg;
} RTC_CONTROL_STRUCT;

// ---------------------------
// Centruy register
// RTC Address: 0xF
// ---------------------------
typedef union
{
	struct
	{
		bitfield tenCentruy:4;
		bitfield oneCentury:4;
	} bit; 

	uint8 reg;
} RTC_CENTURY_STRUCT;

#if 0 // finish this...
typedef struct
{
	RTC_SECONDS_ALARM_STRUCT 	secondsAlarm;	// Adress 1
	RTC_MINUTES_ALARM_STRUCT	minutesAlarm;	// Adress 3 
	RTC_HOURS_ALARM_STRUCT 		hoursAlarm;		// Adress 5
	RTC_DAY_ALARM_STRUCT		dayAlarm;		// Adress 7
} RTC_MEMORY_MAP_STRUCT;
#endif

// ===========================
// RTC Definitions
// ===========================
// Name		Description
// ----     ---------------------------------------------
// 24/12	24 or 12 hour representation
// ABE		Alarm interrupt enable in battery backup mode
// AF		Alarm interrupt flag
// AIE		Alarm interrupt enable
// ALM0-1	Alarm mask bits
// BVF		Battery valid flag
// DSE		Daylight savings enable
// PF		Periodic interrupt flag
// PIE		Periodic interrupt enable
// AM/PM	PM or AM indication
// PWRF		Power fail interrupt flag
// PWRIE	Power fail interrupt enable
// RS0-3	Periodic interrupt rate
// STOP		Oscillator stop and start (Active low)
// UTI		Update transfer inhibit
// WD0-2	Watchdog timeout rate

// ===========================
// RTC Register Defines
// ===========================
#define RTC_SECONDS 		(*(volatile RTC_SECONDS_STRUCT*) 		(RTC_ADDRESS + (2 * 0x00)))
#define RTC_SECONDS_ALARM 	(*(volatile RTC_SECONDS_ALARM_STRUCT*) 	(RTC_ADDRESS + (2 * 0x01)))
#define RTC_MINUTES 		(*(volatile RTC_MINUTES_STRUCT*) 		(RTC_ADDRESS + (2 * 0x02)))
#define RTC_MINUTES_ALARM 	(*(volatile RTC_MINUTES_ALARM_STRUCT*) 	(RTC_ADDRESS + (2 * 0x03)))
#define RTC_HOURS 			(*(volatile RTC_HOURS_STRUCT*) 			(RTC_ADDRESS + (2 * 0x04)))
#define RTC_HOURS_ALARM 	(*(volatile RTC_HOURS_ALARM_STRUCT*) 	(RTC_ADDRESS + (2 * 0x05)))
#define RTC_DAY 			(*(volatile RTC_DAY_STRUCT*) 			(RTC_ADDRESS + (2 * 0x06)))
#define RTC_DAY_ALARM 		(*(volatile RTC_DAY_ALARM_STRUCT*)		(RTC_ADDRESS + (2 * 0x07)))
#define RTC_DAY_OF_WEEK 	(*(volatile RTC_DAY_OF_WEEK_STRUCT*) 	(RTC_ADDRESS + (2 * 0x08)))
#define RTC_MONTH 			(*(volatile RTC_MONTH_STRUCT*) 			(RTC_ADDRESS + (2 * 0x09)))
#define RTC_YEAR 			(*(volatile RTC_YEAR_STRUCT*) 			(RTC_ADDRESS + (2 * 0x0A)))
#define RTC_RATES 			(*(volatile RTC_RATES_STRUCT*) 			(RTC_ADDRESS + (2 * 0x0B)))
#define RTC_ENABLES 		(*(volatile RTC_ENABLES_STRUCT*) 		(RTC_ADDRESS + (2 * 0x0C)))
#define RTC_FLAGS 			(*(volatile RTC_FLAGS_STRUCT*) 			(RTC_ADDRESS + (2 * 0x0D)))
#define RTC_CONTROL 		(*(volatile RTC_CONTROL_STRUCT*) 		(RTC_ADDRESS + (2 * 0x0E)))
#define RTC_CENTURY			(*(volatile RTC_CENTURY_STRUCT*) 		(RTC_ADDRESS + (2 * 0x0F)))

// ===========================
// RTC Memory Map
// ===========================
typedef struct
{
	RTC_SECONDS_STRUCT			seconds;		// Adress 0
	RTC_SECONDS_ALARM_STRUCT 	secondsAlarm;	// Adress 1
	RTC_MINUTES_STRUCT 			minutes;		// Adress 2
	RTC_MINUTES_ALARM_STRUCT	minutesAlarm;	// Adress 3 
	RTC_HOURS_STRUCT 			hours;			// Adress 4
	RTC_HOURS_ALARM_STRUCT 		hoursAlarm;		// Adress 5
	RTC_DAY_STRUCT				day;			// Adress 6
	RTC_DAY_ALARM_STRUCT		dayAlarm;		// Adress 7
	RTC_DAY_OF_WEEK_STRUCT		dayOfWeek;		// Adress 8
	RTC_MONTH_STRUCT			month;			// Adress 9
	RTC_YEAR_STRUCT				year;			// Adress A 
	RTC_RATES_STRUCT			rates;			// Adress B
	RTC_ENABLES_STRUCT			enables;		// Adress C  
	RTC_FLAGS_STRUCT			flags;			// Adress D
	RTC_CONTROL_STRUCT			control;		// Adress E
	RTC_CENTURY_STRUCT			century;		// Adress F
} RTC_MEMORY_MAP_STRUCT;

#else // ns8100

//=============================================================================
//=== New RTC =================================================================
//=============================================================================

#define RTC_WRITE_CMD   0x20
#define RTC_READ_CMD    0xA0
#define RTC_ACCESS_DELAY	100

#define RTC_CLOCK_STOPPED	0x20
#define RTC_CLOCK_INTEGRITY	0x80
#define RTC_24_HOUR_MODE	0x00
#define RTC_12_HOUR_MODE	0x04
#define RTC_TIMESTAMP_INT_ENABLE	0x04
#define RTC_ALARM_INT_ENABLE		0x02
#define RTC_COUNTDOWN_INT_ENABLE	0x01
#define RTC_BCD_SECONDS_MASK	0x7F
#define RTC_BCD_MINUTES_MASK	0x7F
#define RTC_BCD_HOURS_MASK		0x3F
#define RTC_BCD_DAYS_MASK		0x3F
#define RTC_BCD_WEEKDAY_MASK	0x07
#define RTC_BCD_MONTHS_MASK		0x1F
#define RTC_BCD_YEARS_MASK		0xFF
#define RTC_DISABLE_ALARM		0x80
#define RTC_ENABLE_ALARM		0x00

#define BCD_CONVERT_TO_UINT8(x, filter)	((((x & filter) >> 4) * 10) + (x & 0x0F))
#define UINT8_CONVERT_TO_BCD(x, filter)	((((uint8)(x / 10) << 4) & filter) | ((x % 10) & 0x0F))

// ===========================
// RTC Registers
// ===========================

// ---------------------------
// Control register 1
// Address: 0x0
// ---------------------------
typedef union
{
	struct
	{
		bitfield ext_test:   	1;
		bitfield t:			   	1;
		bitfield stop:		  	1;
		bitfield tsf1:		   	1;
		bitfield por_ovrd:	  	1;
		bitfield format_12_24:	1;
		bitfield mi:			1;
		bitfield si:			1;
	} bit;

	uint8 reg;
} RTC_CONTROL_1_STRUCT;

// ---------------------------
// Control register 2
// RTC Address: 0x1
// ---------------------------
typedef union
{
	struct
	{
		bitfield msf:		   	1;
		bitfield wdtf:		   	1;
		bitfield tsf2:		  	1;
		bitfield af:		   	1;
		bitfield cdtf:		  	1;
		bitfield tsie:			1;
		bitfield aie:			1;
		bitfield cdtie:			1;
	} bit;

   uint8 reg;
} RTC_CONTROL_2_STRUCT;

// ---------------------------
// Control register 3
// RTC Address: 0x2
// ---------------------------
typedef union
{
	struct
	{
		bitfield pwrmng:	  	3;
		bitfield btse:			1;
		bitfield bf:			1;
		bitfield blf:			1;
		bitfield bie:			1;
		bitfield blie:			1;
	} bit; 

	uint8 reg;
} RTC_CONTROL_3_STRUCT;

// ---------------------------
// Seconds register
// RTC Address: 0x3
// ---------------------------
typedef union
{
	struct
	{
		bitfield osf:		1;
		bitfield tenSecond:	3;
		bitfield oneSecond:	4;
	} bit; 

	uint8 reg;
} RTC_SECONDS_STRUCT_TEMP;

// ---------------------------
// Minutes register
// RTC Address: 0x4
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused: 	1;
		bitfield tenMinute:	3;
		bitfield oneMinute:	4;
	} bit; 

	uint8 reg;
} RTC_MINUTES_STRUCT_TEMP;

// ---------------------------
// Hours register
// RTC Address: 0x5
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:	2;
		bitfield ampm: 		1;
		bitfield tenHour:	1;
		bitfield oneHour:	4;
	} bit; 

	struct
	{
		bitfield unused:	2;
		bitfield tenHour:	2;
		bitfield oneHour:	4;
	} hours_24;

	uint8 reg;
} RTC_HOURS_STRUCT_TEMP;

// ---------------------------
// Days register
// RTC Address: 0x6
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:	2;
		bitfield tenDay:	2;
		bitfield oneDay:	4;
	} bit; 

	uint8 reg;
} RTC_DAYS_STRUCT;

// ---------------------------
// Weekdays register
// RTC Address: 0x7
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:   	5;
		bitfield weekday:	3;
	} bit; 

	uint8 reg;
} RTC_WEEKDAYS_STRUCT;

// ---------------------------
// Months register
// RTC Address: 0x8
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:	3;
		bitfield tenMonth:	1;
		bitfield oneMonth:	4;
	} bit; 

	uint8 reg;
} RTC_MONTHS_STRUCT;

// ---------------------------
// Years register
// RTC Address: 0x9
// ---------------------------
typedef union
{
	struct
	{
		bitfield tenYear:	4;
		bitfield oneYear:	4;
	} bit; 

	uint8 reg;
} RTC_YEARS_STRUCT;

// ---------------------------
// Second alarm register
// RTC Address: 0xA
// ---------------------------
typedef union
{
	struct
	{
		bitfield ae_s:				1;
		bitfield tenSecond_alarm:	3;
		bitfield oneSecond_alarm:	4;
	} bit; 

	uint8 reg;
} RTC_SECOND_ALARM_STRUCT;

// ---------------------------
// Minute alarm register
// RTC Address: 0xB
// ---------------------------
typedef union
{
	struct
	{
		bitfield ae_m:				1;
		bitfield tenMinute_alarm:	3;
		bitfield oneMinute_alarm:	4;
	} bit; 

	uint8 reg;
} RTC_MINUTE_ALARM_STRUCT;

// ---------------------------
// Hour alarm register
// RTC Address: 0xC
// ---------------------------
typedef union
{
	struct
	{
		bitfield ae_h:			1;
		bitfield unused:		1;
		bitfield ampm:			1;
		bitfield tenHour_alarm:	1;
		bitfield oneHour_alarm:	4;
	} bit; 

	struct
	{
		bitfield ae_h:			1;
		bitfield unused:		1;
		bitfield tenHour_alarm:	2;
		bitfield oneHour_alarm:	4;
	} hours_24;

	uint8 reg;
} RTC_HOUR_ALARM_STRUCT;

// ---------------------------
// Day alarm register
// RTC Address: 0xD
// ---------------------------
typedef union
{
	struct
	{
		bitfield ae_d:			1;
		bitfield unused:		1;
		bitfield tenDay_alarm:	2;
		bitfield oneDay_alarm:	4;
	} bit; 

	uint8 reg;
} RTC_DAY_ALARM_STRUCT_TEMP;

// ---------------------------
// Weekday alarm register
// RTC Address: 0xE
// ---------------------------
typedef union
{
	struct
	{
		bitfield ae_w:			1;
		bitfield unused:		4;
		bitfield weekday_alarm:	3;
	} bit; 

	uint8 reg;
} RTC_WEEKDAY_ALARM_STRUCT;

// ---------------------------
// Clock out control register
// RTC Address: 0xF
// ---------------------------
typedef union
{
	struct
	{
		bitfield tcr:			2;
		bitfield unused:		3;
		bitfield cof:			3;
	} bit; 

	uint8 reg;
} RTC_CLOCK_OUT_CONTROL_STRUCT;

// ---------------------------
// Watchdog control register
// RTC Address: 0x10
// ---------------------------
typedef union
{
	struct
	{
		bitfield wd_cd:			2;
		bitfield ti_tp:			1;
		bitfield unused:		3;
		bitfield tf:			2;
	} bit; 

	uint8 reg;
} RTC_WATCHDOG_CONTROL_STRUCT;

// ---------------------------
// Watchdog register
// RTC Address: 0x11
// ---------------------------
typedef union
{
	struct
	{
		bitfield watchdog:		8;
	} bit; 

	uint8 reg;
} RTC_WATCHDOG_STRUCT;

// ---------------------------
// Timestamp control register
// RTC Address: 0x12
// ---------------------------
typedef union
{
	struct
	{
		bitfield tsm:			1;
		bitfield tsoff:			1;
		bitfield unused:		1;
		bitfield one_O_16_ts:	5;
	} bit; 

	uint8 reg;
} RTC_TIMESTAMP_CONTROL_STRUCT;

// ---------------------------
// Timestamp seconds register
// RTC Address: 0x13
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		1;
		bitfield seconds:		7;
	} bit; 

	uint8 reg;
} RTC_TIMESTAMP_SECONDS_STRUCT;

// ---------------------------
// Timestamp minutes register
// RTC Address: 0x14
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		1;
		bitfield minutes:		7;
	} bit; 

	uint8 reg;
} RTC_TIMESTAMP_MINUTES_STRUCT;

// ---------------------------
// Timestamp hours register
// RTC Address: 0x15
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		2;
		bitfield ampm:			1;
		bitfield hours:			5;
	} bit; 

	struct
	{
		bitfield unused:		2;
		bitfield hours:			6;
	} hours_24; 

	uint8 reg;
} RTC_TIMESTAMP_HOURS_STRUCT;

// ---------------------------
// Timestamp days register
// RTC Address: 0x16
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		2;
		bitfield days:			6;
	} bit; 

	uint8 reg;
} RTC_TIMESTAMP_DAYS_STRUCT;

// ---------------------------
// Timestamp months register
// RTC Address: 0x17
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		3;
		bitfield months:		5;
	} bit; 

	uint8 reg;
} RTC_TIMESTAMP_MONTHS_STRUCT;

// ---------------------------
// Timestamp years register
// RTC Address: 0x18
// ---------------------------
typedef union
{
	struct
	{
		bitfield years:			8;
	} bit; 

	uint8 reg;
} RTC_TIMESTAMP_YEARS_STRUCT;

// ---------------------------
// Aging offset register
// RTC Address: 0x19
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		4;
		bitfield ao:			4;
	} bit; 

	uint8 reg;
} RTC_AGING_OFFSET_STRUCT;

// ---------------------------
// RAM address MSB register
// RTC Address: 0x1A
// ---------------------------
typedef union
{
	struct
	{
		bitfield unused:		7;
		bitfield ra8:			1;
	} bit; 

	uint8 reg;
} RTC_RAM_ADDRESS_MSB_STRUCT;

// ---------------------------
// RAM address LSB register
// RTC Address: 0x1B
// ---------------------------
typedef union
{
	struct
	{
		bitfield ra:			8;
	} bit; 

	uint8 reg;
} RTC_RAM_ADDRESS_LSB_STRUCT;

// ---------------------------
// RAM write register
// RTC Address: 0x1C
// ---------------------------
typedef union
{
	struct
	{
		bitfield write_data:	8;
	} bit; 

	uint8 reg;
} RTC_RAM_WRITE_STRUCT;

// ---------------------------
// RAM read register
// RTC Address: 0x1D
// ---------------------------
typedef union
{
	struct
	{
		bitfield read_data:		8;
	} bit; 

	uint8 reg;
} RTC_RAM_READ_STRUCT;

// ===========================
// RTC Memory Location Defines
// ===========================
#define RTC_CONTROL_1_ADDR			0x00
#define RTC_CONTROL_2_ADDR			0x01
#define RTC_CONTROL_3_ADDR			0x02
#define RTC_SECONDS_ADDR_TEMP		0x03
#define RTC_MINUTES_ADDR_TEMP		0x04
#define RTC_HOURS_ADDR_TEMP			0x05
#define RTC_DAYS_ADDR				0x06
#define RTC_WEEKDAYS_ADDR			0x07
#define RTC_MONTHS_ADDR				0x08
#define RTC_YEARS_ADDR				0x09
#define RTC_SECOND_ALARM_ADDR		0x0a
#define RTC_MINUTE_ALARM_ADDR		0x0b
#define RTC_HOUR_ALARM_ADDR			0x0c
#define RTC_DAY_ALARM_ADDR_TEMP		0x0d
#define RTC_WEEKDAY_ALARM_ADDR		0x0e
#define RTC_CLOCK_OUT_CONTROL_ADDR	0x0f
#define RTC_WATCHDOG_CONTROL_ADDR	0x10
#define RTC_WATCHDOG_ADDR			0x11
#define RTC_TIMESTAMP_CONTROL_ADDR	0x12
#define RTC_TIMESTAMP_SECONDS_ADDR	0x13
#define RTC_TIMESTAMP_MINUTES_ADDR	0x14
#define RTC_TIMESTAMP_HOURS_ADDR	0x15
#define RTC_TIMESTAMP_DAYS_ADDR		0x16
#define RTC_TIMESTAMP_MONTHS_ADDR	0x17
#define RTC_TIMESTAMP_YEARS_ADDR	0x18
#define RTC_AGING_OFFSET_ADDR		0x19
#define RTC_RAM_ADDRESS_MSB_ADDR	0x1a
#define RTC_RAM_ADDRESS_LSB_ADDR	0x1b
#define RTC_RAM_WRITE_ADDR			0x1c
#define RTC_RAM_READ_ADDR			0x1d

#if 0
typedef struct
{
	RTC_CONTROL_1_STRUCT			control_1;
	RTC_CONTROL_2_STRUCT			control_2;
	RTC_CONTROL_3_STRUCT			control_3;
	RTC_SECONDS_STRUCT_TEMP			seconds;
	RTC_MINUTES_STRUCT_TEMP			minutes;
	RTC_HOURS_STRUCT_TEMP			hours;
	RTC_DAYS_STRUCT					days;
	RTC_WEEKDAYS_STRUCT				weekdays;
	RTC_MONTHS_STRUCT				months;
	RTC_YEARS_STRUCT				years;
	RTC_SECOND_ALARM_STRUCT			second_alarm;
	RTC_MINUTE_ALARM_STRUCT			minute_alarm;
	RTC_HOUR_ALARM_STRUCT			hour_alarm;
	RTC_DAY_ALARM_STRUCT_TEMP		day_alarm;
	RTC_WEEKDAY_ALARM_STRUCT		weekday_alarm;
	RTC_CLOCK_OUT_CONTROL_STRUCT	clock_out_control;
	RTC_WATCHDOG_CONTROL_STRUCT		watchdog_control;
	RTC_WATCHDOG_STRUCT				watchdog;
	RTC_TIMESTAMP_CONTROL_STRUCT	timestamp_control;
	RTC_TIMESTAMP_SECONDS_STRUCT	timestamp_seconds;
	RTC_TIMESTAMP_MINUTES_STRUCT	timestamp_minutes;
	RTC_TIMESTAMP_HOURS_STRUCT		timestamp_hours;
	RTC_TIMESTAMP_DAYS_STRUCT		timestamp_days;
	RTC_TIMESTAMP_MONTHS_STRUCT		timestamp_months;
	RTC_TIMESTAMP_YEARS_STRUCT		timestamp_years;
	RTC_AGING_OFFSET_STRUCT			aging_offset;
	RTC_RAM_ADDRESS_MSB_STRUCT		ram_address_msb;
	RTC_RAM_ADDRESS_LSB_STRUCT		ram_address_lsb;
	RTC_RAM_WRITE_STRUCT			ram_write;
	RTC_RAM_READ_STRUCT				ram_read;
} RTC_MEM_MAP_STRUCT;
#else
typedef struct
{
	uint8 control_1;
	uint8 control_2;
	uint8 control_3;
	uint8 seconds;
	uint8 minutes;
	uint8 hours;
	uint8 days;
	uint8 weekdays;
	uint8 months;
	uint8 years;
	uint8 second_alarm;
	uint8 minute_alarm;
	uint8 hour_alarm;
	uint8 day_alarm;
	uint8 weekday_alarm;
	uint8 clock_out_control;
	uint8 watchdog_control;
	uint8 watchdog;
	uint8 timestamp_control;
	uint8 timestamp_seconds;
	uint8 timestamp_minutes;
	uint8 timestamp_hours;
	uint8 timestamp_days;
	uint8 timestamp_months;
	uint8 timestamp_years;
	uint8 aging_offset;
	uint8 ram_address_msb;
	uint8 ram_address_lsb;
	uint8 ram_write;
	uint8 ram_read;
} RTC_MEM_MAP_STRUCT;
#endif

#if 0
typedef struct
{
	RTC_SECONDS_STRUCT_TEMP			seconds;
	RTC_MINUTES_STRUCT_TEMP			minutes;
	RTC_HOURS_STRUCT_TEMP			hours;
	RTC_DAYS_STRUCT					days;
	RTC_WEEKDAYS_STRUCT				weekdays;
	RTC_MONTHS_STRUCT				months;
	RTC_YEARS_STRUCT				years;
} RTC_DATE_TIME_STRUCT;
#else
typedef struct
{
	uint8 seconds;
	uint8 minutes;
	uint8 hours;
	uint8 days;
	uint8 weekdays;
	uint8 months;
	uint8 years;
} RTC_DATE_TIME_STRUCT;
#endif

#if 0
typedef struct
{
	RTC_DAYS_STRUCT					days;
	RTC_WEEKDAYS_STRUCT				weekdays;
	RTC_MONTHS_STRUCT				months;
	RTC_YEARS_STRUCT				years;
} RTC_DATE_STRUCT;
#else
typedef struct
{
	uint8 days;
	uint8 weekdays;
	uint8 months;
	uint8 years;
} RTC_DATE_STRUCT;
#endif

#if 0
typedef struct
{
	RTC_SECONDS_STRUCT_TEMP			seconds;
	RTC_MINUTES_STRUCT_TEMP			minutes;
	RTC_HOURS_STRUCT_TEMP			hours;
} RTC_TIME_STRUCT;
#else
typedef struct
{
	uint8 seconds;
	uint8 minutes;
	uint8 hours;
} RTC_TIME_STRUCT;
#endif

#if 0
typedef struct
{
	RTC_SECOND_ALARM_STRUCT			second_alarm;
	RTC_MINUTE_ALARM_STRUCT			minute_alarm;
	RTC_HOUR_ALARM_STRUCT			hour_alarm;
	RTC_DAY_ALARM_STRUCT_TEMP		day_alarm;
	RTC_WEEKDAY_ALARM_STRUCT		weekday_alarm;
} RTC_ALARM_STRUCT;
#else
typedef struct
{
	uint8 second_alarm;
	uint8 minute_alarm;
	uint8 hour_alarm;
	uint8 day_alarm;
	uint8 weekday_alarm;
} RTC_ALARM_STRUCT;
#endif

#endif // end of ns8100

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
BOOLEAN InitExternalRtc(void);
uint8 setRtcTime(DATE_TIME_STRUCT* time);
uint8 setRtcDate(DATE_TIME_STRUCT* time);
DATE_TIME_STRUCT getRtcTime(void);
void updateCurrentTime(void);
DATE_TIME_STRUCT getCurrentTime(void);
void DisableRtcAlarm(void);
void EnableRtcAlarm(uint8 day, uint8 hour, uint8 minute, uint8 second);
void convertCurrentTimeForFat(uint8* fatTimeField);
void convertCurrentDateForFat(uint8* fatTimeDate);
void rtcWrite(uint8 register_address, int length, uint8* data);
void rtcRead(uint8 register_address, int length, uint8* data);

#endif // _RTC_H_
