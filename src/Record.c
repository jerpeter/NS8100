///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Record.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:57 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Record.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Menu.h"
#include "Flash.h"
#include "Record.h"
#include "Summary.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "Display.h"
#include "spi.h"
#include "eeprom_test_menu.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define RECORD_STORAGE_SIZE_x16	FLASH_BOOT_SECTOR_SIZE_x16

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

/****************************************
*	Function:  storeMnData()
*	Purpose:   
****************************************/
void saveRecData(void* src_ptr, uint32 num, uint8 type)
{        
	uint16 loc;
	uint16 rec_size;

	switch (type)
	{
		case REC_TRIGGER_USER_MENU_TYPE:
			debug("Programming Trigger Record...\n");
			((REC_EVENT_MN_STRUCT *)src_ptr)->time_stamp = getCurrentTime();

			rec_size = sizeof(REC_EVENT_MN_STRUCT);
			loc = (uint16)(sizeof(REC_EVENT_MN_STRUCT) * num);
			SaveParameterMemory((uint8*)src_ptr, loc, rec_size);
			break;

		case REC_HELP_USER_MENU_TYPE:
			debug("Programming Help Record...\n");

			((REC_HELP_MN_STRUCT *)src_ptr)->encode_ln = 0xA5A5;

			rec_size = sizeof(REC_HELP_MN_STRUCT);
			loc = (sizeof(REC_EVENT_MN_STRUCT)) * (MAX_NUM_OF_SAVED_SETUPS + 1);
			SaveParameterMemory((uint8*)src_ptr, loc, rec_size);
			break;

		case REC_MODEM_SETUP_TYPE:
			debug("Programming Modem Setup Configuration...\n");

			((MODEM_SETUP_STRUCT *)src_ptr)->invalid = 0x0000;

			rec_size = sizeof(MODEM_SETUP_STRUCT);
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1) + 
					sizeof(REC_HELP_MN_STRUCT));
			SaveParameterMemory((uint8*)src_ptr, loc, rec_size);
			break;

		case REC_FACTORY_SETUP_TYPE:
			debug("Programming Factory Setup Record...\n");

			((FACTORY_SETUP_STRUCT *)src_ptr)->invalid = 0x0000;

			rec_size = sizeof(FACTORY_SETUP_STRUCT);
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1) + 
					sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT));
			SaveParameterMemory((uint8*)src_ptr, loc, rec_size);
			break;

		case REC_UNIQUE_EVENT_ID_TYPE:
			debug("Programming Current Event Number Record...\n");

			((CURRENT_EVENT_NUMBER_STRUCT*)src_ptr)->invalid = 0x0000;

			rec_size = sizeof(CURRENT_EVENT_NUMBER_STRUCT);
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1) + 
					sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT) +
					sizeof(FACTORY_SETUP_STRUCT));
			SaveParameterMemory((uint8*)src_ptr, loc, rec_size);
			break;

		case REC_UNIQUE_MONITOR_LOG_ID_TYPE:
			debug("Programming Monitor Log ID Record...\n");

			((MONITOR_LOG_ID_STRUCT*)src_ptr)->invalid = 0x0000;

			rec_size = sizeof(MONITOR_LOG_ID_STRUCT);
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1) + 
					sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT) +
					sizeof(FACTORY_SETUP_STRUCT) + sizeof(CURRENT_EVENT_NUMBER_STRUCT));
			SaveParameterMemory((uint8*)src_ptr, loc, rec_size);
			break;

		default: // If type doesnt match, just return
			return;
			break;
	}
}

