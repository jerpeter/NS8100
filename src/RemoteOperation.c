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
void HandleAAA(CMD_BUFFER_STRUCT* inCmd)
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
void HandleDCM(CMD_BUFFER_STRUCT* inCmd)
{
	SYSTEM_CFG cfg;					// 424 bytes, or 848 chars from the pc.
	uint8 dcmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	int majorVer, minorVer;
	char buildVer;

	UNUSED(inCmd);

	memset(&cfg, 0, sizeof(cfg));

	cfg.mode = g_triggerRecord.opMode;
	cfg.monitorStatus = g_sampleProcessing; 
	cfg.currentTime = GetCurrentTime();
	
	cfg.eventCfg.distToSource = (uint32)(g_triggerRecord.trec.dist_to_source * 100.0);
	cfg.eventCfg.weightPerDelay = (uint32)(g_triggerRecord.trec.weight_per_delay * 100.0);
	cfg.eventCfg.sampleRate = (uint16)g_triggerRecord.trec.sample_rate;
	
	// Scan for major and minor version from the app version string and store in the config
	sscanf(&g_buildVersion[0], "%d.%d.%c", &majorVer, &minorVer, &buildVer);
	
	cfg.eventCfg.appMajorVersion = (uint8)majorVer;
	cfg.eventCfg.appMinorVersion = (uint8)minorVer;
	cfg.appBuildVersion = buildVer;

	// Waveform specific - Initial conditions.
#if 1 // Original - Fixed method (Dave prefers trigger level as 16-bit regardless of bit accuracy setting)
	cfg.eventCfg.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
#else // New - Bit accuracy adjusted
	if ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.seismicTriggerLevel == EXTERNAL_TRIGGER_CHAR))
	{
		cfg.eventCfg.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
	}
	else
	{
		cfg.eventCfg.seismicTriggerLevel = (g_triggerRecord.trec.seismicTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint));
	}
#endif

#if 1 // Original - Fixed method (Dave prefers trigger level as 16-bit regardless of bit accuracy setting)
	cfg.eventCfg.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
#else // New - Bit accuracy adjusted
	if ((g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.airTriggerLevel == EXTERNAL_TRIGGER_CHAR))
	{
		cfg.eventCfg.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
	}
	else
	{
		cfg.eventCfg.airTriggerLevel = (g_triggerRecord.trec.airTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint));
	}
#endif
	cfg.eventCfg.recordTime = g_triggerRecord.trec.record_time;

	// static non changing.
	cfg.eventCfg.seismicSensorType = (uint16)(g_factorySetupRecord.sensor_type);
	cfg.eventCfg.airSensorType = SENSOR_MICROPHONE;

	cfg.eventCfg.bitAccuracy = g_triggerRecord.trec.bitAccuracy;
	cfg.eventCfg.numOfChannels = NUMBER_OF_CHANNELS_DEFAULT;
	cfg.eventCfg.adjustForTempDrift = g_triggerRecord.trec.adjustForTempDrift;
	cfg.eventCfg.pretrigBufferDivider = g_unitConfig.pretrigBufferDivider;
	cfg.eventCfg.numOfSamples = 0;				// Not used for configuration settings

	if ((g_factorySetupRecord.aweight_option == ENABLED) && (g_unitConfig.airScale == AIR_SCALE_LINEAR))
	{
		// Need to signal remote side that current setting is linear but that A-weighting is available
		cfg.eventCfg.aWeighting = (AIR_SCALE_LINEAR | (cfg.eventCfg.aWeighting << cfg.eventCfg.aWeighting));
	}
	else { cfg.eventCfg.aWeighting = g_factorySetupRecord.aweight_option; }

	cfg.eventCfg.preBuffNumOfSamples = (g_triggerRecord.trec.sample_rate / g_unitConfig.pretrigBufferDivider);
	cfg.eventCfg.calDataNumOfSamples = CALIBRATION_NUMBER_OF_SAMPLES;
	cfg.eventCfg.activeChannels = NUMBER_OF_CHANNELS_DEFAULT;

	// Overloaded elements rightfully in their own place
	cfg.extraUnitCfg.sensitivity = (uint8)g_triggerRecord.srec.sensitivity;
	cfg.extraUnitCfg.barScale = g_triggerRecord.berec.barScale;
	cfg.extraUnitCfg.barChannel = g_triggerRecord.berec.barChannel;

	// Needed for events but not remote config
	cfg.eventCfg.seismicUnitsOfMeasure = g_unitConfig.unitsOfMeasure;
	cfg.eventCfg.airUnitsOfMeasure = g_unitConfig.unitsOfAir;
	cfg.eventCfg.airTriggerInUnits = 0x0; // Not needed
	cfg.eventCfg.seismicTriggerInUnits = 0x0; // Not needed

	// Bargraph specific - Initial conditions.
	cfg.eventCfg.barInterval = (uint16)g_triggerRecord.bgrec.barInterval;
	cfg.eventCfg.summaryInterval = (uint16)g_triggerRecord.bgrec.summaryInterval;

	memcpy((uint8*)cfg.eventCfg.companyName, g_triggerRecord.trec.client, COMPANY_NAME_STRING_SIZE - 2);
	memcpy((uint8*)cfg.eventCfg.seismicOperator, g_triggerRecord.trec.oper, SEISMIC_OPERATOR_STRING_SIZE - 2);
	memcpy((uint8*)cfg.eventCfg.sessionLocation, g_triggerRecord.trec.loc, SESSION_LOCATION_STRING_SIZE - 2);
	memcpy((uint8*)cfg.eventCfg.sessionComments, g_triggerRecord.trec.comments, SESSION_COMMENTS_STRING_SIZE - 2);
	
	cfg.autoCfg.autoMonitorMode = g_unitConfig.autoMonitorMode;
	cfg.autoCfg.autoCalMode = g_unitConfig.autoCalMode;
	cfg.autoCfg.externalTrigger = g_unitConfig.externalTrigger;
	cfg.autoCfg.rs232PowerSavings = g_unitConfig.rs232PowerSavings;

	cfg.extraUnitCfg.autoPrint = g_unitConfig.autoPrint;
	cfg.extraUnitCfg.languageMode = g_unitConfig.languageMode;
	cfg.extraUnitCfg.vectorSum = g_unitConfig.vectorSum;
	cfg.extraUnitCfg.unitsOfMeasure = g_unitConfig.unitsOfMeasure;
	cfg.extraUnitCfg.unitsOfAir = g_unitConfig.unitsOfAir;
	cfg.extraUnitCfg.freqPlotMode = g_unitConfig.freqPlotMode;
	cfg.extraUnitCfg.freqPlotType = g_unitConfig.freqPlotType;

	cfg.alarmCfg.alarmOneMode = g_unitConfig.alarmOneMode;
	cfg.alarmCfg.alarmOneSeismicLevel = g_unitConfig.alarmOneSeismicLevel;
	cfg.alarmCfg.alarmOneSeismicMinLevel = g_unitConfig.alarmOneSeismicMinLevel;
	cfg.alarmCfg.alarmOneAirLevel = g_unitConfig.alarmOneAirLevel;
	cfg.alarmCfg.alarmOneAirMinLevel = g_unitConfig.alarmOneAirMinLevel;
	cfg.alarmCfg.alarmOneTime = (uint32)(g_unitConfig.alarmOneTime * (float)100.0);

	cfg.alarmCfg.alarmTwoMode = g_unitConfig.alarmTwoMode;
	cfg.alarmCfg.alarmTwoSeismicLevel = g_unitConfig.alarmTwoSeismicLevel;
	cfg.alarmCfg.alarmTwoSeismicMinLevel = g_unitConfig.alarmTwoSeismicMinLevel;
	cfg.alarmCfg.alarmTwoAirLevel = g_unitConfig.alarmTwoAirLevel;
	cfg.alarmCfg.alarmTwoAirMinLevel = g_unitConfig.alarmTwoAirMinLevel;
	cfg.alarmCfg.alarmTwoTime = (uint32)(g_unitConfig.alarmTwoTime * (float)100.0);

	cfg.timerCfg.timerMode = g_unitConfig.timerMode;
	cfg.timerCfg.timerModeFrequency = g_unitConfig.timerModeFrequency;
