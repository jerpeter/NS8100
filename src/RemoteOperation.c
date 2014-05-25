///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleAAA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Operating parameter commands
//==================================================

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleDCM(CMD_BUFFER_STRUCT* inCmd)
{
	SYSTEM_CFG cfg;					// 424 bytes, or 848 chars from the pc.
	uint8 dcmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	int majorVer, minorVer;
	char buildVer;

	UNUSED(inCmd);

	byteSet(&cfg, 0, sizeof(SYSTEM_CFG));

	cfg.mode = g_triggerRecord.op_mode;
	cfg.monitorStatus = g_sampleProcessing; 
	cfg.currentTime = getCurrentTime();
	
	cfg.eventCfg.distToSource = (uint32)(g_triggerRecord.trec.dist_to_source * 100.0);
	cfg.eventCfg.weightPerDelay = (uint32)(g_triggerRecord.trec.weight_per_delay * 100.0);
	cfg.eventCfg.sampleRate = (uint16)g_triggerRecord.trec.sample_rate;
	
	// Scan for major and minor version from the app version string and store in the config
	sscanf(&g_buildVersion[0], "%d.%d.%c", &majorVer, &minorVer, &buildVer);
	
	cfg.eventCfg.appMajorVersion = (uint8)majorVer;
	cfg.eventCfg.appMinorVersion = (uint8)minorVer;
	cfg.appBuildVersion = buildVer;

	// Waveform specific - Initial conditions.
#if 0 // Normal
	cfg.eventCfg.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
#else // Bit accuracy adjusted
	if ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.seismicTriggerLevel == EXTERNAL_TRIGGER_CHAR))
	{
		cfg.eventCfg.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
	}
	else
	{
		cfg.eventCfg.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);
	}
#endif

#if 0 // Normal
	cfg.eventCfg.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
#else // Try
	if ((g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.airTriggerLevel == EXTERNAL_TRIGGER_CHAR))
	{
		cfg.eventCfg.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
	}
	else
	{
		cfg.eventCfg.airTriggerLevel = (airTriggerConvert(g_triggerRecord.trec.airTriggerLevel)) / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);
	}
#endif
	cfg.eventCfg.recordTime = g_triggerRecord.trec.record_time;

	// static non changing.
	cfg.eventCfg.seismicSensorType = (uint16)(g_factorySetupRecord.sensor_type);
#if 0 // Port missing change
	cfg.eventCfg.airSensorType = (uint16)0x0;
#else // Updated
	cfg.eventCfg.airSensorType = (uint16)(g_helpRecord.unitsOfAir);
#endif
	cfg.eventCfg.bitAccuracy = g_triggerRecord.trec.bitAccuracy;
	cfg.eventCfg.numOfChannels = 4;
	cfg.eventCfg.aWeighting = (uint8)g_factorySetupRecord.aweight_option;
	cfg.eventCfg.adjustForTempDrift = g_triggerRecord.trec.adjustForTempDrift;
	cfg.eventCfg.numOfSamples = 0;				// Not used for configuration settings

#if 0 // Old and incorrectly overloaded
	cfg.eventCfg.preBuffNumOfSamples = (uint16)g_triggerRecord.srec.sensitivity;
	cfg.eventCfg.calDataNumOfSamples = g_triggerRecord.berec.barScale;
	cfg.eventCfg.activeChannels = g_triggerRecord.berec.barChannel;
#else // Updated with notes
	// *WARNING* Poorly done overloading of this element name with the wrong config type (done by the original author)
	// Not happy about it, but leaving it in place to reduce changes to Dave's Supergraphics - JP
	cfg.eventCfg.preBuffNumOfSamples = (uint16)g_triggerRecord.srec.sensitivity;

	// *WARNING* Poorly done overloading of this element name with the wrong config type (done by the original author)
	// Not happy about it, but leaving it in place to reduce changes to Dave's Supergraphics - JP
	cfg.eventCfg.calDataNumOfSamples = g_triggerRecord.berec.barScale;

	// *WARNING* Poorly done overloading of this element name with the wrong config type (done by the original author)
	// Not happy about it, but leaving it in place to reduce changes to Dave's Supergraphics - JP
	cfg.eventCfg.activeChannels = g_triggerRecord.berec.barChannel;

	// *NOTE* Actual element config type is already incorrectly being overloaded (done by the original author)
	// Using the first element of free space in this structure to transfer the real Pretrigger buffer size config
	cfg.eventCfg.unused[0] = g_helpRecord.pretrigBufferDivider;
