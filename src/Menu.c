///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Menu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:51 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Menu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Common.h"
#include "Old_Board.h"
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
static MB_CHOICE s_messageChoices[MB_TOTAL_CHOICES] =
{
	//{Num Choices,		1st/Single,	2nd Choice,	}
	//{MB_ONE_CHOICE,	"OK\0",		"\0"		},
	//{MB_TWO_CHOICES,	"YES\0",	"NO\0"		},
	//{MB_TWO_CHOICES,	"OK\0",		"CANCEL\0"	}
	{MB_ONE_CHOICE,		OK_TEXT,	NULL_TEXT	},
	{MB_TWO_CHOICES,	YES_TEXT,	NO_TEXT		},
	{MB_TWO_CHOICES,	OK_TEXT,	CANCEL_TEXT	}
	// Add new s_messageChoices entry for new choices aboove this line
};

/****************************************
*	Function:	setupMnDef
*	Purpose:
****************************************/
void setupMnDef(void)
{
	char buff[50];

    debug("Init Power on LCD...\n");

	// Turn the LCD on
	powerControl(LCD_POWER_ENABLE, ON);

    debug("Init Load Trig Rec...\n");

	// Load Trig Record 0 to get the last settings
	debug("Trigger Record: Loading stored settings into cache\n");
	getRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

    debug("Init Trig Rec Defaults...\n");

	// Check if the Default Trig Record is uninitialized
	if ((g_triggerRecord.op_mode != WAVEFORM_MODE) && (g_triggerRecord.op_mode != BARGRAPH_MODE) &&
		(g_triggerRecord.op_mode != COMBO_MODE))
	{
		debugWarn("Trigger Record: Operation Mode not set\n");
		debug("Trigger Record: Loading defaults and setting mode to Waveform\n");
		loadTrigRecordDefaults((REC_EVENT_MN_STRUCT*)&g_triggerRecord, WAVEFORM_MODE);
	}
	else
	{
		// Make sure record is marked valid
		g_triggerRecord.validRecord = YES;
	}

    debug("Init Load Help Rec...\n");

	// Load the Help Record
	getRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

    debug("Init Help Rec Defaults...\n");

	// Check if the Help Record is uninitialized
	if (g_helpRecord.encode_ln != 0xA5A5)
	{
		// Set defaults in Help Record
		debugWarn("Help record: Not found.\n");
		debug("Loading Help Menu Defaults\n");
		loadHelpRecordDefaults((REC_HELP_MN_STRUCT*)&g_helpRecord);
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}
	else
	{
		// Help Record is valid
		debug("Help record: Found.\n");

#if 0 // fix_ns8100
		// Set the baud rate to the user stored baud rate setting (initialized to 38400)
		if (g_helpRecord.baud_rate == BAUD_RATE_19200)
		{
			uart_init(19200, CRAFT_COM_PORT);
		}
		else if (g_helpRecord.baud_rate == BAUD_RATE_9600)
		{
			uart_init(9600, CRAFT_COM_PORT);
		}
#endif		
	}

    debug("Init Build Language Table...\n");

	// Build the language table based on the user's last language choice
	build_languageLinkTable(g_helpRecord.lang_mode);

    debug("Init Activate Help Rec Options...\n");

	debug("Activate help record items\n");
	// Initialize Help Record items such as contrast, timers
	activateHelpRecordOptions();

	// Wait 1/2 second for the LCD power to settle
	soft_usecWait(500 * SOFT_MSECS);

	debug("Display Splash screen\n");
	// Display the Splash screen
	displaySplashScreen();

	// Wait at least 3 seconds for the main screen to be displayed
	soft_usecWait(3 * SOFT_SECS);

	// Check if the unit is set for Mini and auto print is enabled
	if ((MINIGRAPH_UNIT) && (g_helpRecord.auto_print == YES))
	{
		// Disable Auto printing
		g_helpRecord.auto_print = NO;
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}

    debug("Init Load Factory Setup Rec...\n");

	debug("Load Factory setup record\n");
	// Load the Factory Setup Record
	getRecData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);

	// Check if the Factory Setup Record is valid
	if (!g_factorySetupRecord.invalid)
	{
		// Print the Factory Setup Record to the console
		byteSet(&buff[0], 0, sizeof(buff));
		convertTimeStampToString(buff, &g_factorySetupRecord.cal_date, REC_DATE_TIME_TYPE);

		debug("Factory Setup: Serial #: %s\n", g_factorySetupRecord.serial_num);
		debug("Factory Setup: Cal Date: %s\n", buff);
		debug("Factory Setup: Sensor Type: %d %s\n", (g_factorySetupRecord.sensor_type),
				((g_factorySetupRecord.sensor_type == SENSOR_ACC) ? "(Acc)" : ""));
		debug("Factory Setup: A-Weighting: %s\n", (g_factorySetupRecord.aweight_option == YES) ? "Enabled" : "Disabled");
	}
	else // Factory Setup Record is not found or invalid
	{
		// Warn the user
		debugWarn("Factory setup record not found.\n");
		//messageBox(getLangText(ERROR_TEXT), getLangText(FACTORY_SETUP_DATA_COULD_NOT_BE_FOUND_TEXT), MB_OK);
	}

    debug("Init Load Modem Setup Rec...\n");

	debug("Load Modem Setup record\n");
	// Load the Modem Setup Record
	getRecData(&g_modemSetupRecord, 0, REC_MODEM_SETUP_TYPE);

	// Check if the Modem Setup Record is invalid
	if (g_modemSetupRecord.invalid)
	{
		// Warn the user
		debugWarn("Modem setup record not found.\n");

		// Initialize the Modem Setup Record
		loadModemSetupRecordDefaults();

		// Save the Modem Setup Record
		saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
	}
	else
	{
		validateModemSetupParameters();
	}

    debug("Init Current Event Number...\n");

	// Init Global Unique Event Number
	initCurrentEventNumber();

    debug("Init Auto Dialout...\n");

	// Init AutoDialout
	initAutoDialout();

    debug("Init Monitor Log...\n");

	// Init Monitor Log
	initMonitorLog();

    debug("Init Flash Buffers...\n");

	// Init Flash Buffers
	InitFlashBuffs();

    debug("Init Sensor Parameters...\n");

	initSensorParameters(g_factorySetupRecord.sensor_type, (uint8)g_triggerRecord.srec.sensitivity);
}