/****************************************
*	Function:  getRecData()
*	Purpose:
****************************************/
void getRecData(void* dst_ptr, uint32 num, uint8 type)
{ 
	uint16 loc;

	switch (type)
	{
		case REC_TRIGGER_USER_MENU_TYPE:
			loc = (uint16)(sizeof(REC_EVENT_MN_STRUCT) * num);
			GetParameterMemory((uint8*)dst_ptr, loc, sizeof(REC_EVENT_MN_STRUCT));
			break;

		case REC_PRINTER_USER_MENU_TYPE:
			break;

		case REC_HELP_USER_MENU_TYPE:
			loc = sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1);
			GetParameterMemory((uint8*)dst_ptr, loc, sizeof(REC_HELP_MN_STRUCT));
			break;

		case REC_MODEM_SETUP_TYPE:
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1)) + 
					sizeof(REC_HELP_MN_STRUCT);
			GetParameterMemory((uint8*)dst_ptr, loc, sizeof(MODEM_SETUP_STRUCT));
			break;

		case REC_FACTORY_SETUP_TYPE:
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1)) + 
					sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT);
			GetParameterMemory((uint8*)dst_ptr, loc, sizeof(FACTORY_SETUP_STRUCT));
			break;

		case REC_UNIQUE_EVENT_ID_TYPE:
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1)) + 
					sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT) +
					sizeof(FACTORY_SETUP_STRUCT);
			GetParameterMemory((uint8*)dst_ptr, loc, sizeof(CURRENT_EVENT_NUMBER_STRUCT));
			break;

		case REC_UNIQUE_MONITOR_LOG_ID_TYPE:
			loc = (sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1)) + 
					sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT) +
					sizeof(FACTORY_SETUP_STRUCT) + sizeof(CURRENT_EVENT_NUMBER_STRUCT);
			GetParameterMemory((uint8*)dst_ptr, loc, sizeof(MONITOR_LOG_ID_STRUCT));
			break;

		default:
			break;
	}
}

/****************************************
*	Function:  convertTimeToString()
*	Purpose:
****************************************/
void convertTimeStampToString(char* buff,void* rec_ptr,uint8 type)
{        
	REC_EVENT_MN_STRUCT *ttemp;
	DATE_TIME_STRUCT *tempTime;
	uint8 tbuff[5];

	// Clear the buffer.
	byteSet(&tbuff[0], 0, sizeof(tbuff));

	switch (type)
	{
		case REC_TRIGGER_USER_MENU_TYPE:
			ttemp = (REC_EVENT_MN_STRUCT *)rec_ptr;

			sprintf((char*)buff,"%02d/%02d/%02d %02d:%02d",
				ttemp->time_stamp.month, ttemp->time_stamp.day, ttemp->time_stamp.year, 
				ttemp->time_stamp.hour,	ttemp->time_stamp.min);
			break;

		case REC_DATE_TYPE:
			tempTime = (DATE_TIME_STRUCT *)rec_ptr;

			if((tempTime->month >= 1) && (tempTime->month <= 12))
			{	
				strcpy((char*)tbuff, (char*)(g_monthTable[tempTime->month].name));
			}
			else
			{
				strcpy((char*)tbuff, (char*)(g_monthTable[1].name));
			}			
			
			sprintf((char*)buff,"%02d-%s-%02d", tempTime->day, tbuff, tempTime->year);
			break;

		case REC_DATE_TIME_TYPE:
			tempTime = (DATE_TIME_STRUCT *)rec_ptr;

			if ((tempTime->month >= 1) && (tempTime->month <= 12))
			{	
				strcpy((char*)tbuff, (char*)(g_monthTable[tempTime->month].name));
			}
			else
			{
				strcpy((char*)tbuff, (char*)(g_monthTable[1].name));
			}			
			
			sprintf((char*)buff,"%02d-%s-%02d %02d:%02d:%02d",
				tempTime->day, tbuff, tempTime->year, tempTime->hour, 
				tempTime->min, tempTime->sec);
			break;

		case REC_DATE_TIME_AM_PM_TYPE:
			tempTime = (DATE_TIME_STRUCT *)rec_ptr;

			if ((tempTime->month >= 1) && (tempTime->month <= 12))
			{	
				strcpy((char*)tbuff, (char*)(g_monthTable[tempTime->month].name));
			}
			else
			{
				strcpy((char*)tbuff, (char*)(g_monthTable[1].name));
			}			

			if (tempTime->hour > 12)
			{
				sprintf((char*)buff,"%02d-%s-%02d %02d:%02d:%02d pm",
					tempTime->day, tbuff, tempTime->year, (tempTime->hour - 12), 
					tempTime->min, tempTime->sec);
			}
			else
			{
				sprintf((char*)buff,"%02d-%s-%02d %02d:%02d:%02d am",
					tempTime->day, tbuff, tempTime->year, tempTime->hour, 
					tempTime->min, tempTime->sec);
			}
			break;

		case REC_DATE_TIME_DISPLAY:
			tempTime = (DATE_TIME_STRUCT *)rec_ptr;
			
			if ((tempTime->month >= 1) && (tempTime->month <= 12))
			{	
				strcpy((char*)tbuff, (char*)(g_monthTable[tempTime->month].name));
			}
			else
			{
				strcpy((char*)tbuff, (char*)(g_monthTable[1].name));
			}			
			
			sprintf((char*)buff,"%02d%s%02d %02d:%02d",
				tempTime->day, tbuff, tempTime->year,
				tempTime->hour, tempTime->min);
			break;

		case REC_DATE_TIME_MONITOR:
			tempTime = (DATE_TIME_STRUCT *)rec_ptr;
			
			if ((tempTime->month >= 1) && (tempTime->month <= 12))
			{	
				strcpy((char*)tbuff, (char*)(g_monthTable[tempTime->month].name));
			}
			else
			{
				strcpy((char*)tbuff, (char*)(g_monthTable[1].name));
			}			

			sprintf((char*)buff,"%02d-%s-%02d", 
				tempTime->day, tbuff, tempTime->year);
			break;

		default:
			break;
	}
}

