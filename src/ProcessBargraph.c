///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: ProcessBargraph.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:55 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ProcessBargraph.c,v $
///	$Revision: 1.2 $
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
#include "Rec.h"
#include "Menu.h"
#include "ProcessBargraph.h"
#include "Flash.h"
#include "EventProcessing.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern uint16* endFlashSectorPtr;
extern EVT_RECORD g_RamEventRecord;				// The event record in Ram, being filled for the current event.
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;// Sensor information, calculated when sensor parameters change.
extern REC_EVENT_MN_STRUCT trig_rec;		// Current trigger record structure in ram.
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern REC_HELP_MN_STRUCT help_rec;
extern MN_EVENT_STRUCT mn_event_flags;		// Event flags to indicate current calling menu
extern SYS_EVENT_STRUCT SysEvents_flags;	// System flags for main loop processing.
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);

extern uint16* startOfPreTrigBuff;
extern uint16* tailOfPreTrigBuff;
extern uint16* endOfPreTrigBuff;

// Print flag to indicate printing attempt or skip printing.
extern uint8 print_out_mode;
extern uint16* gp_bg430DataStart;
extern uint16* gp_bg430DataWrite;
extern uint16* gp_bg430DataRead;
extern uint16* gp_bg430DataEnd;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
// This ptr points to the current/in use (ram) summary table entry. It is
// used to access the linkPtr which is really the flash event record Ptr.
// And the record ptr is the location in ram. i.e.
// EVENT_REC* currentEventRec = (EVENT_REC *)gBargraphSummaryPtr->linkPtr;
SUMMARY_DATA* gBargraphSummaryPtr = 0;

// For bargraph processing.
uint8 gOneMinuteCount = 0;					// Counters
uint32 gOneSecondCnt = 0;
uint32 gBarIntervalCnt = 0;
uint32 gTotalBarIntervalCnt = 0;
uint32 gSummaryIntervalCnt = 0;
uint16 gSummaryCnt = 1;

BARGRAPH_FREQ_CALC_BUFFER g_bargraphFreqCalcBuffer;