/****************************************
*	Function:  initSensorParameters()
*	Purpose:
****************************************/
void initSensorParameters(uint16 sensor_type, uint8 sensitivity)
{
	uint16 sensorTestVal = (uint16)MAX_NORMALIZED_SENSOR;
	uint8 gainFactor = (uint8)((sensitivity == LOW) ? 2 : 4);

	// Sensor type information
	g_sensorInfoPtr->numOfChannels = NUMBER_OF_CHANNELS_DEFAULT;		// The number of channels from a sensor.
	g_sensorInfoPtr->unitsFlag = g_helpRecord.units_of_measure;			// 0 = SAE; 1 = Metric

	g_sensorInfoPtr->sensorAccuracy = SENSOR_ACCURACY_DEFAULT;		// 100, sensor values are X 100 for accuaracy.
	g_sensorInfoPtr->ADCResolution = ADC_RESOLUTION;				// Raw data Input Range, unless ADC is changed

	// Get the shift value
	g_sensorInfoPtr->shiftVal = 1;
	while( (sensorTestVal != sensor_type) && (sensorTestVal >= (uint16)MIN_NORMALIZED_SENSOR) )
	{
		sensorTestVal = (uint16)(sensorTestVal >> 1);
		g_sensorInfoPtr->shiftVal <<= 1;
	}

	g_sensorInfoPtr->sensorTypeNormalized = (float)(sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_DEFAULT);

	if((IMPERIAL_TYPE == g_helpRecord.units_of_measure) || (sensor_type == SENSOR_ACC))
	{
		g_sensorInfoPtr->measurementRatio = (float)IMPERIAL; 				// 1 = SAE; 25.4 = Metric
	}
	else
	{
		g_sensorInfoPtr->measurementRatio = (float)METRIC; 				// 1 = SAE; 25.4 = Metric
	}

	// Get the sensor type in terms of the measurement units.
	g_sensorInfoPtr->sensorTypeNormalized = (float)(g_sensorInfoPtr->sensorTypeNormalized) * (float)(g_sensorInfoPtr->measurementRatio);

	// the conversion is length(in or mm) = hexValue * (sensor scale/ADC Max Value)
	g_sensorInfoPtr->hexToLengthConversion = (float)( (float)ADC_RESOLUTION / (float)g_sensorInfoPtr->sensorTypeNormalized );

	g_sensorInfoPtr->sensorValue = (uint16)(g_factorySetupRecord.sensor_type / gainFactor); // sensor value X 100.
}

