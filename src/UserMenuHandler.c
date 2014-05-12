///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: UserMenuHandler.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:09 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/UserMenuHandler.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Menu.h"
#include "Rec.h"
#include "Display.h"
#include "Typedefs.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern REC_EVENT_MN_STRUCT trig_rec;
extern REC_HELP_MN_STRUCT help_rec;
extern USER_MENU_STRUCT helpMenu[];
extern uint32 num_speed;
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern USER_MENU_TAGS_STRUCT menuTags[];
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
USER_MENU_CACHE_STRUCT		userMenuCache[35];
USER_MENU_CACHE_DATA		userMenuCacheData;
USER_MENU_CACHE_STRUCT* 	userMenuCachePtr = &userMenuCache[0];
void (*userMenuHandler)(uint8, void*);

USER_TYPE_STRUCT unitTypes[TOTAL_TYPES] = {
{"", NO_TYPE, 1},
{"", NO_ALT_TYPE, 1},
{"in", IN_TYPE, 1},
{"mm", MM_TYPE, (float)25.4},
{"ft", FT_TYPE, 1},
{"m", M_TYPE, (float)0.3048},
{"lbs", LBS_TYPE, 1},
{"kg", KG_TYPE, (float)0.45454},
{"dB", DB_TYPE, 1},
{"dBa", DBA_TYPE, 1},
{"secs", SECS_TYPE, 1},
{"mins", MINS_TYPE, 1},
{"mg", MG_TYPE, 1}
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void userMn(INPUT_MSG_STRUCT);
void userMnProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void advanceInputChar(uint32 dir);
void copyMenuToCache(USER_MENU_STRUCT* currentMenu);
void copyDataToCache(void* data);
void copyDataToMenu(MN_LAYOUT_STRUCT*);
uint16 findCurrentItemEntry(uint32 item);
void advanceInputNumber(uint32 dir);
void removeExtraSpaces(void);

/****************************************
*	Function:	userMn
*	Purpose:
****************************************/
void userMn(INPUT_MSG_STRUCT msg)
{ 
    static WND_LAYOUT_STRUCT wnd_layout;
    static MN_LAYOUT_STRUCT mn_layout;
     
	// Handle all the preprocessing to setup the menu before it is displayed
    userMnProc(msg, &wnd_layout,&mn_layout);
    
	// Verify that the active menu is still the User Menu
    if (active_menu == USER_MENU)
    {
		// Setup the LCD map with the title position set inside the menu structure
        displayUserMenu(&wnd_layout, &mn_layout, USER_MENU_TITLE_POSITION(userMenuCachePtr));

		// Write the LCD map to the screen
        writeMapToLcd(mmap);
    }
}

/****************************************
*	Function:	userMnProc
*	Purpose:
****************************************/
void userMnProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	uint32 input;
	//uint16 tempCurrLine = 0, tempSubLine = 0;
	//uint16 tempLength = 0;

	// Switch on the incoming command type	
	switch (msg.cmd)
	{
		// New menu display command
		case (ACTIVATE_MENU_WITH_DATA_CMD):
			wnd_layout_ptr->start_col = DEFAULT_COL_SIX;
			wnd_layout_ptr->end_col =   DEFAULT_END_COL;
			wnd_layout_ptr->start_row = DEFAULT_MENU_ROW_ONE;
			wnd_layout_ptr->end_row =   DEFAULT_MENU_ROW_SEVEN;
			mn_layout_ptr->top_ln =     1;
			mn_layout_ptr->sub_ln =     0;

			// Copy the static menu data into the user menu display and set the user menu handler
			copyMenuToCache((USER_MENU_STRUCT*)msg.data[CURRENT_USER_MENU]);

			// Check if the current menu is a select type
			if (USER_MENU_TYPE(userMenuCachePtr) == SELECT_TYPE)
			{
				// Get the current item and set the current line to be highlighted
				mn_layout_ptr->curr_ln = findCurrentItemEntry(msg.data[CURRENT_ITEM_VALUE]);

				// Adjust top line if current line is below the first screen's worth of text
				if (mn_layout_ptr->curr_ln > 6)
					mn_layout_ptr->top_ln = (uint16)(mn_layout_ptr->curr_ln - 5);
			}
			else // Handle other types, INTEGER_BYTE_TYPE, INTEGER_WORD_TYPE, INTEGER_WORD_FIXED_TYPE, INTEGER_LONG_TYPE, 
					// INTEGER_SPECIAL_TYPE, INTEGER_COUNT_TYPE, STRING_TYPE, FLOAT_TYPE, FLOAT_SPECIAL_TYPE, FLOAT_WITH_N_TYPE
			{
				// Get the default item and set the current line to be highlighted
				mn_layout_ptr->curr_ln = USER_MENU_DEFAULT_ROW(userMenuCachePtr);

				// Copy the (passed by pointer) variable menu data to the user menu data cache
				copyDataToCache((void*)msg.data[CURRENT_DATA_POINTER]);

				// Copy the variable data from the user menu data cache to the user menu display
				copyDataToMenu(mn_layout_ptr);
			}

			debug("User Menu <%s>, Displayable Items: %d\n", userMenuCachePtr[MENU_INFO].text, USER_MENU_DISPLAY_ITEMS(userMenuCachePtr));
		break;

		// Hanle a keypress message
		case (KEYPRESS_MENU_CMD):
			input = msg.data[0];

			// Check if the current menu is a select type
			if (USER_MENU_TYPE(userMenuCachePtr) == SELECT_TYPE)
			{
				// Check if the total number of active items (minus the title and end line) is less than 10
				if (USER_MENU_ACTIVE_ITEMS(userMenuCachePtr) < 10)
				{
					// Handle converting a number key input into a manu selection
					if ((input >= ONE_KEY) && (input <= (USER_MENU_ACTIVE_ITEMS(userMenuCachePtr) | 0x30)))
					{
						// Convert the ASCII key hex value to a true number to set the current line
						mn_layout_ptr->curr_ln = (uint16)(input - 0x30);

						// Set the input to be an Enter key to allow processing the current line
						input = ENTER_KEY;
					}
				}
			}

			// Handle the specific key that was pressed
			switch (input)
			{
				case (ENTER_KEY):
					// Make sure the user menu handler is not null before jumping to the routine
					if (userMenuHandler != NULL)
					{
						if (USER_MENU_TYPE(userMenuCachePtr) == SELECT_TYPE)
						{
							// Set the current index to the user menu current line
							userMenuCacheData.currentIndex = mn_layout_ptr->curr_ln;
							
							// Call the user menu handler, passing the key and the address of the index
							(*userMenuHandler)(ENTER_KEY, &userMenuCacheData.currentIndex);
						}
						else if (USER_MENU_TYPE(userMenuCachePtr) == STRING_TYPE)
						{
							// Remove any extra spaces as the end of the string
							removeExtraSpaces();

							// Call the user menu handler, passing the key and the address of the string
							(*userMenuHandler)(ENTER_KEY, &userMenuCacheData.text);
						}
						else if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_BYTE_TYPE)
						{
							// Call the user menu handler, passing the key and the address of the byte data
							(*userMenuHandler)(ENTER_KEY, &userMenuCacheData.numByteData);
						}
						else if ((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_WORD_TYPE) ||
								(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_WORD_FIXED_TYPE))
						{
							// Call the user menu handler, passing the key and the address of the word data
							(*userMenuHandler)(ENTER_KEY, &userMenuCacheData.numWordData);
						}
						else if ((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_LONG_TYPE) || 
								(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_SPECIAL_TYPE) ||
								(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_COUNT_TYPE))
						{
							// Call the user menu handler, passing the key and the address of the long data
							(*userMenuHandler)(ENTER_KEY, &userMenuCacheData.numLongData);
						}
						else if ((USER_MENU_TYPE(userMenuCachePtr) == FLOAT_TYPE) ||
								(USER_MENU_TYPE(userMenuCachePtr) == FLOAT_SPECIAL_TYPE) ||
								(USER_MENU_TYPE(userMenuCachePtr) == FLOAT_WITH_N_TYPE))
						{
							// Call the user menu handler, passing the key and the address of the float data
							(*userMenuHandler)(ENTER_KEY, &userMenuCacheData.floatData);
						}
					}
				break;      

				case (ESC_KEY):
					if (userMenuHandler != NULL)
					{
						// Call the user menu handler, passing the key and null for the data pointer
						(*userMenuHandler)(ESC_KEY, NULL);
					}
				break;

				case (HELP_KEY):
					// Jump to the Help menu
					ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
					(*menufunc_ptrs[active_menu]) (mn_msg);
				break;

				case (DOWN_ARROW_KEY):
				case (UP_ARROW_KEY):
					if (USER_MENU_TYPE(userMenuCachePtr) == SELECT_TYPE)
					{
						// Scroll the highlighted menu item up or down based on the key used
						userMenuScroll((uint8)input, SELECT_MN_WND_LNS, mn_layout_ptr);
					}
					else if (USER_MENU_TYPE(userMenuCachePtr) == STRING_TYPE)
					{
						// If the current char is a null and not the max index of the string
						if ((userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] == '\0') &&
							(userMenuCachePtr[CURRENT_TEXT_INDEX].data < userMenuCachePtr[MAX_TEXT_CHARS].data))
						{
							// Set the char at the current index to be an "A"
							userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] = 'A';
						}
						else
						{
							// Handle advancing the character up or down based on the key used
							advanceInputChar(input);
						}

						// Copy the string data to the user menu display
						copyDataToMenu(mn_layout_ptr);
					}
					else // INTEGER_BYTE_TYPE, INTEGER_WORD_TYPE, INTEGER_WORD_FIXED_TYPE, INTEGER_LONG_TYPE, 
							// INTEGER_SPECIAL_TYPE, INTEGER_COUNT_TYPE, FLOAT_TYPE, FLOAT_SPECIAL_TYPE, FLOAT_WITH_N_TYPE
					{
						// Handle advancing the numerical data up or down based on the key used
						advanceInputNumber(input);

						// Copy the numerical data to the user menu display
						copyDataToMenu(mn_layout_ptr);
					}
				break;

				case (PLUS_KEY):
					if (USER_MENU_TYPE(userMenuCachePtr) == SELECT_TYPE)
					{
						// Change the contrast
						adjustLcdContrast(LIGHTER);					
					}
					else if (USER_MENU_TYPE(userMenuCachePtr) == STRING_TYPE)
					{
						// Check if the current index is less than the max number of characters for the string
						if (userMenuCachePtr[CURRENT_TEXT_INDEX].data < userMenuCachePtr[MAX_TEXT_CHARS].data)
						{
							// Check if a null is in the current location
							if (userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] == '\0')
							{
								// Inject a space at the current position to keep the string continuous
								userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data++] = ' ';
								
								// Set the next char position to a null to allow for termination
								userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] = '\0';
							}
							else
							{
								// Increment the index
								userMenuCachePtr[CURRENT_TEXT_INDEX].data++;
							}

							// Copy the string data to the user menu display							
							copyDataToMenu(mn_layout_ptr);
						}
					}
				break;

				case (MINUS_KEY):
					if (USER_MENU_TYPE(userMenuCachePtr) == SELECT_TYPE)
					{
						// Change the contrast
						adjustLcdContrast(DARKER);
					}
					else if (USER_MENU_TYPE(userMenuCachePtr) == STRING_TYPE)
					{
						// Check if the current index is greater than or equal to the first position
						if (userMenuCachePtr[CURRENT_TEXT_INDEX].data >= 0)
						{
							// Check if a space is in the current location, and a null follows it
							if ((userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] == ' ') &&
								(userMenuCacheData.text[(userMenuCachePtr[CURRENT_TEXT_INDEX].data) + 1] == '\0'))
							{
								// Set the current space char to be a null
								userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] = '\0';
							}

							// Check if the index is past the first position
							if (userMenuCachePtr[CURRENT_TEXT_INDEX].data > 0)
							{
								// Decrement the index
								userMenuCachePtr[CURRENT_TEXT_INDEX].data--;
							}

							// Copy the string data to the user menu display
							copyDataToMenu(mn_layout_ptr);
						}
					}
				break;

				case (DELETE_KEY):
					if (USER_MENU_TYPE(userMenuCachePtr) == STRING_TYPE)
					{
						// Check if the current index is the first position and the string length is greater than zero
						if ((userMenuCachePtr[CURRENT_TEXT_INDEX].data == 0) && (strlen(userMenuCacheData.text) > 0))
						{
							// Set the current index to 1 to allow the first char to be deleted (next if statement)
							userMenuCachePtr[CURRENT_TEXT_INDEX].data = 1;
						}

						// Check if the current index is beyond the first position
						if (userMenuCachePtr[CURRENT_TEXT_INDEX].data > 0)
						{
							//debug("User Input Data String: <%s>\n", userMenuCacheData.text);

							// Copy the string from the current index to one position left of the index
							strcpy((char*)&(userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data - 1]), 
									(char*)&(userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data]));

							//debug("User Input Data String: <%s>\n", userMenuCacheData.text);

							// Set the index to be one less (back to the same char)
							userMenuCachePtr[CURRENT_TEXT_INDEX].data--;
						}

						// Copy the string data to the user menu display
						copyDataToMenu(mn_layout_ptr);
					}
				break;
				
				default:
					if (USER_MENU_TYPE(userMenuCachePtr) == STRING_TYPE)
					{
						// Check if the current index is less then the max length of the string
						if (userMenuCachePtr[CURRENT_TEXT_INDEX].data < userMenuCachePtr[MAX_TEXT_CHARS].data)
						{
							// Set the char at the current index to be the key input
							userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data++] = (char)input;
						}	

						// Copy the string data to the user menu display
						copyDataToMenu(mn_layout_ptr);
					}
				break;
			}
		break;
	}
}

