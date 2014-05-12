/*******************************************************************************
*  Nomis Seismograph, Inc.
*  Copyright 2002-2005, All Rights Reserved
*
*  $RCSfile: MonitorLogMenu.c,v $
*
*  $Author: lking $
*  $Date: 2011/07/30 17:30:07 $
*  $Source: /Nomis_NS8100/ns7100_Port/source/MonitorLogMenu.c,v $
*
*  $Revision: 1.1 $
*******************************************************************************/

/*******************************************************************************
*  Includes
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Menu.h"
#include "Display.h"
#include "Board.h"
#include "Rec.h"
#include "Uart.h"
#include "Display.h"
#include "Keypad.h"
#include "TextTypes.h"

/*******************************************************************************
*  Defines
*******************************************************************************/
#define MONITOR_LOG_MN_TABLE_SIZE 8
#define MONITOR_LOG_WND_STARTING_COL 6
#define MONITOR_LOG_WND_END_COL 127
#define MONITOR_LOG_WND_STARTING_ROW 8
#define MONITOR_LOG_WND_END_ROW 55
#define MONITOR_LOG_MN_TBL_START_LINE 0

/*******************************************************************************
*  Externs
*******************************************************************************/
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern USER_MENU_STRUCT configMenu[];
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
extern uint32 __monitorLogTblKey;
extern uint16 __monitorLogTblIndex;
extern uint16 __monitorLogUniqueEntryId;
extern MONITOR_LOG_ENTRY_STRUCT __monitorLogTbl[TOTAL_MONITOR_LOG_ENTRIES];
extern USER_MENU_STRUCT monitorLogMenu[];
extern REC_HELP_MN_STRUCT help_rec;

/*******************************************************************************
*  Globals
*******************************************************************************/
int16 monitorMnCurrentLogIndex = 0;
uint16 monitorMnLastLogIndex = 0;
uint16 monitorMnStartLogIndex = 0;