/****************************************
*	Function:  loadUserMenuTable()
*	Purpose:
****************************************/
void loadTempMenuTable(TEMP_MENU_DATA_STRUCT* currentMenu)
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

/****************************************
*	Function:	mnScroll
*	Purpose:
****************************************/
void mnScroll(char direction, char wnd_size, MN_LAYOUT_STRUCT* mn_layout_ptr)
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

/****************************************
*	Function:	userMenuScroll
*	Purpose:
****************************************/
void userMenuScroll(uint32 direction, char wnd_size, MN_LAYOUT_STRUCT* mn_layout_ptr)
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

/****************************************
*	Function:	 dsplySelMn
*	Purpose:
****************************************/
void dsplySelMn(WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr, uint8 titlePosition)
{
   uint8 buff[50];
   uint8 top;
   uint8 menu_ln;
   uint32 length;

   byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

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

   wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

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
	            wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_LN);
			}
			else
			{
				wnd_layout_ptr->index = mn_layout_ptr->sub_ln;
	            wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_CHAR);
			}
         }
         else
         {
            wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
         }
      }
      else
         break;

      wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
      menu_ln++;
   }/* END OF WHILE LOOP */
}

/****************************************
*	Function:	 displayUserMenu
*	Purpose:
****************************************/
void displayUserMenu(WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr, uint8 titlePosition)
{
	uint8 buff[50]; /* made it bigger then NUM_CHAR_PER_LN just in case someone trys to make a big string.*/
	uint16 top;
	uint8 menu_ln;
	uint32 length;

	// Clear out LCD map buffer
	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

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
	wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

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
		            wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_LN);
				}
				else
				{
					// Write just one char highlighted
					wnd_layout_ptr->index = mn_layout_ptr->sub_ln;
		            wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, CURSOR_CHAR);
				}
			}
			else // Write text as a regular line
			{
				wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
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

/****************************************
*	Function:   wndMpWrtString
*	Purpose:
*
*	ProtoType:	void wndMpWrtString(char* buff,
*                                  Wnd_Layout_Struct *wnd_layout,
*                                  int32 font_type,
*                                  int32 ln_type)
*	Input:
*	Output:
****************************************/
void wndMpWrtString(uint8* buff, WND_LAYOUT_STRUCT* wnd_layout, int font_type, int ln_type)
{
   const uint8 (*fmap_ptr)[FONT_MAX_COL_SIZE];
   uint8 mmcurr_row;
   uint8 mmend_row;
   uint8 mmcurr_col;
   uint8 mmend_col;
   uint8 cbit_size;
   uint8 crow_size;
   uint8 ccol_size;
   uint8 temp1;
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

//=============================================================================
// Function:	messageBorder
// Purpose:		create a border for a MessageBox
//=============================================================================
void messageBorder(void)
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

//=============================================================================
// Function:	messageTitle
// Purpose:		write the title string into the MessageBox (highlighted)
//=============================================================================
void messageTitle(char* titleString)
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

//=============================================================================
// Function:	messageText
// Purpose:		write the message text string into the MessageBox
//=============================================================================
void messageText(char* textString)
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

//=============================================================================
// Function:	messageChoice
// Purpose:		write the choice/choices into the MessageBox
//=============================================================================
void messageChoice(MB_CHOICE_TYPE choiceType)
{
	uint8 i = 0, j = 0;
	uint8 text1Position = 0, text2Position = 0, startPosition = 0;
	uint8 startRow = 0;
	char firstChoiceText[18];
	char secondChoiceText[18];

	strcpy((char*)firstChoiceText, getLangText(s_messageChoices[choiceType].firstTextEntry));
	strcpy((char*)secondChoiceText, getLangText(s_messageChoices[choiceType].secondTextEntry));

	// 64 = half screen, char len * 3 = char width*6(pixel width)/2(half)
	text1Position = (uint8)(64 - strlen((char*)firstChoiceText) * 3);
	text2Position = (uint8)(64 - strlen((char*)secondChoiceText) * 3);

	// Find starting pixel position with extra char space, 6 = extra char space in pixel width
	startPosition = (uint8)((text1Position < text2Position ? text1Position : text2Position) - 6);

	if (s_messageChoices[choiceType].numChoices == MB_ONE_CHOICE)
		startRow = 6;
	else // s_messageChoices[choiceType].numChoices == MB_TWO_CHOICES
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

	if (s_messageChoices[choiceType].numChoices == MB_TWO_CHOICES)
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

//=============================================================================
// Function:	messageChoiceActiveSwap
// Purpose:		swap the active (highlighted) choices in the MessageBox
//=============================================================================
void messageChoiceActiveSwap(MB_CHOICE_TYPE choiceType)
{
	uint8 i = 0;
	uint8 text1Position = 0, text2Position = 0, startPosition = 0;
	char firstChoiceText[18];
	char secondChoiceText[18];

	strcpy((char*)firstChoiceText, getLangText(s_messageChoices[choiceType].firstTextEntry));
	strcpy((char*)secondChoiceText, getLangText(s_messageChoices[choiceType].secondTextEntry));

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

//=============================================================================
// Function:	messageBox
// Purpose:		provide a generic MessageBox with text and choices
//=============================================================================
uint8 messageBoxActiveFlag = NO;
uint8 messageBox(char* titleString, char* textString, MB_CHOICE_TYPE choiceType)
{
	uint8 activeChoice = MB_FIRST_CHOICE;
	volatile uint8 key = 0;

	// fix_ns8100
	// Temp flag for key processing from serial port
	//messageBoxActiveFlag = YES;
	// End of temp code

	// Build MessageBox into g_mmap with the following calls
	messageBorder();
	messageTitle(titleString);
	messageText(textString);
	messageChoice(choiceType);

	writeMapToLcd(g_mmap);

	// Loop forever unitl an enter or escape key is found
	while ((key != ENTER_KEY) && (key != ESC_KEY))
	{
		// Blocking call to wait for a key to be pressed on the keypad
		key = getKeypadKey(WAIT_FOR_KEY);

		// Check if there are two choices
		if (s_messageChoices[choiceType].numChoices == MB_TWO_CHOICES)
		{
			switch (key)
			{
				case UP_ARROW_KEY:
					// Check if the active choice is the second/bottom choice
					if (activeChoice == MB_SECOND_CHOICE)
					{
						// Swap the active choice
						messageChoiceActiveSwap(choiceType);
						writeMapToLcd(g_mmap);

						activeChoice = MB_FIRST_CHOICE;
					}
					break;
				case DOWN_ARROW_KEY:
					// Check if the active choice is the first/top choice
					if (activeChoice == MB_FIRST_CHOICE)
					{
						// Swap the active choice
						messageChoiceActiveSwap(choiceType);
						writeMapToLcd(g_mmap);

						activeChoice = MB_SECOND_CHOICE;
					}
					break;
			}
		}
	}

	// Clear LCD map buffer to remove message from showing up
	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));
	writeMapToLcd(g_mmap);

	// Temp flag for key processing from serial port
	messageBoxActiveFlag = NO;
	// End of temp code

	if (key == ENTER_KEY)
		return (activeChoice);
	else // key == ESC_KEY (escape)
		return (MB_NO_ACTION);
}

//=============================================================================
// Function:	overlayMessage
// Purpose:		provide a generic overlay message with variable display time
//=============================================================================
void overlayMessage(char* titleString, char* textString, uint32 displayTime)
{
	messageBorder();
	messageTitle(titleString);
	messageText(textString);

	writeMapToLcd(g_mmap);
	soft_usecWait(displayTime);
}

//=============================================================================
// Function:	updateModeMenuTitle
// Purpose:		Update the Mode menu title with the appropriate mode string
//=============================================================================
void updateModeMenuTitle(uint8 mode)
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

/****************************************
*	Function:	displaySplashScreen
*	Purpose:
****************************************/
uint8 testg_mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];

void displaySplashScreen(void)
{
	WND_LAYOUT_STRUCT wnd_layout;
	uint8 buff[50];
	uint8 length;

	wnd_layout.end_row = DEFAULT_END_ROW;
	wnd_layout.end_col = DEFAULT_END_COL;

	// Clear cached LCD memory map
	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	//----------------------------------------------------------------------------------------
	// Add in a title for the menu
	//----------------------------------------------------------------------------------------
	byteSet(&buff[0], 0, sizeof(buff));

	if (SUPERGRAPH_UNIT)
	{
		sprintf((char*)(&buff[0]), "%s", "SUPERGRAPH");
	}
	else // Minigraph
	{
		sprintf((char*)(&buff[0]), "%s", "NOMIS 8100 GRAPH");
	}
	length = (uint8)strlen((char*)(&buff[0]));

	wnd_layout.curr_row = DEFAULT_MENU_ROW_ONE;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

	//----------------------------------------------------------------------------------------
	// Add in Software Version
	//----------------------------------------------------------------------------------------
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)(&buff[0]), "%s %s", getLangText(SOFTWARE_VER_TEXT), g_appVersion);
	length = (uint8)strlen((char*)(&buff[0]));

	wnd_layout.curr_row = DEFAULT_MENU_ROW_THREE;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

	//----------------------------------------------------------------------------------------
	// Add in Software Date and Time
	//----------------------------------------------------------------------------------------
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)(&buff[0]), "%s", g_appDate);
	length = (uint8)strlen((char*)buff);

	wnd_layout.curr_row = DEFAULT_MENU_ROW_FOUR;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

	//----------------------------------------------------------------------------------------
	// Add in Battery Voltage
	//----------------------------------------------------------------------------------------
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)(&buff[0]), "%s: %.2f", getLangText(BATT_VOLTAGE_TEXT), convertedBatteryLevel(BATTERY_VOLTAGE));
	length = (uint8)strlen((char*)(&buff[0]));

	wnd_layout.curr_row = DEFAULT_MENU_ROW_SIX;
	wnd_layout.curr_col = (uint16)(((wnd_layout.end_col)/2) - ((length * SIX_COL_SIZE)/2));
	wndMpWrtString(&buff[0], &wnd_layout, SIX_BY_EIGHT_FONT, REG_LN);

    debug("Init Write Splash Screen to LCD...\n");

	// Write the map to the LCD
	writeMapToLcd(g_mmap);
}

