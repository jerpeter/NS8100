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
#include "Board.h"
#include "Record.h"
#include "Uart.h"
#include "Display.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define MONITOR_LOG_MN_TABLE_SIZE 8
#define MONITOR_LOG_WND_STARTING_COL 6
#define MONITOR_LOG_WND_END_COL 127
#define MONITOR_LOG_WND_STARTING_ROW 8
#define MONITOR_LOG_WND_END_ROW 55
#define MONITOR_LOG_MN_TBL_START_LINE 0

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT monitorLogMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static int16 s_monitorMnCurrentLogIndex = 0;
static uint16 s_monitorMnLastLogIndex = 0;
static uint16 s_monitorMnStartLogIndex = 0;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void monitorLogMn(INPUT_MSG_STRUCT);
void monitorLogMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *, MN_LAYOUT_STRUCT *);
void monitorLogMnDsply(WND_LAYOUT_STRUCT *);

/****************************************
*	Function:	monitorLogMn
*	Purpose:
****************************************/
void monitorLogMn(INPUT_MSG_STRUCT msg)
{
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;

    monitorLogMnProc(msg, &wnd_layout, &mn_layout);

    if(g_activeMenu == VIEW_MONITOR_LOG_MENU)
    {
        monitorLogMnDsply(&wnd_layout);
        writeMapToLcd(g_mmap);
    }
}

