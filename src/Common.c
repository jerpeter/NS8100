///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Common.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:46 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Common.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Mmc2114.h"
#include "Common.h"
#include "Uart.h"
#include "Display.h"
#include "Menu.h"
#include "SoftTimer.h"
#include "PowerManagement.h"
#include "Keypad.h"
#include "SysEvents.h"
#include "Old_Board.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define N_BITS 32
#define MAX_BIT ((N_BITS + 1) / 2 - 1)

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern int32 active_menu;
extern SYS_EVENT_STRUCT SysEvents_flags;
extern REC_HELP_MN_STRUCT help_rec;
extern char applicationVersion[];
extern char applicationDate[];
extern uint8 contrast_value;
extern uint32 isTriggered;
extern uint32 processingCal;
extern uint16 gCalTestExpected;
extern uint8 g_doneTakingEvents;
extern uint8 g_sampleProcessing;
extern REC_EVENT_MN_STRUCT trig_rec;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
INPUT_MSG_STRUCT input_buffer[INPUT_BUFFER_SIZE];
INPUT_MSG_STRUCT *wrt_ptr = &(input_buffer[0]);
INPUT_MSG_STRUCT *rd_ptr = &(input_buffer[0]);
char appVersion[15];
char appDate[15];
char appTime[15];
void (*bootloader)(void) = NULL;

MONTH_TABLE_STRUCT monthTable[] =  {
	{0, "\0\0\0\0", 0},
	{JAN, "JAN\0", 31},
	{FEB, "FEB\0", 28},
	{MAR, "MAR\0", 31},
	{APR, "APR\0", 30},
	{MAY, "MAY\0", 31},
	{JUN, "JUN\0", 30},
	{JUL, "JUL\0", 31},
	{AUG, "AUG\0", 31},
	{SEP, "SEP\0", 30},
	{OCT, "OCT\0", 31},
	{NOV, "NOV\0", 30},
	{DEC, "DEC\0", 31}
};

///----------------------------------------------------------------------------
///	Function:	convertedBatteryLevel
///	Purpose:	Convert the raw battery level from the a-to-d to the actual value
///----------------------------------------------------------------------------
float convertedBatteryLevel(uint8 type)
{
	float rawBatteryValue = (float)0.0;
	uint16 rawBatteryLevel = 0;
	uint16 battResult1 = 0;
	uint16 battResult2 = 0;

#if 0 // fix_ns8100
	switch (type)
	{
		case EXT_CHARGE_VOLTAGE:
			battResult1 = *((uint16*)(IMM_ADDRESS + DTOA_EXT_CHARGE_RESULT_1));
			battResult2 = *((uint16*)(IMM_ADDRESS + DTOA_EXT_CHARGE_RESULT_2));
			break;

		case BATTERY_VOLTAGE:
			battResult1 = *((uint16*)(IMM_ADDRESS + DTOA_BATT_VOLT_RESULT_1));
			battResult2 = *((uint16*)(IMM_ADDRESS + DTOA_BATT_VOLT_RESULT_2));
			break;
	}
#endif

	// Take the higher of the two battery results (fix for Mcore A/D reference count bug)
	if (battResult1 > battResult2)
		rawBatteryLevel = battResult1;
	else
		rawBatteryLevel = battResult2;

	rawBatteryValue = (float)(((float)rawBatteryLevel * (float)REFERENCE_VOLTAGE) / ((float)BATT_RESOLUTION));

	switch (type)
	{
		case EXT_CHARGE_VOLTAGE:
			rawBatteryValue = (float)((float)rawBatteryValue * (float)EXT_CHARGE_RESISTOR_RATION);
			break;
		case BATTERY_VOLTAGE:
			rawBatteryValue = (float)((float)rawBatteryValue * (float)BATT_RESISTOR_RATIO);
			break;
	}

	return (rawBatteryValue);
}

