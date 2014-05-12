///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: TimerModeDateMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:01 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/TimerModeDateMenu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Old_Board.h"
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
extern MN_MEM_DATA_STRUCT mn_ptr[DEFAULT_MN_SIZE];
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern uint32 num_speed;
extern uint32 fixed_special_speed;
extern REC_HELP_MN_STRUCT help_rec;
extern USER_MENU_STRUCT configMenu[];
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
extern MONTH_TABLE_STRUCT monthTable[];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
uint32 start_leap_num = 0;
uint32 stop_leap_num = 0;
uint32 start_num_month  = 0;
uint32 stop_num_month = 0;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void timerModeDateMn (INPUT_MSG_STRUCT);
void timerModeDateMnProc(INPUT_MSG_STRUCT, REC_MN_STRUCT*, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void dsplyTimerModeDateMn(REC_MN_STRUCT*, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void loadTimerModeDateMnDefRec(REC_MN_STRUCT*, DATE_TIME_STRUCT*);
void timerModeDateDvScroll(char dir_key, REC_MN_STRUCT*);
void tmKeepDateTime(void);
void timerModeDateScroll(char, MN_LAYOUT_STRUCT*);
uint8 validateTimerModeSettings(void);

/****************************************
*	Function:	    timerModeDateMn
*	Purpose:
****************************************/
void timerModeDateMn (INPUT_MSG_STRUCT msg)
{
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;
    static REC_MN_STRUCT mn_rec[6];

    timerModeDateMnProc(msg, mn_rec, &wnd_layout, &mn_layout);

    if (active_menu == TIMER_MODE_DATE_MENU)
    {
        dsplyTimerModeDateMn(mn_rec, &wnd_layout, &mn_layout);
        writeMapToLcd(mmap);
    }
}

/****************************************
*	Function:	    timerModeDateMnProc
*	Purpose:
****************************************/
void timerModeDateMnProc(INPUT_MSG_STRUCT msg,
                    REC_MN_STRUCT *rec_ptr,
                    WND_LAYOUT_STRUCT *wnd_layout_ptr,
                    MN_LAYOUT_STRUCT *mn_layout_ptr)
{

	INPUT_MSG_STRUCT mn_msg;
	DATE_TIME_STRUCT time;
	uint32 input;
	//uint8 dayOfWeek = 0;

   switch (msg.cmd)
   {
      case (ACTIVATE_MENU_CMD):
            wnd_layout_ptr->start_col = TIMER_MODE_DATE_WND_STARTING_COL;
            wnd_layout_ptr->end_col =   TIMER_MODE_DATE_WND_END_COL;
            wnd_layout_ptr->start_row = TIMER_MODE_DATE_WND_STARTING_ROW;
            wnd_layout_ptr->end_row =   TIMER_MODE_DATE_WND_END_ROW;

            mn_layout_ptr->curr_ln =    0;
            mn_layout_ptr->top_ln =     0;
            mn_layout_ptr->sub_ln =     0;

            time = getCurrentTime();
            loadTimerModeDateMnDefRec(rec_ptr, &time);
            rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
            break;

      case (KEYPRESS_MENU_CMD):

            input = msg.data[0];
			switch (input)
			{
				case (ENTER_KEY):
					help_rec.tm_start_date.day = (char)rec_ptr[TMD_START_DAY].numrec.tindex;
					help_rec.tm_start_date.month = (char)rec_ptr[TMD_START_MONTH].numrec.tindex;
					help_rec.tm_start_date.year = (char)rec_ptr[TMD_START_YEAR].numrec.tindex;
					help_rec.tm_stop_date.day = (char)rec_ptr[TMD_STOP_DAY].numrec.tindex;
					help_rec.tm_stop_date.month = (char)rec_ptr[TMD_STOP_MONTH].numrec.tindex;
					help_rec.tm_stop_date.year = (char)rec_ptr[TMD_STOP_YEAR].numrec.tindex;

					processTimerModeSettings(PROMPT);

					ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
					(*menufunc_ptrs[active_menu]) (mn_msg);
					break;
               case (DOWN_ARROW_KEY):
                     if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
                     {
                        timerModeDateDvScroll(DOWN, &rec_ptr[mn_layout_ptr->curr_ln]);
                     }
                     break;
               case (UP_ARROW_KEY):
                     if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
                     {
                        timerModeDateDvScroll(UP, &rec_ptr[mn_layout_ptr->curr_ln]);
                     }
                     break;
               case (PLUS_KEY):
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
                     timerModeDateScroll(DOWN, mn_layout_ptr);
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
                     break;
               case (MINUS_KEY):
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
                     timerModeDateScroll(UP, mn_layout_ptr);
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
                     break;
               case (ESC_KEY):
                     active_menu = TIMER_MODE_TIME_MENU;
                     ACTIVATE_MENU_MSG();
                     (*menufunc_ptrs[active_menu]) (mn_msg);
                     break;
               default:
                     break;
            }
            break;

      default:
            break;
   }

}

/****************************************
*	Function:	timerModeDateScroll
*	Purpose:
****************************************/
void timerModeDateScroll(char direction, MN_LAYOUT_STRUCT* mn_layout_ptr)
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

/****************************************
*	Function:	timerModeDateDvScroll
*	Purpose:
****************************************/
void timerModeDateDvScroll(char dir_key, REC_MN_STRUCT *rec_ptr)
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

/****************************************
*	Function:	dsplyTimerModeDateMn
*	Purpose:
****************************************/
void dsplyTimerModeDateMn(REC_MN_STRUCT *rec_ptr,
                     WND_LAYOUT_STRUCT *wnd_layout_ptr,
                     MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	uint8 sbuff[50];
	uint8 top;
	uint8 menu_ln;
	uint8 length = 0;

	byteSet(&(mmap[0][0]), 0, sizeof(mmap));

	menu_ln = 0;
	top = (uint8)mn_layout_ptr->top_ln;

	// Add in a title for the menu
	byteSet(&sbuff[0], 0, sizeof(sbuff));
	sprintf((char*)sbuff, "-%s-", getLangText(ACTIVE_DATE_PERIOD_TEXT));
	length = (uint8)strlen((char*)sbuff);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// Add in a title for the menu
	byteSet(&sbuff[0], 0, sizeof(sbuff));
	sprintf((char*)sbuff, "(%s)", getLangText(DAY_MONTH_YEAR_TEXT));
	length = (uint8)strlen((char*)sbuff);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	byteSet(&sbuff[0], 0, sizeof(sbuff));

	wnd_layout_ptr->curr_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col =   wnd_layout_ptr->start_col;

	rec_ptr[TMD_START_DAY].numrec.nmax = getDaysPerMonth((uint8)(rec_ptr[TMD_START_MONTH].numrec.tindex),
																(uint8)(rec_ptr[TMD_START_YEAR].numrec.tindex));
	if (rec_ptr[TMD_START_DAY].numrec.tindex > rec_ptr[TMD_START_DAY].numrec.nmax)
		rec_ptr[TMD_START_DAY].numrec.tindex = rec_ptr[TMD_START_DAY].numrec.nmax;

	rec_ptr[TMD_STOP_DAY].numrec.nmax = getDaysPerMonth((uint8)(rec_ptr[TMD_STOP_MONTH].numrec.tindex),
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
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((9 * SIX_COL_SIZE)/2));

	// Display the day
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_START_DAY].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_START_DAY) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the month text
	sprintf((char*)sbuff, "%s", (char*)&(monthTable[(uint8)(rec_ptr[TMD_START_MONTH].numrec.tindex)].name[0]));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_START_MONTH) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the year
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_START_YEAR].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_START_YEAR) ? CURSOR_LN : REG_LN);

	// ---------------------------------------------------------------------------------
	// Write out the stop date line which includes the stop date text, day, month, and year values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;

	// Display the date text
	sprintf((char*)sbuff, "%s: ", getLangText(STOP_DATE_TEXT));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((9 * SIX_COL_SIZE)/2));

	// Display the day
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_STOP_DAY].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_STOP_DAY) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the month text
	sprintf((char*)sbuff, "%s", (char*)&(monthTable[(uint8)(rec_ptr[TMD_STOP_MONTH].numrec.tindex)].name[0]));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_STOP_MONTH) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)("-"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the year
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[TMD_STOP_YEAR].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMD_STOP_YEAR) ? CURSOR_LN : REG_LN);
}

