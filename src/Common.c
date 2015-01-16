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
#include "ctype.h"
#include "usb_drv.h"

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
					//debug("Ext Charge Voltage A/D Reading: 0x%x\r\n", adVoltageReadValue);
				}				
				break;

			case BATTERY_VOLTAGE:
				{
					adc_start(&AVR32_ADC);
					adVoltageReadValue = adc_get_value(&AVR32_ADC, VBAT_CHANNEL);

					// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
					SoftUsecWait(4);
					//debug("Battery Voltage A/D Reading: 0x%x\r\n", adVoltageReadValue);
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
			//debug("Ext Charge Voltage A/D Reading: 0x%x\r\n", adVoltageReadValue);
		}
		break;

		case BATTERY_VOLTAGE:
		{
			adc_start(&AVR32_ADC);
			adVoltageReadValue = adc_get_value(&AVR32_ADC, VBAT_CHANNEL);
			adVoltageLevel = adVoltageReadValue * (REFERENCE_VOLTAGE * VOLTAGE_RATIO_BATT);

			// Need delay to prevent lockup on spin, EOC check inside adc_get_value appears not working as intended
			//SoftUsecWait(4);
			//debug("Battery Voltage A/D Reading: 0x%x\r\n", adVoltageReadValue);
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

	//debug("Ext Charge Voltage A/D Reading: 0x%x, Value: %f\r\n", adVoltageReadValue, adVoltageLevel);

#if 0 // Test
	debug("Battery Voltage: %f\r\n", GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
#endif

	if (adVoltageLevel > EXTERNAL_VOLTAGE_PRESENT)
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
		debug("Keypress command absorbed to signal unit to power on LCD\r\n");

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
		case CTRL_CMD:
			{
				debug("Handling Ctrl Sequence\r\n");
				HandleCtrlKeyCombination((char)mn_msg.data[0]);
			}
			break;

		case BACK_LIGHT_CMD:
			{
				debug("Handling Backlight Command\r\n");
				SetNextLcdBacklightState();
			}
			break;

		case KEYPRESS_MENU_CMD:
			{
				debug("Handling Keypress Command\r\n");
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
	unsigned long int countdown = (usecs << 2) + usecs;
	
	for(; countdown > 0;)
	{
		countdown--;
	}
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
	debug("Software Version: %s (Build Date & Time: %s)\r\n", g_buildVersion, g_buildDate);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "fsaccess.h"
void BuildLanguageLinkTable(uint8 languageSelection)
{
	uint16 i, currIndex;
	char* languageTablePtr;
#if 1 // Atmel fat driver
	int languageFile = -1;
#else // Port fat driver
	FL_FILE* languageFile = NULL;
#endif
	char languageFilename[50];

	memset((char*)&languageFilename[0], 0, sizeof(languageFilename));

#if 1 // Atmel fat driver
	strcpy((char*)&languageFilename[0], "A:\\Language\\");
#else // Port fat driver
	strcpy((char*)&languageFilename[0], "C:\\Language\\");
#endif

	switch (languageSelection)
	{
		case ENGLISH_LANG: languageTablePtr = englishLanguageTable;
			strcat((char*)&languageFilename[0], "English.tbl");
			break;

		case FRENCH_LANG: languageTablePtr = frenchLanguageTable;
			strcat((char*)&languageFilename[0], "French.tbl");
			break;

		case ITALIAN_LANG: languageTablePtr = italianLanguageTable;
			strcat((char*)&languageFilename[0], "Italian.tbl");
			break;

		case GERMAN_LANG: languageTablePtr = germanLanguageTable;
			strcat((char*)&languageFilename[0], "German.tbl");
			break;

		case SPANISH_LANG: languageTablePtr = spanishLanguageTable;
			strcat((char*)&languageFilename[0], "Spanish.tbl");
			break;

		default:
			languageTablePtr = englishLanguageTable;
			strcat((char*)&languageFilename[0], "English.tbl");
			g_unitConfig.languageMode = ENGLISH_LANG;
			SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
			break;
	}

	// Default to internal language tables as a backup (overwritten if a language file is found)
	g_languageLinkTable[0] = languageTablePtr;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Build language table");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

		// Attempt to find the file on the SD file system
#if 1 // Atmel fat driver
		languageFile = open((char*)&languageFilename[0], O_RDONLY);
#else // Port fat driver
		languageFile = fl_fopen((char*)&languageFilename[0], "r");
#endif

#if 1 // Atmel fat driver
		if (languageFile != -1)
#else // Port fat driver
		if (languageFile)
#endif
		{
			debug("Loading language table from file: %s, Length: %d\r\n", (char*)&languageFilename[0], fsaccess_file_get_size(languageFile));

			memset(&g_languageTable, '\0', sizeof(g_languageTable));

			if (fsaccess_file_get_size(languageFile) > LANGUAGE_TABLE_MAX_SIZE)
			{
				// Error case - Just read the maximum buffer size and pray
#if 1 // Atmel fat driver
				read(languageFile, (uint8*)&g_languageTable[0], LANGUAGE_TABLE_MAX_SIZE);
#else // Port fat driver
				fl_fread(languageFile, (uint8*)&g_languageTable[0], LANGUAGE_TABLE_MAX_SIZE);
#endif
			}
			else
			{
#if 1 // Atmel fat driver
				read(languageFile, (uint8*)&g_languageTable[0], fsaccess_file_get_size(languageFile));
#else // Port fat driver
				fl_fread(languageFile, (uint8*)&g_languageTable[0], languageFile->filelength);
#endif
			}

#if 1 // Atmel fat driver
			close(languageFile);
#else // Port fat driver
			fl_fclose(languageFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
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
		debug("Check text: <%s>discovered\r\n", getLangText(i));
	}
#endif

	debug("Language Link Table built.\r\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
extern const char default_boot_name[];
void CheckBootloaderAppPresent(void)
{
#if 1 // Atmel fat driver
	int file = -1;
#else // Port fat driver
	FL_FILE* file = NULL;
#endif

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Check bootloader available");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		strcpy((char*)g_spareBuffer, "A:\\System\\%s");
#else // Port fat driver
		strcpy((char*)g_spareBuffer, "C:\\System\\%s");
#endif

		strcat((char*)g_spareBuffer, default_boot_name);

#if 1 // Atmel fat driver
		file = open((char*)g_spareBuffer, O_RDONLY);
#else // Port fat driver
		file = fl_fopen((char*)g_spareBuffer, "r");
#endif

#if 1 // Atmel fat driver
		if (file != -1)
#else // Port fat driver
		if (file)
#endif
		{
			debug("Bootloader found and available\r\n");

#if 1 // Atmel fat driver
			close(file);
#else // Port fat driver
			fl_fclose(file);
#endif
		}

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdjustPowerSavings(void)
{
	uint32 usartRetries = USART_DEFAULT_TIMEOUT;

	// Check if the Unit Config is not valid since there's a chance it's referenced before it's loaded
	if (g_unitConfig.validationKey != 0xA5A5)
	{
		// Load the Unit Config to get the stored Power Savings setting
		GetRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
	}

	// Check if the Unit Config is valid
	if (g_unitConfig.validationKey == 0xA5A5)
	{
		switch (g_unitConfig.powerSavingsLevel)
		{
			//----------------------------------------------------------------------------
			case POWER_SAVINGS_MINIMUM:
			//----------------------------------------------------------------------------
				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x00010000;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC, USBB; Disable: PDCA, MACB
				AVR32_PM.hsbmask = 0x0000004F;

				// Leave active: USART0, USART 1, USART 3, TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 2, PDCA
				AVR32_PM.pbamask = 0x00004BFB;

				// Leave active: SMC, FLASHC, HMATRIX, USBB; Disable: SDRAMC, MACB
				AVR32_PM.pbbmask = 0x00000017;

				// Check if the USB is disabled
				if (!Is_usb_enabled())
				{
					// Enable the USB
					Usb_enable();
				}

				// Enable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
			break;

			//----------------------------------------------------------------------------
			case POWER_SAVINGS_NORMAL:
			//----------------------------------------------------------------------------
				// Wait for serial data to pushed out to prevent the driver from lagging the system
				while (((AVR32_USART0.csr & AVR32_USART_CSR_TXRDY_MASK) == 0) && usartRetries)
				{
					usartRetries--;
				}

				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x00010000;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC, USBB; Disable: PDCA, MACB
				AVR32_PM.hsbmask = 0x0000004F;

				// Leave active: USART 1, USART 3, TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 2, PDCA
				AVR32_PM.pbamask = 0x00004AFB;

				// Leave active: SMC, FLASHC, HMATRIX, USBB; Disable: SDRAMC, MACB
				AVR32_PM.pbbmask = 0x00000017;

				// Check if the USB is disabled
				if (!Is_usb_enabled())
				{
					// Enable the USB
					Usb_enable();
				}

				// Enable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
			break;

			//----------------------------------------------------------------------------
			case POWER_SAVINGS_MOST:
			//----------------------------------------------------------------------------
				// Check if the USB is enabled
				if (Is_usb_enabled())
				{
					// Disable the USB
					Usb_disable();
				}

				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x00010000;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC; Disable: PDCA, MACB, USBB
				AVR32_PM.hsbmask = 0x00000047;

				// Leave active: USART1, TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 2 & 3, PDCA
				AVR32_PM.pbamask = 0x000042FB;

				// Leave active: SMC, FLASHC, HMATRIX; Disable: SDRAMC, MACB, USBB
				AVR32_PM.pbbmask = 0x00000015;

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

				// Check if the USB is enabled
				if (Is_usb_enabled())
				{
					// Disable the USB
					Usb_disable();
				}

				// Leave active: SYSTIMER; Disable: OCD
				AVR32_PM.cpumask = 0x00010000;

				// Leave active: EBI, PBA & PBB BRIDGE, FLASHC; Disable: PDCA, MACB, USBB
				AVR32_PM.hsbmask = 0x0000004F; //0x00000047;

				// Leave active: TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 1 & 2 & 3, PDCA
				AVR32_PM.pbamask = 0x000040FB;

				// Leave active: SMC, FLASHC, HMATRIX; Disable: SDRAMC, MACB, USBB
				AVR32_PM.pbbmask = 0x00000017; //0x00000015;

				// Disable rs232 driver and receiver (Active low control)
				PowerControl(SERIAL_232_DRIVER_ENABLE, OFF);
				PowerControl(SERIAL_232_RECEIVER_ENABLE, OFF);
			break;

			//----------------------------------------------------------------------------
			default: // POWER_SAVINGS_NONE
			//----------------------------------------------------------------------------
				// Leave active: All; Disable: None
				AVR32_PM.cpumask = 0x00010002;

				// Leave active: All; Disable: None
				AVR32_PM.hsbmask = 0x0000007F;

				// Leave active: All; Disable: None
				AVR32_PM.pbamask = 0x0000FFFF;

				// Leave active: All; Disable: None
				AVR32_PM.pbbmask = 0x0000003F;

				// Check if the USB is disabled
				if (!Is_usb_enabled())
				{
					// Enable the USB
					Usb_enable();
				}

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

		memset(buff, 0, bufSize);
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

	debug("RTC: Current Date: %s %02d, %4d\r\n", g_monthTable[time.month].name,	time.day, (time.year + 2000));
	debug("RTC: Current Time: %02d:%02d:%02d %s\r\n", ((time.hour > 12) ? (time.hour - 12) : time.hour),
			time.min, time.sec, ((time.hour > 12) ? "PM" : "AM"));
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ReportFileSystemAccessProblem(char* attemptedProcess)
{
	uint16 i = 0;

	debugErr("Unable to access file system during: %s\r\n", attemptedProcess);
	for (; i < strlen(attemptedProcess); i++) { attemptedProcess[i] = toupper(attemptedProcess[i]); }
	sprintf((char*)g_spareBuffer, "FILE SYSTEM BUSY DURING: %s", attemptedProcess);
	OverlayMessage(getLangText(ERROR_TEXT), (char*)g_spareBuffer, (2 * SOFT_SECS));
}