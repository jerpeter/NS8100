///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: BatteryMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:46 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/BatteryMenu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Menu.h"
#include "Display.h"
#include "Old_Board.h"
#include "Uart.h"
#include "Display.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define BATTERY_MN_TABLE_SIZE 8
#define BATTERY_WND_STARTING_COL 6
#define BATTERY_WND_END_COL 127
#define BATTERY_WND_STARTING_ROW 8
#define BATTERY_WND_END_ROW 55
#define BATTERY_MN_TBL_START_LINE 0

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern USER_MENU_STRUCT configMenu[];
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void batteryMn (INPUT_MSG_STRUCT);
void batteryMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT *);
void batteryMnDsply(WND_LAYOUT_STRUCT*);

/****************************************
*	Function:	batteryMn
*	Purpose:
****************************************/
void batteryMn(INPUT_MSG_STRUCT msg)
{
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;

    batteryMnProc(msg, &wnd_layout, &mn_layout);

    if (active_menu == BATTERY_MENU)
    {
        batteryMnDsply(&wnd_layout);
        writeMapToLcd(mmap);
    }
}

/****************************************
*	Function:	batteryMnProc
*	Purpose:
****************************************/
void batteryMnProc(INPUT_MSG_STRUCT msg,
                   WND_LAYOUT_STRUCT *wnd_layout_ptr,
                   MN_LAYOUT_STRUCT *mn_layout_ptr)
{

   INPUT_MSG_STRUCT mn_msg;
   uint32 input;

   switch (msg.cmd)
   {
      case (ACTIVATE_MENU_CMD):
            wnd_layout_ptr->start_col = BATTERY_WND_STARTING_COL;
            wnd_layout_ptr->end_col =   BATTERY_WND_END_COL;
            wnd_layout_ptr->start_row = BATTERY_WND_STARTING_ROW;
            wnd_layout_ptr->end_row =   BATTERY_WND_END_ROW;

            mn_layout_ptr->curr_ln =    1;
            mn_layout_ptr->top_ln =     1;
            mn_layout_ptr->sub_ln =     0;
            break;

      case (KEYPRESS_MENU_CMD):

            input = msg.data[0];
            switch (input)
            {
              case (ENTER_KEY):
					ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
                     (*menufunc_ptrs[active_menu]) (mn_msg);
                     break;
               case (DOWN_ARROW_KEY):
                     break;
               case (UP_ARROW_KEY):
                     break;
               case (MINUS_KEY): adjustLcdContrast(DARKER); break;
               case (PLUS_KEY): adjustLcdContrast(LIGHTER); break;
               case (ESC_KEY):
					ACTIVATE_USER_MENU_MSG(&configMenu, BATTERY);
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
*	Function:	batteryMnDsply
*	Purpose:
****************************************/
void batteryMnDsply(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
    uint8 buff[25];
	char spaceBuff[25];
    uint8 batt_buff[11];
    uint32 x;
    float curr_batt_volts;
    float batt_rng;
    uint8 length;

    byteSet(&(mmap[0][0]), 0, sizeof(mmap));

	// Add in a title for the menu
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "-%s-", getLangText(BATTERY_VOLTAGE_TEXT));
	length = (uint8)strlen((char*)buff);

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

    wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
    wnd_layout_ptr->next_row = wnd_layout_ptr->start_row;
    wnd_layout_ptr->next_col = wnd_layout_ptr->start_col;

    curr_batt_volts = convertedBatteryLevel(BATTERY_VOLTAGE);

	// ********** Print Battery text **********
	byteSet(&buff[0], 0, sizeof(buff));
    sprintf((char*)buff,"%.2f %s", curr_batt_volts, getLangText(VOLTS_TEXT));
	debug("Battery: %s\n", buff);

    wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
    wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// ********** Print the Low and Full text **********
	byteSet(&buff[0], 0, sizeof(buff));
	byteSet(&spaceBuff[0], 0, sizeof(spaceBuff));
	byteSet(&spaceBuff[0], ' ', sizeof(spaceBuff) - 1);

	length = (uint8)(strlen(getLangText(LOW_TEXT)) + strlen(getLangText(FULL_TEXT)));
	spaceBuff[(20 - length)] = '\0';

	sprintf((char*)buff,"%s%s%s", getLangText(LOW_TEXT), spaceBuff, getLangText(FULL_TEXT));

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
    wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// ********** E========F **********
    byteSet(&buff[0], 0, sizeof(buff));
    byteSet(&batt_buff[0], ' ', (sizeof(buff) - 1));

    batt_rng = (float).25;
    x = 0;
    if (curr_batt_volts > BATT_MIN_VOLTS)
    {
		curr_batt_volts = (float)(curr_batt_volts - BATT_MIN_VOLTS);
    }
    else
    {
		curr_batt_volts = 0;
    }

	// Creating a string to give the appearance of a battery metter. Using '=' as the bar
    for (x = 0; x < 10; x++)
	{
    	if ((curr_batt_volts > batt_rng) && (x < 9))
    	{
    		batt_buff[x] = '=';
    		curr_batt_volts -= batt_rng;
		}
		else
		{
			batt_buff[x] = '|';
			break;
		}
	}
    batt_buff[10] = 0; // Assign to null

	length = (uint8)sprintf((char*)buff,"[%s]", batt_buff);
	wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

    wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
    wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// ********** Print other battery voltages **********
	byteSet(&buff[0], 0, sizeof(buff));

	curr_batt_volts = convertedBatteryLevel(EXT_CHARGE_VOLTAGE);

	// Check if the external charge voltage is above 0.5 volts indicating that it's active
	if (curr_batt_volts < 0.5)
	{
		curr_batt_volts = 0;
	}

    length = (uint8)sprintf((char*)buff,"(%.2f %s)", curr_batt_volts, getLangText(VOLTS_TEXT));

	wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
    wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
    wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
}