///----------------------------------------------------------------------------
///	Function:	adjustedRawBatteryLevel
///	Purpose:	Calculate the adjusted raw battery from the min level
///----------------------------------------------------------------------------
uint8 adjustedRawBatteryLevel(void)
{
	uint16 rawBatteryLevel = 0;
	uint16 rawMinBatteryLevel = 0;
	uint8 adjustedRawBattLevel = 0;
	uint16 battResult1 = *((uint16*)(IMM_ADDRESS + DTOA_BATT_VOLT_RESULT_1));
	uint16 battResult2 = *((uint16*)(IMM_ADDRESS + DTOA_BATT_VOLT_RESULT_2));

	// Get the current raw battery level
	if (battResult1 > battResult2)
	{
		rawBatteryLevel = battResult1;
	}
	else
	{
		rawBatteryLevel = battResult2;
	}

	// Calculate the raw minimum battery level
	rawMinBatteryLevel = (uint16)(((BATT_MIN_VOLTS * BATT_RESOLUTION)/BATT_RESISTOR_RATIO) / REFERENCE_VOLTAGE);

	// Adjust the raw battery level with the raw minimum battery level
	// If the raw battery is less than the raw minimum, set adjusted to zero.
	if (rawBatteryLevel <= rawMinBatteryLevel)
		adjustedRawBattLevel = 0;
	// If the raw difference is greater than a one byte value (2.7 volts), set to max value stored in one byte
	else if ((rawBatteryLevel - rawMinBatteryLevel) > 0xff)
		adjustedRawBattLevel = 0xff;
	// Calculate the adjusted raw battery level
	else
		adjustedRawBattLevel = (uint8)(rawBatteryLevel - rawMinBatteryLevel);

	return (adjustedRawBattLevel);
}

///----------------------------------------------------------------------------
///	Function:	ckInputMsg
///	Purpose:
///----------------------------------------------------------------------------
uint16 ckInputMsg(INPUT_MSG_STRUCT *msg_ptr)
{
    int32 data_index;
   /*If the pointers are not equal that means a msg has been
   put into the input buffer queue. */


    if (rd_ptr != wrt_ptr)
    {

        msg_ptr->cmd = rd_ptr->cmd;
        msg_ptr->length = rd_ptr->length;

        for (data_index = 0; data_index < msg_ptr->length; data_index++)
        {

           msg_ptr->data[data_index] = rd_ptr->data[data_index];

        }

        if (rd_ptr == (input_buffer + (INPUT_BUFFER_SIZE - 1)))
        {
            rd_ptr = input_buffer;
        }
        else
        {
            rd_ptr++;
        }

        return (INPUT_BUFFER_NOT_EMPTY);
    }


    return (INPUT_BUFFER_EMPTY);

}

///----------------------------------------------------------------------------
///	Function:	procInputMsg
///	Purpose:
///----------------------------------------------------------------------------
void procInputMsg(INPUT_MSG_STRUCT msg)
{
	keypressEventMgr();

	// Check if the LCD power is off
	if (getSystemEventState(UPDATE_MENU_EVENT))
	{
		debug("Keypress command absorbed to signal unit to power on LCD\n");

		// Clear out the message parameters
		msg.cmd = 0; msg.length = 0; msg.data[0] = 0;

		// Clear the flag
		clearSystemEventFlag(UPDATE_MENU_EVENT);

		// Recall the current active menu
		(*menufunc_ptrs[active_menu])(msg);

		// Done processing, return
		return;
	}

	switch (msg.cmd)
	{
		case PAPER_FEED_CMD:
			debug("Handling Paperfeed Command\n");
			break;

		case BACK_LIGHT_CMD:
			debug("Handling Backlight Command\n");
			setNextLcdBacklightState();
			break;

		case CTRL_CMD:
			debug("Handling Ctrl Sequence\n");
			handleCtrlKeyCombination((char)msg.data[0]);
			break;

		case KEYPRESS_MENU_CMD:
			debug("Handling Keypress Command\n");
			(*menufunc_ptrs[active_menu])(msg);
			break;

		case POWER_OFF_CMD:
			PowerUnitOff(SHUTDOWN_UNIT);
			break;
	}
}

