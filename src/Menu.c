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
#include "Common.h"
#include "Menu.h"
#include "PowerManagement.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "SysEvents.h"
#include "Summary.h"
#include "EventProcessing.h"
#include "Display.h"
#include "Keypad.h"
#include "Font_Six_by_Eight_table.h"
#include "TextTypes.h"
#include "RemoteCommon.h"
#include "usart.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define END_DATA_STRING_SIZE 6
#define MIN_CHAR_STRING 0x41
#define MIN_NUM_STRING 0x2E
#define MAX_CHAR_STRING 0x5a
#define MAX_NUM_STRING 0x39
#define INPUT_MENU_SECOND_LINE_INDENT 6

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT modeMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static MB_CHOICE s_MessageChoices[MB_TOTAL_CHOICES] =
{
	//{Num Choices,		1st/Single,	2nd Choice,	}
	//{MB_ONE_CHOICE,	"OK\0",		"\0"		},
	//{MB_TWO_CHOICES,	"YES\0",	"NO\0"		},
	//{MB_TWO_CHOICES,	"OK\0",		"CANCEL\0"	}
	{MB_ONE_CHOICE,		OK_TEXT,	NULL_TEXT	},
	{MB_TWO_CHOICES,	YES_TEXT,	NO_TEXT		},
	{MB_TWO_CHOICES,	OK_TEXT,	CANCEL_TEXT	}
	// Add new s_MessageChoices entry for new choices aboove this line
};

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void LoadTempMenuTable(TEMP_MENU_DATA_STRUCT* currentMenu)
{
	uint16 i = 0;

	while (currentMenu[i].textEntry != TOTAL_TEXT_STRINGS)
	{
		sprintf((char*)g_menuPtr[i].data, "%s%s%s", g_menuTags[currentMenu[i].preTag].text,
				getLangText(currentMenu[i].textEntry), g_menuTags[currentMenu[i].postTag].text);
		i++;
	}

	strcpy((char*)g_menuPtr[i].data, ".end.");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MenuScroll(char direction, char wnd_size, MN_LAYOUT_STRUCT* mn_layout_ptr)
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
			if (mn_layout_ptr->curr_ln > 1)
			{
				if (mn_layout_ptr->curr_ln == mn_layout_ptr->top_ln)
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
void UserMenuScroll(uint32 direction, char wnd_size, MN_LAYOUT_STRUCT* mn_layout_ptr)
{
	char buff[50];

	strcpy(buff, (g_userMenuCachePtr + mn_layout_ptr->curr_ln + 1)->text);

	switch (direction)
	{
		case (DOWN_ARROW_KEY):
			if (strcmp(buff, ".end."))
			{
				mn_layout_ptr->curr_ln++;

				if ((mn_layout_ptr->curr_ln - mn_layout_ptr->top_ln) >= wnd_size)
				{
					mn_layout_ptr->top_ln++;
				}
			}
		break;

		case (UP_ARROW_KEY):
			if (mn_layout_ptr->curr_ln > 1)
			{
				if (mn_layout_ptr->curr_ln == mn_layout_ptr->top_ln)
				{
					if (mn_layout_ptr->top_ln > 1)
					{
						mn_layout_ptr->top_ln--;
					}
				}

				mn_layout_ptr->curr_ln--;
			}
		break;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplaySelectMenu(WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr, uint8 titlePosition)
{
   uint8 buff[50];
   uint8 top;
   uint8 menu_ln;
   uint32 length;

   memset(&(g_mmap[0][0]), 0, sizeof(g_mmap));

   menu_ln = 0;
   top = 0;
   strcpy((char*)buff, (char*)(g_menuPtr + top + menu_ln)->data);
   length = strlen((char*)buff);
   wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;

	if (titlePosition == TITLE_LEFT_JUSTIFIED)
	{
		wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
   	}
   	else // titlePosition is TITLE_CENTERED
   	{
   		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
   	}

   WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

   menu_ln = 0;
   top = (uint8)mn_layout_ptr->top_ln;
   wnd_layout_ptr->curr_row = wnd_layout_ptr->start_row;
   wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
   wnd_layout_ptr->next_row = wnd_layout_ptr->start_row;
   wnd_layout_ptr->next_col = wnd_layout_ptr->start_col;

   while (wnd_layout_ptr->curr_row <= wnd_layout_ptr->end_row)
   {
      strcpy((char*)buff, (char*)(g_menuPtr + top + menu_ln)->data);
      if (strcmp((char*)buff, ".end."))
      {
         if (menu_ln == (mn_layout_ptr->curr_ln - mn_layout_ptr->top_ln))
         {
			if (mn_layout_ptr->sub_ln == 0)
			{
	            WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_LN);
			}
			else
			{
				wnd_layout_ptr->index = mn_layout_ptr->sub_ln;
	            WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_CHAR);
			}
         }
         else
         {
            WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
         }
      }
      else
         break;

      wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
      menu_ln++;
   }/* END OF WHILE LOOP */
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayUserMenu(WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr, uint8 titlePosition)
{
	uint8 buff[50]; /* made it bigger then NUM_CHAR_PER_LN just in case someone trys to make a big string.*/
	uint16 top;
	uint8 menu_ln;
	uint32 length;

	// Clear out LCD map buffer
	memset(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	// Init var's
	menu_ln = 0;
	top = 0;

	// Copy Menu Title into buffer
	strcpy((char*)(&buff[0]), (g_userMenuCachePtr + top + menu_ln)->text);

	// Get length of title
	length = strlen((char*)(&buff[0]));

	// Set current row to the top row
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;

	// Check if title position is specified as left justified
	if (titlePosition == TITLE_LEFT_JUSTIFIED)
	{
		wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	}
	else // titlePosition is TITLE_CENTERED
	{
		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	}

	// Write string to LCD map
	WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	// Reset var's... purpose?
	menu_ln = 0;
	top = mn_layout_ptr->top_ln;

	// Reset menu display parameters
	wnd_layout_ptr->curr_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col =   wnd_layout_ptr->start_col;

	// Handle the rest of the menu table
	while (wnd_layout_ptr->curr_row <= wnd_layout_ptr->end_row)
	{
		// Copy the next menu text into the buffer
		strcpy((char*)(&buff[0]), (g_userMenuCachePtr + top + menu_ln)->text);

		// Check if we have reached the end of the menu text
		if (strcmp((char*)(&buff[0]), ".end."))
		{
			// If the incrementing menu line matches the current line minus the top line
			if (menu_ln == (mn_layout_ptr->curr_ln - mn_layout_ptr->top_ln))
			{
				if (mn_layout_ptr->sub_ln == 0)
				{
					// Write the text to the LCD map highlighted
		            WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_LN);
				}
				else
				{
					// Write just one char highlighted
					wnd_layout_ptr->index = mn_layout_ptr->sub_ln;
		            WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_CHAR);
				}
			}
			else // Write text as a regular line
			{
				WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			}
		}
		else // Reached end of menu text
			break;

		// Set current row to already advanced next row
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		// Advance menu line
		menu_ln++;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WndMpWrtString(uint8* buff, WND_LAYOUT_STRUCT* wnd_layout, int font_type, int ln_type)
{
   const uint8 (*fmap_ptr)[FONT_MAX_COL_SIZE];
   uint8 mmcurr_row;
   uint8 mmend_row;
   uint8 mmcurr_col;
   uint8 mmend_col;
   uint8 cbit_size;
   uint8 crow_size;
   uint8 ccol_size;
   uint8 temp1 = 0;
   int32 index;
   int32 row;
   int32 col;
   int32 bits_wrtn;
   int32 first_column;

   first_column = 0;

   switch (font_type)
   {

      case  SIX_BY_EIGHT_FONT:
            cbit_size = EIGHT_ROW_SIZE;
            crow_size = (uint8)(cbit_size / 8);
            if (EIGHT_ROW_SIZE % 8)
               crow_size++;
            ccol_size = SIX_COL_SIZE;
            fmap_ptr = font_table_68;

            wnd_layout->next_row = (uint16)(wnd_layout->curr_row + cbit_size);
            wnd_layout->next_col = (uint16)(wnd_layout->curr_col + ccol_size);
            break;

      default:
            break;
   }

   mmcurr_row  = (uint8)(wnd_layout->curr_row /8);
   if (wnd_layout->curr_row %8)
      mmcurr_row++;

   mmend_row  = (uint8)(wnd_layout->end_row /8);
   if (wnd_layout->end_row %8)
      mmend_row++;

   mmcurr_col = (uint8)(wnd_layout->curr_col);
   mmend_col = (uint8)wnd_layout->end_col;

   index = 0;

	// While not at the end of the string
	while (buff[index] != '\0')
	{
		// Loop through the pixel width (columns) of the font
		for (row = 0, col = 0; col < ccol_size; col++)
		{
			// Check that the column and row are within range
			if (((col + mmcurr_col) <= mmend_col) && ((row + mmcurr_row) <= mmend_row))
			{
				if ((ln_type == REG_LN) || (ln_type == CURSOR_CHAR))
				{
					// Get the font table bitmap for each column
					g_mmap[mmcurr_row + row][mmcurr_col + col] |= fmap_ptr[buff[index]][col] << (wnd_layout->curr_row %8);
				}
				else if (ln_type == CURSOR_LN)
				{
					// Get the inverse of the font table bitmap for the column
					temp1 = (uint8)~(fmap_ptr[buff[index]][col]);

					// Write the inverse bitmap into the buffer
					g_mmap[mmcurr_row + row][mmcurr_col + col] |= temp1 << (wnd_layout->curr_row %8);
					// Write the last pixel highlighted (reversed) in each column for the previous row
					g_mmap[mmcurr_row + row -1][mmcurr_col + col] |= 0x80;

					// Check if this is the first char
					if (first_column == 0)
					{
						// pre-highlight (reverse) the column just before the first char
						g_mmap[mmcurr_row + row][mmcurr_col + col -1] |= 0xFF;
						// pre-highlight (reverse) the last pixel in the column for the previous row
						g_mmap[mmcurr_row + row -1][mmcurr_col + col -1] |= 0x80;

						// Increment to prevent accessing this code again
						first_column++;
					}
				}
			}
		}

		// See if character externds past current row
		bits_wrtn = (8 - (wnd_layout->curr_row %8));
		if (bits_wrtn >= cbit_size)
		{
			row++;
		}

		// Finish writing the rest of the character
		for (; row < crow_size; row++)
		{
			for (col = 0; col < ccol_size; col++)
			{
				if ((ln_type == REG_LN) || (ln_type == CURSOR_CHAR))
				{
					if (((col + mmcurr_col) <= mmend_col) && ((row + mmcurr_row) <= mmend_row))
					{
						temp1  = (uint8)((fmap_ptr[buff[index]][col] >> (8 - (wnd_layout->curr_row %8))));
						g_mmap[mmcurr_row + row + 1][mmcurr_col + col] = temp1;
					}
				}
				else if (ln_type == CURSOR_LN)
				{
					if (((col + mmcurr_col) <= mmend_col) && ((row + mmcurr_row) <= mmend_row))
					{
						g_mmap[mmcurr_row + row + 1][mmcurr_col + col] = (uint8)(temp1 & (0xff >> (cbit_size - (cbit_size - bits_wrtn))));

						temp1  = (uint8)((fmap_ptr[buff[index]][col] >> (8 - (wnd_layout->curr_row %8))));
						g_mmap[mmcurr_row + row + 1][mmcurr_col + col] = temp1;
					}
				}
			}
		}

		// Increment to the next column
		mmcurr_col += ccol_size;
		// Increment the character index
		index++;
	}

	if (ln_type == CURSOR_CHAR)
	{
		mmcurr_col = (uint8)((wnd_layout->curr_col) + ((wnd_layout->index - 1) * ccol_size));

		if ((mmcurr_col > 0) && (mmcurr_row > 0))
		{
			// Set the column before the char reversed
			g_mmap[mmcurr_row][mmcurr_col - 1] |= 0xff;
			// Set the previous row last pixel before the char reversed
			g_mmap[mmcurr_row - 1][mmcurr_col - 1] |= 0x80;
		}

		// Loop through the pixel width (columns) of the cursor char
		for (row = 0, col = 0; col < ccol_size; col++)
		{
			// Check that the column and row are within range
			if (((col + mmcurr_col) <= mmend_col) && ((row + mmcurr_row) <= mmend_row))
			{
				temp1 = (uint8)~g_mmap[mmcurr_row + row][mmcurr_col + col];

				// Get the font table bitmap for each column
				g_mmap[mmcurr_row + row][mmcurr_col + col] = temp1;

				if ((mmcurr_row + row - 1) >= 0)
				{
					// Write the last pixel highlighted (reversed) in each column for the previous row
					g_mmap[mmcurr_row + row - 1][mmcurr_col + col] |= 0x80;
				}
			}
		}
	}

	// Store next column location
	wnd_layout->next_col = mmcurr_col;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MessageBorder(void)
{
	uint8 i = 0;

	// Top and bottom horizontal bars
	for (i = 12; i < 116; i++)
	{
		g_mmap[0][i] |= 0xe0;
		g_mmap[7][i] |= 0x07;
	}

	// Left and right vertical bars
	for (i = 1; i < 7; i++)
	{
		g_mmap[i][9] = 0xff;
		g_mmap[i][10] = 0xff;
		g_mmap[i][11] = 0xff;
		g_mmap[i][116] = 0xff;
		g_mmap[i][117] = 0xff;
		g_mmap[i][118] = 0xff;
	}

	// Rounded ends
	g_mmap[0][9] |= 0x80;		g_mmap[0][10] |= 0xc0;	g_mmap[0][11] |= 0xe0;
	g_mmap[0][116] |= 0xe0;	g_mmap[0][117] |= 0xc0;	g_mmap[0][118] |= 0x80;
	g_mmap[7][9] |= 0x01;		g_mmap[7][10] |= 0x03;	g_mmap[7][11] |= 0x07;
	g_mmap[7][116] |= 0x07;	g_mmap[7][117] |= 0x03;	g_mmap[7][118] |= 0x01;

	// Clear inside message box area minus title area (highlighted)
	for (i = 12; i < 116; i++)
	{
		g_mmap[2][i] = 0x00;
		g_mmap[3][i] = 0x00;
		g_mmap[4][i] = 0x00;
		g_mmap[5][i] = 0x00;
		g_mmap[6][i] = 0x00;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MessageTitle(char* titleString)
{
	uint8 i = 0, j = 0;
	uint8 length = 0;
	uint8 textPosition = 0;

	// Find starting pixel length of title
	length = (uint8)(strlen((char*)titleString) * 3); // result in pixel width, 3 = 6 (font width) / 2
	if (length > 52)
		textPosition = 12;
	else
		textPosition = (uint8)(64 - length);

	// Reverse fill up to the beginning of the title
	for (i = 12; i < textPosition; i++)
		g_mmap[1][i] = 0xff;

	// Write the title (highlighted)
	i = 0;
	while ((titleString[i] != '\0') && (textPosition < 116))
	{
		g_mmap[1][textPosition++] = (uint8)~font_table_68[(uint8)(titleString[i])][j++];
		if (j == 6) {i++; j = 0;}
	}

	// Reverse fill from title to end of row
	for (i = textPosition; i < 116; i++)
		g_mmap[1][i] = 0xff;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MessageText(char* textString)
{
	uint8 i = 0, j = 0;
	uint8 textPosition = 0;
	uint8 length = 0, subLength = 0;

	// ======================
	// Setup 1st line Indexes
	// ======================
	// Find out if message text goes beyond 1 line (17 chars)
	length = subLength = (uint8)strlen((char*)textString);
	if (length > 17) // max 17 chars per line
	{
		subLength = 17;

		// Look for a space
		while (textString[subLength-1] != ' ')
		{
			if (--subLength == 0)
				break;
		}

		// If no space was found, use the max length
		if (subLength == 0)
			subLength = 17;
	}

	// =========================
	// Write 1st line of Message
	// =========================
	textPosition = 13; // Shifted by one pixel since 2 extra pixel rows

	// Write in the number of characters that will fit on the 1st line
	for (i = 0, j = 0; i < subLength;)
	{
		g_mmap[2][textPosition++] = font_table_68[(uint8)(textString[i])][j++];
		// When 6 pixel columns have been written, advance to the next character
		if (j == 6) {i++; j = 0;}
	}

	// ======================
	// Setup 2nd line Indexes
	// ======================
	// Find out if total length minus the current index goes beyond the 2nd line
	if ((length-i) > 17)
	{
		// New sub length is a max of the current index plus 17
		subLength = (uint8)(i + 17);

		// Look for a space
		while ((textString[subLength-1] != ' ') && (subLength > i))
		{
			// If the subLength equals the current index, leave
			if (--subLength == i)
				break;
		}

		// If no space was found, use the index plus 17
		if (subLength == i)
			subLength = (uint8)(i + 17);
	}
	else subLength = length;

	// =========================
	// Write 2nd line of Message
	// =========================
	textPosition = 13; // Shifted by one pixel since 2 extra pixel rows

	// Write in the number of characters that will fit on the 2nd line
	for (j = 0; i < subLength;)
	{
		g_mmap[3][textPosition++] = font_table_68[(uint8)(textString[i])][j++];
		// When 6 pixel columns have been written, advance to the next character
		if (j == 6) {i++; j = 0;}
	}

	// ======================
	// Setup 3rd line Indexes
	// ======================
	// Find out if total length minus the current index goes beyond the 2nd line
	if ((length-i) > 17)
	{
		// New sub length is a max of the current index plus 17
		subLength = (uint8)(i + 17);

		// Look for a space
		while ((textString[subLength-1] != ' ') && (subLength > i))
		{
			// If the subLength equals the current index, leave
			if (--subLength == i)
				break;
		}

		// If no space was found, use the index plus 17
		if (subLength == i)
			subLength = (uint8)(i + 17);
	}
	else subLength = length;

	// =========================
	// Write 3nd line of Message
	// =========================
	textPosition = 13; // Shifted by one pixel since 2 extra pixel rows

	// Write in the number of characters that will fit on the 3rd line
	for (j = 0; i < subLength;)
	{
		g_mmap[4][textPosition++] = font_table_68[(uint8)(textString[i])][j++];
		// When 6 pixel columns have been written, advance to the next character
		if (j == 6) {i++; j = 0;}
	}

	// =============================================================
	// Setup 4th line Indexes (only valid for message with 1 choice)
	// =============================================================
	// Find out if total length minus the current index goes beyond the 3rd line
	if ((length-i) > 17)
		subLength = (uint8)(i + 17);
	else subLength = length;

	// ================================================================
	// Write 4th line of Message (only valid for message with 1 choice)
	// ================================================================
	textPosition = 13; // Shifted by one pixel since 2 extra pixel rows
	for (j = 0; i < subLength;)
	{
		g_mmap[5][textPosition++] = font_table_68[(uint8)(textString[i])][j++];
		// When 6 pixel columns have been written, advance to the next character
		if (j == 6) {i++; j = 0;}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MessageChoice(MB_CHOICE_TYPE choiceType)
{
	uint8 i = 0, j = 0;
	uint8 text1Position = 0, text2Position = 0, startPosition = 0;
	uint8 startRow = 0;
	char firstChoiceText[18];
	char secondChoiceText[18];

	strcpy((char*)firstChoiceText, getLangText(s_MessageChoices[choiceType].firstTextEntry));
	strcpy((char*)secondChoiceText, getLangText(s_MessageChoices[choiceType].secondTextEntry));

	// 64 = half screen, char len * 3 = char width*6(pixel width)/2(half)
	text1Position = (uint8)(64 - strlen((char*)firstChoiceText) * 3);
	text2Position = (uint8)(64 - strlen((char*)secondChoiceText) * 3);

	// Find starting pixel position with extra char space, 6 = extra char space in pixel width
	startPosition = (uint8)((text1Position < text2Position ? text1Position : text2Position) - 6);

	if (s_MessageChoices[choiceType].numChoices == MB_ONE_CHOICE)
		startRow = 6;
	else // s_MessageChoices[choiceType].numChoices == MB_TWO_CHOICES
		startRow = 5;

	// Clear out unused portion of first choice row in case message text ran long
	for (i = 12; i < startPosition; i++)
	{
		g_mmap[startRow][i] = 0x00;
		g_mmap[startRow][127-i] = 0x00;
	}

	// Highlight extra char space before and after text
	for (i = startPosition; i < text1Position; i++)
	{
		g_mmap[startRow-1][i] |= 0x80;
		g_mmap[startRow][i] = 0xff;
		g_mmap[startRow-1][127-i] |= 0x80;
		g_mmap[startRow][127-i] = 0xff;
	}

	// Display first choice
	i = 0;
	while ((firstChoiceText[i] != '\0') && (text1Position < 116))
	{
		// Steal pixel line for active (reversed) choice
		g_mmap[startRow-1][text1Position] |= 0x80;
		// Write in text (highlighted)
		g_mmap[startRow][text1Position++] = (uint8)~font_table_68[(uint8)(firstChoiceText[i])][j++];
		// When 6 pixel columns have been written, advance to the next character
		if (j == 6) {i++; j = 0;}
	}

	// Add active choice round ends
	g_mmap[startRow][startPosition - 2] = 0x3e;
	g_mmap[startRow][startPosition - 1] = 0x7f;
	g_mmap[startRow][128 - startPosition] = 0x7f;
	g_mmap[startRow][128 - startPosition + 1] = 0x3e;

	if (s_MessageChoices[choiceType].numChoices == MB_TWO_CHOICES)
	{
		// Display second choice
		i = 0;
		while ((secondChoiceText[i] != '\0') && (text2Position < 116))
		{
			// Write in the text (plain)
			g_mmap[6][text2Position++] = font_table_68[(uint8)(secondChoiceText[i])][j++];
			// When 6 pixel columns have been written, advance to the next character
			if (j == 6) {i++; j = 0;}
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MessageChoiceActiveSwap(MB_CHOICE_TYPE choiceType)
{
	uint8 i = 0;
	uint8 text1Position = 0, text2Position = 0, startPosition = 0;
	char firstChoiceText[18];
	char secondChoiceText[18];

	strcpy((char*)firstChoiceText, getLangText(s_MessageChoices[choiceType].firstTextEntry));
	strcpy((char*)secondChoiceText, getLangText(s_MessageChoices[choiceType].secondTextEntry));

	// 64 = half screen, char len * 3 = char width*6(pixel width)/2(half)
	text1Position = (uint8)(64 - strlen((char*)firstChoiceText) * 3);
	text2Position = (uint8)(64 - strlen((char*)secondChoiceText) * 3);

	// Find starting pixel position with extra char space, 6 = extra char space in pixel width
	startPosition = (uint8)((text1Position < text2Position ? text1Position : text2Position) - 6);

	for (i = startPosition; i < (128-startPosition); i++)
	{
		// Toggle bottom pixel line of 4th row
		g_mmap[4][i] ^= 0x80;
		// Inverse the row/text leaving the bottom pixel row active (highlighted)
		g_mmap[5][i] = (uint8)(~g_mmap[5][i] | 0x80);
		// Inverse the row/text
		g_mmap[6][i] = (uint8)~g_mmap[6][i];
	}

	// Xor the round ends of the choices to invert them
	g_mmap[5][startPosition-2] ^= 0x3e;
	g_mmap[5][startPosition-1] ^= 0x7f;
	g_mmap[5][128-startPosition] ^= 0x7f;
	g_mmap[5][128-startPosition+1] ^= 0x3e;
	g_mmap[6][startPosition-2] ^= 0x3e;
	g_mmap[6][startPosition-1] ^= 0x7f;
	g_mmap[6][128-startPosition] ^= 0x7f;
	g_mmap[6][128-startPosition+1] ^= 0x3e;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 MessageBox(char* titleString, char* textString, MB_CHOICE_TYPE choiceType)
{
	uint8 activeChoice = MB_FIRST_CHOICE;
	volatile uint8 key = 0;

	g_debugBufferCount = 1;
	strcpy((char*)g_debugBuffer, textString);

	// Build MessageBox into g_mmap with the following calls
	MessageBorder();
	MessageTitle(titleString);
	MessageText(textString);
	MessageChoice(choiceType);

	WriteMapToLcd(g_mmap);

	debug("MB: Look for a key\r\n");

	// Loop forever unitl an enter or escape key is found
	while ((key != ENTER_KEY) && (key != ESC_KEY) && (key != ON_ESC_KEY))
	{
		// Blocking call to wait for a key to be pressed on the keypad
		key = GetKeypadKey(WAIT_FOR_KEY);

		// Check if there are two choices
		if (s_MessageChoices[choiceType].numChoices == MB_TWO_CHOICES)
		{
			switch (key)
			{
				case UP_ARROW_KEY:
					// Check if the active choice is the second/bottom choice
					if (activeChoice == MB_SECOND_CHOICE)
					{
						// Swap the active choice
						MessageChoiceActiveSwap(choiceType);
						WriteMapToLcd(g_mmap);

						activeChoice = MB_FIRST_CHOICE;
					}
					break;
				case DOWN_ARROW_KEY:
					// Check if the active choice is the first/top choice
					if (activeChoice == MB_FIRST_CHOICE)
					{
						// Swap the active choice
						MessageChoiceActiveSwap(choiceType);
						WriteMapToLcd(g_mmap);

						activeChoice = MB_SECOND_CHOICE;
					}
					break;
			}
		}
	}

	// Clear LCD map buffer to remove message from showing up
	memset(&(g_mmap[0][0]), 0, sizeof(g_mmap));
	WriteMapToLcd(g_mmap);

	g_debugBufferCount = 0;

	if (key == ENTER_KEY) { return (activeChoice); }
	if (key == ON_ESC_KEY) { return (MB_SPECIAL_ACTION); }
	else // key == ESC_KEY (escape)
	{
		return (MB_NO_ACTION);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void OverlayMessage(char* titleString, char* textString, uint32 displayTime)
{
	MessageBorder();
	MessageTitle(titleString);
	MessageText(textString);

	WriteMapToLcd(g_mmap);
	SoftUsecWait(displayTime);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateModeMenuTitle(uint8 mode)
{
	switch (mode)
	{
		case WAVEFORM_MODE:
			modeMenu[0].textEntry = WAVEFORM_MODE_TEXT;
		break;

		case BARGRAPH_MODE:
			modeMenu[0].textEntry = BARGRAPH_MODE_TEXT;
		break;

		case COMBO_MODE:
			modeMenu[0].textEntry = COMBO_MODE_TEXT;
		break;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 testg_mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

void DisplaySplashScreen(void)
{
	WND_LAYOUT_STRUCT wnd_layout;
	uint8 buff[50];
	uint8 length;

	wnd_layout.end_row = DEFAULT_END_ROW;
	wnd_layout.end_col = DEFAULT_END_COL;

	// Clear cached LCD memory map
	memset(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	//----------------------------------------------------------------------------------------
	// Add in a title for the menu
	//----------------------------------------------------------------------------------------
	memset(&buff[0], 0, sizeof(buff));

	length = sprintf((char*)(&buff[0]), "%s", "NOMIS 8100 GRAPH");
	wnd_layout.curr_row = DEFAULT_MENU_ROW_ONE;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	WndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

	//----------------------------------------------------------------------------------------
	// Add in Software Version
	//----------------------------------------------------------------------------------------
	memset(&buff[0], 0, sizeof(buff));
	sprintf((char*)(&buff[0]), "%s %s", getLangText(SOFTWARE_VER_TEXT), (char*)g_buildVersion);
	length = (uint8)strlen((char*)(&buff[0]));

	wnd_layout.curr_row = DEFAULT_MENU_ROW_THREE;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	WndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

	//----------------------------------------------------------------------------------------
	// Add in Software Date and Time
	//----------------------------------------------------------------------------------------
	memset(&buff[0], 0, sizeof(buff));
	sprintf((char*)(&buff[0]), "%s", (char*)g_buildDate);
	length = (uint8)strlen((char*)buff);

	wnd_layout.curr_row = DEFAULT_MENU_ROW_FOUR;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	WndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

	//----------------------------------------------------------------------------------------
	// Add in Battery Voltage
	//----------------------------------------------------------------------------------------
	memset(&buff[0], 0, sizeof(buff));
	sprintf((char*)(&buff[0]), "%s: %.2f", getLangText(BATT_VOLTAGE_TEXT), GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
	length = (uint8)strlen((char*)(&buff[0]));

	wnd_layout.curr_row = DEFAULT_MENU_ROW_SIX;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	WndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

    debug("Init Write Splash Screen to LCD...\r\n");

	// Write the map to the LCD
	WriteMapToLcd(g_mmap);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayCalDate(void)
{
	char dateString[35];
	char mesage[75];
	DATE_TIME_STRUCT tempTime;

	if (!g_factorySetupRecord.invalid)
	{
		memset(&dateString[0], 0, sizeof(dateString));
		memset(&mesage[0], 0, sizeof(mesage));

		ConvertCalDatetoDateTime(&tempTime, &g_currentCalDate);
		ConvertTimeStampToString(dateString, &tempTime, REC_DATE_TYPE);

		sprintf((char*)mesage, "%s: %s", getLangText(CALIBRATION_DATE_TEXT), (char*)dateString);
		MessageBox(getLangText(STATUS_TEXT), (char*)mesage, MB_OK);
	}
	else
	{
		MessageBox(getLangText(STATUS_TEXT), getLangText(CALIBRATION_DATE_NOT_SET_TEXT), MB_OK);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplaySensorType(void)
{
	uint16 sensorType = NULL_TEXT;
	char message[75];

	if (!g_factorySetupRecord.invalid)
	{
		memset(&message[0], 0, sizeof(message));
		switch (g_factorySetupRecord.sensor_type)
		{
			case SENSOR_20_IN	: sensorType = X1_20_IPS_TEXT; break;
			case SENSOR_10_IN	: sensorType = X2_10_IPS_TEXT; break;
			case SENSOR_5_IN	: sensorType = X4_5_IPS_TEXT; break;
			case SENSOR_2_5_IN	: sensorType = X8_2_5_IPS_TEXT; break;
			case SENSOR_ACC		: sensorType = ACC_793L_TEXT; break;
		}

		sprintf((char*)message, "%s: %s", getLangText(SENSOR_GAIN_TYPE_TEXT), getLangText(sensorType));
		MessageBox(getLangText(STATUS_TEXT), (char*)message, MB_OK);
	}
	else
	{
		MessageBox(getLangText(STATUS_TEXT), getLangText(SENSOR_GAIN_TYPE_NOT_SET_TEXT), MB_OK);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplaySerialNumber(void)
{
	char message[75];

	if (!g_factorySetupRecord.invalid)
	{
		memset(&message[0], 0, sizeof(message));
		sprintf((char*)message, "%s: %s", getLangText(SERIAL_NUMBER_TEXT), (char*)g_factorySetupRecord.serial_num);
		MessageBox(getLangText(STATUS_TEXT), (char*)message, MB_OK);
	}
	else
	{
		MessageBox(getLangText(STATUS_TEXT), getLangText(SERIAL_NUMBER_NOT_SET_TEXT), MB_OK);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayTimerModeSettings(void)
{
	char message[75];
	char activeTime[15];
	char activeDates[25];
	uint16 activeModeTextType;

	memset(&message[0], 0, sizeof(message));
	memset(&activeTime[0], 0, sizeof(activeTime));
	memset(&activeDates[0], 0, sizeof(activeDates));

	sprintf((char*)activeTime, "%02d:%02d -> %02d:%02d", g_unitConfig.timerStartTime.hour, g_unitConfig.timerStartTime.min,
			g_unitConfig.timerStopTime.hour, g_unitConfig.timerStopTime.min);

	switch (g_unitConfig.timerModeFrequency)
	{
		case TIMER_MODE_ONE_TIME: 	activeModeTextType = ONE_TIME_TEXT; 		break;
		case TIMER_MODE_HOURLY: 	activeModeTextType = HOURLY_TEXT; 			break;
		case TIMER_MODE_DAILY: 		activeModeTextType = DAILY_EVERY_DAY_TEXT; 	break;
		case TIMER_MODE_WEEKDAYS: 	activeModeTextType = DAILY_WEEKDAYS_TEXT; 	break;
		case TIMER_MODE_WEEKLY: 	activeModeTextType = WEEKLY_TEXT; 			break;
		case TIMER_MODE_MONTHLY: 	activeModeTextType = MONTHLY_TEXT; 			break;
		default:					activeModeTextType = ERROR_TEXT;			break;
	}

	sprintf((char*)activeDates, "%02d-%s-%02d -> %02d-%s-%02d", g_unitConfig.timerStartDate.day,
			(char*)&(g_monthTable[(uint8)(g_unitConfig.timerStartDate.month)].name[0]), g_unitConfig.timerStartDate.year,
			g_unitConfig.timerStopDate.day, (char*)&(g_monthTable[(uint8)(g_unitConfig.timerStopDate.month)].name[0]),
			g_unitConfig.timerStopDate.year);

	// Display SAVED SETTINGS, ACTIVE TIME PERIOD HH:MM -> HH:MM
	sprintf((char*)message, "%s, %s: %s", getLangText(SAVED_SETTINGS_TEXT),
			getLangText(ACTIVE_TIME_PERIOD_TEXT), activeTime);
	MessageBox(getLangText(TIMER_MODE_TEXT), (char*)message, MB_OK);

	// Display MODE, HH:MM -> HH:MM
	sprintf((char*)message, "%s, %s", getLangText(activeModeTextType), activeDates);
	MessageBox(getLangText(TIMER_MODE_TEXT), (char*)message, MB_OK);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayFlashUsageStats(void)
{
	char message[75];
	char sizeUsedStr[30];
	char sizeFreeStr[30];

	if (g_sdCardUsageStats.sizeUsed < 1000)
		sprintf(&sizeUsedStr[0], "%s: %3.1fKB %d%%", getLangText(USED_TEXT),((float)g_sdCardUsageStats.sizeUsed / (float)1000), g_sdCardUsageStats.percentUsed);
	else if (g_sdCardUsageStats.sizeUsed < 1000000)
		sprintf(&sizeUsedStr[0], "%s: %3luKB %d%%", getLangText(USED_TEXT),(g_sdCardUsageStats.sizeUsed / 1000), g_sdCardUsageStats.percentUsed);
	else
		sprintf(&sizeUsedStr[0], "%s: %4.0fMB %d%%", getLangText(USED_TEXT),((float)g_sdCardUsageStats.sizeUsed / (float)1000000), g_sdCardUsageStats.percentUsed);

	if (g_sdCardUsageStats.sizeFree < 1000)
		sprintf(&sizeFreeStr[0], "%s: %3.1fKB %d%%", getLangText(FREE_TEXT),((float)g_sdCardUsageStats.sizeFree / (float)1000), g_sdCardUsageStats.percentFree);
	else if (g_sdCardUsageStats.sizeFree < 1000000)
		sprintf(&sizeFreeStr[0], "%s: %3luKB %d%%", getLangText(FREE_TEXT),(g_sdCardUsageStats.sizeFree / 1000), g_sdCardUsageStats.percentFree);
	else
		sprintf(&sizeFreeStr[0], "%s: %4.0fMB %d%%", getLangText(FREE_TEXT),((float)g_sdCardUsageStats.sizeFree / (float)1000000), g_sdCardUsageStats.percentFree);

	sprintf(&message[0], "%s       %s %s %s: %s", getLangText(EVENT_DATA_TEXT),sizeUsedStr, sizeFreeStr, getLangText(WRAPPED_TEXT),(g_sdCardUsageStats.wrapped == YES) ? "YES" : "NO");

	MessageBox(getLangText(FLASH_USAGE_STATS_TEXT), (char*)message, MB_OK);

	if (g_unitConfig.flashWrapping == NO)
	{
		if ((g_sdCardUsageStats.waveEventsLeft < 1000) && (g_sdCardUsageStats.barHoursLeft < 1000)) { sprintf(&message[0], "%s %s: %d, %s: ~%d", getLangText(SPACE_REMAINING_TEXT),getLangText(WAVEFORMS_TEXT), g_sdCardUsageStats.waveEventsLeft, getLangText(BAR_HOURS_TEXT), g_sdCardUsageStats.barHoursLeft); }
		else { sprintf(&message[0], "%s %s: %dK, %s: ~%dK", getLangText(SPACE_REMAINING_TEXT), getLangText(WAVEFORMS_TEXT), (uint16)(g_sdCardUsageStats.waveEventsLeft / 1000), getLangText(BAR_HOURS_TEXT), (uint16)(g_sdCardUsageStats.barHoursLeft / 1000)); }
	}
	else // Wrapping is on
	{
		if ((g_sdCardUsageStats.waveEventsLeft < 1000) && (g_sdCardUsageStats.barHoursLeft < 1000)) { sprintf(&message[0], "%s %s: %d, %s: ~%d", getLangText(BEFORE_OVERWRITE_TEXT),getLangText(WAVEFORMS_TEXT), g_sdCardUsageStats.waveEventsLeft, getLangText(BAR_HOURS_TEXT), g_sdCardUsageStats.barHoursLeft); }
		else { sprintf(&message[0], "%s %s: %dK, %s: ~%dK", getLangText(BEFORE_OVERWRITE_TEXT), getLangText(WAVEFORMS_TEXT), (uint16)(g_sdCardUsageStats.waveEventsLeft / 1000), getLangText(BAR_HOURS_TEXT), (uint16)(g_sdCardUsageStats.barHoursLeft / 1000)); }
	}

	MessageBox(getLangText(FLASH_USAGE_STATS_TEXT), (char*)message, MB_OK);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayAutoDialInfo(void)
{
	char message[75];
	char dateStr[35];

	if (__autoDialoutTbl.lastConnectTime.valid == NO)
	{
		sprintf(&dateStr[0], "N/A");
	}
	else
	{
		sprintf(&dateStr[0], "%d/%d/%02d %02d:%02d:%02d",
				__autoDialoutTbl.lastConnectTime.month,
				__autoDialoutTbl.lastConnectTime.day,
				__autoDialoutTbl.lastConnectTime.year,
				__autoDialoutTbl.lastConnectTime.hour,
				__autoDialoutTbl.lastConnectTime.min,
				__autoDialoutTbl.lastConnectTime.sec);
	}

	sprintf(&message[0], "%s: %d, %s: %d, %s: %s", getLangText(LAST_DIAL_EVENT_TEXT),__autoDialoutTbl.lastDownloadedEvent,
			getLangText(LAST_RECEIVED_TEXT), GetLastStoredEventNumber(), getLangText(LAST_CONNECTED_TEXT),dateStr);

	MessageBox(getLangText(AUTO_DIALOUT_INFO_TEXT), (char*)message, MB_OK);
}
