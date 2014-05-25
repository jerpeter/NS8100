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
#include "Typedefs.h"
#include "Menu.h"
#include "Display.h"
#include "RealTimeClock.h"
#include "Uart.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define TIMER_MODE_TIME_MN_SIZE_EXTENSION 
#define TIMER_MODE_TIME_MN_TABLE_SIZE 6 
#define TIMER_MODE_TIME_WND_STARTING_COL 3
#define TIMER_MODE_TIME_WND_END_COL 127 
#define TIMER_MODE_TIME_WND_STARTING_ROW DEFAULT_MENU_ROW_THREE
#define TIMER_MODE_TIME_WND_END_ROW DEFAULT_MENU_ROW_SIX
#define TIMER_MODE_TIME_MN_TBL_START_LINE 0
#define TMT_START_HOUR	0
#define TMT_START_MIN	1
#define TMT_STOP_HOUR	2
#define TMT_STOP_MIN	3
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
extern USER_MENU_STRUCT timerModeFreqMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static int s_dataRecordCurrentItem;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void timerModeTimeMn (INPUT_MSG_STRUCT);
void timerModeTimeMnProc(INPUT_MSG_STRUCT,
                    REC_MN_STRUCT *,
                    WND_LAYOUT_STRUCT *,
                    MN_LAYOUT_STRUCT *);
void dsplyTimerModeTimeMn(REC_MN_STRUCT *,
                     WND_LAYOUT_STRUCT *,
                     MN_LAYOUT_STRUCT *);
void loadTimerModeTimeMnDefRec(REC_MN_STRUCT *,
                          DATE_TIME_STRUCT *);
void timerModeTimeDvScroll (char dir_key,
                       REC_MN_STRUCT *);