#if 1 // fix_ns8100 - Size changed to uint32 however this field isn't needed
	cfg.timerCfg.TimerModeActiveMinutes = (uint16)g_unitConfig.TimerModeActiveMinutes;
#endif

	if (DISABLED == g_unitConfig.timerMode)
	{
		cfg.timerCfg.timer_stop = cfg.timerCfg.timer_start = GetCurrentTime();
	}
	else
	{
		cfg.timerCfg.timer_start.hour = g_unitConfig.timerStartTime.hour;
		cfg.timerCfg.timer_start.min = g_unitConfig.timerStartTime.min;
		cfg.timerCfg.timer_start.sec = g_unitConfig.timerStartTime.sec;
		cfg.timerCfg.timer_start.day = g_unitConfig.timerStartDate.day;
		cfg.timerCfg.timer_start.month = g_unitConfig.timerStartDate.month;
		cfg.timerCfg.timer_start.year = g_unitConfig.timerStartDate.year;

		cfg.timerCfg.timer_stop.hour = g_unitConfig.timerStopTime.hour;
		cfg.timerCfg.timer_stop.min = g_unitConfig.timerStopTime.min;
		cfg.timerCfg.timer_stop.sec = g_unitConfig.timerStopTime.sec;
		cfg.timerCfg.timer_stop.day = g_unitConfig.timerStopDate.day;
		cfg.timerCfg.timer_stop.month = g_unitConfig.timerStopDate.month;
		cfg.timerCfg.timer_stop.year = g_unitConfig.timerStopDate.year;
	}
	
	// Add in the flash wrapping option
#if 0 // Normal
	cfg.flashWrapping = g_unitConfig.flashWrapping;
#else // Forcing flash wrapping to be disabled
	cfg.flashWrapping = NO;
