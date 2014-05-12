///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
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
#include "spi.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define	RTC_SPI_NPCS	1

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
#if 0 // ns7100
// RTC Alarm Frequency table, 3 sets both ALM1 and ALM0 bits to a 1, 0 clears bits
static RTC_ALARM_FREQ_STRUCT s_rtcAlarmFreq[] = {
{ONCE_PER_SECOND, 3, 3, 3, 3},
{ONCE_PER_MINUTE_WHEN_SECONDS_MATCH, 0, 3, 3, 3},
{ONCE_PER_HOUR_WHEN_MINUTES_AND_SECONDS_MATCH, 0, 0, 3, 3},
{ONCE_PER_DAY_WHEN_HOURS_MINUTES_AND_SECONDS_MATCH, 0, 0, 0, 3},
{WHEN_DAY_HOURS_MINUTES_AND_SECONDS_MATCH, 0, 0, 0, 0}
};
#endif

/****************************************
*	Function:   InitExternalRtc(void)
*	Purpose:
****************************************/
BOOLEAN InitExternalRtc(void)
{
	// REDO & Update to new RTC
	DATE_TIME_STRUCT time;
	RTC_MEM_MAP_STRUCT rtcMap;

	//debug("Init Soft timer\n");
	// Initialize the softtimer array.
	byteSet(&g_rtcTimerBank[0], 0, (sizeof(SOFT_TIMER_STRUCT) * NUM_OF_SOFT_TIMERS));

	debug("\r\n\n______________________________________________________________________________________\n");
	debug("External RTC Init...\n");

	// Get the base of the external RTC memory map
	rtcRead(RTC_CONTROL_1_ADDR, 10, (uint8*)&rtcMap);
	
	if ((rtcMap.seconds & RTC_CLOCK_INTEGRITY) || (rtcMap.control_1 & RTC_CLOCK_STOPPED))
	{
		debug("Init RTC: Clock integrity not guaranteed or Clock stopped, setting default time and date\n");

		rtcMap.control_1 |= RTC_24_HOUR_MODE;
		rtcWrite(RTC_CONTROL_1_ADDR, 1, &rtcMap.control_1);

		// Setup an initial date and time
		time.year = 12;
		time.month = 8;
		time.day = 1;
		time.weekday = getDayOfWeek(time.year, time.month, time.day);
		time.hour = 8;
		time.min = 1;
		time.sec = 1;

		setRtcDate(&time);
		setRtcTime(&time);
	}
	else
	{
		debug("Ext RTC: Clock running and intergrity validated\n");
	}

#if 1 // Normal
	// Clear all flags (write 0's) and disable interrupt generation for all but timestamp pin
	rtcMap.control_2 = (RTC_ALARM_INT_ENABLE);
	rtcWrite(RTC_CONTROL_2_ADDR, 1, &rtcMap.control_2);
#else // Test
	// Disable alarm settings and clear flags 
	DisableRtcAlarm();
#endif

	// Need to initialize the global Current Time
	updateCurrentTime();

	// Check for RTC reset
	if ((g_currentTime.year == 0) || (g_currentTime.month == 0) || (g_currentTime.day == 0))
	{
		debugWarn("Warning: External RTC date not set, assuming power loss reset... applying a default date\n");
		// BCD formats
		g_currentTime.year = 0x12;
		g_currentTime.month = 0x08;
		g_currentTime.day = 0x01;

		setRtcDate(&g_currentTime);
	}

	return (TRUE);
}

/****************************************
*	Function:   setRtcTime
*	Purpose: Set the RTC clock time.
****************************************/
uint8 setRtcTime(DATE_TIME_STRUCT* time)
{
	RTC_TIME_STRUCT rtcTime;
	uint8 status = FAILED;

	if ((time->hour < 24) && (time->min < 60) && (time->sec < 60))
	{
		status = PASSED;

		// Setup time registers (24 Hour settings), BCD format
		rtcTime.hours = UINT8_CONVERT_TO_BCD(time->hour, RTC_BCD_HOURS_MASK);
		rtcTime.minutes = UINT8_CONVERT_TO_BCD(time->min, RTC_BCD_MINUTES_MASK);
		rtcTime.seconds = UINT8_CONVERT_TO_BCD(time->sec, RTC_BCD_SECONDS_MASK);

		rtcWrite(RTC_SECONDS_ADDR_TEMP, 3, (uint8*)&rtcTime);
	}

	return (status);
}

