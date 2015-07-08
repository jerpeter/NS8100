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
#include "math.h"
#include "Typedefs.h"
#include "InitDataBuffers.h"
#include "SysEvents.h"
#include "Record.h"
#include "Menu.h"
#include "Summary.h"
#include "EventProcessing.h"
#include "Board.h"
#include "Record.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "PowerManagement.h"
#include "RemoteCommon.h"
#include "FAT32_FileLib.h"

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
void InitDataBuffs(uint8 opMode)
{ 
	uint32 pretriggerBufferSize;
	uint32 sampleRate;
	
	if (opMode == MANUAL_CAL_MODE)
	{
		// Per requirement fix sample rate for Calibration
		sampleRate = MANUAL_CAL_DEFAULT_SAMPLE_RATE;

		// Variable Pretrigger size in words; sample rate / Pretrigger buffer divider times channels plus 1 extra sample (to ensure a full Pretrigger plus a trigger sample)
		pretriggerBufferSize = ((uint32)(sampleRate / MANUAL_CAL_PRETRIGGER_BUFFER_DIVIDER) * g_sensorInfo.numOfChannels) + g_sensorInfo.numOfChannels;
	}
	else // Waveform, Bargraph, Combo
	{
		sampleRate = g_triggerRecord.trec.sample_rate;

		// Variable Pretrigger size in words; sample rate / Pretrigger buffer divider times channels plus 1 extra sample (to ensure a full Pretrigger plus a trigger sample)
		pretriggerBufferSize = ((uint32)(sampleRate / g_unitConfig.pretrigBufferDivider) * g_sensorInfo.numOfChannels) + g_sensorInfo.numOfChannels;
	}

	// Setup the Pretrigger buffer pointers
	g_startOfPretriggerBuff = &(g_pretriggerBuff[0]);
	g_tailOfPretriggerBuff = &(g_pretriggerBuff[0]);
	g_endOfPretriggerBuff = &(g_pretriggerBuff[pretriggerBufferSize]);

	// Setup Bit Accuracy globals
	if (opMode == MANUAL_CAL_MODE)
		{ g_bitAccuracyMidpoint = ACCURACY_16_BIT_MIDPOINT; g_bitShiftForAccuracy = AD_BIT_ACCURACY - ACCURACY_16_BIT; }
	else if (g_triggerRecord.trec.bitAccuracy == ACCURACY_10_BIT)
		{ g_bitAccuracyMidpoint = ACCURACY_10_BIT_MIDPOINT; g_bitShiftForAccuracy = AD_BIT_ACCURACY - ACCURACY_10_BIT; }
	else if (g_triggerRecord.trec.bitAccuracy == ACCURACY_12_BIT) 
		{ g_bitAccuracyMidpoint = ACCURACY_12_BIT_MIDPOINT; g_bitShiftForAccuracy = AD_BIT_ACCURACY - ACCURACY_12_BIT; }
	else if (g_triggerRecord.trec.bitAccuracy == ACCURACY_14_BIT) 
		{ g_bitAccuracyMidpoint = ACCURACY_14_BIT_MIDPOINT; g_bitShiftForAccuracy = AD_BIT_ACCURACY - ACCURACY_14_BIT; }
	else // Default to 16-bit accuracy
		{ g_bitAccuracyMidpoint = ACCURACY_16_BIT_MIDPOINT; g_bitShiftForAccuracy = AD_BIT_ACCURACY - ACCURACY_16_BIT; } 

	// Setup the pending event record information that is available at this time
	InitEventRecord(opMode);

	// Setup buffers based on mode
	if ((opMode == WAVEFORM_MODE) || (opMode == MANUAL_CAL_MODE) || (opMode == COMBO_MODE))
	{
		// Calculate samples for each section and total event
		g_samplesInPretrig = (uint32)(sampleRate / g_unitConfig.pretrigBufferDivider);
		g_samplesInBody = (uint32)(sampleRate * g_triggerRecord.trec.record_time);
		g_samplesInCal = (uint32)MAX_CAL_SAMPLES;
		g_samplesInEvent = g_samplesInPretrig + g_samplesInBody + g_samplesInCal;

		// Calculate word size for each section and total event, since buffer is an array of words
		g_wordSizeInPretrig = g_samplesInPretrig * g_sensorInfo.numOfChannels;
		g_wordSizeInBody = g_samplesInBody * g_sensorInfo.numOfChannels;
		g_wordSizeInCal = g_samplesInCal * g_sensorInfo.numOfChannels;
		g_wordSizeInEvent = g_wordSizeInPretrig + g_wordSizeInBody + g_wordSizeInCal;

		if (opMode == COMBO_MODE)
		{
			// Calculate total event buffers available (partial event buffer size)
			g_maxEventBuffers = (uint16)((EVENT_BUFF_SIZE_IN_WORDS - COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS) / (g_wordSizeInEvent + (sizeof(DATE_TIME_STRUCT) / 2)));

			// Init the starting event date and timestamp buffer to the beginning of the event buffer past the bargraph portion
			g_startOfEventDateTimestampBufferPtr = (DATE_TIME_STRUCT*)&(g_eventDataBuffer[COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS]);

			// Init starting event buffer pointer beyond the event date and timestamp buffer
			g_startOfEventBufferPtr = &(g_eventDataBuffer[(COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS + (g_maxEventBuffers * (sizeof(DATE_TIME_STRUCT) / 2)))]);
		}
		else // ((opMode == WAVEFORM_MODE) || (opMode == MANUAL_CAL_MODE))
		{
			// Calculate total event buffers available (full event buffer size plus date and time structure)
			g_maxEventBuffers = (uint16)(EVENT_BUFF_SIZE_IN_WORDS / (g_wordSizeInEvent + (sizeof(DATE_TIME_STRUCT) / 2)));

			// Init the starting event date and timestamp buffer to the beginning of the event buffer
			g_startOfEventDateTimestampBufferPtr = (DATE_TIME_STRUCT*)&(g_eventDataBuffer[0]);

			// Init starting event buffer pointer beyond the event date and timestamp buffer
			g_startOfEventBufferPtr = &(g_eventDataBuffer[(g_maxEventBuffers * (sizeof(DATE_TIME_STRUCT) / 2))]);
		}

		g_freeEventBuffers = g_maxEventBuffers;
		g_eventBufferReadIndex = 0;
		g_eventBufferWriteIndex = 0;

		g_currentEventSamplePtr = g_currentEventStartPtr = g_startOfEventBufferPtr;
		g_eventBufferPretrigPtr = g_startOfEventBufferPtr;
		g_eventBufferBodyPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig;
		g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

		// Init flags
		g_isTriggered = 0;
		g_processingCal = 0;
		g_calTestExpected = 0;
		g_doneTakingEvents = NO;
		
		// Update the data length to be used based on the size calculations
		if (opMode == WAVEFORM_MODE)
		{
			// Update the waveform data length to be used based on the size calculations
			g_pendingEventRecord.header.dataLength = (g_wordSizeInEvent * 2);
		}
		else if (opMode == MANUAL_CAL_MODE)
		{
			// Update the manual cal data length to be used based on the size calculations
			g_pendingEventRecord.header.dataLength = (g_wordSizeInCal * 2);
		}
		else if (opMode == COMBO_MODE)
		{		
			// Update the combo-waveform data length to be used based on the size calculations
			g_pendingEventRecord.header.dataLength = (g_wordSizeInEvent * 2);

			// Bargraph init
			g_bargraphDataStartPtr = &(g_eventDataBuffer[0]);
			g_bargraphDataWritePtr = &(g_eventDataBuffer[0]);
			g_bargraphDataReadPtr = &(g_eventDataBuffer[0]);
			g_bargraphDataEndPtr = &(g_eventDataBuffer[COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS]);

			// Start the total off with zero (incremented when bar and summary intervals are stored)
			g_pendingBargraphRecord.header.dataLength = 0;

			StartNewBargraph();
		}
	}
	else if (opMode == BARGRAPH_MODE)
	{		
		g_bargraphDataStartPtr = &(g_eventDataBuffer[0]);
		g_bargraphDataWritePtr = &(g_eventDataBuffer[0]);
		g_bargraphDataReadPtr = &(g_eventDataBuffer[0]);
		g_bargraphDataEndPtr = &(g_eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS]);

		// Start the total off with zero (incremented when bar and summary intervals are stored)
		g_pendingBargraphRecord.header.dataLength = 0;
		
		StartNewBargraph();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 CalcSumFreq(uint16* dataPtr, uint32 sampleRate, uint16* startAddrPtr, uint16* endAddrPtr)
{
	uint16* samplePtr;
	uint32 dataCount = 0;
	uint16 freq = 0;

	samplePtr = dataPtr;

	if (*samplePtr >= (g_bitAccuracyMidpoint + FREQ_VALID_PEAK))
	{
		while ((*samplePtr >= (g_bitAccuracyMidpoint + FREQ_CROSSOVER_BACKWARD)) && (samplePtr > (startAddrPtr + 4)))
		{
			samplePtr -= 4;
			dataCount++;
		}

		samplePtr = dataPtr;
		while ((*samplePtr >= (g_bitAccuracyMidpoint + FREQ_CROSSOVER_FORWARD)) && (samplePtr < (endAddrPtr - 4)))
		{
			samplePtr += 4;
			dataCount++;
		}
	}
	else if (*samplePtr <= (g_bitAccuracyMidpoint - FREQ_VALID_PEAK))
	{
		while ((*samplePtr <= (g_bitAccuracyMidpoint - FREQ_CROSSOVER_BACKWARD)) && (samplePtr > (startAddrPtr + 4)))
		{
			samplePtr -= 4;
			dataCount++;
		}

		samplePtr = dataPtr;
		while ((*samplePtr <= (g_bitAccuracyMidpoint - FREQ_CROSSOVER_FORWARD)) && (samplePtr < (endAddrPtr - 4)))
		{
			samplePtr += 4;
			dataCount++;
		}
	}
	else
	{
		// Peak was less than 4 counts, assume to be noise
		return ((uint16)0);
	}

	// total counts between 0 crossings
	dataCount = (uint32)((dataCount - 1) * 2);

	// Whole part of freq shifted to make one decimal place.
	if (dataCount > 0)
	{
		freq = (uint16)(((float)(sampleRate * 10) / (float)dataCount));
	}

	return (freq);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 FixDataToZero(uint16 data_)
{
	if (data_ > g_bitAccuracyMidpoint)
		data_ = (uint16)(data_ - g_bitAccuracyMidpoint);
	else
		data_ = (uint16)(g_bitAccuracyMidpoint - data_);

	return (data_);
}
