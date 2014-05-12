///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: MainMenu.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/MainMenu.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Common.h"
#include "Menu.h"
#include "Display.h"
#include "Uart.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define MAIN_MN_TABLE_SIZE		10 
#define MAIN_WND_STARTING_COL	6
#define MAIN_WND_END_COL		127 
#define MAIN_WND_STARTING_ROW	8
#define MAIN_WND_END_ROW		55             
#define MAIN_MN_TBL_START_LINE	0
                                                                                                                                                
///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern MN_MEM_DATA_STRUCT mn_ptr[DEFAULT_MN_SIZE];
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern REC_EVENT_MN_STRUCT trig_rec;
extern USER_MENU_STRUCT modeMenu[];
extern USER_MENU_STRUCT helpMenu[];
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
TEMP_MENU_DATA_STRUCT main_mn_table_test[MAIN_MN_TABLE_SIZE] = {
{MAIN_PRE_TAG, NOMIS_MAIN_MENU_TEXT, MAIN_POST_TAG},
{NO_TAG, SELECT_TEXT, NO_TAG},
{NO_TAG, NULL_TEXT, NO_TAG},
{ITEM_1, SELF_TRIGGER_TEXT, NO_TAG},
{ITEM_2, BAR_GRAPH_TEXT, NO_TAG},
{ITEM_3, COMBO_TEXT, NO_TAG},
{ITEM_4, SAVED_SETTINGS_TEXT, NO_TAG},
{NO_TAG, TOTAL_TEXT_STRINGS, NO_TAG}
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void mainMn(INPUT_MSG_STRUCT);
void mainMnProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void mainMnScroll(char, char, MN_LAYOUT_STRUCT*);

/****************************************
*	Function:	mainMn
*	Purpose:
****************************************/
void mainMn(INPUT_MSG_STRUCT msg)
{ 
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;
  
    mainMnProc(msg, &wnd_layout, &mn_layout);           

    if (active_menu == MAIN_MENU)
    {
        dsplySelMn(&wnd_layout, &mn_layout, TITLE_CENTERED);
        writeMapToLcd(mmap);
    }
}

/****************************************
*	Function:	mainMnProc
*	Purpose:
****************************************/
void mainMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	uint32 input;

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col = MAIN_WND_STARTING_COL;
			wnd_layout_ptr->end_col =   MAIN_WND_END_COL;
			wnd_layout_ptr->start_row = MAIN_WND_STARTING_ROW;
			wnd_layout_ptr->end_row =   MAIN_WND_END_ROW;

			mn_layout_ptr->top_ln = 1;
			mn_layout_ptr->sub_ln = 0;

			if (msg.data[0] != ESC_KEY)
			{
				mn_layout_ptr->curr_ln = 3;
			}
			else
			{
				if (mn_layout_ptr->curr_ln > 6)
				mn_layout_ptr->top_ln = (uint16)(mn_layout_ptr->curr_ln - 5);
			}

			loadTempMenuTable(main_mn_table_test);
			break;

		case (KEYPRESS_MENU_CMD):
			input = msg.data[0];
			// Handle converting a number key input into a selection
			if ((input >= ONE_KEY) && (input <= FOUR_KEY))
			{
				// Convert ASCII data to a hex number to set the current line
				mn_layout_ptr->curr_ln = (uint16)((input - 0x30) + 2);
				input = ENTER_KEY;
			}

			switch (input)
			{
				case (ENTER_KEY):
					switch (mn_layout_ptr->curr_ln)
					{
						case (3):
							if (factory_setup_rec.invalid)
							{
								debugWarn("Factory setup record not found.\n");
								messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
							}
							else
							{
								trig_rec.op_mode = WAVEFORM_MODE;
								updateModeMenuTitle(trig_rec.op_mode);
								ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
								(*menufunc_ptrs[active_menu]) (mn_msg);
							}
							break; 
						case (4):
							if (factory_setup_rec.invalid)
							{
								debugWarn("Factory setup record not found.\n");
								messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
							}
							else
							{
								trig_rec.op_mode = BARGRAPH_MODE;
								updateModeMenuTitle(trig_rec.op_mode);
								ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
								(*menufunc_ptrs[active_menu]) (mn_msg);
							}
							break;
						case (5): // Combo mode - not operational yet
							messageBox(getLangText(STATUS_TEXT), getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT), MB_OK);
							break;
						case (6): active_menu = LOAD_REC_MENU;
							ACTIVATE_MENU_MSG(); (*menufunc_ptrs[active_menu]) (mn_msg);
							break;
						default:
							break;
					}
					break;      
				case (DOWN_ARROW_KEY):
					mainMnScroll(DOWN, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (UP_ARROW_KEY):
					mainMnScroll(UP, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (MINUS_KEY): adjustLcdContrast(DARKER); break;
				case (PLUS_KEY): adjustLcdContrast(LIGHTER); break;
				case (ESC_KEY):
					// Reset the current line to tbe the top line	
					mn_layout_ptr->curr_ln = 3;
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

/****************************************
*	Function:	mainMnScroll
*	Purpose:
****************************************/
void mainMnScroll(char direction, char wnd_size, MN_LAYOUT_STRUCT * mn_layout_ptr)
{   
   uint8 buff[50];
   
   strcpy((char*)buff, (char*)(mn_ptr + mn_layout_ptr->curr_ln + 1)->data);

   switch (direction)
   {
      case (DOWN):
           if (strcmp((char*)buff, ".end."))
           {
              mn_layout_ptr->curr_ln++;

              if ((mn_layout_ptr->curr_ln - mn_layout_ptr->top_ln) >= wnd_size) 
              {
                 mn_layout_ptr->top_ln++;
              }
           }
           break;
           
      case (UP):
           if (mn_layout_ptr->curr_ln > 3)
           {
              if (mn_layout_ptr->curr_ln == (mn_layout_ptr->top_ln + 2))
              {
                if (mn_layout_ptr->top_ln > 1)
                {
                    mn_layout_ptr->top_ln--;
                }
              }

              mn_layout_ptr->curr_ln--;
           }
           break;

      default:
           break; 
   }
   
}
