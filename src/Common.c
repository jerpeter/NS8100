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
#include "TextTypes.h"
#include "adc.h"
#include "FAT32_FileLib.h"
#include "usart.h"

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
///	Function Break
///----------------------------------------------------------------------------
#define AD_VOLTAGE_READ_LOOP_COUNT	15
float GetExternalVoltageLevelAveraged(uint8 type)
{
	uint32 adVoltageReadValue = 0;
	float adVoltageLevel = (float)0.0;

#if NS8100_ORIGINAL
	uint32 adChannelValueLow = 0xFFFF;
	uint32 adChannelValueHigh = 0;
	uint32 adChannelSum = 0;
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
					SoftUsecWait(4);
					//debug("Ext Charge Voltage A/D Reading: 0x%x\n", adVoltageReadValue);
				}				
				break;

			case BATTERY_VOLTAGE:
				{
					adc_start(&AVR32_ADC);
					adVoltageReadValue = adc_get_value(&AVR32_ADC, VBAT_CHANNEL);

					// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
					SoftUsecWait(4);
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
#else // NS8100_ALPHA
	switch (type)
	{
		case EXT_CHARGE_VOLTAGE:
		{
			adc_start(&AVR32_ADC);
			adVoltageReadValue = adc_get_value(&AVR32_ADC, VIN_CHANNEL);
			adVoltageLevel = adVoltageReadValue * (REFERENCE_VOLTAGE * VOLTAGE_RATIO_EXT_CHARGE);

			// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
			//SoftUsecWait(4);
			//debug("Ext Charge Voltage A/D Reading: 0x%x\n", adVoltageReadValue);
		}
		break;

		case BATTERY_VOLTAGE:
		{
			adc_start(&AVR32_ADC);
			adVoltageReadValue = adc_get_value(&AVR32_ADC, VBAT_CHANNEL);
			adVoltageLevel = adVoltageReadValue * (REFERENCE_VOLTAGE * VOLTAGE_RATIO_BATT);

			// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
			//SoftUsecWait(4);
			//debug("Battery Voltage A/D Reading: 0x%x\n", adVoltageReadValue);
		}
		break;
	}
#endif
	
	adVoltageLevel /= BATT_RESOLUTION;

	return (adVoltageLevel);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
BOOLEAN CheckExternalChargeVoltagePresent(void)
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
	debug("Battery Voltage: %f\n", GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
#endif

	if (adVoltageLevel > 5.0)
		externalChargePresent = YES;
		
	return (externalChargePresent);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 AdjustedRawBatteryLevel(void)
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
///	Function Break
///----------------------------------------------------------------------------
uint16 CheckInputMsg(INPUT_MSG_STRUCT *msg_ptr)
{
	uint16 data_index;

	/* If the pointers are not equal that means a msg has been
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
///	Function Break
///----------------------------------------------------------------------------
void ProcessInputMsg(INPUT_MSG_STRUCT mn_msg)
{
	KeypressEventMgr();

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
				HandleCtrlKeyCombination((char)mn_msg.data[0]);
			}
			break;

		case BACK_LIGHT_CMD:
			{
				debug("Handling Backlight Command\n");
				SetNextLcdBacklightState();
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
///	Function Break
///----------------------------------------------------------------------------
uint16 SendInputMsg(INPUT_MSG_STRUCT *msg_ptr)
{
    uint16 data_index;

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
///	Function Break
///----------------------------------------------------------------------------
void SoftUsecWait(uint32 usecs)
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
///	Function Break
///----------------------------------------------------------------------------
void SpinBar(void)
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
///	Function Break
///----------------------------------------------------------------------------
uint16 SwapInt(uint16 Scr)
{
  uint16 swap1;
  uint16 swap2;

  swap1 = (uint16)((Scr >> 8) & 0x00ff);
  swap2 = (uint16)((Scr << 8) & 0xff00);

  return ((uint16)(swap1 | swap2));

}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
float HexToDB(uint16 data, uint8 dataNormalizedFlag, uint16 bitAccuracyMidpoint)
{
	float tempValue;

	tempValue =  HexToMB(data, dataNormalizedFlag, bitAccuracyMidpoint) * (float)DB_CONVERSION_VALUE;

	if (tempValue > 0)
	{
		tempValue = (float)log10f(tempValue) * (float)20.0;
	}

	return (tempValue);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
float HexToMB(uint16 data, uint8 dataNormalizedFlag, uint16 bitAccuracyMidpoint)
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
///	Function Break
///----------------------------------------------------------------------------
float HexToPsi(uint16 data, uint8 dataNormalizedFlag, uint16 bitAccuracyMidpoint)
{
	float psi;

	psi =  HexToMB(data, dataNormalizedFlag, bitAccuracyMidpoint);

	psi = (float)((psi * (float)14.7)/(float)1013.25);

	return (psi);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 DbToHex(uint16 db)
{
	// This is the inverse log of base 10.
	double dbValue = (double)pow((double)10, ((double)db/(double)20.0));

	// Do the conversion. 1/.0000002 = 5000000
	dbValue = (double)dbValue / (double)DB_CONVERSION_VALUE;

	// Do the conversion. millibar conversion 400 = 10000/25
	dbValue = (double)dbValue * (double)MB_CONVERSION_VALUE;

	return (ceil(dbValue));
}

#if 1 // Updated (Port lost change)
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 MbToHex(float mb)
{
	// Range is from 0 to 2048. Incoming mb is adjusted up by 10,000. Divide by 25 to bring to the correct range.
	uint16 mbValue = ceil(mb / ADJUSTED_MB_TO_HEX_VALUE);

	return (mbValue);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint32 ConvertDBtoMB(uint32 level)
{
#if 0 // Poor
	uint16 hexData;
	uint32 remainder;

	if (level != NO_TRIGGER_CHAR)
	{
		hexData = DbToHex(level);
		level = (uint32)(10000 * HexToMB((uint16)hexData, DATA_NORMALIZED, g_bitAccuracyMidpoint));
		remainder = level % AIR_TRIGGER_MB_INC_VALUE;
		hexData = (uint16)(level - remainder);
	}
	
	return(hexData);
#else // Better
	double mbValue;
	uint32 mbConverted;

	if (level == NO_TRIGGER_CHAR)
	{
		return (NO_TRIGGER_CHAR);
	}
	else
	{
		// This is the inverse log of base 10.
		mbValue = (double)pow((double)10, ((double)level/(double)20.0));

		// Do the conversion divide by 5000000 and multiply by 10000 equals divide by 500
		mbValue /= (DB_CONVERSION_VALUE / (MB_CONVERSION_VALUE * ADJUSTED_MB_TO_HEX_VALUE));

		mbConverted = ceil(mbValue);
		
		if (mbConverted <= AIR_TRIGGER_MB_MIN_VALUE) { mbConverted = AIR_TRIGGER_MB_MIN_VALUE; }
		else if (mbConverted >= AIR_TRIGGER_MB_MAX_VALUE) { mbConverted = AIR_TRIGGER_MB_MAX_VALUE; }
		
		return (mbConverted);
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint32 ConvertMBtoDB(uint32 level)
{
#if 0 // Poor
	uint16 hexData;
	uint16 returnData;

	if (level != NO_TRIGGER_CHAR)
	{
		hexData = MbToHex((float) level);
		returnData = (uint16)HexToDB((uint16)hexData, DATA_NORMALIZED, g_bitAccuracyMidpoint);
	}
	
	return(returnData);
#else // Better
	double dbValue;
	uint32 dbConverted;
	
	if (level == NO_TRIGGER_CHAR)
	{
		return (NO_TRIGGER_CHAR);
	}
	else
	{
		dbValue = (double)level * (double)(DB_CONVERSION_VALUE / (MB_CONVERSION_VALUE * ADJUSTED_MB_TO_HEX_VALUE));

		dbValue = (float)log10f(dbValue) * (float)20.0;

		dbConverted = floor(dbValue);
		
		if (dbConverted <= AIR_TRIGGER_MIN_VALUE) { dbConverted = AIR_TRIGGER_MIN_VALUE; }
		else if (dbConverted >= AIR_TRIGGER_MAX_VALUE) { dbConverted = AIR_TRIGGER_MAX_VALUE; }

		return (dbConverted);
	}
#endif
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 Isqrt(uint32 x)
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
///	Function Break
///----------------------------------------------------------------------------
void InitVersionMsg(void)
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
///	Function Break
///----------------------------------------------------------------------------
void BuildLanguageLinkTable(uint8 languageSelection)
{
	uint16 i, currIndex;
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
			sprintf((char*)&languageFilename[0], "C:\\Language\\Spanish.tbl");
			break;
#endif

		default:
			languageTablePtr = englishLanguageTable;
			sprintf((char*)&languageFilename[0], "C:\\Language\\English.tbl");
			g_helpRecord.languageMode = ENGLISH_LANG;
			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
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

		ByteSet(&g_languageTable, '\0', sizeof(g_languageTable));

		if (languageFile->filelength > LANGUAGE_TABLE_MAX_SIZE)
		{
			// Error case - Just read the maximum buffer size and pray
			fl_fread(languageFile, (uint8*)&g_languageTable[0], LANGUAGE_TABLE_MAX_SIZE);
		}
		else
		{
			fl_fread(languageFile, (uint8*)&g_languageTable[0], languageFile->filelength);
		}

		fl_fclose(languageFile);
	}

	// Loop and convert all line feeds and carriage returns to nulls, and leaving the last char element as a null
	for (i = 1; i < (LANGUAGE_TABLE_MAX_SIZE - 1); i++)
	{
		// Check if a CR or LF was used as an element separator
		if ((g_languageTable[i] == '\r') || (g_languageTable[i] == '\n'))
		{
			// Convert the CR of LF to a Null
			g_languageTable[i] = '\0';

			// Check if a CR/LF or LF/CR combo was used to as the element separator
			if ((g_languageTable[i + 1] == '\r') || (g_languageTable[i + 1] == '\n'))
			{
				// Skip the second character of the combo separator
				i++;
			}
		}
	}

	languageTablePtr = &g_languageTable[0];

	// Set the first element of the link table to the start of the language table
	g_languageLinkTable[0] = languageTablePtr;
	
	// Build the language link table by pointing to the start of every string following a Null
	for (i = 1, currIndex = 0; i < TOTAL_TEXT_STRINGS; i++)
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
///	Function Break
///----------------------------------------------------------------------------
void GetBootFunctionAddress(void)
{
	s_bootloader = NULL;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void JumpToBootFunction(void)
{
	if (s_bootloader != NULL)
	{
		// Check if the unit is in monitor mode
		if (g_sampleProcessing == ACTIVE_STATE)
		{
			// Check if auto monitor is disabled
			if (g_helpRecord.autoMonitorMode == AUTO_NO_TIMEOUT)
			{
				g_helpRecord.autoMonitorMode = AUTO_TWO_MIN_TIMEOUT;

				SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
			}

			// Turn printing off
			g_helpRecord.autoPrint = NO;

			StopMonitoring(g_triggerRecord.op_mode, FINISH_PROCESSING);
		}

		// Check if the LCD is currently powered
		if (GetPowerControlState(LCD_POWER_ENABLE) == ON)
		{
			// Turn on the LCD power
			PowerControl(LCD_POWER_ENABLE, ON);

			// Set contrast
			SetLcdContrast(g_contrast_value);

			// Setup LCD segments and clear display buffer
			InitLcdDisplay();

			// Turn the backlight on and set to low
			SetLcdBacklightState(BACKLIGHT_DIM);
		}

		// Wait a second
		SoftUsecWait(1 * SOFT_SECS);

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
///	Function Break
///----------------------------------------------------------------------------
void ByteCpy(void* dest, void* src, uint32 size)
{
	while ((int32)size-- > 0)
	{
		((uint8*)dest)[size] = ((uint8*)src)[size];
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ByteSet(void* dest, uint8 value, uint32 size)
{
	while ((int32)size-- > 0)
	{
		((uint8*)dest)[size] = value;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdjustPowerSavings(void)
{
	uint32 usartRetries = USART_DEFAULT_TIMEOUT;

	// Check if the Help Record is not valid since there's a chance it's referenced before it's loaded
	if (g_helpRecord.validationKey != 0xA5A5)
	{
		// Load the Help Record to get the stored Power Savings setting
		GetRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}

	// Check if the Help Record is valid
	if (g_helpRecord.validationKey == 0xA5A5)
	{
		switch (g_helpRecord.powerSavingsLevel)
		{
			//----------------------------------------------------------------------------
			case POWER_SAVINGS_MINIMUM:
			//----------------------------------------------------------------------------
				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x0100;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC, USBB; Disable: PDCA, MACB
				AVR32_PM.hsbmask = 0x004F;

				// Leave active: USART 1, USART 3, TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 2, PDCA
				AVR32_PM.pbamask = 0x4AFB;

				// Leave active: SMC, FLASHC, HMATRIX, USBB; Disable: SDRAMC, MACB
				AVR32_PM.pbbmask = 0x0017;

				// Enable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
			break;

			//----------------------------------------------------------------------------
			case POWER_SAVINGS_MOST:
			//----------------------------------------------------------------------------
				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x0100;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC; Disable: PDCA, MACB, USBB
				AVR32_PM.hsbmask = 0x0047;

				// Leave active: USART1, TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 2 & 3, PDCA
				AVR32_PM.pbamask = 0x42FB;

				// Leave active: SMC, FLASHC, HMATRIX; Disable: SDRAMC, MACB, USBB
				AVR32_PM.pbbmask = 0x0015;

				// Enable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
			break;

			//----------------------------------------------------------------------------
			case POWER_SAVINGS_MAX:
			//----------------------------------------------------------------------------
				// Wait for serial data to pushed out to prevent the driver from lagging the system
				while (((AVR32_USART1.csr & AVR32_USART_CSR_TXRDY_MASK) == 0) && usartRetries)
				{
					usartRetries--;
				}

				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x0100;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC; Disable: PDCA, MACB, USBB
				AVR32_PM.hsbmask = 0x0047;

				// Leave active: TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 1 & 2 & 3, PDCA
				AVR32_PM.pbamask = 0x40FB;

				// Leave active: SMC, FLASHC, HMATRIX; Disable: SDRAMC, MACB, USBB
				AVR32_PM.pbbmask = 0x0015;

				// Disable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, OFF);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, OFF);
			break;

			//----------------------------------------------------------------------------
			default: // POWER_SAVINGS_NONE
			//----------------------------------------------------------------------------
				// Leave active: All; Disable: None
				AVR32_PM.cpumask = 0x0102;

				// Leave active: All; Disable: None
				AVR32_PM.hsbmask = 0x007F;

				// Leave active: All; Disable: None
				AVR32_PM.pbamask = 0xFFFF;

				// Leave active: All; Disable: None
				AVR32_PM.pbbmask = 0x003F;

				// Enable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
			break;
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void GetDateString(char* buff, uint8 monthNum, uint8 bufSize)
{
	if (bufSize > 3)
	{
		if ((monthNum < 1) || (monthNum > 12))
		{
			monthNum = 1;
		}

		ByteSet(buff, 0, bufSize);
		strcpy((char*)buff, (char*)g_monthTable[monthNum].name);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 GetDaysPerMonth(uint8 month, uint16 year)
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
///	Function:	GetDayOfWeek
///	Purpose:
///----------------------------------------------------------------------------
uint8 GetDayOfWeek(uint8 year, uint8 month, uint8 day)
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
///	Function Break
///----------------------------------------------------------------------------
uint16 GetTotalDaysFromReference(TM_DATE_STRUCT date)
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
///	Function Break
///----------------------------------------------------------------------------
void InitTimeMsg(void)
{
#if (GLOBAL_DEBUG_PRINT_ENABLED == ALL_DEBUG)
	//DATE_TIME_STRUCT time = GetCurrentTime();
	DATE_TIME_STRUCT time = GetExternalRtcTime();
#endif

	debug("RTC: Current Date: %s %02d, %4d\n", g_monthTable[time.month].name,	time.day, (time.year + 2000));
	debug("RTC: Current Time: %02d:%02d:%02d %s\n", ((time.hour > 12) ? (time.hour - 12) : time.hour),
			time.min, time.sec, ((time.hour > 12) ? "PM" : "AM"));
}