/****************************************
*	Function:   setRtcDate
*	Purpose: Set the RTC date values.
****************************************/
uint8 setRtcDate(DATE_TIME_STRUCT* time)
{
	RTC_DATE_STRUCT rtcDate;
	uint8 status = FAILED;

	// Check if years and months settings are valid
	if ((time->year <= 99) && (time->month > 0) && (time->month <= TOTAL_MONTHS))
	{
		// Check is the days setting is valid for the month given that month setting has been validated
		if ((time->day > 0) && (time->day <= g_monthTable[time->month].days))
		{
			// Flag success since month, day and year settings are valid
			status = PASSED;

			// Setup time registers (24 Hour settings), BCD format
			rtcDate.years = UINT8_CONVERT_TO_BCD(time->year, RTC_BCD_YEARS_MASK);
			rtcDate.months = UINT8_CONVERT_TO_BCD(time->month, RTC_BCD_MONTHS_MASK);
			rtcDate.days = UINT8_CONVERT_TO_BCD(time->day, RTC_BCD_DAYS_MASK);

			// Calculate the weekday based on the new date
			rtcDate.weekdays = getDayOfWeek(time->year, time->month, time->day);

			//debug("Ext RTC: Apply Date: %x-%x-%x\n", rtcDate.months, rtcDate.days, rtcDate.years);

			rtcWrite(RTC_DAYS_ADDR, 4, (uint8*)&rtcDate);
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

	rtcRead(RTC_SECONDS_ADDR_TEMP, 7, (uint8*)&translateTime);

	// Get time and date registers (24 Hour settings), BCD format
	time.year = BCD_CONVERT_TO_UINT8(translateTime.years, RTC_BCD_YEARS_MASK);
	time.month = BCD_CONVERT_TO_UINT8(translateTime.months, RTC_BCD_MONTHS_MASK);
	time.day = BCD_CONVERT_TO_UINT8(translateTime.days, RTC_BCD_DAYS_MASK);
#if 0 // ns7100
	time.weekday = getDayOfWeek(time.year, time.month, time.day);
#else // ns8100
	time.weekday = BCD_CONVERT_TO_UINT8(translateTime.weekdays, RTC_BCD_WEEKDAY_MASK);
#endif
	time.hour = BCD_CONVERT_TO_UINT8(translateTime.hours, RTC_BCD_HOURS_MASK);
	time.min = BCD_CONVERT_TO_UINT8(translateTime.minutes, RTC_BCD_MINUTES_MASK);
	time.sec = BCD_CONVERT_TO_UINT8(translateTime.seconds, RTC_BCD_SECONDS_MASK);

	//debug("Ext RTC: Get Time: %02d:%02d:%02d (%d), %02d-%02d-%02d\n\n", time.hour, time.min, time.sec, time.weekday, time.month, time.day, time.year);

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
*	Function:    convertCurrentTimeForFat
*	Purpose:
****************************************/
void convertCurrentTimeForFat(uint8* fatTimeField)
{
	DATE_TIME_STRUCT currentTime = getCurrentTime();
	uint16 conversionTime;
	
/*
	The File Time:	The two bytes at offsets 0x16 and 0x17 are treated as a 16 bit value; remember that the least significant byte is at offset 0x16. They contain the time when the file was created or last updated. The time is mapped in the bits as follows; the first line indicates the byte's offset, the second line indicates (in decimal) individual bit numbers in the 16 bit value, and the third line indicates what is stored in each bit.
	<------- 0x17 --------> <------- 0x16 -------->
	15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	h  h  h  h  h  m  m  m  m  m  m  x  x  x  x  x
	hhhhh	--> indicates the binary number of hours (0-23)
	mmmmmm	--> indicates the binary number of minutes (0-59)
	xxxxx	--> indicates the binary number of two-second periods (0-29), representing seconds 0 to 58.
*/
	
	conversionTime = ((currentTime.sec / 2) & 0x1f);
	conversionTime |= ((currentTime.min & 0x3f) << 5);
	conversionTime |= ((currentTime.hour & 0x1f) << 11);
	
	fatTimeField[0] = (uint8)(conversionTime);
	fatTimeField[1] = (uint8)(conversionTime >> 8);
}

/****************************************
*	Function:    convertCurrentDateForFat
*	Purpose:
****************************************/
void convertCurrentDateForFat(uint8* fatDateField)
{
	DATE_TIME_STRUCT currentDate = getCurrentTime();
	uint16 conversionDate;

/*
	The File Date:	The two bytes at offsets 0x18 and 0x19 are treated as a 16 bit value; remember that the least significant byte is at offset 0x18. They contain the date when the file was created or last updated. The date is mapped in the bits as follows; the first line indicates the byte's offset, the second line indicates (in decimal) individual bit numbers in the 16 bit value, and the third line indicates what is stored in each bit.
	<------- 0x19 --------> <------- 0x18 -------->
	15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	y  y  y  y  y  y  y  m  m  m  m  d  d  d  d  d
	yyyyyyy	--> indicates the binary year offset from 1980 (0-119), representing the years 1980 to 2099
	mmmm	--> indicates the binary month number (1-12)
	ddddd	-->	indicates the binary day number (1-31) 
*/

	conversionDate = ((currentDate.day) & 0x1f);
	conversionDate |= ((currentDate.month & 0x0f) << 5);
	conversionDate |= (((currentDate.year + 20) & 0x7f) << 9);
	
	fatDateField[0] = (uint8)(conversionDate);
	fatDateField[1] = (uint8)(conversionDate >> 8);
}

/****************************************
*	Function:    DisableRtcAlarm
*	Purpose:
****************************************/
void DisableRtcAlarm(void)
{
	RTC_ALARM_STRUCT disableAlarm;

	uint8 clearAlarmFlag = 0xEA; // Logic 0 on the bit will clear Alarm flag, bit 4

	disableAlarm.second_alarm = RTC_DISABLE_ALARM;
	disableAlarm.minute_alarm = RTC_DISABLE_ALARM;
	disableAlarm.hour_alarm = RTC_DISABLE_ALARM;
	disableAlarm.day_alarm = RTC_DISABLE_ALARM;
	disableAlarm.weekday_alarm = RTC_DISABLE_ALARM;
	
	// Turn off all the alarm enable flags
	rtcWrite(RTC_SECOND_ALARM_ADDR, 5, (uint8*)&disableAlarm);
	
	// Clear the Alarm flag
	rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
}

#if 1 // Normal
/****************************************
*	Function:    EnableRtcAlarm
*	Purpose:
****************************************/
void EnableRtcAlarm(uint8 day, uint8 hour, uint8 minute, uint8 second)
{
	RTC_ALARM_STRUCT enableAlarm;

	uint8 clearAlarmFlag = 0xEA; // Logic 0 on the bit will clear Alarm flag, bit 4

	enableAlarm.day_alarm = (UINT8_CONVERT_TO_BCD(day, RTC_BCD_DAYS_MASK) | RTC_ENABLE_ALARM);
	enableAlarm.hour_alarm = (UINT8_CONVERT_TO_BCD(hour, RTC_BCD_HOURS_MASK) | RTC_ENABLE_ALARM);
	enableAlarm.minute_alarm = (UINT8_CONVERT_TO_BCD(minute, RTC_BCD_MINUTES_MASK) | RTC_ENABLE_ALARM);
	enableAlarm.second_alarm = (UINT8_CONVERT_TO_BCD(second, RTC_BCD_SECONDS_MASK) | RTC_ENABLE_ALARM);
	enableAlarm.weekday_alarm = RTC_DISABLE_ALARM;
	
	rtcWrite(RTC_SECOND_ALARM_ADDR, 5, (uint8*)&enableAlarm);

	// Clear the Alarm flag
	rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
	
	debug("Enable RTC Alarm with Day: %d, Hour: %d, Minute: %d and Second: %d\n", day, hour, minute, second);
}
#else // Test
/****************************************
*	Function:    EnableRtcAlarm
*	Purpose:
****************************************/
void EnableRtcAlarm(uint8 day, uint8 hour, uint8 minute, uint8 second)
{
	RTC_ALARM_STRUCT enableAlarm;

	uint8 clearAlarmFlag = 0xEA; // Logic 0 on the bit will clear Alarm flag, bit 4

	enableAlarm.day_alarm = RTC_DISABLE_ALARM;
	enableAlarm.hour_alarm = RTC_DISABLE_ALARM;
	enableAlarm.minute_alarm = RTC_DISABLE_ALARM;
	enableAlarm.second_alarm = (UINT8_CONVERT_TO_BCD(second, RTC_BCD_SECONDS_MASK) | RTC_ENABLE_ALARM);
	enableAlarm.weekday_alarm = RTC_DISABLE_ALARM;
	
	rtcWrite(RTC_SECOND_ALARM_ADDR, 5, (uint8*)&enableAlarm);

	// Clear the Alarm flag
	rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
}
#endif

/****************************************
*	Function:    
*	Purpose:
****************************************/
void rtcWrite(uint8 registerAddress, int length, uint8* data)
{
	uint16 dataContainer = 0;

	spi_selectChip(&AVR32_SPI1, RTC_SPI_NPCS);

	// Small delay before the RTC device is accessible
	soft_usecWait(RTC_ACCESS_DELAY);

	spi_write(&AVR32_SPI1, (RTC_WRITE_CMD | (registerAddress & 0x1F)));

	while (length--)
	{
		// Small delay before the RTC device is accessible
		soft_usecWait(RTC_ACCESS_DELAY);

		dataContainer = *data++;
		spi_write(&AVR32_SPI1, dataContainer);
	}

	spi_unselectChip(&AVR32_SPI1, RTC_SPI_NPCS);
}

/****************************************
*	Function:    
*	Purpose:
****************************************/
void rtcRead(uint8 registerAddress, int length, uint8* data)
{
	uint16 dataContainer = 0;

	spi_selectChip(&AVR32_SPI1, RTC_SPI_NPCS);

	// Small delay before the RTC device is accessible
	soft_usecWait(RTC_ACCESS_DELAY);

	spi_write(&AVR32_SPI1, (RTC_READ_CMD | (registerAddress & 0x1F)));

	while (length--)
	{
		// Small delay before the RTC device is accessible
		soft_usecWait(RTC_ACCESS_DELAY);

		spi_write(&AVR32_SPI1, 0xFF);
		spi_read(&AVR32_SPI1, &dataContainer);

		*data++ = (uint8)dataContainer;
	}

	spi_unselectChip(&AVR32_SPI1, RTC_SPI_NPCS);
}

#if 0 // Add / Fix later
/****************************************
*	Function:    
*	Purpose:
****************************************/
static DATE_TIME_STRUCT timestampEventCache[10];
static uint16 timestampEventIndex = 0;

void timestampEvent(void)
{
	if (timestampEventFlag = YES)
	{
		
	}
}
#endif