#endif

	// Spare fields, just use as a data marker
	cfg.unused[0] = 0x0A;
	cfg.unused[1] = 0x0B;
	cfg.unused[2] = 0x0C;
	cfg.unused[3] = 0x0D;
	cfg.unused[4] = 0x0E;
	cfg.unused[5] = 0x0F;

	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);
	BuildOutgoingSimpleHeaderBuffer((uint8*)dcmHdr, (uint8*)"DCMx", (uint8*)msgTypeStr,
		(uint32)(MESSAGE_SIMPLE_TOTAL_LENGTH + sizeof(SYSTEM_CFG)), COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&dcmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	ModemPuts((uint8*)dcmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag);

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&cfg, sizeof(SYSTEM_CFG), g_transmitCRC);		

	// Send the configuration data
	ModemPuts((uint8*)&cfg, sizeof(SYSTEM_CFG), g_binaryXferFlag);

	// Ending Footer
	ModemPuts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleUCM(CMD_BUFFER_STRUCT* inCmd)
{
	SYSTEM_CFG cfg;
	uint8* cfgPtr = (uint8*)(&cfg);
	uint8 timerModeModified = FALSE;
	uint16 buffDex;
	uint32 timeCheck;
	uint32 maxRecordTime;
	uint32 returnCode = CFG_ERR_NONE;
	uint32 msgCRC = 0;
	uint32 inCRC;
	uint16 j = 0;
	uint8 ucmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	float timeCheckFloat;
	float timeAlarmFloat;

	if (ACTIVE_STATE == g_sampleProcessing)
	{
		returnCode = CFG_ERR_MONITORING_STATE;
	}
	else
	{	
		memset(&cfg, 0, sizeof(cfg));

		// Check to see if the incoming message is the correct size
		if ((uint32)((inCmd->size - 16)/2) < sizeof(cfg))
		{
			debug("WARNING:Msg Size incorrect msgSize=%d cfgSize=%d \r\n", ((inCmd->size - 16)/2), sizeof(cfg));
		}
		
		// Move the string data into the configuration structure. String is (2 * cfgSize)
		buffDex = MESSAGE_HEADER_SIMPLE_LENGTH;
		while ((buffDex < inCmd->size) && (buffDex < (MESSAGE_HEADER_SIMPLE_LENGTH + (sizeof(cfg) * 2))) &&
				(buffDex < CMD_BUFFER_SIZE))
		{	
			*cfgPtr++ = ConvertAscii2Binary(inCmd->msg[buffDex], inCmd->msg[buffDex + 1]);
			buffDex += 2;
		}		

		// Check if the SuperGraphics version is non-zero suggesting that it's a version that supports sending back CRC
		if (cfg.sgVersion)
		{
			// Get the CRC value from the data stream
			while ((buffDex < inCmd->size) && (buffDex < CMD_BUFFER_SIZE) && (j < 4))
			{
				// Add each CRC value by byte
				((uint8*)&inCRC)[j++] = ConvertAscii2Binary(inCmd->msg[buffDex], inCmd->msg[buffDex + 1]);
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
				BuildOutgoingSimpleHeaderBuffer((uint8*)ucmHdr, (uint8*)"UCMx",
					(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);	

				// Send Starting CRLF
				ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);

				// Calculate the CRC on the header
				g_transmitCRC = CalcCCITT32((uint8*)&ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

				// Send Simple header
				ModemPuts((uint8*)ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);

				// Send Ending Footer
				ModemPuts((uint8*)&msgCRC, 4, NO_CONVERSION);
				ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);
				
				return;
			}
		}

		//---------------------------------------------------------------------------
		// Check the new operation mode
		//---------------------------------------------------------------------------
		switch (cfg.mode)
		{
			case WAVEFORM_MODE: 
			case BARGRAPH_MODE:
			case COMBO_MODE:
				g_triggerRecord.opMode = cfg.mode;
				break;

			case MANUAL_CAL_MODE:
			case MANUAL_TRIGGER_MODE:
			default:
				returnCode = CFG_ERR_TRIGGER_MODE;
				goto SEND_UCM_ERROR_CODE;
				break;
		}
		
		//---------------------------------------------------------------------------
		// Check date and time to see if an update needs to be applied
		//---------------------------------------------------------------------------
		if ((cfg.currentTime.day == 0) && (cfg.currentTime.month == 0) && (cfg.currentTime.year == 0) &&
			(cfg.currentTime.sec == 0) && (cfg.currentTime.min == 0) && (cfg.currentTime.hour == 0))
		{
			debug("Do not update time.\r\n");
		}
		else // Update date and time
		{
			// Check for correct values and update the month.
			if ((cfg.currentTime.day == 0) || (cfg.currentTime.day > 31) || (cfg.currentTime.month == 0) ||
				(cfg.currentTime.month > 12) || (cfg.currentTime.year > 99))
			{
				returnCode = CFG_ERR_SYSTEM_DATE;
				goto SEND_UCM_ERROR_CODE;
			}
			else // Date validated
			{
				SetExternalRtcDate(&(cfg.currentTime));
				g_unitConfig.timerMode = DISABLED;

				// Disable the Power Off timer if it's set
				ClearSoftTimer(POWER_OFF_TIMER_NUM);
			}
			
			// Check for correct values and update the date.
			if ((cfg.currentTime.sec > 59) || (cfg.currentTime.min > 59) || (cfg.currentTime.hour > 23))
			{
				returnCode = CFG_ERR_SYSTEM_TIME;
				goto SEND_UCM_ERROR_CODE;
			}
			else // Time validated
			{
				SetExternalRtcTime(&(cfg.currentTime));
				g_unitConfig.timerMode = DISABLED;
				
				// Disable the Power Off timer if it's set
				ClearSoftTimer(POWER_OFF_TIMER_NUM);
			}

			// Update the current time.
			UpdateCurrentTime();
		}

		//---------------------------------------------------------------------------
		// Distance to source check, cfg is in uint32 format not float.
		//---------------------------------------------------------------------------
		if (cfg.eventCfg.distToSource > (uint32)(DISTANCE_TO_SOURCE_MAX_VALUE * 100))
		{
			returnCode = CFG_ERR_DIST_TO_SRC;
			goto SEND_UCM_ERROR_CODE;
		}
		else
		{
			g_triggerRecord.trec.dist_to_source = (float)((float)cfg.eventCfg.distToSource / (float)100.0);
		}

		//---------------------------------------------------------------------------
		// Weight per delay check, cfg is in uint32 format not float
		//---------------------------------------------------------------------------
		if (cfg.eventCfg.weightPerDelay > (uint32)(WEIGHT_PER_DELAY_MAX_VALUE * 100))
		{
			returnCode = CFG_ERR_WEIGHT_DELAY;
			goto SEND_UCM_ERROR_CODE;
		}
		else
		{
			g_triggerRecord.trec.weight_per_delay = (float)((float)cfg.eventCfg.weightPerDelay / (float)100.0);
		}

		//---------------------------------------------------------------------------
		// Sample Rate check
		//---------------------------------------------------------------------------
		if ((SAMPLE_RATE_1K == cfg.eventCfg.sampleRate) || (SAMPLE_RATE_2K == cfg.eventCfg.sampleRate) || (SAMPLE_RATE_4K == cfg.eventCfg.sampleRate) ||
			(SAMPLE_RATE_8K == cfg.eventCfg.sampleRate) || (SAMPLE_RATE_16K == cfg.eventCfg.sampleRate))
		{
			if (((BARGRAPH_MODE == g_triggerRecord.opMode) || (COMBO_MODE == g_triggerRecord.opMode)) &&
				(cfg.eventCfg.sampleRate > SAMPLE_RATE_4K))
			{
				returnCode = CFG_ERR_SAMPLE_RATE;
				goto SEND_UCM_ERROR_CODE;
			}
			else
			{
				g_triggerRecord.trec.sample_rate = (uint32)cfg.eventCfg.sampleRate;
			}
		}
		else
		{
			returnCode = CFG_ERR_SAMPLE_RATE;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Seismic Trigger Level check
		//---------------------------------------------------------------------------
		if ((MANUAL_TRIGGER_CHAR == cfg.eventCfg.seismicTriggerLevel) || (NO_TRIGGER_CHAR == cfg.eventCfg.seismicTriggerLevel) ||
			((cfg.eventCfg.seismicTriggerLevel >= SEISMIC_TRIGGER_MIN_VALUE) && (cfg.eventCfg.seismicTriggerLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
		{
			g_triggerRecord.trec.seismicTriggerLevel = cfg.eventCfg.seismicTriggerLevel;
		}
		else
		{
			returnCode = CFG_ERR_SEISMIC_TRIG_LVL;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Update air sensor type DB or MB
		//---------------------------------------------------------------------------
		if (cfg.extraUnitCfg.unitsOfAir == MILLIBAR_TYPE)
		{
			g_unitConfig.unitsOfAir = MILLIBAR_TYPE;
		}
		else
		{
			g_unitConfig.unitsOfAir = DECIBEL_TYPE;
		}

		//---------------------------------------------------------------------------
		// A-weighting check
		//---------------------------------------------------------------------------
		if ((cfg.eventCfg.aWeighting == YES) || (cfg.eventCfg.aWeighting == NO))
		{
			if (g_factorySetupRecord.aweight_option == ENABLED)
			{
				if (cfg.eventCfg.aWeighting == YES) { g_unitConfig.airScale = AIR_SCALE_A_WEIGHTING; }
				else { g_unitConfig.airScale = AIR_SCALE_LINEAR; }
			}
			else { g_unitConfig.airScale = AIR_SCALE_LINEAR; }
		}
		else
		{
			returnCode = CFG_ERR_A_WEIGHTING;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Air Trigger Level check
		//---------------------------------------------------------------------------
		if ((MANUAL_TRIGGER_CHAR == cfg.eventCfg.airTriggerLevel) || (NO_TRIGGER_CHAR == cfg.eventCfg.airTriggerLevel) ||
			((cfg.eventCfg.airTriggerLevel >= AIR_TRIGGER_MIN_COUNT) && (cfg.eventCfg.airTriggerLevel <= (uint32)AIR_TRIGGER_MAX_COUNT)))
		{
			g_triggerRecord.trec.airTriggerLevel = cfg.eventCfg.airTriggerLevel;
		}
		else
		{
			returnCode = CFG_ERR_SOUND_TRIG_LVL;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Record time check
		//---------------------------------------------------------------------------
		// Find Max Record time for a given sample rate. Max time is equal to
		// Size of the (event buff - size of Pretrigger buffer - size of calibration buff) 
		// divided by (sample rate * number of channels).
		// Number of channels is hard coded to 4 = NUMBER_OF_CHANNELS_DEFAULT 
		//---------------------------------------------------------------------------
		maxRecordTime = (uint16)(((uint32)((EVENT_BUFF_SIZE_IN_WORDS - 
				((g_triggerRecord.trec.sample_rate / g_unitConfig.pretrigBufferDivider) * NUMBER_OF_CHANNELS_DEFAULT) -
				((g_triggerRecord.trec.sample_rate / MIN_SAMPLE_RATE) * MAX_CAL_SAMPLES * NUMBER_OF_CHANNELS_DEFAULT)) / 
				(g_triggerRecord.trec.sample_rate * NUMBER_OF_CHANNELS_DEFAULT))));

		if ((cfg.eventCfg.recordTime >= 1) && (cfg.eventCfg.recordTime <= maxRecordTime))
		{
			g_triggerRecord.trec.record_time = cfg.eventCfg.recordTime;
		}
		else
		{
			returnCode = CFG_ERR_RECORD_TIME;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Bar Interval Level check
		//---------------------------------------------------------------------------
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
				goto SEND_UCM_ERROR_CODE;
				break;
		}

		//---------------------------------------------------------------------------
		// Bar Interval Level check
		//---------------------------------------------------------------------------
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
				goto SEND_UCM_ERROR_CODE;
				break;
		}

		//---------------------------------------------------------------------------
		// Copy Client, Operator, Location and Comments strings to default trigger record
		//---------------------------------------------------------------------------
		memcpy((uint8*)g_triggerRecord.trec.client, cfg.eventCfg.companyName, COMPANY_NAME_STRING_SIZE - 2);
		memcpy((uint8*)g_triggerRecord.trec.oper, cfg.eventCfg.seismicOperator, SEISMIC_OPERATOR_STRING_SIZE - 2);
		memcpy((uint8*)g_triggerRecord.trec.loc, cfg.eventCfg.sessionLocation, SESSION_LOCATION_STRING_SIZE - 2);
		memcpy((uint8*)g_triggerRecord.trec.comments, cfg.eventCfg.sessionComments, SESSION_COMMENTS_STRING_SIZE - 2);

		//---------------------------------------------------------------------------
		// Sensitivity check
		//---------------------------------------------------------------------------
		if ((cfg.extraUnitCfg.sensitivity == LOW) || (cfg.extraUnitCfg.sensitivity == HIGH))
		{
			g_triggerRecord.srec.sensitivity = cfg.extraUnitCfg.sensitivity;
		}
		else
		{
			returnCode = CFG_ERR_SENSITIVITY;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Bar Scale check
		//---------------------------------------------------------------------------
		if ((cfg.extraUnitCfg.barScale == 1) || (cfg.extraUnitCfg.barScale == 2) || (cfg.extraUnitCfg.barScale == 4) || (cfg.extraUnitCfg.barScale == 8))
		{
			g_triggerRecord.berec.barScale = (uint8)cfg.extraUnitCfg.barScale;
		}
		else
		{
			returnCode = CFG_ERR_SCALING;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Bar Channel check
		//---------------------------------------------------------------------------
		if (cfg.extraUnitCfg.barChannel <= BAR_AIR_CHANNEL)
		{
			g_triggerRecord.berec.barChannel = cfg.extraUnitCfg.barChannel;
		}
		else
		{
			returnCode = CFG_ERR_BAR_PRINT_CHANNEL;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Pretrigger Buffer check
		//---------------------------------------------------------------------------
		if ((cfg.eventCfg.pretrigBufferDivider == PRETRIGGER_BUFFER_QUARTER_SEC_DIV) || (cfg.eventCfg.pretrigBufferDivider == PRETRIGGER_BUFFER_HALF_SEC_DIV) ||
			(cfg.eventCfg.pretrigBufferDivider == PRETRIGGER_BUFFER_FULL_SEC_DIV))
		{
			g_unitConfig.pretrigBufferDivider = cfg.eventCfg.pretrigBufferDivider;
		}
		else
		{
			returnCode = CFG_ERR_PRETRIG_BUFFER_DIV;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Bit Accuracy check
		//---------------------------------------------------------------------------
		if ((cfg.eventCfg.bitAccuracy == ACCURACY_10_BIT) || (cfg.eventCfg.bitAccuracy == ACCURACY_12_BIT) || 
			(cfg.eventCfg.bitAccuracy == ACCURACY_14_BIT) || (cfg.eventCfg.bitAccuracy == ACCURACY_16_BIT))
		{
			g_triggerRecord.trec.bitAccuracy = cfg.eventCfg.bitAccuracy;
		}
		else
		{
			returnCode = CFG_ERR_BIT_ACCURACY;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Adjust for Temperature drift check
		//---------------------------------------------------------------------------
		if ((cfg.eventCfg.adjustForTempDrift == YES) || (cfg.eventCfg.adjustForTempDrift == NO))
		{
			g_triggerRecord.trec.adjustForTempDrift = cfg.eventCfg.adjustForTempDrift;
		}
		else
		{
			returnCode = CFG_ERR_TEMP_ADJUST;
			goto SEND_UCM_ERROR_CODE;
		}

		//---------------------------------------------------------------------------
		// Auto Monitor Mode check
		//---------------------------------------------------------------------------
		switch (cfg.autoCfg.autoMonitorMode)
		{
			case AUTO_TWO_MIN_TIMEOUT:
			case AUTO_THREE_MIN_TIMEOUT:
			case AUTO_FOUR_MIN_TIMEOUT:
			case AUTO_NO_TIMEOUT:
				g_unitConfig.autoMonitorMode = cfg.autoCfg.autoMonitorMode;
				break;
				
			default:
				returnCode = CFG_ERR_AUTO_MON_MODE;
				goto SEND_UCM_ERROR_CODE;
				break;
		}

		//---------------------------------------------------------------------------
		// Auto Calibration Mode check
		//---------------------------------------------------------------------------
		switch (cfg.autoCfg.autoCalMode)
		{	
			case AUTO_24_HOUR_TIMEOUT:
			case AUTO_48_HOUR_TIMEOUT:
			case AUTO_72_HOUR_TIMEOUT:
			case AUTO_NO_CAL_TIMEOUT:
				g_unitConfig.autoCalMode = cfg.autoCfg.autoCalMode;
				break;
				
			default:
				returnCode = CFG_ERR_AUTO_CAL_MODE;
				goto SEND_UCM_ERROR_CODE;
				break;
		}

		//---------------------------------------------------------------------------
		// External Trigger check
		//---------------------------------------------------------------------------
		switch (cfg.autoCfg.externalTrigger)
		{
			case ENABLED:
			case DISABLED:
				g_unitConfig.externalTrigger = cfg.autoCfg.externalTrigger;
				break;

			default:
#if 0 // Normal
				returnCode = CFG_ERR_EXTERNAL_TRIGGER;
				goto SEND_UCM_ERROR_CODE;
#else // Don't force error until remote side has added ability
				// Don't change current unit config
#endif
				break;
		}

		//---------------------------------------------------------------------------
		// External Trigger check
		//---------------------------------------------------------------------------
		switch (cfg.autoCfg.rs232PowerSavings)
		{
			case ENABLED:
			case DISABLED:
				g_unitConfig.rs232PowerSavings = cfg.autoCfg.rs232PowerSavings;
			break;

			default:
#if 0 // Normal
				returnCode = CFG_ERR_RS232_POWER_SAVINGS;
				goto SEND_UCM_ERROR_CODE;
#else // Don't force error until remote side has added ability
				// Don't change current unit config
#endif
			break;
		}

		//---------------------------------------------------------------------------
		// Auto print
		//---------------------------------------------------------------------------
		if ((cfg.extraUnitCfg.autoPrint == NO) || (cfg.extraUnitCfg.autoPrint == YES))
		{
			g_unitConfig.autoPrint = cfg.extraUnitCfg.autoPrint;
		}
		else
		{
			returnCode = CFG_ERR_AUTO_PRINT;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Language Mode
		//---------------------------------------------------------------------------
		switch (cfg.extraUnitCfg.languageMode)
		{
			case ENGLISH_LANG:
			case FRENCH_LANG:
			case ITALIAN_LANG:
			case GERMAN_LANG:
			case SPANISH_LANG:
				if (g_unitConfig.languageMode != cfg.extraUnitCfg.languageMode)
				{
					g_unitConfig.languageMode = cfg.extraUnitCfg.languageMode;
					BuildLanguageLinkTable(g_unitConfig.languageMode);
				}
				break;
					
			default:
				returnCode = CFG_ERR_LANGUAGE_MODE;
				goto SEND_UCM_ERROR_CODE;
				break;	
		}

		//---------------------------------------------------------------------------
		// Vector Sum check
		//---------------------------------------------------------------------------
		if ((cfg.extraUnitCfg.vectorSum == NO) || (cfg.extraUnitCfg.vectorSum == YES))
		{
			g_unitConfig.vectorSum = cfg.extraUnitCfg.vectorSum;
		}
		else
		{
			returnCode = CFG_ERR_VECTOR_SUM;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Units of Measure check
		//---------------------------------------------------------------------------
		if ((cfg.extraUnitCfg.unitsOfMeasure == IMPERIAL_TYPE) || (cfg.extraUnitCfg.unitsOfMeasure == METRIC_TYPE))
		{
			g_unitConfig.unitsOfMeasure = cfg.extraUnitCfg.unitsOfMeasure;
		}
		else
		{
			returnCode = CFG_ERR_UNITS_OF_MEASURE;
			goto SEND_UCM_ERROR_CODE;
		}
		
		//---------------------------------------------------------------------------
		// Frequency plot mode , Yes or No or On or Off
		//---------------------------------------------------------------------------
		if ((cfg.extraUnitCfg.freqPlotMode == NO) || (cfg.extraUnitCfg.freqPlotMode == YES))
		{
			g_unitConfig.freqPlotMode = cfg.extraUnitCfg.freqPlotMode;
		}
		else
		{
			returnCode = CFG_ERR_FREQ_PLOT_MODE;
			goto SEND_UCM_ERROR_CODE;
		}

		// Frequency plot mode , Yes or No or On or Off
		switch (cfg.extraUnitCfg.freqPlotType)
		{
			case FREQ_PLOT_US_BOM_STANDARD:
			case FREQ_PLOT_FRENCH_STANDARD:
			case FREQ_PLOT_DIN_4150_STANDARD:
			case FREQ_PLOT_BRITISH_7385_STANDARD:
			case FREQ_PLOT_SPANISH_STANDARD:
				g_unitConfig.freqPlotType = cfg.extraUnitCfg.freqPlotType;
				break;
					
			default:
				returnCode = CFG_ERR_FREQ_PLOT_TYPE;
				goto SEND_UCM_ERROR_CODE;
				break;	
		}

		//---------------------------------------------------------------------------
		// Alarm 1 check
		//---------------------------------------------------------------------------
		if (ALARM_MODE_OFF == cfg.alarmCfg.alarmOneMode)
		{
			g_unitConfig.alarmOneMode = ALARM_MODE_OFF;
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
					g_unitConfig.alarmOneMode = cfg.alarmCfg.alarmOneMode;
				break;
						
				default:
					g_unitConfig.alarmOneMode = ALARM_MODE_OFF;
					returnCode = CFG_ERR_ALARM_ONE_MODE;
					goto SEND_UCM_ERROR_CODE;
				break;
			}

			if ((ALARM_MODE_SEISMIC == g_unitConfig.alarmOneMode) || (ALARM_MODE_BOTH == g_unitConfig.alarmOneMode))
			{
				// Alarm One Seismic trigger level check for Waveform mode only
				if ((cfg.mode == WAVEFORM_MODE) && ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmOneSeismicLevel) ||
					((cfg.alarmCfg.alarmOneSeismicLevel >= g_triggerRecord.trec.seismicTriggerLevel) &&
					(cfg.alarmCfg.alarmOneSeismicLevel <= SEISMIC_TRIGGER_MAX_VALUE))))
				{
					g_unitConfig.alarmOneSeismicLevel = cfg.alarmCfg.alarmOneSeismicLevel;
				}
				// Alarm One Seismic trigger level check for other modes
				else if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmOneSeismicLevel) 	||
						((cfg.alarmCfg.alarmOneSeismicLevel >= SEISMIC_TRIGGER_MIN_VALUE) &&
						(cfg.alarmCfg.alarmOneSeismicLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
				{
					g_unitConfig.alarmOneSeismicLevel = cfg.alarmCfg.alarmOneSeismicLevel;
				}
				else
				{					
					returnCode = CFG_ERR_ALARM_ONE_SEISMIC_LVL;
					goto SEND_UCM_ERROR_CODE;
				}
			}

			if ((ALARM_MODE_AIR == g_unitConfig.alarmOneMode) || (ALARM_MODE_BOTH == g_unitConfig.alarmOneMode))
			{
				// Alarm One Air trigger level check DB/MB for Waveform mode only
				if ((cfg.mode == WAVEFORM_MODE) && ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmOneAirLevel) ||
					((cfg.alarmCfg.alarmOneAirLevel >= g_triggerRecord.trec.airTriggerLevel) &&
					(cfg.alarmCfg.alarmOneAirLevel <= AIR_TRIGGER_MAX_COUNT))))
				{
					g_unitConfig.alarmOneAirLevel = cfg.alarmCfg.alarmOneAirLevel;
				}
				// Alarm One Air trigger level check DB/MB for other modes
				else if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmOneAirLevel) || ((cfg.alarmCfg.alarmOneAirLevel >= AIR_TRIGGER_MIN_COUNT) &&
						(cfg.alarmCfg.alarmOneAirLevel <= AIR_TRIGGER_MAX_COUNT)))
				{
					g_unitConfig.alarmOneAirLevel = cfg.alarmCfg.alarmOneAirLevel;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_ONE_SOUND_LVL;
					goto SEND_UCM_ERROR_CODE;
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
					g_unitConfig.alarmOneTime = (float)timeAlarmFloat;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_ONE_TIME;
					goto SEND_UCM_ERROR_CODE;
				}
			}
			else
			{
				returnCode = CFG_ERR_ALARM_ONE_TIME;
				goto SEND_UCM_ERROR_CODE;
			}
		}			

		//---------------------------------------------------------------------------
		// Alarm 2 check
		//---------------------------------------------------------------------------
		if (ALARM_MODE_OFF == cfg.alarmCfg.alarmTwoMode)
		{
			g_unitConfig.alarmTwoMode = ALARM_MODE_OFF;
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
					g_unitConfig.alarmTwoMode = cfg.alarmCfg.alarmTwoMode;
					break;
						
				default:
					g_unitConfig.alarmTwoMode = ALARM_MODE_OFF;
					returnCode = CFG_ERR_ALARM_TWO_MODE;
					goto SEND_UCM_ERROR_CODE;
					break;	
			}

			if ((ALARM_MODE_SEISMIC == g_unitConfig.alarmTwoMode) || (ALARM_MODE_BOTH == g_unitConfig.alarmTwoMode))
			{
				// Alarm Two Seismic trigger level check for Waveform mode only
				if ((cfg.mode == WAVEFORM_MODE) && ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmTwoSeismicLevel) ||
					((cfg.alarmCfg.alarmTwoSeismicLevel >= g_triggerRecord.trec.seismicTriggerLevel) &&
					(cfg.alarmCfg.alarmTwoSeismicLevel <= SEISMIC_TRIGGER_MAX_VALUE))))
				{
					g_unitConfig.alarmTwoSeismicLevel = cfg.alarmCfg.alarmTwoSeismicLevel;
				}
				// Alarm Two Seismic trigger level check for other modes
				else if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmTwoSeismicLevel) ||
						((cfg.alarmCfg.alarmTwoSeismicLevel >= g_triggerRecord.trec.seismicTriggerLevel) &&
						(cfg.alarmCfg.alarmTwoSeismicLevel <= SEISMIC_TRIGGER_MAX_VALUE)))
				{
					g_unitConfig.alarmTwoSeismicLevel = cfg.alarmCfg.alarmTwoSeismicLevel;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_SEISMIC_LVL;
					goto SEND_UCM_ERROR_CODE;
				}
			}
			
			if ((ALARM_MODE_AIR == g_unitConfig.alarmTwoMode) || (ALARM_MODE_BOTH == g_unitConfig.alarmTwoMode))
			{
				// Alarm Two Air trigger level check DB/MB for Waveform mode only
				if ((cfg.mode == WAVEFORM_MODE) && ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmTwoAirLevel) ||
					((cfg.alarmCfg.alarmTwoAirLevel >= g_triggerRecord.trec.airTriggerLevel) &&
					(cfg.alarmCfg.alarmTwoAirLevel <= AIR_TRIGGER_MAX_COUNT))))
				{
					g_unitConfig.alarmTwoAirLevel = cfg.alarmCfg.alarmTwoAirLevel;
				}
				// Alarm Two Air trigger level check DB/MB for other modes
				else if ((NO_TRIGGER_CHAR == cfg.alarmCfg.alarmTwoAirLevel) ||
						((cfg.alarmCfg.alarmTwoAirLevel >= g_triggerRecord.trec.airTriggerLevel) &&
						(cfg.alarmCfg.alarmTwoAirLevel <= AIR_TRIGGER_MAX_COUNT)))
				{
					g_unitConfig.alarmTwoAirLevel = cfg.alarmCfg.alarmTwoAirLevel;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_SOUND_LVL;
					goto SEND_UCM_ERROR_CODE;
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
					g_unitConfig.alarmTwoTime = timeAlarmFloat;
				}
				else
				{
					returnCode = CFG_ERR_ALARM_TWO_TIME;
					goto SEND_UCM_ERROR_CODE;
				}
			}
			else
			{
				returnCode = CFG_ERR_ALARM_TWO_TIME;
				goto SEND_UCM_ERROR_CODE;
			}
		}

		//---------------------------------------------------------------------------
		// Timer mode check
		//---------------------------------------------------------------------------
		if (!((DISABLED == cfg.timerCfg.timerMode) && (DISABLED == g_unitConfig.timerMode)))
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
						goto SEND_UCM_ERROR_CODE;
						break;
				}

				// time valid value check.
				if (!((cfg.timerCfg.timer_start.hour < 24) && 
					 (cfg.timerCfg.timer_start.min < 60) &&
					 (cfg.timerCfg.timer_start.year < 100) &&
					 (cfg.timerCfg.timer_start.month >= 1) && (cfg.timerCfg.timer_start.month <= 12) &&
					 (cfg.timerCfg.timer_start.day >= 1) && (cfg.timerCfg.timer_start.day < 32)))
				{
					returnCode = CFG_ERR_START_TIME;
					goto SEND_UCM_ERROR_CODE;
				}

				if (!((cfg.timerCfg.timer_stop.hour < 24) && 
					 (cfg.timerCfg.timer_stop.min < 60) &&
					 (cfg.timerCfg.timer_stop.year < 100) &&
					 (cfg.timerCfg.timer_stop.month >= 1) && (cfg.timerCfg.timer_stop.month <= 12) &&
					 (cfg.timerCfg.timer_stop.day >= 1) && (cfg.timerCfg.timer_stop.day < 32)))
				{
					returnCode = CFG_ERR_STOP_TIME;
					goto SEND_UCM_ERROR_CODE;
				}
				
				if ((CFG_ERR_START_TIME != returnCode) && (CFG_ERR_STOP_TIME != returnCode) &&
					(CFG_ERR_TIMER_MODE_FREQ != returnCode))
				{
					g_unitConfig.timerModeFrequency = cfg.timerCfg.timerModeFrequency;
					g_unitConfig.timerStopTime.hour = cfg.timerCfg.timer_stop.hour;
					g_unitConfig.timerStopTime.min = cfg.timerCfg.timer_stop.min;
					g_unitConfig.timerStopDate.day = cfg.timerCfg.timer_stop.day;
					g_unitConfig.timerStopDate.month = cfg.timerCfg.timer_stop.month;
					g_unitConfig.timerStopDate.year = cfg.timerCfg.timer_stop.year;

					g_unitConfig.timerStartTime.hour = cfg.timerCfg.timer_start.hour;
					g_unitConfig.timerStartTime.min = cfg.timerCfg.timer_start.min;
					g_unitConfig.timerStartDate.day = cfg.timerCfg.timer_start.day;
					g_unitConfig.timerStartDate.month = cfg.timerCfg.timer_start.month;
					g_unitConfig.timerStartDate.year = cfg.timerCfg.timer_start.year;

					g_unitConfig.timerMode = cfg.timerCfg.timerMode;
				}					
			}
			else if (DISABLED == cfg.timerCfg.timerMode)
			{
				// Setting it to disabled, no error check needed.
				g_unitConfig.timerMode = cfg.timerCfg.timerMode;
			}
			else
			{	
				// In valid value for the timer mode, return an error
				returnCode = CFG_ERR_TIMER_MODE;
				goto SEND_UCM_ERROR_CODE;
			}
		}
	
		// Check if the Supergraphics
		if (cfg.sgVersion)
		{
			if ((cfg.flashWrapping == NO) || (cfg.flashWrapping == YES))
			{			
#if 0 // Normal
				g_unitConfig.flashWrapping = cfg.flashWrapping;
#else // Forcing flash wrapping to be disabled
				g_unitConfig.flashWrapping = NO;
#endif
			}
			else
			{
				// Invalid value for the flash wrapping option
				returnCode = CFG_ERR_FLASH_WRAPPING;
				goto SEND_UCM_ERROR_CODE;
			}
		}

		//---------------------------------------------------------------------------
		// Check if no errors to this point
		//---------------------------------------------------------------------------
		if (returnCode == CFG_ERR_NONE)
		{
			// Saved updated settings
			SaveRecordData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);
			SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
			SaveRecordData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);
		}
	}

	if (TRUE == timerModeModified)
	{
		ProcessTimerModeSettings(NO_PROMPT);
		// If the user wants to enable, but it is now disable send an error.
		if (DISABLED == g_unitConfig.timerMode)
		{
			returnCode = CFG_ERR_TIMER_MODE;
		}
	}
	