/*******************************************************************************
*  Prototypes
*******************************************************************************/
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

    if(active_menu == VIEW_MONITOR_LOG_MENU)
    {
        monitorLogMnDsply(&wnd_layout);
        writeMapToLcd(mmap);
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

			monitorMnCurrentLogIndex = (int16)getStartingMonitorLogTableIndex();
			monitorMnLastLogIndex = __monitorLogTblIndex;

			while((__monitorLogTbl[monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY) &&
					(monitorMnCurrentLogIndex != monitorMnLastLogIndex))
			{
				monitorMnCurrentLogIndex++;

				if(monitorMnCurrentLogIndex >= TOTAL_MONITOR_LOG_ENTRIES)
					monitorMnCurrentLogIndex = 0;
			}

			monitorMnStartLogIndex = (uint16)monitorMnCurrentLogIndex;

			if((monitorMnCurrentLogIndex == monitorMnLastLogIndex) && (__monitorLogTbl[monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY))
			{
				monitorMnCurrentLogIndex = -1;
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
					if((monitorMnCurrentLogIndex != monitorMnLastLogIndex) && (monitorMnCurrentLogIndex >= 0))
					{
						monitorMnCurrentLogIndex++;

						if(monitorMnCurrentLogIndex >= TOTAL_MONITOR_LOG_ENTRIES)
							monitorMnCurrentLogIndex = 0;

						while((__monitorLogTbl[monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY) &&
								(monitorMnCurrentLogIndex != monitorMnLastLogIndex))
						{
							monitorMnCurrentLogIndex++;

							if(monitorMnCurrentLogIndex >= TOTAL_MONITOR_LOG_ENTRIES)
								monitorMnCurrentLogIndex = 0;
						}
					}
				break;

				case (UP_ARROW_KEY):
					if((monitorMnCurrentLogIndex != monitorMnStartLogIndex) && (monitorMnCurrentLogIndex >= 0))
					{
						monitorMnCurrentLogIndex--;

						if(monitorMnCurrentLogIndex < 0)
							monitorMnCurrentLogIndex = TOTAL_MONITOR_LOG_ENTRIES - 1;

						while((__monitorLogTbl[monitorMnCurrentLogIndex].status != COMPLETED_LOG_ENTRY) &&
								(monitorMnCurrentLogIndex != monitorMnStartLogIndex))
						{
							monitorMnCurrentLogIndex--;

							if(monitorMnCurrentLogIndex < 0)
								monitorMnCurrentLogIndex = TOTAL_MONITOR_LOG_ENTRIES - 1;
						}
					}
				break;

				case (MINUS_KEY): adjustLcdContrast(DARKER); break;
				case (PLUS_KEY): adjustLcdContrast(LIGHTER); break;

				case (ESC_KEY):
					ACTIVATE_USER_MENU_MSG(&monitorLogMenu, DEFAULT_ITEM_1);
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
*	Function:	monitorLogMnDsply
*	Purpose:
****************************************/
void monitorLogMnDsply(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
    uint8 buff[25];
    uint8 length;

    byteSet(&(mmap[0][0]), 0, sizeof(mmap));

	// Add in a title for the menu
	byteSet(&buff[0], 0, sizeof(buff));
	length = (uint8)sprintf((char*)buff, "-%s-", getLangText(VIEW_MONITOR_LOG_TEXT));

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	if(monitorMnCurrentLogIndex != -1)
	{
		// Display Mode text
		byteSet(&buff[0], 0, sizeof(buff));
		switch(__monitorLogTbl[monitorMnCurrentLogIndex].mode)
		{
			case WAVEFORM_MODE:   length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(WAVEFORM_MODE_TEXT)); break;
			case BARGRAPH_MODE:	  length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(BARGRAPH_MODE_TEXT)); break;
			case MANUAL_CAL_MODE: length = (uint8)sprintf((char*)(&buff[0]), "%s", getLangText(CALIBRATION_TEXT)); break;
		}

		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// Display Start Time text
		byteSet(&buff[0], 0, sizeof(buff));
		convertTimeStampToString((char*)(&buff[0]), &__monitorLogTbl[monitorMnCurrentLogIndex].startTime, REC_DATE_TIME_TYPE);
		length = (uint8)strlen((char*)(&buff[0]));

		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// Display Stop Time text
		byteSet(&buff[0], 0, sizeof(buff));
		convertTimeStampToString((char*)(&buff[0]), &__monitorLogTbl[monitorMnCurrentLogIndex].stopTime, REC_DATE_TIME_TYPE);
		length = (uint8)strlen((char*)(&buff[0]));

		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// Display Number of Events recorded text
		if(__monitorLogTbl[monitorMnCurrentLogIndex].eventsRecorded == 0)
		{
			length = (uint8)sprintf((char*)(&buff[0]), "%s %s", getLangText(NO_TEXT), getLangText(EVENTS_RECORDED_TEXT));

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
		else
		{
			length = (uint8)sprintf((char*)(&buff[0]), "%s: %d", getLangText(EVENTS_RECORDED_TEXT), __monitorLogTbl[monitorMnCurrentLogIndex].eventsRecorded);

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			if(__monitorLogTbl[monitorMnCurrentLogIndex].eventsRecorded == 1)
			{
				length = (uint8)sprintf((char*)(&buff[0]), "EVENT #s: %d", __monitorLogTbl[monitorMnCurrentLogIndex].startEventNumber);
			}
			else
			{
				length = (uint8)sprintf((char*)(&buff[0]), "EVENT #s: %d-%d", __monitorLogTbl[monitorMnCurrentLogIndex].startEventNumber,
										(__monitorLogTbl[monitorMnCurrentLogIndex].startEventNumber + __monitorLogTbl[monitorMnCurrentLogIndex].eventsRecorded - 1));
			}

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}

		if(monitorMnCurrentLogIndex == monitorMnStartLogIndex)
		{
			// Display Start of Log
			byteSet(&buff[0], 0, sizeof(buff));
			length = (uint8)sprintf((char*)buff, "<%s>", "START OF LOG");

			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
			wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
		else if(monitorMnCurrentLogIndex == monitorMnLastLogIndex)
		{
			// Display End of Log
			byteSet(&buff[0], 0, sizeof(buff));
			length = (uint8)sprintf((char*)buff, "<%s>", "END OF LOG");

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