#endif

	// Bargraph specific - Initial conditions.
	cfg.eventCfg.barInterval = (uint16)g_triggerRecord.bgrec.barInterval;
	cfg.eventCfg.summaryInterval = (uint16)g_triggerRecord.bgrec.summaryInterval;

	byteCpy((uint8*)cfg.eventCfg.companyName, g_triggerRecord.trec.client, COMPANY_NAME_STRING_SIZE - 2);
	byteCpy((uint8*)cfg.eventCfg.seismicOperator, g_triggerRecord.trec.oper, SEISMIC_OPERATOR_STRING_SIZE - 2);
	byteCpy((uint8*)cfg.eventCfg.sessionLocation, g_triggerRecord.trec.loc, SESSION_LOCATION_STRING_SIZE - 2);
	byteCpy((uint8*)cfg.eventCfg.sessionComments, g_triggerRecord.trec.comments, SESSION_COMMENTS_STRING_SIZE - 2);
	
	cfg.autoCfg.autoMonitorMode = g_helpRecord.autoMonitorMode;
	cfg.autoCfg.autoCalMode = g_helpRecord.autoCalMode;

	cfg.printerCfg.autoPrint = g_helpRecord.autoPrint;
	cfg.printerCfg.languageMode = g_helpRecord.languageMode;
	cfg.printerCfg.vectorSum = g_helpRecord.vectorSum;
	cfg.printerCfg.unitsOfMeasure = g_helpRecord.unitsOfMeasure;
	cfg.printerCfg.freqPlotMode = g_helpRecord.freqPlotMode;
	cfg.printerCfg.freqPlotType = g_helpRecord.freqPlotType;

	cfg.alarmCfg.alarmOneMode = g_helpRecord.alarmOneMode;
	cfg.alarmCfg.alarmTwoMode = g_helpRecord.alarmTwoMode;
	cfg.alarmCfg.alarmOneSeismicLevel = g_helpRecord.alarmOneSeismicLevel;
	cfg.alarmCfg.alarmOneSeismicMinLevel = g_helpRecord.alarmOneSeismicMinLevel;
	cfg.alarmCfg.alarmOneAirLevel = g_helpRecord.alarmOneAirLevel;
	cfg.alarmCfg.alarmOneAirMinLevel = g_helpRecord.alarmOneAirMinLevel;
	cfg.alarmCfg.alarmTwoSeismicLevel = g_helpRecord.alarmTwoSeismicLevel;
	cfg.alarmCfg.alarmTwoSeismicMinLevel = g_helpRecord.alarmTwoSeismicMinLevel;
	cfg.alarmCfg.alarmTwoAirLevel = g_helpRecord.alarmTwoAirLevel;
	cfg.alarmCfg.alarmTwoAirMinLevel = g_helpRecord.alarmTwoAirMinLevel;
	cfg.alarmCfg.alarmOneTime = (uint32)(g_helpRecord.alarmOneTime * (float)100.0);
	cfg.alarmCfg.alarmTwoTime = (uint32)(g_helpRecord.alarmTwoTime * (float)100.0);

	cfg.timerCfg.timerMode = g_helpRecord.timerMode;
	cfg.timerCfg.timerModeFrequency = g_helpRecord.timerModeFrequency;
#if 1 // fix_ns8100 - Size changed to uint32 however this field isn't needed
	cfg.timerCfg.timerModeActiveMinutes = (uint16)g_helpRecord.timerModeActiveMinutes;