//=============================================================================
// Function:	displayCalDate
// Purpose:
//=============================================================================
void displayCalDate(void)
{
	char dateString[35];
	char mesage[75];

	if (!g_factorySetupRecord.invalid)
	{
		byteSet(&dateString[0], 0, sizeof(dateString));
		byteSet(&mesage[0], 0, sizeof(mesage));
		convertTimeStampToString(dateString, &g_factorySetupRecord.cal_date, REC_DATE_TIME_TYPE);

		sprintf((char*)mesage, "%s: %s", getLangText(CALIBRATION_DATE_TEXT), (char*)dateString);
		messageBox(getLangText(STATUS_TEXT), (char*)mesage, MB_OK);
	}
	else
	{
		messageBox(getLangText(STATUS_TEXT), getLangText(CALIBRATION_DATE_NOT_SET_TEXT), MB_OK);
	}
}

//=============================================================================
// Function:	displaySensorType
// Purpose:
//=============================================================================
void displaySensorType(void)
{
	uint16 sensorType = NULL_TEXT;
	char message[75];

	if (!g_factorySetupRecord.invalid)
	{
		byteSet(&message[0], 0, sizeof(message));
		switch (g_factorySetupRecord.sensor_type)
		{
			case SENSOR_20_IN	: sensorType = X1_20_IPS_TEXT; break;
			case SENSOR_10_IN	: sensorType = X2_10_IPS_TEXT; break;
			case SENSOR_5_IN	: sensorType = X4_5_IPS_TEXT; break;
			case SENSOR_2_5_IN	: sensorType = X8_2_5_IPS_TEXT; break;
			case SENSOR_ACC		: sensorType = ACC_793L_TEXT; break;
		}

		sprintf((char*)message, "%s: %s", getLangText(SENSOR_GAIN_TYPE_TEXT), getLangText(sensorType));
		messageBox(getLangText(STATUS_TEXT), (char*)message, MB_OK);
	}
	else
	{
		messageBox(getLangText(STATUS_TEXT), getLangText(SENSOR_GAIN_TYPE_NOT_SET_TEXT), MB_OK);
	}
}

