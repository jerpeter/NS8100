///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: DateTimeMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:46 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/DateTimeMenu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Menu.h"
#include "RealTimeClock.h"
#include "Display.h"
#include "Uart.h"
#include "Keypad.h"
#include "SoftTimer.h"
#include "PowerManagement.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define DATE_TIME_MN_TABLE_SIZE 6
#define DATE_TIME_WND_STARTING_COL 3
#define DATE_TIME_WND_END_COL 127
#define DATE_TIME_WND_STARTING_ROW DEFAULT_MENU_ROW_TWO
#define DATE_TIME_WND_END_ROW DEFAULT_MENU_ROW_THREE
#define DATE_TIME_MN_TBL_START_LINE 0
#define DTM_HOUR	0
#define DTM_MIN		1
#define DTM_SEC		2
#define DTM_DAY		3
#define DTM_MONTH	4
#define DTM_YEAR	5
#define END_DATA_STRING_SIZE 6
#define MIN_CHAR_STRING 0x41
#define MIN_NUM_STRING 0x2E
#define MAX_CHAR_STRING 0x5a
#define MAX_NUM_STRING 0x39
#define INPUT_MENU_SECOND_LINE_INDENT 6
#define MAX_CHARS_PER_LINE 20
#define DATE_TIME_MN_WND_LNS 6

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT serialNumberMenu[];
extern USER_MENU_STRUCT configMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void dateTimeMn (INPUT_MSG_STRUCT);
void dateTimeMnProc(INPUT_MSG_STRUCT,
                    REC_MN_STRUCT *,
                    WND_LAYOUT_STRUCT *,
                    MN_LAYOUT_STRUCT *);
void dsplyDateTimeMn(REC_MN_STRUCT *,
                     WND_LAYOUT_STRUCT *,
                     MN_LAYOUT_STRUCT *);
void loadDateTimeMnDefRec(REC_MN_STRUCT *,
                          DATE_TIME_STRUCT *);
void dateTimeDvScroll(char, REC_MN_STRUCT *);
void dateTimeScroll(char, MN_LAYOUT_STRUCT *);

/****************************************
*	Function:	dateTimeMn()
*	Purpose:
****************************************/
void dateTimeMn (INPUT_MSG_STRUCT msg)
{
	static WND_LAYOUT_STRUCT wnd_layout;
	static MN_LAYOUT_STRUCT mn_layout;
	static REC_MN_STRUCT mn_rec[6];

	dateTimeMnProc(msg, mn_rec, &wnd_layout, &mn_layout);

	if (g_activeMenu == DATE_TIME_MENU)
	{
		dsplyDateTimeMn(mn_rec, &wnd_layout, &mn_layout);
		writeMapToLcd(g_mmap);
	}
}