///----------------------------------------------------------------------------
///	Function:	sendInputMsg
///	Purpose:
///----------------------------------------------------------------------------
uint16 sendInputMsg(INPUT_MSG_STRUCT *msg_ptr)
{
    int32 data_index;

    if (wrt_ptr != (input_buffer + (INPUT_BUFFER_SIZE - 1)))
    {
        if ((wrt_ptr + 1) != rd_ptr)
        {
            wrt_ptr->cmd = msg_ptr->cmd;
            wrt_ptr->length = msg_ptr->length;
            for (data_index = 0; data_index < msg_ptr->length; data_index++)
            {
                wrt_ptr->data[data_index] = msg_ptr->data[data_index];
            }

            wrt_ptr++;
        }
        else
        {
            return (FAILED);
        }
    }
    else
    {
        if (rd_ptr != input_buffer)
        {
            wrt_ptr->cmd = msg_ptr->cmd;
            wrt_ptr->length = msg_ptr->length;

            for (data_index = 0; data_index < msg_ptr->length; data_index++)
            {
                wrt_ptr->data[data_index] = msg_ptr->data[data_index];
            }

            wrt_ptr = input_buffer;
        }
        else
        {
            return (FAILED);
        }
    }

    return (PASSED);
}

///----------------------------------------------------------------------------
///	Function:	soft_usecWait
///	Purpose:
///----------------------------------------------------------------------------
void soft_usecWait(uint32 usecs)
{
	volatile uint32 i = 0;

	// The system clock devided 16M is approx. a microsecond if running code within processor
	for (i = 0; i < (uint32)(usecs * (SYSCLK / 16000000)); i++) {}

	// The system clock devided 6M is approx. a microsecond if running code outside processor with 2 wait states
	//for (i = 0; i < (uint32)(usecs * (SYSCLK / 6000000)); i++) {}
}

///----------------------------------------------------------------------------
///	Function:	spinBar
///	Purpose:	Spin a bar around while waiting
///----------------------------------------------------------------------------
void spinBar(void)
{
	static uint8 BarPos = 0;

	switch (BarPos)
	{
		case 0 : case 4 : debugChar('|'); debugChar(0x8); BarPos++; return;
		case 1 : case 5 : debugChar('/'); debugChar(0x8); BarPos++; return;
		case 2 : case 6 : debugChar('-'); debugChar(0x8); BarPos++; return;
		case 3 : case 7 : debugChar('\\'); debugChar(0x8); (BarPos == 7) ? BarPos = 0 : BarPos++; return;
	}
}

///----------------------------------------------------------------------------
///	Function:	swapInt
///	Purpose:
///----------------------------------------------------------------------------
uint16 swapInt(uint16 Scr)
{
  uint16 swap1;
  uint16 swap2;

  swap1 = (uint16)((Scr >> 8) & 0x00ff);
  swap2 = (uint16)((Scr << 8) & 0xff00);

  return ((uint16)(swap1 | swap2));

}

///----------------------------------------------------------------------------
///	Function:	hexToDB
///	Purpose:	20 * log(+/- (mb/.0000002)), mb always positive
///				.0000002 = 1/5000000 = 1/DB_CONVERSION_VALUE
///				log(100) = 2 : 100 = 10**2 : 100 = 10 ** log(100)
///----------------------------------------------------------------------------
float hexToDB(uint16 data, uint8 dataNormalizedFlag)
{
	float tempValue;

	tempValue =  hexToMillBars(data, dataNormalizedFlag) * (float)DB_CONVERSION_VALUE;

	if (tempValue > 0)
	{
		tempValue = (float)log10f(tempValue) * (float)20.0;
	}

	return (tempValue);
}

///----------------------------------------------------------------------------
///	Function:	hexToMillBars
///	Purpose:	ABS((hexValue - 2048) * 25 / 10000), Where (hexValue - 2048)
///				is for the full spread between 0 - 4096
///----------------------------------------------------------------------------
float hexToMillBars(uint16 data, uint8 dataNormalizedFlag)
{
	float millibars;

	if (dataNormalizedFlag == DATA_NOT_NORMALIZED)
	{
		if (data >= 0x800)
		{
			data = (uint16)(data - 0x0800);
		}
		else
		{
			data = (uint16)(0x0800 - data);
		}
	}

	millibars = (float)((float)(data * 25)/(float)10000.0);

	return (millibars);
}

///----------------------------------------------------------------------------
///	Function:	hexToPsi
///	Purpose:	psi = mb * 14.70/1013.25, 1 atmosphere = 14.7 psi = 1013.25 mb
///----------------------------------------------------------------------------
float hexToPsi(uint16 data, uint8 dataNormalizedFlag)
{
	float psi;

	psi =  hexToMillBars(data, dataNormalizedFlag);

	psi = (float)((psi * (float)14.7)/(float)1013.25);

	return (psi);
}