//=============================================================================
// Function:	displaySerialNumber
// Purpose:
//=============================================================================
void displaySerialNumber(void)
{
	char message[75];

	if (!g_factorySetupRecord.invalid)
	{
		byteSet(&message[0], 0, sizeof(message));
		sprintf((char*)message, "%s: %s", getLangText(SERIAL_NUMBER_TEXT), (char*)g_factorySetupRecord.serial_num);
		messageBox(getLangText(STATUS_TEXT), (char*)message, MB_OK);
	}
	else
	{
		messageBox(getLangText(STATUS_TEXT), getLangText(SERIAL_NUMBER_NOT_SET_TEXT), MB_OK);
	}
}

//=============================================================================
// Function:	displayTimerModeSettings
// Purpose:
//=============================================================================
void displayTimerModeSettings(void)
{
	char message[75];
	char activeTime[15];
	char activeDates[25];
	uint16 activeModeTextType;

	byteSet(&message[0], 0, sizeof(message));
	byteSet(&activeTime[0], 0, sizeof(activeTime));
	byteSet(&activeDates[0], 0, sizeof(activeDates));

	sprintf((char*)activeTime, "%02d:%02d -> %02d:%02d", g_helpRecord.tm_start_time.hour, g_helpRecord.tm_start_time.min,
			g_helpRecord.tm_stop_time.hour, g_helpRecord.tm_stop_time.min);

	switch (g_helpRecord.timer_mode_freq)
	{
		case TIMER_MODE_ONE_TIME: 	activeModeTextType = ONE_TIME_TEXT; 		break;
		case TIMER_MODE_DAILY: 		activeModeTextType = DAILY_EVERY_DAY_TEXT; 	break;
		case TIMER_MODE_WEEKDAYS: 	activeModeTextType = DAILY_WEEKDAYS_TEXT; 	break;
		case TIMER_MODE_WEEKLY: 	activeModeTextType = WEEKLY_TEXT; 			break;
		case TIMER_MODE_MONTHLY: 	activeModeTextType = MONTHLY_TEXT; 			break;
	}

	sprintf((char*)activeDates, "%02d-%s-%02d -> %02d-%s-%02d", g_helpRecord.tm_start_date.day,
			(char*)&(g_monthTable[(uint8)(g_helpRecord.tm_start_date.month)].name[0]), g_helpRecord.tm_start_date.year,
			g_helpRecord.tm_stop_date.day, (char*)&(g_monthTable[(uint8)(g_helpRecord.tm_stop_date.month)].name[0]),
			g_helpRecord.tm_stop_date.year);

	// Display SAVED SETTINGS, ACTIVE TIME PERIOD HH:MM -> HH:MM
	sprintf((char*)message, "%s, %s: %s", getLangText(SAVED_SETTINGS_TEXT),
			getLangText(ACTIVE_TIME_PERIOD_TEXT), activeTime);
	messageBox(getLangText(TIMER_MODE_TEXT), (char*)message, MB_OK);

	// Display MODE, HH:MM -> HH:MM
	sprintf((char*)message, "%s, %s", getLangText(activeModeTextType), activeDates);
	messageBox(getLangText(TIMER_MODE_TEXT), (char*)message, MB_OK);
}