/****************************************
*	Function:	monitorLogMnProc
*	Purpose:
****************************************/
void monitorLogMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	uint32 input;

	switch(msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col = MONITOR_LOG_WND_STARTING_COL;
			wnd_layout_ptr->end_col =   MONITOR_LOG_WND_END_COL;
			wnd_layout_ptr->start_row = MONITOR_LOG_WND_STARTING_ROW;
			wnd_layout_ptr->end_row =   MONITOR_LOG_WND_END_ROW;

			mn_layout_ptr->curr_ln =    1;
			mn_layout_ptr->top_ln =     1;
			mn_layout_ptr->sub_ln =     0;

			s_monitorMnCurrentLogIndex = (int16)getStartingMonitorLogTableIndex();
			s_monitorMnLastLogIndex = __monitorLogTblIndex;

			// Loop through circular buffer to find starting index
			while((__monitorLogTbl[s_monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY) &&
					(s_monitorMnCurrentLogIndex != s_monitorMnLastLogIndex))
			{
				s_monitorMnCurrentLogIndex++;

				if(s_monitorMnCurrentLogIndex >= (int16)TOTAL_MONITOR_LOG_ENTRIES)
					s_monitorMnCurrentLogIndex = 0;
			}

			s_monitorMnStartLogIndex = (uint16)s_monitorMnCurrentLogIndex;

			if((s_monitorMnCurrentLogIndex == s_monitorMnLastLogIndex) && 
				(__monitorLogTbl[s_monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY))
			{
				s_monitorMnCurrentLogIndex = -1;
			}
		break;

		case (KEYPRESS_MENU_CMD):
			input = msg.data[0];
			switch(input)
			{
				case (ENTER_KEY):
					messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				break;

				case (DOWN_ARROW_KEY):
					if((s_monitorMnCurrentLogIndex != s_monitorMnLastLogIndex) && (s_monitorMnCurrentLogIndex >= 0))
					{
						s_monitorMnCurrentLogIndex++;

						if(s_monitorMnCurrentLogIndex >= (int16)TOTAL_MONITOR_LOG_ENTRIES)
							s_monitorMnCurrentLogIndex = 0;

						while((__monitorLogTbl[s_monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY) &&
								(s_monitorMnCurrentLogIndex != s_monitorMnLastLogIndex))
						{
							s_monitorMnCurrentLogIndex++;

							if(s_monitorMnCurrentLogIndex >= (int16)TOTAL_MONITOR_LOG_ENTRIES)
								s_monitorMnCurrentLogIndex = 0;
						}
					}
				break;

				case (UP_ARROW_KEY):
					if((s_monitorMnCurrentLogIndex != s_monitorMnStartLogIndex) && (s_monitorMnCurrentLogIndex >= 0))
					{
						s_monitorMnCurrentLogIndex--;

						if(s_monitorMnCurrentLogIndex < 0)
							s_monitorMnCurrentLogIndex = TOTAL_MONITOR_LOG_ENTRIES - 1;

						while((__monitorLogTbl[s_monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY) &&
								(s_monitorMnCurrentLogIndex != s_monitorMnStartLogIndex))
						{
							s_monitorMnCurrentLogIndex--;

							if(s_monitorMnCurrentLogIndex < 0)
								s_monitorMnCurrentLogIndex = TOTAL_MONITOR_LOG_ENTRIES - 1;
						}
					}
				break;

				case (MINUS_KEY): adjustLcdContrast(DARKER); break;
				case (PLUS_KEY): adjustLcdContrast(LIGHTER); break;

				case (ESC_KEY):
					SETUP_USER_MENU_MSG(&monitorLogMenu, DEFAULT_ITEM_1);
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

/****************************************
*	Function:	monitorLogMnDsply
*	Purpose:
****************************************/
void monitorLogMnDsply(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
    uint8 buff[25];
    uint8 length;

    byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	// Add in a title for the menu
	byteSet(&buff[0], 0, sizeof(buff));
	length = (uint8)sprintf((char*)buff, "-%s-", getLangText(VIEW_MONITOR_LOG_TEXT));

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	if (s_monitorMnCurrentLogIndex != -1)
	{
		if (__monitorLogTbl[s_monitorMnCurrentLogIndex].status != EMPTY_LOG_ENTRY)
		{
			// Display Mode text
			byteSet(&buff[0], 0, sizeof(buff));
			switch(__monitorLogTbl[s_monitorMnCurrentLogIndex].mode)
			{
				case WAVEFORM_MODE:   length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(WAVEFORM_MODE_TEXT)); break;
				case BARGRAPH_MODE:	  length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(BARGRAPH_MODE_TEXT)); break;
				case MANUAL_CAL_MODE: length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(CALIBRATION_TEXT)); break;
				case COMBO_MODE:	  length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(COMBO_MODE_TEXT)); break;
			}

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			// Display Start Time text
			byteSet(&buff[0], 0, sizeof(buff));
			convertTimeStampToString((char*)(&buff[0]), &__monitorLogTbl[s_monitorMnCurrentLogIndex].startTime, REC_DATE_TIME_TYPE);
			length = (uint8)strlen((char*)(&buff[0]));

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			// Display Stop Time text
			byteSet(&buff[0], 0, sizeof(buff));
			convertTimeStampToString((char*)(&buff[0]), &__monitorLogTbl[s_monitorMnCurrentLogIndex].stopTime, REC_DATE_TIME_TYPE);
			length = (uint8)strlen((char*)(&buff[0]));

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			// Display Number of Events recorded text
			if(__monitorLogTbl[s_monitorMnCurrentLogIndex].eventsRecorded == 0)
			{
				length = (uint8)sprintf((char*)(&buff[0]), "%s %s", getLangText(NO_TEXT), getLangText(EVENTS_RECORDED_TEXT));

				wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
				wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
				wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			}
			else
			{
				length = (uint8)sprintf((char*)(&buff[0]), "%s: %d", getLangText(EVENTS_RECORDED_TEXT), __monitorLogTbl[s_monitorMnCurrentLogIndex].eventsRecorded);

				wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
				wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
				wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

				if(__monitorLogTbl[s_monitorMnCurrentLogIndex].eventsRecorded == 1)
				{
#if 0 // Port lost change
					length = (uint8)sprintf((char*)(&buff[0]), "EVENT #s: %d", __monitorLogTbl[s_monitorMnCurrentLogIndex].startEventNumber);
#else // Updated
					length = (uint8)sprintf((char*)(&buff[0]), "%s: %d", getLangText(EVENT_NUMBER_TEXT), __monitorLogTbl[s_monitorMnCurrentLogIndex].startEventNumber);
#endif
				}
				else
				{
#if 0 // Port lost change
					length = (uint8)sprintf((char*)(&buff[0]), "EVENT #s: %d-%d", __monitorLogTbl[s_monitorMnCurrentLogIndex].startEventNumber,
											(__monitorLogTbl[s_monitorMnCurrentLogIndex].startEventNumber + __monitorLogTbl[s_monitorMnCurrentLogIndex].eventsRecorded - 1));
#else // Updated
					length = (uint8)sprintf((char*)(&buff[0]), "%s: %d-%d", getLangText(EVENT_NUMBER_TEXT), __monitorLogTbl[s_monitorMnCurrentLogIndex].startEventNumber,
											(__monitorLogTbl[s_monitorMnCurrentLogIndex].startEventNumber + __monitorLogTbl[s_monitorMnCurrentLogIndex].eventsRecorded - 1));
#endif
				}

				wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
				wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
				wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			}
		}

		if(s_monitorMnCurrentLogIndex == s_monitorMnStartLogIndex)
		{
			// Display Start of Log
			byteSet(&buff[0], 0, sizeof(buff));
#if 0 // Port lost change
			length = (uint8)sprintf((char*)buff, "<%s>", "START OF LOG");
#else // Updated
			length = (uint8)sprintf((char*)buff, "<%s>", getLangText(START_OF_LOG_TEXT));
#endif
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
		else if(s_monitorMnCurrentLogIndex == s_monitorMnLastLogIndex)
		{
			// Display End of Log
			byteSet(&buff[0], 0, sizeof(buff));
#if 0 // Port lost change
			length = (uint8)sprintf((char*)buff, "<%s>", "END OF LOG");
#else // Updated
			length = (uint8)sprintf((char*)buff, "<%s>", getLangText(END_OF_LOG_TEXT));
#endif
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
	}
	else
	{
		// Display Mode text
		byteSet(&buff[0], 0, sizeof(buff));
		length = (uint8)sprintf((char*)(&buff[0]), "<%s>", getLangText(EMPTY_TEXT));

		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	}
}
