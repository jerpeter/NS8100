///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Rtc.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/Rtc.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Old_Board.h"
#include "Typedefs.h"
#include "RealTimeClock.h"
#include "SoftTimer.h"
#include "Uart.h"
#include "rtc_test_menu.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern MONTH_TABLE_STRUCT monthTable[];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
DATE_TIME_STRUCT  g_currentTime;
SOFT_TIMER_STRUCT g_rtcTimerBank[NUM_OF_SOFT_TIMERS];
uint32 g_rtcSoftTimerTickCount = 0;
volatile uint32 g_rtcCurrentTickCount = 0;
uint32 g_UpdateCounter = 0;
uint8 autoCalDaysToWait = 0;

// RTC Alarm Frequency table, 3 sets both ALM1 and ALM0 bits to a 1, 0 clears bits
RTC_ALARM_FREQ_STRUCT rtcAlarmFreq[] = {
{ONCE_PER_SECOND, 3, 3, 3, 3},
{ONCE_PER_MINUTE_WHEN_SECONDS_MATCH, 0, 3, 3, 3},
{ONCE_PER_HOUR_WHEN_MINUTES_AND_SECONDS_MATCH, 0, 0, 3, 3},
{ONCE_PER_DAY_WHEN_HOURS_MINUTES_AND_SECONDS_MATCH, 0, 0, 0, 3},
{WHEN_DAY_HOURS_MINUTES_AND_SECONDS_MATCH, 0, 0, 0, 0}
};

/****************************************
*	Function:   InitRtc(void)
*	Purpose:
****************************************/
BOOLEAN InitRtc(void)
{
	// REDO & Update to new RTC
	DATE_TIME_STRUCT time;

	//debug("Init Soft timer\n");
	// Initialize the softtimer array.
	byteSet(&g_rtcTimerBank[0], 0, (sizeof(SOFT_TIMER_STRUCT) * NUM_OF_SOFT_TIMERS));

	//debug("Freeze Updates\n");
	// Freeze the RTC registers (Unknown if needed for settings and control registers)
	RTC_FREEZE_UPDATES;

	//debug("Set rates\n");
	// Set rates register. Watchdog timeout rate (1.5sec) (Watchdog disabled), Periodic interrupt rate (500ms)
	RTC_RATES.bit.watchdogTimeoutRate = WATCHDOG_TIMOUT_1;
	RTC_RATES.bit.periodicIntRate = PULSE_2_PER_SEC;

	//debug("Set Interrupt enables\n");
	// Set interrupt enables
	RTC_ENABLES.bit.alarmIntEnable = OFF;
	RTC_ENABLES.bit.periodicIntEnable = ON;
	RTC_ENABLES.bit.powerFailIntEnable = ON;
	RTC_ENABLES.bit.batteryBackAlarmIntEnable = OFF;

	//debug("Clear flags\n");
	// Clear Flags register by reading it
	RTC_FLAGS.reg = RTC_FLAGS.reg;

	//debug("Set control bits\n");
	// Set control bits
	RTC_CONTROL.bit._24_hourEnable = ON;
	RTC_CONTROL.bit.daylightSavingsEnable = OFF;

	//debug("Unfreeeze registers\n");
	// UnFreeze the RTC registers
	RTC_UNFREEZE_UPDATES;

	//debug("Check Oscillator\n");
	// Check if the RTC oscillator is not enabled (RTC not running)
	if (RTC_CONTROL.bit.oscillatorEnable == OFF)
	{
		//debug("Enable Oscillator\n");
		// Enable the oscillator to allow the RTC to keep time
		RTC_CONTROL.bit.oscillatorEnable = ON;

		//debug("Init time\n");
		// Setup an initial date and time
		time.year = 07;
		time.month = 1;
		time.day = 1;
		time.weekday = getDayOfWeek(time.year, time.month, time.day);
		time.hour = 8;
		time.min = 0;
		time.sec = 0;
		time.clk_12_24 = 1;
		time.hundredths = 0;

		setRtcDate(&time);
		setRtcTime(&time);
	}

	//debug("Update Current time\n");
	// Need to initialize the global Current Time
	updateCurrentTime();

	return (TRUE);
}

/****************************************
*	Function:   setRtcTime
*	Purpose: Set the RTC clock time.
****************************************/
uint8 setRtcTime(DATE_TIME_STRUCT* time)
{
	uint8 status = FAILED;
	uint8 hours;
	uint8 minutes;
	uint8 seconds;
	uint16 newTime[3];

	if ((time->hour < 24) && (time->min < 60) && (time->sec < 60))
	{
		status = PASSED;

		// Setup hour register (24 Hour settings), BCD format
		hours = (uint8)((time->hour / 10) << 4);
		hours |= (uint8)(time->hour % 10);

		// Setup minute register, BCD format
		minutes = (uint8)((time->min / 10) << 4);
		minutes |= (uint8)(time->min % 10);

		// Setup seconds register, BCD format
		seconds = (uint8)((time->sec / 10) << 4);
		seconds |= (uint8)(time->sec % 10);

		//RTC_FREEZE_UPDATES;

		newTime[0]=seconds;
		newTime[1]=minutes;
		newTime[2]=hours;
		rtc_write(RTC_SECONDS_ADDR_TEMP, 3, (uint16*)&newTime[0]);
		
		//RTC_UNFREEZE_UPDATES;
	}

	return (status);
}