///----------------------------------------------------------------------------
///	Function:	dbToHex
///	Purpose:	Convert the db value to a raw hex number between 0 and 2048
///----------------------------------------------------------------------------
float dbToHex(float db)
{
	// This is the inverse log of base 10.
	double dbValue = (double)pow((double)10, ((double)db/(double)20.0));

	// Do the conversion. 1/.0000002 = 5000000
	dbValue = (double)dbValue / (double)DB_CONVERSION_VALUE;

	// Do the conversion. millibar conversion 400 = 10000/25
	dbValue = (double)dbValue * (double)400.0;

	return ((float)dbValue);
}

///----------------------------------------------------------------------------
///	Function:	isqrt
///	Purpose:	Provide an integer square root routine that is as fast as
///				possible (~25 usecs)
///----------------------------------------------------------------------------
uint16 isqrt(uint32 x)
{
	register uint32 xroot, m2, x2;

	xroot = 0;
	m2 = 1 << MAX_BIT * 2;

	do
	{
		x2 = xroot + m2;
		xroot >>= 1;

		if (x2 <= x)
		{
			x -= x2;
			xroot += m2;
		}
	}
	while (m2 >>= 2);

	if (xroot < x)
		return ((uint16)(xroot + 1));

	return ((uint16)xroot);
}

///----------------------------------------------------------------------------
///	Function:	startPitTimer
///	Purpose:
///----------------------------------------------------------------------------
void startPitTimer(PIT_TIMER timer)
{
#if 0 // fix_ns8100
	// Enable the specified PIT timer
	if (timer == KEYPAD_TIMER)
	{
		reg_PCSR1.reg |= 0x0001;
	}
	else // PIT 2 timer
	{
		reg_PCSR2.reg |= 0x0001;
	}
#endif
}

///----------------------------------------------------------------------------
///	Function:	stopPitTimer
///	Purpose:
///----------------------------------------------------------------------------
void stopPitTimer(PIT_TIMER timer)
{
#if 0 // fix_ns8100
	// Disable the specified PIT timer
	if (timer == KEYPAD_TIMER)
	{
		reg_PCSR1.reg &= ~0x0001;
	}
	else // PIT 2 timer
	{
		reg_PCSR2.reg &= ~0x0001;
	}
#endif
}

///----------------------------------------------------------------------------
///	Function:	checkPitTimer
///	Purpose:
///----------------------------------------------------------------------------
BOOLEAN checkPitTimer(PIT_TIMER timer)
{
#if 0 // fix_ns8100
	if (timer == KEYPAD_TIMER)
	{
		if (reg_PCSR1.reg & 0x0001)
			return (ENABLED);
		else
			return (DISABLED);
	}
	else // PIT 2 timer
	{
		if (reg_PCSR2.reg & 0x0001)
			return (ENABLED);
		else
			return (DISABLED);
	}
#else
	return (DISABLED);
#endif
}

///----------------------------------------------------------------------------
///	Function:	configPitTimer
///	Purpose:
///----------------------------------------------------------------------------
void configPitTimer(PIT_TIMER timer, uint16 clockDivider, uint16 modulus)
{
#if 0 // fix_ns8100
	if (timer == KEYPAD_TIMER)
	{
		reg_PCSR1.bit.PRE = clockDivider;
		reg_PMR1 = modulus;
	}
	else // PIT 2 timer
	{
		reg_PCSR2.bit.PRE = clockDivider;
		reg_PMR2 = modulus;
	}
#endif
}

