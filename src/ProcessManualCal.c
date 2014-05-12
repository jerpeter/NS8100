///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: ProcessManualCal.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:57 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ProcessManualCal.c,v $
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
// Function:	ProcessManuelCalPulse
// Purpose:		
//*****************************************************************************
void ProcessManuelCalPulse(void)
{
	DATE_TIME_STRUCT triggerTimeStamp;
	//SUMMARY_DATA* sumEntry;

	static uint8 s_manualCalInProgess = FALSE;

	switch (*g_tailOfPreTrigBuff & EMBEDDED_CMD)
	{
		case CAL_START:
			s_manualCalInProgess = TRUE;
			if (g_freeEventBuffers != 0)
			{
				g_pendingEventRecord.summary.captured.eventTime = triggerTimeStamp = getCurrentTime();
#if 0 // ns7100
				g_summaryTable[g_eventBufferIndex].linkPtr = g_eventBufferPretrigPtr;
#endif
				*(g_eventBufferPretrigPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_START);
				*(g_eventBufferPretrigPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_START);
				*(g_eventBufferPretrigPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_START);
				*(g_eventBufferPretrigPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_START);

				g_eventBufferPretrigPtr += 4;
				g_tailOfPreTrigBuff += 4;
			}
			break;

		case CAL_END:
			s_manualCalInProgess = FALSE;

			*(g_eventBufferPretrigPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_END);
			*(g_eventBufferPretrigPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_END);
			*(g_eventBufferPretrigPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_END);
			*(g_eventBufferPretrigPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_END);

			g_eventBufferPretrigPtr += 4;
			g_tailOfPreTrigBuff += 4;
			
			raiseSystemEventFlag(MANUEL_CAL_EVENT);
			g_manualCalFlag = FALSE;
			g_manualCalSampleCount = 0;
			break;

		default:
			if (s_manualCalInProgess == TRUE)
			{
				*(g_eventBufferPretrigPtr + 0) = *(g_tailOfPreTrigBuff + 0);
				*(g_eventBufferPretrigPtr + 1) = *(g_tailOfPreTrigBuff + 1);
				*(g_eventBufferPretrigPtr + 2) = *(g_tailOfPreTrigBuff + 2);
				*(g_eventBufferPretrigPtr + 3) = *(g_tailOfPreTrigBuff + 3);

				g_eventBufferPretrigPtr += 4;
				g_tailOfPreTrigBuff += 4;
			}
			else
			{
				g_tailOfPreTrigBuff += g_sensorInfoPtr->numOfChannels;
			}
			break;
	}
  
	if (g_manualCalFlag == FALSE)
	{
		g_freeEventBuffers--;
		g_eventBufferIndex++;
		if (g_eventBufferIndex < g_maxEventBuffers)
		{
				g_eventBufferPretrigPtr = g_eventBufferBodyPtr + g_wordSizeInCal;
				g_eventBufferBodyPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig;
		}
		else
		{
			g_eventBufferIndex = 0;
			g_eventBufferPretrigPtr = g_startOfEventBufferPtr;
			g_eventBufferBodyPtr = g_startOfEventBufferPtr + g_wordSizeInPretrig;
		}
	}  

	// Check if the end of the PreTrigger buffer has been reached
	if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;
}

//*****************************************************************************
// Function:	MoveManuelCalToFlash
// Purpose:
//*****************************************************************************
void MoveManuelCalToFlash(void)
{
	static SUMMARY_DATA* sumEntry;
	static SUMMARY_DATA* ramSummaryEntry;
	uint16 i;
	uint16 sample;
	uint16 normalizedData;
	uint16 hiA = 0, hiR = 0, hiV = 0, hiT = 0;
	uint16 lowA = 0xFFF, lowR = 0xFFF, lowV = 0xFFF, lowT = 0xFFF;

	if (g_freeEventBuffers < g_maxEventBuffers)
	{
		if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
		{
			debugErr("Out of Ram Summary Entrys\n");
		}

#if 0 // ns7100
		// Set our flash event data pointer to the start of the flash event
		advFlashDataPtrToEventData(ramSummaryEntry);
#endif

		sumEntry = &g_summaryTable[g_currentEventBuffer];
		sumEntry->mode = MANUAL_CAL_MODE;

		// Initialize the freq data counts.
		sumEntry->waveShapeData.a.freq = 0;
		sumEntry->waveShapeData.r.freq = 0;
		sumEntry->waveShapeData.v.freq = 0;
		sumEntry->waveShapeData.t.freq = 0;

		for (i = (uint16)g_samplesInCal; i != 0; i--)
		{
			//=========================================================
			// First channel - A
			sample = (uint16)((*(g_currentEventSamplePtr + 0)) & DATA_MASK);

			if (sample > hiA) hiA = sample;
			if (sample < lowA) lowA = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.a.peak)
			{
				sumEntry->waveShapeData.a.peak = normalizedData;
				sumEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + 0);
			}

			//=========================================================
			// Second channel - R
			sample = (uint16)((*(g_currentEventSamplePtr + 1)) & DATA_MASK);

			if (sample > hiR) hiR = sample;
			if (sample < lowR) lowR = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.r.peak)
			{
				sumEntry->waveShapeData.r.peak = normalizedData;
				sumEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + 1);
			}

			//=========================================================
			// Third channel - V
			sample = (uint16)((*(g_currentEventSamplePtr + 2)) & DATA_MASK);

			if (sample > hiV) hiV = sample;
			if (sample < lowV) lowV = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.v.peak)
			{
				sumEntry->waveShapeData.v.peak = normalizedData;
				sumEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + 2);
			}

			//=========================================================
			// Fourth channel - T
			sample = (uint16)((*(g_currentEventSamplePtr + 3)) & DATA_MASK);

			if (sample > hiT) hiT = sample;
			if (sample < lowT) lowT = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.t.peak)
			{
				sumEntry->waveShapeData.t.peak = normalizedData;
				sumEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + 3);
			}

			// Store entire sample
