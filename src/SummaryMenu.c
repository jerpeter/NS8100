///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: SummaryMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:00 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/SummaryMenu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Menu.h"
#include "EventProcessing.h"
#include "Summary.h"
#include "Uart.h"
#include "Display.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define SUMMARY_MN_TABLE_SIZE 		9 
#define SUMMARY_WND_STARTING_COL 	DEFAULT_COL_THREE
#define SUMMARY_WND_END_COL 		DEFAULT_END_COL
#define SUMMARY_WND_STARTING_ROW 	DEFAULT_MENU_ROW_ONE
#define SUMMARY_WND_END_ROW 		DEFAULT_MENU_ROW_SEVEN
#define SUMMARY_MN_TBL_START_LINE 	0
#define SUMMARY_MENU_ACTIVE_ITEMS	7
#define NO_EVENT_LINK	 			0xFFFFFFFF

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT configMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static SUMMARY_DATA *s_flashReadSummaryTablePtr = &__ramFlashSummaryTbl[0];
static uint16 s_topMenuSummaryIndex = 0;
static uint16 s_currentSummaryIndex = 0;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void summaryMn(INPUT_MSG_STRUCT);

void summaryMnProc(INPUT_MSG_STRUCT,
                   WND_LAYOUT_STRUCT *,
                   SUMMARY_DATA *);
                   
void dsplySummaryMn(WND_LAYOUT_STRUCT *,
                    SUMMARY_DATA *);
                    
void summaryMnScroll(char direction);
uint16 getFirstValidRamSummaryIndex(void);
uint16 getNextValidRamSummaryIndex(uint16 currentValidSummaryIndex);
uint16 getPreviousValidRamSummaryIndex(uint16 currentValidSummaryIndex);
BOOLEAN checkRamSummaryIndexForValidEventLink(uint16 ramSummaryIndex);

/****************************************
*	Function:	summaryMn
*	Purpose:
****************************************/
void summaryMn(INPUT_MSG_STRUCT msg)
{  
	static WND_LAYOUT_STRUCT wnd_layout;
	//static MN_LAYOUT_STRUCT mn_layout;
	//BOOL mode = 0;

	summaryMnProc(msg, &wnd_layout, s_flashReadSummaryTablePtr);
	
	if (g_activeMenu == SUMMARY_MENU)
	{
		dsplySummaryMn(&wnd_layout, s_flashReadSummaryTablePtr);
		writeMapToLcd(g_mmap);
	}
}