/****************************************
*	Function:	advanceInputChar
*	Purpose:	Increment or decrement the char value at the current index for the current menu
****************************************/
void advanceInputChar(uint32 direction)
{
	// Store the char at the current index
	char currVal = userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data];
	char newVal = 'A';

	// Check to make sure the index isn't the max (one past the end of the allocated amount)
	if (userMenuCachePtr[CURRENT_TEXT_INDEX].data < userMenuCachePtr[MAX_TEXT_CHARS].data)
	{
		if (direction == UP_ARROW_KEY)
		{
			switch (currVal)
			{
				// Advance the char based on the following "rules"
				case ' ': newVal = 'A'; break;
				case 'Z': newVal = '0'; break;
				case '9': newVal = '#'; break;
				case '#': newVal = '%'; break;
				case '&': newVal = '('; break;
				case '/': newVal = '\\'; break;
				case '\\': newVal = ':'; break;
				case ':': newVal = '<'; break;
				case '<': newVal = '>'; break;
				case '@': newVal = '='; break;
				case '=': newVal = '^'; break;
				case '^': newVal = '^'; break;
				default : newVal = ++currVal; break;
			}
		}
		else // direction == DOWN_ARROW_KEY
		{
			switch (currVal)
			{
				// Advance the char based on the following "rules"
				case '^': newVal = '='; break;
				case '=': newVal = '@'; break;
				case '>': newVal = '<'; break;
				case '<': newVal = ':'; break;
				case ':': newVal = '\\'; break;
				case '\\': newVal = '/'; break;
				case '(': newVal = '&'; break;
				case '%': newVal = '#'; break;
				case '#': newVal = '9'; break;
				case '0': newVal = 'Z'; break;
				case 'A': newVal = ' '; break;
				case ' ': newVal = ' '; break;
				default : newVal = --currVal; break;
			}
		}

		debug("User Input Advance Char: %c\n", newVal);

		// Change the char at the current position to be the new value
		userMenuCacheData.text[userMenuCachePtr[CURRENT_TEXT_INDEX].data] = newVal;
	}
}