/****************************************
*	Function:   setRtcDate
*	Purpose: Set the RTC date values.
****************************************/
uint8 setRtcDate(DATE_TIME_STRUCT* time)
{
	uint8 status = FAILED;
	uint8 year;
	uint8 month;
	uint8 day;
	uint8 dayOfWeek;
	uint16 newDate[4];

	// Check if years and months settings are valid
	if ((time->year <= 99) && (time->month > 0) && (time->month <= TOTAL_MONTHS))
	{
		// Check is the days setting is valid for the month given that month setting has been validated
		if ((time->day > 0) && (time->day <= monthTable[time->month].days))
		{
			// Flag success since month, day and year settings are valid
			status = PASSED;

			// Setup year register, BCD format
			year = (uint8)((time->year / 10) << 4);
			year |= (uint8)(time->year % 10);

			// Setup month register, BCD format
			month = (uint8)((time->month / 10) << 4);
			month |= (uint8)(time->month % 10);

			// Setup day register, BCD format
			day = (uint8)((time->day / 10) << 4);
			day |= (uint8)(time->day % 10);

			// Setup day of week register (Sun: 00, Mon: 01, Tue: 02, Wed: 03, Thur: 04, Fri: 05, Sat: 06)
			dayOfWeek = getDayOfWeek(time->year, time->month, time->day);

			//RTC_FREEZE_UPDATES;

			// Write out the register values to the RTC
			newDate[0] = day;
			newDate[1] = dayOfWeek;
			newDate[2] = month;
			newDate[3] = year;
			rtc_write(RTC_DAYS_ADDR, 4, &newDate[0]);

			//RTC_UNFREEZE_UPDATES;
		}
	}

	return (status);
}

/****************************************
*	Function:   getRtcTime
*	Purpose:
*
*	ProtoType:  getRtcTime(void)
*	Input:
*	Output:
****************************************/
DATE_TIME_STRUCT getRtcTime(void)
{
	DATE_TIME_STRUCT time;
	RTC_DATE_TIME_STRUCT translateTime;
	uint16 currentDateTime[7];
	
	byteSet(&time, 0, sizeof(DATE_TIME_STRUCT));

	rtc_read(RTC_SECONDS_ADDR_TEMP, 7, (uint16*)&currentDateTime);

	//RTC_FREEZE_UPDATES;

	translateTime.seconds.reg = currentDateTime[0];
	translateTime.minutes.reg = currentDateTime[1];
	translateTime.hours.reg = currentDateTime[2];
	translateTime.days.reg = currentDateTime[3];
	translateTime.weekdays.reg = currentDateTime[4];
	translateTime.months.reg = currentDateTime[5];
	translateTime.years.reg = currentDateTime[6];

	time.year = (uint8)(translateTime.years.bit.oneYear + (translateTime.years.bit.tenYear * 10));
	time.month = (uint8)(translateTime.months.bit.oneMonth + (translateTime.months.bit.tenMonth * 10));
	time.weekday = (uint8)(translateTime.weekdays.bit.weekday);
	time.day = (uint8)(translateTime.days.bit.oneDay + (translateTime.days.bit.tenDay * 10));
	time.hour = (uint8)(translateTime.hours.bit.oneHour + (translateTime.hours.bit.tenHour * 10));
	time.min = (uint8)(translateTime.minutes.bit.oneMinute + (translateTime.minutes.bit.tenMinute * 10));
	time.sec = (uint8)(translateTime.seconds.bit.oneSecond + (translateTime.seconds.bit.tenSecond * 10));

	//RTC_UNFREEZE_UPDATES;

    return (time);
}

/****************************************
*	Function:   updateCurrentTime
*	Purpose:	Update the current time with the RTC time
****************************************/
void updateCurrentTime(void)
{
	g_rtcCurrentTickCount = 0;
	g_currentTime = getRtcTime();
}

/****************************************
*	Function:   getCurrentTime
*	Purpose:	Gets the current time adjusted by the expired seconds since being stored
****************************************/
DATE_TIME_STRUCT getCurrentTime(void)
{
	DATE_TIME_STRUCT currentTime = g_currentTime;
	uint16 expiredSecs = (uint16)(g_rtcCurrentTickCount / 2);

	// Check if the real time hasn't been updated in the last minute (Should occur every 30 seconds)
	if (expiredSecs >= 60)
	{
		// Unlikely this case will ever occur. It's been over a minute, just re-read the real time
		updateCurrentTime();
		currentTime = g_currentTime;
	}
	// Check if the amount of expired seconds results in a minute change
	else if ((currentTime.sec + expiredSecs) > 59)
	{
		// Check if the amount of expired seconds results in an hour change
		if ((currentTime.min + 1) > 59)
		{
			// Check if the amount of expired seconds results in a day change
			if ((currentTime.hour + 1) > 23)
			{
				// Quit fooling around, and re-read the real time
				updateCurrentTime();
				currentTime = g_currentTime;
			}
			else // Still within the day
			{
				// Increment the hour
				currentTime.hour += 1;
				// Reset the minutes
				currentTime.min = 0;
				// Calculate the number of seconds past the 60 second boundary
				currentTime.sec = (uint8)((currentTime.sec + expiredSecs) - 60);
			}
		}
		else // Still within the hour
		{
			// Increment the minute
			currentTime.min += 1;
			// Calculate the number of seconds past the 60 second boundary
			currentTime.sec = (uint8)((currentTime.sec + expiredSecs) - 60);
		}
	}
	else // Still within the minute
	{
		currentTime.sec += expiredSecs;
	}

	return (currentTime);
}

