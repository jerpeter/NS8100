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
#include "Old_Board.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "Record.h"
#include "Menu.h"
#include "ProcessBargraph.h"
#include "Flash.h"
#include "EventProcessing.h"
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

//*****************************************************************************
// Function:	StartNewBargraph
// Purpose :
//*****************************************************************************
void StartNewBargraph(void)
{
	g_bargraphSummaryPtr = NULL;

	// Get the address of an empty Ram summary
	if (GetRamSummaryEntry(&g_bargraphSummaryPtr) == FALSE)
	{
		debug("Out of Ram Summary Entrys\n");
		return;
	}

	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_bargraphBarInterval[0]), 0, (sizeof(BARGRAPH_BAR_INTERVAL_DATA) * NUM_OF_BAR_INTERVAL_BUFFERS));
	byteSet(&(g_bargraphSummaryInterval[0]), 0, (SUMMARY_INTERVAL_SIZE_IN_BYTES * NUM_OF_SUM_INTERVAL_BUFFERS));

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(&g_bargraphFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));

	g_summaryCount = 0;
	g_oneSecondCnt = 0;			// Count the secs so that we can increment the sum capture rate.
	g_oneMinuteCount = 0;		// Flag to mark the end of a summary sample in terms of time.
	g_barIntervalCnt = 0;		// Count the number of samples that make up a bar interval.
	g_summaryIntervalCnt = 0;	// Count the number of bars that make up a summary interval.
	g_totalBarIntervalCnt = 0;

	g_bargraphBarIntervalWritePtr = &(g_bargraphBarInterval[0]);
	g_bargraphBarIntervalReadPtr = &(g_bargraphBarInterval[0]);
	g_bargraphSumIntervalWritePtr = &(g_bargraphSummaryInterval[0]);
	g_bargraphSumIntervalReadPtr = &(g_bargraphSummaryInterval[0]);

	// Clear out the Bar Interval buffer to be used next
	byteSet(g_bargraphBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(g_bargraphSumIntervalWritePtr, 0, SUMMARY_INTERVAL_SIZE_IN_BYTES);

	MoveStartOfBargraphEventRecordToFlash();

	// Update the current monitor log entry
	updateMonitorLogEntry();
}

//*****************************************************************************
// Function:
// Purpose :
//*****************************************************************************
void EndBargraph(void)
{
	uint32 eventDataFlag;

	while (CalculateBargraphData() == BG_BUFFER_NOT_EMPTY) {}

	// For the last time, put the data into the event buffer.
	eventDataFlag = moveBarIntervalDataToFile();

	// If no data, both flags are zero, then skip, end of record.
	if ((eventDataFlag > 0) || (g_summaryIntervalCnt>0))
	{
		moveSummaryIntervalDataToFile();
	}

	MoveEndOfBargraphEventRecordToFlash();
	g_fileProcessActiveUsbLockout = OFF;
}

/*****************************************************************************
* Function:		ProcessBargraphData (Step 2)
* Purpose:		Copy A/D channel data from Pretrigger buffer into event buffer
******************************************************************************/
#if 0
void ProcessBargraphData(void)
{
	// Check to see if we have a chunk of ram buffer to write, otherwise check for data wrapping.
	if ((g_bargraphDataEndPtr - g_bargraphDataWritePtr) >= 4)
	{
		// Move from the Pretrigger buffer to our large ram buffer.
		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;

		// Check for the end and if so go to the top
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) 
			g_bargraphDataWritePtr = g_bargraphDataStartPtr;
	}
	else
	{
		// Move from the Pretrigger buffer to our large ram buffer, but check for data wrapping.
		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

		*g_bargraphDataWritePtr++ = *g_tailOfPretriggerBuff++;
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;
	}

	// Handle Pretrigger buffer pointer for circular buffer
	//if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;

	// Alert system that we have data in ram buffer, raise flag to calculate and move data to flash.
	raiseSystemEventFlag(BARGRAPH_EVENT);
}
#endif

//*****************************************************************************
// Function: moveBarIntervalDataToFile(void)
// Purpose : Transfer the data from the bar interval buffers into the event file
//*****************************************************************************
uint32 moveBarIntervalDataToFile(void)
{
	uint32 accumulatedBarIntervalCount = g_barIntervalCnt;

	// If Bar Intervals have been cached
	if (g_barIntervalCnt > 0)
	{
		// Reset the bar interval count
		g_barIntervalCnt = 0;

		fl_fwrite(g_bargraphBarIntervalWritePtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA), 1, g_currentEventFileHandle);		
		g_pendingBargraphRecord.header.dataLength += sizeof(BARGRAPH_BAR_INTERVAL_DATA);

		// Advance the Bar Interval global buffer pointer
		advanceBarIntervalBufPtr(WRITE_PTR);

		// Clear out the Bar Interval buffer to be used next
		byteSet(g_bargraphBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));

		// Count the total number of intervals captured.
		g_bargraphSumIntervalWritePtr->barIntervalsCaptured++;
	}

	// Flag for the end of bargraph, used to indicate that a bar interval was stored, thus a summary should be too
	return (accumulatedBarIntervalCount);
}