/****************************************
*	Function:	summaryMnProc
*	Purpose:
****************************************/
void summaryMnProc(INPUT_MSG_STRUCT msg,
                   WND_LAYOUT_STRUCT *wnd_layout_ptr,
                   SUMMARY_DATA *rd_summary_ptr)
{
	//uint16 i = 0;
	INPUT_MSG_STRUCT mn_msg;
	EVT_RECORD* eventRecord;
	uint16 tempSummaryIndex = 0;

	if (msg.cmd == ACTIVATE_MENU_CMD)
	{
		wnd_layout_ptr->start_col = SUMMARY_WND_STARTING_COL;   /* 6 */
		wnd_layout_ptr->end_col =   SUMMARY_WND_END_COL;        /* 127 leaving one pixel space at the end */
		wnd_layout_ptr->start_row = SUMMARY_WND_STARTING_ROW;   /* 8 */
		wnd_layout_ptr->end_row =   SUMMARY_WND_END_ROW;        /* 6 */

		g_summaryListMenuActive = YES;

		if (msg.data[0] == START_FROM_TOP)
		{
			// Find the first valid summary index and set the top and current index to match
			s_topMenuSummaryIndex = s_currentSummaryIndex = getFirstValidRamSummaryIndex();
			debug("First valid ram summary index: %d\n", s_topMenuSummaryIndex);
			
			g_summaryListArrowChar = DOWN_ARROW_CHAR;
		}
	}
	else if (msg.cmd == KEYPRESS_MENU_CMD)
	{
		switch (msg.data[0])
		{
			case (ENTER_KEY):
				// Check if the top menu summary index represents a valid index
				if (s_topMenuSummaryIndex < TOTAL_RAM_SUMMARIES)
				{
					g_resultsRamSummaryPtr = (rd_summary_ptr + s_currentSummaryIndex);
#if 0 // ns7100
					eventRecord = (EVT_RECORD *)g_resultsRamSummaryPtr->linkPtr;
#else // ns8100
					static EVT_RECORD resultsEventRecord;
					eventRecord = &resultsEventRecord;
	
					getEventFileRecord(__ramFlashSummaryTbl[s_currentSummaryIndex].fileEventNum, &resultsEventRecord);
					g_resultsRamSummaryPtr = &(__ramFlashSummaryTbl[s_currentSummaryIndex]);
					g_resultsRamSummaryIndex = s_currentSummaryIndex;
					g_updateResultsEventRecord = YES;

					debug("Summary menu: updating event record cache for event #%d, Addr Compare: 0x%x <-> 0x%x\n",
							__ramFlashSummaryTbl[s_currentSummaryIndex].fileEventNum, g_resultsRamSummaryPtr, g_resultsRamSummaryPtr);
#endif

char dateStr[50];
					debugRaw("\n\tEvt: %04d, Mode: %d\n", eventRecord->summary.eventNumber, eventRecord->summary.mode);
					convertTimeStampToString(&dateStr[0], &(eventRecord->summary.captured.calDate), REC_DATE_TIME_TYPE);
					debugRaw("\tCal Date: %s\n", dateStr);
					convertTimeStampToString(&dateStr[0], &(eventRecord->summary.captured.eventTime), REC_DATE_TIME_TYPE);
					debugRaw("\tEvent Time: %s\n", dateStr);
					convertTimeStampToString(&dateStr[0], &(eventRecord->summary.captured.endTime), REC_DATE_TIME_TYPE);
					debugRaw("\tStart Time: %s\n\n", dateStr);
					
					switch (eventRecord->summary.mode)
					{
						case WAVEFORM_MODE:
						case MANUAL_CAL_MODE:
							g_printOutMode = WAVEFORM_MODE;
							g_activeMenu = RESULTS_MENU;
							ACTIVATE_MENU_MSG();
							(*menufunc_ptrs[g_activeMenu]) (mn_msg);
							break;   
	                   case BARGRAPH_MODE: 
							g_printOutMode = BARGRAPH_MODE;
							g_activeMenu = RESULTS_MENU;
							ACTIVATE_MENU_MSG();
							(*menufunc_ptrs[g_activeMenu]) (mn_msg);
							break;    
						case COMBO_MODE:
							break;
						case MANUAL_TRIGGER_MODE:
							break;
						default:
							break;
					} 
				}
				break;
				
			case (DOWN_ARROW_KEY):
				// Check if the top menu summary index represents a valid index
				if (s_topMenuSummaryIndex < TOTAL_RAM_SUMMARIES)
				{
					summaryMnScroll(DOWN);
				}
				break;
			case (UP_ARROW_KEY):
				// Check if the top menu summary index represents a valid index
				if (s_topMenuSummaryIndex < TOTAL_RAM_SUMMARIES)
				{
					summaryMnScroll(UP);
				}
				break;
				
			case (ESC_KEY):
				tempSummaryIndex = getFirstValidRamSummaryIndex();
				
				// Check if the top menu summary index represents a valid index and if the current index isn't the first
				if ((s_topMenuSummaryIndex < TOTAL_RAM_SUMMARIES) && (tempSummaryIndex != s_currentSummaryIndex))
				{
					s_topMenuSummaryIndex = s_currentSummaryIndex = tempSummaryIndex;

					g_summaryListArrowChar = DOWN_ARROW_CHAR;

					debug("Summary List: Setting Current Index to first valid Summary index\n");
				}
				else // We're done here
				{
					g_summaryListMenuActive = NO;

					ACTIVATE_USER_MENU_MSG(&configMenu, SUMMARIES_EVENTS);
					(*menufunc_ptrs[g_activeMenu]) (mn_msg);
				}
				break;
			default:
				break;
		}
	}
}