/****************************************
*	Function:	loadTimerModeDateMnDefRec
*	Purpose:
****************************************/
void loadTimerModeDateMnDefRec(REC_MN_STRUCT *rec_ptr, DATE_TIME_STRUCT *time_ptr)
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

/****************************************
*	Function:	tmKeepDateTime
*	Purpose:
****************************************/
void tmKeepDateTime(void)
{
	// Find the Time mode active time period in ticks

	// Check if the stop time (in minutes resolution) is greater than the start time, thus running the same day
	if (((help_rec.tm_stop_time.hour * 60) + help_rec.tm_stop_time.min) >
		((help_rec.tm_start_time.hour * 60) + help_rec.tm_start_time.min))
	{
		// Set active minutes as the difference in the stop and start times
		help_rec.timer_mode_active_minutes = (uint16)(((help_rec.tm_stop_time.hour * 60) + help_rec.tm_stop_time.min) -
												((help_rec.tm_start_time.hour * 60) + help_rec.tm_start_time.min));
	}
	else // The timer mode active period will see midnight, thus running 2 consecutive days
	{
		// Set active minutes as the difference from midnight and the start time, plus the stop time the next day
		help_rec.timer_mode_active_minutes = (uint16)((24 * 60) - ((help_rec.tm_start_time.hour * 60) + help_rec.tm_start_time.min) +
												((help_rec.tm_stop_time.hour * 60) + help_rec.tm_stop_time.min));
	}

	debug("Timer Active Minutes: %d\n", help_rec.timer_mode_active_minutes);

    saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
}