// A queue of buffers containing summary Interval data, so the
// summary interval data can be printed outside of the ISR context.
CALCULATED_DATA_STRUCT g_bargraphSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS];
CALCULATED_DATA_STRUCT* g_bargraphSumIntervalWritePtr = &(g_bargraphSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_bargraphSumIntervalReadPtr = &(g_bargraphSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_bargraphSumIntervalEndPtr = &(g_bargraphSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS-1]);

// A queue of buffers containing bar Interval data, so the
// bar interval data can be printed outside of the ISR context.
BARGRAPH_BAR_INTERVAL_DATA g_bargraphBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS];
BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalWritePtr = &(g_bargraphBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalReadPtr = &(g_bargraphBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_bargraphBarIntervalEndPtr = &(g_bargraphBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS - 1]);

// The following are the Event Data ram buffer pointers.
uint16 g_bargrapheventDataBufferer[BG_DATA_BUFFER_SIZE];
uint16* g_bargraphEventDataStartPtr;
uint16* g_bargraphEventDataWritePtr;
uint16* g_bargraphEventDataReadPtr;
uint16* g_bargraphEventDataEndPtr;

uint16 aImpulsePeak;
uint16 rImpulsePeak;
uint16 vImpulsePeak;
uint16 tImpulsePeak;
uint32 vsImpulsePeak;
uint16 impulseMenuCount;
uint16 aJobPeak;
uint16 aJobFreq;
uint16 rJobPeak;
uint16 rJobFreq;
uint16 vJobPeak;
uint16 vJobFreq;
uint16 tJobPeak;
uint16 tJobFreq;
uint32 vsJobPeak;

//*****************************************************************************
// Function:	StartNewBargraph
// Purpose :
//*****************************************************************************
void StartNewBargraph(void)
{
	gBargraphSummaryPtr = NULL;

#if 0 // fix_ns8100
	// Get the address of an empty Ram summary
	if (GetFlashSumEntry(&gBargraphSummaryPtr) == FALSE)
	{
		debug("Out of Flash Summary Entrys\n");
		return;
	}
	else
	{
		// Setup location of the event data in flash. True for bargraph and waveform.
		advFlashDataPtrToEventData(gBargraphSummaryPtr);
	}
#endif

	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_bargraphBarInterval[0]), 0, (sizeof(BARGRAPH_BAR_INTERVAL_DATA) * NUM_OF_BAR_INTERVAL_BUFFERS));

	byteSet(&(g_bargraphSummaryInterval[0]), 0, (SUMMARY_INTERVAL_SIZE_IN_BYTES * NUM_OF_SUM_INTERVAL_BUFFERS));

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(&g_bargraphFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));

	gSummaryCnt = 0;
	gOneSecondCnt = 0;			// Count the secs so that we can increment the sum capture rate.
	gOneMinuteCount = 0;		// Flag to mark the end of a summary sample in terms of time.
	gBarIntervalCnt = 0;		// Count the number of samples that make up a bar interval.
	gSummaryIntervalCnt = 0;	// Count the number of bars that make up a summary interval.
	gTotalBarIntervalCnt = 0;

	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_bargrapheventDataBufferer[0]), 0, (sizeof(uint16) * BG_DATA_BUFFER_SIZE));

	g_bargraphEventDataStartPtr = &(g_bargrapheventDataBufferer[0]);
	g_bargraphEventDataWritePtr = &(g_bargrapheventDataBufferer[0]);
	g_bargraphEventDataReadPtr = &(g_bargrapheventDataBufferer[0]);
	g_bargraphEventDataEndPtr = &(g_bargrapheventDataBufferer[BG_DATA_BUFFER_SIZE - 1]);

	g_bargraphBarIntervalWritePtr = &(g_bargraphBarInterval[0]);
	g_bargraphBarIntervalReadPtr = &(g_bargraphBarInterval[0]);
	g_bargraphSumIntervalWritePtr = &(g_bargraphSummaryInterval[0]);
	g_bargraphSumIntervalReadPtr = &(g_bargraphSummaryInterval[0]);

	// Clear out the Bar Interval buffer to be used next
	byteSet(g_bargraphBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(g_bargraphSumIntervalWritePtr, 0, SUMMARY_INTERVAL_SIZE_IN_BYTES);

#if 0 // fix_ns8100
	// The ramevent record was byteSet to 0xFF, to write in a full structure block.
	MoveStartOfBargraphEventRecordToFlash();
#endif

	// The ramevent record was byteSet to 0xFF, to write in a full structure block.
	// Save the captured event after the the preliminary ram event record.
	// EVENT_SUMMARY_STRUCT - Fill in the event CAPTURE_INFO_STRUCT data
	// For Bargraph, these values are filled in at the end of bargraph.
	g_RamEventRecord.summary.captured.batteryLevel =
		(uint32)((float)100.0 * (float)convertedBatteryLevel(BATTERY_VOLTAGE));
	g_RamEventRecord.summary.captured.printerStatus = (uint8)(help_rec.auto_print);
	g_RamEventRecord.summary.captured.calDate = factory_setup_rec.cal_date;

	// Get the time
	g_RamEventRecord.summary.captured.eventTime =
		g_RamEventRecord.summary.captured.endTime = getCurrentTime();

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

	while (BG_BUFFER_NOT_EMPTY == CalculateBargraphData()) {}

	// For the last time, put the data into the event buffer.
	eventDataFlag = putBarIntervalDataIntoEventDataBufferer();

	// If no data, both flags are zero, then skip, end of record.
	if ((eventDataFlag > 0) || (gSummaryIntervalCnt>0))
	{
		putSummaryIntervalDataIntoEventDataBufferer();
		MoveBargraphEventDataToFlash();
	}

	MoveEndOfBargraphEventRecordToFlash();
}

/*****************************************************************************
* Function:		ProcessBargraphData (Step 2)
* Purpose:		Copy A/D channel data from pretrigger buffer into event buffer
******************************************************************************/
void ProcessBargraphData(void)
{
	uint16 commandNibble = 0x0000;

	// Store the command nibble for the first channel of data
	commandNibble = (uint16)(*(tailOfPreTrigBuff) & EMBEDDED_CMD);

	// Check if the command is a trigger two, which is the 1st warning event
	if (commandNibble == TRIG_TWO)
	{
		raiseSystemEventFlag(WARNING1_EVENT);
	}
	// Check if the command is a trigger three, which is the 2nd warning event
	else if (commandNibble == TRIG_THREE)
	{
		raiseSystemEventFlag(WARNING2_EVENT);
	}

	// Check to see if we have a chunk of ram buffer to write, otherwise check for data wrapping.
	if ((gp_bg430DataEnd - gp_bg430DataWrite) >= 4)
	{
		// Move from the pre-trigger buffer to our large ram buffer.
		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;

		// Check for the end and if so go to the top
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

	}
	else
	{
		// Move from the pre-trigger buffer to our large ram buffer, but check for data wrapping.
		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

		*gp_bg430DataWrite++ = *tailOfPreTrigBuff++;
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;
	}

	// Handle preTriggerBuf pointer for circular buffer
	if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;

	// Alert system that we have data in ram buffer, raise flag to calculate and move data to flash.
	raiseSystemEventFlag(BARGRAPH_EVENT);
}