/****************************************
*	Function:	advanceInputNumber
*	Purpose:	Increment or decrement the numerical value for the current menu
****************************************/
void advanceInputNumber(uint32 direction)
{
	// Check if direction is an up arrow to increment the number
	if (direction == UP_ARROW_KEY)
	{
		// Switch on the user menu type
		switch (USER_MENU_TYPE(userMenuCachePtr))
		{
			case INTEGER_BYTE_TYPE:
				// Check if the current integer byte data is less than the max
				if (userMenuCacheData.numByteData < userMenuCacheData.intMaxValue)
				{
					// Check if the data incremented by the key scrolling speed is greater than the max
					if ((userMenuCacheData.numByteData + num_speed) > userMenuCacheData.intMaxValue)
					{
						// Set the max value
						userMenuCacheData.numByteData = (uint8)userMenuCacheData.intMaxValue;
					}
					else
					{
						// Increment the data by the key scrolling speed
						userMenuCacheData.numByteData += num_speed;
					}
				}
			break;

			case INTEGER_WORD_TYPE:
			case INTEGER_WORD_FIXED_TYPE:
				// Check if the current integer word data is less than the max
				if (userMenuCacheData.numWordData < userMenuCacheData.intMaxValue)
				{
					// Check if the data incremented by the key scrolling speed is greater than the max
					if ((userMenuCacheData.numWordData + num_speed) > userMenuCacheData.intMaxValue)
					{
						// Set the max value
						userMenuCacheData.numWordData = (uint16)userMenuCacheData.intMaxValue;
					}
					else
					{
						// Increment the data by the key scrolling speed
						userMenuCacheData.numWordData += num_speed;
					}
				}
			break;

			case INTEGER_LONG_TYPE:
			case INTEGER_SPECIAL_TYPE:
			case INTEGER_COUNT_TYPE:
				// Check if the menu type is integer special or integer count and currently set as NO_TRIGGER
				if (((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_SPECIAL_TYPE) || 
					(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_COUNT_TYPE)) && 
					(userMenuCacheData.numLongData == NO_TRIGGER_CHAR))
				{
					// Check if the boundary condition is not the top (bottom then)
					if (userMenuCacheData.boundary != TOP_BOUNDARY)
					{
						// Set the min value						
						userMenuCacheData.numLongData = userMenuCacheData.intMinValue;

						// Clear the boundary condition
						userMenuCacheData.boundary = NO_BOUNDARY;
					}
				}
				// Check if the current integer long data is less than the max
				else if (userMenuCacheData.numLongData < userMenuCacheData.intMaxValue)
				{
					// Check if the data incremented by the key scrolling speed is greater than the max
					if ((userMenuCacheData.numLongData + num_speed) > userMenuCacheData.intMaxValue)
					{
						// Set the max value
						userMenuCacheData.numLongData = userMenuCacheData.intMaxValue;
					}
					else
					{
						// Increment the data by the key scrolling speed
						userMenuCacheData.numLongData += num_speed;
					}
				}
				// Check if the menu type is integer special or integer count
				else if ((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_SPECIAL_TYPE) ||
						(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_COUNT_TYPE))
				{
					// If here, then the current data is equal to the max

					// Set the current data to the NO_TRIGGER value
					userMenuCacheData.numLongData = NO_TRIGGER_CHAR;

					// Set the boundary condition to the top
					userMenuCacheData.boundary = TOP_BOUNDARY;
				}
			break;

			case FLOAT_TYPE:
			case FLOAT_SPECIAL_TYPE:
			case FLOAT_WITH_N_TYPE:
				// Check if the menu type is float special and currently set as NO_TRIGGER
				if ((USER_MENU_TYPE(userMenuCachePtr) == FLOAT_SPECIAL_TYPE) && 
					(userMenuCacheData.floatData == NO_TRIGGER_CHAR))
				{
					// Check if the boundary condition is not the top (bottom then)
					if (userMenuCacheData.boundary != TOP_BOUNDARY)
					{
						// Set the min value						
						userMenuCacheData.floatData = userMenuCacheData.floatMinValue;

						// Clear the boundary condition
						userMenuCacheData.boundary = NO_BOUNDARY;
					}
				}
				// Check if the current float long data is less than the max
				else if (userMenuCacheData.floatData < userMenuCacheData.floatMaxValue)
				{
					// Check if the data incremented by the key scrolling speed times the increment value is greater than the max
					if ((userMenuCacheData.floatData + (userMenuCacheData.floatIncrement * num_speed)) >
						userMenuCacheData.floatMaxValue)
					{
						// Set the max value
						userMenuCacheData.floatData = userMenuCacheData.floatMaxValue;
					}
					else
					{
						// Increment the data by the key scrolling speed times the increment value
						userMenuCacheData.floatData += (userMenuCacheData.floatIncrement * num_speed);
					}
				}
				// Check if the menu type is float special
				else if (USER_MENU_TYPE(userMenuCachePtr) == FLOAT_SPECIAL_TYPE)
				{
					// If here, then the current data is equal to the max

					// Set the current data to the NO_TRIGGER value
					userMenuCacheData.floatData = NO_TRIGGER_CHAR;

					// Set the boundary condition to the top
					userMenuCacheData.boundary = TOP_BOUNDARY;
				}
			break;
		}
	}
	else // direction == DOWN_ARROW_KEY, need to decrement the number
	{
		switch (USER_MENU_TYPE(userMenuCachePtr))
		{
			case INTEGER_BYTE_TYPE:
				// Check if the current integer byte data is greater than the min
				if (userMenuCacheData.numByteData > userMenuCacheData.intMinValue)
				{
					// Check if the current data is greater than the key scrolling speed
					if (userMenuCacheData.numByteData > num_speed)
					{
						// Check if the data decremented by the key scrolling speed is less than the min
						if ((userMenuCacheData.numByteData - num_speed) < userMenuCacheData.intMinValue)
						{
							// Set the min value
							userMenuCacheData.numByteData = (uint8)userMenuCacheData.intMinValue;
						}
						else
						{
							// Decrement the data by the key scrolling speed
							userMenuCacheData.numByteData -= num_speed;
						}
					}
					else // current data is less than the key scrolling speed
					{
						// Set the min value
						userMenuCacheData.numByteData = (uint8)userMenuCacheData.intMinValue;
					}
				}
			break;

			case INTEGER_WORD_TYPE:
			case INTEGER_WORD_FIXED_TYPE:
				// Check if the current integer word data is greater than the min
				if (userMenuCacheData.numWordData > userMenuCacheData.intMinValue)
				{
					// Check if the current data is greater than the key scrolling speed
					if (userMenuCacheData.numWordData > num_speed)
					{
						// Check if the data decremented by the key scrolling speed is less than the min
						if ((userMenuCacheData.numWordData - num_speed) < userMenuCacheData.intMinValue)
						{
							// Set the min value
							userMenuCacheData.numWordData = (uint16)userMenuCacheData.intMinValue;
						}
						else
						{
							// Decrement the data by the key scrolling speed
							userMenuCacheData.numWordData -= num_speed;
						}
					}
					else // current data is less than the key scrolling speed
					{
						// Set the min value
						userMenuCacheData.numWordData = (uint16)userMenuCacheData.intMinValue;
					}
				}
			break;

			case INTEGER_LONG_TYPE:
			case INTEGER_SPECIAL_TYPE:
			case INTEGER_COUNT_TYPE:
				// Check if the menu type is integer special or integer count and currently set as NO_TRIGGER
				if (((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_SPECIAL_TYPE) || 
					(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_COUNT_TYPE)) && 
					(userMenuCacheData.numLongData == NO_TRIGGER_CHAR))
				{
					// Check if the boundary condition is not the bottom (top then)
					if (userMenuCacheData.boundary != BOTTOM_BOUNDARY)
					{
						// Set the max value						
						userMenuCacheData.numLongData = userMenuCacheData.intMaxValue;

						// Clear the boundary condition
						userMenuCacheData.boundary = NO_BOUNDARY;
					}
				}
				// Check if the current integer long data is greater than the min
				else if (userMenuCacheData.numLongData > userMenuCacheData.intMinValue)
				{
					// Check if the current data is greater than the key scrolling speed
					if (userMenuCacheData.numLongData > num_speed)
					{
						// Check if the data decremented by the key scrolling speed is less than the min
						if ((userMenuCacheData.numLongData - num_speed) < userMenuCacheData.intMinValue)
						{
							// Set the min value
							userMenuCacheData.numLongData = userMenuCacheData.intMinValue;
						}
						else
						{
							// Decrement the data by the key scrolling speed
							userMenuCacheData.numLongData -= num_speed;
						}
					}
					else // current data is less than the key scrolling speed
					{
						userMenuCacheData.numLongData = userMenuCacheData.intMinValue;
					}
				}
				// Check if the menu type is integer special or integer count
				else if ((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_SPECIAL_TYPE) ||
						(USER_MENU_TYPE(userMenuCachePtr) == INTEGER_COUNT_TYPE))
				{
					// If here, then the current data is equal to the min

					// Set the current data to the NO_TRIGGER value
					userMenuCacheData.numLongData = NO_TRIGGER_CHAR;

					// Set the boundary condition to the bottom
					userMenuCacheData.boundary = BOTTOM_BOUNDARY;
				}
			break;

			case FLOAT_TYPE:
			case FLOAT_SPECIAL_TYPE:
			case FLOAT_WITH_N_TYPE:
				// Check if the menu type is float special and currently set as NO_TRIGGER
				if ((USER_MENU_TYPE(userMenuCachePtr) == FLOAT_SPECIAL_TYPE) && 
					(userMenuCacheData.floatData == NO_TRIGGER_CHAR))
				{
					// Check if the boundary condition is not the bottom (top then)
					if (userMenuCacheData.boundary != BOTTOM_BOUNDARY)
					{
						// Set the max value						
						userMenuCacheData.floatData = userMenuCacheData.floatMaxValue;

						// Clear the boundary condition
						userMenuCacheData.boundary = NO_BOUNDARY;
					}
				}
				// Check if the current float long data is greater than the min
				else if (userMenuCacheData.floatData > userMenuCacheData.floatMinValue)
				{
					// Check if the current data is greater than the key scrolling speed times the increment value
					if (userMenuCacheData.floatData > (userMenuCacheData.floatIncrement * num_speed))
					{
						// Check if the data decremented by the key scrolling speed times the increment value is less than the min
						if ((userMenuCacheData.floatData - (userMenuCacheData.floatIncrement * num_speed)) <
							userMenuCacheData.floatMinValue)
						{
							// Set the min value
							userMenuCacheData.floatData = userMenuCacheData.floatMinValue;
						}
						else
						{
							// Decrement the data by the key scrolling speed times the increment value
							userMenuCacheData.floatData -= (userMenuCacheData.floatIncrement * num_speed);
						}
					}
					else // current data is less than the key scrolling speed
					{
						userMenuCacheData.floatData = userMenuCacheData.floatMinValue;
					}
				}
				// Check if the menu type is float special
				else if (USER_MENU_TYPE(userMenuCachePtr) == FLOAT_SPECIAL_TYPE)
				{
					// If here, then the current data is equal to the min

					// Set the current data to the NO_TRIGGER value
					userMenuCacheData.floatData = NO_TRIGGER_CHAR;

					// Set the boundary condition to the bottom
					userMenuCacheData.boundary = BOTTOM_BOUNDARY;
				}
			break;
		}
	}
}

