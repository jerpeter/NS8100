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
#include "PowerManagement.h"
#include "RemoteCommon.h"
#if 0 // Port fat driver
#include "FAT32_FileLib.h"
#include "FAT32_Disk.h"
#include "FAT32_Access.h"
#endif

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
#include "fsaccess.h"
void MoveManualCalToFile(void)
{
	//static SUMMARY_DATA* sumEntry;
	static SUMMARY_DATA* ramSummaryEntryPtr;
	uint16 i;
	uint16 sample;
	uint16 normalizedData;
	uint16 hiA = 0, hiR = 0, hiV = 0, hiT = 0;
	uint16 lowA = 0xFFFF, lowR = 0xFFFF, lowV = 0xFFFF, lowT = 0xFFFF;
	uint16* startOfEventPtr;
	uint16* endOfEventDataPtr;

#if 1 // Atmel fat driver
	int manualCalFileHandle = -1;
#else // Port fat driver
	FL_FILE* manualCalFileHandle = NULL;
#endif

	debug("Processing Manual Cal to be saved\r\n");

	if (g_freeEventBuffers < g_maxEventBuffers)
	{
		if (GetRamSummaryEntry(&ramSummaryEntryPtr) == FALSE)
		{
			debugErr("Out of Ram Summary Entrys\r\n");
		}

		g_pendingEventRecord.summary.captured.eventTime = GetCurrentTime();

#if 0 // Old
		sumEntry = &g_summaryTable[g_eventBufferReadIndex];
#endif

#if 1 // Need to clear out the Summary entry since it's initialized to all 0xFF's
		memset(ramSummaryEntryPtr, 0, sizeof(SUMMARY_DATA));
#else // Old
		// Initialize the freq data counts.
		ramSummaryEntryPtr->waveShapeData.a.freq = 0;
		ramSummaryEntryPtr->waveShapeData.r.freq = 0;
		ramSummaryEntryPtr->waveShapeData.v.freq = 0;
		ramSummaryEntryPtr->waveShapeData.t.freq = 0;
#endif

		ramSummaryEntryPtr->mode = MANUAL_CAL_MODE;

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

			if (normalizedData > ramSummaryEntryPtr->waveShapeData.a.peak)
			{
				ramSummaryEntryPtr->waveShapeData.a.peak = normalizedData;
				ramSummaryEntryPtr->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + A_CHAN_OFFSET);
			}

			//=========================================================
			// Second channel - R
			sample = *(g_currentEventSamplePtr + R_CHAN_OFFSET);

			if (sample > hiR) hiR = sample;
			if (sample < lowR) lowR = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > ramSummaryEntryPtr->waveShapeData.r.peak)
			{
				ramSummaryEntryPtr->waveShapeData.r.peak = normalizedData;
				ramSummaryEntryPtr->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + R_CHAN_OFFSET);
			}

			//=========================================================
			// Third channel - V
			sample = *(g_currentEventSamplePtr + V_CHAN_OFFSET);

			if (sample > hiV) hiV = sample;
			if (sample < lowV) lowV = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > ramSummaryEntryPtr->waveShapeData.v.peak)
			{
				ramSummaryEntryPtr->waveShapeData.v.peak = normalizedData;
				ramSummaryEntryPtr->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + V_CHAN_OFFSET);
			}

			//=========================================================
			// Fourth channel - T
			sample = *(g_currentEventSamplePtr + T_CHAN_OFFSET);

			if (sample > hiT) hiT = sample;
			if (sample < lowT) lowT = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > ramSummaryEntryPtr->waveShapeData.t.peak)
			{
				ramSummaryEntryPtr->waveShapeData.t.peak = normalizedData;
				ramSummaryEntryPtr->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + T_CHAN_OFFSET);
			}

			g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
		}

		ramSummaryEntryPtr->waveShapeData.a.peak = (uint16)(hiA - lowA + 1);
		ramSummaryEntryPtr->waveShapeData.r.peak = (uint16)(hiR - lowR + 1);
		ramSummaryEntryPtr->waveShapeData.v.peak = (uint16)(hiV - lowV + 1);
		ramSummaryEntryPtr->waveShapeData.t.peak = (uint16)(hiT - lowT + 1);

		ramSummaryEntryPtr->waveShapeData.a.freq = CalcSumFreq(ramSummaryEntryPtr->waveShapeData.a.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);
		ramSummaryEntryPtr->waveShapeData.r.freq = CalcSumFreq(ramSummaryEntryPtr->waveShapeData.r.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);
		ramSummaryEntryPtr->waveShapeData.v.freq = CalcSumFreq(ramSummaryEntryPtr->waveShapeData.v.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);
		ramSummaryEntryPtr->waveShapeData.t.freq = CalcSumFreq(ramSummaryEntryPtr->waveShapeData.t.peakPtr, SAMPLE_RATE_1K, startOfEventPtr, endOfEventDataPtr);

		CompleteRamEventSummary(ramSummaryEntryPtr);

		CacheResultsEventInfo((EVT_RECORD*)&g_pendingEventRecord);

		if (g_fileAccessLock != AVAILABLE)
		{
			ReportFileSystemAccessProblem("Save Manual Cal");
		}
		else // (g_fileAccessLock == AVAILABLE)
		{
			while ((volatile uint8)g_spi1AccessLock != AVAILABLE) {;}
			g_spi1AccessLock = EVENT_LOCK;

			//g_fileAccessLock = FILE_LOCK;
			nav_select(FS_NAV_ID_DEFAULT);

			// Get new event file handle
			manualCalFileHandle = GetEventFileHandle(g_pendingEventRecord.summary.eventNumber, CREATE_EVENT_FILE);

#if 1 // Atmel fat driver
			if (manualCalFileHandle == -1)
#else // Port fat driver
			if (manualCalFileHandle == NULL)
#endif
			{
				debugErr("Failed to get a new file handle for the Manual Cal event\r\n");
			}
			else // Write the file event to the SD card
			{
				sprintf((char*)&g_spareBuffer[0], "CALIBRATION EVENT #%d BEING SAVED...", g_pendingEventRecord.summary.eventNumber);
				OverlayMessage("EVENT COMPLETE", (char*)&g_spareBuffer[0], 0);

#if 1 // Atmel fat driver
				// Write the event record header and summary
				write(manualCalFileHandle, &g_pendingEventRecord, sizeof(EVT_RECORD));

				// Write the event data, containing the Pretrigger, event and cal
				write(manualCalFileHandle, g_currentEventStartPtr, (g_wordSizeInCal * 2));

				SetFileDateTimestamp(FS_DATE_LAST_WRITE);

				// Done writing the event file, close the file handle
				g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
				close(manualCalFileHandle);
#else // Port fat driver
				// Write the event record header and summary
				fl_fwrite(&g_pendingEventRecord, sizeof(EVT_RECORD), 1, manualCalFileHandle);

				// Write the event data, containing the Pretrigger, event and cal
				fl_fwrite(g_currentEventStartPtr, g_wordSizeInCal, 2, manualCalFileHandle);

				// Done writing the event file, close the file handle
				fl_fclose(manualCalFileHandle);
#endif

				debug("Manual Cal Event file closed\r\n");

				ramSummaryEntryPtr->fileEventNum = g_pendingEventRecord.summary.eventNumber;

				AddEventToSummaryList(&g_pendingEventRecord);

				// Don't log a monitor entry for Manual Cal
				//UpdateMonitorLogEntry();

				// After event numbers have been saved, store current event number in persistent storage.
				StoreCurrentEventNumber();

				UpdateSDCardUsageStats(sizeof(EVT_RECORD) + g_wordSizeInCal);

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

			g_lastCompletedRamSummaryIndex = ramSummaryEntryPtr;

			if (g_triggerRecord.op_mode != COMBO_MODE)
			{
				// Set printout mode to allow the results menu processing to know this is a manual cal pulse
				raiseMenuEventFlag(RESULTS_MENU_EVENT);
			}

			g_freeEventBuffers++;

			g_fileAccessLock = AVAILABLE;
			g_spi1AccessLock = AVAILABLE;
		}
	}
	else
	{
		debugWarn("Manual Cal: No free buffers\r\n");

		clearSystemEventFlag(MANUAL_CAL_EVENT);
	}
}
