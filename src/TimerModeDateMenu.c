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
#include <string.h>
#include "Common.h"
#include "Typedefs.h"
#include "Menu.h"
#include "Uart.h"
#include "Display.h"
#include "RealTimeClock.h"
#include "PowerManagement.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define TIMER_MODE_DATE_MN_SIZE_EXTENSION
#define TIMER_MODE_DATE_MN_TABLE_SIZE 8
#define TIMER_MODE_DATE_WND_STARTING_COL 3
#define TIMER_MODE_DATE_WND_END_COL 127
#define TIMER_MODE_DATE_WND_STARTING_ROW DEFAULT_MENU_ROW_THREE
#define TIMER_MODE_DATE_WND_END_ROW DEFAULT_MENU_ROW_SIX
#define TIMER_MODE_DATE_MN_TBL_START_LINE 0
#define TMD_START_DAY	0
#define TMD_START_MONTH	1
#define TMD_START_YEAR	2
#define TMD_STOP_DAY	3
#define TMD_STOP_MONTH	4
#define TMD_STOP_YEAR	5
#define END_DATA_STRING_SIZE 6
#define MIN_CHAR_STRING 0x41
#define MIN_NUM_STRING 0x2E
#define MAX_CHAR_STRING 0x5a
#define MAX_NUM_STRING 0x39
#define INPUT_MENU_SECOND_LINE_INDENT 6
#define MAX_CHARS_PER_LINE 20
#define DATE_TIME_MN_WND_LNS 6
#define IN_PROGRESS	2

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT configMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void TimerModeDateMenu(INPUT_MSG_STRUCT);
void TimerModeDateMenuProc(INPUT_MSG_STRUCT, REC_MN_STRUCT*, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void TimerModeDateMenuDisplay(REC_MN_STRUCT*, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void LoadTimerModeDateMnDefRec(REC_MN_STRUCT*, DATE_TIME_STRUCT*);
void TimerModeDateMenuDvScroll(char dir_key, REC_MN_STRUCT*);
void TimerModeActiveMinutes(void);
void TimerModeDateMenuScroll(char, MN_LAYOUT_STRUCT*);
uint8 ValidateTimerModeSettings(void);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TimerModeDateMenu (INPUT_MSG_STRUCT msg)
{
	static WND_LAYOUT_STRUCT wnd_layout;
	static MN_LAYOUT_STRUCT mn_layout;
	static REC_MN_STRUCT mn_rec[6];

	TimerModeDateMenuProc(msg, mn_rec, &wnd_layout, &mn_layout);

	if (g_activeMenu == TIMER_MODE_DATE_MENU)
	{
		TimerModeDateMenuDisplay(mn_rec, &wnd_layout, &mn_layout);
		WriteMapToLcd(g_mmap);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TimerModeDateMenuProc(INPUT_MSG_STRUCT msg, REC_MN_STRUCT *rec_ptr, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	DATE_TIME_STRUCT time;
	uint32 input;
	//uint8 dayOfWeek = 0;

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col = TIMER_MODE_DATE_WND_STARTING_COL;
			wnd_layout_ptr->end_col = TIMER_MODE_DATE_WND_END_COL;
			wnd_layout_ptr->start_row = TIMER_MODE_DATE_WND_STARTING_ROW;
			wnd_layout_ptr->end_row = TIMER_MODE_DATE_WND_END_ROW;

			mn_layout_ptr->curr_ln = 0;
			mn_layout_ptr->top_ln = 0;
			mn_layout_ptr->sub_ln = 0;

			time = GetCurrentTime();
			LoadTimerModeDateMnDefRec(rec_ptr, &time);
			rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
			break;

		case (KEYPRESS_MENU_CMD):

			input = msg.data[0];
			switch (input)
			{
				case (ENTER_KEY):
					g_unitConfig.timerStartDate.day = (char)rec_ptr[TMD_START_DAY].numrec.tindex;
					g_unitConfig.timerStartDate.month = (char)rec_ptr[TMD_START_MONTH].numrec.tindex;
					g_unitConfig.timerStartDate.year = (char)rec_ptr[TMD_START_YEAR].numrec.tindex;
					g_unitConfig.timerStopDate.day = (char)rec_ptr[TMD_STOP_DAY].numrec.tindex;
					g_unitConfig.timerStopDate.month = (char)rec_ptr[TMD_STOP_MONTH].numrec.tindex;
					g_unitConfig.timerStopDate.year = (char)rec_ptr[TMD_STOP_YEAR].numrec.tindex;

					ProcessTimerModeSettings(PROMPT);

					SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
					JUMP_TO_ACTIVE_MENU();
					break;
				case (DOWN_ARROW_KEY):
						if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
						{
						TimerModeDateMenuDvScroll(DOWN, &rec_ptr[mn_layout_ptr->curr_ln]);
						}
						break;
				case (UP_ARROW_KEY):
						if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
						{
						TimerModeDateMenuDvScroll(UP, &rec_ptr[mn_layout_ptr->curr_ln]);
						}
						break;
				case (PLUS_KEY):
						rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
						TimerModeDateMenuScroll(DOWN, mn_layout_ptr);
						rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
						break;
				case (MINUS_KEY):
						rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
						TimerModeDateMenuScroll(UP, mn_layout_ptr);
						rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
						break;
				case (ESC_KEY):
						SETUP_MENU_MSG(TIMER_MODE_TIME_MENU);
						JUMP_TO_ACTIVE_MENU();
						break;
				default:
						break;
			}
			break;

		default:
			break;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TimerModeDateMenuScroll(char direction, MN_LAYOUT_STRUCT* mn_layout_ptr)
{
	switch (direction)
	{
		case (DOWN):
			if (mn_layout_ptr->curr_ln < 5)
			{
				mn_layout_ptr->curr_ln++;
			}
		break;

		case (UP):
			if (mn_layout_ptr->curr_ln > 0)
			{
				mn_layout_ptr->curr_ln--;
			}
		break;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TimerModeDateMenuDvScroll(char dir_key, REC_MN_STRUCT *rec_ptr)
{
	switch (dir_key)
	{
		case (UP):
			if (rec_ptr->numrec.tindex < rec_ptr->numrec.nmax)
			{
				rec_ptr->numrec.tindex += 1;
			}
			else
			{
				rec_ptr->numrec.tindex = rec_ptr->numrec.nmin;
			}
			break;

		case (DOWN):
			if (rec_ptr->numrec.tindex > rec_ptr->numrec.nmin)
			{
				rec_ptr->numrec.tindex -= 1;
			}
			else
			{
				rec_ptr->numrec.tindex = rec_ptr->numrec.nmax;
			}
			break;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TimerModeDateMenuDisplay(REC_MN_STRUCT *rec_ptr, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	uint8 sbuff[50];
	uint8 top;
	uint8 menu_ln;
	uint8 length = 0;

	memset(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	menu_ln = 0;
	top = (uint8)mn_layout_ptr->top_ln;

	// Add in a title for the menu
	memset(&sbuff[0], 0, sizeof(sbuff));
	sprintf((char*)sbuff, "-%s-", getLangText(ACTIVE_DATE_PERIOD_TEXT));
	length = (uint8)strlen((char*)sbuff);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// Add in a title for the menu
	memset(&sbuff[0], 0, sizeof(sbuff));
	sprintf((char*)sbuff, "(%s)", getLangText(DAY_MONTH_YEAR_TEXT));
	length = (uint8)strlen((char*)sbuff);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	memset(&sbuff[0], 0, sizeof(sbuff));

	wnd_layout_ptr->curr_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col = wnd_layout_ptr->start_col;

	rec_ptr[TMD_START_DAY].numrec.nmax = GetDaysPerMonth((uint8)(rec_ptr[TMD_START_MONTH].numrec.tindex),
																(uint8)(rec_ptr[TMD_START_YEAR].numrec.tindex));
	if (rec_ptr[TMD_START_DAY].numrec.tindex > rec_ptr[TMD_START_DAY].numrec.nmax)
		rec_ptr[TMD_START_DAY].numrec.tindex = rec_ptr[TMD_START_DAY].numrec.nmax;

	rec_ptr[TMD_STOP_DAY].numrec.nmax = GetDaysPerMonth((uint8)(rec_ptr[TMD_STOP_MONTH].numrec.tindex),
																(uint8)(rec_ptr[TMD_STOP_YEAR].numrec.tindex));
	if (rec_ptr[TMD_STOP_DAY].numrec.tindex > rec_ptr[TMD_STOP_DAY].numrec.nmax)
		rec_ptr[TMD_STOP_DAY].numrec.tindex = rec_ptr[TMD_STOP_DAY].numrec.nmax;

	// ---------------------------------------------------------------------------------
	// Write out the start date line which includes the start date text, day, month, and year values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;

	// Display the start date text
	sprintf((char*)sbuff, "%s: ", getLangText(START_DATE_TEXT));
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((9 * SIX_COL_SIZE)/2));

	// Display the day
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_START_DAY].numrec.tindex);
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_START_DAY) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	WndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the month text
	sprintf((char*)sbuff, "%s", (char*)&(g_monthTable[(uint8)(rec_ptr[TMD_START_MONTH].numrec.tindex)].name[0]));
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_START_MONTH) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	WndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the year
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_START_YEAR].numrec.tindex);
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_START_YEAR) ? CURSOR_LN : REG_LN);

	// ---------------------------------------------------------------------------------
	// Write out the stop date line which includes the stop date text, day, month, and year values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;

	// Display the date text
	sprintf((char*)sbuff, "%s: ", getLangText(STOP_DATE_TEXT));
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((9 * SIX_COL_SIZE)/2));

	// Display the day
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_STOP_DAY].numrec.tindex);
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_STOP_DAY) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	WndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the month text
	sprintf((char*)sbuff, "%s", (char*)&(g_monthTable[(uint8)(rec_ptr[TMD_STOP_MONTH].numrec.tindex)].name[0]));
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_STOP_MONTH) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	WndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the year
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_STOP_YEAR].numrec.tindex);
	WndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_STOP_YEAR) ? CURSOR_LN : REG_LN);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void LoadTimerModeDateMnDefRec(REC_MN_STRUCT *rec_ptr, DATE_TIME_STRUCT *time_ptr)
{
	// START DAY
	rec_ptr[TMD_START_DAY].enterflag = FALSE;
	rec_ptr[TMD_START_DAY].type = INPUT_NUM_STRING;
	rec_ptr[TMD_START_DAY].wrapflag = FALSE;
	rec_ptr[TMD_START_DAY].numrec.nindex = 0;
	rec_ptr[TMD_START_DAY].numrec.nmax = 31;
	rec_ptr[TMD_START_DAY].numrec.nmin = 1;
	rec_ptr[TMD_START_DAY].numrec.incr_value = 1;
	rec_ptr[TMD_START_DAY].numrec.tindex = time_ptr->day;
	rec_ptr[TMD_START_DAY].numrec.num_type = FIXED_TIME_TYPE_DAY;

	// START MONTH
	rec_ptr[TMD_START_MONTH].enterflag = FALSE;
	rec_ptr[TMD_START_MONTH].type = INPUT_NUM_STRING;
	rec_ptr[TMD_START_MONTH].wrapflag = FALSE;
	rec_ptr[TMD_START_MONTH].numrec.nindex = 0;
	rec_ptr[TMD_START_MONTH].numrec.nmax = 12;
	rec_ptr[TMD_START_MONTH].numrec.nmin = 1;
	rec_ptr[TMD_START_MONTH].numrec.incr_value = 1;
	rec_ptr[TMD_START_MONTH].numrec.tindex = time_ptr->month;
	rec_ptr[TMD_START_MONTH].numrec.num_type = FIXED_TIME_TYPE_MONTH;

	// START YEAR
	rec_ptr[TMD_START_YEAR].enterflag = FALSE;
	rec_ptr[TMD_START_YEAR].type = INPUT_NUM_STRING;
	rec_ptr[TMD_START_YEAR].wrapflag = FALSE;
	rec_ptr[TMD_START_YEAR].numrec.nindex = 0;
	rec_ptr[TMD_START_YEAR].numrec.nmax = 99;
	rec_ptr[TMD_START_YEAR].numrec.nmin = 0;
	rec_ptr[TMD_START_YEAR].numrec.incr_value = 1;
	rec_ptr[TMD_START_YEAR].numrec.tindex = time_ptr->year;
	rec_ptr[TMD_START_YEAR].numrec.num_type = FIXED_TIME_TYPE_YEAR;

	// STOP DAY
	rec_ptr[TMD_STOP_DAY].enterflag = FALSE;
	rec_ptr[TMD_STOP_DAY].type = INPUT_NUM_STRING;
	rec_ptr[TMD_STOP_DAY].wrapflag = FALSE;
	rec_ptr[TMD_STOP_DAY].numrec.nindex = 0;
	rec_ptr[TMD_STOP_DAY].numrec.nmax = 31;
	rec_ptr[TMD_STOP_DAY].numrec.nmin = 1;
	rec_ptr[TMD_STOP_DAY].numrec.incr_value = 1;
	rec_ptr[TMD_STOP_DAY].numrec.tindex = time_ptr->day;
	rec_ptr[TMD_STOP_DAY].numrec.num_type = FIXED_TIME_TYPE_DAY;

	// STOP MONTH
	rec_ptr[TMD_STOP_MONTH].enterflag = FALSE;
	rec_ptr[TMD_STOP_MONTH].type = INPUT_NUM_STRING;
	rec_ptr[TMD_STOP_MONTH].wrapflag = FALSE;
	rec_ptr[TMD_STOP_MONTH].numrec.nindex = 0;
	rec_ptr[TMD_STOP_MONTH].numrec.nmax = 12;
	rec_ptr[TMD_STOP_MONTH].numrec.nmin = 1;
	rec_ptr[TMD_STOP_MONTH].numrec.incr_value = 1;
	rec_ptr[TMD_STOP_MONTH].numrec.tindex = time_ptr->month;
	rec_ptr[TMD_STOP_MONTH].numrec.num_type = FIXED_TIME_TYPE_MONTH;

	// STOP YEAR
	rec_ptr[TMD_STOP_YEAR].enterflag = FALSE;
	rec_ptr[TMD_STOP_YEAR].type = INPUT_NUM_STRING;
	rec_ptr[TMD_STOP_YEAR].wrapflag = FALSE;
	rec_ptr[TMD_STOP_YEAR].numrec.nindex = 0;
	rec_ptr[TMD_STOP_YEAR].numrec.nmax = 99;
	rec_ptr[TMD_STOP_YEAR].numrec.nmin = 0;
	rec_ptr[TMD_STOP_YEAR].numrec.incr_value = 1;
	rec_ptr[TMD_STOP_YEAR].numrec.tindex = (time_ptr->year + 1);
	rec_ptr[TMD_STOP_YEAR].numrec.num_type = FIXED_TIME_TYPE_YEAR;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TimerModeActiveMinutes(void)
{
	// Find the Time mode active time period in minutes

	// Check for specialty case hourly
	if (g_unitConfig.timerModeFrequency == TIMER_MODE_HOURLY)
	{
		// Check if the stop min is greater than the start min
		if (g_unitConfig.timerStopTime.min > g_unitConfig.timerStartTime.min)
		{
			g_unitConfig.TimerModeActiveMinutes = (g_unitConfig.timerStopTime.min - g_unitConfig.timerStartTime.min);
		}
		else // The timer mode hourly active period will cross the hour boundary
		{
			g_unitConfig.TimerModeActiveMinutes = (60 + g_unitConfig.timerStopTime.min - g_unitConfig.timerStartTime.min);
		}
		
		// In order to restart up every hour, the active minutes needs to be less than 60, 1 min for shutdown + 1 min for being off
		if (g_unitConfig.TimerModeActiveMinutes > 58)
			g_unitConfig.TimerModeActiveMinutes = 58;
	}
	// Check if the stop time (in minutes resolution) is greater than the start time, thus running the same day
	else if (((g_unitConfig.timerStopTime.hour * 60) + g_unitConfig.timerStopTime.min) >
		((g_unitConfig.timerStartTime.hour * 60) + g_unitConfig.timerStartTime.min))
	{
		// Set active minutes as the difference in the stop and start times
		g_unitConfig.TimerModeActiveMinutes = (uint16)(((g_unitConfig.timerStopTime.hour * 60) + g_unitConfig.timerStopTime.min) -
												((g_unitConfig.timerStartTime.hour * 60) + g_unitConfig.timerStartTime.min));

		// Check for specialty case one time
		if (g_unitConfig.timerModeFrequency == TIMER_MODE_ONE_TIME)
		{
			// Calculate the number of days to run consecutively (Days * Hours in a day * Minutes in an hour) and add to the active minutes
			g_unitConfig.TimerModeActiveMinutes += (24 * 60 * (GetTotalDaysFromReference(g_unitConfig.timerStopDate) -
														GetTotalDaysFromReference(g_unitConfig.timerStartDate)));
		}
	}
	else // The timer mode active period will see midnight, thus running 2 consecutive days
	{
		// Set active minutes as the difference from midnight and the start time, plus the stop time the next day
		g_unitConfig.TimerModeActiveMinutes = (uint16)((24 * 60) - ((g_unitConfig.timerStartTime.hour * 60) + g_unitConfig.timerStartTime.min) +
												((g_unitConfig.timerStopTime.hour * 60) + g_unitConfig.timerStopTime.min));

		// Check for specialty case one time
		if (g_unitConfig.timerModeFrequency == TIMER_MODE_ONE_TIME)
		{
			// Calculate the number of days to run consecutively (Days * Hours in a day * Minutes in an hour) minus crossover day and add to the active minutes
			g_unitConfig.TimerModeActiveMinutes += (24 * 60 * (GetTotalDaysFromReference(g_unitConfig.timerStopDate) -
														GetTotalDaysFromReference(g_unitConfig.timerStartDate) - 1));
		}
	}

	debug("Timer Active Minutes: %d\r\n", g_unitConfig.TimerModeActiveMinutes);

	SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ValidateTimerModeSettings(void)
{
	DATE_TIME_STRUCT time = GetCurrentTime();

	char start_min = g_unitConfig.timerStartTime.min;
	char start_hour = g_unitConfig.timerStartTime.hour;
	char stop_min = g_unitConfig.timerStopTime.min;
	char stop_hour = g_unitConfig.timerStopTime.hour;
	char start_day = g_unitConfig.timerStartDate.day;
	char start_month = g_unitConfig.timerStartDate.month;
	char start_year = g_unitConfig.timerStartDate.year;
	char stop_day = g_unitConfig.timerStopDate.day;
	char stop_month = g_unitConfig.timerStopDate.month;
	char stop_year = g_unitConfig.timerStopDate.year;

	debug("Timer Date: (Start) %d/%d/%d -> (End) %d/%d/%d\r\n", start_month, start_day, start_year, stop_month, stop_day, stop_year);

	// Check if user picked a start date that is before the current day
	if ((start_year < time.year) ||
		((start_year == time.year) && (start_month < time.month)) ||
		((start_year == time.year) && (start_month == time.month) && (start_day < time.day)))
	{
		return (FAILED);
	}

	// Check if user picked a stop date that is before a start date
	if ((stop_year < start_year) ||
		((stop_year == start_year) && (stop_month < start_month)) ||
		((stop_year == start_year) && (stop_month == start_month) && (stop_day < start_day)))
	{
		return (FAILED);
	}

	// Check if start date and end date are the same (one-time freq)
	if ((stop_year == start_year) && (stop_month == start_month) && (stop_day == start_day))
	{
		// Check if the user specified the unit to run past midnight resulting in a second day
		if ((stop_hour < start_hour) || ((stop_hour == start_hour) && (stop_min < start_min)))
		{
			return (FAILED);
		}
	}

	// Check if start time and end time are the same
	if ((start_hour == stop_hour) && (start_min == stop_min))
	{
		return (FAILED);
	}

	// Check if start date is the current day
	if ((start_year == time.year) && (start_month == time.month) && (start_day == time.day))
	{
		// Check if the user specified the unit to run before the current time
		if ((start_hour < time.hour) || ((start_hour == time.hour) && (start_min <= time.min)))
		{
			// Check if the current time is less than the end time, signaling that timer mode is in progress
			if ((time.hour < stop_hour) || ((time.hour == stop_hour) && (time.min < stop_min)))
			{
				return (IN_PROGRESS);
			}
			else // Timer mode setting would have already completed, thus the current day is an invalid start day
			{
				return (FAILED);
			}
		}
	}

	return (PASSED);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ProcessTimerModeSettings(uint8 mode)
{
	//uint8 dayOfWeek = 0;
	uint8 startDay = 0;
	uint8 startHour = 0;
	char stringBuff[75];
	uint16 minutesLeft = 0;
	DATE_TIME_STRUCT currentTime = GetCurrentTime();
	uint8 status = ValidateTimerModeSettings();

	// Check if the timer mode settings check failed or if the timer mode setting has been disabled
	if ((status == FAILED) || (g_unitConfig.timerMode == DISABLED))
	{
		g_unitConfig.timerMode = DISABLED;
		SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);

		// Disable the Power Off timer in case it's set
		ClearSoftTimer(POWER_OFF_TIMER_NUM);

		if (mode == PROMPT)
		{
			memset(&stringBuff[0], 0, sizeof(stringBuff));
			sprintf(stringBuff, "%s %s", getLangText(TIMER_SETTINGS_INVALID_TEXT), getLangText(TIMER_MODE_DISABLED_TEXT));
			MessageBox(getLangText(ERROR_TEXT), stringBuff, MB_OK);
		}
	}
	else // status == PASSED || status == IN_PROGRESS
	{
		// Calculate timer mode active run time in minutes
		TimerModeActiveMinutes();

		// Init start day based on the start date provided by the user
		startDay = g_unitConfig.timerStartDate.day;

		// Check if in progress, requiring extra logic to determine alarm settings
		if (status == IN_PROGRESS)
		{
			// Check if the stop time is greater than the start time
			if ((g_unitConfig.timerStopTime.hour > g_unitConfig.timerStartTime.hour) ||
				((g_unitConfig.timerStopTime.hour == g_unitConfig.timerStartTime.hour) &&
				(g_unitConfig.timerStopTime.min > g_unitConfig.timerStartTime.min)))
			{
				// Advance the start day
				startDay++;

				// Check if the start day is beyond the total days in the current month
				if (startDay > g_monthTable[(uint8)(g_unitConfig.timerStartDate.month)].days)
				{
					// Set the start day to the first day of next month
					startDay = 1;
				}
			}
		}

		// Check for specialty case hourly mode and in progress, requiring extra logic to determine alarm settings
		if ((g_unitConfig.timerModeFrequency == TIMER_MODE_HOURLY) && (status == IN_PROGRESS))
		{
			// Check if another hour time slot to run again today
			if (currentTime.hour != g_unitConfig.timerStopTime.hour)
			{
				// Start day remains the same
				startDay = g_unitConfig.timerStartDate.day;

				// Check if current hour time slot has not started
				if (currentTime.min < g_unitConfig.timerStartTime.min)
				{
					// Set alarm for the same hour
					startHour = currentTime.hour;
				}
				else
				{
					// Set alarm for the next hour
					startHour = currentTime.hour + 1;
					
					// Account for end of day boundary
					if (startHour > 23)
					{
						startHour = 0;

						// Advance the start day
						startDay++;

						// Check if the start day is beyond the total days in the current month
						if (startDay > g_monthTable[(uint8)(g_unitConfig.timerStartDate.month)].days)
						{
							// Set the start day to the first day of next month
							startDay = 1;
						}
					}					
				}
				
				EnableExternalRtcAlarm(startDay, startHour, g_unitConfig.timerStartTime.min, 0);
			}
			else // This is the last hour time slot to run today, set alarm for next day
			{
				// startDay calculated correctly in above previous status == IN_PROGRESS logic
				EnableExternalRtcAlarm(startDay, g_unitConfig.timerStartTime.hour, g_unitConfig.timerStartTime.min, 0);
			}
		}
		else // All other timer modes
		{
			EnableExternalRtcAlarm(startDay, g_unitConfig.timerStartTime.hour, g_unitConfig.timerStartTime.min, 0);
		}

		if (status == PASSED)
		{
			if (mode == PROMPT)
			{
				memset(&stringBuff[0], 0, sizeof(stringBuff));
				sprintf(stringBuff, "%s %s", getLangText(TIMER_MODE_NOW_ACTIVE_TEXT), getLangText(PLEASE_POWER_OFF_UNIT_TEXT));
				MessageBox(getLangText(STATUS_TEXT), stringBuff, MB_OK);
			}

			// Check if start time is greater than the current time
			if (((g_unitConfig.timerStartTime.hour * 60) + g_unitConfig.timerStartTime.min) >
				((currentTime.hour * 60) + currentTime.min))
			{
				// Take the difference between start time and current time
				minutesLeft = (uint16)(((g_unitConfig.timerStartTime.hour * 60) + g_unitConfig.timerStartTime.min) -
							((currentTime.hour * 60) + currentTime.min));
			}
			else // Current time is after the start time, meaning the start time is the next day
			{
				// Take the difference between 24 hours and the current time plus the start time
				minutesLeft = (uint16)((24 * 60) - ((currentTime.hour * 60) + currentTime.min) +
							((g_unitConfig.timerStartTime.hour * 60) + g_unitConfig.timerStartTime.min));
			}

			// Check if the start time is within the next minute
			if (minutesLeft <= 1)
			{
				OverlayMessage(getLangText(WARNING_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), 2 * SOFT_SECS);

				// Need to shutdown the unit now, otherwise the start time window will be missed
				PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
			}
			else // More than 1 minute left before the start time
			{
				// Make sure the unit turns off one minute before the start time if the user forgets to turn the unit off
				minutesLeft -= 1;

				// Set the Power off soft timer to prevent the unit from staying on past the Timer mode start time
				AssignSoftTimer(POWER_OFF_TIMER_NUM, (uint32)(minutesLeft * 60 * 2), PowerOffTimerCallback);
			}
		}
		else // status == IN_PROGRESS
		{
			if (mode == PROMPT)
			{
				memset(&stringBuff[0], 0, sizeof(stringBuff));
				sprintf(stringBuff, "%s", getLangText(TIMER_MODE_NOW_ACTIVE_TEXT));
				MessageBox(getLangText(STATUS_TEXT), stringBuff, MB_OK);
			}

			// Check if specialty mode hourly
			if (g_unitConfig.timerModeFrequency == TIMER_MODE_HOURLY)
			{
				if (currentTime.min	< g_unitConfig.timerStopTime.min)
				{
					minutesLeft = (g_unitConfig.timerStopTime.min - currentTime.min);
				}
				else
				{
					minutesLeft = (60 + g_unitConfig.timerStopTime.min - currentTime.min);
				}
				
				if (minutesLeft > 58)
					minutesLeft = 58;
			}
			// Check if the current time is greater than the stop time, indicating that midnight boundary was crossed
			else if (((currentTime.hour * 60) + currentTime.min) > ((g_unitConfig.timerStopTime.hour * 60) + g_unitConfig.timerStopTime.min))
			{
				// Calculate the time left before powering off to be 24 + the stop time minus the current time
				minutesLeft = (uint16)(((24 * 60) + (g_unitConfig.timerStopTime.hour * 60) + g_unitConfig.timerStopTime.min) -
								((currentTime.hour * 60) + currentTime.min));
			}
			else // Current time is less than start time, operating within the same day
			{
				// Calculate the time left before powering off to be the stop time minus the current time
				minutesLeft = (uint16)(((g_unitConfig.timerStopTime.hour * 60) + g_unitConfig.timerStopTime.min) -
								((currentTime.hour * 60) + currentTime.min));
			}

			// Make sure timeout value is not zero
			if (minutesLeft == 0) minutesLeft = 1;

			debug("Timer Mode: In progress, minutes left before power off: %d (Expired secs this min: %d)\r\n", minutesLeft, currentTime.sec);

			// Setup soft timer to turn system off when timer mode is finished for the day
			AssignSoftTimer(POWER_OFF_TIMER_NUM, (uint32)((minutesLeft * 60 * 2) - (currentTime.sec	* 2)), PowerOffTimerCallback);
		}
	}
}
