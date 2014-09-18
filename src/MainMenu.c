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
#include "Globals.h"
extern USER_MENU_STRUCT modeMenu[];
extern USER_MENU_STRUCT helpMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static TEMP_MENU_DATA_STRUCT s_mainMenuTable[MAIN_MN_TABLE_SIZE] = {
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
void MainMenu(INPUT_MSG_STRUCT);
void MainMenuProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void MainMenuScroll(char, char, MN_LAYOUT_STRUCT*);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MainMenu(INPUT_MSG_STRUCT msg)
{ 
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;
  
    MainMenuProc(msg, &wnd_layout, &mn_layout);

    if (g_activeMenu == MAIN_MENU)
    {
        DisplaySelectMenu(&wnd_layout, &mn_layout, TITLE_CENTERED);
        WriteMapToLcd(g_mmap);
    }
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MainMenuProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	DATE_TIME_STRUCT currentTime = GetCurrentTime();
	uint32 input;
	uint8 length;
#if 0 // Test (Storage for override of main menu keys)
	RTC_MEM_MAP_STRUCT rtcMap;
#endif

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

			LoadTempMenuTable(s_mainMenuTable);
			
			// Add in time (hour:min) to the 2nd LCD line right justified
			length = strlen(getLangText(SELECT_TEXT));
			memset(&(g_menuPtr[1].data[length]), ' ', (12 - length));
			sprintf((char*)&(g_menuPtr[1].data[12]), "%02d:%02d %s", ((currentTime.hour % 12) == 0) ? 12 : (currentTime.hour % 12),
					currentTime.min, ((currentTime.hour / 12) == 1) ? "PM" : "AM");

			sprintf((char*)&(g_menuPtr[2].data[0]), "_____________________");

			// Since time was added, start the menu update timer
			AssignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, MenuUpdateTimerCallBack);
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
						case (DEFAULT_ROW_3): // Waveform
							if ((g_factorySetupRecord.invalid) || (g_lowBatteryState == YES)) { PromptUserUnableToEnterMonitoring(); }
							else
							{
								g_triggerRecord.op_mode = WAVEFORM_MODE;
								ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
								UpdateModeMenuTitle(g_triggerRecord.op_mode);
								SETUP_USER_MENU_MSG(&modeMenu, MONITOR);
								JUMP_TO_ACTIVE_MENU();
							}
							break; 

						case (DEFAULT_ROW_4): // Bargraph
							if ((g_factorySetupRecord.invalid) || (g_lowBatteryState == YES)) { PromptUserUnableToEnterMonitoring(); }
							else
							{
								g_triggerRecord.op_mode = BARGRAPH_MODE;
								ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
								UpdateModeMenuTitle(g_triggerRecord.op_mode);
								SETUP_USER_MENU_MSG(&modeMenu, MONITOR);
								JUMP_TO_ACTIVE_MENU();
							}
							break;

						case (DEFAULT_ROW_5): // Combo
							if ((g_factorySetupRecord.invalid) || (g_lowBatteryState == YES)) { PromptUserUnableToEnterMonitoring(); }
							else
							{
								g_triggerRecord.op_mode = COMBO_MODE;
								ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
								UpdateModeMenuTitle(g_triggerRecord.op_mode);
								SETUP_USER_MENU_MSG(&modeMenu, MONITOR);
								JUMP_TO_ACTIVE_MENU();
							}
							break;

						case (DEFAULT_ROW_6): // Load Saved Record
							ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
							SETUP_MENU_MSG(LOAD_REC_MENU);
							JUMP_TO_ACTIVE_MENU();
							break;
						default:
							break;
					}
					break;      

				case (DOWN_ARROW_KEY):
					MainMenuScroll(DOWN, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (UP_ARROW_KEY):
					MainMenuScroll(UP, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (MINUS_KEY): 
					AdjustLcdContrast(DARKER);
					break;
				case (PLUS_KEY): 
					AdjustLcdContrast(LIGHTER);
					break;
				case (ESC_KEY):
					// Reset the current line to tbe the top line	
					mn_layout_ptr->curr_ln = 3;
					break;
				case (HELP_KEY):
					ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
					SETUP_USER_MENU_MSG(&helpMenu, CONFIG);
					JUMP_TO_ACTIVE_MENU();
					break;
				default:
					break;
			}
			break;

		default:
			// Menu called without action, most likely the menu update timer
			sprintf((char*)&(g_menuPtr[1].data[12]), "%02d:%02d %s", ((currentTime.hour % 12) == 0) ? 12 : (currentTime.hour % 12),
					currentTime.min, ((currentTime.hour / 12) == 1) ? "PM" : "AM");

			// Since time was added, start the menu update timer
			AssignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, MenuUpdateTimerCallBack);
			break;    
	}

}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MainMenuScroll(char direction, char wnd_size, MN_LAYOUT_STRUCT * mn_layout_ptr)
{   
   uint8 buff[50];
   
   strcpy((char*)buff, (char*)(g_menuPtr + mn_layout_ptr->curr_ln + 1)->data);

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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PromptUserUnableToEnterMonitoring(void)
{
	if (g_factorySetupRecord.invalid)
	{
		debugWarn("Factory setup record not found.\r\n");
		MessageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
	}
	else if (g_lowBatteryState == YES)
	{
		debugWarn("Monitoring unavailable due to low battery voltage\r\n");
		sprintf((char*)g_spareBuffer, "%s %s (%3.2f)", getLangText(BATTERY_VOLTAGE_TEXT), getLangText(LOW_TEXT), (GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE)));
		OverlayMessage(getLangText(WARNING_TEXT), (char*)g_spareBuffer, (3 * SOFT_SECS));
	}
}