//*****************************************************************************
// Function: putBarIntervalDataIntoEventDataBufferer(void)
// Purpose : Transfer the data from the bar interval buffers into the event
//				write buffers for storage into flash.
//*****************************************************************************
uint32 putBarIntervalDataIntoEventDataBufferer(void)
{
	uint32 remainingIntervalCnt = gBarIntervalCnt;

	// If there has been no more data captures, it COULD happen.
	if (gBarIntervalCnt > 0)
	{
		// Reset the bar interval count
		gBarIntervalCnt = 0;

		// Save Bar Interval peak Air into event data buffer
		*(g_bargraphEventDataWritePtr++) = g_bargraphBarIntervalWritePtr->aMax;
		END_OF_EVENT_BUFFER_CHECK(g_bargraphEventDataWritePtr, g_bargraphEventDataStartPtr, g_bargraphEventDataEndPtr);

		// Save Bar Interval peak RVT into event data buffer
		*(g_bargraphEventDataWritePtr++) = g_bargraphBarIntervalWritePtr->rvtMax;
		END_OF_EVENT_BUFFER_CHECK(g_bargraphEventDataWritePtr, g_bargraphEventDataStartPtr, g_bargraphEventDataEndPtr);

		// Save Bar Interval peak Vector Sum into event data buffer. since vs is a long save two words.
		*(g_bargraphEventDataWritePtr++) = (uint16)((g_bargraphBarIntervalWritePtr->vsMax & 0xFFFF0000) >> 16);
		END_OF_EVENT_BUFFER_CHECK(g_bargraphEventDataWritePtr, g_bargraphEventDataStartPtr, g_bargraphEventDataEndPtr);
		*(g_bargraphEventDataWritePtr++) = (uint16)(g_bargraphBarIntervalWritePtr->vsMax & 0x0000FFFF);
		END_OF_EVENT_BUFFER_CHECK(g_bargraphEventDataWritePtr, g_bargraphEventDataStartPtr, g_bargraphEventDataEndPtr);

		// Advance the Bar Interval global buffer pointer
		advanceBarIntervalBufPtr(WRITE_PTR);

		// Clear out the Bar Interval buffer to be used next
		byteSet(g_bargraphBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));

		// Count the total number of intervals captured.
		g_bargraphSumIntervalWritePtr->barIntervalsCaptured ++;
	}

	// This is a flag for the end of bargraph function. It is used to indicate that
	// there IS bar and summary data to be processed.
	return (remainingIntervalCnt);
}