/****************************************
*	Function:  copyFlashBlock()
*	Purpose:
****************************************/
void copyFlashBlock(uint16* dst, uint16* src, uint32 len)
{
	while (len > 0)
	{
		// Copy src data into dest
		*dst = *src;

		// Increment dest and src pointers
		dst++;
		src++;

		// Decrement length
		len--;
	}

	return;
}

/****************************************
*	Function:  copyRecIntoFlashBk()
*	Purpose:
****************************************/
void copyRecIntoFlashBk(uint16* dst,uint16* src, uint32 loc, uint32 len)
{
	//debugPrint(RAW, "\n CRIFBK -> ");

	while (len > 0)
	{
		// Copy src data into dest offset by loc
		*(dst + loc) = *src;

		//debugPrint(RAW, "%x ", *(dst + loc));

		// Increment dest and src pointers
		dst++;
		src++;

		// Decrement length
		len--;
	}
}

/****************************************
*	Function:  checkForAvailableTriggerRecordEntry()
*	Purpose:
****************************************/
uint8 checkForAvailableTriggerRecordEntry(char* name, uint8* match)
{
	REC_EVENT_MN_STRUCT temp_rec;
	uint8 i;
	uint8 availableRecord = 0;

	// Loop through all the valid saved setup record slots (0 isn't valid, used for the default saved setup)
	for (i = 1; i <= MAX_NUM_OF_SAVED_SETUPS; i++)
	{
		// Load the temp record with the saved setup
		getRecData(&temp_rec, i, REC_TRIGGER_USER_MENU_TYPE);

		// Check if any of the saved setups have the same name (8 is the max number of chars that can be input)
		if (strncmp((char*)temp_rec.name, name, 8) == 0)
		{
			// Set the data pointed to by match to YES to signal a name conflict
			*match = YES;

			// Return the record location index of the matched name
			return (i);
		}
		
		// Check if an empty location hasn't been found yet (0 isn't valid)
		if (availableRecord == 0)
		{
			// Check if the current record location isn't valid
			if (temp_rec.validRecord != YES)
			{
				// Set the emptyLocation to the current index
				availableRecord = i;
			}
		}
	}
	
	return (availableRecord);
}

