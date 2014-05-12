///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteOperation.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:59 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteOperation.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "RemoteCommon.h"
#include "RemoteOperation.h"
#include "Uart.h"
#include "Menu.h"
#include "InitDataBuffers.h"
#include "SysEvents.h"
#include "PowerManagement.h"
#include "Crc.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

//==================================================
// Function: handleAAA
// Description:  
// 		Dummy function for testing purposes.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleAAA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}


//==================================================
// Operating parameter commands
//==================================================

//==================================================
// Function: handleDCM
// Description: 
// 		Download configuration data structure.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleDCM(CMD_BUFFER_STRUCT* inCmd)
{
	SYSTEM_CFG cfg;					// 424 bytes, or 848 chars from the pc.
	uint8 dcmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	int majorVer, minorVer;

	UNUSED(inCmd);

	byteSet(&cfg, 0, sizeof(SYSTEM_CFG));

	cfg.mode = g_triggerRecord.op_mode;
	cfg.monitorStatus = g_sampleProcessing; 
	cfg.currentTime = getCurrentTime();
	
	cfg.eventCfg.distToSource = (uint32)(g_triggerRecord.trec.dist_to_source * 100.0);
	cfg.eventCfg.weightPerDelay = (uint32)(g_triggerRecord.trec.weight_per_delay * 100.0);
	cfg.eventCfg.sampleRate = (uint16)g_triggerRecord.trec.sample_rate;
	
	// Scan for major and minor version from the app version string and store in the config
	sscanf(&g_appVersion[0], "%d.%d", &majorVer, &minorVer);
	
	cfg.eventCfg.appMajorVersion = (uint8)majorVer;
	cfg.eventCfg.appMinorVersion = (uint8)minorVer;

	// Waveform specific - Intitial conditions.
	cfg.eventCfg.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
	cfg.eventCfg.airTriggerLevel = g_triggerRecord.trec.soundTriggerLevel;
	cfg.eventCfg.recordTime = g_triggerRecord.trec.record_time;

	// static non changing.
	cfg.eventCfg.seismicSensorType = (uint16)(g_factorySetupRecord.sensor_type);
	cfg.eventCfg.airSensorType = (uint16)0x0;
	cfg.eventCfg.bitAccuracy = 12;
	cfg.eventCfg.numOfChannels = 4;
	cfg.eventCfg.aWeighting = (uint8)g_factorySetupRecord.aweight_option;

	cfg.eventCfg.numOfSamples = 0;				// Not used for configuration settings
	// Used for setting sesitivity
	cfg.eventCfg.preBuffNumOfSamples = (uint16)g_triggerRecord.srec.sensitivity;
	cfg.eventCfg.calDataNumOfSamples = g_triggerRecord.berec.barScale;
	cfg.eventCfg.activeChannels = g_triggerRecord.berec.barChannel;

	// Bargraph specific - Intitial conditions.
	cfg.eventCfg.barInterval = (uint16)g_triggerRecord.bgrec.barInterval;
	cfg.eventCfg.summaryInterval = (uint16)g_triggerRecord.bgrec.summaryInterval;

	byteCpy((uint8*)cfg.eventCfg.companyName, g_triggerRecord.trec.client, COMPANY_NAME_STRING_SIZE - 2);
	byteCpy((uint8*)cfg.eventCfg.seismicOperator, g_triggerRecord.trec.oper, SEISMIC_OPERATOR_STRING_SIZE - 2);
	byteCpy((uint8*)cfg.eventCfg.sessionLocation, g_triggerRecord.trec.loc, SESSION_LOCATION_STRING_SIZE - 2);
	byteCpy((uint8*)cfg.eventCfg.sessionComments, g_triggerRecord.trec.comments, SESSION_COMMENTS_STRING_SIZE - 2);
	
	cfg.autoCfg.auto_monitor_mode = g_helpRecord.auto_monitor_mode;
	cfg.autoCfg.auto_cal_mode = g_helpRecord.auto_cal_mode;

	cfg.printerCfg.auto_print = g_helpRecord.auto_print;
	cfg.printerCfg.lang_mode = g_helpRecord.lang_mode;
	cfg.printerCfg.vector_sum = g_helpRecord.vector_sum;
	cfg.printerCfg.units_of_measure = g_helpRecord.units_of_measure;
	cfg.printerCfg.freq_plot_mode = g_helpRecord.freq_plot_mode;
	cfg.printerCfg.freq_plot_type = g_helpRecord.freq_plot_type;

	cfg.alarmCfg.alarm_one_mode = g_helpRecord.alarm_one_mode;
	cfg.alarmCfg.alarm_two_mode = g_helpRecord.alarm_two_mode;
	cfg.alarmCfg.alarm_one_seismic_lvl = g_helpRecord.alarm_one_seismic_lvl;
	cfg.alarmCfg.alarm_one_seismic_min_lvl = g_helpRecord.alarm_one_seismic_min_lvl;
	cfg.alarmCfg.alarm_one_air_lvl = g_helpRecord.alarm_one_air_lvl;
	cfg.alarmCfg.alarm_one_air_min_lvl = g_helpRecord.alarm_one_air_min_lvl;
	cfg.alarmCfg.alarm_two_seismic_lvl = g_helpRecord.alarm_two_seismic_lvl;
	cfg.alarmCfg.alarm_two_seismic_min_lvl = g_helpRecord.alarm_two_seismic_min_lvl;
	cfg.alarmCfg.alarm_two_air_lvl = g_helpRecord.alarm_two_air_lvl;
	cfg.alarmCfg.alarm_two_air_min_lvl = g_helpRecord.alarm_two_air_min_lvl;
	cfg.alarmCfg.alarm_one_time = (uint32)(g_helpRecord.alarm_one_time * (float)100.0);
	cfg.alarmCfg.alarm_two_time = (uint32)(g_helpRecord.alarm_two_time * (float)100.0);

	cfg.timerCfg.timer_mode = g_helpRecord.timer_mode;
	cfg.timerCfg.timer_mode_freq = g_helpRecord.timer_mode_freq;
	cfg.timerCfg.timer_mode_active_minutes = g_helpRecord.timer_mode_active_minutes;

	if (DISABLED == g_helpRecord.timer_mode) 
	{
		cfg.timerCfg.timer_stop = cfg.timerCfg.timer_start = getCurrentTime();
	}
	else
	{
		cfg.timerCfg.timer_start.hour = g_helpRecord.tm_start_time.hour;
		cfg.timerCfg.timer_start.min = g_helpRecord.tm_start_time.min;
		cfg.timerCfg.timer_start.sec = g_helpRecord.tm_start_time.sec;
		cfg.timerCfg.timer_start.day = g_helpRecord.tm_start_date.day;
		cfg.timerCfg.timer_start.month = g_helpRecord.tm_start_date.month;
		cfg.timerCfg.timer_start.year = g_helpRecord.tm_start_date.year;

		cfg.timerCfg.timer_stop.hour = g_helpRecord.tm_stop_time.hour;
		cfg.timerCfg.timer_stop.min = g_helpRecord.tm_stop_time.min;
		cfg.timerCfg.timer_stop.sec = g_helpRecord.tm_stop_time.sec;
		cfg.timerCfg.timer_stop.day = g_helpRecord.tm_stop_date.day;
		cfg.timerCfg.timer_stop.month = g_helpRecord.tm_stop_date.month;
		cfg.timerCfg.timer_stop.year = g_helpRecord.tm_stop_date.year;
	}
	
	// Add in the flash wrapping option
	cfg.flashWrapping = g_helpRecord.flash_wrapping;

	// Spare fields, just use as a data marker
	cfg.unused[0] = 0x0A;
	cfg.unused[1] = 0x0B;
	cfg.unused[2] = 0x0C;
	cfg.unused[3] = 0x0D;
	cfg.unused[4] = 0x0E;
	cfg.unused[5] = 0x0F;

	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);
	buildOutgoingSimpleHeaderBuffer((uint8*)dcmHdr, (uint8*)"DCMx", (uint8*)msgTypeStr, 
		(uint32)(MESSAGE_SIMPLE_TOTAL_LENGTH + sizeof(SYSTEM_CFG)), COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&dcmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	modem_puts((uint8*)dcmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag);

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&cfg, sizeof(SYSTEM_CFG), g_transmitCRC);		

	// Send the configuration data
	modem_puts((uint8*)&cfg, sizeof(SYSTEM_CFG), g_binaryXferFlag);

	// Ending Footer
	modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