/****************************************
*	Function:	validateTimerModeSettings
*	Purpose:
****************************************/
uint8 validateTimerModeSettings(void)
{
	DATE_TIME_STRUCT time = getCurrentTime();

	char start_min = help_rec.tm_start_time.min;
	char start_hour = help_rec.tm_start_time.hour;
	char stop_min = help_rec.tm_stop_time.min;
	char stop_hour = help_rec.tm_stop_time.hour;
	char start_day = help_rec.tm_start_date.day;
	char start_month = help_rec.tm_start_date.month;
	char start_year = help_rec.tm_start_date.year;
	char stop_day = help_rec.tm_stop_date.day;
	char stop_month = help_rec.tm_stop_date.month;
	char stop_year = help_rec.tm_stop_date.year;

	debug("Timer Date: (Start) %d/%d/%d -> (End) %d/%d/%d\n", start_month, start_day, start_year,
															  stop_month, stop_day, stop_year);

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

/****************************************
*	Function:	processTimerModeSettings
*	Purpose:
****************************************/
void processTimerModeSettings(uint8 mode)
{
	//uint8 dayOfWeek = 0;
	uint8 startDay = 0;
	char stringBuff[75];
	uint16 minutesLeft = 0;
	DATE_TIME_STRUCT currentTime = getCurrentTime();
	uint8 status = validateTimerModeSettings();

	// Check if the timer mode settings check failed or if the timer mode setting has been disabled
	if ((status == FAILED) || (help_rec.timer_mode == DISABLED))
	{
		help_rec.timer_mode = DISABLED;
		saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		// Disable the Power Off timer in case it's set
		clearSoftTimer(POWER_OFF_TIMER_NUM);

		if (mode == PROMPT)
		{
			byteSet(&stringBuff[0], 0, sizeof(stringBuff));
			sprintf(stringBuff, "%s %s", getLangText(TIMER_SETTINGS_INVALID_TEXT), getLangText(TIMER_MODE_DISABLED_TEXT));
			messageBox(getLangText(ERROR_TEXT), stringBuff, MB_OK);
		}
	}
	else // status == PASSED || status == IN_PROGRESS
	{
		tmKeepDateTime();

		RTC_FLAGS.reg = RTC_FLAGS.reg;

		//dayOfWeek = getDayOfWeek(help_rec.tm_start_date.year, help_rec.tm_start_date.month,
		//						help_rec.tm_start_date.day);

		startDay = help_rec.tm_start_date.day;

		// Check if in progress
		if(status == IN_PROGRESS)
		{
			// Check if the stop time is greater than the start time
			if((help_rec.tm_stop_time.hour > help_rec.tm_start_time.hour) ||
				((help_rec.tm_stop_time.hour == help_rec.tm_start_time.hour) &&
				(help_rec.tm_stop_time.min > help_rec.tm_start_time.min)))
			{
				// Advance the start day
				startDay++;

				// Check if the start day is beond the total days in the current month
				if (startDay > monthTable[(uint8)(help_rec.tm_start_date.month)].days)
				{
					// Set the start day to the first day of next month
					startDay = 1;
				}
			}
		}

		EnableRtcAlarm(startDay, help_rec.tm_start_time.hour, help_rec.tm_start_time.min, 0);

		if (status == PASSED)
		{
			if (mode == PROMPT)
			{
				byteSet(&stringBuff[0], 0, sizeof(stringBuff));
				sprintf(stringBuff, "%s %s", getLangText(TIMER_MODE_NOW_ACTIVE_TEXT), getLangText(PLEASE_POWER_OFF_UNIT_TEXT));
				messageBox(getLangText(STATUS_TEXT), stringBuff, MB_OK);
			}

			// Check if start time is greater than the current time
			if (((help_rec.tm_start_time.hour * 60) + help_rec.tm_start_time.min) >
				((currentTime.hour * 60) + currentTime.min))
			{
				// Take the difference between start time and current time
				minutesLeft = (uint16)(((help_rec.tm_start_time.hour * 60) + help_rec.tm_start_time.min) -
							((currentTime.hour * 60) + currentTime.min));
			}
			else // Current time is after the start time, meaning the start time is the next day
			{
				// Take the difference between 24 hours and the current time plus the start time
				minutesLeft = (uint16)((24 * 60) - ((currentTime.hour * 60) + currentTime.min) +
							((help_rec.tm_start_time.hour * 60) + help_rec.tm_start_time.min));
			}

			// Check if the start time is within the next minute
			if (minutesLeft <= 1)
			{
				overlayMessage(getLangText(WARNING_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), 2 * SOFT_SECS);

				// Need to shutdown the unit now, otherwise the start time window will be missed
				PowerUnitOff(SHUTDOWN_UNIT); // Return unnecessary
			}
			else // More than 1 minute left before the start time
			{
				// Make sure the unit turns off one minute before the start time if the user forgets to turn the unit off
				minutesLeft -= 1;

				// Set the Power off soft timer to prevent the unit from staying on past the Timer mode start time
				assignSoftTimer(POWER_OFF_TIMER_NUM, (uint32)(minutesLeft * 60 * 2), powerOffTimerCallback);
			}
		}
		else // status == IN_PROGRESS
		{
			// Disable the Power Off key
			powerControl(POWER_SHUTDOWN_ENABLE, OFF);

			if (mode == PROMPT)
			{
				byteSet(&stringBuff[0], 0, sizeof(stringBuff));
				sprintf(stringBuff, "%s", getLangText(TIMER_MODE_NOW_ACTIVE_TEXT));
				messageBox(getLangText(STATUS_TEXT), stringBuff, MB_OK);
			}

			// Check if the current time is greater than the stop time, indicating that midnight boundary was crossed
			if(((currentTime.hour * 60) + currentTime.min) > ((help_rec.tm_stop_time.hour * 60) + help_rec.tm_stop_time.min))
			{
				// Calculate the time left before powering off to be 24 + the stop time minus the current time
				minutesLeft = (uint16)(((24 * 60) + (help_rec.tm_stop_time.hour * 60) + help_rec.tm_stop_time.min) -
								((currentTime.hour * 60) + currentTime.min));
			}
			else // Current time is less than start time, operating within the same day
			{
				// Calculate the time left before powering off to be the stop time minus the current time
				minutesLeft = (uint16)(((help_rec.tm_stop_time.hour * 60) + help_rec.tm_stop_time.min) -
								((currentTime.hour * 60) + currentTime.min));
			}

			// Make sure timeout value is not zero
			if (minutesLeft == 0) minutesLeft = 1;

			debug("Timer Mode: In progress, minutes left before power off: %d\n", minutesLeft);

			// Setup soft timer to turn system off when timer mode is finished for the day
			assignSoftTimer(POWER_OFF_TIMER_NUM, (uint32)(minutesLeft * 60 * 2), powerOffTimerCallback);
		}
	}
}