/****************************************
*	Function:	 loadTrigRecordDefaults
*	Purpose:
****************************************/
void loadTrigRecordDefaults(REC_EVENT_MN_STRUCT *rec_ptr, uint8 op_mode)
{
	// General components
	rec_ptr->validRecord = YES;
	rec_ptr->op_mode = op_mode;
	rec_ptr->trec.sample_rate = SAMPLE_RATE_1K;
	rec_ptr->srec.sensitivity = LOW;
	rec_ptr->trec.dist_to_source = 0;
	rec_ptr->trec.weight_per_delay = 0;
	rec_ptr->trec.record_time = 0;
	rec_ptr->trec.seismicTriggerLevel = NO_TRIGGER_CHAR;
	rec_ptr->trec.airTriggerLevel = NO_TRIGGER_CHAR;
	rec_ptr->bgrec.barInterval = SIXTY_SEC_PRD;
	rec_ptr->bgrec.summaryInterval = ONE_HOUR_INTVL;
	rec_ptr->berec.barScale = BAR_SCALE_FULL;
	rec_ptr->berec.barChannel = BAR_BOTH_CHANNELS;
	rec_ptr->berec.impulseMenuUpdateSecs = 1;

	// Clear strings
	byteSet((char*)rec_ptr->trec.client, 0, sizeof(rec_ptr->trec.client));
	byteSet((char*)rec_ptr->trec.loc, 0, sizeof(rec_ptr->trec.loc));
	byteSet((char*)rec_ptr->trec.comments, 0, sizeof(rec_ptr->trec.comments));
	byteSet((char*)rec_ptr->trec.oper, 0, sizeof(rec_ptr->trec.oper));

	// Mode specific 
	switch (op_mode)
	{
		case(WAVEFORM_MODE):
			rec_ptr->trec.seismicTriggerLevel = 25; // A/D Counts
			rec_ptr->trec.record_time = 3;
			break;

		case(BARGRAPH_MODE):
			break;

		case(COMBO_MODE):
			break;

		case(MANUAL_TRIGGER_MODE):
			break;
	} 
}

/****************************************
*	Function:	 loadHelpRecordDefaults
*	Purpose:
****************************************/
void loadHelpRecordDefaults(REC_HELP_MN_STRUCT *rec_ptr)
{  
	// Initialize the help record
	byteSet(rec_ptr, 0, sizeof(REC_HELP_MN_STRUCT));

	// Set default conditions
	rec_ptr->flash_wrapping = YES;
	rec_ptr->auto_monitor_mode = AUTO_NO_TIMEOUT;
	rec_ptr->auto_cal_mode = AUTO_NO_CAL_TIMEOUT;
	rec_ptr->alarm_one_mode = ALARM_MODE_OFF;
	rec_ptr->alarm_two_mode = ALARM_MODE_OFF;
	rec_ptr->alarm_one_seismic_lvl = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
	rec_ptr->alarm_one_air_lvl     = ALARM_ONE_AIR_DEFAULT_TRIG_LVL;
	rec_ptr->alarm_two_seismic_lvl = ALARM_TWO_SEIS_DEFAULT_TRIG_LVL;
	rec_ptr->alarm_two_air_lvl     = ALARM_TWO_AIR_DEFAULT_TRIG_LVL;
	rec_ptr->alarm_one_time  = ALARM_OUTPUT_TIME_DEFAULT;
	rec_ptr->alarm_two_time  = ALARM_OUTPUT_TIME_DEFAULT;
#if 0 // ns7100
	rec_ptr->baud_rate = BAUD_RATE_38400;
#else // ns8100
	rec_ptr->baud_rate = BAUD_RATE_115200;
#endif
	rec_ptr->copies = 1;
	rec_ptr->freq_plot_type = 1;
	rec_ptr->lang_mode = ENGLISH_LANG;
	rec_ptr->lcd_contrast = DEFUALT_CONTRAST;
	rec_ptr->lcd_timeout = 2;
	rec_ptr->timer_mode = DISABLED;
	rec_ptr->units_of_measure = IMPERIAL_TYPE;
	rec_ptr->vector_sum = DISABLED;
	rec_ptr->report_displacement = DISABLED;
	rec_ptr->auto_cal_in_waveform = NO;

	if (SUPERGRAPH_UNIT)
	{
		rec_ptr->auto_print = ON;
		rec_ptr->freq_plot_mode = ON;
		rec_ptr->print_monitor_log = YES;
	}
	else // Mini unit
	{
		rec_ptr->auto_print = OFF;
		rec_ptr->freq_plot_mode = OFF;
		rec_ptr->print_monitor_log = NO;
	}
}

