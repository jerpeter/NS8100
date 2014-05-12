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
#include <stdlib.h>
#include <math.h>
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
#include "adc.h"
#include "FAT32_FileLib.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define N_BITS 32
#define MAX_BIT ((N_BITS + 1) / 2 - 1)

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern const char applicationVersion[];
extern const char applicationDate[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static INPUT_MSG_STRUCT* s_inputWritePtr = &(g_input_buffer[0]);
static INPUT_MSG_STRUCT* s_inputReadPtr = &(g_input_buffer[0]);
static void (*s_bootloader)(void) = NULL;

///----------------------------------------------------------------------------
///	Function:	getExternalVoltageLevelAveraged
///	Purpose:	Convert the raw external voltage level from the a-to-d to the actual value
///----------------------------------------------------------------------------
#define AD_VOLTAGE_READ_LOOP_COUNT	15
float getExternalVoltageLevelAveraged(uint8 type)
{
	uint32 adVoltageReadValue = 0;
	uint32 adChannelSum = 0;
	uint32 adChannelValueLow = 0xFFFF;
	uint32 adChannelValueHigh = 0;
	float adVoltageLevel = (float)0.0;
	uint32 i = 0;

	// Need to average some amount of reads
	for(i = 0; i < AD_VOLTAGE_READ_LOOP_COUNT; i++)
	{
		switch (type)
		{
			case EXT_CHARGE_VOLTAGE:
				{
					adc_start(&AVR32_ADC);
					adVoltageReadValue = adc_get_value(&AVR32_ADC, VIN_CHANNEL);
					
					// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
					soft_usecWait(4);
					//debug("Ext Charge Voltage A/D Reading: 0x%x\n", adVoltageReadValue);
				}				
				break;

			case BATTERY_VOLTAGE:
				{
					adc_start(&AVR32_ADC);
					adVoltageReadValue = adc_get_value(&AVR32_ADC, VBAT_CHANNEL);

					// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
					soft_usecWait(4);
					//debug("Battery Voltage A/D Reading: 0x%x\n", adVoltageReadValue);
				}
				break;
		}
		
		if (adVoltageReadValue < adChannelValueLow)
			adChannelValueLow = adVoltageReadValue;

		if (adVoltageReadValue > adChannelValueHigh)
			adChannelValueHigh = adVoltageReadValue;
			
		adChannelSum += adVoltageReadValue;
	}

	// Remove the lowest and highest values read
	adVoltageLevel = adChannelSum - (adChannelValueLow + adChannelValueHigh);
	
	adVoltageLevel /= (AD_VOLTAGE_READ_LOOP_COUNT - 2);
		
	// Converted A/D value / 1024 * 3.3 * RATIO
	switch (type)
	{
		case EXT_CHARGE_VOLTAGE:
			adVoltageLevel *= (REFERENCE_VOLTAGE * VOLTAGE_RATIO_EXT_CHARGE);
			break;

		case BATTERY_VOLTAGE:
			adVoltageLevel *= (REFERENCE_VOLTAGE * VOLTAGE_RATIO_BATT);
			break;
	}
	
	adVoltageLevel /= BATT_RESOLUTION;

	return (adVoltageLevel);
}

///----------------------------------------------------------------------------
///	Function:	checkExternalChargeVoltagePresent
///	Purpose:	Convert the raw external voltage level from the a-to-d to the actual value
///----------------------------------------------------------------------------
BOOLEAN checkExternalChargeVoltagePresent(void)
{
	float adVoltageLevel = (float)0.0;
	BOOLEAN	externalChargePresent = NO;
	uint32 adVoltageReadValue = 0;

	adc_start(&AVR32_ADC);

#if 1 // Normal operation
	adVoltageReadValue = adc_get_value(&AVR32_ADC, VIN_CHANNEL);
#else // Test with battery
	adVoltageReadValue = adc_get_value(&AVR32_ADC, VBAT_CHANNEL);
#endif
					
	adVoltageLevel = adVoltageReadValue * (REFERENCE_VOLTAGE * VOLTAGE_RATIO_EXT_CHARGE);
	adVoltageLevel /= BATT_RESOLUTION;

	//debug("Ext Charge Voltage A/D Reading: 0x%x, Value: %f\n", adVoltageReadValue, adVoltageLevel);

#if 0 // Test
	debug("Battery Voltage: %f\n", getExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
#endif

	if (adVoltageLevel > 5.0)
		externalChargePresent = YES;
		
	return (externalChargePresent);
}

///----------------------------------------------------------------------------
///	Function:	adjustedRawBatteryLevel
///	Purpose:	Calculate the adjusted raw battery from the min level
///----------------------------------------------------------------------------
uint8 adjustedRawBatteryLevel(void)
{
	uint8 adjustedRawBattLevel = 0;
#if 0 // Unused
	uint16 rawBatteryLevel = 0;
	uint16 rawMinBatteryLevel = 0;
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
#endif

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


    if (s_inputReadPtr != s_inputWritePtr)
    {

        msg_ptr->cmd = s_inputReadPtr->cmd;
        msg_ptr->length = s_inputReadPtr->length;

        for (data_index = 0; data_index < msg_ptr->length; data_index++)
        {

           msg_ptr->data[data_index] = s_inputReadPtr->data[data_index];

        }

        if (s_inputReadPtr == (g_input_buffer + (INPUT_BUFFER_SIZE - 1)))
        {
            s_inputReadPtr = g_input_buffer;
        }
        else
        {
            s_inputReadPtr++;
        }

        return (INPUT_BUFFER_NOT_EMPTY);
    }


    return (INPUT_BUFFER_EMPTY);

}

///----------------------------------------------------------------------------
///	Function:	procInputMsg
///	Purpose:
///----------------------------------------------------------------------------
void procInputMsg(INPUT_MSG_STRUCT mn_msg)
{
	keypressEventMgr();

	// Check if the LCD power is off
	if (getSystemEventState(UPDATE_MENU_EVENT))
	{
		debug("Keypress command absorbed to signal unit to power on LCD\n");

		// Clear out the message parameters
		mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;

		// Clear the flag
		clearSystemEventFlag(UPDATE_MENU_EVENT);

		// Recall the current active menu
		JUMP_TO_ACTIVE_MENU();

		// Done processing, return
		return;
	}

	switch (mn_msg.cmd)
	{
#if 0 // ns7100
		case PAPER_FEED_CMD:
			debug("Handling Paperfeed Command\n");
			break;
#endif
		case CTRL_CMD:
			{
				debug("Handling Ctrl Sequence\n");
				handleCtrlKeyCombination((char)mn_msg.data[0]);
			}
			break;

		case BACK_LIGHT_CMD:
			{
				debug("Handling Backlight Command\n");
				setNextLcdBacklightState();
			}
			break;

		case KEYPRESS_MENU_CMD:
			{
				debug("Handling Keypress Command\n");
				JUMP_TO_ACTIVE_MENU();
			}				
			break;

		case POWER_OFF_CMD:
			{
				PowerUnitOff(SHUTDOWN_UNIT);
			}			
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

    if (s_inputWritePtr != (g_input_buffer + (INPUT_BUFFER_SIZE - 1)))
    {
        if ((s_inputWritePtr + 1) != s_inputReadPtr)
        {
            s_inputWritePtr->cmd = msg_ptr->cmd;
            s_inputWritePtr->length = msg_ptr->length;
            for (data_index = 0; data_index < msg_ptr->length; data_index++)
            {
                s_inputWritePtr->data[data_index] = msg_ptr->data[data_index];
            }

            s_inputWritePtr++;
        }
        else
        {
            return (FAILED);
        }
    }
    else
    {
        if (s_inputReadPtr != g_input_buffer)
        {
            s_inputWritePtr->cmd = msg_ptr->cmd;
            s_inputWritePtr->length = msg_ptr->length;

            for (data_index = 0; data_index < msg_ptr->length; data_index++)
            {
                s_inputWritePtr->data[data_index] = msg_ptr->data[data_index];
            }

            s_inputWritePtr = g_input_buffer;
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
#if 0 // ns7100
	volatile uint32 i = 0;

	// The system clock devided 16M is approx. a microsecond if running code within processor
	for (i = 0; i < (uint32)(usecs * (SYSCLK / 16000000)); i++) {}

	// The system clock devided 6M is approx. a microsecond if running code outside processor with 2 wait states
	//for (i = 0; i < (uint32)(usecs * (SYSCLK / 6000000)); i++) {}

#else // ns8100

	unsigned long int countdown = (usecs << 2) + usecs;
	
	for(; countdown > 0;)
	{
		countdown--;
	}

#endif
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
float hexToDB(uint16 data, uint8 dataNormalizedFlag, uint16 bitAccuracyMidpoint)
{
	float tempValue;

	tempValue =  hexToMillBars(data, dataNormalizedFlag, bitAccuracyMidpoint) * (float)DB_CONVERSION_VALUE;

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
float hexToMillBars(uint16 data, uint8 dataNormalizedFlag, uint16 bitAccuracyMidpoint)
{
	float millibars;

	if (dataNormalizedFlag == DATA_NOT_NORMALIZED)
	{
		if (data >= bitAccuracyMidpoint)
		{
			data = (uint16)(data - bitAccuracyMidpoint);
		}
		else
		{
			data = (uint16)(bitAccuracyMidpoint - data);
		}
	}

	millibars = (float)((float)(data * 25)/(float)10000.0);
	
#if 1 // ns8100 - Scale appropriate to bit accuracy based on the original calc for 12-bit
	millibars /= (float)((float)bitAccuracyMidpoint / (float)ACCURACY_12_BIT_MIDPOINT);
#endif

	return (millibars);
}

///----------------------------------------------------------------------------
///	Function:	hexToPsi
///	Purpose:	psi = mb * 14.70/1013.25, 1 atmosphere = 14.7 psi = 1013.25 mb
///----------------------------------------------------------------------------
float hexToPsi(uint16 data, uint8 dataNormalizedFlag, uint16 bitAccuracyMidpoint)
{
	float psi;

	psi =  hexToMillBars(data, dataNormalizedFlag, bitAccuracyMidpoint);

	psi = (float)((psi * (float)14.7)/(float)1013.25);

	return (psi);
}

///----------------------------------------------------------------------------
///	Function:	dbToHex
///	Purpose:	Convert the db value to a raw hex number between 0 and 2048
///----------------------------------------------------------------------------
uint16 dbToHex(uint16 db)
{
	// This is the inverse log of base 10.
	double dbValue = (double)pow((double)10, ((double)db/(double)20.0));

	// Do the conversion. 1/.0000002 = 5000000
	dbValue = (double)dbValue / (double)DB_CONVERSION_VALUE;

	// Do the conversion. millibar conversion 400 = 10000/25
	dbValue = (double)dbValue * (double)400.0;

	return (ceil(dbValue));
}

#if 1 // Updated (Port lost change)
/****************************************
*  Function:   mbToHex
*  Purpose:
****************************************/
float mbToHex(float mb)
{
	double mbValue = (double)mb / (double)25;

	// Do the conversion. 1/.0000002 = 5000000
	mbValue = (double)mbValue * (double)MB_CONVERSION_VALUE;

	if (mbValue < (double)4.0)
	{
		mbValue = (double)4.0;
	}

	// Now adjust for the zero midpoint at 2048.
	//mbValue += 0x800;

	return( (float)mbValue );
}

/****************************************
*  Function:   convertDBtoMB
*  Purpose: Convert the db value to mb
****************************************/
uint16 convertDBtoMB(uint32 level)
{
	uint16 hexData;
	uint32 remainder;

	if(level != NO_TRIGGER_CHAR)
	{
		hexData = (uint16)dbToHex((float) level);
		level = (uint32)(10000 * hexToMillBars((uint16)hexData, DATA_NORMALIZED, g_bitAccuracyMidpoint));
		remainder = level % AIR_TRIGGER_MB_INC_VALUE;
		hexData = (uint16)(level - remainder);
	}
	return(hexData);
}

/****************************************
*  Function:   convertMBtoDB
*  Purpose: Convert the mb value to db
****************************************/
uint16 convertMBtoDB(uint32 level)
{
	uint16 hexData;
	uint16 returnData;

	if(level != NO_TRIGGER_CHAR)
	{
		hexData = (uint16)mbToHex((float) level);
		returnData = (uint16)hexToDB((uint16)hexData, DATA_NORMALIZED, g_bitAccuracyMidpoint);
	}
	return(returnData);
}
#endif

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
#if 0 // ns7100
void startPitTimer(PIT_TIMER timer)
{
	// Enable the specified PIT timer
	if (timer == KEYPAD_TIMER)
	{
		reg_PCSR1.reg |= 0x0001;
	}
	else // PIT 2 timer
	{
		reg_PCSR2.reg |= 0x0001;
	}
}
#endif

///----------------------------------------------------------------------------
///	Function:	stopPitTimer
///	Purpose:
///----------------------------------------------------------------------------
#if 0 // ns7100
void stopPitTimer(PIT_TIMER timer)
{
	// Disable the specified PIT timer
	if (timer == KEYPAD_TIMER)
	{
		reg_PCSR1.reg &= ~0x0001;
	}
	else // PIT 2 timer
	{
		reg_PCSR2.reg &= ~0x0001;
	}
}
#endif

///----------------------------------------------------------------------------
///	Function:	checkPitTimer
///	Purpose:
///----------------------------------------------------------------------------
#if 0 // ns7100
BOOLEAN checkPitTimer(PIT_TIMER timer)
{
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
}
#endif

///----------------------------------------------------------------------------
///	Function:	configPitTimer
///	Purpose:
///----------------------------------------------------------------------------
#if 0 // ns7100
void configPitTimer(PIT_TIMER timer, uint16 clockDivider, uint16 modulus)
{
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
}
#endif

///----------------------------------------------------------------------------
///	Function:	initVersionStrings
///	Purpose:
///----------------------------------------------------------------------------
#if 0 // ns7100
void initVersionStrings(void)
{
	uint8 length = 0;
	char tempDate[15];
	uint8 month = 0;

	byteSet(&g_appVersion[0], 0, sizeof(g_appVersion));
	byteSet(&g_appDate[0], 0, sizeof(g_appDate));
	byteSet(&g_appTime[0], 0, sizeof(g_appTime));
	byteSet(&tempDate[0], 0, sizeof(tempDate));

	// Scan the Date and Time into buffers
	sscanf((char*)&applicationDate, "$Date: %s %s", (char*)&tempDate, (char*)&g_appTime);

	// Copy over version number string from the app version string starting at position 11
	sprintf((char*)&g_appVersion, "%s", (char*)&(applicationVersion[11]));

	// Get the length of the version number string
	length = (uint8)strlen(g_appVersion);

	// Set the null 2 characters from the end of the string to get rid of the " $"
	g_appVersion[length-2] = '\0';

	// Copy over the Day
	g_appDate[0] = tempDate[8];
	g_appDate[1] = tempDate[9];
	g_appDate[2] = ' ';

	// Copy over the Month converted from the month number to the shortened month text
	month = (uint8)(atoi((char*)&(tempDate[5])));
	strcpy((char*)&(g_appDate[3]), (char*)g_monthTable[month].name);
	g_appDate[6] = ' ';

	// Copy over the Year
	strncpy((char*)&(g_appDate[7]), (char*)&(tempDate[0]), 4);
}
#endif

///----------------------------------------------------------------------------
///	Function:	initVersionMsg
///	Purpose:
///----------------------------------------------------------------------------
void initVersionMsg(void)
{
#if 0 // ns7100
	debugRaw("\n\n");
	debugRaw("  Software Version: %s\n", g_appVersion);
	debugRaw("  Software Date:    %s\n", g_appDate);
	debugRaw("  Software Time:    %s\n", g_appTime);
	debugRaw("\n");
#else
	debugRaw("\n\n");
	debugRaw("  Software Version    : %s\n", g_buildVersion);
	debugRaw("  Software Date & Time: %s\n", g_buildDate);
	debugRaw("\n");
#endif
}

///----------------------------------------------------------------------------
///	Function:	build_languageLinkTable
///	Purpose:	Build the language link table based on the language choosen
///----------------------------------------------------------------------------
void build_languageLinkTable(uint8 languageSelection)
{
	uint16 i, currIndex = 0;
	char* languageTablePtr;
	FL_FILE* languageFile = NULL;
	char languageFilename[50];

	switch (languageSelection)
	{
		case ENGLISH_LANG: languageTablePtr = englishLanguageTable;
			sprintf((char*)&languageFilename[0], "C:\\Language\\English.tbl");
			break;

		case FRENCH_LANG: languageTablePtr = frenchLanguageTable;
			sprintf((char*)&languageFilename[0], "C:\\Language\\French.tbl");
			break;

		case ITALIAN_LANG: languageTablePtr = italianLanguageTable;
			sprintf((char*)&languageFilename[0], "C:\\Language\\Italian.tbl");
			break;

		case GERMAN_LANG: languageTablePtr = germanLanguageTable;
			sprintf((char*)&languageFilename[0], "C:\\Language\\German.tbl");
			break;

#if 1 // Updated (Port lost change)
		case SPANISH_LANG: languageTablePtr = spanishLanguageTable;
			//sprintf((char*)&languageFilename[0], "C:\\Language\\Spanish.tbl"); // fix_ns8100
			break;
#endif

		default:
			languageTablePtr = englishLanguageTable;
			sprintf((char*)&languageFilename[0], "C:\\Language\\English.tbl");
			g_helpRecord.lang_mode = ENGLISH_LANG;
			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
			break;
	}

	// Attempt to find the file on the SD file system
	languageFile = fl_fopen((char*)&languageFilename[0], "r");
	
	if (languageFile == NULL)
	{
		debugWarn("Language file not found, loading language table from app...\n");

		// File not found, default to internal language tables
		g_languageLinkTable[0] = languageTablePtr;
	}
	else // Language file found
	{
		debug("Loading language table from file: %s, Length: %d\n", (char*)&languageFilename[0], languageFile->filelength);

		byteSet(&g_languageTable, '\0', sizeof(g_languageTable));

		if (languageFile->filelength > LANGUAGE_TABLE_MAX_SIZE)
		{
			// fix_ns8100 - Clean up error case
			// Error case - Read the maximum buffer size and pray
			fl_fread(languageFile, (uint8*)&g_languageTable[0], LANGUAGE_TABLE_MAX_SIZE);
		}
		else
		{
			fl_fread(languageFile, (uint8*)&g_languageTable[0], languageFile->filelength);
		}

		// Loop and convert all line feeds and carriage returns to nulls, and leaving the last char element as a null
		for (i = 1; i < (LANGUAGE_TABLE_MAX_SIZE - 1); i++)
		{
			if ((g_languageTable[i] == '\r') || (g_languageTable[i] == '\n'))
			{
				g_languageTable[i] = '\0';
			}
		}

		languageTablePtr = &g_languageTable[0];
	}

	g_languageLinkTable[0] = languageTablePtr;
	
	for (i = 1; i < TOTAL_TEXT_STRINGS; i++)
	{
		while (languageTablePtr[currIndex++] != '\0') { /* spin */ };

		g_languageLinkTable[i] = languageTablePtr + currIndex;
	}

#if 0 // Visible check of the language table
	for (i = 0; i < TOTAL_TEXT_STRINGS; i++)
	{
		debug("Check text: <%s>\n", getLangText(i));
	}
#endif

	debug("Language Link Table built.\n");
}

///----------------------------------------------------------------------------
///	Function:	getBootFunctionAddress
///	Purpose:	Grab the address stored in the RTC that the bootloader assigned
///----------------------------------------------------------------------------
void getBootFunctionAddress(void)
{
	s_bootloader = NULL;
}

///----------------------------------------------------------------------------
///	Function:	jumpToBootFunction
///	Purpose:	Jump to the bootloader app loader routine
///----------------------------------------------------------------------------
void jumpToBootFunction(void)
{
	if (s_bootloader != NULL)
	{
		// Check if the unit is in monitor mode
		if (g_sampleProcessing == ACTIVE_STATE)
		{
			// Check if auto monitor is disabled
			if (g_helpRecord.auto_monitor_mode == AUTO_NO_TIMEOUT)
			{
				g_helpRecord.auto_monitor_mode = AUTO_TWO_MIN_TIMEOUT;

				saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
			}

			// Turn printing off
			g_helpRecord.auto_print = NO;

			stopMonitoring(g_triggerRecord.op_mode, FINISH_PROCESSING);
		}

		// Check if the LCD is currently powered
		if (getPowerControlState(LCD_POWER_ENABLE) == ON)
		{
			// Turn on the LCD power
			powerControl(LCD_POWER_ENABLE, ON);

			// Set contrast
			setLcdContrast(g_contrast_value);

			// Setup LCD segments and clear display buffer
			initLcdDisplay();

			// Turn the backlight on and set to low
			setLcdBacklightState(BACKLIGHT_DIM);
		}

		// Wait a second
		soft_usecWait(1 * SOFT_SECS);

		// Disable interrupts to prevent access to ISR's while in boot
#if 0 // ns7100
		DisableInterrupts;
#else // ns8100
		Disable_global_interrupt();
#endif
		// Jump to the bootloader routine to update the app image
		(*s_bootloader)();
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
		strcpy((char*)buff, (char*)g_monthTable[monthNum].name);
	}
}

///----------------------------------------------------------------------------
///	Function:	getDaysPerMonth
///	Purpose:
///----------------------------------------------------------------------------
uint8 getDaysPerMonth(uint8 month, uint16 year)
{
	uint8 daysInTheMonth = 0;

	daysInTheMonth = (uint8)g_monthTable[month].days;

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

	// For each month passed in the current year, add in number of days passed for the specific month
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

	// If the day of the week is Saturday, set it to the day after Friday (based on RTC week starting Sunday)
	if (dayOfWeek == 0) 
		dayOfWeek = 7;

	// Days of week in RTC is zero based, with Sunday being 0. Adjust 1's based day of the week to zero's based
	dayOfWeek--;

	return (dayOfWeek);
}

///----------------------------------------------------------------------------
///	Function:	getTotalDaysFromReference
///	Purpose:
///----------------------------------------------------------------------------
uint16 getTotalDaysFromReference(TM_DATE_STRUCT date)
{
	uint16 numOfDaysPassed = 0;

	// Reference Date: Saturday, Jan 1st, 2000
	// Dates before 01-01-2000 not meant to be calculated

	// Calculate days in years passed
	numOfDaysPassed = (uint16)((date.year) * 365);

	// Calculate leap days of years passed
	numOfDaysPassed += (date.year + 3) / 4;

	// Add in days passed of current month offset by 1 due to day 1 as a reference
	numOfDaysPassed += date.day - 1;

	// For each month passed in the current year, add in number of days passed for the specific month
	while (--date.month > 0)
	{
		switch (date.month)
		{
			case JAN: case MAR: case MAY: case JUL: case AUG: case OCT:
				numOfDaysPassed += 31;
				break;

			case APR: case JUN: case SEP: case NOV:
				numOfDaysPassed += 30;
				break;

			case FEB:
				// If this was a leap year, add a day
				if ((date.year % 4) == 0)
					numOfDaysPassed += 1;
				numOfDaysPassed += 28;
				break;
		}
	}

	return (numOfDaysPassed);
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

	debug("RTC: Current Date: %s %02d, %4d\n", g_monthTable[time.month].name,	time.day, (time.year + 2000));
	debug("RTC: Current Time: %02d:%02d:%02d %s\n", ((time.hour > 12) ? (time.hour - 12) : time.hour),
			time.min, time.sec, ((time.hour > 12) ? "PM" : "AM"));
}