//==================================================
// Function: handleUCM
// Description: 
// 		Upload configuration data structure.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleUCM(CMD_BUFFER_STRUCT* inCmd)
{
	SYSTEM_CFG cfg;
	uint8* cfgPtr = (uint8*)(&cfg);
	uint8 timerModeModified = FALSE;
	uint16 buffDex;
	uint32 timeCheck;
	uint32 maxRecordTime;
	uint32 returnCode = CFG_ERR_NONE;
	uint32 sizeOfCfg;
	uint32 msgCRC = 0;
	uint32 inCRC;
	uint16 j = 0;
	uint8 ucmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	float  timeCheckFloat;
	float  timeAlarmFloat;

	if (ACTIVE_STATE == g_sampleProcessing)
	{   
		returnCode = CFG_ERR_MONITORING_STATE;
	}
	else
	{	
	 	sizeOfCfg = sizeof(SYSTEM_CFG);
		byteSet((uint8*)&cfg, 0, sizeOfCfg);

		// Check to see if the incomming message is the correct size
		if (((inCmd->size - 16)/2) < sizeOfCfg)
		{
			debug("WARNING:Msg Size incorrect msgSize=%d cfgSize=%d \n", ((inCmd->size - 16)/2), sizeOfCfg);
		}
		
		// Move the string data into the configuration structure. String is (2 * cfgSize)
		buffDex = MESSAGE_HEADER_SIMPLE_LENGTH;
		while ((buffDex < inCmd->size) && (buffDex < (MESSAGE_HEADER_SIMPLE_LENGTH + (sizeOfCfg * 2))) && 
				(buffDex < CMD_BUFFER_SIZE))
		{	
			*cfgPtr++ = convertAscii2Binary(inCmd->msg[buffDex], inCmd->msg[buffDex + 1]);
			buffDex += 2;
		}		

		// Check if the SuperGraphics version is non-zero suppesting that it's a version that supports sending back CRC
		if (cfg.sgVersion)
		{
			// Get the CRC value from the data stream
			while ((buffDex < inCmd->size) && (buffDex < CMD_BUFFER_SIZE) && (j < 4))
			{
				// Add each CRC value by byte
				((uint8*)&inCRC)[j++] = convertAscii2Binary(inCmd->msg[buffDex], inCmd->msg[buffDex + 1]);
				buffDex += 2;
			}

			// Calcualte the CRC on the transmitted header and the converted binary data
			msgCRC = CalcCCITT32((uint8*)&(inCmd->msg[0]), MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);
			msgCRC = CalcCCITT32((uint8*)&cfg, sizeof(cfg), msgCRC);

			// Check if the incoming CRC value matches the calucalted message CRC
			if (inCRC != msgCRC)
			{
				// Signal a bad CRC value
				returnCode = CFG_ERR_BAD_CRC;

				sprintf((char*)msgTypeStr, "%02lu", returnCode);
				buildOutgoingSimpleHeaderBuffer((uint8*)ucmHdr, (uint8*)"UCMx", 
					(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);	

				// Send Starting CRLF
				modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

				// Calculate the CRC on the header
				g_transmitCRC = CalcCCITT32((uint8*)&ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

				// Send Simple header
				modem_puts((uint8*)ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);

				// Send Ending Footer
				modem_puts((uint8*)&msgCRC, 4, NO_CONVERSION);
				modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
				
				return;
			}
		}

		//--------------------------------

		// Distance to source check, cfg is in uint32 format not float.
		switch (cfg.mode)
		{
			case WAVEFORM_MODE: 
			case BARGRAPH_MODE:
			case COMBO_MODE:
				g_triggerRecord.op_mode = cfg.mode;
				break;

			case MANUAL_CAL_MODE:
			case MANUAL_TRIGGER_MODE:
			default:
				returnCode = CFG_ERR_TRIGGER_MODE;
				break;
		}
		
		// Check for correct values and update the month.
		if ((cfg.currentTime.day == 0) 	&& 
			(cfg.currentTime.month == 0) 	&& 
			(cfg.currentTime.year == 0)	&&
			(cfg.currentTime.sec == 0) 	&&
			(cfg.currentTime.min == 0) 	&&
			(cfg.currentTime.hour == 0)) 
		{
			debug("Do not update time.\n");
		}
		else
		{

			// Check for correct values and update the month.
			if ((cfg.currentTime.day == 0) 	|| 
				(cfg.currentTime.day > 31)	||
				(cfg.currentTime.month == 0) 	|| 
				(cfg.currentTime.month > 12) 	||
				(cfg.currentTime.year > 99))
			{
				returnCode = CFG_ERR_SYSTEM_DATE;
			}
			else
			{
				setRtcDate(&(cfg.currentTime));
				g_helpRecord.timer_mode = DISABLED;
				// Enable the Power Off key
				powerControl(POWER_SHUTDOWN_ENABLE, ON);
				
				// Disable the Power Off timer if it's set
				clearSoftTimer(POWER_OFF_TIMER_NUM);
			}
			
			// Check for correct values and update the date.
			if ((cfg.currentTime.sec > 59) 	||
				(cfg.currentTime.min > 59) 	||
				(cfg.currentTime.hour > 23)) 
			{
				returnCode = CFG_ERR_SYSTEM_TIME;
			}
			else
			{
				setRtcTime(&(cfg.currentTime));
				g_helpRecord.timer_mode = DISABLED;
				
				// Enable the Power Off key
				powerControl(POWER_SHUTDOWN_ENABLE, ON);
				
				// Disable the Power Off timer if it's set
				clearSoftTimer(POWER_OFF_TIMER_NUM);
			}

			// Update the current time.
			updateCurrentTime();
		}

		// Distance to source check, cfg is in uint32 format not float.
		if (cfg.eventCfg.distToSource > (uint32)(DISTANCE_TO_SOURCE_MAX_VALUE * 100))
		{
			returnCode = CFG_ERR_DIST_TO_SRC;
		}
		else
		{
			g_triggerRecord.trec.dist_to_source = (float)((float)cfg.eventCfg.distToSource / (float)100.0);
		}

		// Weight per delay check, cfg is in uint32 format not float.
		if (cfg.eventCfg.weightPerDelay > (uint32)(WEIGHT_PER_DELAY_MAX_VALUE * 100))
		{
			returnCode = CFG_ERR_WEIGHT_DELAY;
		}
		else
		{
			g_triggerRecord.trec.weight_per_delay = (float)((float)cfg.eventCfg.weightPerDelay / (float)100.0);
		}

		// Sample Rate check
		if ((512 == cfg.eventCfg.sampleRate) || (1024 == cfg.eventCfg.sampleRate) || (2048 == cfg.eventCfg.sampleRate) ||
			(4096 == cfg.eventCfg.sampleRate) || (8192 == cfg.eventCfg.sampleRate))
		{
			if ((BARGRAPH_MODE == g_triggerRecord.op_mode) && (1024 != cfg.eventCfg.sampleRate))
			{
				returnCode = CFG_ERR_SAMPLE_RATE;
			}
			else
			{
				g_triggerRecord.trec.sample_rate = (uint32)cfg.eventCfg.sampleRate;
			}
		}
		else
		{
			returnCode = CFG_ERR_SAMPLE_RATE;
		}

		// Seismic Trigger Level check
		if ((MANUAL_TRIGGER_CHAR == cfg.eventCfg.seismicTriggerLevel) 	||
			(NO_TRIGGER_CHAR == cfg.eventCfg.seismicTriggerLevel) 	||
			((cfg.eventCfg.seismicTriggerLevel >= SEISMIC_TRIGGER_MIN_VALUE) &&
			  (cfg.eventCfg.seismicTriggerLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
		{
			g_triggerRecord.trec.seismicTriggerLevel = cfg.eventCfg.seismicTriggerLevel;
			g_helpRecord.alarm_one_seismic_min_lvl = g_triggerRecord.trec.seismicTriggerLevel;
			g_helpRecord.alarm_two_seismic_min_lvl = g_triggerRecord.trec.seismicTriggerLevel;
		}
		else
		{
			returnCode = CFG_ERR_SEISMIC_TRIG_LVL;
		}
		
		// Sound Trigger Level check
		if ((MANUAL_TRIGGER_CHAR == cfg.eventCfg.airTriggerLevel) 	||
			(NO_TRIGGER_CHAR == cfg.eventCfg.airTriggerLevel) 	||
			((cfg.eventCfg.airTriggerLevel >= AIR_TRIGGER_MIN_VALUE) &&
			(cfg.eventCfg.airTriggerLevel <= AIR_TRIGGER_MAX_VALUE)))
		{
			g_triggerRecord.trec.soundTriggerLevel = cfg.eventCfg.airTriggerLevel;
			g_helpRecord.alarm_one_air_min_lvl = g_triggerRecord.trec.soundTriggerLevel;
			g_helpRecord.alarm_two_air_min_lvl = g_triggerRecord.trec.soundTriggerLevel;
		}
		else
		{
			returnCode = CFG_ERR_SOUND_TRIG_LVL;
		}

		// Find Max Record time for a given sample rate. Max time is equal to
		// Size of the (event buff - size of pre trigger buffer - size of calibration buff) 
		// divided by (sample rate * number of channels).
		// Number of channels is hardcoded to 4 = NUMBER_OF_CHANNELS_DEFAULT 
		maxRecordTime = (uint16)(((uint32)((EVENT_BUFF_SIZE_IN_WORDS - 
			((g_triggerRecord.trec.sample_rate / 4) * NUMBER_OF_CHANNELS_DEFAULT) - 
				((g_triggerRecord.trec.sample_rate / MIN_SAMPLE_RATE) * MAX_CAL_SAMPLES * NUMBER_OF_CHANNELS_DEFAULT)) / 
					(g_triggerRecord.trec.sample_rate * NUMBER_OF_CHANNELS_DEFAULT))));

		if ((cfg.eventCfg.recordTime >= 1) &&
			(cfg.eventCfg.recordTime <= maxRecordTime))
		{
			g_triggerRecord.trec.record_time = cfg.eventCfg.recordTime;
		}
		else
		{
			returnCode = CFG_ERR_RECORD_TIME;
		}
		

		// Contains the bargraph scale range
		// cfg.eventCfg.calDataNumOfSamples;

		// Bargraph specific - Intitial conditions.
		// Bar Interval Level check
		switch (cfg.eventCfg.barInterval)
		{
			case ONE_SEC_PRD: 
			case TEN_SEC_PRD:
			case TWENTY_SEC_PRD:
			case THIRTY_SEC_PRD:
			case FOURTY_SEC_PRD:
			case FIFTY_SEC_PRD:
			case SIXTY_SEC_PRD:
				g_triggerRecord.bgrec.barInterval = (uint32)cfg.eventCfg.barInterval;
				break;

			default:
				returnCode = CFG_ERR_BAR_INTERVAL;
				break;
		}

		// Bar Interval Level check
		switch (cfg.eventCfg.summaryInterval)
		{
			case FIVE_MINUTE_INTVL:
			case FIFTEEN_MINUTE_INTVL:
			case THIRTY_MINUTE_INTVL:
			case ONE_HOUR_INTVL:
			case TWO_HOUR_INTVL:
			case FOUR_HOUR_INTVL:
			case EIGHT_HOUR_INTVL:
			case TWELVE_HOUR_INTVL:
				g_triggerRecord.bgrec.summaryInterval = (uint32)cfg.eventCfg.summaryInterval;
				break;

			default:
				returnCode = CFG_ERR_SUM_INTERVAL;
				break;
		}

		// No check, just do not copy over the end of the string length.
		byteCpy((uint8*)g_triggerRecord.trec.client, cfg.eventCfg.companyName, COMPANY_NAME_STRING_SIZE - 2);
		byteCpy((uint8*)g_triggerRecord.trec.oper, cfg.eventCfg.seismicOperator, SEISMIC_OPERATOR_STRING_SIZE - 2);
		byteCpy((uint8*)g_triggerRecord.trec.loc, cfg.eventCfg.sessionLocation, SESSION_LOCATION_STRING_SIZE - 2);
		byteCpy((uint8*)g_triggerRecord.trec.comments, cfg.eventCfg.sessionComments, SESSION_COMMENTS_STRING_SIZE - 2);

		if ((LOW == cfg.eventCfg.preBuffNumOfSamples) || (HIGH == cfg.eventCfg.preBuffNumOfSamples))
		{
			g_triggerRecord.srec.sensitivity = cfg.eventCfg.preBuffNumOfSamples;		// Sesitivity
		}
		else
		{
			returnCode = CFG_ERR_SENSITIVITY;
		}
		
		if ((1 == cfg.eventCfg.calDataNumOfSamples) || (2 == cfg.eventCfg.calDataNumOfSamples) ||
			(4 == cfg.eventCfg.calDataNumOfSamples) || (8 == cfg.eventCfg.calDataNumOfSamples))
		{
			g_triggerRecord.berec.barScale = (uint8)cfg.eventCfg.calDataNumOfSamples;
		}
		else
		{
			returnCode = CFG_ERR_SCALING;
		}

		if (cfg.eventCfg.activeChannels <= BAR_AIR_CHANNEL) // Implied (cfg.eventCfg.activeChannels >= 0)
		{
			g_triggerRecord.berec.barChannel = cfg.eventCfg.activeChannels;
		}
		else
		{
			returnCode = CFG_ERR_BAR_PRINT_CHANNEL;
		}

		// Auto Monitor Mode check
		switch (cfg.autoCfg.auto_monitor_mode)
		{
			case AUTO_TWO_MIN_TIMEOUT:
			case AUTO_THREE_MIN_TIMEOUT:
			case AUTO_FOUR_MIN_TIMEOUT:
			case AUTO_NO_TIMEOUT:
				g_helpRecord.auto_monitor_mode = cfg.autoCfg.auto_monitor_mode;
				break;
				
			default:
				returnCode = CFG_ERR_AUTO_MON_MODE;
				break;
		}

		// Auto Monitor Mode check
		switch (cfg.autoCfg.auto_cal_mode)
		{	
			case AUTO_24_HOUR_TIMEOUT:
			case AUTO_48_HOUR_TIMEOUT:
			case AUTO_72_HOUR_TIMEOUT:
			case AUTO_NO_CAL_TIMEOUT:
				g_helpRecord.auto_cal_mode = cfg.autoCfg.auto_cal_mode;
				break;
				
			default:
				returnCode = CFG_ERR_AUTO_CAL_MODE;
				break;
		}

		// Auto print
		if ((YES == cfg.printerCfg.auto_print) || (NO == cfg.printerCfg.auto_print))
		{
			if (SUPERGRAPH_UNIT)
			{
				g_helpRecord.auto_print = cfg.printerCfg.auto_print;
			}
		}
		else
		{
			returnCode = CFG_ERR_AUTO_PRINT;
		}
		
		// Language Mode	
		switch (cfg.printerCfg.lang_mode)
		{
			case ENGLISH_LANG:
			case FRENCH_LANG:
			case ITALIAN_LANG:
			case GERMAN_LANG:
				g_helpRecord.lang_mode = cfg.printerCfg.lang_mode;
				build_languageLinkTable(g_helpRecord.lang_mode);
				break;
					
			default:
				returnCode = CFG_ERR_LANGUAGE_MODE;
				break;	
		}

		// Vector Sum check
		if ((YES == cfg.printerCfg.vector_sum) || (NO == cfg.printerCfg.vector_sum))
		{
			g_helpRecord.vector_sum = cfg.printerCfg.vector_sum;
		}
		else
		{
			returnCode = CFG_ERR_VECTOR_SUM;
		}
		
		// Units of Measuere check
		if ((IMPERIAL_TYPE == cfg.printerCfg.units_of_measure) || 
			(METRIC_TYPE == cfg.printerCfg.units_of_measure))
		{
			g_helpRecord.units_of_measure = cfg.printerCfg.units_of_measure;
		}
		else
		{
			returnCode = CFG_ERR_UNITS_OF_MEASURE;
		}
		
		// Frequency plot mode , Yes or No or On or Off
		if ((YES == cfg.printerCfg.freq_plot_mode) || (NO == cfg.printerCfg.freq_plot_mode))
		{
			g_helpRecord.freq_plot_mode = cfg.printerCfg.freq_plot_mode;
		}
		else
		{
			returnCode = CFG_ERR_FREQ_PLOT_MODE;
		}


		// Frequency plot mode , Yes or No or On or Off
		switch (cfg.printerCfg.freq_plot_type)
		{
			case FREQ_PLOT_US_BOM_STANDARD:
			case FREQ_PLOT_FRENCH_STANDARD:
			case FREQ_PLOT_DIN_4150_STANDARD:
			case FREQ_PLOT_BRITISH_7385_STANDARD:
			case FREQ_PLOT_SPANISH_STANDARD:
				g_helpRecord.freq_plot_type = cfg.printerCfg.freq_plot_type;
				break;
					
			default:
				returnCode = CFG_ERR_FREQ_PLOT_TYPE;
				break;	
		}

		if (ALARM_MODE_OFF == cfg.alarmCfg.alarm_one_mode)
		{
			g_helpRecord.alarm_one_mode = ALARM_MODE_OFF;
		}
		else
		{
			// Alarm One mode
			switch (cfg.alarmCfg.alarm_one_mode)
			{
				case ALARM_MODE_OFF:
				case ALARM_MODE_SEISMIC:
				case ALARM_MODE_AIR:
				case ALARM_MODE_BOTH:
					g_helpRecord.alarm_one_mode = cfg.alarmCfg.alarm_one_mode;
					break;
						
				default:
					g_helpRecord.alarm_one_mode = ALARM_MODE_OFF;
					returnCode = CFG_ERR_ALARM_ONE_MODE;
					break;	
			}

			g_helpRecord.alarm_one_seismic_lvl = NO_TRIGGER_CHAR;
			g_helpRecord.alarm_one_air_lvl = NO_TRIGGER_CHAR;

			if ((ALARM_MODE_BOTH == g_helpRecord.alarm_one_mode) ||
				(ALARM_MODE_SEISMIC == g_helpRecord.alarm_one_mode))    
			{
				// Alarm One Seismic trigger level check.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarm_one_seismic_lvl) 	||
					((cfg.alarmCfg.alarm_one_seismic_lvl >= g_triggerRecord.trec.seismicTriggerLevel) &&
					  (cfg.alarmCfg.alarm_one_seismic_lvl <= SEISMIC_TRIGGER_MAX_VALUE)))
				{
					g_helpRecord.alarm_one_seismic_lvl = cfg.alarmCfg.alarm_one_seismic_lvl;
				}
				else
				{					
					returnCode = CFG_ERR_ALARM_ONE_SEISMIC_LVL;
				}
			}

			if ((ALARM_MODE_BOTH == g_helpRecord.alarm_one_mode) ||
				(ALARM_MODE_AIR == g_helpRecord.alarm_one_mode))    
			{
				// Alarm One Air trigger level check.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarm_one_air_lvl) 	||
					((cfg.alarmCfg.alarm_one_air_lvl >= g_triggerRecord.trec.soundTriggerLevel) &&
					  (cfg.alarmCfg.alarm_one_air_lvl <= AIR_TRIGGER_MAX_VALUE)))
				{
					g_helpRecord.alarm_one_air_lvl = cfg.alarmCfg.alarm_one_air_lvl;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_ONE_SOUND_LVL;
				}
			}

			// Alarm One Time check
			timeAlarmFloat = (float)((float)cfg.alarmCfg.alarm_one_time/(float)100.0);
			if (	(timeAlarmFloat >= (float)ALARM_OUTPUT_TIME_MIN) && 
				(timeAlarmFloat <= (float)ALARM_OUTPUT_TIME_MAX))
			{
				timeCheck = (uint32)timeAlarmFloat;
				timeCheckFloat = (float)timeAlarmFloat - (float)timeCheck;
				if ((timeCheckFloat == (float)0.0) || (timeCheckFloat == (float)0.5))
				{
					g_helpRecord.alarm_one_time = (float)timeAlarmFloat;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_ONE_TIME;
				}
			}
			else
			{
				returnCode = CFG_ERR_ALARM_ONE_TIME;
			}
		}			



		if (ALARM_MODE_OFF == cfg.alarmCfg.alarm_two_mode)
		{
			g_helpRecord.alarm_two_mode = ALARM_MODE_OFF;
		}
		else
		{

			// Alarm Two mode
			switch (cfg.alarmCfg.alarm_two_mode)
			{
				case ALARM_MODE_OFF:
				case ALARM_MODE_SEISMIC:
				case ALARM_MODE_AIR:
				case ALARM_MODE_BOTH:
					g_helpRecord.alarm_two_mode = cfg.alarmCfg.alarm_two_mode;
					break;
						
				default:
					g_helpRecord.alarm_two_mode = ALARM_MODE_OFF;
					returnCode = CFG_ERR_ALARM_TWO_MODE;
					break;	
			}

			g_helpRecord.alarm_two_seismic_lvl = NO_TRIGGER_CHAR;
			g_helpRecord.alarm_two_air_lvl = NO_TRIGGER_CHAR;

			if ((ALARM_MODE_BOTH == g_helpRecord.alarm_two_mode) ||
				(ALARM_MODE_SEISMIC == g_helpRecord.alarm_two_mode))    
			{
				// Alarm Two Seismic trigger level check.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarm_two_seismic_lvl) 	||
					((cfg.alarmCfg.alarm_two_seismic_lvl >= g_triggerRecord.trec.seismicTriggerLevel) &&
					  (cfg.alarmCfg.alarm_two_seismic_lvl <= SEISMIC_TRIGGER_MAX_VALUE)))
				{
					g_helpRecord.alarm_two_seismic_lvl = cfg.alarmCfg.alarm_two_seismic_lvl;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_SEISMIC_LVL;
				}
			}
			
			if ((ALARM_MODE_BOTH == g_helpRecord.alarm_two_mode) ||
				(ALARM_MODE_AIR == g_helpRecord.alarm_two_mode))    
			{
				// Alarm Two Air trigger level check.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarm_two_air_lvl) 	||
					((cfg.alarmCfg.alarm_two_air_lvl >= g_triggerRecord.trec.soundTriggerLevel) &&
					  (cfg.alarmCfg.alarm_two_air_lvl <= AIR_TRIGGER_MAX_VALUE)))
				{
					g_helpRecord.alarm_two_air_lvl = cfg.alarmCfg.alarm_two_air_lvl;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_SOUND_LVL;
				}
			}

			// Alarm Two Time check
			timeAlarmFloat = (float)((float)cfg.alarmCfg.alarm_two_time/(float)100.0);
			if (	(timeAlarmFloat >= (float)ALARM_OUTPUT_TIME_MIN) && 
				(timeAlarmFloat <= (float)ALARM_OUTPUT_TIME_MAX))
			{
				timeCheck = (uint32)timeAlarmFloat;
				timeCheckFloat = (float)timeAlarmFloat - (float)timeCheck;
				if ((timeCheckFloat == (float)0.0) || (timeCheckFloat == (float)0.5))
				{
					g_helpRecord.alarm_two_time = timeAlarmFloat;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_TIME;
				}
			}
			else
			{
				returnCode = CFG_ERR_ALARM_TWO_TIME;
			}
		}

		if (!((DISABLED == cfg.timerCfg.timer_mode) && (DISABLED == g_helpRecord.timer_mode)))
		{
			timerModeModified = TRUE;
			
			// Timer Mode check
			if (ENABLED == cfg.timerCfg.timer_mode)
			{
				// Timer Mode Frequency check
				switch (cfg.timerCfg.timer_mode_freq)
				{
					case TIMER_MODE_ONE_TIME:
					case TIMER_MODE_DAILY:
					case TIMER_MODE_WEEKDAYS:
					case TIMER_MODE_WEEKLY:
					case TIMER_MODE_MONTHLY:
						break;
					
					default:
						returnCode = CFG_ERR_TIMER_MODE_FREQ;
						break;
				}

				// time valid value check.
				if (!((cfg.timerCfg.timer_start.hour < 24) && 
					 (cfg.timerCfg.timer_start.min  < 60) && 
					 (cfg.timerCfg.timer_start.year < 100) &&
					 (cfg.timerCfg.timer_start.month >= 1) && (cfg.timerCfg.timer_start.month  <= 12) &&
					 (cfg.timerCfg.timer_start.day  >= 1) && (cfg.timerCfg.timer_start.day  < 32)))
				{
					returnCode = CFG_ERR_START_TIME;
				}

				if (!((cfg.timerCfg.timer_stop.hour < 24) && 
					 (cfg.timerCfg.timer_stop.min  < 60) && 
					 (cfg.timerCfg.timer_stop.year < 100) &&
					 (cfg.timerCfg.timer_stop.month >= 1) && (cfg.timerCfg.timer_stop.month  <= 12) &&
					 (cfg.timerCfg.timer_stop.day  >= 1) && (cfg.timerCfg.timer_stop.day  < 32)))
				{
					returnCode = CFG_ERR_STOP_TIME;
				}
				
				if ((CFG_ERR_START_TIME != returnCode) && (CFG_ERR_STOP_TIME != returnCode) &&
					(CFG_ERR_TIMER_MODE_FREQ != returnCode))
				{
					g_helpRecord.timer_mode_freq = cfg.timerCfg.timer_mode_freq;
					g_helpRecord.tm_stop_time.hour = cfg.timerCfg.timer_stop.hour;
					g_helpRecord.tm_stop_time.min = cfg.timerCfg.timer_stop.min;
					g_helpRecord.tm_stop_date.day = cfg.timerCfg.timer_stop.day;
					g_helpRecord.tm_stop_date.month = cfg.timerCfg.timer_stop.month;
					g_helpRecord.tm_stop_date.year = cfg.timerCfg.timer_stop.year;

					g_helpRecord.tm_start_time.hour = cfg.timerCfg.timer_start.hour;
					g_helpRecord.tm_start_time.min = cfg.timerCfg.timer_start.min;
					g_helpRecord.tm_start_date.day = cfg.timerCfg.timer_start.day;
					g_helpRecord.tm_start_date.month = cfg.timerCfg.timer_start.month;
					g_helpRecord.tm_start_date.year = cfg.timerCfg.timer_start.year;

					g_helpRecord.timer_mode = cfg.timerCfg.timer_mode;	
				}					
			}
			else if (DISABLED == cfg.timerCfg.timer_mode)
			{
				// Setting it to disabled, no error check needed.
				g_helpRecord.timer_mode = cfg.timerCfg.timer_mode;
			}
			else
			{	
				// In valid value for the timer mode, return an error
				returnCode = CFG_ERR_TIMER_MODE;
			}
		}
	
		// Check if the Supergraphics
		if (cfg.sgVersion)
		{
			if ((cfg.flashWrapping == NO) || (cfg.flashWrapping == YES))
			{			
				g_helpRecord.flash_wrapping = cfg.flashWrapping;
			}
			else
			{
				// Invalid value for the flash wrapping option
				returnCode = CFG_ERR_FLASH_WRAPPING;
			}
		}

		// Check if it has all been verified and now save the data. Data in error is not saved.
		saveRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}

	if (TRUE == timerModeModified)
	{
		processTimerModeSettings(NO_PROMPT);
		// If the user wants to enable, but it is now disable send an error.
		if (DISABLED == g_helpRecord.timer_mode)
		{
			returnCode = CFG_ERR_TIMER_MODE;
		}
	}
	

	// -------------------------------------
	// Return codes
	sprintf((char*)msgTypeStr, "%02lu", returnCode);
	buildOutgoingSimpleHeaderBuffer((uint8*)ucmHdr, (uint8*)"UCMx", 
		(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	modem_puts((uint8*)&ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);

	// Send Ending Footer
	modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	return;
}

//==================================================
// Function: handleDMM
// Description: Download modem configuration data structure.
//--------------------------------------------------
void handleDMM(CMD_BUFFER_STRUCT* inCmd)
{
	MODEM_SETUP_STRUCT modemCfg;
	uint8 dmmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];

	UNUSED(inCmd);

	byteSet(&modemCfg, 0, sizeof(MODEM_SETUP_STRUCT));

	byteCpy((uint8*)&modemCfg, &g_modemSetupRecord, sizeof(MODEM_SETUP_STRUCT));

	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);
	buildOutgoingSimpleHeaderBuffer((uint8*)dmmHdr, (uint8*)"DMMx", (uint8*)msgTypeStr, 
		(uint32)(MESSAGE_SIMPLE_TOTAL_LENGTH + sizeof(MODEM_SETUP_STRUCT)), COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&dmmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	modem_puts((uint8*)dmmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag);

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&modemCfg, sizeof(MODEM_SETUP_STRUCT), g_transmitCRC);		

	// Send the configuration data
	modem_puts((uint8*)&modemCfg, sizeof(MODEM_SETUP_STRUCT), g_binaryXferFlag);

	// Ending Footer
	modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

//==================================================
// Function: handleUMM
// Description:	Upload modem configuration data structure.
//--------------------------------------------------
void handleUMM(CMD_BUFFER_STRUCT* inCmd)
{
	MODEM_SETUP_STRUCT modemCfg;
	uint8* modemCfgPtr = (uint8*)(&modemCfg);
	uint16 i = 0, j = 0;
	uint32 returnCode = CFG_ERR_NONE;
	uint32 msgCRC = 0;
	uint32 inCRC = 0;
	uint8 ummHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];

	byteSet(&modemCfg, 0, sizeof(MODEM_SETUP_STRUCT));

	// Move the string data into the configuration structure. String is (2 * cfgSize)
	i = MESSAGE_HEADER_SIMPLE_LENGTH;
	while ((i < inCmd->size) && (i < (MESSAGE_HEADER_SIMPLE_LENGTH + (sizeof(MODEM_SETUP_STRUCT) * 2))) && 
			(i < CMD_BUFFER_SIZE))
	{	
		*modemCfgPtr++ = convertAscii2Binary(inCmd->msg[i], inCmd->msg[i + 1]);
		i += 2;
	}		

	// Get the CRC value from the data stream
	while ((i < inCmd->size) && (i < CMD_BUFFER_SIZE) && (j < 4))
	{
		((uint8*)&inCRC)[j++] = convertAscii2Binary(inCmd->msg[i], inCmd->msg[i + 1]);
		i += 2;
	}

	// Compare CRC transmitted CRC value with the CRC calculation on the data and check if not equal
	msgCRC = CalcCCITT32((uint8*)&(inCmd->msg[0]), MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);
	msgCRC = CalcCCITT32((uint8*)&modemCfg, sizeof(MODEM_SETUP_STRUCT), msgCRC);

	if (inCRC != msgCRC)
	{
		// Signal a bad CRC value
		returnCode = CFG_ERR_BAD_CRC;
	}
	else
	{
		if ((modemCfg.modemStatus != NO) && (modemCfg.modemStatus != YES))
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		else if (modemCfg.unlockCode > 9999)
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		else if ((modemCfg.retries < 1) || (modemCfg.retries > 9))
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		else if ((modemCfg.retryTime < 1) || (modemCfg.retryTime > 60))
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		else if (strlen(modemCfg.init) > 64)
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		else if (strlen(modemCfg.dial) > 32)
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		else if (strlen(modemCfg.reset) > 16)
		{
			returnCode = CFG_ERR_MODEM_CONFIG;
		}
		
		if (returnCode == CFG_ERR_NONE)
		{
			g_modemSetupRecord = modemCfg;

			saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
		}
	}

	// -------------------------------------
	// Return codes
	sprintf((char*)msgTypeStr, "%02lu", returnCode);
	buildOutgoingSimpleHeaderBuffer((uint8*)ummHdr, (uint8*)"UMMx", 
		(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&ummHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	modem_puts((uint8*)ummHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);

	// Send Ending Footer
	modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

//==================================================
// Function: convertAscii2Binary
// Description:	convert 2 bytes of ascii into 1 binary data byte
//--------------------------------------------------
uint8 convertAscii2Binary(uint8 firstByte, uint8 secondByte)
{
	uint8 binaryByte = 0;
	uint8 nibble1 = 0;
	uint8 nibble2 = 0;

	// Get the first nibble
	if ((firstByte >= 0x30) && (firstByte <= 0x39))
		nibble1 = (uint8)(firstByte - 0x30);
	else if ((firstByte >= 0x41) && (firstByte <= 0x46))
		nibble1 = (uint8)(firstByte - 0x37);
	
	// Get the second nibble
	if ((secondByte >= 0x30) && (secondByte <= 0x39))
		nibble2 = (uint8)(secondByte - 0x30);
	else if ((secondByte >= 0x41) && (secondByte <= 0x46))
		nibble2 = (uint8)(secondByte - 0x37);

	// Put it into the byte.
	binaryByte = (uint8)((uint8)(nibble1 << 4) + (uint8)nibble2);
	
	return (binaryByte);
}

//==================================================
// Function: handleVTI
// Description: 
// 		View time.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVTI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSTI
// Description: 
// 		Set time.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSTI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVDA
// Description: 
// 		View date.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVDA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSDA
// Description: 
// 		Set date.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSDA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleZRO
// Description: 
// 		Zero sensors.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleZRO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleTTO
// Description: 
// 		Toggle test mode on/off.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleTTO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleCAL
// Description: 
// 		Calibrate sensors with cal pulse.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleCAL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVOL
// Description: 
// 		View on/off log.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVOL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVCG
// Description: 
// 		View command log.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVCG(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVSL
// Description: 
// 		View summary log.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVSL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVEL
// Description: 
// 		View event log.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVEL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVBD
// Description: 
// 		View backlight delay.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSBD
// Description: 
// 		Set backlight delay.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVDT
// Description: 
// 		View display timeout.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSDT
// Description: 
// 		Set display timeout.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVCL
// Description: 
// 		View contrast level.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSCL
// Description: 
// 		Set contrast level.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

void modemInitProcess(void)
{
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

	debug("modemInitProcess\n");

	if (READ_DCD == NO_CONNECTION)
	{
		if (strlen(g_modemSetupRecord.reset) != 0)
		{
			uart_puts((char*)(g_modemSetupRecord.reset), CRAFT_COM_PORT);
			uart_puts((char*)&g_CRLF, CRAFT_COM_PORT);

			soft_usecWait(3 * SOFT_SECS);
		}

		if (strlen(g_modemSetupRecord.init) != 0)
		{
			uart_puts((char*)(g_modemSetupRecord.init), CRAFT_COM_PORT);
			uart_puts((char*)&g_CRLF, CRAFT_COM_PORT);
		}
	}

	// Assume connected.
	g_modemStatus.numberOfRings = 0;
	g_modemStatus.ringIndicator = 0;

	g_modemStatus.connectionState = CONNECTED;
	g_modemStatus.firstConnection = NOP_CMD;
}

//==================================================
//	Procedure: modemResetProcess()
//	Description:
//
//	Input: void
//	Output: none
//--------------------------------------------------
void modemResetProcess(void)
{
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;
	debug("handleMRC\n");

	g_modemStatus.systemIsLockedFlag = YES;

	CLEAR_DTR;

	g_modemResetStage = 1;
	assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(15 * TICKS_PER_SEC), modemResetTimerCallback);
}

//==================================================
// Function: handleMRC
// Description:
// 		Modem reset.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMRS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	g_modemStatus.systemIsLockedFlag = YES;

	if (YES == g_modemSetupRecord.modemStatus)
	{
		modemResetProcess();
	}

	return;
}

//==================================================
// Function: handleMVS
// Description:
// 		Modem view settings.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMVS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMPO
// Description:
// 		Toggle modem on/off.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMPO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMMO
// Description:
// 		Toggle modem mode transmit/receive.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMMO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMNO
// Description:
// 		Toggle modem phone number A/B/C.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMNO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMTO
// Description:
// 		Toggle modem log on/off.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMTO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSD
// Description:
// 		Modem set default initialization string.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSR
// Description:
// 		Modem set receive initialization string.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSR(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMST
// Description:
// 		Modem set transmit initialization string.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMST(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSA
// Description:
// 		Modem set phone number A.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSB
// Description:
// 		Modem set phone number B.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSB(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSC
// Description:
// 		Modem set phone number C.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSC(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMVI
// Description:
// 		Modem view last call in detail.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMVI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMVO
// Description:
// 		Modem view last call out detail.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMVO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

