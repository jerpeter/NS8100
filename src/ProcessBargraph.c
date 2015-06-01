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
#include "Typedefs.h"
#include "InitDataBuffers.h"
#include "SysEvents.h"
#include "Summary.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "Record.h"
#include "Menu.h"
#include "ProcessBargraph.h"
#include "EventProcessing.h"
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
void StartNewBargraph(void)
{
	g_bargraphSummaryPtr = NULL;

	// Get the address of an empty Ram summary
	if (GetRamSummaryEntry(&g_bargraphSummaryPtr) == FALSE)
	{
		debug("%s: Out of Ram Summary Entrys\r\n", (g_triggerRecord.op_mode == BARGRAPH_MODE) ? "Bargraph" : "Combo");
		return;
	}

	// Clear the Bar and Summary Interval buffers
	memset(&g_bargraphBarInterval[0], 0, sizeof(g_bargraphBarInterval));
	memset(&g_bargraphSummaryInterval, 0, sizeof(g_bargraphSummaryInterval));
	memset(&g_bargraphFreqCalcBuffer, 0, sizeof(g_bargraphFreqCalcBuffer));

	// Init counts
	g_summaryCount = 0;
	g_oneSecondCnt = 0;
	g_oneMinuteCount = 0;
	g_barSampleCount = 0;
	g_totalBarIntervalCnt = 0;
	g_bargraphBarIntervalsCached = 0;

	// Init buffer pointers
	g_bargraphBarIntervalWritePtr = &g_bargraphBarInterval[0];
	g_bargraphBarIntervalReadPtr = &g_bargraphBarInterval[0];
	g_bargraphSummaryIntervalPtr = &g_bargraphSummaryInterval;

	MoveStartOfBargraphEventRecordToFile();

	// Update the current monitor log entry
	UpdateMonitorLogEntry();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void EndBargraph(void)
{
	while (CalculateBargraphData() == BG_BUFFER_NOT_EMPTY) {}

	// Check if any bar intervals are cached or if no summaries have yet been recorded
	if ((g_bargraphBarIntervalsCached) || (g_summaryCount == 0))
	{
		// Save the Bar and Summary intervals
		MoveSummaryIntervalDataToFile();
	}

	MoveEndOfBargraphEventRecordToFile();
	g_fileProcessActiveUsbLockout = OFF;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "fsaccess.h"
void MoveBarIntervalDataToFile(void)
{
#if 1 // Atmel fat driver
	int bargraphFileHandle = -1;
#else // Port fat driver
	FL_FILE* bargraphFileHandle = NULL;
#endif

	// If Bar Intervals have been cached
	if (g_bargraphBarIntervalsCached > 0)
	{
		if (g_fileAccessLock != AVAILABLE)
		{
			if (g_triggerRecord.op_mode == BARGRAPH_MODE) { ReportFileSystemAccessProblem("Save Bar Interval"); }
			else { ReportFileSystemAccessProblem("Save Combo Bar Interval"); }
		}
		else // (g_fileAccessLock == AVAILABLE)
		{
			while ((volatile uint8)g_spi1AccessLock != AVAILABLE) {;}
			g_spi1AccessLock = EVENT_LOCK;

			//g_fileAccessLock = FILE_LOCK;
			nav_select(FS_NAV_ID_DEFAULT);

			bargraphFileHandle = GetEventFileHandle(g_pendingBargraphRecord.summary.eventNumber, APPEND_EVENT_FILE);

			while (g_bargraphBarIntervalsCached)
			{
#if 1 // Atmel fat driver
				write(bargraphFileHandle, g_bargraphBarIntervalReadPtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
#else // Port fat driver
				fl_fwrite(g_bargraphBarIntervalReadPtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA), 1, bargraphFileHandle);
#endif

				g_pendingBargraphRecord.header.dataLength += sizeof(BARGRAPH_BAR_INTERVAL_DATA);

				// Advance the Bar Interval global buffer pointer
				AdvanceBarIntervalBufPtr(READ_PTR);

				g_bargraphBarIntervalsCached--;
			}

#if 1 // Atmel fat driver
			SetFileDateTimestamp(FS_DATE_LAST_WRITE);

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(bargraphFileHandle);
#else // Port fat driver
			fl_fclose(bargraphFileHandle);
#endif
			debug("%s event file closed\r\n", (g_triggerRecord.op_mode == BARGRAPH_MODE) ? "Bargraph" : "Combo - Bargraph");

			g_fileAccessLock = AVAILABLE;
			g_spi1AccessLock = AVAILABLE;
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CompleteSummaryInterval(void)
{
	float rFreq = (float)0, vFreq = (float)0, tFreq = (float)0;

	// Note: This should be raw unadjusted freq
	if(g_bargraphSummaryIntervalPtr->r.frequency > 0)
	{
		rFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSummaryIntervalPtr->r.frequency * 2) - 1));
	}

	if(g_bargraphSummaryIntervalPtr->v.frequency > 0)
	{
		vFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSummaryIntervalPtr->v.frequency * 2) - 1));
	}

	if(g_bargraphSummaryIntervalPtr->t.frequency > 0)
	{
		tFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSummaryIntervalPtr->t.frequency * 2) - 1));
	}

	// Calculate the Peak Displacement
	g_bargraphSummaryIntervalPtr->a.displacement = 0;
	g_bargraphSummaryIntervalPtr->r.displacement = (uint32)(g_bargraphSummaryIntervalPtr->r.peak * 1000000 / 2 / PI / rFreq);
	g_bargraphSummaryIntervalPtr->v.displacement = (uint32)(g_bargraphSummaryIntervalPtr->v.peak * 1000000 / 2 / PI / vFreq);
	g_bargraphSummaryIntervalPtr->t.displacement = (uint32)(g_bargraphSummaryIntervalPtr->t.peak * 1000000 / 2 / PI / tFreq);

	// Calculate the Peak Acceleration
	g_bargraphSummaryIntervalPtr->a.acceleration = 0;
	g_bargraphSummaryIntervalPtr->r.acceleration = (uint32)(g_bargraphSummaryIntervalPtr->r.peak * 1000 * 2 * PI * rFreq);
	g_bargraphSummaryIntervalPtr->v.acceleration = (uint32)(g_bargraphSummaryIntervalPtr->v.peak * 1000 * 2 * PI * vFreq);
	g_bargraphSummaryIntervalPtr->t.acceleration = (uint32)(g_bargraphSummaryIntervalPtr->t.peak * 1000 * 2 * PI * tFreq);

	// Store timestamp for the end of the summary interval
	g_summaryCount++;
	g_bargraphSummaryIntervalPtr->summariesCaptured = g_summaryCount;
	g_bargraphSummaryIntervalPtr->intervalEnd_Time = GetCurrentTime();
	g_bargraphSummaryIntervalPtr->batteryLevel = (uint32)(100.0 * GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
	g_bargraphSummaryIntervalPtr->calcStructEndFlag = 0xEECCCCEE;	// End structure flag
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MoveSummaryIntervalDataToFile(void)
{
#if 1 // Atmel fat driver
	int bargraphFileHandle = -1;
#else // Port fat driver
	FL_FILE* bargraphFileHandle = NULL;
#endif

	CompleteSummaryInterval();

	if (g_fileAccessLock != AVAILABLE)
	{
		if (g_triggerRecord.op_mode == BARGRAPH_MODE) { ReportFileSystemAccessProblem("Save Sum Interval"); }
		else { ReportFileSystemAccessProblem("Save Combo Sum Interval"); }
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		while ((volatile uint8)g_spi1AccessLock != AVAILABLE) {;}
		g_spi1AccessLock = EVENT_LOCK;

		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

		bargraphFileHandle = GetEventFileHandle(g_pendingBargraphRecord.summary.eventNumber, APPEND_EVENT_FILE);

		// Write any cached bar intervals before storing the summary interval (may not match with bar interval write threshold)
		while (g_bargraphBarIntervalsCached)
		{
#if 1 // Atmel fat driver
			write(bargraphFileHandle, g_bargraphBarIntervalReadPtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
#else // Port fat driver
			fl_fwrite(g_bargraphBarIntervalReadPtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA), 1, bargraphFileHandle);
#endif

			g_pendingBargraphRecord.header.dataLength += sizeof(BARGRAPH_BAR_INTERVAL_DATA);

			// Advance the Bar Interval global buffer pointer
			AdvanceBarIntervalBufPtr(READ_PTR);

			g_bargraphBarIntervalsCached--;
		}

#if 1 // Atmel fat driver
		write(bargraphFileHandle, g_bargraphSummaryIntervalPtr, sizeof(CALCULATED_DATA_STRUCT));
#else // Port fat driver
		fl_fwrite(g_bargraphSummaryIntervalPtr, sizeof(CALCULATED_DATA_STRUCT), 1, bargraphFileHandle);
#endif

		g_pendingBargraphRecord.header.dataLength += sizeof(CALCULATED_DATA_STRUCT);

#if 1 // Atmel fat driver
		SetFileDateTimestamp(FS_DATE_LAST_WRITE);

		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(bargraphFileHandle);
#else // Port fat driver
		fl_fclose(bargraphFileHandle);
#endif

		debug("%s event file closed\r\n", (g_triggerRecord.op_mode == BARGRAPH_MODE) ? "Bargraph" : "Combo - Bargraph");

		g_fileAccessLock = AVAILABLE;
		g_spi1AccessLock = AVAILABLE;
	}

	// Update the job totals.
	UpdateBargraphJobTotals();

	// Clear the Summary Interval structure for use again
	memset(g_bargraphSummaryIntervalPtr, 0, sizeof(g_bargraphSummaryInterval));

	// Clear out the Freq Calc structure for use again
	memset(&g_bargraphFreqCalcBuffer, 0, sizeof(g_bargraphFreqCalcBuffer));
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 CalculateBargraphData(void)
{
	// Temp variables, assigned as static to prevent storing on stack
	static uint32 vsTemp;
	static uint16 aTempNorm, rTempNorm, vTempNorm, tTempNorm;
	static DATE_TIME_STRUCT aTempTime, rTempTime, vTempTime, tTempTime;
	static uint8 g_aJobFreqFlag = NO, g_rJobFreqFlag = NO, g_vJobFreqFlag = NO, g_tJobFreqFlag = NO;
	static uint8 g_aJobPeakMatch = NO, g_rJobPeakMatch = NO, g_vJobPeakMatch = NO, g_tJobPeakMatch = NO;

	int32 falloutCounter = SAMPLE_RATE_512;
	SAMPLE_DATA_STRUCT currentDataSample;

	// While the bargraph data read pointer has not caught up with the write pointer and the fallout counter hasn't reached zero
	while ((g_bargraphDataReadPtr != g_bargraphDataWritePtr) && (--falloutCounter > 0))
	{
		// Read the next data sample set to be processed
		currentDataSample = *(SAMPLE_DATA_STRUCT*)g_bargraphDataReadPtr;

		// Increment to the next data sample set
		g_bargraphDataReadPtr += NUMBER_OF_CHANNELS_DEFAULT;
		
		// Wrap the data read pointer if at or beyond the end of the buffer
		if (g_bargraphDataReadPtr >= g_bargraphDataEndPtr) g_bargraphDataReadPtr = g_bargraphDataStartPtr;

		// Adjust for the correct bit accuracy
		currentDataSample.a >>= g_bitShiftForAccuracy;
		currentDataSample.r >>= g_bitShiftForAccuracy;
		currentDataSample.v >>= g_bitShiftForAccuracy;
		currentDataSample.t >>= g_bitShiftForAccuracy;

		// Normalize the raw data without the message bits
		aTempNorm = FixDataToZero(currentDataSample.a);
		rTempNorm = FixDataToZero(currentDataSample.r);
		vTempNorm = FixDataToZero(currentDataSample.v);
		tTempNorm = FixDataToZero(currentDataSample.t);

		// Find the vector sum of the current sample
		vsTemp =((uint32)rTempNorm * (uint32)rTempNorm) +
				((uint32)vTempNorm * (uint32)vTempNorm) +
				((uint32)tTempNorm * (uint32)tTempNorm);

		//=================================================
		// Impulse Interval
		//=================================================
		if (g_impulseMenuCount >= g_triggerRecord.berec.impulseMenuUpdateSecs)
		{
			g_impulseMenuCount = 0;
			g_aImpulsePeak = g_rImpulsePeak = g_vImpulsePeak = g_tImpulsePeak = 0;
			g_vsImpulsePeak = 0;
		}

		// ---------------------------------
		// A, R, V, T channel and Vector Sum
		// ---------------------------------
		// Store the max A, R, V or T normalized value if a new max was found
		if (aTempNorm > g_aImpulsePeak) g_aImpulsePeak = aTempNorm;
		if (rTempNorm > g_rImpulsePeak) g_rImpulsePeak = rTempNorm;
		if (vTempNorm > g_vImpulsePeak) g_vImpulsePeak = vTempNorm;
		if (tTempNorm > g_tImpulsePeak) g_tImpulsePeak = tTempNorm;
		if (vsTemp > g_vsImpulsePeak) g_vsImpulsePeak = vsTemp;

		//=================================================
		// Bar Interval
		//=================================================
		// ---------
		// A channel
		// ---------
		// Store the max Air normalized value if a new max was found
		if (aTempNorm > g_bargraphBarIntervalWritePtr->aMax)
			g_bargraphBarIntervalWritePtr->aMax = aTempNorm;

		// ---------------
		// R, V, T channel
		// ---------------
		// Store the max R, V or T normalized value if a new max was found
		if (rTempNorm > g_bargraphBarIntervalWritePtr->rvtMax)
			g_bargraphBarIntervalWritePtr->rvtMax = rTempNorm;

		if (vTempNorm > g_bargraphBarIntervalWritePtr->rvtMax)
			g_bargraphBarIntervalWritePtr->rvtMax = vTempNorm;

		if (tTempNorm > g_bargraphBarIntervalWritePtr->rvtMax)
			g_bargraphBarIntervalWritePtr->rvtMax = tTempNorm;

		// ----------
		// Vector Sum
		// ----------
		// Store the max Vector Sum if a new max was found
		if (vsTemp > g_bargraphBarIntervalWritePtr->vsMax)
			g_bargraphBarIntervalWritePtr->vsMax = vsTemp;

		//=================================================
		// Summary Interval
		//=================================================
		// ---------
		// A channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (aTempNorm >= g_bargraphSummaryIntervalPtr->a.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (aTempNorm == g_bargraphSummaryIntervalPtr->a.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.a.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				aTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSummaryIntervalPtr->a.peak = aTempNorm;
				// Update timestamp
				g_bargraphSummaryIntervalPtr->a_Time = GetCurrentTime();
				// Reset match flag since a new peak was found
				g_bargraphFreqCalcBuffer.a.matchFlag = FALSE;
			}

			if (aTempNorm >= g_aJobPeak)
			{
				if (aTempNorm == g_aJobPeak)
					g_aJobPeakMatch = YES;
				else
				{
					g_aJobPeak = aTempNorm;
					g_aJobPeakMatch = NO;
				}

				g_aJobFreqFlag = YES;
			}

			// Set update flag since we have either a matched or a new max
			g_bargraphFreqCalcBuffer.a.updateFlag = TRUE;
		}

		// ---------
		// R channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (rTempNorm >= g_bargraphSummaryIntervalPtr->r.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (rTempNorm == g_bargraphSummaryIntervalPtr->r.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.r.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				rTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSummaryIntervalPtr->r.peak = rTempNorm;
				// Update timestamp
				g_bargraphSummaryIntervalPtr->r_Time = GetCurrentTime();
				// Reset match flag since a new peak was found
				g_bargraphFreqCalcBuffer.r.matchFlag = FALSE;
			}

			if (rTempNorm >= g_rJobPeak)
			{
				if (rTempNorm == g_rJobPeak)
					g_rJobPeakMatch = YES;
				else
				{
					g_rJobPeak = rTempNorm;
					g_rJobPeakMatch = NO;
				}

				g_rJobFreqFlag = YES;
			}

			// Set update flag since we have either a matched or a new max
			g_bargraphFreqCalcBuffer.r.updateFlag = TRUE;
		}

		// ---------
		// V channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (vTempNorm >= g_bargraphSummaryIntervalPtr->v.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (vTempNorm == g_bargraphSummaryIntervalPtr->v.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.v.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				vTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSummaryIntervalPtr->v.peak = vTempNorm;
				// Update timestamp
				g_bargraphSummaryIntervalPtr->v_Time = GetCurrentTime();
				// Reset match flag since a new peak was found
				g_bargraphFreqCalcBuffer.v.matchFlag = FALSE;
			}

			if (vTempNorm >= g_vJobPeak)
			{
				if (vTempNorm == g_vJobPeak)
					g_vJobPeakMatch = YES;
				else
				{
					g_vJobPeak = vTempNorm;
					g_vJobPeakMatch = NO;
				}

				g_vJobFreqFlag = YES;
			}

			// Set update flag since we have either a matched or a new max
			g_bargraphFreqCalcBuffer.v.updateFlag = TRUE;
		}

		// ---------
		// T channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (tTempNorm >= g_bargraphSummaryIntervalPtr->t.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (tTempNorm == g_bargraphSummaryIntervalPtr->t.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.t.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				tTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSummaryIntervalPtr->t.peak = tTempNorm;
				// Update timestamp
				g_bargraphSummaryIntervalPtr->t_Time = GetCurrentTime();
				// Reset match flag since a new peak was found
				g_bargraphFreqCalcBuffer.t.matchFlag = FALSE;
			}

			if (tTempNorm >= g_tJobPeak)
			{
				if (tTempNorm == g_tJobPeak)
					g_tJobPeakMatch = YES;
				else
				{
					g_tJobPeak = tTempNorm;
					g_tJobPeakMatch = NO;
				}

				g_tJobFreqFlag = YES;
			}

			// Set update flag since we have either a matched or a new max
			g_bargraphFreqCalcBuffer.t.updateFlag = TRUE;
		}

		// ----------
		// Vector Sum
		// ----------
		// Store the max Vector Sum if a new max was found
		if (vsTemp > g_bargraphSummaryIntervalPtr->vectorSumPeak)
		{
			// Store max vector sum
			g_bargraphSummaryIntervalPtr->vectorSumPeak = vsTemp;
			// Store timestamp
			g_bargraphSummaryIntervalPtr->vs_Time = GetCurrentTime();

			if (vsTemp > g_vsJobPeak)
			{
				g_vsJobPeak = vsTemp;
			}
		}

		//=================================================
		// Freq Calc Information
		//=================================================
		// ------------
		// All Channels
		// ------------
		// Check if the freq count is zero, meaning initial sample or the start of a new summary interval (doesn't matter which channel is checked)
		if(g_bargraphFreqCalcBuffer.a.freq_count == 0)
		{
			g_bargraphFreqCalcBuffer.a.sign = (uint16)(currentDataSample.a & g_bitAccuracyMidpoint);
			g_bargraphFreqCalcBuffer.r.sign = (uint16)(currentDataSample.r & g_bitAccuracyMidpoint);
			g_bargraphFreqCalcBuffer.v.sign = (uint16)(currentDataSample.v & g_bitAccuracyMidpoint);
			g_bargraphFreqCalcBuffer.t.sign = (uint16)(currentDataSample.t & g_bitAccuracyMidpoint);
		}

		// ---------
		// A channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_bargraphFreqCalcBuffer.a.sign ^ (currentDataSample.a & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.a.sign = (uint16)(currentDataSample.a & g_bitAccuracyMidpoint);

			// If the update flag was set, update freq count information
			if (g_bargraphFreqCalcBuffer.a.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_bargraphFreqCalcBuffer.a.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_bargraphFreqCalcBuffer.a.freq_count > g_bargraphSummaryIntervalPtr->a.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSummaryIntervalPtr->a.frequency = g_bargraphFreqCalcBuffer.a.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSummaryIntervalPtr->a_Time = aTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSummaryIntervalPtr->a.frequency = g_bargraphFreqCalcBuffer.a.freq_count;
				}

				// Reset flags
				g_bargraphFreqCalcBuffer.a.updateFlag = FALSE;
				g_bargraphFreqCalcBuffer.a.matchFlag = FALSE;
			}

			if (g_aJobFreqFlag == YES)
			{
				if (g_aJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.a.freq_count > g_aJobFreq)
						g_aJobFreq = g_bargraphFreqCalcBuffer.a.freq_count;

					g_aJobPeakMatch = NO;
				}
				else
				{
					g_aJobFreq = g_bargraphFreqCalcBuffer.a.freq_count;
				}

				g_aJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_bargraphFreqCalcBuffer.a.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_bargraphFreqCalcBuffer.a.freq_count++;
		}

		// ---------
		// R channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_bargraphFreqCalcBuffer.r.sign ^ (currentDataSample.r & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.r.sign = (uint16)(currentDataSample.r & g_bitAccuracyMidpoint);

			// If the update flag was set, update freq count information
			if (g_bargraphFreqCalcBuffer.r.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_bargraphFreqCalcBuffer.r.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_bargraphFreqCalcBuffer.r.freq_count > g_bargraphSummaryIntervalPtr->r.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSummaryIntervalPtr->r.frequency = g_bargraphFreqCalcBuffer.r.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSummaryIntervalPtr->r_Time = rTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSummaryIntervalPtr->r.frequency = g_bargraphFreqCalcBuffer.r.freq_count;
				}

				// Reset flags
				g_bargraphFreqCalcBuffer.r.updateFlag = FALSE;
				g_bargraphFreqCalcBuffer.r.matchFlag = FALSE;
			}

			if (g_rJobFreqFlag == YES)
			{
				if (g_rJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.r.freq_count > g_rJobFreq)
						g_rJobFreq = g_bargraphFreqCalcBuffer.r.freq_count;

					g_rJobPeakMatch = NO;
				}
				else
				{
					g_rJobFreq = g_bargraphFreqCalcBuffer.r.freq_count;
				}

				g_rJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_bargraphFreqCalcBuffer.r.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_bargraphFreqCalcBuffer.r.freq_count++;
		}

		// ---------
		// V channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_bargraphFreqCalcBuffer.v.sign ^ (currentDataSample.v & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.v.sign = (uint16)(currentDataSample.v & g_bitAccuracyMidpoint);

			// If the update flag was set, update freq count information
			if (g_bargraphFreqCalcBuffer.v.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_bargraphFreqCalcBuffer.v.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_bargraphFreqCalcBuffer.v.freq_count > g_bargraphSummaryIntervalPtr->v.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSummaryIntervalPtr->v.frequency = g_bargraphFreqCalcBuffer.v.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSummaryIntervalPtr->v_Time = vTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSummaryIntervalPtr->v.frequency = g_bargraphFreqCalcBuffer.v.freq_count;
				}

				// Reset flags
				g_bargraphFreqCalcBuffer.v.updateFlag = FALSE;
				g_bargraphFreqCalcBuffer.v.matchFlag = FALSE;
			}

			if (g_vJobFreqFlag == YES)
			{
				if (g_vJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.v.freq_count > g_vJobFreq)
						g_vJobFreq = g_bargraphFreqCalcBuffer.v.freq_count;

					g_vJobPeakMatch = NO;
				}
				else
				{
					g_vJobFreq = g_bargraphFreqCalcBuffer.v.freq_count;
				}

				g_vJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_bargraphFreqCalcBuffer.v.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_bargraphFreqCalcBuffer.v.freq_count++;
		}

		// ---------
		// T channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_bargraphFreqCalcBuffer.t.sign ^ (currentDataSample.t & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.t.sign = (uint16)(currentDataSample.t & g_bitAccuracyMidpoint);

			// If the update flag was set, update freq count information
			if (g_bargraphFreqCalcBuffer.t.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_bargraphFreqCalcBuffer.t.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_bargraphFreqCalcBuffer.t.freq_count > g_bargraphSummaryIntervalPtr->t.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSummaryIntervalPtr->t.frequency = g_bargraphFreqCalcBuffer.t.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSummaryIntervalPtr->t_Time = tTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSummaryIntervalPtr->t.frequency = g_bargraphFreqCalcBuffer.t.freq_count;
				}

				// Reset flags
				g_bargraphFreqCalcBuffer.t.updateFlag = FALSE;
				g_bargraphFreqCalcBuffer.t.matchFlag = FALSE;
			}

			if (g_tJobFreqFlag == YES)
			{
				if (g_tJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.t.freq_count > g_tJobFreq)
						g_tJobFreq = g_bargraphFreqCalcBuffer.t.freq_count;

					g_tJobPeakMatch = NO;
				}
				else
				{
					g_tJobFreq = g_bargraphFreqCalcBuffer.t.freq_count;
				}

				g_tJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_bargraphFreqCalcBuffer.t.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_bargraphFreqCalcBuffer.t.freq_count++;
		}

		//=================================================
		// End of Bar Interval
		//=================================================
		if (++g_barSampleCount == (uint32)(g_triggerRecord.bgrec.barInterval * g_triggerRecord.trec.sample_rate))
		{
			g_bargraphBarIntervalsCached++;
			g_barSampleCount = 0;

			// Advance the Bar Interval global buffer pointer
			AdvanceBarIntervalBufPtr(WRITE_PTR);

			// Check if enough bar intervals have been cached
			if (g_bargraphBarIntervalsCached >= NUM_OF_BAR_INTERVALS_TO_HOLD)
			{
				MoveBarIntervalDataToFile();
			}

			//=================================================
			// End of Summary Interval
			//=================================================
			if (++g_bargraphSummaryIntervalPtr->barIntervalsCaptured == (uint32)(g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval))
			{
				// Move Summary Interval data to the event file
				MoveSummaryIntervalDataToFile();
			}
		}

	} // While != loop

	if (g_bargraphDataWritePtr != g_bargraphDataReadPtr)
	{
		return (BG_BUFFER_NOT_EMPTY);
	}
	else
	{
		return (BG_BUFFER_EMPTY);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MoveStartOfBargraphEventRecordToFile(void)
{
#if 1 // Atmel fat driver
	int bargraphFileHandle = -1;
#else // Port fat driver
	FL_FILE* bargraphFileHandle = NULL;
#endif

	if (g_fileAccessLock != AVAILABLE)
	{
		if (g_triggerRecord.op_mode == BARGRAPH_MODE) { ReportFileSystemAccessProblem("Save Bar Start"); }
		else { ReportFileSystemAccessProblem("Save Combo Bar Start"); }
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

		// Create new Bargraph event file
		bargraphFileHandle = GetEventFileHandle(g_pendingBargraphRecord.summary.eventNumber, CREATE_EVENT_FILE);

#if 1 // Atmel fat driver
		if (bargraphFileHandle == -1)
#else // Port fat driver
		if (bargraphFileHandle == NULL)
#endif
		{ debugErr("Failed to get a new file handle for the current %s event\r\n", (g_triggerRecord.op_mode == BARGRAPH_MODE) ? "Bargraph" : "Combo - Bargraph"); }

		// Write in the current but unfinished event record to provide an offset to start writing in the data
#if 1 // Atmel fat driver
		write(bargraphFileHandle, &g_pendingBargraphRecord, sizeof(EVT_RECORD));
		SetFileDateTimestamp(FS_DATE_LAST_WRITE);
		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(bargraphFileHandle);
#else // Port fat driver
		fl_fwrite(&g_pendingBargraphRecord, sizeof(EVT_RECORD), 1, bargraphFileHandle);
		fl_fclose(bargraphFileHandle);
#endif

		debug("%s event file closed\r\n", (g_triggerRecord.op_mode == BARGRAPH_MODE) ? "Bargraph" : "Combo - Bargraph");

		// Consume event number (also allow other events to be recorded during a Combo - Bargraph session)
		StoreCurrentEventNumber();

		if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			// Update the Waveform pending record with the new event number to use
			g_pendingEventRecord.summary.eventNumber = g_nextEventNumberToUse;
		}

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MoveEndOfBargraphEventRecordToFile(void)
{
#if 1 // Atmel fat driver
	int bargraphFileHandle = -1;
#else // Port fat driver
	FL_FILE* bargraphFileHandle = NULL;
#endif

	if (g_fileAccessLock != AVAILABLE)
	{
		if (g_triggerRecord.op_mode == BARGRAPH_MODE) { ReportFileSystemAccessProblem("Save Bar End"); }
		else { ReportFileSystemAccessProblem("Save Combo Bar End"); }
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

		// The following data will be filled in when the data has been moved over to flash.
		g_pendingBargraphRecord.header.summaryChecksum = 0xAABB;
		g_pendingBargraphRecord.header.dataChecksum = 0xCCDD;
		g_pendingBargraphRecord.header.dataCompression = 0;

		g_pendingBargraphRecord.summary.captured.endTime = GetCurrentTime();

		if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			// Mark Combo - Waveform event numbers captured
			if ((g_pendingBargraphRecord.summary.eventNumber + 1) == g_nextEventNumberToUse)
			{
				debug("Combo - Bargraph: No other events recorded during session\r\n");
			}
			else // ((g_pendingBargraphRecord.summary.eventNumber + 1) != g_nextEventNumberToUse)
			{
				g_pendingBargraphRecord.summary.captured.comboEventsRecordedDuringSession = (g_nextEventNumberToUse - (g_pendingBargraphRecord.summary.eventNumber + 1));
				g_pendingBargraphRecord.summary.captured.comboEventsRecordedStartNumber = (g_pendingBargraphRecord.summary.eventNumber + 1);
				g_pendingBargraphRecord.summary.captured.comboEventsRecordedEndNumber = (g_nextEventNumberToUse - 1);

				debug("Combo - Bargraph: Events recorded during session, Total: %d (%d to %d)\r\n", g_pendingBargraphRecord.summary.captured.comboEventsRecordedDuringSession,
						g_pendingBargraphRecord.summary.captured.comboEventsRecordedStartNumber, g_pendingBargraphRecord.summary.captured.comboEventsRecordedEndNumber);
			}
		}

		bargraphFileHandle = GetEventFileHandle(g_pendingBargraphRecord.summary.eventNumber, OVERWRITE_EVENT_FILE);

#if 1 // Atmel fat driver
		// Make sure at the beginning of the file
		file_seek(0, FS_SEEK_SET);

		// Rewrite the event record
		write(bargraphFileHandle, &g_pendingBargraphRecord, sizeof(EVT_RECORD));

		SetFileDateTimestamp(FS_DATE_LAST_WRITE);

		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(bargraphFileHandle);
#else // Port fat driver
		// Make sure at the beginning of the file
		fl_fseek(bargraphFileHandle, 0, SEEK_SET);

		// Rewrite the event record
		fl_fwrite(&g_pendingBargraphRecord, sizeof(EVT_RECORD), 1, bargraphFileHandle);

		fl_fclose(bargraphFileHandle);
#endif

		debug("%s event file closed\r\n", (g_triggerRecord.op_mode == BARGRAPH_MODE) ? "Bargraph" : "Combo - Bargraph");

		AddEventToSummaryList(&g_pendingBargraphRecord);

		g_fileAccessLock = AVAILABLE;

		UpdateSDCardUsageStats(sizeof(EVT_RECORD) + g_pendingBargraphRecord.header.dataLength);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdvanceBarIntervalBufPtr(uint8 bufferType)
{
	if (bufferType == READ_PTR)
	{
		g_bargraphBarIntervalReadPtr++;
		if (g_bargraphBarIntervalReadPtr > g_bargraphBarIntervalEndPtr)
			g_bargraphBarIntervalReadPtr = &(g_bargraphBarInterval[0]);
	}
	else // (bufferType == WRITE_PTR)
	{
		g_bargraphBarIntervalWritePtr++;
		if (g_bargraphBarIntervalWritePtr > g_bargraphBarIntervalEndPtr)
			g_bargraphBarIntervalWritePtr = &(g_bargraphBarInterval[0]);

		memset(g_bargraphBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateBargraphJobTotals(void)
{
	// Update Bargraph Job Totals structure with most recent Summary Interval max's
	// ---------
	// A channel
	// ---------
	if (g_bargraphSummaryIntervalPtr->a.peak > g_pendingBargraphRecord.summary.calculated.a.peak)
	{
		g_pendingBargraphRecord.summary.calculated.a = g_bargraphSummaryIntervalPtr->a;
		g_pendingBargraphRecord.summary.calculated.a_Time = g_bargraphSummaryIntervalPtr->a_Time;
	}
	else if (g_bargraphSummaryIntervalPtr->a.peak == g_pendingBargraphRecord.summary.calculated.a.peak)
	{
		if (g_bargraphSummaryIntervalPtr->a.frequency < g_pendingBargraphRecord.summary.calculated.a.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.a = g_bargraphSummaryIntervalPtr->a;
			g_pendingBargraphRecord.summary.calculated.a_Time = g_bargraphSummaryIntervalPtr->a_Time;
		}
	}

	// ---------
	// R channel
	// ---------
	if (g_bargraphSummaryIntervalPtr->r.peak > g_pendingBargraphRecord.summary.calculated.r.peak)
	{
		g_pendingBargraphRecord.summary.calculated.r = g_bargraphSummaryIntervalPtr->r;
		g_pendingBargraphRecord.summary.calculated.r_Time = g_bargraphSummaryIntervalPtr->r_Time;
	}
	else if (g_bargraphSummaryIntervalPtr->r.peak == g_pendingBargraphRecord.summary.calculated.r.peak)
	{
		if (g_bargraphSummaryIntervalPtr->r.frequency < g_pendingBargraphRecord.summary.calculated.r.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.r = g_bargraphSummaryIntervalPtr->r;
			g_pendingBargraphRecord.summary.calculated.r_Time = g_bargraphSummaryIntervalPtr->r_Time;
		}
	}

	// ---------
	// V channel
	// ---------
	if (g_bargraphSummaryIntervalPtr->v.peak > g_pendingBargraphRecord.summary.calculated.v.peak)
	{
		g_pendingBargraphRecord.summary.calculated.v = g_bargraphSummaryIntervalPtr->v;
		g_pendingBargraphRecord.summary.calculated.v_Time = g_bargraphSummaryIntervalPtr->v_Time;
	}
	else if (g_bargraphSummaryIntervalPtr->v.peak == g_pendingBargraphRecord.summary.calculated.v.peak)
	{
		if (g_bargraphSummaryIntervalPtr->v.frequency < g_pendingBargraphRecord.summary.calculated.v.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.v = g_bargraphSummaryIntervalPtr->v;
			g_pendingBargraphRecord.summary.calculated.v_Time = g_bargraphSummaryIntervalPtr->v_Time;
		}
	}

	// ---------
	// T channel
	// ---------
	if (g_bargraphSummaryIntervalPtr->t.peak > g_pendingBargraphRecord.summary.calculated.t.peak)
	{
		g_pendingBargraphRecord.summary.calculated.t = g_bargraphSummaryIntervalPtr->t;
		g_pendingBargraphRecord.summary.calculated.t_Time = g_bargraphSummaryIntervalPtr->t_Time;
	}
	else if (g_bargraphSummaryIntervalPtr->t.peak == g_pendingBargraphRecord.summary.calculated.t.peak)
	{
		if (g_bargraphSummaryIntervalPtr->t.frequency < g_pendingBargraphRecord.summary.calculated.t.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.t = g_bargraphSummaryIntervalPtr->t;
			g_pendingBargraphRecord.summary.calculated.t_Time = g_bargraphSummaryIntervalPtr->t_Time;
		}
	}

	// ----------
	// Vector Sum
	// ----------
	if (g_bargraphSummaryIntervalPtr->vectorSumPeak > g_pendingBargraphRecord.summary.calculated.vectorSumPeak)
	{
		g_pendingBargraphRecord.summary.calculated.vectorSumPeak = g_bargraphSummaryIntervalPtr->vectorSumPeak;
		g_pendingBargraphRecord.summary.calculated.vs_Time = g_bargraphSummaryIntervalPtr->vs_Time;
	}

	g_pendingBargraphRecord.summary.calculated.summariesCaptured = g_bargraphSummaryIntervalPtr->summariesCaptured;
	g_pendingBargraphRecord.summary.calculated.barIntervalsCaptured += g_bargraphSummaryIntervalPtr->barIntervalsCaptured;
	g_pendingBargraphRecord.summary.calculated.intervalEnd_Time = g_bargraphSummaryIntervalPtr->intervalEnd_Time;
	g_pendingBargraphRecord.summary.calculated.batteryLevel = g_bargraphSummaryIntervalPtr->batteryLevel;
	g_pendingBargraphRecord.summary.calculated.calcStructEndFlag = 0xEECCCCEE;
}