#endif

	if (DISABLED == g_helpRecord.timerMode)
	{
		cfg.timerCfg.timer_stop = cfg.timerCfg.timer_start = getCurrentTime();
	}
	else
	{
		cfg.timerCfg.timer_start.hour = g_helpRecord.timerStartTime.hour;
		cfg.timerCfg.timer_start.min = g_helpRecord.timerStartTime.min;
		cfg.timerCfg.timer_start.sec = g_helpRecord.timerStartTime.sec;
		cfg.timerCfg.timer_start.day = g_helpRecord.timerStartDate.day;
		cfg.timerCfg.timer_start.month = g_helpRecord.timerStartDate.month;
		cfg.timerCfg.timer_start.year = g_helpRecord.timerStartDate.year;

		cfg.timerCfg.timer_stop.hour = g_helpRecord.timerStopTime.hour;
		cfg.timerCfg.timer_stop.min = g_helpRecord.timerStopTime.min;
		cfg.timerCfg.timer_stop.sec = g_helpRecord.timerStopTime.sec;
		cfg.timerCfg.timer_stop.day = g_helpRecord.timerStopDate.day;
		cfg.timerCfg.timer_stop.month = g_helpRecord.timerStopDate.month;
		cfg.timerCfg.timer_stop.year = g_helpRecord.timerStopDate.year;
	}
	
	// Add in the flash wrapping option
	cfg.flashWrapping = g_helpRecord.flashWrapping;

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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

		// Check to see if the incoming message is the correct size
		if ((uint32)((inCmd->size - 16)/2) < sizeOfCfg)
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
				g_helpRecord.timerMode = DISABLED;
				// Disable Power Off protection
				powerControl(POWER_OFF_PROTECTION_ENABLE, OFF);
				
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
				g_helpRecord.timerMode = DISABLED;
				
				// Disable Power Off protection
				powerControl(POWER_OFF_PROTECTION_ENABLE, OFF);
				
				// Disable the Power Off timer if it's set
				clearSoftTimer(POWER_OFF_TIMER_NUM);
			}

			// Update the current time.
			updateCurrentTime();
		}

		//--------------------------------
		// Distance to source check, cfg is in uint32 format not float.
		//--------------------------------
		if (cfg.eventCfg.distToSource > (uint32)(DISTANCE_TO_SOURCE_MAX_VALUE * 100))
		{
			returnCode = CFG_ERR_DIST_TO_SRC;
		}
		else
		{
			g_triggerRecord.trec.dist_to_source = (float)((float)cfg.eventCfg.distToSource / (float)100.0);
		}

		//--------------------------------
		// Weight per delay check, cfg is in uint32 format not float.
		//--------------------------------
		if (cfg.eventCfg.weightPerDelay > (uint32)(WEIGHT_PER_DELAY_MAX_VALUE * 100))
		{
			returnCode = CFG_ERR_WEIGHT_DELAY;
		}
		else
		{
			g_triggerRecord.trec.weight_per_delay = (float)((float)cfg.eventCfg.weightPerDelay / (float)100.0);
		}

		//--------------------------------
		// Sample Rate check
		//--------------------------------
		if ((SAMPLE_RATE_512 == cfg.eventCfg.sampleRate) || (SAMPLE_RATE_1K == cfg.eventCfg.sampleRate) || 
			(SAMPLE_RATE_2K == cfg.eventCfg.sampleRate) || (SAMPLE_RATE_4K == cfg.eventCfg.sampleRate) || 
			(SAMPLE_RATE_8K == cfg.eventCfg.sampleRate) || (SAMPLE_RATE_16K == cfg.eventCfg.sampleRate))
		{
			if (((BARGRAPH_MODE == g_triggerRecord.op_mode) || (COMBO_MODE == g_triggerRecord.op_mode)) && 
				(cfg.eventCfg.sampleRate > SAMPLE_RATE_4K))
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

		//--------------------------------
		// Seismic Trigger Level check
		//--------------------------------
		if ((MANUAL_TRIGGER_CHAR == cfg.eventCfg.seismicTriggerLevel) 	||
			(NO_TRIGGER_CHAR == cfg.eventCfg.seismicTriggerLevel) 	||
			((cfg.eventCfg.seismicTriggerLevel >= SEISMIC_TRIGGER_MIN_VALUE) &&
			  (cfg.eventCfg.seismicTriggerLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
		{
			g_triggerRecord.trec.seismicTriggerLevel = cfg.eventCfg.seismicTriggerLevel;
			g_helpRecord.alarmOneSeismicMinLevel = g_triggerRecord.trec.seismicTriggerLevel;
			g_helpRecord.alarmTwoSeismicMinLevel = g_triggerRecord.trec.seismicTriggerLevel;
		}
		else
		{
			returnCode = CFG_ERR_SEISMIC_TRIG_LVL;
		}
		
#if 1 // Updated (Port missing change)
		//--------------------------------
		// Update air sensor type DB or MB
		//--------------------------------
		if ((uint8)cfg.eventCfg.airSensorType == MILLIBAR_TYPE)
		{
			g_helpRecord.unitsOfAir = MILLIBAR_TYPE;
		}
		else
		{
			g_helpRecord.unitsOfAir = DECIBEL_TYPE;
		}
#endif

		//--------------------------------
		// A-weighting check
		//--------------------------------
		if ((cfg.eventCfg.aWeighting == YES) || (cfg.eventCfg.aWeighting == NO))
		{
			g_factorySetupRecord.aweight_option = cfg.eventCfg.aWeighting;
		}
		else
		{
			returnCode = CFG_ERR_A_WEIGHTING;
		}

		//--------------------------------
		// Sound Trigger Level check
		//--------------------------------
		if ((MANUAL_TRIGGER_CHAR == cfg.eventCfg.airTriggerLevel) 	||
			(NO_TRIGGER_CHAR == cfg.eventCfg.airTriggerLevel) 	||
			((cfg.eventCfg.airTriggerLevel >= AIR_TRIGGER_MIN_VALUE) &&
#if 0 // Port lost change
			(cfg.eventCfg.airTriggerLevel <= AIR_TRIGGER_MAX_VALUE)))
#else // Updated
			(cfg.eventCfg.airTriggerLevel <= (uint32)AIR_TRIGGER_MB_MAX_VALUE))) // fix_ns8100 - Check DB/MB separately
#endif
		{
			g_triggerRecord.trec.airTriggerLevel = cfg.eventCfg.airTriggerLevel;
			g_helpRecord.alarmOneAirMinLevel = g_triggerRecord.trec.airTriggerLevel;
			g_helpRecord.alarmTwoAirMinLevel = g_triggerRecord.trec.airTriggerLevel;
		}
		else
		{
			returnCode = CFG_ERR_SOUND_TRIG_LVL;
		}

		//--------------------------------
		// Find Max Record time for a given sample rate. Max time is equal to
		// Size of the (event buff - size of Pretrigger buffer - size of calibration buff) 
		// divided by (sample rate * number of channels).
		// Number of channels is hard coded to 4 = NUMBER_OF_CHANNELS_DEFAULT 
		//--------------------------------
		maxRecordTime = (uint16)(((uint32)((EVENT_BUFF_SIZE_IN_WORDS - 
#if 0 // Fixed Pretrigger size
			((g_triggerRecord.trec.sample_rate / 4) * NUMBER_OF_CHANNELS_DEFAULT) - 
#else // Variable Pretrigger size
				((g_triggerRecord.trec.sample_rate / g_helpRecord.pretrigBufferDivider) * NUMBER_OF_CHANNELS_DEFAULT) -
#endif
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

#if 0 // Old and incorrectly overloaded
		if ((LOW == cfg.eventCfg.preBuffNumOfSamples) || (HIGH == cfg.eventCfg.preBuffNumOfSamples))
		{
			g_triggerRecord.srec.sensitivity = cfg.eventCfg.preBuffNumOfSamples; // Sensitivity
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
#else // Updated with notes
		// *WARNING* Poorly done overloading of this element name with the wrong config type (done by the original author)
		// Not happy about it, but leaving it in place to reduce changes to Dave's Supergraphics - JP
		if ((cfg.eventCfg.preBuffNumOfSamples == LOW) || (cfg.eventCfg.preBuffNumOfSamples == HIGH))
		{
			g_triggerRecord.srec.sensitivity = cfg.eventCfg.preBuffNumOfSamples; // Sensitivity
		}
		else
		{
			returnCode = CFG_ERR_SENSITIVITY;
		}
		
		// *WARNING* Poorly done overloading of this element name with the wrong config type (done by the original author)
		// Not happy about it, but leaving it in place to reduce changes to Dave's Supergraphics - JP
		if ((1 == cfg.eventCfg.calDataNumOfSamples) || (2 == cfg.eventCfg.calDataNumOfSamples) ||
			(4 == cfg.eventCfg.calDataNumOfSamples) || (8 == cfg.eventCfg.calDataNumOfSamples))
		{
			g_triggerRecord.berec.barScale = (uint8)cfg.eventCfg.calDataNumOfSamples;
		}
		else
		{
			returnCode = CFG_ERR_SCALING;
		}

		// *WARNING* Poorly done overloading of this element name with the wrong config type (done by the original author)
		// Not happy about it, but leaving it in place to reduce changes to Dave's Supergraphics - JP
		if (cfg.eventCfg.activeChannels <= BAR_AIR_CHANNEL) //if ((cfg.eventCfg.activeChannels >= 0) && (cfg.eventCfg.activeChannels <= BAR_AIR_CHANNEL))
		{
			g_triggerRecord.berec.barChannel = cfg.eventCfg.activeChannels;
		}
		else
		{
			returnCode = CFG_ERR_BAR_PRINT_CHANNEL;
		}

		// *NOTE* Actual element config type is already incorrectly being overloaded (done by the original author)
		// Using the first element of free space in this structure to transfer the real Pretrigger buffer size config
		if ((cfg.eventCfg.unused[0] == PRETRIGGER_BUFFER_QUARTER_SEC_DIV) || (cfg.eventCfg.unused[0] == PRETRIGGER_BUFFER_HALF_SEC_DIV) ||
			(cfg.eventCfg.unused[0] == PRETRIGGER_BUFFER_FULL_SEC_DIV))
		{
			g_helpRecord.pretrigBufferDivider = cfg.eventCfg.unused[0];
		}
		else
		{
			returnCode = CFG_ERR_PRETRIG_BUFFER_DIV;
		}
#endif

		if ((cfg.eventCfg.bitAccuracy == ACCURACY_10_BIT) || (cfg.eventCfg.bitAccuracy == ACCURACY_12_BIT) || 
			(cfg.eventCfg.bitAccuracy == ACCURACY_14_BIT) || (cfg.eventCfg.bitAccuracy == ACCURACY_16_BIT))
		{
			g_triggerRecord.trec.bitAccuracy = cfg.eventCfg.bitAccuracy;
		}
		else
		{
			returnCode = CFG_ERR_BIT_ACCURACY;
		}
		
		if ((cfg.eventCfg.adjustForTempDrift == YES) || (cfg.eventCfg.adjustForTempDrift == NO))
		{
			g_triggerRecord.trec.adjustForTempDrift = cfg.eventCfg.adjustForTempDrift;
		}
		else
		{
			returnCode = CFG_ERR_TEMP_ADJUST;
		}

		// Auto Monitor Mode check
		switch (cfg.autoCfg.autoMonitorMode)
		{
			case AUTO_TWO_MIN_TIMEOUT:
			case AUTO_THREE_MIN_TIMEOUT:
			case AUTO_FOUR_MIN_TIMEOUT:
			case AUTO_NO_TIMEOUT:
				g_helpRecord.autoMonitorMode = cfg.autoCfg.autoMonitorMode;
				break;
				
			default:
				returnCode = CFG_ERR_AUTO_MON_MODE;
				break;
		}

		// Auto Monitor Mode check
		switch (cfg.autoCfg.autoCalMode)
		{	
			case AUTO_24_HOUR_TIMEOUT:
			case AUTO_48_HOUR_TIMEOUT:
			case AUTO_72_HOUR_TIMEOUT:
			case AUTO_NO_CAL_TIMEOUT:
				g_helpRecord.autoCalMode = cfg.autoCfg.autoCalMode;
				break;
				
			default:
				returnCode = CFG_ERR_AUTO_CAL_MODE;
				break;
		}

		// Auto print
		if ((YES == cfg.printerCfg.autoPrint) || (NO == cfg.printerCfg.autoPrint))
		{
			if (SUPERGRAPH_UNIT)
			{
				g_helpRecord.autoPrint = cfg.printerCfg.autoPrint;
			}
		}
		else
		{
			returnCode = CFG_ERR_AUTO_PRINT;
		}
		
		// Language Mode	
		switch (cfg.printerCfg.languageMode)
		{
			case ENGLISH_LANG:
			case FRENCH_LANG:
			case ITALIAN_LANG:
			case GERMAN_LANG:
#if 1 // Updated (Port missing change)
			case SPANISH_LANG:
#endif
				g_helpRecord.languageMode = cfg.printerCfg.languageMode;
				build_languageLinkTable(g_helpRecord.languageMode);
				break;
					
			default:
				returnCode = CFG_ERR_LANGUAGE_MODE;
				break;	
		}

		// Vector Sum check
		if ((YES == cfg.printerCfg.vectorSum) || (NO == cfg.printerCfg.vectorSum))
		{
			g_helpRecord.vectorSum = cfg.printerCfg.vectorSum;
		}
		else
		{
			returnCode = CFG_ERR_VECTOR_SUM;
		}
		
		// Units of Measure check
		if ((IMPERIAL_TYPE == cfg.printerCfg.unitsOfMeasure) || (METRIC_TYPE == cfg.printerCfg.unitsOfMeasure))
		{
			g_helpRecord.unitsOfMeasure = cfg.printerCfg.unitsOfMeasure;
		}
		else
		{
			returnCode = CFG_ERR_UNITS_OF_MEASURE;
		}
		
#if 0 // fix_ns8100
		// Units of Air check
		if ((DECIBEL_TYPE == cfg.printerCfg.unitsOfMeasure) || (MILLIBAR_TYPE == cfg.printerCfg.unitsOfMeasure))
		{
			g_helpRecord.unitsOfAir = cfg.printerCfg.unitsOfAir;
		}
		else
		{
			returnCode = CFG_ERR_UNITS_OF_MEASURE; // fix_ns8100 - Create CFG_ERR_UNITS_OF_AIR
		}
#endif

		// Frequency plot mode , Yes or No or On or Off
		if ((YES == cfg.printerCfg.freqPlotMode) || (NO == cfg.printerCfg.freqPlotMode))
		{
			g_helpRecord.freqPlotMode = cfg.printerCfg.freqPlotMode;
		}
		else
		{
			returnCode = CFG_ERR_FREQ_PLOT_MODE;
		}


		// Frequency plot mode , Yes or No or On or Off
		switch (cfg.printerCfg.freqPlotType)
		{
			case FREQ_PLOT_US_BOM_STANDARD:
			case FREQ_PLOT_FRENCH_STANDARD:
			case FREQ_PLOT_DIN_4150_STANDARD:
			case FREQ_PLOT_BRITISH_7385_STANDARD:
			case FREQ_PLOT_SPANISH_STANDARD:
				g_helpRecord.freqPlotType = cfg.printerCfg.freqPlotType;
				break;
					
			default:
				returnCode = CFG_ERR_FREQ_PLOT_TYPE;
				break;	
		}

		if (ALARM_MODE_OFF == cfg.alarmCfg.alarmOneMode)
		{
			g_helpRecord.alarmOneMode = ALARM_MODE_OFF;
		}
		else
		{
			// Alarm One mode
			switch (cfg.alarmCfg.alarmOneMode)
			{
				case ALARM_MODE_OFF:
				case ALARM_MODE_SEISMIC:
				case ALARM_MODE_AIR:
				case ALARM_MODE_BOTH:
					g_helpRecord.alarmOneMode = cfg.alarmCfg.alarmOneMode;
					break;
						
				default:
					g_helpRecord.alarmOneMode = ALARM_MODE_OFF;
					returnCode = CFG_ERR_ALARM_ONE_MODE;
					break;	
			}

			g_helpRecord.alarmOneSeismicLevel = NO_TRIGGER_CHAR;
			g_helpRecord.alarmOneAirLevel = NO_TRIGGER_CHAR;

			if ((ALARM_MODE_BOTH == g_helpRecord.alarmOneMode) ||
				(ALARM_MODE_SEISMIC == g_helpRecord.alarmOneMode))
			{
				// Alarm One Seismic trigger level check.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmOneSeismicLevel) 	||
					((cfg.alarmCfg.alarmOneSeismicLevel >= g_triggerRecord.trec.seismicTriggerLevel) &&
					  (cfg.alarmCfg.alarmOneSeismicLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
				{
					g_helpRecord.alarmOneSeismicLevel = cfg.alarmCfg.alarmOneSeismicLevel;
				}
				else
				{					
					returnCode = CFG_ERR_ALARM_ONE_SEISMIC_LVL;
				}
			}

			if ((ALARM_MODE_BOTH == g_helpRecord.alarmOneMode) || (ALARM_MODE_AIR == g_helpRecord.alarmOneMode))
			{
	            // Alarm One Air trigger level check DB/MB.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmOneAirLevel) 	||
					((cfg.alarmCfg.alarmOneAirLevel >= g_triggerRecord.trec.airTriggerLevel) &&
#if 0 // Port lost change					  
					(cfg.alarmCfg.alarmOneAirLevel <= AIR_TRIGGER_MAX_VALUE)))
#else // Updated
					(cfg.alarmCfg.alarmOneAirLevel <= AIR_TRIGGER_MB_MAX_VALUE))) // fix_ns8100 - Check DB/MB separately
#endif
				{
					g_helpRecord.alarmOneAirLevel = cfg.alarmCfg.alarmOneAirLevel;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_ONE_SOUND_LVL;
				}
			}

			// Alarm One Time check
			timeAlarmFloat = (float)((float)cfg.alarmCfg.alarmOneTime/(float)100.0);
			if (	(timeAlarmFloat >= (float)ALARM_OUTPUT_TIME_MIN) && 
				(timeAlarmFloat <= (float)ALARM_OUTPUT_TIME_MAX))
			{
				timeCheck = (uint32)timeAlarmFloat;
				timeCheckFloat = (float)timeAlarmFloat - (float)timeCheck;
				if ((timeCheckFloat == (float)0.0) || (timeCheckFloat == (float)0.5))
				{
					g_helpRecord.alarmOneTime = (float)timeAlarmFloat;
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



		if (ALARM_MODE_OFF == cfg.alarmCfg.alarmTwoMode)
		{
			g_helpRecord.alarmTwoMode = ALARM_MODE_OFF;
		}
		else
		{

			// Alarm Two mode
			switch (cfg.alarmCfg.alarmTwoMode)
			{
				case ALARM_MODE_OFF:
				case ALARM_MODE_SEISMIC:
				case ALARM_MODE_AIR:
				case ALARM_MODE_BOTH:
					g_helpRecord.alarmTwoMode = cfg.alarmCfg.alarmTwoMode;
					break;
						
				default:
					g_helpRecord.alarmTwoMode = ALARM_MODE_OFF;
					returnCode = CFG_ERR_ALARM_TWO_MODE;
					break;	
			}

			g_helpRecord.alarmTwoSeismicLevel = NO_TRIGGER_CHAR;
			g_helpRecord.alarmTwoAirLevel = NO_TRIGGER_CHAR;

			if ((ALARM_MODE_BOTH == g_helpRecord.alarmTwoMode) ||
				(ALARM_MODE_SEISMIC == g_helpRecord.alarmTwoMode))
			{
				// Alarm Two Seismic trigger level check.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmTwoSeismicLevel) 	||
					((cfg.alarmCfg.alarmTwoSeismicLevel >= g_triggerRecord.trec.seismicTriggerLevel) &&
					  (cfg.alarmCfg.alarmTwoSeismicLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
				{
					g_helpRecord.alarmTwoSeismicLevel = cfg.alarmCfg.alarmTwoSeismicLevel;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_SEISMIC_LVL;
				}
			}
			
			if ((ALARM_MODE_BOTH == g_helpRecord.alarmTwoMode) || (ALARM_MODE_AIR == g_helpRecord.alarmTwoMode))
			{
	            // Alarm Two Air trigger level check DB/MB.
				if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmTwoAirLevel) 	||
					((cfg.alarmCfg.alarmTwoAirLevel >= g_triggerRecord.trec.airTriggerLevel) &&
#if 0 // Port lost change
					(cfg.alarmCfg.alarmTwoAirLevel <= AIR_TRIGGER_MAX_VALUE)))
#else // Updated
					(cfg.alarmCfg.alarmTwoAirLevel <= AIR_TRIGGER_MB_MAX_VALUE))) // fix_ns8100 - Check DB/MB separately
#endif
				{
					g_helpRecord.alarmTwoAirLevel = cfg.alarmCfg.alarmTwoAirLevel;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_SOUND_LVL;
				}
			}

			// Alarm Two Time check
			timeAlarmFloat = (float)((float)cfg.alarmCfg.alarmTwoTime/(float)100.0);
			if (	(timeAlarmFloat >= (float)ALARM_OUTPUT_TIME_MIN) && 
				(timeAlarmFloat <= (float)ALARM_OUTPUT_TIME_MAX))
			{
				timeCheck = (uint32)timeAlarmFloat;
				timeCheckFloat = (float)timeAlarmFloat - (float)timeCheck;
				if ((timeCheckFloat == (float)0.0) || (timeCheckFloat == (float)0.5))
				{
					g_helpRecord.alarmTwoTime = timeAlarmFloat;
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

		if (!((DISABLED == cfg.timerCfg.timerMode) && (DISABLED == g_helpRecord.timerMode)))
		{
			timerModeModified = TRUE;
			
			// Timer Mode check
			if (ENABLED == cfg.timerCfg.timerMode)
			{
				// Timer Mode Frequency check
				switch (cfg.timerCfg.timerModeFrequency)
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
					g_helpRecord.timerModeFrequency = cfg.timerCfg.timerModeFrequency;
					g_helpRecord.timerStopTime.hour = cfg.timerCfg.timer_stop.hour;
					g_helpRecord.timerStopTime.min = cfg.timerCfg.timer_stop.min;
					g_helpRecord.timerStopDate.day = cfg.timerCfg.timer_stop.day;
					g_helpRecord.timerStopDate.month = cfg.timerCfg.timer_stop.month;
					g_helpRecord.timerStopDate.year = cfg.timerCfg.timer_stop.year;

					g_helpRecord.timerStartTime.hour = cfg.timerCfg.timer_start.hour;
					g_helpRecord.timerStartTime.min = cfg.timerCfg.timer_start.min;
					g_helpRecord.timerStartDate.day = cfg.timerCfg.timer_start.day;
					g_helpRecord.timerStartDate.month = cfg.timerCfg.timer_start.month;
					g_helpRecord.timerStartDate.year = cfg.timerCfg.timer_start.year;

					g_helpRecord.timerMode = cfg.timerCfg.timerMode;
				}					
			}
			else if (DISABLED == cfg.timerCfg.timerMode)
			{
				// Setting it to disabled, no error check needed.
				g_helpRecord.timerMode = cfg.timerCfg.timerMode;
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
				g_helpRecord.flashWrapping = cfg.flashWrapping;
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
		saveRecData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);
	}

	if (TRUE == timerModeModified)
	{
		processTimerModeSettings(NO_PROMPT);
		// If the user wants to enable, but it is now disable send an error.
		if (DISABLED == g_helpRecord.timerMode)
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVTI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleSTI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVDA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleSDA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleZRO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleTTO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleCAL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVOL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVCG(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVSL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVEL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleSBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleSDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleVCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleSCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void modemResetProcess(void)
{
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;
	debug("handleMRC\n");

	g_modemStatus.systemIsLockedFlag = YES;

	if(g_autoRetries == 0)
	{
		CLEAR_DTR;
	}	

	g_modemResetStage = 1;
	assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(15 * TICKS_PER_SEC), modemResetTimerCallback);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMRS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	g_modemStatus.systemIsLockedFlag = YES;

	if (YES == g_modemSetupRecord.modemStatus)
	{
		g_autoRetries = 0;
		modemResetProcess();
	}

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMVS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMPO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMMO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMNO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMTO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMSD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMSR(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMST(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMSA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMSB(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMSC(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMVI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void handleMVO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