//*****************************************************************************
// Function: putSummaryIntervalDataIntoEventDataBufferer(void)
// Purpose : Transfer the data from the summary interval buffers into the event
//				write buffers for storage into flash.
//*****************************************************************************
void putSummaryIntervalDataIntoEventDataBufferer(void)
{
	uint32 sizeOfRemainingBuffer;
	uint8* tempSummaryIntervalWritePtr;
	float rFreq, vFreq, tFreq;

	rFreq = (float)((float)trig_rec.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1));
	vFreq = (float)((float)trig_rec.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1));
	tFreq = (float)((float)trig_rec.trec.sample_rate / (float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1));

	// Calculate the Peak Displacement
	g_bargraphSumIntervalWritePtr->a.displacement = 0;
	g_bargraphSumIntervalWritePtr->r.displacement = (uint32)(g_bargraphSumIntervalWritePtr->r.peak * 1000000 / 2 / PI / rFreq);
	g_bargraphSumIntervalWritePtr->v.displacement = (uint32)(g_bargraphSumIntervalWritePtr->v.peak * 1000000 / 2 / PI / vFreq);
	g_bargraphSumIntervalWritePtr->t.displacement = (uint32)(g_bargraphSumIntervalWritePtr->t.peak * 1000000 / 2 / PI / tFreq);

	// Store timestamp for the end of the summary interval
	gSummaryCnt++;
	g_bargraphSumIntervalWritePtr->summariesCaptured = gSummaryCnt;
	g_bargraphSumIntervalWritePtr->intervalEnd_Time = getCurrentTime();
	g_bargraphSumIntervalWritePtr->batteryLevel =
		(uint32)(100.0 * convertedBatteryLevel(BATTERY_VOLTAGE));
	g_bargraphSumIntervalWritePtr->calcStructEndFlag = 0xEECCCCEE;	// End structure flag

	// Reset summary interval count
	gSummaryIntervalCnt = 0;

	// Calculate the size of the remaining buffer in terms of bytes.
	sizeOfRemainingBuffer =
		(uint32)((uint8*)g_bargraphEventDataEndPtr - (uint8*)g_bargraphEventDataWritePtr);

	// If buffersize is greater then the strucutre size then copy and increment write ptr.
	// If less then, then copy only the bytes that fit, then start at the top.
	if (sizeOfRemainingBuffer > SUMMARY_INTERVAL_SIZE_IN_BYTES)
	{
		byteCpy((uint8*)g_bargraphEventDataWritePtr,
			(uint8*)g_bargraphSumIntervalWritePtr, sizeof(CALCULATED_DATA_STRUCT));
		g_bargraphEventDataWritePtr += SUMMARY_INTERVAL_SIZE_IN_WORDS;
	}

	else
	{
		// Copy only the size of data that fits.
		byteCpy((uint8*)g_bargraphEventDataWritePtr,
			(uint8*)g_bargraphSumIntervalWritePtr, sizeOfRemainingBuffer);

		// Go to the start of the buffer.
		g_bargraphEventDataWritePtr = g_bargraphEventDataStartPtr;

		// ptr to the start of the uncopied data.
		tempSummaryIntervalWritePtr = (uint8*)g_bargraphSumIntervalWritePtr + sizeOfRemainingBuffer;

		// Number of bytes remaining to move.
		sizeOfRemainingBuffer = SUMMARY_INTERVAL_SIZE_IN_BYTES - sizeOfRemainingBuffer;
		byteCpy((uint8*)g_bargraphEventDataWritePtr,
			(uint8*)tempSummaryIntervalWritePtr, sizeOfRemainingBuffer);

		// Move write ptr by the number of words.
		g_bargraphEventDataWritePtr += ((sizeOfRemainingBuffer+1)/2);
	}

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
	static uint16 aTemp, rTemp, vTemp, tTemp;
	static uint32 vsTemp;
	static uint16 aTempNorm, rTempNorm, vTempNorm, tTempNorm;
	static DATE_TIME_STRUCT aTempTime, rTempTime, vTempTime, tTempTime;
	static uint8 aJobFreqFlag = NO, rJobFreqFlag = NO, vJobFreqFlag = NO, tJobFreqFlag = NO;
	static uint8 aJobPeakMatch = NO, rJobPeakMatch = NO, vJobPeakMatch = NO, tJobPeakMatch = NO;

	uint8 gotDataFlag = NO;
	//int32 delta = 0;
	int32 processingCount = 500;
	uint16* dataReadStart;
	INPUT_MSG_STRUCT msg;

	while ((gp_bg430DataRead != gp_bg430DataWrite) && (processingCount-- > 0))
	{

		if (processingCount == 1)
		{
			if (gp_bg430DataWrite < gp_bg430DataRead)
			{
				if (((gp_bg430DataWrite - gp_bg430DataStart) +
					 (gp_bg430DataEnd - gp_bg430DataRead)) > 500)
					processingCount = 500;
			}
			else
			{
				if ((gp_bg430DataWrite - gp_bg430DataRead) > 500)
					processingCount = 500;
			}
		}

		dataReadStart = gp_bg430DataRead;

		// Make sure that we will not catch up to writing the data, almost impossible.
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debugErr("1a - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		// Move from the pre-trigger buffer to our large ram buffer, but check for data wrapping.
		aTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;

		// We have caught up to the end of the write with out it being completed.
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debugErr("1b - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		rTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debugErr("1c - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		vTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debugErr("1d - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		tTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;

		// If here we got data;
		gotDataFlag = YES;

		// Normalize the raw data without the message bits
		aTempNorm = FixDataToZero(aTemp);
		rTempNorm = FixDataToZero(rTemp);
		vTempNorm = FixDataToZero(vTemp);
		tTempNorm = FixDataToZero(tTemp);

		// Find the vector sum of the current sample
		vsTemp =((uint32)rTempNorm * (uint32)rTempNorm) +
				((uint32)vTempNorm * (uint32)vTempNorm) +
				((uint32)tTempNorm * (uint32)tTempNorm);

		//=================================================
		// Impulse Interval
		//=================================================
		if (impulseMenuCount >= trig_rec.berec.impulseMenuUpdateSecs)
		{
			impulseMenuCount = 0;
			aImpulsePeak = rImpulsePeak = vImpulsePeak = tImpulsePeak = 0;
			vsImpulsePeak = 0;
		}

		// ---------------------------------
		// A, R, V, T channel and Vector Sum
		// ---------------------------------
		// Store the max A, R, V or T normalized value if a new max was found
		if (aTempNorm > aImpulsePeak) aImpulsePeak = aTempNorm;
		if (rTempNorm > rImpulsePeak) rImpulsePeak = rTempNorm;
		if (vTempNorm > vImpulsePeak) vImpulsePeak = vTempNorm;
		if (tTempNorm > tImpulsePeak) tImpulsePeak = tTempNorm;
		if (vsTemp > vsImpulsePeak) vsImpulsePeak = vsTemp;

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

			if (aTempNorm >= aJobPeak)
			{
				if (aTempNorm == aJobPeak)
					aJobPeakMatch = YES;
				else
				{
					aJobPeak = aTempNorm;
					aJobPeakMatch = NO;
				}

				aJobFreqFlag = YES;
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

			if (rTempNorm >= rJobPeak)
			{
				if (rTempNorm == rJobPeak)
					rJobPeakMatch = YES;
				else
				{
					rJobPeak = rTempNorm;
					rJobPeakMatch = NO;
				}

				rJobFreqFlag = YES;
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

			if (vTempNorm >= vJobPeak)
			{
				if (vTempNorm == vJobPeak)
					vJobPeakMatch = YES;
				else
				{
					vJobPeak = vTempNorm;
					vJobPeakMatch = NO;
				}

				vJobFreqFlag = YES;
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

			if (tTempNorm >= tJobPeak)
			{
				if (tTempNorm == tJobPeak)
					tJobPeakMatch = YES;
				else
				{
					tJobPeak = tTempNorm;
					tJobPeakMatch = NO;
				}

				tJobFreqFlag = YES;
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

			if (vsTemp > vsJobPeak)
			{
				vsJobPeak = vsTemp;
			}
		}

		//=================================================
		// Freq Calc Information
		//=================================================
		// ---------
		// A channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_bargraphFreqCalcBuffer.a.sign ^ (aTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.a.sign = (uint16)(aTemp & SIGNBIT_MASK);

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

			if (aJobFreqFlag == YES)
			{
				if (aJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.a.freq_count > aJobFreq)
						aJobFreq = g_bargraphFreqCalcBuffer.a.freq_count;

					aJobPeakMatch = NO;
				}
				else
				{
					aJobFreq = g_bargraphFreqCalcBuffer.a.freq_count;
				}

				aJobFreqFlag = NO;
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
		if (g_bargraphFreqCalcBuffer.r.sign ^ (rTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.r.sign = (uint16)(rTemp & SIGNBIT_MASK);

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

			if (rJobFreqFlag == YES)
			{
				if (rJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.r.freq_count > rJobFreq)
						rJobFreq = g_bargraphFreqCalcBuffer.r.freq_count;

					rJobPeakMatch = NO;
				}
				else
				{
					rJobFreq = g_bargraphFreqCalcBuffer.r.freq_count;
				}

				rJobFreqFlag = NO;
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
		if (g_bargraphFreqCalcBuffer.v.sign ^ (vTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.v.sign = (uint16)(vTemp & SIGNBIT_MASK);

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

			if (vJobFreqFlag == YES)
			{
				if (vJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.v.freq_count > vJobFreq)
						vJobFreq = g_bargraphFreqCalcBuffer.v.freq_count;

					vJobPeakMatch = NO;
				}
				else
				{
					vJobFreq = g_bargraphFreqCalcBuffer.v.freq_count;
				}

				vJobFreqFlag = NO;
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
		if (g_bargraphFreqCalcBuffer.t.sign ^ (tTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_bargraphFreqCalcBuffer.t.sign = (uint16)(tTemp & SIGNBIT_MASK);

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

			if (tJobFreqFlag == YES)
			{
				if (tJobPeakMatch == YES)
				{
					if (g_bargraphFreqCalcBuffer.t.freq_count > tJobFreq)
						tJobFreq = g_bargraphFreqCalcBuffer.t.freq_count;

					tJobPeakMatch = NO;
				}
				else
				{
					tJobFreq = g_bargraphFreqCalcBuffer.t.freq_count;
				}

				tJobFreqFlag = NO;
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
		if (++gBarIntervalCnt >= (uint32)(trig_rec.bgrec.barInterval * trig_rec.trec.sample_rate))
		{
			putBarIntervalDataIntoEventDataBufferer();

			//=================================================
			// End of Summary Interval
			//=================================================
			if (++gSummaryIntervalCnt >=
				(uint32)(trig_rec.bgrec.summaryInterval / trig_rec.bgrec.barInterval))
			{
				putSummaryIntervalDataIntoEventDataBufferer();
			}

			// Move data to flash.
			MoveBargraphEventDataToFlash();

			if (help_rec.flash_wrapping == NO)
			{
				if (checkSpaceForBarSummaryInterval() == NO)
				{
					msg.cmd = STOP_MONITORING_CMD;
					msg.length = 1;
					(*menufunc_ptrs[MONITOR_MENU])(msg);
				}
			}
		}

	} // While != loop

	if (gp_bg430DataWrite != gp_bg430DataRead)
	{
		return (BG_BUFFER_NOT_EMPTY);
	}
	else
	{
		return (BG_BUFFER_EMPTY);
	}
}

//*****************************************************************************
// Function: MoveBargraphEventDataToFlash(void)
// Purpose : Move the data in the bargarpheventDataBufferer into flash. Copy
//				all the data from the buffer until the readPtr == writePtr.
//*****************************************************************************
void MoveBargraphEventDataToFlash(void)
{
	// While we haven't found the end of the Bargraph Event data
	while (g_bargraphEventDataReadPtr != g_bargraphEventDataWritePtr)
	{
		// Write in data (alternates between Air and RVT max normalized values) for a bar interval
		storeData(g_bargraphEventDataReadPtr, 1);
		g_bargraphEventDataReadPtr += 1;

		// Check to see if the read ptr has gone past the end, if it has start at the top again.
		END_OF_EVENT_BUFFER_CHECK(g_bargraphEventDataReadPtr, g_bargraphEventDataStartPtr, g_bargraphEventDataEndPtr);
	}
}

//*****************************************************************************
// Function: MoveStartofBargraphEventRecordToFlash
// Purpose : Move the Bargraph Event Record into flash. Copy most of the
//				event record data. Unable to copy the captured data structures
//				until they are filled out. so only copy the header and system
//				data structs.
//*****************************************************************************
void MoveStartOfBargraphEventRecordToFlash(void)
{
	uint32 moveSizeInWords;
	uint16* destPtr;
	uint16* srcPtr;
	EVT_RECORD* flashEventRecord;

	// Check if we have a valid flash ptr.
	if (gBargraphSummaryPtr == 0)
	{	// If failed the ptr will be 0.
		debug("Out of Flash Summary Entrys\n");
	}
	else
	{
		flashEventRecord = (EVT_RECORD *)(gBargraphSummaryPtr->linkPtr);

		moveSizeInWords = (sizeof(EVT_RECORD))/2;
		destPtr = (uint16*)(flashEventRecord);
		srcPtr = (uint16*)(&g_RamEventRecord);
		flashWrite(destPtr, srcPtr, moveSizeInWords);

		byteSet((uint8*)&(g_RamEventRecord.summary.calculated), 0, sizeof(CALCULATED_DATA_STRUCT));
	}

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
	// This global will contain the ram record with the reference to the start of the event record ptr.
	// SUMMARY_DATA* gBargraphSummaryPtr;
	// EVENT_REC* flashEventRecord = (EVENT_REC *)gBargraphSummaryPtr->linkPtr;

	//uint16 checksumDex = 0;
	uint32 moveSizeInWords;

	EVT_RECORD* flashEventRecord = (EVT_RECORD*)gBargraphSummaryPtr->linkPtr;
	uint16* eventDataPtr;
	uint16* destPtr;
	uint16* srcPtr;
	uint16* flashPtr = getFlashDataPointer();

	// Check if we have a valid flash ptr.
	if (gBargraphSummaryPtr == 0)
	{	// If failed the ptr will be 0.
		debug("Out of Flash Summary Entrys\n");
	}
	else
	{
		// Set our flash event data pointer to the start of the flash event
	    eventDataPtr = (uint16*)((uint8*)flashEventRecord + sizeof(EVT_RECORD));

		// Check if the flash pointer is greater than the event data pointer suggesting that the event has not wrapped
		if (flashPtr > eventDataPtr)
		{
			g_RamEventRecord.header.dataLength = (uint32)((uint32)flashPtr - (uint32)eventDataPtr);
		}
		else
		{
			// The size of the data to the end of the buffer.
			g_RamEventRecord.header.dataLength = (uint32)((FLASH_EVENT_END - (uint32)eventDataPtr) +
				((uint32)flashPtr - FLASH_EVENT_START));
		}

/*
		// Is the checksum done in words or bytes, do we do a crc?
		for (checksumDex = 0; checksumDex < g_RamEventRecord.header.dataLength; checksumDex++)
		{
			g_RamEventRecord.header.dataChecksum = 0;
		}

		// Is the checksum done in words or bytes, do we do a crc?
		for (checksumDex = 0; checksumDex < sizeof(EVENT_SUMMARY_STRUCT); checksumDex++)
		{
			g_RamEventRecord.header.summaryChecksum = 0;
		}
*/
		// The following data will be filled in when the data has been moved over to flash.
		g_RamEventRecord.header.summaryChecksum = 0xAABB;
		g_RamEventRecord.header.dataChecksum = 0xCCDD;
		g_RamEventRecord.header.dataCompression = (uint16)NULL;

		// We dont worry about the end of the falsh buffer because when the new flash event record is setup,
		// a check is made so the event record is one contiguous structure, not including the event data.
		// Not sure if flashwrite will mod the ptrs, so setup dest and src ptrs. We are copying over the
		// dataLength(32), summaryChecksum, dataChecksum, and dataCompression, unused, unused;
		moveSizeInWords = 7;
		destPtr = (uint16*)(&(flashEventRecord->header.dataLength));
		srcPtr = (uint16*)(&(g_RamEventRecord.header.dataLength));
		flashWrite(destPtr, srcPtr, moveSizeInWords);

		g_RamEventRecord.summary.captured.endTime = getCurrentTime();

		// Setup ptrs so we are at the beginning of the CAPTURE_INFO_STRUCT. Copy over only the
		// CAPTURE_INFO_STRUCT and CALCULATED_DATA_STRUCT. Get the size of the data to copy.
		moveSizeInWords = sizeof(CAPTURE_INFO_STRUCT)/2;
		destPtr = (uint16*)(&(flashEventRecord->summary.captured));
		srcPtr = (uint16*)(&(g_RamEventRecord.summary.captured));
		flashWrite(destPtr, srcPtr, moveSizeInWords);

		// Setup ptrs so we are at the beginning of the CAPTURE_INFO_STRUCT. Copy over only the
		// CAPTURE_INFO_STRUCT and CALCULATED_DATA_STRUCT. Get the size of the data to copy.
		moveSizeInWords = sizeof(CALCULATED_DATA_STRUCT)/2;
		destPtr = (uint16*)(&(flashEventRecord->summary.calculated));
		srcPtr = (uint16*)(&(g_RamEventRecord.summary.calculated));
		flashWrite(destPtr, srcPtr, moveSizeInWords);
	}

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
	if (sumIntervalPtr->a.peak > g_RamEventRecord.summary.calculated.a.peak)
	{
		g_RamEventRecord.summary.calculated.a = sumIntervalPtr->a;
		g_RamEventRecord.summary.calculated.a_Time = sumIntervalPtr->a_Time;
	}
	else if (sumIntervalPtr->a.peak == g_RamEventRecord.summary.calculated.a.peak)
	{
		if (sumIntervalPtr->a.frequency > g_RamEventRecord.summary.calculated.a.frequency)
		{
			g_RamEventRecord.summary.calculated.a = sumIntervalPtr->a;
			g_RamEventRecord.summary.calculated.a_Time = sumIntervalPtr->a_Time;
		}
	}

	// ---------
	// R channel
	// ---------
	if (sumIntervalPtr->r.peak > g_RamEventRecord.summary.calculated.r.peak)
	{
		g_RamEventRecord.summary.calculated.r = sumIntervalPtr->r;
		g_RamEventRecord.summary.calculated.r_Time = sumIntervalPtr->r_Time;
	}
	else if (sumIntervalPtr->r.peak == g_RamEventRecord.summary.calculated.r.peak)
	{
		if (sumIntervalPtr->r.frequency > g_RamEventRecord.summary.calculated.r.frequency)
		{
			g_RamEventRecord.summary.calculated.r = sumIntervalPtr->r;
			g_RamEventRecord.summary.calculated.r_Time = sumIntervalPtr->r_Time;
		}
	}

	// ---------
	// V channel
	// ---------
	if (sumIntervalPtr->v.peak > g_RamEventRecord.summary.calculated.v.peak)
	{
		g_RamEventRecord.summary.calculated.v = sumIntervalPtr->v;
		g_RamEventRecord.summary.calculated.v_Time = sumIntervalPtr->v_Time;
	}
	else if (sumIntervalPtr->v.peak == g_RamEventRecord.summary.calculated.v.peak)
	{
		if (sumIntervalPtr->v.frequency > g_RamEventRecord.summary.calculated.v.frequency)
		{
			g_RamEventRecord.summary.calculated.v = sumIntervalPtr->v;
			g_RamEventRecord.summary.calculated.v_Time = sumIntervalPtr->v_Time;
		}
	}

	// ---------
	// T channel
	// ---------
	if (sumIntervalPtr->t.peak > g_RamEventRecord.summary.calculated.t.peak)
	{
		g_RamEventRecord.summary.calculated.t = sumIntervalPtr->t;
		g_RamEventRecord.summary.calculated.t_Time = sumIntervalPtr->t_Time;
	}
	else if (sumIntervalPtr->t.peak == g_RamEventRecord.summary.calculated.t.peak)
	{
		if (sumIntervalPtr->t.frequency > g_RamEventRecord.summary.calculated.t.frequency)
		{
			g_RamEventRecord.summary.calculated.t = sumIntervalPtr->t;
			g_RamEventRecord.summary.calculated.t_Time = sumIntervalPtr->t_Time;
		}
	}

	// ----------
	// Vector Sum
	// ----------
	if (sumIntervalPtr->vectorSumPeak > g_RamEventRecord.summary.calculated.vectorSumPeak)
	{
		g_RamEventRecord.summary.calculated.vectorSumPeak = sumIntervalPtr->vectorSumPeak;
		g_RamEventRecord.summary.calculated.vs_Time = sumIntervalPtr->vs_Time;
	}

	g_RamEventRecord.summary.calculated.summariesCaptured = sumIntervalPtr->summariesCaptured;
	g_RamEventRecord.summary.calculated.barIntervalsCaptured += sumIntervalPtr->barIntervalsCaptured;
	g_RamEventRecord.summary.calculated.intervalEnd_Time = sumIntervalPtr->intervalEnd_Time;
	g_RamEventRecord.summary.calculated.batteryLevel = sumIntervalPtr->batteryLevel;
	g_RamEventRecord.summary.calculated.calcStructEndFlag = 0xEECCCCEE;
}

//*****************************************************************************
// Function:	checkSpaceForBarSummaryInterval
// Purpose:
//*****************************************************************************
BOOLEAN checkSpaceForBarSummaryInterval(void)
{
	FLASH_USAGE_STRUCT flashStats = getFlashUsageStats();
	uint32 barIntervalSize;
	BOOLEAN spaceLeft;

	barIntervalSize = (sizeof(CALCULATED_DATA_STRUCT) + (((trig_rec.bgrec.summaryInterval / trig_rec.bgrec.barInterval) + 1) * 8));

	if (flashStats.sizeFree > barIntervalSize)
		spaceLeft = YES;
	else
		spaceLeft = NO;

	return (spaceLeft);
}