#if 0 // ns7100
			storeData(g_currentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);
#endif
			
			g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
		}

		sumEntry->waveShapeData.a.peak = (uint16)(hiA - lowA + 1);
		sumEntry->waveShapeData.r.peak = (uint16)(hiR - lowR + 1);
		sumEntry->waveShapeData.v.peak = (uint16)(hiV - lowV + 1);
		sumEntry->waveShapeData.t.peak = (uint16)(hiT - lowT + 1);

		sumEntry->waveShapeData.a.freq = CalcSumFreq(sumEntry->waveShapeData.a.peakPtr, 1024);
		sumEntry->waveShapeData.r.freq = CalcSumFreq(sumEntry->waveShapeData.r.peakPtr, 1024);   
		sumEntry->waveShapeData.v.freq = CalcSumFreq(sumEntry->waveShapeData.v.peakPtr, 1024);   
		sumEntry->waveShapeData.t.freq = CalcSumFreq(sumEntry->waveShapeData.t.peakPtr, 1024);       

		completeRamEventSummary(ramSummaryEntry, sumEntry);

		// Get new event file handle
		g_currentEventFileHandle = getEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);
				
		if (g_currentEventFileHandle == NULL)
		{
			debugErr("Failed to get a new file handle for the Manual Cal event!\n");
		}					
		else // Write the file event to the SD card
		{
			char tempBuffer[50];
					
			sprintf(&tempBuffer[0], "CALIBRATION EVENT #%d BEING SAVED...", g_nextEventNumberToUse);
			overlayMessage("EVENT COMPLETE", &tempBuffer[0], 0);

			// Write the event record header and summary
			fl_fwrite(&g_pendingEventRecord, sizeof(EVT_RECORD), 1, g_currentEventFileHandle);

			// Write the event data, containing the pretrigger, event and cal
			fl_fwrite(g_currentEventStartPtr, g_wordSizeInCal, 2, g_currentEventFileHandle);

			// Done writing the event file, close the file handle
			fl_fclose(g_currentEventFileHandle);
			debug("Event file closed\n");

			ramSummaryEntry->fileEventNum = g_nextEventNumberToUse;
					
			// Don't log a monitor entry for Manual Cal
			//updateMonitorLogEntry();

			// After event numbers have been saved, store current event number in persistent storage.
			storeCurrentEventNumber();

			// Now store the updated event number in the universal ram storage.
			g_pendingEventRecord.summary.eventNumber = g_nextEventNumberToUse;
		}

		if (++g_currentEventBuffer == g_maxEventBuffers)
		{
			g_currentEventBuffer = 0;
			g_currentEventSamplePtr = g_startOfEventBufferPtr;
		}
		else
		{
			g_currentEventSamplePtr = g_startOfEventBufferPtr + (g_currentEventBuffer * g_wordSizeInEvent);
		}
		clearSystemEventFlag(MANUEL_CAL_EVENT);

		g_lastCompletedRamSummary = ramSummaryEntry;

		// Set printout mode to allow the results menu processing to know this is a manual cal pulse
		g_printOutMode = MANUAL_CAL_MODE;
		raiseMenuEventFlag(RESULTS_MENU_EVENT);

		g_freeEventBuffers++;
	}
	else
	{
		clearSystemEventFlag(MANUEL_CAL_EVENT);
	}
}