///----------------------------------------------------------------------------
///	Function:	initVersionStrings
///	Purpose:
///----------------------------------------------------------------------------
void initVersionStrings(void)
{
	uint8 length = 0;
	char tempDate[15];
	uint8 month = 0;

	byteSet(&appVersion[0], 0, sizeof(appVersion));
	byteSet(&appDate[0], 0, sizeof(appDate));
	byteSet(&appTime[0], 0, sizeof(appTime));
	byteSet(&tempDate[0], 0, sizeof(tempDate));

	// Scan the Date and Time into buffers
	sscanf((char*)&applicationDate, "$Date: %s %s", (char*)&tempDate, (char*)&appTime);

	// Copy over version number string from the app version string starting at position 11
	sprintf((char*)&appVersion, "%s", (char*)&(applicationVersion[11]));

	// Get the length of the version number string
	length = (uint8)strlen(appVersion);

	// Set the null 2 characters from the end of the string to get rid of the " $"
	appVersion[length-2] = '\0';

	// Copy over the Day
	appDate[0] = tempDate[8];
	appDate[1] = tempDate[9];
	appDate[2] = ' ';

	// Copy over the Month converted from the month number to the shortened month text
	month = (uint8)(atoi((char*)&(tempDate[5])));
	strcpy((char*)&(appDate[3]), (char*)monthTable[month].name);
	appDate[6] = ' ';

	// Copy over the Year
	strncpy((char*)&(appDate[7]), (char*)&(tempDate[0]), 4);
}

///----------------------------------------------------------------------------
///	Function:	initVersionMsg
///	Purpose:
///----------------------------------------------------------------------------
void initVersionMsg(void)
{
	debugRaw("\n\n");
	debugRaw("  Software Version: %s\n", appVersion);
	debugRaw("  Software Date:    %s\n", appDate);
	debugRaw("  Software Time:    %s\n", appTime);
	debugRaw("\n");
}

///----------------------------------------------------------------------------
///	Function:	buildLanguageLinkTable
///	Purpose:	Build the language link table based on the language choosen
///----------------------------------------------------------------------------
void buildLanguageLinkTable(uint8 languageSelection)
{
	uint16 i, currIndex = 0;
	char* languageTablePtr;

	switch (languageSelection)
	{
		case ENGLISH_LANG: languageTablePtr = englishLanguageTable;
			break;

		case FRENCH_LANG: languageTablePtr = frenchLanguageTable;
			break;

		case ITALIAN_LANG: languageTablePtr = italianLanguageTable;
			break;

		case GERMAN_LANG: languageTablePtr = germanLanguageTable;
			break;

		default:
			languageTablePtr = englishLanguageTable;
			help_rec.lang_mode = ENGLISH_LANG;
			saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
			break;
	}

	languageLinkTable[0] = languageTablePtr;

	for (i = 1; i < TOTAL_TEXT_STRINGS; i++)
	{
		while (languageTablePtr[currIndex++] != '\0') {};

		languageLinkTable[i] = languageTablePtr + currIndex;
	}

	debug("Language Link Table built.\n");
}

///----------------------------------------------------------------------------
///	Function:	getBootFunctionAddress
///	Purpose:	Grab the address stored in the RTC that the bootloader assigned
///----------------------------------------------------------------------------
void getBootFunctionAddress(void)
{
	bootloader = NULL;
}

///----------------------------------------------------------------------------
///	Function:	jumpToBootFunction
///	Purpose:	Jump to the bootloader app loader routine
///----------------------------------------------------------------------------
void jumpToBootFunction(void)
{
	if (bootloader != NULL)
	{
		// Check if the unit is in monitor mode
		if (g_sampleProcessing == SAMPLING_STATE)
		{
			// Check if auto monitor is disabled
			if (help_rec.auto_monitor_mode == AUTO_NO_TIMEOUT)
			{
				help_rec.auto_monitor_mode = AUTO_TWO_MIN_TIMEOUT;

				saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
			}

			// Turn printing off
			help_rec.auto_print = NO;

			stopMonitoring(trig_rec.op_mode, FINISH_PROCESSING);
		}

		// Check if the LCD is currently powered
		if (getPowerControlState(LCD_POWER_ENABLE) == ON)
		{
			// Turn on the LCD power
			powerControl(LCD_POWER_ENABLE, ON);

			// Set contrast
			setLcdContrast(contrast_value);

			// Setup LCD segments and clear display buffer
			initLcdDisplay();

			// Turn the backlight on and set to low
			setLcdBacklightState(BACKLIGHT_DIM);
		}

		// Wait a second
		soft_usecWait(1 * SOFT_SECS);

		// Disable interrupts to prevent access to ISR's while in boot
#if 0 // fix_ns8100
		DisableInterrupts;
#endif
		// Jump to the bootloader routine to update the app image
		(*bootloader)();
	}
}

