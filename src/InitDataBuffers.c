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
#include "Ispi.h"
#include "Msgs430.h"
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
	uint32 preTriggerSize;
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

	initEventRecord(&g_RamEventRecord, op_mode);

	// Setup the pre-trigger buffer pointers
	g_startOfPreTrigBuff = &(g_preTrigBuff[0]);
	g_tailOfPreTrigBuff = &(g_preTrigBuff[0]);

	// Set the PreTrigger size in words, 1/4 sec @ sample rate * number of channels, plus 1 sample
	preTriggerSize = ((uint32)(sampleRate / 4) * g_sensorInfoPtr->numOfChannels) + g_sensorInfoPtr->numOfChannels;
	// Set up the end of the pre trigger buffer
	g_endOfPreTrigBuff = &(g_preTrigBuff[preTriggerSize]);

	if ((op_mode == WAVEFORM_MODE) || (op_mode == MANUAL_CAL_MODE))
	{
		// Calculate samples for each section and total event
		g_samplesInBody = (uint32)(sampleRate * g_triggerRecord.trec.record_time);
		g_samplesInPretrig  = (uint32)(sampleRate / 4);
		g_samplesInCal  = (uint32)((sampleRate / MIN_SAMPLE_RATE) * MAX_CAL_SAMPLES);
		g_samplesInEvent  = g_samplesInPretrig + g_samplesInBody + g_samplesInCal;

		// Calculate word size for each section and total event, since buffer is an array of words
		g_wordSizeInPretrig = g_samplesInPretrig * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInBody = g_samplesInBody * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInCal = g_samplesInCal * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInEvent = g_wordSizeInPretrig + g_wordSizeInBody + g_wordSizeInCal;

		// Calculate total event buffers available
		g_maxEventBuffers = (uint16)(EVENT_BUFF_SIZE_IN_WORDS / (g_wordSizeInPretrig + g_wordSizeInBody + g_wordSizeInCal));
		g_freeEventBuffers = g_maxEventBuffers;

		// Init starting event buffer pointers
		g_startOfEventBufferPtr = &(g_eventDataBuffer[0]);
		g_currentEventStartPtr = g_currentEventSamplePtr = g_startOfEventBufferPtr;
		g_currentEventBuffer = 0;
		g_eventBufferIndex = 0;

		g_eventBufferPretrigPtr = g_startOfEventBufferPtr;
		g_eventBufferBodyPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig;
		g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

		// Init flags
		g_isTriggered = 0;
		g_processingCal = 0;
		g_calTestExpected = 0;
		g_doneTakingEvents = NO;
	}
	else if (op_mode == BARGRAPH_MODE)
	{		
		g_bg430DataStartPtr = &(g_eventDataBuffer[0]);
		g_bg430DataWritePtr = &(g_eventDataBuffer[0]);
		g_bg430DataReadPtr = &(g_eventDataBuffer[0]);
		g_bg430DataEndPtr = &(g_eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS - 4]);

		StartNewBargraph();
	}
	else if (op_mode == COMBO_MODE)
	{		
		// Waveform init
		// Calculate samples for each section and total event
		g_samplesInBody = (uint32)(sampleRate * g_triggerRecord.trec.record_time);
		g_samplesInPretrig  = (uint32)(sampleRate / 4);
		g_samplesInCal  = (uint32)((sampleRate / MIN_SAMPLE_RATE) * MAX_CAL_SAMPLES);
		g_samplesInEvent  = g_samplesInPretrig + g_samplesInBody + g_samplesInCal;

		// Calculate word size for each section and total event, since buffer is an array of words
		g_wordSizeInPretrig = g_samplesInPretrig * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInBody = g_samplesInBody * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInCal = g_samplesInCal * g_sensorInfoPtr->numOfChannels;
		g_wordSizeInEvent = g_wordSizeInPretrig + g_wordSizeInBody + g_wordSizeInCal;

		// Calculate total event buffers available
		g_maxEventBuffers = (uint16)((EVENT_BUFF_SIZE_IN_WORDS - COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET) / 
							(g_wordSizeInPretrig + g_wordSizeInBody + g_wordSizeInCal));
		g_freeEventBuffers = g_maxEventBuffers;

		// Init starting event buffer pointers
		g_startOfEventBufferPtr = &(g_eventDataBuffer[COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET]);
		g_currentEventStartPtr = g_currentEventSamplePtr = g_startOfEventBufferPtr; 
		g_currentEventBuffer = 0;
		g_eventBufferIndex = 0;

		g_eventBufferPretrigPtr = g_startOfEventBufferPtr;
		g_eventBufferBodyPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig;
		g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

		// Init flags
		g_isTriggered = 0;
		g_processingCal = 0;
		g_calTestExpected = 0;
		g_doneTakingEvents = NO;

		// Bargraph init
		g_bg430DataStartPtr = &(g_eventDataBuffer[0]);
		g_bg430DataWritePtr = &(g_eventDataBuffer[0]);
		g_bg430DataReadPtr = &(g_eventDataBuffer[0]);
		g_bg430DataEndPtr = &(g_eventDataBuffer[COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET - 16]);

		StartNewCombo();
	}
}

//*****************************************************************************
// Function:	CalcSumFreq
// Purpose:		
//*****************************************************************************
uint16 CalcSumFreq(uint16* workingPtr, uint16 sampleRate)
{
	uint16* samplePtr;
	uint32 dataCount = 0;
	uint16 freq = 0;

	samplePtr = workingPtr;

	if ((*samplePtr & DATA_MASK) >= POS_VALID_PEAK)
	{
		while (((*samplePtr & DATA_MASK) >= POS_CROSSOVER_BACKWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_START))
		{
			samplePtr -= 4;
			dataCount++;
		}
    
		samplePtr = workingPtr;
		while (((*samplePtr & DATA_MASK) >= POS_CROSSOVER_FORWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_END))
		{
			samplePtr += 4;
			dataCount++;
		}
	}
	else if ((*samplePtr & DATA_MASK) <= NEG_VALID_PEAK)
	{
		while (((*samplePtr & DATA_MASK) <= NEG_CROSSOVER_BACKWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_START))
		{
			samplePtr -= 4;
			dataCount++;
		}
    
		samplePtr = workingPtr;
		while (((*samplePtr & DATA_MASK) <= NEG_CROSSOVER_FORWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_END))
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
   if (data_ > 0x800)
     data_ = (uint16)(data_ - 0x0800);
   else
     data_ = (uint16)(0x0800 - data_);
     
   return (data_);  
}