/****************************************
*	Function:    DisableRtcAlarm
*	Purpose:
****************************************/
void DisableRtcAlarm(void)
{
	RTC_ALARM_STRUCT disableAlarm;
	uint16 clearAlarmFlag = 0xEF; // Logic 0 to clear Alarm flag, bit 4

#if 0
	RTC_ENABLES.bit.alarmIntEnable = OFF;
	RTC_ENABLES.bit.batteryBackAlarmIntEnable = OFF;

	// Clear flags just in case
	RTC_FLAGS.reg = RTC_FLAGS.reg;
#endif

	disableAlarm.second_alarm.reg = 0x80;
	disableAlarm.minute_alarm.reg = 0x80;
	disableAlarm.hour_alarm.reg =  0x80;
	disableAlarm.day_alarm.reg = 0x80;
	disableAlarm.weekday_alarm.reg = 0x80;
	
	// Turn off all the alarm enable flags
	rtc_write(RTC_SECOND_ALARM_ADDR, 5, (uint16*)&disableAlarm);
	
	// Clear the Alarm flag
	rtc_write(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
}

/****************************************
*	Function:    EnableRtcAlarm
*	Purpose:
****************************************/
void EnableRtcAlarm(uint8 day, uint8 hour, uint8 minute, uint8 second)
{
	RTC_ALARM_STRUCT enableAlarm;
	uint8 clearAlarmFlag = 0xEF; // Logic 0 to clear Alarm flag, bit 4

	//RTC_FREEZE_UPDATES;

	enableAlarm.day_alarm.bit.ae_d = 0; // Enable
	enableAlarm.day_alarm.bit.tenDay_alarm = (uint8)(day / 10);
	enableAlarm.day_alarm.bit.oneDay_alarm = (uint8)(day % 10);

	enableAlarm.hour_alarm.bit.ae_h = 0; // Enable
	enableAlarm.hour_alarm.bit.tenHour_alarm = (uint8)(hour / 10);
	enableAlarm.hour_alarm.bit.oneHour_alarm = (uint8)(hour % 10);

	enableAlarm.minute_alarm.bit.ae_m = 0; // Enable
	enableAlarm.minute_alarm.bit.tenMinute_alarm = (uint8)(minute / 10);
	enableAlarm.minute_alarm.bit.oneMinute_alarm = (uint8)(minute % 10);

	enableAlarm.second_alarm.bit.ae_s = 0; // Enable
	enableAlarm.second_alarm.bit.tenSecond_alarm = (uint8)(second / 10);
	enableAlarm.second_alarm.bit.oneSecond_alarm = (uint8)(second % 10);

	enableAlarm.weekday_alarm.bit.ae_w = 1;
	
	rtc_write(RTC_SECOND_ALARM_ADDR, 5, (uint16*)&enableAlarm);

	// Clear the Alarm flag
	rtc_write(RTC_CONTROL_2_ADDR, 1, (uint16*)&clearAlarmFlag);

	//SetAlarmFrequency(WHEN_DAY_HOURS_MINUTES_AND_SECONDS_MATCH);
	//RTC_ENABLES.bit.alarmIntEnable = ON;
	//RTC_ENABLES.bit.batteryBackAlarmIntEnable = ON;
	//RTC_UNFREEZE_UPDATES;
}

/****************************************
*	Function:    SetAlarmFrequency
*	Purpose:
****************************************/
void SetAlarmFrequency(uint8 mode)
{
	RTC_DAY_ALARM.alarmBit.alarmMask = rtcAlarmFreq[mode].dayAlarmMask;
	RTC_HOURS_ALARM.alarmBit.alarmMask = rtcAlarmFreq[mode].hoursAlarmMask;
	RTC_MINUTES_ALARM.alarmBit.alarmMask = rtcAlarmFreq[mode].minutesAlarmMask;
	RTC_SECONDS_ALARM.alarmBit.alarmMask = rtcAlarmFreq[mode].secondsAlarmMask;
}

/****************************************
*	Function:    SetPeriodicInterruptFrequency
*	Purpose:
****************************************/
void SetPeriodicInterruptFrequency(uint8 frequency)
{
	RTC_RATES.bit.periodicIntRate = frequency;
}