//=============================================================================
// Function:	displayFlashUsageStats
// Purpose:
//=============================================================================
void displayFlashUsageStats(void)
{
	FLASH_USAGE_STRUCT usage;
	char message[75];
	char sizeUsedStr[30];
	char sizeFreeStr[30];

	getFlashUsageStats(&usage);

	if (usage.sizeUsed < 1000)
		sprintf(&sizeUsedStr[0], "USED: %3.1fKB %d%%", ((float)usage.sizeUsed / (float)1000), usage.percentUsed);
	else if (usage.sizeUsed < 1000000)
		sprintf(&sizeUsedStr[0], "USED: %3dKB %d%%", (int)(usage.sizeUsed / 1000), usage.percentUsed);
	else
		sprintf(&sizeUsedStr[0], "USED: %3.1fMB %d%%", ((float)usage.sizeUsed / (float)1000000), usage.percentUsed);

	if (usage.sizeFree < 1000)
		sprintf(&sizeFreeStr[0], "FREE: %3.1fKB %d%%", ((float)usage.sizeFree / (float)1000), usage.percentFree);
	else if (usage.sizeFree < 1000000)
		sprintf(&sizeFreeStr[0], "FREE: %3dKB %d%%", (int)(usage.sizeFree / 1000), usage.percentFree);
	else
		sprintf(&sizeFreeStr[0], "FREE: %3.1fMB %d%%", ((float)usage.sizeFree / (float)1000000), usage.percentFree);

	sprintf(&message[0], "EVENT DATA       %s %s WRAPPED: %s", sizeUsedStr, sizeFreeStr, (usage.wrapped == YES) ? "YES" : "NO");

	messageBox("FLASH USAGE STATS", (char*)message, MB_OK);

	if (g_helpRecord.flash_wrapping == NO)
		sprintf(&message[0], "SPACE REMAINING (CURR. SETTINGS) WAVEFORMS: %d, BAR HOURS: ~%d",
				usage.waveEventsLeft, usage.barHoursLeft);
	else // Wrapping is on
		sprintf(&message[0], "BEFORE OVERWRITE (CURR. SETTINGS) WAVEFORMS: %d, BAR HOURS: ~%d",
				usage.waveEventsLeft, usage.barHoursLeft);

	messageBox("FLASH USAGE STATS", (char*)message, MB_OK);
}

//=============================================================================
// Function:	displayAutoDialInfo
// Purpose:
//=============================================================================
void displayAutoDialInfo(void)
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

	sprintf(&message[0], "LAST DL EVT: %d, LAST REC: %d, LAST CONNECT: %s",
			__autoDialoutTbl.lastDownloadedEvent, getLastStoredEventNumber(), dateStr);

	messageBox("AUTO DIALOUT INFO", (char*)message, MB_OK);
}