/****************************************
*	Function:	 activateHelpRecordOptions
*	Purpose:
****************************************/
void activateHelpRecordOptions(void)
{
	g_contrast_value = g_helpRecord.lcd_contrast;

	if ((g_contrast_value < MIN_CONTRAST) || (g_contrast_value > MAX_CONTRAST))
	{
		g_helpRecord.lcd_contrast = g_contrast_value = DEFUALT_CONTRAST;
	}               

	setLcdContrast(g_contrast_value);

	// The choices are between metric and sae measurement systems.
	g_sensorInfoPtr->unitsFlag = g_helpRecord.units_of_measure;

	assignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, displayTimerCallBack);

	if ((g_helpRecord.lcd_timeout < LCD_TIMEOUT_MIN_VALUE) || (g_helpRecord.lcd_timeout > LCD_TIMEOUT_MAX_VALUE))
	{
		g_helpRecord.lcd_timeout = LCD_TIMEOUT_DEFAULT_VALUE;
	}
	assignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_helpRecord.lcd_timeout * TICKS_PER_MIN), lcdPwTimerCallBack);

	debug("Auto Monitor Mode: %s\n", (g_helpRecord.auto_monitor_mode == AUTO_NO_TIMEOUT) ? "Disabled" : "Enabled");
    assignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_helpRecord.auto_monitor_mode * TICKS_PER_MIN), autoMonitorTimerCallBack);     

	if (g_helpRecord.auto_cal_mode != AUTO_NO_CAL_TIMEOUT)
	{
		g_autoCalDaysToWait = 1;
	}
}

/****************************************
*	Function:	 loadModemSetupRecordDefaults
*	Purpose:
****************************************/
void loadModemSetupRecordDefaults()
{
	// Initialize the help record
	byteSet(&g_modemSetupRecord, 0, sizeof(MODEM_SETUP_STRUCT));

	g_modemSetupRecord.modemStatus = NO;
	g_modemSetupRecord.retries = MODEM_RETRY_DEFAULT_VALUE;
	g_modemSetupRecord.retryTime = MODEM_RETRY_TIME_DEFAULT_VALUE;

	// No need to set these since the memset to zero satisifies initialization
	//g_modemSetupRecord.unlockCode
	//g_modemSetupRecord.init
	//g_modemSetupRecord.dial
	//g_modemSetupRecord.reset
}

