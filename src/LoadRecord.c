///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: LoadRecord.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/LoadRecord.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Menu.h"
#include "Display.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define LOAD_REC_MN_TABLE_SIZE		8 
#define LOAD_REC_WND_STARTING_COL	DEFAULT_COL_THREE
#define LOAD_REC_WND_END_COL		DEFAULT_END_COL 
#define LOAD_REC_WND_STARTING_ROW	DEFAULT_MENU_ROW_ONE
#define LOAD_REC_WND_END_ROW		DEFAULT_MENU_ROW_SEVEN             

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern MN_MEM_DATA_STRUCT mn_ptr[DEFAULT_MN_SIZE];
extern int32 active_menu;
extern REC_EVENT_MN_STRUCT trig_rec;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern USER_MENU_STRUCT modeMenu[];
extern USER_MENU_STRUCT helpMenu[];
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
TEMP_MENU_DATA_STRUCT load_rec_mn_table_test [LOAD_REC_MN_TABLE_SIZE] = {
{TITLE_PRE_TAG, SAVED_SETTINGS_TEXT, TITLE_POST_TAG},
{NO_TAG, DEFAULT_SELF_TRG_TEXT, NO_TAG},
{NO_TAG, DEFAULT_BAR_TEXT, NO_TAG},
{NO_TAG, DEFAULT_COMBO_TEXT, NO_TAG},
{NO_TAG, TOTAL_TEXT_STRINGS, NO_TAG}
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void loadRecMn (INPUT_MSG_STRUCT);
void loadRecMnProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);

/****************************************
*	Function:	loadRecMn
*	Purpose:
****************************************/
void loadRecMn(INPUT_MSG_STRUCT msg)
{ 
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;
  
    loadRecMnProc(msg, &wnd_layout, &mn_layout);           

    if (active_menu == LOAD_REC_MENU)
    {
        dsplySelMn(&wnd_layout, &mn_layout, TITLE_CENTERED);
        writeMapToLcd(mmap);
    }
}

/****************************************
*	Function:	loadRecMnProc
*	Purpose:
****************************************/
void loadRecMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	uint8 buff[20];
	REC_EVENT_MN_STRUCT temp_rec;
	INPUT_MSG_STRUCT mn_msg;
	uint32 input;
	uint8 i;
	char message[50];

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col = LOAD_REC_WND_STARTING_COL;
			wnd_layout_ptr->end_col =   LOAD_REC_WND_END_COL;
			wnd_layout_ptr->start_row = LOAD_REC_WND_STARTING_ROW;
			wnd_layout_ptr->end_row =   LOAD_REC_WND_END_ROW;

			mn_layout_ptr->curr_ln =    1;
			mn_layout_ptr->top_ln =     1; 

			loadTempMenuTable(load_rec_mn_table_test);

			for (i = 1; i <= MAX_NUM_OF_SAVED_SETUPS; i++)
			{
				getRecData(&temp_rec, i, REC_TRIGGER_USER_MENU_TYPE);

				if (temp_rec.validRecord == YES)
				{
					switch (temp_rec.op_mode)
					{
						case (WAVEFORM_MODE):
							sprintf((char*)buff, "%s (%s)", (char*)temp_rec.name, getLangText(SELF_TRG_TEXT));
							break;
						case (BARGRAPH_MODE):
							sprintf((char*)buff, "%s (%s)", (char*)temp_rec.name, getLangText(BAR_TEXT));
							break;
						case (COMBO_MODE):
							sprintf((char*)buff, "%s (%s)", (char*)temp_rec.name, getLangText(COMBO_TEXT));
							break;
						default:
							sprintf((char*)buff, "%s (UNK)", (char*)temp_rec.name);
							break;
					}

				}
				else
				{
					sprintf((char*)buff, "<%s>", getLangText(EMPTY_TEXT));
				}

				// Add 3 to i to offset past the default settings
				strcpy((char*)&(mn_ptr[i + 3].data[0]), (char*)buff);
			}

			// Add in the "<END>" text to signal the end of the list
			sprintf((char*)buff, "<%s>", getLangText(END_TEXT));
			strcpy((char*)&(mn_ptr[i + 3].data[0]), (char*)buff);
			i++;

			// Add the end string to allow other routines to match on the end of the menu
			strcpy((char*)&(mn_ptr[i + 3].data[0]), ".end.");
			break;

		case (KEYPRESS_MENU_CMD):
			input = msg.data[0];
			switch (input)
			{
				case (ENTER_KEY):
					switch (mn_layout_ptr->curr_ln)
					{
						case (1): 
							loadTrigRecordDefaults(&trig_rec, WAVEFORM_MODE);
							break; 
						case (2): 
							loadTrigRecordDefaults(&trig_rec, BARGRAPH_MODE);
							break;
						case (3):
							messageBox(getLangText(STATUS_TEXT), getLangText(COMBO_MODE_NOT_IMPLEMENTED_TEXT), MB_OK);
							messageBox(getLangText(STATUS_TEXT), getLangText(SETTINGS_WILL_NOT_BE_LOADED_TEXT), MB_OK);

							// Do nothing
							return;
							break;  
						default:
							// Check if the first char matches a "<" for either "<EMPTY>" or "<END>"
							if (strncmp((char*)&mn_ptr[mn_layout_ptr->curr_ln].data[0], "<", 1) == 0)
							{
								// Record slot is empty, do nothing
								return;
							}
							else
							{
								getRecData(&trig_rec, (uint32)(mn_layout_ptr->curr_ln - 3), REC_TRIGGER_USER_MENU_TYPE);
							}
							break;
					}

					updateModeMenuTitle(trig_rec.op_mode);
					ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
					(*menufunc_ptrs[active_menu]) (mn_msg);
					break;      

				case (DELETE_KEY):
					// Check if the current line is beyond the default entries
					if (mn_layout_ptr->curr_ln > 3)
					{
						getRecData(&temp_rec, (uint32)(mn_layout_ptr->curr_ln - 3), REC_TRIGGER_USER_MENU_TYPE);

						if (temp_rec.validRecord == YES)
						{
							byteSet(&message[0], 0, sizeof(message));
							sprintf(message, "%s (%s)", getLangText(DELETE_SAVED_SETUP_Q_TEXT), temp_rec.name);

							if (messageBox(getLangText(WARNING_TEXT), message, MB_YESNO) == MB_FIRST_CHOICE)					
							{
								byteSet(&temp_rec.name, 0, sizeof(temp_rec.name));
								temp_rec.validRecord = NO;

								saveRecData(&temp_rec, (uint32)(mn_layout_ptr->curr_ln - 3), REC_TRIGGER_USER_MENU_TYPE);
								
								sprintf((char*)&(mn_ptr[mn_layout_ptr->curr_ln].data[0]), "<%s>", getLangText(EMPTY_TEXT));
							}
						}
					}
					break;

				case (DOWN_ARROW_KEY): 
					mnScroll(DOWN, SELECT_MN_WND_LNS, mn_layout_ptr);

					// Prevent the cursor from selecting the "<END>" text
					if (mn_layout_ptr->curr_ln == 18)
					{
						mn_layout_ptr->curr_ln = 17;
					}
					break;
				case (UP_ARROW_KEY): 
					mnScroll(UP, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (ESC_KEY):
					active_menu = MAIN_MENU;
					ACTIVATE_MENU_MSG(); mn_msg.data[0] = ESC_KEY;
					(*menufunc_ptrs[active_menu]) (mn_msg);
					break;
				case (HELP_KEY):
					ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
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