void tmKeepTime(void* src_ptr);
void timerModeTimeScroll(char, MN_LAYOUT_STRUCT *);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void timerModeTimeMn (INPUT_MSG_STRUCT msg)
{ 
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;
    static REC_MN_STRUCT mn_rec[6];
  
    timerModeTimeMnProc(msg, mn_rec, &wnd_layout, &mn_layout);           

    if (g_activeMenu == TIMER_MODE_TIME_MENU)
    {
        dsplyTimerModeTimeMn(mn_rec, &wnd_layout, &mn_layout);
        writeMapToLcd(g_mmap);
    }
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void timerModeTimeMnProc(INPUT_MSG_STRUCT msg, REC_MN_STRUCT *rec_ptr, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
   INPUT_MSG_STRUCT mn_msg;
   DATE_TIME_STRUCT time;
   uint32 input;
   
   switch (msg.cmd)
   {
      case (ACTIVATE_MENU_CMD):
            wnd_layout_ptr->start_col = TIMER_MODE_TIME_WND_STARTING_COL;
            wnd_layout_ptr->end_col =  TIMER_MODE_TIME_WND_END_COL;
            wnd_layout_ptr->start_row = TIMER_MODE_TIME_WND_STARTING_ROW;
            wnd_layout_ptr->end_row =   TIMER_MODE_TIME_WND_END_ROW;
            
            mn_layout_ptr->curr_ln =    0; 
            mn_layout_ptr->top_ln =     0;
            mn_layout_ptr->sub_ln =     0;
        
            time = getCurrentTime();
            loadTimerModeTimeMnDefRec(rec_ptr,&time);
            rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
            break;
            
      case (ACTIVATE_MENU_WITH_DATA_CMD):
            wnd_layout_ptr->start_col = TIMER_MODE_TIME_WND_STARTING_COL;
            wnd_layout_ptr->end_col =  TIMER_MODE_TIME_WND_END_COL;
            wnd_layout_ptr->start_row = TIMER_MODE_TIME_WND_STARTING_ROW;
            wnd_layout_ptr->end_row =   TIMER_MODE_TIME_WND_END_ROW;
            
            mn_layout_ptr->curr_ln =    0; 
            mn_layout_ptr->top_ln =     0;
            mn_layout_ptr->sub_ln =     0;
        
            rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
            break;
            
      case (KEYPRESS_MENU_CMD):
      
            input = msg.data[0];
            switch (input)
            {
                  
               case (ENTER_KEY):
                     rec_ptr[s_dataRecordCurrentItem].enterflag = FALSE;
                     tmKeepTime(rec_ptr);

                     SETUP_MENU_MSG(TIMER_MODE_DATE_MENU);
                     JUMP_TO_ACTIVE_MENU();
                     break;      
               case (DOWN_ARROW_KEY):
                     if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
                     {
                        timerModeTimeDvScroll(DOWN,&rec_ptr[mn_layout_ptr->curr_ln]);
                     }
                     break;
               case (UP_ARROW_KEY):
                     if (rec_ptr[mn_layout_ptr->curr_ln].enterflag == TRUE)
                     {
                        timerModeTimeDvScroll(UP,&rec_ptr[mn_layout_ptr->curr_ln]);
                     }
                     break;
               case (PLUS_KEY):
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
                     timerModeTimeScroll(DOWN, mn_layout_ptr);
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
                     break;
               case (MINUS_KEY):
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = FALSE;
                     timerModeTimeScroll(UP, mn_layout_ptr);
                     rec_ptr[mn_layout_ptr->curr_ln].enterflag = TRUE;
                     break;
               case (ESC_KEY):
					SETUP_USER_MENU_MSG(&timerModeFreqMenu, g_helpRecord.timerModeFrequency);
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
void timerModeTimeScroll(char direction, MN_LAYOUT_STRUCT* mn_layout_ptr)
{   
	switch (direction)
	{
		case (DOWN):
			if (mn_layout_ptr->curr_ln < 3)
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
void timerModeTimeDvScroll (char dir_key,REC_MN_STRUCT *rec_ptr)
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
void dsplyTimerModeTimeMn(REC_MN_STRUCT *rec_ptr, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	char sbuff[50];
	uint8 top;
	uint8 menu_ln;
	uint8 length = 0;

	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	menu_ln = 0;
	top = (uint8)mn_layout_ptr->top_ln;              

	// Add in a title for the menu
	byteSet(&sbuff[0], 0, sizeof(sbuff));
	sprintf((char*)sbuff, "-%s-", getLangText(ACTIVE_TIME_PERIOD_TEXT));
	length = (uint8)strlen((char*)sbuff);
	
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// Add in unit format for the menu
	byteSet(&sbuff[0], 0, sizeof(sbuff));
	sprintf((char*)sbuff, "(24 %s:%s)", getLangText(HOUR_TEXT), getLangText(MINUTE_TEXT));
	length = (uint8)strlen((char*)sbuff);
	
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
	wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	byteSet(&sbuff[0], 0, sizeof(sbuff));

	wnd_layout_ptr->curr_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col =   wnd_layout_ptr->start_col;

	// ---------------------------------------------------------------------------------
	// Write out the start time line which includes the start time text, hour and min values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;

	// Display the start time text
	sprintf((char*)sbuff, "%s: ", getLangText(START_TIME_TEXT));
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((5 * SIX_COL_SIZE)/2));

	// Display the start hour
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[TMT_START_HOUR].numrec.tindex);
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMT_START_HOUR) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)(":"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the start min
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[TMT_START_MIN].numrec.tindex);
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMT_START_MIN) ? CURSOR_LN : REG_LN);

	// ---------------------------------------------------------------------------------
	// Write out the stop time line which includes the stop time text, hour and min values
	// ---------------------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;

	// Display the stop time text
	sprintf((char*)sbuff, "%s: ", getLangText(STOP_TIME_TEXT));
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((5 * SIX_COL_SIZE)/2));

	// Display the stop hour
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[TMT_STOP_HOUR].numrec.tindex);
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMT_STOP_HOUR) ? CURSOR_LN : REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	wndMpWrtString((uint8*)(":"), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_col = wnd_layout_ptr->next_col;

	// Display the stop min
	sprintf((char*)sbuff, "%02d", (uint16)rec_ptr[TMT_STOP_MIN].numrec.tindex);
	wndMpWrtString((uint8*)(&sbuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, (mn_layout_ptr->curr_ln == TMT_STOP_MIN) ? CURSOR_LN : REG_LN);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void loadTimerModeTimeMnDefRec(REC_MN_STRUCT *rec_ptr,DATE_TIME_STRUCT *time_ptr)
{
    // START HOUR
    rec_ptr[TMT_START_HOUR].enterflag = FALSE;
    rec_ptr[TMT_START_HOUR].type = INPUT_NUM_STRING;
    rec_ptr[TMT_START_HOUR].wrapflag = FALSE;
    rec_ptr[TMT_START_HOUR].numrec.nmax = 23;
    rec_ptr[TMT_START_HOUR].numrec.nmin = 0;
    rec_ptr[TMT_START_HOUR].numrec.incr_value = 1;
    rec_ptr[TMT_START_HOUR].numrec.tindex = time_ptr->hour;
    rec_ptr[TMT_START_HOUR].numrec.num_type = WHOLE_NUM_TYPE;

    // START MIN
    rec_ptr[TMT_START_MIN].enterflag = FALSE;
    rec_ptr[TMT_START_MIN].type = INPUT_NUM_STRING;
    rec_ptr[TMT_START_MIN].wrapflag = FALSE;
    rec_ptr[TMT_START_MIN].numrec.nmax = 59;
    rec_ptr[TMT_START_MIN].numrec.nmin = 0;
    rec_ptr[TMT_START_MIN].numrec.incr_value = 1;

	if ((time_ptr->min + 5) > 59)
	{
		rec_ptr[TMT_START_MIN].numrec.tindex = (time_ptr->min + 5) - 60;

		// Inc the start hour
		if ((time_ptr->hour + 1) > 23)
		{
			rec_ptr[TMT_START_HOUR].numrec.tindex = 0;
		}
		else
		{
			rec_ptr[TMT_START_HOUR].numrec.tindex = time_ptr->hour + 1;
		}
	}
	else
	{
	    rec_ptr[TMT_START_MIN].numrec.tindex = time_ptr->min + 5;
	}
    rec_ptr[TMT_START_MIN].numrec.num_type = WHOLE_NUM_TYPE;

    // STOP HOUR
    rec_ptr[TMT_STOP_HOUR].enterflag = FALSE;
    rec_ptr[TMT_STOP_HOUR].type = INPUT_NUM_STRING;
    rec_ptr[TMT_STOP_HOUR].wrapflag = FALSE;
    rec_ptr[TMT_STOP_HOUR].numrec.nmax = 23;
    rec_ptr[TMT_STOP_HOUR].numrec.nmin = 0;
    rec_ptr[TMT_STOP_HOUR].numrec.incr_value = 1;
    rec_ptr[TMT_STOP_HOUR].numrec.tindex = time_ptr->hour;
    rec_ptr[TMT_STOP_HOUR].numrec.num_type = WHOLE_NUM_TYPE;
    
    // STOP MIN
    rec_ptr[TMT_STOP_MIN].enterflag = FALSE;
    rec_ptr[TMT_STOP_MIN].type = INPUT_NUM_STRING;
    rec_ptr[TMT_STOP_MIN].wrapflag = FALSE;
    rec_ptr[TMT_STOP_MIN].numrec.nmax = 59;
    rec_ptr[TMT_STOP_MIN].numrec.nmin = 0;
    rec_ptr[TMT_STOP_MIN].numrec.incr_value = 1;

    if ((time_ptr->min + 10) > 59)
    {
		rec_ptr[TMT_STOP_MIN].numrec.tindex = (time_ptr->min + 10) - 60;

		// Inc the stop hour
		if ((time_ptr->hour + 1) > 23)
		{	
		    rec_ptr[TMT_STOP_HOUR].numrec.tindex = 0;
		}
		else
		{	
		    rec_ptr[TMT_STOP_HOUR].numrec.tindex = time_ptr->hour + 1;
		}
    }
    else
    {
             rec_ptr[TMT_STOP_MIN].numrec.tindex = time_ptr->min + 10;
    } 
    rec_ptr[TMT_STOP_MIN].numrec.num_type = WHOLE_NUM_TYPE;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void tmKeepTime(void* src_ptr)
{
   REC_MN_STRUCT *rtemp;
   
   rtemp = (REC_MN_STRUCT *)src_ptr;
   
   g_helpRecord.timerStartTime.hour = (char)rtemp[TMT_START_HOUR].numrec.tindex;
   g_helpRecord.timerStartTime.min = (char)rtemp[TMT_START_MIN].numrec.tindex;

   g_helpRecord.timerStopTime.hour = (char)rtemp[TMT_STOP_HOUR].numrec.tindex;
   g_helpRecord.timerStopTime.min = (char)rtemp[TMT_STOP_MIN].numrec.tindex;

	debug("Timer Time: (Start) %d:%d -> (End) %d:%d\n", g_helpRecord.timerStartTime.hour,
			g_helpRecord.timerStartTime.min, g_helpRecord.timerStopTime.hour, g_helpRecord.timerStopTime.min);
}
