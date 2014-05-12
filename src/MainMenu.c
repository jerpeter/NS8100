///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: MainMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:51 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/MainMenu.c,v $
///	$Revision: 1.2 $
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

    if (g_activeMenu == MAIN_MENU)
    {
        dsplySelMn(&wnd_layout, &mn_layout, TITLE_CENTERED);
        writeMapToLcd(g_mmap);
    }
}

/****************************************
*	Function:	mainMnProc
*	Purpose:
****************************************/
void mainMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	DATE_TIME_STRUCT currentTime = getCurrentTime();
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

			loadTempMenuTable(s_mainMenuTable);
			
			// Add in time (hour:min) to the 2nd LCD line right justified
			length = strlen(getLangText(SELECT_TEXT));
			byteSet(&(g_menuPtr[1].data[length]), ' ', (12 - length));
			sprintf((char*)&(g_menuPtr[1].data[12]), "%02d:%02d %s", ((currentTime.hour % 12) == 0) ? 12 : (currentTime.hour % 12),
					currentTime.min, ((currentTime.hour / 12) == 1) ? "PM" : "AM");

			sprintf((char*)&(g_menuPtr[2].data[0]), "_____________________");

			// Since time was added, start the menu update timer
			assignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, menuUpdateTimerCallBack);
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
							if (g_factorySetupRecord.invalid)
							{
								debugWarn("Factory setup record not found.\n");
								messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
							}
							else
							{
								g_triggerRecord.op_mode = WAVEFORM_MODE;
								clearSoftTimer(MENU_UPDATE_TIMER_NUM);
								updateModeMenuTitle(g_triggerRecord.op_mode);
								ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
								(*menufunc_ptrs[g_activeMenu]) (mn_msg);
							}
							break; 

						case (DEFAULT_ROW_4): // Bargraph
							if (g_factorySetupRecord.invalid)
							{
								debugWarn("Factory setup record not found.\n");
								messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
							}
							else
							{
								g_triggerRecord.op_mode = BARGRAPH_MODE;
								clearSoftTimer(MENU_UPDATE_TIMER_NUM);
								updateModeMenuTitle(g_triggerRecord.op_mode);
								ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
								(*menufunc_ptrs[g_activeMenu]) (mn_msg);
							}
							break;

						case (DEFAULT_ROW_5): // Combo
							if (g_factorySetupRecord.invalid)
							{
								debugWarn("Factory setup record not found.\n");
								messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
							}
							else
							{
								g_triggerRecord.op_mode = COMBO_MODE;
								clearSoftTimer(MENU_UPDATE_TIMER_NUM);
								updateModeMenuTitle(g_triggerRecord.op_mode);
								ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
								(*menufunc_ptrs[g_activeMenu]) (mn_msg);
							}
							break;

						case (DEFAULT_ROW_6): // Load Saved Record
							clearSoftTimer(MENU_UPDATE_TIMER_NUM);
							g_activeMenu = LOAD_REC_MENU;
							ACTIVATE_MENU_MSG(); (*menufunc_ptrs[g_activeMenu]) (mn_msg);
							break;
						default:
							break;
					}
					break;      

				case (DOWN_ARROW_KEY):
#if 0 // Test (Override of the key for testing)
					// Set RTC Timestamp pin high
					gpio_clr_gpio_pin(AVR32_PIN_PB18);
#endif
					mainMnScroll(DOWN, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (UP_ARROW_KEY):
#if 0 // Test (Override of the key for testing)
					// Set RTC Timestamp pin high
					gpio_set_gpio_pin(AVR32_PIN_PB18);
#endif
					mainMnScroll(UP, SELECT_MN_WND_LNS, mn_layout_ptr);
					break;
				case (MINUS_KEY): 
#if 0 // Test (Override of the key for testing)
					//powerDownSDCard();
				    //gpio_enable_gpio_pin(AVR32_PIN_PA19);
					//gpio_clr_gpio_pin(AVR32_PIN_PA19);

					rtcRead(RTC_CONTROL_1_ADDR, 2, &rtcMap.control_1);
					debug("RTC Control 1: 0x%x, Control 2: 0x%x\n", rtcMap.control_1, rtcMap.control_2);
					rtcMap.control_1 &= ~(0x10);
					rtcMap.control_2 = RTC_ALARM_INT_ENABLE;
					debug("RTC New Control 1: 0x%x, Control 2: 0x%x\n", rtcMap.control_1, rtcMap.control_2);
					rtcWrite(RTC_CONTROL_1_ADDR, 2, &rtcMap.control_1);
#else
					adjustLcdContrast(DARKER);
#endif
					break;
				case (PLUS_KEY): 
#if 0 // Test (Override of the key for testing)
				    //gpio_set_gpio_pin(AVR32_PIN_PB19);
					//gpio_enable_module_pin(AVR32_PIN_PA19, AVR32_SPI1_NPCS_2_0_FUNCTION);
					//powerUpSDCardAndInitFat32();

					rtcRead(RTC_CONTROL_1_ADDR, 2, &rtcMap.control_1);
					debug("RTC Control 1: 0x%x, Control 2: 0x%x\n", rtcMap.control_1, rtcMap.control_2);
					rtcMap.control_1 &= ~(0x10);
					rtcMap.control_2 = (RTC_ALARM_INT_ENABLE | RTC_TIMESTAMP_INT_ENABLE);
					debug("RTC New Control 1: 0x%x, Control 2: 0x%x\n", rtcMap.control_1, rtcMap.control_2);
					rtcWrite(RTC_CONTROL_1_ADDR, 2, &rtcMap.control_1);
#else
					adjustLcdContrast(LIGHTER); 
#endif
					break;
				case (ESC_KEY):
					// Reset the current line to tbe the top line	
					mn_layout_ptr->curr_ln = 3;
					break;
				case (HELP_KEY):
					clearSoftTimer(MENU_UPDATE_TIMER_NUM);
					ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
					(*menufunc_ptrs[g_activeMenu]) (mn_msg);
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
			assignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, menuUpdateTimerCallBack);
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