/****************************************
*	Function:	dateTimeMnProc
*	Purpose:
****************************************/
void dateTimeMnProc(INPUT_MSG_STRUCT msg, REC_MN_STRUCT *rec_ptr,
                    WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	static DATE_TIME_STRUCT time;
	void* tempPtr = NULL;

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col = DATE_TIME_WND_STARTING_COL;
			wnd_layout_ptr->end_col =   DATE_TIME_WND_END_COL;
			wnd_layout_ptr->start_row = DATE_TIME_WND_STARTING_ROW;
			wnd_layout_ptr->end_row =   DATE_TIME_WND_END_ROW;

			mn_layout_ptr->curr_ln =    0;
			mn_layout_ptr->top_ln =     0;
			mn_layout_ptr->sub_ln =     0;

			time = getRtcTime();

			debug("Date/Time Menu: Current Date: %d-%d-%d  Current Time: %d:%d:%d.%d \n",
				time.day, time.month, time.year,
					time.hour, time.min, time.sec, time.hundredths);

			loadDateTimeMnDefRec(rec_ptr, &time);
			rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;

			break;

		case (KEYPRESS_MENU_CMD):

			switch (msg.data[0])
			{
				case (ENTER_KEY):
					time.hour = (char)rec_ptr[DTM_HOUR].numrec.tindex;
					time.min = (char)rec_ptr[DTM_MIN].numrec.tindex;
					time.sec = (char)rec_ptr[DTM_SEC].numrec.tindex;
					time.day = (char)rec_ptr[DTM_DAY].numrec.tindex;
					time.month = (char)rec_ptr[DTM_MONTH].numrec.tindex;
					time.year = (char)rec_ptr[DTM_YEAR].numrec.tindex;

					// Set the new time in the clock. ALSO get the time to reset the globals.
					setRtcTime(&time);
					setRtcDate(&time);
					updateCurrentTime();

					debug("Date/Time Menu: New Date: %d-%d-%d  New Time: %d:%d:%d.%d \n",
						time.day, time.month, time.year,
							time.hour, time.min, time.sec, time.hundredths);

					// Check if Timer mode is enabled
					if (g_helpRecord.timer_mode == ENABLED)
					{
						// Cancel Timer mode
						messageBox(getLangText(STATUS_TEXT), getLangText(TIMER_MODE_DISABLED_TEXT), MB_OK);

						g_helpRecord.timer_mode = DISABLED;

						// Disable the Power Off Protection
						powerControl(POWER_OFF_PROTECTION_ENABLE, OFF);

						// Disable the Power Off timer if it's set
						clearSoftTimer(POWER_OFF_TIMER_NUM);

						saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
					}

					if (g_factorySetupSequence == PROCESS_FACTORY_SETUP)
					{
						if (!g_factorySetupRecord.invalid)
							tempPtr = &g_factorySetupRecord.serial_num;

						ACTIVATE_USER_MENU_MSG(&serialNumberMenu, tempPtr);
					}
					else
					{
						ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
					}

					(*menufunc_ptrs[g_activeMenu]) (mn_msg);
					break;

				case (DOWN_ARROW_KEY):
					if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
					{
						dateTimeDvScroll(DOWN,&rec_ptr[mn_layout_ptr->curr_ln]);
					}
					break;
				case (UP_ARROW_KEY):
					if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
					{
						dateTimeDvScroll(UP,&rec_ptr[mn_layout_ptr->curr_ln]);
					}
					break;
				case (PLUS_KEY):
					rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
					dateTimeScroll(DOWN, mn_layout_ptr);
					rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
					break;
				case (MINUS_KEY):
					rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
					dateTimeScroll(UP, mn_layout_ptr);
					rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
					break;
				case (ESC_KEY):
					if (g_factorySetupSequence == PROCESS_FACTORY_SETUP)
					{
						if (messageBox(getLangText(WARNING_TEXT), getLangText(PROCEED_WITHOUT_SETTING_DATE_AND_TIME_Q_TEXT), MB_YESNO) == MB_FIRST_CHOICE)
						{
							if (!g_factorySetupRecord.invalid)
								tempPtr = &g_factorySetupRecord.serial_num;

							ACTIVATE_USER_MENU_MSG(&serialNumberMenu, tempPtr);
						}
						else
						{
							// Do nothing
							break;
						}
					}
					else
					{
						ACTIVATE_USER_MENU_MSG(&configMenu, DATE_TIME);
					}

					(*menufunc_ptrs[g_activeMenu]) (mn_msg);
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
*	Function:	dateTimeScroll
*	Purpose:
****************************************/
void dateTimeScroll(char direction, MN_LAYOUT_STRUCT* mn_layout_ptr)
{
	if (direction == DOWN)
	{
		if (mn_layout_ptr->curr_ln < 5)
		{
			mn_layout_ptr->curr_ln++;
		}
	}
	else if (direction == UP)
	{
		if (mn_layout_ptr->curr_ln > 0)
		{
			mn_layout_ptr->curr_ln--;
		}
	}
}

/****************************************
*	Function:	dateTimeDvScroll
*	Purpose:
****************************************/
void dateTimeDvScroll(char dir_key, REC_MN_STRUCT *rec_ptr)
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
*	Function:	dsplyDateTimeMn
*	Purpose:
****************************************/
void dsplyDateTimeMn(REC_MN_STRUCT *rec_ptr, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	uint8 sbuff[50];
	uint8 top;
	uint8 menu_ln;
	uint8 length = 0;

	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));
	byteSet(&sbuff[0], 0, sizeof(sbuff));

	menu_ln = 0;
	top = (uint8)mn_layout_ptr->top_ln;

	// Add in a title for the menu
	sprintf((char*)sbuff, "-%s-", getLangText(DATE_TIME_TEXT));
	length = (uint8)strlen((char*)sbuff);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	byteSet(&sbuff[0], 0, sizeof(sbuff));

	wnd_layout_ptr->curr_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col =   wnd_layout_ptr->start_col;

	// Get the days per month, pass in the month num and the year for leap year
	rec_ptr[DTM_DAY].numrec.nmax = getDaysPerMonth((uint8)rec_ptr[DTM_MONTH].numrec.tindex,
													(uint8)rec_ptr[DTM_YEAR].numrec.tindex);
	if (rec_ptr[DTM_DAY].numrec.tindex > rec_ptr[DTM_DAY].numrec.nmax)
		rec_ptr[DTM_DAY].numrec.tindex = rec_ptr[DTM_DAY].numrec.nmax;

	// ---------------------------------------------------------------------------------
	// Write out the time line which includes the time text, hour, min, and sec values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;

	// Display the time text
	sprintf((char*)sbuff, "%s: ", getLangText(TIME_TEXT));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the hour
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[DTM_HOUR].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == DTM_HOUR) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)":", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the min
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[DTM_MIN].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == DTM_MIN) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)":", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the sec
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[DTM_SEC].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == DTM_SEC) ? CURSOR_LN : REG_LN);

	// ---------------------------------------------------------------------------------
	// Write out the date line which includes the date text, day, month, and year values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;

	// Display the date text
	sprintf((char*)sbuff, "%s: ", getLangText(DATE_TEXT));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the day
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[DTM_DAY].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == DTM_DAY) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)"-", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the month text
	sprintf((char*)sbuff, "%s", (char*)&(g_monthTable[(uint8)(rec_ptr[DTM_MONTH].numrec.tindex)].name[0]));
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == DTM_MONTH) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)"-", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the year
	sprintf((char*)sbuff, "%02d", (uint16)(uint16)rec_ptr[DTM_YEAR].numrec.tindex);
	wndMpWrtString(sbuff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == DTM_YEAR) ? CURSOR_LN : REG_LN);
}