/****************************************
*	Function:	 validateModemSetupParameters
*	Purpose:
****************************************/
void validateModemSetupParameters(void)
{
	uint8 updated = NO;

	if ((g_modemSetupRecord.retries < MODEM_RETRY_MIN_VALUE) || (g_modemSetupRecord.retries > MODEM_RETRY_MAX_VALUE))
	{
		g_modemSetupRecord.retries = MODEM_RETRY_DEFAULT_VALUE;
		updated = YES;	
	}

	if ((g_modemSetupRecord.retryTime < MODEM_RETRY_TIME_MIN_VALUE) || (g_modemSetupRecord.retryTime > MODEM_RETRY_TIME_MAX_VALUE))
	{
		g_modemSetupRecord.retryTime = MODEM_RETRY_TIME_DEFAULT_VALUE;
		updated = YES;
	}
	
	if (updated)
	{
		// Save the Modem Setup Record
		saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
	}
}

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
void GetParameterMemory(uint8* dataDest, uint16 startAddr, uint16 dataLength)
{
	uint16 tempData;
	
	//debugRaw("\nGPM: Addr: %x -> ", startAddr);

	spi_selectChip(&AVR32_SPI1, EEPROM_SPI_NPCS);

	// Write Command
	spi_write(&AVR32_SPI1, EEPROM_READ_DATA);
	spi_write(&AVR32_SPI1, (startAddr >> 8) & 0xFF);
	spi_write(&AVR32_SPI1, startAddr & 0xFF);

	while(dataLength--)
	{
		spi_write(&AVR32_SPI1, 0xFF);
		spi_read(&AVR32_SPI1, &tempData);

		//debugRaw("%02x ", (uint8)(tempData));

		// Store the byte data into the data array and inc the pointer			
		*dataDest++ = (uint8)tempData;
	}
	   
	spi_unselectChip(&AVR32_SPI1, EEPROM_SPI_NPCS);
}

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
#define EEPROM_PAGE_SIZE	32
void SaveParameterMemory(uint8* dataSrc, uint16 startAddr, uint16 dataLength)
{
	uint16 tempData;
	uint16 writeLength;
	uint8 checkForPartialFirstPage = YES;
	uint8 lengthToPageBoundary = 0;
	
	while(dataLength)
	{
		//debug("SPM: Addr: %x Len: %04d -> ", startAddr, dataLength);

		// Activate write enable
		spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
		spi_write(EEPROM_SPI, EEPROM_WRITE_ENABLE); // Write Command
		spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);

		// Write data
		spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
		spi_write(EEPROM_SPI, EEPROM_WRITE_DATA); // Write Command
		spi_write(EEPROM_SPI, (startAddr >> 8) & 0xFF);
		spi_write(EEPROM_SPI, startAddr & 0xFF);

		// Adjust for page boundaries
		if(checkForPartialFirstPage == YES)
		{
			checkForPartialFirstPage = NO;
			
			lengthToPageBoundary = (EEPROM_PAGE_SIZE - (startAddr % 32));
			
			// Check data length against remaining length of 32 page boundary
			if(dataLength <= lengthToPageBoundary)
			{
				// Complete data storage within first/partial page
				writeLength = dataLength;
				dataLength = 0;
			}
			else // Data length goes beyond first page boundary
			{
				writeLength = lengthToPageBoundary;
				dataLength -= lengthToPageBoundary;
				startAddr += lengthToPageBoundary;
			}
		}
		else if(dataLength <= EEPROM_PAGE_SIZE) // Check if the rest of the data fits into the next page
		{
			writeLength = dataLength;
			dataLength = 0;
		}
		else // Already aligned to a page boundary, and more than 1 page of data to write
		{
			writeLength = EEPROM_PAGE_SIZE;
			dataLength -= EEPROM_PAGE_SIZE;	
			startAddr += EEPROM_PAGE_SIZE;
		}			
			
		while(writeLength--)
		{
			//debugRaw("%02x ", *dataSrc);

			tempData = *dataSrc++;
			spi_write(EEPROM_SPI, tempData);
		}

		spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
		soft_usecWait(5 * SOFT_MSECS);
	}
}

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
void EraseParameterMemory(uint16 startAddr, uint16 dataLength)
{
	uint16 tempData = 0x00FF;
	uint16 pageSize;

	while(dataLength)
	{
		// Activate write enable
		spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
		spi_write(EEPROM_SPI, EEPROM_WRITE_ENABLE); // Write Command
		spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);

		// Write data
		spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
		spi_write(EEPROM_SPI, EEPROM_WRITE_DATA); // Write Command
		spi_write(EEPROM_SPI, (startAddr >> 8) & 0xFF);
		spi_write(EEPROM_SPI, startAddr & 0xFF);

		// Check if current data length is less than 32 and can be finished in a page
		if(dataLength <= 32)
		{
			pageSize = dataLength;
			dataLength = 0;
		}
		else // While loop will run again
		{
			pageSize = 32;
			dataLength -= 32;	
			startAddr += 32;
		}			
			
		while(pageSize--)
		{
			spi_write(EEPROM_SPI, tempData);
			
			soft_usecWait(1 * SOFT_MSECS);
		}

		spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
		soft_usecWait(5 * SOFT_MSECS);
	}
}

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
#if 0
uint8 ReadParameterMemory(uint16 address)
{
	uint8 data = 0;

	while(ReadStatusParameterMemory() & 0x01) {};

	// Assert slave select
	reg_SPIPORT.reg &= ~0x08;

	// Read command
	reg_SPIDR = 0x03;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write upper address
	reg_SPIDR = (uint8)(address >> 8);
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write lower address
	reg_SPIDR = (uint8)(address & 0x00FF);
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write dummy data
	reg_SPIDR = 0x00;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Read data
	data = reg_SPIDR;

	// Deassert slave select
	reg_SPIPORT.reg |= 0x08;

	return (data);
}
#endif	

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
#if 0
void WriteParameterMemory(uint16 address, uint8 data)
{
	while(ReadStatusParameterMemory() & 0x01) {};
	SetWriteEnableLatchParameterMemory();

	// Assert slave select
	reg_SPIPORT.reg &= ~0x08;

	// Issue a write command
	reg_SPIDR = 0x02;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write the upper address
	reg_SPIDR = (uint8)(address >> 8);
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write the lower address
	reg_SPIDR = (uint8)(address & 0x00FF);
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write the data
	reg_SPIDR = (uint8)data;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Dummy read
	data = reg_SPIDR;

	// Deassert slave select
	reg_SPIPORT.reg |= 0x08;
}
#endif

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		Disable Writes
//-----------------------------------------------------------------------------
#if 0
void ResetWriteEnableLatchParameterMemory(void)
{
	uint8 data;

	// Assert slave select
	reg_SPIPORT.reg &= ~0x08;

	// Issue reset write enable latch
	reg_SPIDR = 0x04;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Dummy read
	data = reg_SPIDR;

	// Deassert slave select
	reg_SPIPORT.reg |= 0x08;
}
#endif

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		Enable Writes
//-----------------------------------------------------------------------------
#if 0
void SetWriteEnableLatchParameterMemory(void)
{
	uint8 data;
	
	// Assert slave select
	reg_SPIPORT.reg &= ~0x08;

	// Issue write enable latch
	reg_SPIDR = 0x06;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Dummy read
	data = reg_SPIDR;

	// Deassert slave select
	reg_SPIPORT.reg |= 0x08;
}
#endif

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
#if 0
uint8 ReadStatusParameterMemory(void)
{
	uint8 status = 0;

	// Assert slave select
	reg_SPIPORT.reg &= ~0x08;

	// Issue read status comamnd
	reg_SPIDR = 0x05;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Dummy read
	reg_SPIDR = 0x00;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Read status byte
	status = reg_SPIDR;

	// Deassert slave select
	reg_SPIPORT.reg |= 0x08;

	return (status);
}
#endif

//-----------------------------------------------------------------------------
// Fcuntion:	
// Purpose:		
//-----------------------------------------------------------------------------
#if 0
void WriteStatusParameterMemory(uint8 data)
{
	// Assert slave select
	reg_SPIPORT.reg &= ~0x08;

	// Issue write status command
	reg_SPIDR = 0x01;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Write data
	reg_SPIDR = (uint8)data;
	while ((reg_SPISR.reg & 0x80) == 0) {}

	// Dummy read
	data = reg_SPIDR;

	// Deassert slave select
	reg_SPIPORT.reg |= 0x08;
}
#endif