///----------------------------------------------------------------------------
///	Function:	byteCpy
///	Purpose:	Our own memcpy
///----------------------------------------------------------------------------
void byteCpy(void* dest, void* src, uint32 size)
{
	while ((int32)size-- > 0)
	{
		((uint8*)dest)[size] = ((uint8*)src)[size];
	}
}

///----------------------------------------------------------------------------
///	Function:	byteSet
///	Purpose:	Our own memset
///----------------------------------------------------------------------------
void byteSet(void* dest, uint8 value, uint32 size)
{
	while ((int32)size-- > 0)
	{
		((uint8*)dest)[size] = value;
	}
}

///----------------------------------------------------------------------------
///	Function:	getDateString
///	Purpose:
///----------------------------------------------------------------------------
void getDateString(char* buff, uint8 monthNum, uint8 bufSize)
{
	if (bufSize > 3)
	{
		if ((monthNum < 1) || (monthNum > 12))
		{
			monthNum = 1;
		}

		byteSet(buff, 0, bufSize);
		strcpy((char*)buff, (char*)monthTable[monthNum].name);
	}
}

///----------------------------------------------------------------------------
///	Function:	getDaysPerMonth
///	Purpose:
///----------------------------------------------------------------------------
uint8 getDaysPerMonth(uint8 month, uint16 year)
{
	uint8 daysInTheMonth = 0;

	daysInTheMonth = (uint8)monthTable[month].days;

	if (month == FEB)
	{
		if ((year % 4) == 0)
		{
			// Leap year
			daysInTheMonth++;
		}
	}

	return (daysInTheMonth);
}

///----------------------------------------------------------------------------
///	Function:	getDayOfWeek
///	Purpose:
///----------------------------------------------------------------------------
uint8 getDayOfWeek(uint8 year, uint8 month, uint8 day)
{
	uint16 numOfDaysPassed = 0;
	uint8 dayOfWeek = 0;

	// Reference Date: Saturday, Jan 1st, 2000
	// Dates before 01-01-2000 not meant to be calculated

	// Calc days in years passed
	numOfDaysPassed = (uint16)((year) * 365);

	// Calc leap days of years passed
	numOfDaysPassed += (year + 3) / 4;

	// Add in days passed of current month offset by 1 due to day 1 as a reference
	numOfDaysPassed += day - 1;

	// For each month passed, add in number of days passed
	while (--month > 0)
	{
		switch (month)
		{
			case JAN: case MAR: case MAY: case JUL: case AUG: case OCT:
				numOfDaysPassed += 31;
				break;

			case APR: case JUN: case SEP: case NOV:
				numOfDaysPassed += 30;
				break;

			case FEB:
				// If this was a leap year, add a day
				if ((year % 4) == 0)
					numOfDaysPassed += 1;
				numOfDaysPassed += 28;
				break;
		}
	}

	// Mod to find day offset from reference
	dayOfWeek = (uint8)(numOfDaysPassed % 7);

	// If the day of the week is Saturday, set it to the day after Sunday (instead of the day before) due to the following calculation to convert to zero based weekdays
	if (dayOfWeek == 0) 
		dayOfWeek = 7;

	// Days of week in RTC is zero based, with Sunday being 0. Adjust 1's based day of the week to zero's based
	dayOfWeek--;

	return (dayOfWeek);
}

///----------------------------------------------------------------------------
///	Function:	initTimeMsg
///	Purpose:
///----------------------------------------------------------------------------
void initTimeMsg(void)
{
#if (GLOBAL_DEBUG_PRINT_ENABLED == ALL_DEBUG)
	//DATE_TIME_STRUCT time = getCurrentTime();
	DATE_TIME_STRUCT time = getRtcTime();
#endif

	debug("RTC: Current Date: %s %02d, %4d\n", monthTable[time.month].name,	time.day, (time.year + 2000));
	debug("RTC: Current Time: %02d:%02d:%02d %s\n", ((time.hour > 12) ? (time.hour - 12) : time.hour),
			time.min, time.sec, ((time.hour > 12) ? "PM" : "AM"));
}