/****************************************
*	Function:	loadDateTimeMnDefRec
*	Purpose:
****************************************/
void loadDateTimeMnDefRec(REC_MN_STRUCT *rec_ptr,DATE_TIME_STRUCT *time_ptr)
{
	byteSet(rec_ptr, 0, (sizeof(REC_MN_STRUCT) * 6));

	// HOURS
	rec_ptr[DTM_HOUR].enterflag = FALSE;
	rec_ptr[DTM_HOUR].type = INPUT_NUM_STRING;
	rec_ptr[DTM_HOUR].wrapflag = FALSE;
	rec_ptr[DTM_HOUR].numrec.nmax = 23;
	rec_ptr[DTM_HOUR].numrec.nmin = 0;
	rec_ptr[DTM_HOUR].numrec.incr_value = 1;
	rec_ptr[DTM_HOUR].numrec.tindex = time_ptr->hour;
	rec_ptr[DTM_HOUR].numrec.num_type = WHOLE_NUM_TYPE;

	// MINUTES
	rec_ptr[DTM_MIN].enterflag = FALSE;
	rec_ptr[DTM_MIN].type = INPUT_NUM_STRING;
	rec_ptr[DTM_MIN].wrapflag = FALSE;
	rec_ptr[DTM_MIN].numrec.nmax = 59;
	rec_ptr[DTM_MIN].numrec.nmin = 0;
	rec_ptr[DTM_MIN].numrec.incr_value = 1;
	rec_ptr[DTM_MIN].numrec.tindex = time_ptr->min;
	rec_ptr[DTM_MIN].numrec.num_type = WHOLE_NUM_TYPE;

	// SECONDS
	rec_ptr[DTM_SEC].enterflag = FALSE;
	rec_ptr[DTM_SEC].type = INPUT_NUM_STRING;
	rec_ptr[DTM_SEC].wrapflag = FALSE;
	rec_ptr[DTM_SEC].numrec.nmax = 59;
	rec_ptr[DTM_SEC].numrec.nmin = 0;
	rec_ptr[DTM_SEC].numrec.incr_value = 1;
	rec_ptr[DTM_SEC].numrec.tindex = time_ptr->sec;
	rec_ptr[DTM_SEC].numrec.num_type = WHOLE_NUM_TYPE;

	// DAY
	rec_ptr[DTM_DAY].enterflag = FALSE;
	rec_ptr[DTM_DAY].type = INPUT_NUM_STRING;
	rec_ptr[DTM_DAY].wrapflag = FALSE;
	rec_ptr[DTM_DAY].numrec.nmax = 31;
	rec_ptr[DTM_DAY].numrec.nmin = 1;
	rec_ptr[DTM_DAY].numrec.incr_value = 1;
	rec_ptr[DTM_DAY].numrec.tindex = time_ptr->day;
	rec_ptr[DTM_DAY].numrec.num_type = FIXED_TIME_TYPE_DAY;

	// MONTH
	rec_ptr[DTM_MONTH].enterflag = FALSE;
	rec_ptr[DTM_MONTH].type = INPUT_NUM_STRING;
	rec_ptr[DTM_MONTH].wrapflag = FALSE;
	rec_ptr[DTM_MONTH].numrec.nmax = 12;
	rec_ptr[DTM_MONTH].numrec.nmin = 1;
	rec_ptr[DTM_MONTH].numrec.incr_value = 1;
	rec_ptr[DTM_MONTH].numrec.tindex = time_ptr->month;
	rec_ptr[DTM_MONTH].numrec.num_type = FIXED_TIME_TYPE_MONTH;

	// YEAR
	rec_ptr[DTM_YEAR].enterflag = FALSE;
	rec_ptr[DTM_YEAR].type = INPUT_NUM_STRING;
	rec_ptr[DTM_YEAR].wrapflag = FALSE;
	rec_ptr[DTM_YEAR].numrec.nmax = 99;
	rec_ptr[DTM_YEAR].numrec.nmin = 0;
	rec_ptr[DTM_YEAR].numrec.incr_value = 1;
	rec_ptr[DTM_YEAR].numrec.tindex = time_ptr->year;
	rec_ptr[DTM_YEAR].numrec.num_type = FIXED_TIME_TYPE_YEAR;
}