/****************************************
*	Function:	 dsplySummaryMn
*	Purpose:
****************************************/
void dsplySummaryMn(WND_LAYOUT_STRUCT *wnd_layout_ptr,
                    SUMMARY_DATA *rd_summary_ptr)
{
	char dateBuff[25];
	char lineBuff[30];
	char modeBuff[2];
	uint16 itemsDisplayed = 1;
	uint16 length;
	uint16 tempSummaryIndex = 0;
	EVT_RECORD* eventRecord;

	// Clear the LCD map
	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	// Display the Title centered on the Top line
	sprintf(lineBuff, "-%s-", getLangText(LIST_OF_SUMMARIES_TEXT));
	length = (uint8)strlen((char*)lineBuff);
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));          
	wndMpWrtString((uint8*)(&lineBuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// Setup layout
	wnd_layout_ptr->curr_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col = wnd_layout_ptr->start_col;

	// Check if s_topMenuSummaryIndex is valid
	if (s_topMenuSummaryIndex == TOTAL_RAM_SUMMARIES)
	{
		debug("Summary List: No valid summary found for display\n");	
		sprintf(lineBuff, "<%s>", getLangText(EMPTY_TEXT));
		wndMpWrtString((uint8*)(&lineBuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		// We're done
	}
	else // s_topMenuSummaryIndex is a valid index indicating valid events in storage
	{
		tempSummaryIndex = s_topMenuSummaryIndex;

		while ((itemsDisplayed <= SUMMARY_MENU_ACTIVE_ITEMS) && (tempSummaryIndex < TOTAL_RAM_SUMMARIES))
		{
#if 0 // ns7100
			eventRecord = (EVT_RECORD *)((rd_summary_ptr + tempSummaryIndex)->linkPtr);
#else // ns8100
			static EVT_RECORD resultsEventRecord;
			eventRecord = &resultsEventRecord;
	
			debug("Summary menu: updating event record cache\n");
			getEventFileRecord(__ramFlashSummaryTbl[tempSummaryIndex].fileEventNum, &resultsEventRecord);
#endif

			// Clear and setup the time stamp string for the current event
			byteSet(&dateBuff[0], 0, sizeof(dateBuff));
			convertTimeStampToString(dateBuff, (void*)(&eventRecord->summary.captured.eventTime), 
										REC_DATE_TIME_DISPLAY);

			// Clear and setup the mode string for the curent event
			byteSet(&modeBuff[0], 0, sizeof(modeBuff));
			switch (eventRecord->summary.mode)
			{
				case WAVEFORM_MODE: 		strcpy(modeBuff, "W"); break;
				case BARGRAPH_MODE: 		strcpy(modeBuff, "B"); break;
				case COMBO_MODE:    		strcpy(modeBuff, "C"); break;
				case MANUAL_CAL_MODE:		strcpy(modeBuff, "P"); break;
				case MANUAL_TRIGGER_MODE:	strcpy(modeBuff, "M"); break;
			}
			
			// Clear and setup the event line string for the curent event
			byteSet(&lineBuff[0], 0, sizeof(lineBuff));
			sprintf(lineBuff, "E%03d %s %s", eventRecord->summary.eventNumber, dateBuff, modeBuff);

			// Check if the current line is to be highlighted
			if (tempSummaryIndex == s_currentSummaryIndex)
			{           
				wndMpWrtString((uint8*)(&lineBuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_LN);
			}
			else // Print as a regular line
			{   
				wndMpWrtString((uint8*)(&lineBuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			}

			// Set the current row as the next row
			wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

			// Increment the items displayed
			itemsDisplayed++;
			
			tempSummaryIndex = getNextValidRamSummaryIndex(tempSummaryIndex);
		}

		// Check if the summary index is at the end of the list and still room on the LCD
		if ((itemsDisplayed <= SUMMARY_MENU_ACTIVE_ITEMS) && (tempSummaryIndex == TOTAL_RAM_SUMMARIES))
		{
			debug("Summary List: End of the list\n");	
			sprintf(lineBuff, "<%s>", getLangText(END_TEXT));
			wndMpWrtString((uint8*)(&lineBuff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
	}
}

/****************************************
*	Function:	summaryMnScroll
*	Purpose:
****************************************/
void summaryMnScroll(char direction)
{  
	uint16 tempSummaryIndex = 0;
	uint16 compareCurrentSummaryIndex = s_currentSummaryIndex;
	uint16 i = 0;
	
	g_summaryListArrowChar = BOTH_ARROWS_CHAR;
	
	for(i=0;i<g_keypadNumberSpeed;i++)
	{	
		if (direction == DOWN)
		{
			tempSummaryIndex = getNextValidRamSummaryIndex(s_currentSummaryIndex);
			
			if (tempSummaryIndex < TOTAL_RAM_SUMMARIES)
			{
				s_currentSummaryIndex = tempSummaryIndex;
				
				if ((s_currentSummaryIndex - s_topMenuSummaryIndex) >= SUMMARY_MENU_ACTIVE_ITEMS)
				{
					s_topMenuSummaryIndex = getNextValidRamSummaryIndex(s_topMenuSummaryIndex);
				}
			}
			else // For the last item, bump the top menu summary index one to show the end of the list
			{
				if ((s_currentSummaryIndex - s_topMenuSummaryIndex) >= (SUMMARY_MENU_ACTIVE_ITEMS - 1))
				{
					s_topMenuSummaryIndex = getNextValidRamSummaryIndex(s_topMenuSummaryIndex);
				}
			}
			
			compareCurrentSummaryIndex = getNextValidRamSummaryIndex(s_currentSummaryIndex);
			if(compareCurrentSummaryIndex == TOTAL_RAM_SUMMARIES)
			{
				g_summaryListArrowChar = UP_ARROW_CHAR;
			}

			//debug("Summary List: Scroll Down, Top Menu Index: %d, Current Index: %d\n",
			//		s_topMenuSummaryIndex, s_currentSummaryIndex);
		}
		else if (direction == UP)
		{
			tempSummaryIndex = getPreviousValidRamSummaryIndex(s_currentSummaryIndex);
			
			if (tempSummaryIndex < TOTAL_RAM_SUMMARIES)
			{
				s_currentSummaryIndex = tempSummaryIndex;
				
				if (s_currentSummaryIndex < s_topMenuSummaryIndex)
				{
					s_topMenuSummaryIndex = s_currentSummaryIndex;
				}
			}

			compareCurrentSummaryIndex = getPreviousValidRamSummaryIndex(s_currentSummaryIndex);
			if(compareCurrentSummaryIndex == TOTAL_RAM_SUMMARIES)
			{
				g_summaryListArrowChar = DOWN_ARROW_CHAR;
			}

			//debug("Summary List: Scroll Up, Top Menu Index: %d, Current Index: %d\n",
			//		s_topMenuSummaryIndex, s_currentSummaryIndex);
		}
	}
}
    
/****************************************
*	Function:	getFirstValidRamSummaryIndex
*	Purpose:
****************************************/
uint16 getFirstValidRamSummaryIndex(void)
{
	uint16 ramSummaryIndex = 0;
	
	while ((ramSummaryIndex < TOTAL_RAM_SUMMARIES) && (checkRamSummaryIndexForValidEventLink(ramSummaryIndex) == NO))
		ramSummaryIndex++;

	return (ramSummaryIndex);
}

/****************************************
*	Function:	getNextValidRamSummaryIndex
*	Purpose:
****************************************/
uint16 getNextValidRamSummaryIndex(uint16 currentValidSummaryIndex)
{
	uint16 ramSummaryIndex = currentValidSummaryIndex;

	if (ramSummaryIndex < TOTAL_RAM_SUMMARIES)
	{
		while (checkRamSummaryIndexForValidEventLink(++ramSummaryIndex) == NO)
		{
			if (ramSummaryIndex == TOTAL_RAM_SUMMARIES)
			{
				break;
			}
		}
	}

	return (ramSummaryIndex);
}

/****************************************
*	Function:	getPreviousValidRamSummaryIndex
*	Purpose:
****************************************/
uint16 getPreviousValidRamSummaryIndex(uint16 currentValidSummaryIndex)
{
	uint16 ramSummaryIndex = currentValidSummaryIndex;

	if (ramSummaryIndex == 0)
	{
		ramSummaryIndex = TOTAL_RAM_SUMMARIES;
	}
	else
	{
		while (checkRamSummaryIndexForValidEventLink(--ramSummaryIndex) == NO)
		{
			if (ramSummaryIndex == 0)
			{
				ramSummaryIndex = TOTAL_RAM_SUMMARIES;
				break;
			}
		}
	}

	return (ramSummaryIndex);
}

/****************************************
*	Function:	checkRamSummaryIndexForEventLink
*	Purpose:
****************************************/
BOOLEAN checkRamSummaryIndexForValidEventLink(uint16 ramSummaryIndex)
{
	BOOLEAN validEventLink = NO;

	if (ramSummaryIndex < TOTAL_RAM_SUMMARIES)
	{
		if ((uint32)(__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum) != NO_EVENT_LINK)
		{
			validEventLink = YES;
		}
	}

	return (validEventLink);
}



