/****************************************
*	Function:	copyMenuToCache
*	Purpose:	Copy the static menu data to the user menu display cache
****************************************/
void copyMenuToCache(USER_MENU_STRUCT* currentMenu)
{
	int i;
	
	// Set the user menu handler (data value in the last menu index)
	userMenuHandler = (void*)currentMenu[(USER_MENU_TOTAL_ITEMS(currentMenu) - 1)].data;

	// Clear the user menu display cache
	byteSet(&userMenuCache, 0, sizeof(userMenuCache));

	// Copy the menu contents over to the user menu display cache (minus the last line with the menu handler)
	for (i = 0; i < USER_MENU_DISPLAY_ITEMS(currentMenu); i++)
	{
		// Check if the menu line pre-number value is zero
		if (currentMenu[i].preNum == 0)
		{
			// Copy the pre tag, text entry and post tag to the text of the current user menu display line
			sprintf(userMenuCachePtr[i].text, "%s%s%s", menuTags[currentMenu[i].preTag].text,
					getLangText(currentMenu[i].textEntry), menuTags[currentMenu[i].postTag].text);
		}
		else // A pre-number is present
		{
			// Copy the pre tag, pre num, text entry and post tag to the text of the current user menu display line
			sprintf(userMenuCachePtr[i].text, "%s%d %s%s", menuTags[currentMenu[i].preTag].text,
					currentMenu[i].preNum, getLangText(currentMenu[i].textEntry), 
					menuTags[currentMenu[i].postTag].text);
		}

		// Copy the menu data over
		userMenuCachePtr[i].data = currentMenu[i].data;
	}
	
	// Copy ".end." to the text to signal the end of the user menu display text data
	strcpy(userMenuCachePtr[i].text, ".end.");
}

