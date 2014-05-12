///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: TimerMode.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:01 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/TimerMode.c,v $
///	$Revision: 1.2 $
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
#include "Record.h"
#include "Menu.h"
#include "TextTypes.h"
#include "PowerManagement.h"
#include "RealTimeClock.h"
#include "SysEvents.h"
#include "Keypad.h"
#include "Uart.h"

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

/****************************************
*	Function:    timerModeActiveCheck
*	Purpose:
****************************************/
BOOLEAN timerModeActiveCheck(void)
{
	BOOLEAN status = FALSE;
	DATE_TIME_STRUCT time = getRtcTime();
	uint8 choice;

	if (g_helpRecord.encode_ln == 0xA5A5)
	{
		// Check if timer mode is enabled and if the time of day alarm flag is set
		if (g_helpRecord.timer_mode == ENABLED)
		{
			debug("Timer Mode active\n");

			if ((g_helpRecord.tm_start_time.hour == time.hour) && (g_helpRecord.tm_start_time.min == time.min))
			{
				debug("Timer Mode Check: Matched Timer settings...\n");
				status = TRUE;
			}
			else
			{
				messageBox(getLangText(STATUS_TEXT), getLangText(UNIT_IS_IN_TIMER_MODE_TEXT), MB_OK);
				choice = messageBox(getLangText(WARNING_TEXT), getLangText(CANCEL_TIMER_MODE_Q_TEXT), MB_YESNO);

				if (choice == MB_FIRST_CHOICE)
				{
					g_helpRecord.timer_mode = DISABLED;

					// Save help record
					saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

					overlayMessage(getLangText(STATUS_TEXT), getLangText(TIMER_MODE_DISABLED_TEXT), 2 * SOFT_SECS);
				}
				else // User decided to stay in Timer mode
				{
					// TOD Alarm registers still set from previous run, TOD Alarm Mask re-enabled in rtc init
					// Clear TOD Alarm Mask to allow TOD Alarm interrupt to be generated
					// ADD CODE TO CLEAR ALARM

					overlayMessage(getLangText(WARNING_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), 2 * SOFT_SECS);

					// Turn unit off/sleep
					debug("Timer mode: Staying in Timer mode. Powering off now...\n");
					PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
				}
			}
		}
	}

#if 0 // ns7100
	// Might as well clear flag reguardless of state
	RTC_FLAGS.reg = RTC_FLAGS.reg;
#else // ns8100
	// fix_ns8100
#endif

	return (status);
}