//*****************************************************************************
// Function: moveSummaryIntervalDataToFile(void)
// Purpose : Transfer the data from the summary interval buffers into the event
//				write buffers for storage into flash.
//*****************************************************************************
void moveSummaryIntervalDataToFile(void)
{
#if 0 // Port lost change
	float rFreq, vFreq, tFreq;
#else // Updated
	float rFreq = (float)0, vFreq = (float)0, tFreq = (float)0;
#endif

#if 0 // Port lost change
	rFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1));
	vFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1));
	tFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1));
#else // Updated
	// Note: This should be raw unadjusted freq
	if(g_bargraphSumIntervalWritePtr->r.frequency > 0)
	{
		rFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1));
	}
	
	if(g_bargraphSumIntervalWritePtr->v.frequency > 0)
	{
		vFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1));
	}
	
	if(g_bargraphSumIntervalWritePtr->t.frequency > 0)
	{
		tFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1));
	}
#endif

	// Calculate the Peak Displacement
	g_bargraphSumIntervalWritePtr->a.displacement = 0;
	g_bargraphSumIntervalWritePtr->r.displacement = (uint32)(g_bargraphSumIntervalWritePtr->r.peak * 1000000 / 2 / PI / rFreq);
	g_bargraphSumIntervalWritePtr->v.displacement = (uint32)(g_bargraphSumIntervalWritePtr->v.peak * 1000000 / 2 / PI / vFreq);
	g_bargraphSumIntervalWritePtr->t.displacement = (uint32)(g_bargraphSumIntervalWritePtr->t.peak * 1000000 / 2 / PI / tFreq);

#if 1 // Updated (Port lost change)
	// Calculate the Peak Acceleration
	g_bargraphSumIntervalWritePtr->a.acceleration = 0;
	g_bargraphSumIntervalWritePtr->r.acceleration = (uint32)(g_bargraphSumIntervalWritePtr->r.peak * 1000 * 2 * PI * rFreq);
	g_bargraphSumIntervalWritePtr->v.acceleration = (uint32)(g_bargraphSumIntervalWritePtr->v.peak * 1000 * 2 * PI * vFreq);
	g_bargraphSumIntervalWritePtr->t.acceleration = (uint32)(g_bargraphSumIntervalWritePtr->t.peak * 1000 * 2 * PI * tFreq);
#endif

	// Store timestamp for the end of the summary interval
	g_summaryCount++;
	g_bargraphSumIntervalWritePtr->summariesCaptured = g_summaryCount;
	g_bargraphSumIntervalWritePtr->intervalEnd_Time = getCurrentTime();
	g_bargraphSumIntervalWritePtr->batteryLevel =
		(uint32)(100.0 * getExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
	g_bargraphSumIntervalWritePtr->calcStructEndFlag = 0xEECCCCEE;	// End structure flag

	// Reset summary interval count
	g_summaryIntervalCnt = 0;

	fl_fwrite(g_bargraphSumIntervalWritePtr, sizeof(CALCULATED_DATA_STRUCT), 1, g_currentEventFileHandle);		
	g_pendingBargraphRecord.header.dataLength += sizeof(CALCULATED_DATA_STRUCT);

	// Move update the job totals.
	UpdateBargraphJobTotals(g_bargraphSumIntervalWritePtr);

	// Advance the Summary Interval global buffer pointer
	advanceSumIntervalBufPtr(WRITE_PTR);

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(g_bargraphSumIntervalWritePtr, 0, sizeof(CALCULATED_DATA_STRUCT));
	byteSet(&g_bargraphFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));
}