SEND_UCM_ERROR_CODE:
	if (returnCode != CFG_ERR_NONE)
	{
		debug("UCM error encountered: %d\r\n", returnCode);
	}

	// -------------------------------------
	// Return codes
	sprintf((char*)msgTypeStr, "%02lu", returnCode);
	BuildOutgoingSimpleHeaderBuffer((uint8*)ucmHdr, (uint8*)"UCMx",
		(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	ModemPuts((uint8*)&ucmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);

	// Send Ending Footer
	ModemPuts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleDMM(CMD_BUFFER_STRUCT* inCmd)
{
	MODEM_SETUP_STRUCT modemCfg;
	uint8 dmmHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];

	UNUSED(inCmd);

	memset(&modemCfg, 0, sizeof(modemCfg));

	memcpy((uint8*)&modemCfg, &g_modemSetupRecord, sizeof(MODEM_SETUP_STRUCT));

	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);
	BuildOutgoingSimpleHeaderBuffer((uint8*)dmmHdr, (uint8*)"DMMx", (uint8*)msgTypeStr,
		(uint32)(MESSAGE_SIMPLE_TOTAL_LENGTH + sizeof(MODEM_SETUP_STRUCT)), COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&dmmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	ModemPuts((uint8*)dmmHdr, MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag);

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&modemCfg, sizeof(MODEM_SETUP_STRUCT), g_transmitCRC);		

	// Send the configuration data
	ModemPuts((uint8*)&modemCfg, sizeof(MODEM_SETUP_STRUCT), g_binaryXferFlag);

	// Ending Footer
	ModemPuts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleUMM(CMD_BUFFER_STRUCT* inCmd)
{
	MODEM_SETUP_STRUCT modemCfg;
	uint8* modemCfgPtr = (uint8*)(&modemCfg);
	uint16 i = 0, j = 0;
	uint32 returnCode = CFG_ERR_NONE;
	uint32 msgCRC = 0;
	uint32 inCRC = 0;
	uint8 ummHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+2];

	memset(&modemCfg, 0, sizeof(modemCfg));

	// Move the string data into the configuration structure. String is (2 * cfgSize)
	i = MESSAGE_HEADER_SIMPLE_LENGTH;
	while ((i < inCmd->size) && (i < (MESSAGE_HEADER_SIMPLE_LENGTH + (sizeof(MODEM_SETUP_STRUCT) * 2))) && 
			(i < CMD_BUFFER_SIZE))
	{	
		*modemCfgPtr++ = ConvertAscii2Binary(inCmd->msg[i], inCmd->msg[i + 1]);
		i += 2;
	}		

	// Get the CRC value from the data stream
	while ((i < inCmd->size) && (i < CMD_BUFFER_SIZE) && (j < 4))
	{
		((uint8*)&inCRC)[j++] = ConvertAscii2Binary(inCmd->msg[i], inCmd->msg[i + 1]);
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

			SaveRecordData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
		}
	}

	// -------------------------------------
	// Return codes
	sprintf((char*)msgTypeStr, "%02lu", returnCode);
	BuildOutgoingSimpleHeaderBuffer((uint8*)ummHdr, (uint8*)"UMMx",
		(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);	

	// Send Starting CRLF
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&ummHdr, MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);		

	// Send Simple header
	ModemPuts((uint8*)ummHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);

	// Send Ending Footer
	ModemPuts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ConvertAscii2Binary(uint8 firstByte, uint8 secondByte)
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
void HandleVTI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleSTI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVDA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleSDA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleZRO(CMD_BUFFER_STRUCT* inCmd)
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
void HandleCAL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVOL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVCG(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVSL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVEL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleSBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleSDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleVCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleSCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ModemInitProcess(void)
{
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

	debug("ModemInitProcess\r\n");

	if (READ_DCD == NO_CONNECTION)
	{
		if (strlen(g_modemSetupRecord.reset) != 0)
		{
			UartPuts((char*)(g_modemSetupRecord.reset), CRAFT_COM_PORT);
			UartPuts((char*)&g_CRLF, CRAFT_COM_PORT);

			SoftUsecWait(3 * SOFT_SECS);
		}

		if (strlen(g_modemSetupRecord.init) != 0)
		{
			UartPuts((char*)(g_modemSetupRecord.init), CRAFT_COM_PORT);
			UartPuts((char*)&g_CRLF, CRAFT_COM_PORT);
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
void ModemResetProcess(void)
{
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;
	debug("HandleMRC\r\n");

	g_modemStatus.systemIsLockedFlag = YES;

	if (g_autoRetries == 0)
	{
		CLEAR_DTR;
	}	

	g_modemResetStage = 1;
	AssignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(15 * TICKS_PER_SEC), ModemResetTimerCallback);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMRS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	g_modemStatus.systemIsLockedFlag = YES;

	if (YES == g_modemSetupRecord.modemStatus)
	{
		g_autoRetries = 0;
		ModemResetProcess();
	}

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMVS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMPO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMMO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMNO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMTO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMSD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMSR(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMST(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMSA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMSB(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMSC(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMVI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleMVO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

