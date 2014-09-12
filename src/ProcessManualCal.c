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
#include "FAT32_Disk.h"
#include "FAT32_Access.h"

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
void MoveManualCalToFlash(void)
{
	static SUMMARY_DATA* sumEntry;
	static SUMMARY_DATA* ramSummaryEntry;
	uint16 i;
	uint16 sample;
	uint16 normalizedData;
	uint16 hiA = 0, hiR = 0, hiV = 0, hiT = 0;
	uint16 lowA = 0xFFF, lowR = 0xFFF, lowV = 0xFFF, lowT = 0xFFF;
	uint16* startOfEventPtr;
	uint16* endOfEventDataPtr;

	debug("Processing Manual Cal to be saved\n");

	if (g_freeEventBuffers < g_maxEventBuffers)
	{
		if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
		{
			debugErr("Out of Ram Summary Entrys\n");
		}

		g_pendingEventRecord.summary.captured.eventTime = GetCurrentTime();

		sumEntry = &g_summaryTable[g_eventBufferReadIndex];
		sumEntry->mode = MANUAL_CAL_MODE;

		// Initialize the freq data counts.
		sumEntry->waveShapeData.a.freq = 0;
		sumEntry->waveShapeData.r.freq = 0;
		sumEntry->waveShapeData.v.freq = 0;
		sumEntry->waveShapeData.t.freq = 0;

		g_currentEventSamplePtr = g_currentEventStartPtr;
		startOfEventPtr = g_currentEventStartPtr;
		endOfEventDataPtr = g_currentEventStartPtr + g_wordSizeInCal;
		
		for (i = (uint16)g_samplesInCal; i != 0; i--)
		{
			if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

			//=========================================================
			// First channel - A
			sample = *(g_currentEventSamplePtr + A_CHAN_OFFSET);

			if (sample > hiA) hiA = sample;
			if (sample < lowA) lowA = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.a.peak)
			{
				sumEntry->waveShapeData.a.peak = normalizedData;
				sumEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + A_CHAN_OFFSET);
			}

			//=========================================================
			// Second channel - R
			sample = *(g_currentEventSamplePtr + R_CHAN_OFFSET);

			if (sample > hiR) hiR = sample;
			if (sample < lowR) lowR = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.r.peak)
			{
				sumEntry->waveShapeData.r.peak = normalizedData;
				sumEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + R_CHAN_OFFSET);
			}

			//=========================================================
			// Third channel - V
			sample = *(g_currentEventSamplePtr + V_CHAN_OFFSET);

			if (sample > hiV) hiV = sample;
			if (sample < lowV) lowV = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.v.peak)
			{
				sumEntry->waveShapeData.v.peak = normalizedData;
				sumEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + V_CHAN_OFFSET);
			}

			//=========================================================
			// Fourth channel - T
			sample = *(g_currentEventSamplePtr + T_CHAN_OFFSET);

			if (sample > hiT) hiT = sample;
			if (sample < lowT) lowT = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.t.peak)
			{
				sumEntry->waveShapeData.t.peak = normalizedData;
				sumEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + T_CHAN_OFFSET);
			}

			g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
		}

		sumEntry->waveShapeData.a.peak = (uint16)(hiA - lowA + 1);
		sumEntry->waveShapeData.r.peak = (uint16)(hiR - lowR + 1);
		sumEntry->waveShapeData.v.peak = (uint16)(hiV - lowV + 1);
		sumEntry->waveShapeData.t.peak = (uint16)(hiT - lowT + 1);

		sumEntry->waveShapeData.a.freq = CalcSumFreq(sumEntry->waveShapeData.a.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);
		sumEntry->waveShapeData.r.freq = CalcSumFreq(sumEntry->waveShapeData.r.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);   
		sumEntry->waveShapeData.v.freq = CalcSumFreq(sumEntry->waveShapeData.v.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);   
		sumEntry->waveShapeData.t.freq = CalcSumFreq(sumEntry->waveShapeData.t.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);       

		CompleteRamEventSummary(ramSummaryEntry, sumEntry);
		CacheResultsEventInfo((EVT_RECORD*)&g_pendingEventRecord);

		// Get new event file handle
		g_currentEventFileHandle = GetEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);
				
		if (g_currentEventFileHandle == NULL)
		{
			debugErr("Failed to get a new file handle for the Manual Cal event\n");
			
			//ReInitSdCardAndFat32();
			//g_currentEventFileHandle = GetEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);
		}					
		else // Write the file event to the SD card
		{
			sprintf((char*)&g_spareBuffer[0], "CALIBRATION EVENT #%d BEING SAVED...", g_nextEventNumberToUse);
			OverlayMessage("EVENT COMPLETE", (char*)&g_spareBuffer[0], 0);

			// Write the event record header and summary
			fl_fwrite(&g_pendingEventRecord, sizeof(EVT_RECORD), 1, g_currentEventFileHandle);

			// Write the event data, containing the Pretrigger, event and cal
			fl_fwrite(g_currentEventStartPtr, g_wordSizeInCal, 2, g_currentEventFileHandle);

			// Done writing the event file, close the file handle
			fl_fclose(g_currentEventFileHandle);
			debug("Event file closed\n");

			ramSummaryEntry->fileEventNum = g_nextEventNumberToUse;
					
			// Don't log a monitor entry for Manual Cal
			//UpdateMonitorLogEntry();

			// After event numbers have been saved, store current event number in persistent storage.
			StoreCurrentEventNumber();

			// Now store the updated event number in the universal ram storage.
			g_pendingEventRecord.summary.eventNumber = g_nextEventNumberToUse;
		}

		if (++g_eventBufferReadIndex == g_maxEventBuffers)
		{
			g_eventBufferReadIndex = 0;
			g_currentEventSamplePtr = g_startOfEventBufferPtr;
		}
		else
		{
			g_currentEventSamplePtr = g_startOfEventBufferPtr + (g_eventBufferReadIndex * g_wordSizeInEvent);
		}
		clearSystemEventFlag(MANUAL_CAL_EVENT);

		g_lastCompletedRamSummaryIndex = ramSummaryEntry;

		// Set printout mode to allow the results menu processing to know this is a manual cal pulse
		raiseMenuEventFlag(RESULTS_MENU_EVENT);

		g_freeEventBuffers++;
	}
	else
	{
		debugWarn("Manual Cal: No free buffers\n");

		clearSystemEventFlag(MANUAL_CAL_EVENT);
	}
}
