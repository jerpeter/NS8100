///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: LcdContrastMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:50 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/LcdContrastMenu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Menu.h"
#include "Display.h"
#include "Display.h"
#include "Uart.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define LCD_CONTRAST_MN_TABLE_SIZE DEFAULT_MN_SIZE
#define LCD_CONTRAST_WND_STARTING_COL 6
#define LCD_CONTRAST_WND_END_COL 127
#define LCD_CONTRAST_WND_STARTING_ROW 8
#define LCD_CONTRAST_WND_END_ROW 55
#define LCD_CONTRAST_MN_TBL_START_LINE 0

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern REC_HELP_MN_STRUCT help_rec;
extern uint8 contrast_value;
extern USER_MENU_STRUCT configMenu[];
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
TEMP_MENU_DATA_STRUCT lcd_contrast_mn_table_test [LCD_CONTRAST_MN_TABLE_SIZE] = {
{TITLE_PRE_TAG, LCD_CONTRAST_TEXT, TITLE_POST_TAG},
{ITEM_1, LIGHTER_TEXT, NO_TAG},
{ITEM_2, DEFAULT_TEXT, NO_TAG},
{ITEM_3, DARKER_TEXT, NO_TAG},
{ITEM_4, SAVE_CHANGES_TEXT, NO_TAG},
{NO_TAG, TOTAL_TEXT_STRINGS, NO_TAG}
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void lcdContrastMn (INPUT_MSG_STRUCT);
void lcdContrastMnProc(INPUT_MSG_STRUCT,
                       WND_LAYOUT_STRUCT *,
                       MN_LAYOUT_STRUCT *);
void addLcdContrastLevelDisplay(WND_LAYOUT_STRUCT *wnd_layout_ptr);

/****************************************
*	Function:	lcdContrastMn
*	Purpose:
****************************************/
void lcdContrastMn(INPUT_MSG_STRUCT msg)
{
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;

    lcdContrastMnProc(msg, &wnd_layout, &mn_layout);

    if (active_menu == LCD_CONTRAST_MENU)
    {
        dsplySelMn(&wnd_layout, &mn_layout, TITLE_CENTERED);
        addLcdContrastLevelDisplay(&wnd_layout);
        writeMapToLcd(mmap);
    }
}

/****************************************
*	Function:	lcdContrastMnProc
*	Purpose:
****************************************/
void lcdContrastMnProc(INPUT_MSG_STRUCT msg,
                       WND_LAYOUT_STRUCT *wnd_layout_ptr,
                       MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	uint32 input;

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col = LCD_CONTRAST_WND_STARTING_COL;   /* 6 */
			wnd_layout_ptr->end_col =   LCD_CONTRAST_WND_END_COL;        /* 127 leaving one pixel space at the end*/
			wnd_layout_ptr->start_row = LCD_CONTRAST_WND_STARTING_ROW;   /*/ 8*/
			wnd_layout_ptr->end_row =   LCD_CONTRAST_WND_END_ROW;        /*  6 */

			mn_layout_ptr->curr_ln = 2;
			mn_layout_ptr->top_ln = 1;
			mn_layout_ptr->sub_ln = 0;

			loadTempMenuTable(lcd_contrast_mn_table_test);
			break;

		case (KEYPRESS_MENU_CMD):
			input = msg.data[0];
			// Handle converting a number key input into a selection
			if ((input >= ONE_KEY) && (input <= FOUR_KEY))
			{
				// Convert ASCII data to a hex number to set the current line
				mn_layout_ptr->curr_ln = (uint16)(input - 0x30);
				input = ENTER_KEY;
			}

			switch (input)
			{
				case (ENTER_KEY):
					switch (mn_layout_ptr->curr_ln)
					{
						case (1): // Lighter
							if ((contrast_value - CONTRAST_STEPPING) >= MIN_CONTRAST)
							{
								help_rec.lcd_contrast = contrast_value -= CONTRAST_STEPPING;
							}

							setLcdContrast(contrast_value);
							break;

						case (2): // Default
							help_rec.lcd_contrast = contrast_value = DEFUALT_CONTRAST;

							setLcdContrast(contrast_value);
							break;

						case (3): // Darker
							if ((contrast_value + CONTRAST_STEPPING) <= MAX_CONTRAST)
							{
								help_rec.lcd_contrast  = contrast_value += CONTRAST_STEPPING;
							}

							setLcdContrast(contrast_value);
							break;

						case (4): // Save changes
							saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

							ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
							(*menufunc_ptrs[active_menu]) (mn_msg);
							break;

						default:
							break;
					}
				break;

			case (DOWN_ARROW_KEY): mnScroll(DOWN,SELECT_MN_WND_LNS, mn_layout_ptr); break;
			case (UP_ARROW_KEY): mnScroll(UP,SELECT_MN_WND_LNS,mn_layout_ptr); break;
			case (MINUS_KEY): adjustLcdContrast(DARKER); break;
			case (PLUS_KEY): adjustLcdContrast(LIGHTER); break;

			case (ESC_KEY):
				ACTIVATE_USER_MENU_MSG(&configMenu, LCD_CONTRAST);
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
*	Function:	addLcdContrastLevelDisplay
*	Purpose:	Provide a level indicator for the LCD Contrast
****************************************/
void addLcdContrastLevelDisplay(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
    uint8 buff[25];
    uint8 spaceBuff[25];
	uint8 contrast_buff[11];
    uint8 x;
    uint8 clvl;
    uint8 length;

	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;

	// *** Print the Light and Dark text ***
	byteSet(&buff[0], 0, sizeof(buff));
	byteSet(&spaceBuff[0], 0, sizeof(spaceBuff));
	byteSet(&spaceBuff[0], ' ', sizeof(spaceBuff) - 1);

	length = (uint8)(strlen(getLangText(LIGHT_TEXT)) + strlen(getLangText(DARK_TEXT)));
	spaceBuff[(20 - length)] = '\0';

	sprintf((char*)buff,"%s%s%s", getLangText(LIGHT_TEXT), spaceBuff, getLangText(DARK_TEXT));

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
    wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// *** Print the Contrast Bar
    byteSet(&buff[0], 0, sizeof(buff));
    byteSet(&contrast_buff[0], 0, sizeof(contrast_buff));
    byteSet(&contrast_buff[0], ' ', (sizeof(contrast_buff) - 1));

    clvl = contrast_value;

	// Creating a string to give the appearance of a contrast level. Using '=' as the bar
    for (x = 0; x < 10; x++)
	{
    	if ((clvl > MIN_CONTRAST) && (x < 9))
    	{
    		contrast_buff[x] = '=';
    		clvl -= CONTRAST_STEPPING;
		}
		else
		{
			contrast_buff[x] = '|';
			break;
		}
	}
    contrast_buff[10] = '\0';

    debug("Contrast level: <%s>\n", buff);
    sprintf((char*)buff,"[%s]", contrast_buff);

	length = (uint8)strlen((char*)buff);
	wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
    wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
}