/****************************************
*	Function:    processTimerMode
*	Purpose:
****************************************/
void processTimerMode(void)
{
	DATE_TIME_STRUCT currTime = getRtcTime();

	// Check if the Timer mode activated after stop date
	if (	// First Check for past year
		(currTime.year > g_helpRecord.tm_stop_date.year) ||

		// Second check for equal year but past month
		((currTime.year == g_helpRecord.tm_stop_date.year) && (currTime.month > g_helpRecord.tm_stop_date.month)) ||

		// Third check for equal year, equal month, but past day
		((currTime.year == g_helpRecord.tm_stop_date.year) && (currTime.month == g_helpRecord.tm_stop_date.month) &&
		(currTime.day > g_helpRecord.tm_stop_date.day)))
	{
		// Disable alarm output generation
		debug("Timer Mode: Activated after date...\n");
		debug("Timer Mode: Disabling...\n");
		g_helpRecord.timer_mode = DISABLED;

		// Save help record
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

	    // Deactivate alarm interrupts
	    DisableRtcAlarm();

		// Turn unit off/sleep
		debug("Timer mode: Powering unit off...\n");
		PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
	}
	// Check if the Timer mode activated before start date
	else if (// First Check for before year
		(currTime.year < g_helpRecord.tm_start_date.year) ||

		// Second check for equal year but before month
		((currTime.year == g_helpRecord.tm_start_date.year) && (currTime.month < g_helpRecord.tm_start_date.month)) ||

		// Third check for equal year, equal month, but before day
		((currTime.year == g_helpRecord.tm_start_date.year) && (currTime.month == g_helpRecord.tm_start_date.month) &&
		(currTime.day < g_helpRecord.tm_start_date.day)))
	{
		debug("Timer Mode: Activated before date...\n");
		resetTimeOfDayAlarm();

		// Turn unit off/sleep
		debug("Timer mode: Powering unit off...\n");
		PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
	}
	// Check if the Timer mode activated during active dates but on an off day
	else if ((g_helpRecord.timer_mode_freq == TIMER_MODE_WEEKDAYS) && ((currTime.weekday == SAT) || (currTime.weekday == SUN)))
	{
		debug("Timer Mode: Activated on an off day (weekday freq)...\n");
		resetTimeOfDayAlarm();

		// Turn unit off/sleep
		debug("Timer mode: Powering unit off...\n");
		PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
	}
	// Check if the Timer mode activated during active dates but on an off day
	else if ((g_helpRecord.timer_mode_freq == TIMER_MODE_MONTHLY) && (currTime.day != g_helpRecord.tm_start_date.day))
	{
		debug("Timer Mode: Activated on off day (monthly freq)...\n");
		resetTimeOfDayAlarm();

		// Turn unit off/sleep
		debug("Timer mode: Powering unit off...\n");
		PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
	}
	// Check if the Timer mode activated on end date
	else if ((currTime.year == g_helpRecord.tm_stop_date.year) && (currTime.month == g_helpRecord.tm_stop_date.month) &&
		(currTime.day == g_helpRecord.tm_stop_date.day))
	{
		// Disable alarm output generation
		debug("Timer Mode: Activated on end date...\n");
		debug("Timer Mode: Disabling...\n");
		g_helpRecord.timer_mode = DISABLED;

		// Save help record
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

	    // Deactivate alarm interrupts
	    DisableRtcAlarm();
	}
	else // Timer mode started during the active dates on a day it's supposed to run
	{
		// Reset the Time of Day Alarm to wake the unit up again
		resetTimeOfDayAlarm();
	}

	overlayMessage(getLangText(STATUS_TEXT), getLangText(TIMER_MODE_NOW_ACTIVE_TEXT), (2 * SOFT_SECS));

	raiseTimerEventFlag(TIMER_MODE_TIMER_EVENT);

	// Setup soft timer to turn system off when timer mode is finished for the day
	assignSoftTimer(POWER_OFF_TIMER_NUM, (uint32)(g_helpRecord.timer_mode_active_minutes * 60 * 2), powerOffTimerCallback);

	debug("Timer mode: running...\n");
}