/****************************************
*	Function:	copyDataToCache
*	Purpose:	Copy the variable data (passed in via pointer) to the user menu data cache 
****************************************/
void copyDataToCache(void* data)
{
	// Switch on the menu type
	switch (USER_MENU_TYPE(userMenuCachePtr))
	{
		case STRING_TYPE:
			// Clear the data cache text string
			byteSet(&(userMenuCacheData.text[0]), 0, sizeof(userMenuCacheData.text));
		
			// Check to make sure the data pointer isn't null
			if (data != NULL)
			{
				// Copy the string into the data cache
				strcpy(userMenuCacheData.text, (char*)data);
				debug("User Input Text <%s>, Length: %d\n", userMenuCacheData.text, strlen(userMenuCacheData.text));
			}			

			// Set the current index to the string length
			userMenuCachePtr[CURRENT_TEXT_INDEX].data = strlen(userMenuCacheData.text);
		break;
		
		case INTEGER_BYTE_TYPE:
			// Clear the data cache byte data
			userMenuCacheData.numByteData = 0;
			
			// Check to make sure the data pointer isn't null
			if (data != NULL)
			{
				// Copy the byte into the data cache
				userMenuCacheData.numByteData = *((uint8*)data);
			}

			// Check if the byte data is greater than the max or less than the min
			if ((userMenuCacheData.numByteData > userMenuCacheData.intMaxValue) ||
				(userMenuCacheData.numByteData < userMenuCacheData.intMinValue))
			{
				// Set the default value in the byte data
				userMenuCacheData.numByteData = (uint8)userMenuCacheData.intDefault;
				debug("User Input Integer not within Range, Setting to Default: %d\n", userMenuCacheData.numByteData);
			}
			
			// Set the unit text pointer to the default unit type for the data
			userMenuCacheData.unitText = unitTypes[USER_MENU_DEFAULT_TYPE(userMenuCachePtr)].text;

			// Check if units is metric and the alternative unit type is set
			if ((help_rec.units_of_measure == METRIC_TYPE) && (USER_MENU_ALT_TYPE(userMenuCachePtr) != NO_ALT_TYPE))
			{
				// Adjust the byte data, min and max values by the units conversion for display purposes
				userMenuCacheData.numByteData = (uint8)(userMenuCacheData.numByteData * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				userMenuCacheData.intMinValue = (uint32)(userMenuCacheData.intMinValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				userMenuCacheData.intMaxValue = (uint32)(userMenuCacheData.intMaxValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);

				// Set the unit text pointer to the alternative unit type
				userMenuCacheData.unitText = unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].text;
			}
		break;
		
		case INTEGER_WORD_TYPE:
		case INTEGER_WORD_FIXED_TYPE:
			// Clear the data cache word data
			userMenuCacheData.numWordData = 0;
			
			// Check to make sure the data pointer isn't null
			if (data != NULL)
			{
				// Copy the word into the data cache
				userMenuCacheData.numWordData = *((uint16*)data);
			}

			// Check if the word data is greater than the max or less than the min
			if ((userMenuCacheData.numWordData > userMenuCacheData.intMaxValue) ||
				(userMenuCacheData.numWordData < userMenuCacheData.intMinValue))
			{
				// Set the default value in the word data
				userMenuCacheData.numWordData = (uint16)userMenuCacheData.intDefault;
				debug("User Input Integer not within Range, Setting to Default: %d\n", userMenuCacheData.numWordData);
			}

			// Set the unit text pointer to the default unit type for the data
			userMenuCacheData.unitText = unitTypes[USER_MENU_DEFAULT_TYPE(userMenuCachePtr)].text;

			// Check if units is metric and the alternative unit type is set
			if ((help_rec.units_of_measure == METRIC_TYPE) && (USER_MENU_ALT_TYPE(userMenuCachePtr) != NO_ALT_TYPE))
			{
				// Adjust the word data, min and max values by the units conversion for display purposes
				userMenuCacheData.numWordData = (uint16)(userMenuCacheData.numWordData * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				userMenuCacheData.intMinValue = (uint32)(userMenuCacheData.intMinValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				userMenuCacheData.intMaxValue = (uint32)(userMenuCacheData.intMaxValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);

				// Set the unit text pointer to the alternative unit type
				userMenuCacheData.unitText = unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].text;
			}
		break;
		
		case INTEGER_LONG_TYPE:
		case INTEGER_SPECIAL_TYPE:
		case INTEGER_COUNT_TYPE:
			// Clear the data cache long data
			userMenuCacheData.numLongData = 0;

			// Clear the boundary condition
			userMenuCacheData.boundary = NO_BOUNDARY;
			
			// Check to make sure the data pointer isn't null
			if (data != NULL)
			{
				// Copy the long into the data cache
				userMenuCacheData.numLongData = *((uint32*)data);
			}

			// Check if the menu type is integer long or the data isn't NO_TRIGGER
			if ((USER_MENU_TYPE(userMenuCachePtr) == INTEGER_LONG_TYPE) || (userMenuCacheData.numLongData != NO_TRIGGER_CHAR))
			{
				// Check if the long data is greater than the max or less than the min
				if ((userMenuCacheData.numLongData > userMenuCacheData.intMaxValue) ||
					(userMenuCacheData.numLongData < userMenuCacheData.intMinValue))
				{
					// Set the default value in the long data
					userMenuCacheData.numLongData = userMenuCacheData.intDefault;
					debug("User Input Integer not within Range, Setting to Default: %d\n", userMenuCacheData.numLongData);
				}
			}

			// Set the unit text pointer to the default unit type for the data
			userMenuCacheData.unitText = unitTypes[USER_MENU_DEFAULT_TYPE(userMenuCachePtr)].text;

			// Check if the decibels type is set as the default type
			if (USER_MENU_DEFAULT_TYPE(userMenuCachePtr) == DB_TYPE)
			{
				// Check if the factory setup is valid and A weighting is enabled
				if ((!factory_setup_rec.invalid) && (factory_setup_rec.aweight_option == ENABLED))
				{
					// Set the unit text pointer to the alternative type
					userMenuCacheData.unitText = unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].text;
				}
			}
			// Check if units is metric and the alternative unit type is set
			else if ((help_rec.units_of_measure == METRIC_TYPE) && (USER_MENU_ALT_TYPE(userMenuCachePtr) != NO_ALT_TYPE))
			{
				// Check if the menu type isn't integer count
				if (USER_MENU_TYPE(userMenuCachePtr) != INTEGER_COUNT_TYPE)
				{
					// Check if the data isn't equal to NO_TRIGGER
					if (userMenuCacheData.numLongData != NO_TRIGGER_CHAR)
					{
						// Adjust the long data by the units conversion for display purposes
						userMenuCacheData.numLongData = (uint32)(userMenuCacheData.numLongData * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
					}

					// Adjust the min and max values by the units conversion for display purposes
					userMenuCacheData.intMinValue = (uint32)(userMenuCacheData.intMinValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
					userMenuCacheData.intMaxValue = (uint32)(userMenuCacheData.intMaxValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				}

				// Set the unit text pointer to the alternative unit type
				userMenuCacheData.unitText = unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].text;
			}
		break;
		
		case FLOAT_TYPE:
		case FLOAT_SPECIAL_TYPE:
		case FLOAT_WITH_N_TYPE:
			// Clear the data cache float data
			userMenuCacheData.floatData = 0;

			// Clear the boundary condition
			userMenuCacheData.boundary = NO_BOUNDARY;

			// Check to make sure the data pointer isn't null
			if (data != NULL)
			{
				// Copy the float into the data cache
				userMenuCacheData.floatData = *((float*)data);
			}

			// Check if the menu type isn't float special or the data isn't NO_TRIGGER
			if ((USER_MENU_TYPE(userMenuCachePtr) != FLOAT_SPECIAL_TYPE) || (userMenuCacheData.floatData != NO_TRIGGER_CHAR))
			{
				// Check if the float data is greater than the max or less than the min
				if ((userMenuCacheData.floatData > userMenuCacheData.floatMaxValue) ||
					(userMenuCacheData.floatData < userMenuCacheData.floatMinValue))
				{
					// Set the default value in the long data
					userMenuCacheData.floatData = userMenuCacheData.floatDefault;
					debug("User Input Float not within Range, Setting to Default: %f\n", userMenuCacheData.floatData);
				}
			}

			// Set the unit text pointer to the default unit type for the data
			userMenuCacheData.unitText = unitTypes[USER_MENU_DEFAULT_TYPE(userMenuCachePtr)].text;

			// Check if units is metric and the alternative unit type is set
			if ((help_rec.units_of_measure == METRIC_TYPE) && (USER_MENU_ALT_TYPE(userMenuCachePtr) != NO_ALT_TYPE))
			{
				// Check if the data isn't equal to NO_TRIGGER
				if (userMenuCacheData.floatData != NO_TRIGGER_CHAR)
				{
					// Adjust the float data by the units conversion for display purposes
					userMenuCacheData.floatData = (float)(userMenuCacheData.floatData * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				}

				// Adjust the min and max values by the units conversion for display purposes
				userMenuCacheData.floatMinValue = (float)(userMenuCacheData.floatMinValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);
				userMenuCacheData.floatMaxValue = (float)(userMenuCacheData.floatMaxValue * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion);

				// Set the unit text pointer to the alternative unit type
				userMenuCacheData.unitText = unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].text;
			}
		break;
		
		case DATE_TIME_TYPE:
			// To be added in the future
		break;
	}
}

/****************************************
*	Function:	copyDataToMenu
*	Purpose:	Copy the variable data into the user menu display cache
****************************************/
void copyDataToMenu(MN_LAYOUT_STRUCT* menu_layout)
{
	uint32 charLen = 0;
	int tempRow = USER_MENU_DEFAULT_ROW(userMenuCachePtr);
	int i = 0;
	
	// Switch on the menu type
	switch (USER_MENU_TYPE(userMenuCachePtr))
	{
		case STRING_TYPE:
			// Set the specifications line for max chars
			sprintf(userMenuCachePtr[MAX_TEXT_CHARS].text, "(%s %lu %s)", getLangText(MAX_TEXT),
					userMenuCachePtr[MAX_TEXT_CHARS].data, getLangText(CHARS_TEXT));

			// For the current row to the end of the menu (minus the last line with the menu handler)
			for (i = tempRow; i < USER_MENU_DISPLAY_ITEMS(userMenuCachePtr); i++)
			{
				// Clear the user menu display cache
				byteSet(&(userMenuCachePtr[i].text[0]), 0, MAX_CHAR_PER_LN);
			}

			// get the string length
			charLen = strlen(userMenuCacheData.text);
			//debug("User Input Data String(%d): <%s>\n", charLen, userMenuCacheData.text);

			// Check if the string length is zero
			if (charLen == 0)
			{
				// Print the "<EMPTY>" text
				sprintf(userMenuCachePtr[tempRow].text, "<%s>", getLangText(EMPTY_TEXT));

				// Set the current line and sub line (column)
				menu_layout->curr_ln = (uint16)tempRow;
				menu_layout->sub_ln = 0;
			}
			else // string length is greater than zero
			{
				// While the number of characters left is greater than 20 (chars per line)
				while ((charLen > 20) && (tempRow < USER_MENU_ACTIVE_ITEMS(userMenuCachePtr)))
				{
					// Copy the 20 chars to the user menu display cache
					strncpy(userMenuCachePtr[tempRow].text, (char*)&(userMenuCacheData.text[(tempRow - 2) * 20]), 20);

					// Increment the row
					tempRow++;

					// Decrement the number of chars left
					charLen -= 20;
				}

				// Check if the number of chars left is less than 20
				if (charLen <= 20)
				{
					// Copy the rest of the chars to the current row
					strcpy(userMenuCachePtr[tempRow].text, (char*)&(userMenuCacheData.text[(tempRow - 2) * 20]));
				}

				// Set the current line and sub line (column)
				menu_layout->curr_ln = (uint8)((userMenuCachePtr[CURRENT_TEXT_INDEX].data / 20) + 
										USER_MENU_DEFAULT_ROW(userMenuCachePtr));
				menu_layout->sub_ln = (uint8)((userMenuCachePtr[CURRENT_TEXT_INDEX].data % 20) + 1);
			}
			
			//debug("User Input Current Index: %d, Current Line: %d, Sub Line: %d\n", 
			//		userMenuCachePtr[CURRENT_TEXT_INDEX].data, menu_layout->curr_ln, menu_layout->sub_ln);
		break;
		
		case INTEGER_BYTE_TYPE:
		case INTEGER_WORD_TYPE:
		case INTEGER_WORD_FIXED_TYPE:
		case INTEGER_LONG_TYPE:
		case INTEGER_SPECIAL_TYPE:
		case INTEGER_COUNT_TYPE:
			if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_SPECIAL_TYPE)
			{
				// Set the specifications line for the integer type
				sprintf(userMenuCachePtr[INTEGER_RANGE].text, "(%lu-%lu%s, N)", 
					userMenuCacheData.intMinValue, userMenuCacheData.intMaxValue, userMenuCacheData.unitText);

				// Check if the data is NO_TRIGGER
				if (userMenuCacheData.numLongData == NO_TRIGGER_CHAR)
				{
					// Print a "N"
					sprintf(userMenuCachePtr[tempRow].text, "N");
				}
				else
				{
					// Print the data value
					sprintf(userMenuCachePtr[tempRow].text, "%lu", userMenuCacheData.numLongData);
				}
			}
			else if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_COUNT_TYPE)
			{
				// Check if the units are metric and no alternative type is set and not the accelerometer
				if ((help_rec.units_of_measure == METRIC_TYPE) && (USER_MENU_ALT_TYPE(userMenuCachePtr) != NO_ALT_TYPE) &&
					(factory_setup_rec.sensor_type != SENSOR_ACC))
				{
					// Init the float increment value adjusted by the units conversion
					userMenuCacheData.floatIncrement = ((float)(factory_setup_rec.sensor_type * unitTypes[USER_MENU_ALT_TYPE(userMenuCachePtr)].conversion) / 
														(float)(((trig_rec.srec.sensitivity == LOW) ? 200 : 400) * ADC_RESOLUTION));
				}
				else
				{
					// Init the float increment value
					userMenuCacheData.floatIncrement = ((float)(factory_setup_rec.sensor_type) / 
														(float)(((trig_rec.srec.sensitivity == LOW) ? 200 : 400) * ADC_RESOLUTION));
				}

				// Set the min, max and data count values adjusted by the float incrememnt
				userMenuCacheData.floatMinValue = (float)userMenuCacheData.intMinValue * userMenuCacheData.floatIncrement;
				userMenuCacheData.floatMaxValue = (float)userMenuCacheData.intMaxValue * userMenuCacheData.floatIncrement;
				userMenuCacheData.floatData = (float)userMenuCacheData.numLongData * userMenuCacheData.floatIncrement;
				
				// The following code will check sensor type and sensitivity to auto adjust the accuracy being printed to the screen
				if ((factory_setup_rec.sensor_type == SENSOR_20_IN) || 
					((factory_setup_rec.sensor_type == SENSOR_ACC) && (trig_rec.srec.sensitivity == LOW)) ||
					((factory_setup_rec.sensor_type == SENSOR_10_IN) && (trig_rec.srec.sensitivity == LOW)))
				{
					sprintf(userMenuCachePtr[INTEGER_RANGE].text, "(%.3f-%.3f%s,N)",
						userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
					sprintf(userMenuCachePtr[INTEGER_RANGE+1].text, "(+/- %.3f%s)",
						userMenuCacheData.floatIncrement, userMenuCacheData.unitText);

					if (userMenuCacheData.numLongData == NO_TRIGGER_CHAR)
						sprintf(userMenuCachePtr[tempRow].text, "N");
					else
						sprintf(userMenuCachePtr[tempRow].text, "%.3f", userMenuCacheData.floatData);
				}
				else if (((factory_setup_rec.sensor_type == SENSOR_10_IN) && (trig_rec.srec.sensitivity == HIGH)) ||
						((factory_setup_rec.sensor_type == SENSOR_ACC) && (trig_rec.srec.sensitivity == HIGH)) ||
						((factory_setup_rec.sensor_type == SENSOR_5_IN) && (trig_rec.srec.sensitivity == LOW)))
				{
					sprintf(userMenuCachePtr[INTEGER_RANGE].text, "(%.4f-%.3f%s,N)",
						userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
					sprintf(userMenuCachePtr[INTEGER_RANGE+1].text, "(+/- %.4f%s)",
						userMenuCacheData.floatIncrement, userMenuCacheData.unitText);

					if (userMenuCacheData.numLongData == NO_TRIGGER_CHAR)
						sprintf(userMenuCachePtr[tempRow].text, "N");
					else
						sprintf(userMenuCachePtr[tempRow].text, "%.4f", userMenuCacheData.floatData);
				}
				else if (((factory_setup_rec.sensor_type == SENSOR_5_IN) && (trig_rec.srec.sensitivity == HIGH)) ||
						((factory_setup_rec.sensor_type == SENSOR_2_5_IN) && (trig_rec.srec.sensitivity == LOW)))
				{
					sprintf(userMenuCachePtr[INTEGER_RANGE].text, "(%.5f-%.3f%s,N)",
						userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
					sprintf(userMenuCachePtr[INTEGER_RANGE+1].text, "(+/- %.5f%s)",
						userMenuCacheData.floatIncrement, userMenuCacheData.unitText);

					if (userMenuCacheData.numLongData == NO_TRIGGER_CHAR)
						sprintf(userMenuCachePtr[tempRow].text, "N");
					else
						sprintf(userMenuCachePtr[tempRow].text, "%.5f", userMenuCacheData.floatData);
				}
				else // ((factory_setup_rec.sensor_type == SENSOR_2_5_IN) && (trig_rec.srec.sensitivity == HIGH))
				{
					if (help_rec.units_of_measure == IMPERIAL)
						sprintf(userMenuCachePtr[INTEGER_RANGE].text, "(%.6f-%.3f%s,N)",
							userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
					else
						sprintf(userMenuCachePtr[INTEGER_RANGE].text, "(%0.6f-%.3f%s,N)",
							userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
					sprintf(userMenuCachePtr[INTEGER_RANGE+1].text, "(+/- %.6f%s)",
						userMenuCacheData.floatIncrement, userMenuCacheData.unitText);

					if (userMenuCacheData.numLongData == NO_TRIGGER_CHAR)
						sprintf(userMenuCachePtr[tempRow].text, "N");
					else
						sprintf(userMenuCachePtr[tempRow].text, "%.6f", userMenuCacheData.floatData);
				}
			}
			else //(USER_MENU_TYPE(userMenuCachePtr) != INTEGER_SPECIAL_TYPE, INTEGER_COUNT_TYPE
			{
				if (USER_MENU_TYPE(userMenuCachePtr) != INTEGER_WORD_FIXED_TYPE)
				{
					// Set the specifications line
					sprintf(userMenuCachePtr[INTEGER_RANGE].text, "%s: %lu-%lu %s", getLangText(RANGE_TEXT),
						userMenuCacheData.intMinValue, userMenuCacheData.intMaxValue, userMenuCacheData.unitText);
				}
				else // USER_MENU_TYPE(userMenuCachePtr) == INTEGER_WORD_FIXED_TYPE)
				{
					// Set the specifications line
					sprintf(userMenuCachePtr[INTEGER_RANGE].text, "%s: %04lu-%04lu %s", getLangText(RANGE_TEXT),
						userMenuCacheData.intMinValue, userMenuCacheData.intMaxValue, userMenuCacheData.unitText);
				}

				// Print the data based on the formats for each type
				if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_BYTE_TYPE)
					sprintf(userMenuCachePtr[tempRow].text, "%d", userMenuCacheData.numByteData);
				else if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_WORD_TYPE)
					sprintf(userMenuCachePtr[tempRow].text, "%d", userMenuCacheData.numWordData);
				else if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_WORD_FIXED_TYPE)
					sprintf(userMenuCachePtr[tempRow].text, "%04d", userMenuCacheData.numWordData);
				else if (USER_MENU_TYPE(userMenuCachePtr) == INTEGER_LONG_TYPE)
					sprintf(userMenuCachePtr[tempRow].text, "%lu", userMenuCacheData.numLongData);
			}
		break;
		
		case FLOAT_TYPE:
		case FLOAT_SPECIAL_TYPE:
		case FLOAT_WITH_N_TYPE:
			if (USER_MENU_TYPE(userMenuCachePtr) == FLOAT_SPECIAL_TYPE)
			{
				// Set the specifications line
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "(%.2f-%.2f%s,N)",
					userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
				sprintf(userMenuCachePtr[FLOAT_RANGE+1].text, "(+/- %.3f%s)",
					userMenuCacheData.floatIncrement, userMenuCacheData.unitText);

				if (userMenuCacheData.floatData == NO_TRIGGER_CHAR)
					sprintf(userMenuCachePtr[tempRow].text, "N");
				else
					sprintf(userMenuCachePtr[tempRow].text, "%.2f", userMenuCacheData.floatData);
			}
			else if (USER_MENU_TYPE(userMenuCachePtr) == FLOAT_WITH_N_TYPE)
			{
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "%s: N,%.0f-%.0f %s", getLangText(RANGE_TEXT),
					(userMenuCacheData.floatMinValue + 1), userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);

				if (userMenuCacheData.floatData == 0.0)
					sprintf(userMenuCachePtr[tempRow].text, "N");
				else
					sprintf(userMenuCachePtr[tempRow].text, "%.0f", userMenuCacheData.floatData);
			}
			// The following auto adjusts the formats based on the incrememnt value
			else if (userMenuCacheData.floatIncrement >= (float)(1.0))
			{
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "%s: %.0f-%.0f %s", getLangText(RANGE_TEXT),
					userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
				sprintf(userMenuCachePtr[tempRow].text, "%.0f", userMenuCacheData.floatData);
			}
			else if (userMenuCacheData.floatIncrement >= (float)(0.1))
			{
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "%s: %.1f-%.1f %s", getLangText(RANGE_TEXT),
					userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
				sprintf(userMenuCachePtr[tempRow].text, "%.1f", userMenuCacheData.floatData);
			}
			else if (userMenuCacheData.floatIncrement >= (float)(0.01))
			{
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "%s: %.2f-%.2f %s", getLangText(RANGE_TEXT),
					userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
				sprintf(userMenuCachePtr[tempRow].text, "%.2f", userMenuCacheData.floatData);
			}
			else if (userMenuCacheData.floatIncrement >= (float)(0.001))
			{
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "%s: %.3f-%.3f %s", getLangText(RANGE_TEXT),
					userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
				sprintf(userMenuCachePtr[tempRow].text, "%.3f", userMenuCacheData.floatData);
			}
			else
			{
				sprintf(userMenuCachePtr[FLOAT_RANGE].text, "%s: %.4f-%.4f %s", getLangText(RANGE_TEXT),
					userMenuCacheData.floatMinValue, userMenuCacheData.floatMaxValue, userMenuCacheData.unitText);
				sprintf(userMenuCachePtr[tempRow].text, "%.4f", userMenuCacheData.floatData);
			}
		break;
		
		case DATE_TIME_TYPE:
			// To be added in the future
		break;
	}
}

/****************************************
*	Function:	findCurrentItemEntry
*	Purpose:	Return the index that matches the current data value
****************************************/
uint16 findCurrentItemEntry(uint32 item)
{
	uint16 i;
	uint16 totalMenuElements = (uint8)(USER_MENU_DISPLAY_ITEMS(userMenuCachePtr));

	for (i = 1; i < totalMenuElements; i++)
	{
		// Check if the current item matches the current index
		if (userMenuCachePtr[i].data == item)
			// Return the current entry
			return (i);
	}
	
	// Didnt find item, return default entry
	return (USER_MENU_DEFAULT_ITEM(userMenuCachePtr));
}

/****************************************
*	Function:	removeExtraSpaces
*	Purpose:	Removes trailing spaces left in the text of the user menu data cache
****************************************/
void removeExtraSpaces(void)
{
	uint16 i = (uint16)strlen(userMenuCacheData.text);

	// While the index is greater than the first position and the current position minus one is a space
	while ((i > 0) && (userMenuCacheData.text[i - 1] == ' '))
	{
		// Set the space character to be a null
		userMenuCacheData.text[i - 1] = '\0';
		i--;
	}
}
