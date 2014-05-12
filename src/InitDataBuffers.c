///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: InitDataBuffers.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:49 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/InitDataBuffers.c,v $
///	$Revision: 1.2 $
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
#include "Flash.h"
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

//*****************************************************************************
// Function:	InitDataBuffs
// Purpose:		Setup up buffers for the specific mode selected by the user
//*****************************************************************************
void InitDataBuffs(uint8 op_mode)
{ 
	uint32 quarterSecBufferSize;
	uint32 sampleRate;
	
	if (op_mode == MANUAL_CAL_MODE)
	{
#if 0 // ns7100
		// Set sample rate to 1024 since it's fixed in the 430 for manual calibrations
#endif
		sampleRate = 1024;
	}
	else // Waveform, Bargraph, Combo
	{
		sampleRate = g_triggerRecord.trec.sample_rate;
	}

	// Setup the pre-trigger buffer pointers
	g_startOfQuarterSecBuff = &(g_quarterSecBuff[0]);
	g_tailOfQuarterSecBuff = &(g_quarterSecBuff[0]);

	// Set the Quarter Sec buffer size in words, 1/4 sec @ sample rate * number of channels, plus 1 sample (1/4 sec plus actual trigger sample)
	quarterSecBufferSize = ((uint32)(sampleRate / 4) * g_sensorInfoPtr->numOfChannels) + g_sensorInfoPtr->numOfChannels;
	// Set up the end of the quarter sec buffer
	g_endOfQuarterSecBuff = &(g_quarterSecBuff[quarterSecBufferSize]);

	// Setup the pending event record information that is available at this time
	initEventRecord(op_mode);

	if (g_triggerRecord.trec.bitAccuracy == ACCURACY_10_BIT) { g_sampleDataMidpoint = 0x0200; }
	else if (g_triggerRecord.trec.bitAccuracy == ACCURACY_12_BIT) { g_sampleDataMidpoint = 0x0800; }
	else if (g_triggerRecord.trec.bitAccuracy == ACCURACY_14_BIT) { g_sampleDataMidpoint = 0x2000; }
	else { g_sampleDataMidpoint = 0x8000; } // Default to 16-bit accuracy

	// Setup buffers based on mode
	if ((op_mode == WAVEFORM_MODE) || (op_mode == MANUAL_CAL_MODE) || (op_mode == COMBO_MODE))
	{
		// Calculate samples for each section and total event
		g_samplesInBody = (uint32)(sampleRate * g_triggerRecord.trec.record_time);
		g_samplesInPretrig = (uint32)(sampleRate / 4);
		g_samplesInCal = (uint32)MAX_CAL_SAMPLES;
		g_samplesInEvent = g_samplesInPretrig + g_samplesInBody + g_samplesInCal;

		// Calculate word size for each section and total event, since buffer is an array of words
		g_wordSizeInPretrig = g_samplesInPretrig * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInBody = g_samplesInBody * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInCal = g_samplesInCal * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInEvent = g_wordSizeInPretrig + g_wordSizeInBody + g_wordSizeInCal;

		if (op_mode == COMBO_MODE)
		{
			// Calculate total event buffers available (partial event buffer size)
			g_maxEventBuffers = (uint16)((EVENT_BUFF_SIZE_IN_WORDS - COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS) / g_wordSizeInEvent);

			// Init starting event buffer pointers
			g_startOfEventBufferPtr = &(g_eventDataBuffer[COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS]);
		}
		else // ((op_mode == WAVEFORM_MODE) || (op_mode == MANUAL_CAL_MODE))
		{
			// Calculate total event buffers available (full event buffer size)
			g_maxEventBuffers = (uint16)(EVENT_BUFF_SIZE_IN_WORDS / g_wordSizeInEvent);

			// Init starting event buffer pointers
			g_startOfEventBufferPtr = &(g_eventDataBuffer[0]);
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
		if (op_mode == WAVEFORM_MODE)
		{
			// Update the waveform data length to be used based on the size calculations
			g_pendingEventRecord.header.dataLength = (g_wordSizeInEvent * 2);
		}
		else if (op_mode == MANUAL_CAL_MODE)
		{
			// Update the manual cal data length to be used based on the size calculations
			g_pendingEventRecord.header.dataLength = (g_wordSizeInCal * 2);
		}
		else if (op_mode == COMBO_MODE)
		{		
			// Update the combo-waveform data length to be used based on the size calculations
			g_pendingEventRecord.header.dataLength = (g_wordSizeInEvent * 2);

			// Bargraph init
			g_bargraphDataStartPtr = &(g_eventDataBuffer[0]);
			g_bargraphDataWritePtr = &(g_eventDataBuffer[0]);
			g_bargraphDataReadPtr = &(g_eventDataBuffer[0]);
			g_bargraphDataEndPtr = &(g_eventDataBuffer[COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS - 4]);

			// Start the total off with zero (incremented when bar and summary intervals are stored)
			g_pendingBargraphRecord.header.dataLength = 0;

			StartNewCombo();
		}
	}
	else if (op_mode == BARGRAPH_MODE)
	{		
		g_bargraphDataStartPtr = &(g_eventDataBuffer[0]);
		g_bargraphDataWritePtr = &(g_eventDataBuffer[0]);
		g_bargraphDataReadPtr = &(g_eventDataBuffer[0]);
		g_bargraphDataEndPtr = &(g_eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS - 4]);

		// Start the total off with zero (incremented when bar and summary intervals are stored)
		g_pendingBargraphRecord.header.dataLength = 0;
		
		StartNewBargraph();
	}
}

//*****************************************************************************
// Function:	CalcSumFreq
// Purpose:		
//*****************************************************************************
uint16 CalcSumFreq(uint16* dataPtr, uint32 sampleRate, uint16* startAddrPtr, uint16* endAddrPtr)
{
	uint16* samplePtr;
	uint32 dataCount = 0;
	uint16 freq = 0;

	samplePtr = dataPtr;

	if (*samplePtr >= POS_VALID_PEAK)
	{
		while ((*samplePtr >= POS_CROSSOVER_BACKWARD) && (samplePtr > (startAddrPtr + 4)))
		{
			samplePtr -= 4;
			dataCount++;
		}
    
		samplePtr = dataPtr;
		while ((*samplePtr >= POS_CROSSOVER_FORWARD) && (samplePtr < (endAddrPtr - 4)))
		{
			samplePtr += 4;
			dataCount++;
		}
	}
	else if (*samplePtr <= NEG_VALID_PEAK)
	{
		while ((*samplePtr <= NEG_CROSSOVER_BACKWARD) && (samplePtr > (startAddrPtr + 4)))
		{
			samplePtr -= 4;
			dataCount++;
		}
    
		samplePtr = dataPtr;
		while ((*samplePtr <= NEG_CROSSOVER_FORWARD) && (samplePtr < (endAddrPtr - 4)))
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

//*****************************************************************************
// FUNCTION:	uint16 FixDataToZero(uint16 data_)
// DESCRIPTION
//*****************************************************************************
uint16 FixDataToZero(uint16 data_)
{
   if (data_ > g_sampleDataMidpoint)
     data_ = (uint16)(data_ - g_sampleDataMidpoint);
   else
     data_ = (uint16)(g_sampleDataMidpoint - data_);
     
   return (data_);  
}