/*****************************************************************************
	Function:	CalculateBargraphData
	Purpose :	Calculate bargraph specific info from the raw A/D data that
				was copied into the event buffer
******************************************************************************/
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
		if (aTempNorm >= g_bargraphSumIntervalWritePtr->a.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (aTempNorm == g_bargraphSumIntervalWritePtr->a.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.a.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				aTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSumIntervalWritePtr->a.peak = aTempNorm;
				// Update timestamp
				g_bargraphSumIntervalWritePtr->a_Time = getCurrentTime();
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
		if (rTempNorm >= g_bargraphSumIntervalWritePtr->r.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (rTempNorm == g_bargraphSumIntervalWritePtr->r.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.r.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				rTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSumIntervalWritePtr->r.peak = rTempNorm;
				// Update timestamp
				g_bargraphSumIntervalWritePtr->r_Time = getCurrentTime();
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
		if (vTempNorm >= g_bargraphSumIntervalWritePtr->v.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (vTempNorm == g_bargraphSumIntervalWritePtr->v.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.v.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				vTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSumIntervalWritePtr->v.peak = vTempNorm;
				// Update timestamp
				g_bargraphSumIntervalWritePtr->v_Time = getCurrentTime();
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
		if (tTempNorm >= g_bargraphSumIntervalWritePtr->t.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (tTempNorm == g_bargraphSumIntervalWritePtr->t.peak)
			{
				// Sample matches current max, set match flag
				g_bargraphFreqCalcBuffer.t.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				tTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_bargraphSumIntervalWritePtr->t.peak = tTempNorm;
				// Update timestamp
				g_bargraphSumIntervalWritePtr->t_Time = getCurrentTime();
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
		if (vsTemp > g_bargraphSumIntervalWritePtr->vectorSumPeak)
		{
			// Store max vector sum
			g_bargraphSumIntervalWritePtr->vectorSumPeak = vsTemp;
			// Store timestamp
			g_bargraphSumIntervalWritePtr->vs_Time = getCurrentTime();

			if (vsTemp > g_vsJobPeak)
			{
				g_vsJobPeak = vsTemp;
			}
		}

		//=================================================
		// Freq Calc Information
		//=================================================
#if 1 // Updated (Port lost change)
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
#endif

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
					if (g_bargraphFreqCalcBuffer.a.freq_count > g_bargraphSumIntervalWritePtr->a.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSumIntervalWritePtr->a.frequency = g_bargraphFreqCalcBuffer.a.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSumIntervalWritePtr->a_Time = aTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSumIntervalWritePtr->a.frequency = g_bargraphFreqCalcBuffer.a.freq_count;
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
					if (g_bargraphFreqCalcBuffer.r.freq_count > g_bargraphSumIntervalWritePtr->r.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSumIntervalWritePtr->r.frequency = g_bargraphFreqCalcBuffer.r.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSumIntervalWritePtr->r_Time = rTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSumIntervalWritePtr->r.frequency = g_bargraphFreqCalcBuffer.r.freq_count;
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
					if (g_bargraphFreqCalcBuffer.v.freq_count > g_bargraphSumIntervalWritePtr->v.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSumIntervalWritePtr->v.frequency = g_bargraphFreqCalcBuffer.v.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSumIntervalWritePtr->v_Time = vTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSumIntervalWritePtr->v.frequency = g_bargraphFreqCalcBuffer.v.freq_count;
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
					if (g_bargraphFreqCalcBuffer.t.freq_count > g_bargraphSumIntervalWritePtr->t.frequency)
					{
						// Save the new count (lower freq)
						g_bargraphSumIntervalWritePtr->t.frequency = g_bargraphFreqCalcBuffer.t.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_bargraphSumIntervalWritePtr->t_Time = tTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_bargraphSumIntervalWritePtr->t.frequency = g_bargraphFreqCalcBuffer.t.freq_count;
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
		if (++g_barIntervalCnt >= (uint32)(g_triggerRecord.bgrec.barInterval * g_triggerRecord.trec.sample_rate))
		{
			moveBarIntervalDataToFile();

			//=================================================
			// End of Summary Interval
			//=================================================
			if (++g_summaryIntervalCnt >= (uint32)(g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval))
			{
				moveSummaryIntervalDataToFile();
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

#if 0 // ns7100
//*****************************************************************************
// Function: MoveBargraphEventDataToFlash(void)
// Purpose : Move the data in the bargarphg_eventDataBufferer into flash. Copy
//				all the data from the buffer until the readPtr == writePtr.
//*****************************************************************************
void MoveBargraphEventDataToFlash(void)
{
	// While we haven't found the end of the Bargraph Event data
	while (g_bargraphEventDataReadPtr != g_bargraphEventDataWritePtr)
	{
		// Write in data (alternates between Air and RVT max normalized values) for a bar interval
#if 0 // ns7100
		storeData(g_bargraphEventDataReadPtr, 1);
#else // ns8100
		fl_fwrite(g_bargraphEventDataReadPtr, 1, 2, g_currentEventFileHandle);
#endif

		g_bargraphEventDataReadPtr += 1;

		// Check to see if the read ptr has gone past the end, if it has start at the top again.
		END_OF_EVENT_BUFFER_CHECK(g_bargraphEventDataReadPtr, g_bargraphEventDataStartPtr, g_bargraphEventDataEndPtr);
	}
}
#endif

//*****************************************************************************
// Function: MoveStartofBargraphEventRecordToFlash
// Purpose : Move the Bargraph Event Record into flash. Copy most of the
//				event record data. Unable to copy the captured data structures
//				until they are filled out. so only copy the header and system
//				data structs.
//*****************************************************************************
void MoveStartOfBargraphEventRecordToFlash(void)
{
	// Get new file handle
	g_currentEventFileHandle = getEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);
				
	if (g_currentEventFileHandle == NULL)
	{
		debugErr("Failed to get a new file handle for the current Bargraph event!\n");
	}			

	// Write in the current but unfinished event record to provide an offset to start writing in the data
	fl_fwrite(&g_pendingBargraphRecord, sizeof(EVT_RECORD), 1, g_currentEventFileHandle);
}


//*****************************************************************************
// Function: MoveEndOfBargraphEventRecordToFlash
// Purpose : Move the Bargraph Event Record into flash. Copy most of the
//				event record data. Unable to copy the captured data structures
//				until they are filled out. so only copy the header and system
//				data structs.
//*****************************************************************************
void MoveEndOfBargraphEventRecordToFlash(void)
{
	// The following data will be filled in when the data has been moved over to flash.
	g_pendingBargraphRecord.header.summaryChecksum = 0xAABB;
	g_pendingBargraphRecord.header.dataChecksum = 0xCCDD;
	g_pendingBargraphRecord.header.dataCompression = 0;

	g_pendingBargraphRecord.summary.captured.endTime = getCurrentTime();

	// Make sure at the beginning of the file
	fl_fseek(g_currentEventFileHandle, 0, SEEK_SET);

	// Rewrite the event record
	fl_fwrite(&g_pendingBargraphRecord, sizeof(EVT_RECORD), 1, g_currentEventFileHandle);

	fl_fclose(g_currentEventFileHandle);
	debug("Bargraph event file closed\n");

	// Store and increment the event number even if we do not save the event header information.
	storeCurrentEventNumber();
}

//*****************************************************************************
// Function:	advanceBarIntervalBufPtr
// Purpose :	handle advancing the bar interval buffer pointer
//*****************************************************************************
void advanceBarIntervalBufPtr(uint8 bufferType)
{
	if (bufferType == READ_PTR)
	{
		g_bargraphBarIntervalReadPtr++;
		if (g_bargraphBarIntervalReadPtr > g_bargraphBarIntervalEndPtr)
			g_bargraphBarIntervalReadPtr = &(g_bargraphBarInterval[0]);
	}
	else
	{
		g_bargraphBarIntervalWritePtr++;
		if (g_bargraphBarIntervalWritePtr > g_bargraphBarIntervalEndPtr)
			g_bargraphBarIntervalWritePtr = &(g_bargraphBarInterval[0]);
	}
}

//*****************************************************************************
// Function:	advanceSumIntervalBufPtr
// Purpose :	handle advancing the bar interval buffer pointer
//*****************************************************************************
void advanceSumIntervalBufPtr(uint8 bufferType)
{

	if (bufferType == READ_PTR)
	{
		g_bargraphSumIntervalReadPtr++;
		if (g_bargraphSumIntervalReadPtr > g_bargraphSumIntervalEndPtr)
			g_bargraphSumIntervalReadPtr = &(g_bargraphSummaryInterval[0]);
	}
	else
	{
		g_bargraphSumIntervalWritePtr++;
		if (g_bargraphSumIntervalWritePtr > g_bargraphSumIntervalEndPtr)
			g_bargraphSumIntervalWritePtr = &(g_bargraphSummaryInterval[0]);
	}
}



//*****************************************************************************
// FUNCTION:	void UpdateBargraphJobTotals(void)
// DESCRIPTION
//*****************************************************************************
void UpdateBargraphJobTotals(CALCULATED_DATA_STRUCT* sumIntervalPtr)
{
	// Update Bargraph Job Totals structure with most recent Summary Interval max's
	// ---------
	// A channel
	// ---------
	if (sumIntervalPtr->a.peak > g_pendingBargraphRecord.summary.calculated.a.peak)
	{
		g_pendingBargraphRecord.summary.calculated.a = sumIntervalPtr->a;
		g_pendingBargraphRecord.summary.calculated.a_Time = sumIntervalPtr->a_Time;
	}
	else if (sumIntervalPtr->a.peak == g_pendingBargraphRecord.summary.calculated.a.peak)
	{
		if (sumIntervalPtr->a.frequency > g_pendingBargraphRecord.summary.calculated.a.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.a = sumIntervalPtr->a;
			g_pendingBargraphRecord.summary.calculated.a_Time = sumIntervalPtr->a_Time;
		}
	}

	// ---------
	// R channel
	// ---------
	if (sumIntervalPtr->r.peak > g_pendingBargraphRecord.summary.calculated.r.peak)
	{
		g_pendingBargraphRecord.summary.calculated.r = sumIntervalPtr->r;
		g_pendingBargraphRecord.summary.calculated.r_Time = sumIntervalPtr->r_Time;
	}
	else if (sumIntervalPtr->r.peak == g_pendingBargraphRecord.summary.calculated.r.peak)
	{
		if (sumIntervalPtr->r.frequency > g_pendingBargraphRecord.summary.calculated.r.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.r = sumIntervalPtr->r;
			g_pendingBargraphRecord.summary.calculated.r_Time = sumIntervalPtr->r_Time;
		}
	}

	// ---------
	// V channel
	// ---------
	if (sumIntervalPtr->v.peak > g_pendingBargraphRecord.summary.calculated.v.peak)
	{
		g_pendingBargraphRecord.summary.calculated.v = sumIntervalPtr->v;
		g_pendingBargraphRecord.summary.calculated.v_Time = sumIntervalPtr->v_Time;
	}
	else if (sumIntervalPtr->v.peak == g_pendingBargraphRecord.summary.calculated.v.peak)
	{
		if (sumIntervalPtr->v.frequency > g_pendingBargraphRecord.summary.calculated.v.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.v = sumIntervalPtr->v;
			g_pendingBargraphRecord.summary.calculated.v_Time = sumIntervalPtr->v_Time;
		}
	}

	// ---------
	// T channel
	// ---------
	if (sumIntervalPtr->t.peak > g_pendingBargraphRecord.summary.calculated.t.peak)
	{
		g_pendingBargraphRecord.summary.calculated.t = sumIntervalPtr->t;
		g_pendingBargraphRecord.summary.calculated.t_Time = sumIntervalPtr->t_Time;
	}
	else if (sumIntervalPtr->t.peak == g_pendingBargraphRecord.summary.calculated.t.peak)
	{
		if (sumIntervalPtr->t.frequency > g_pendingBargraphRecord.summary.calculated.t.frequency)
		{
			g_pendingBargraphRecord.summary.calculated.t = sumIntervalPtr->t;
			g_pendingBargraphRecord.summary.calculated.t_Time = sumIntervalPtr->t_Time;
		}
	}

	// ----------
	// Vector Sum
	// ----------
	if (sumIntervalPtr->vectorSumPeak > g_pendingBargraphRecord.summary.calculated.vectorSumPeak)
	{
		g_pendingBargraphRecord.summary.calculated.vectorSumPeak = sumIntervalPtr->vectorSumPeak;
		g_pendingBargraphRecord.summary.calculated.vs_Time = sumIntervalPtr->vs_Time;
	}

	g_pendingBargraphRecord.summary.calculated.summariesCaptured = sumIntervalPtr->summariesCaptured;
	g_pendingBargraphRecord.summary.calculated.barIntervalsCaptured += sumIntervalPtr->barIntervalsCaptured;
	g_pendingBargraphRecord.summary.calculated.intervalEnd_Time = sumIntervalPtr->intervalEnd_Time;
	g_pendingBargraphRecord.summary.calculated.batteryLevel = sumIntervalPtr->batteryLevel;
	g_pendingBargraphRecord.summary.calculated.calcStructEndFlag = 0xEECCCCEE;
}

//*****************************************************************************
// Function:	checkSpaceForBarSummaryInterval
// Purpose:
//*****************************************************************************
BOOLEAN checkSpaceForBarSummaryInterval(void)
{
	FLASH_USAGE_STRUCT flashStats;
	uint32 barIntervalSize;
	BOOLEAN spaceLeft;

	getFlashUsageStats(&flashStats);

	barIntervalSize = (sizeof(CALCULATED_DATA_STRUCT) + (((g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval) + 1) * 8));

	if (flashStats.sizeFree > barIntervalSize)
		spaceLeft = YES;
	else
		spaceLeft = NO;

	return (spaceLeft);
}