/****************************************
*	Function:    handleUserPowerOffDuringTimerMode
*	Purpose:
****************************************/
void handleUserPowerOffDuringTimerMode(void)
{
	INPUT_MSG_STRUCT mn_msg;
	uint8 choice;

	// Simulate a keypress if the user pressed the off key, which doesn't register as a keypess
	keypressEventMgr();

	messageBox(getLangText(STATUS_TEXT), getLangText(UNIT_IS_IN_TIMER_MODE_TEXT), MB_OK);
	choice = messageBox(getLangText(WARNING_TEXT), getLangText(CANCEL_TIMER_MODE_Q_TEXT), MB_YESNO);

	if (choice == MB_FIRST_CHOICE)
	{
		g_helpRecord.timer_mode = DISABLED;

		// Disable the Power Off timer
		clearSoftTimer(POWER_OFF_TIMER_NUM);

		// Save help record
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		// Re-Enable the Power Off key
		powerControl(POWER_SHUTDOWN_ENABLE, ON);

		overlayMessage(getLangText(STATUS_TEXT), getLangText(TIMER_MODE_DISABLED_TEXT), 2 * SOFT_SECS);
	}
	else // User decided to stay in Timer mode
	{
		choice = messageBox(getLangText(STATUS_TEXT), getLangText(POWER_UNIT_OFF_EARLY_Q_TEXT), MB_YESNO);

		if (choice == MB_FIRST_CHOICE)
		{
			messageBox(getLangText(STATUS_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), MB_OK);
			messageBox(getLangText(STATUS_TEXT), getLangText(PLEASE_PRESS_ENTER_TEXT), MB_OK);

			// Re-Enable the Power Off key
			powerControl(POWER_SHUTDOWN_ENABLE, ON);

			// Turn unit off/sleep
			debug("Timer mode: Shutting down unit early due to user request. Powering off now...\n");
			PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
		}
	}

	g_activeMenu = MAIN_MENU;
	ACTIVATE_MENU_MSG();
	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

/****************************************
*	Function:    resetTimeOfDayAlarm
*	Purpose:
****************************************/
void resetTimeOfDayAlarm(void)
{
	DATE_TIME_STRUCT currTime = getRtcTime();
	uint8 startDay = 0;
	uint8 month;

	// RTC Weekday's: Sunday = 1, Monday = 2, Tuesday = 3, Wednesday = 4, Thursday = 5, Friday = 6, Saturday = 7

	if (g_helpRecord.timer_mode_freq == TIMER_MODE_DAILY)
	{
		// Get the current day and add one
		startDay = (uint8)(currTime.day + 1);

		// Check if the start day is beyond the total days in the current month
		if (startDay > g_monthTable[currTime.month].days)
		{
			// Set the start day to the first of next month
			startDay = 1;
		}

		debug("Timer mode: Resetting TOD Alarm with (hour) %d, (min) %d, (start day) %d\n",
				g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, startDay);

		EnableRtcAlarm(startDay, g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, 0);
	}
	else if (g_helpRecord.timer_mode_freq == TIMER_MODE_WEEKDAYS)
	{
		// Get the current day
		startDay = currTime.day;

		if (currTime.weekday == FRI)
		{
			// Advance 3 days
			startDay += 3;
		}
		else if (currTime.weekday == SAT)
		{
			// Advance 2 days
			startDay += 2;
		}
		else // The rest of the days of the week
		{
			// Advance 1 day
			startDay++;
		}

		// Check if the start day is beyond the number of days in the current month
		if (startDay > g_monthTable[currTime.month].days)
		{
			// Subtract out the number of days in the current month to get the start day for next month
			startDay -= g_monthTable[currTime.month].days;
		}

		debug("Timer mode: Resetting TOD Alarm with (hour) %d, (min) %d, (start ay) %d\n",
				g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, startDay);

		EnableRtcAlarm(startDay, g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, 0);
	}
	else if (g_helpRecord.timer_mode_freq == TIMER_MODE_WEEKLY)
	{
		// Set the start day to the current day + 7 days
		startDay = (uint8)(currTime.day + 7);

		// Check if the start day is beyond the number of days in the current month
		if (startDay > g_monthTable[currTime.month].days)
		{
			// Subtract out the days of the current month to get the start day for next month
			startDay -= g_monthTable[currTime.month].days;
		}

		debug("Timer mode: Resetting TOD Alarm with (hour) %d, (min) %d, (start day) %d\n",
				g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, startDay);

		EnableRtcAlarm(startDay, g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, 0);
	}
	else if (g_helpRecord.timer_mode_freq == TIMER_MODE_MONTHLY)
	{
		// Check if advancing a month is still within the same year
		if (currTime.month + 1 <= 12)
		{
			month = (uint8)(currTime.month + 1);
		}
		else // It's December and next month is Jan
		{
			month = 1;
		}

		// Get the current day
		startDay = currTime.day;

		// Check if the stat day is greater than the total number of days in the next month
		if (startDay > g_monthTable[month].days)
		{
			// Limit the start day to the last day of the next month
			startDay = (uint8)(g_monthTable[month].days);
		}

		debug("Timer mode: Resetting TOD Alarm with (hour) %d, (min) %d, (start day) %d\n",
				g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, startDay);

		EnableRtcAlarm(startDay, g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min, 0);
	}
}
