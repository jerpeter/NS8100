//----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: ProcessCombo.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:56 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ProcessCombo.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
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
#include "ProcessCombo.h"
#include "Flash.h"
#include "EventProcessing.h"
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
// Function:	StartNewCombo
// Purpose :
//*****************************************************************************
void StartNewCombo(void)
{
	g_comboSummaryPtr = NULL;

	// Get the address and empty Ram summary
	if (GetRamSummaryEntry(&g_comboSummaryPtr) == FALSE)
	{
		debug("Out of Ram Summary Entrys\n");
		return;
	}

	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_comboBarInterval[0]), 0, (sizeof(BARGRAPH_BAR_INTERVAL_DATA) * NUM_OF_BAR_INTERVAL_BUFFERS));
	byteSet(&(g_comboSummaryInterval[0]), 0, (SUMMARY_INTERVAL_SIZE_IN_BYTES * NUM_OF_SUM_INTERVAL_BUFFERS));

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(&g_comboFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));

	g_summaryCount = 0;
	g_oneSecondCnt = 0;			// Count the secs so that we can increment the sum capture rate.
	g_oneMinuteCount = 0;		// Flag to mark the end of a summary sample in terms of time.
	g_barIntervalCnt = 0;		// Count the number of samples that make up a bar interval.
	g_summaryIntervalCnt = 0;	// Count the number of bars that make up a summary interval.
	g_totalBarIntervalCnt = 0;

#if 0 // Unused
	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_combog_eventDataBufferer[0]), 0, (sizeof(uint16) * BG_DATA_BUFFER_SIZE));

	g_comboEventDataStartPtr = &(g_combog_eventDataBufferer[0]);
	g_comboEventDataWritePtr = &(g_combog_eventDataBufferer[0]);
	g_comboEventDataReadPtr = &(g_combog_eventDataBufferer[0]);
	g_comboEventDataEndPtr = &(g_combog_eventDataBufferer[BG_DATA_BUFFER_SIZE - 1]);
#endif

	g_comboBarIntervalWritePtr = &(g_comboBarInterval[0]);
	g_comboBarIntervalReadPtr = &(g_comboBarInterval[0]);
	g_comboSumIntervalWritePtr = &(g_comboSummaryInterval[0]);
	g_comboSumIntervalReadPtr = &(g_comboSummaryInterval[0]);

	// Clear out the Bar Interval buffer to be used next
	byteSet(g_comboBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(g_comboSumIntervalWritePtr, 0, SUMMARY_INTERVAL_SIZE_IN_BYTES);

	MoveStartOfComboEventRecordToFile();

	g_RamEventRecord.summary.captured.batteryLevel = (uint32)((float)100.0 * (float)convertedBatteryLevel(BATTERY_VOLTAGE));
	g_RamEventRecord.summary.captured.printerStatus = (uint8)(g_helpRecord.auto_print);
	g_RamEventRecord.summary.captured.calDate = g_factorySetupRecord.cal_date;

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
void EndCombo(void)
{
	uint32 barIntervalsStored;

	while (BG_BUFFER_NOT_EMPTY == CalculateComboData()) {}

	// For the last time, put the data into the event buffer.
	barIntervalsStored = moveComboBarIntervalDataToFile();

	// Check if bar intervals were stored or a summery interval is present
	if ((barIntervalsStored) || (g_summaryIntervalCnt))
	{
		moveComboSummaryIntervalDataToFile();
	}

	MoveEndOfComboEventRecordToFile();
}

/*****************************************************************************
* Function:		ProcessComboData (Step 2)
* Purpose:		Copy A/D channel data from pretrigger buffer into event buffer
******************************************************************************/
void ProcessComboData(void)
{
	// In theory, the following would be processed
	// however they both advance the pretrigger pointer
	ProcessComboSampleData();
	ProcessComboBargraphData();

	// Handle preTriggerBuf pointer for circular buffer after processing both individual modes,
	//	neither of which advanced the pointer
	if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;
}

/*****************************************************************************
* Function:		ProcessComboBargraphData (Step 2)
* Purpose:		Copy A/D channel data from pretrigger buffer into event buffer
******************************************************************************/
void ProcessComboBargraphData(void)
{
	// Check to see if we have a chunk of ram buffer to write, otherwise check for data wrapping.
	if ((g_bg430DataEndPtr - g_bg430DataWritePtr) >= 4)
	{
		// Move from the pre-trigger buffer to our large ram buffer.
		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 0);
		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 1);
		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 2);
		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 3);

		// Check for the end and if so go to the top
		if (g_bg430DataWritePtr > g_bg430DataEndPtr) g_bg430DataWritePtr = g_bg430DataStartPtr;

	}
	else
	{
		// Move from the pre-trigger buffer to our large ram buffer, but check for data wrapping.
		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 0);
		if (g_bg430DataWritePtr > g_bg430DataEndPtr) g_bg430DataWritePtr = g_bg430DataStartPtr;

		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 1);
		if (g_bg430DataWritePtr > g_bg430DataEndPtr) g_bg430DataWritePtr = g_bg430DataStartPtr;

		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 2);
		if (g_bg430DataWritePtr > g_bg430DataEndPtr) g_bg430DataWritePtr = g_bg430DataStartPtr;

		*g_bg430DataWritePtr++ = *(g_tailOfPreTrigBuff + 3);
		if (g_bg430DataWritePtr > g_bg430DataEndPtr) g_bg430DataWritePtr = g_bg430DataStartPtr;
	}

	// Alert system that we have data in ram buffer, raise flag to calculate and move data to flash.
	raiseSystemEventFlag(BARGRAPH_EVENT);
}

//*****************************************************************************
// Function:	ProcessComboSampleData (Step 2)
// Purpose :	Copy A/D channel data from pretrigger into event buffers and
//				check for command nibble
//*****************************************************************************
void ProcessComboSampleData(void)
{
	//SUMMARY_DATA* sumEntry;
	uint16 commandNibble;

	// Store the command nibble for the first channel of data
	commandNibble = (uint16)(*(g_tailOfPreTrigBuff) & EMBEDDED_CMD);

	// Check if still waiting for an event
	if (g_isTriggered == NO)
	{
		// Check if not processing a cal pulse
		if (g_processingCal == NO)
		{
			// Check the Command Nibble on the first channel (Acoustic)
			switch (commandNibble)
			{
				case TRIG_ONE:
					// Check if there are still free event containers and we are still taking new events
					if ((g_freeEventBuffers != 0) && (g_doneTakingEvents == NO))
					{
						// Store the exact time we received the trigger data sample
						g_RamEventRecord.summary.captured.eventTime = getCurrentTime();

						// Set loop counter to 1 minus the total samples to be recieved in the event body (minus the trigger data sample)
						g_isTriggered = g_samplesInBody - 1;

#if 0 // ns7100
						// Save the link to the beginning of the pretrigger data
						g_summaryTable[g_currentEventNumber].linkPtr = g_eventBufferPretrigPtr;
#endif
						// Copy PreTrigger data over to the Event body buffer
						*(g_eventBufferBodyPtr + 0) = *(g_tailOfPreTrigBuff + 0);
						*(g_eventBufferBodyPtr + 1) = *(g_tailOfPreTrigBuff + 1);
						*(g_eventBufferBodyPtr + 2) = *(g_tailOfPreTrigBuff + 2);
						*(g_eventBufferBodyPtr + 3) = *(g_tailOfPreTrigBuff + 3);

						// Advance the pointers
						g_eventBufferBodyPtr += 4;
						// Now handled outside of routine
						//g_tailOfPreTrigBuff += 4;

						// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
						if (g_sensorInfoPtr->numOfChannels <= 4)
						{
							// Check if the end of the PreTrigger buffer has been reached
							if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;

							// Copy PreTrigger data over to the Event PreTrigger buffer
							*(g_eventBufferPretrigPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_START);
							*(g_eventBufferPretrigPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_START);
							*(g_eventBufferPretrigPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_START);
							*(g_eventBufferPretrigPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_START);

							// Advance the pointer (Don't advance g_tailOfPreTrigBuff since just reading pretrigger data)
							g_eventBufferPretrigPtr += 4;
						}
					}
					else
					{
						// Full or finished
					}
					break;

				case CAL_START:
					// Set loop counter to 1 minus the total cal samples to be recieved (minus the cal start sample)
					g_processingCal = g_samplesInCal - 1;

					switch (g_calTestExpected)
					{
						// Received a Cal pulse after the event
						case 1:
							// Copy PreTrigger data over to the Event Cal buffer
							*(g_eventBufferCalPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | CAL_START);
							*(g_eventBufferCalPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | CAL_START);
							*(g_eventBufferCalPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | CAL_START);
							*(g_eventBufferCalPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							g_eventBufferCalPtr += 4;
							// Now handled outside of routine
							//g_tailOfPreTrigBuff += 4;

							break;

						// Received a Cal pulse which was delayed once (use/copy the cal for both events)
						case 2:
							// Set the pointer to the second event Cal buffer
							g_delayedOneEventBufferCalPtr = g_eventBufferCalPtr + g_wordSizeInEvent;

							// Copy PreTrigger data over to the Event Cal buffers
							*(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | CAL_START);
							*(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | CAL_START);
							*(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | CAL_START);
							*(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							g_delayedOneEventBufferCalPtr += 4;
							g_eventBufferCalPtr += 4;
							// Now handled outside of routine
							//g_tailOfPreTrigBuff += 4;
							break;

						// Received a Cal pulse which was delayed twice (use/copy the cal for all three events)
						case 3:
							// Set the pointer to the second event Cal buffer
							g_delayedOneEventBufferCalPtr = g_eventBufferCalPtr + g_wordSizeInEvent;

							// Set the pointer to the third event Cal buffer
							g_delayedTwoEventBufferCalPtr = g_delayedOneEventBufferCalPtr + g_wordSizeInEvent;

							// Copy PreTrigger data over to the Event Cal buffers
							*(g_delayedTwoEventBufferCalPtr + 0) = *(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | CAL_START);
							*(g_delayedTwoEventBufferCalPtr + 1) = *(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | CAL_START);
							*(g_delayedTwoEventBufferCalPtr + 2) = *(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | CAL_START);
							*(g_delayedTwoEventBufferCalPtr + 3) = *(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							g_delayedTwoEventBufferCalPtr += 4;
							g_delayedOneEventBufferCalPtr += 4;
							g_eventBufferCalPtr += 4;
							// Now handled outside of routine
							//g_tailOfPreTrigBuff += 4;
							break;
					}
					break;

				default:
					// Advance the PreTrigger buffer the number of active channels
					// Now handled outside of this routine
					//g_tailOfPreTrigBuff += g_sensorInfoPtr->numOfChannels;
					break;
			}
		}
		else // g_processingCal != 0
		{
			// Received another data sample, decrement the count
			g_processingCal--;

			switch (g_calTestExpected)
			{
				case 1:
					// Copy PreTrigger data over to the Event Cal buffer
					*(g_eventBufferCalPtr + 0) = *(g_tailOfPreTrigBuff + 0);
					*(g_eventBufferCalPtr + 1) = *(g_tailOfPreTrigBuff + 1);
					*(g_eventBufferCalPtr + 2) = *(g_tailOfPreTrigBuff + 2);
					*(g_eventBufferCalPtr + 3) = *(g_tailOfPreTrigBuff + 3);

					// Advance the pointers
					g_eventBufferCalPtr += 4;
					// Now handled outside of routine
					//g_tailOfPreTrigBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(g_eventBufferCalPtr - 4) |= CAL_END;
						*(g_eventBufferCalPtr - 3) |= CAL_END;
						*(g_eventBufferCalPtr - 2) |= CAL_END;
						*(g_eventBufferCalPtr - 1) |= CAL_END;

						g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

						//debugRaw("TE\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						g_calTestExpected = 0;
					}
					break;

				case 2:
					*(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = *(g_tailOfPreTrigBuff + 0);
					*(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = *(g_tailOfPreTrigBuff + 1);
					*(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = *(g_tailOfPreTrigBuff + 2);
					*(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = *(g_tailOfPreTrigBuff + 3);

					// Advance the pointers
					g_delayedOneEventBufferCalPtr += 4;
					g_eventBufferCalPtr += 4;
					// Now handled outside of routine
					//g_tailOfPreTrigBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(g_eventBufferCalPtr - 4) |= CAL_END;
						*(g_eventBufferCalPtr - 3) |= CAL_END;
						*(g_eventBufferCalPtr - 2) |= CAL_END;
						*(g_eventBufferCalPtr - 1) |= CAL_END;

						g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

						//debugRaw("TE\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						g_calTestExpected = 0;
					}
					break;

				case 3:
					*(g_delayedTwoEventBufferCalPtr + 0) = *(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = *(g_tailOfPreTrigBuff + 0);
					*(g_delayedTwoEventBufferCalPtr + 1) = *(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = *(g_tailOfPreTrigBuff + 1);
					*(g_delayedTwoEventBufferCalPtr + 2) = *(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = *(g_tailOfPreTrigBuff + 2);
					*(g_delayedTwoEventBufferCalPtr + 3) = *(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = *(g_tailOfPreTrigBuff + 3);

					// Advance the pointers
					g_delayedTwoEventBufferCalPtr += 4;
					g_delayedOneEventBufferCalPtr += 4;
					g_eventBufferCalPtr += 4;
					// Now handled outside of routine
					//g_tailOfPreTrigBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(g_eventBufferCalPtr - 4) |= CAL_END;
						*(g_eventBufferCalPtr - 3) |= CAL_END;
						*(g_eventBufferCalPtr - 2) |= CAL_END;
						*(g_eventBufferCalPtr - 1) |= CAL_END;

						g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

						//debugRaw("TE\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						g_calTestExpected = 0;
					}
					break;
			}
		}
	}
	else // g_isTriggered != 0
	{
		// Check if this is the last sample of the triggered event
		if ((g_isTriggered - 1) == 0)
		{
			// Copy the channel data over from the pretrigger to the buffer
			// Strip off the command nibble and mark the end of the event
			*(g_eventBufferBodyPtr + 0) = (uint16)((*(g_tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_END);
			*(g_eventBufferBodyPtr + 1) = (uint16)((*(g_tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_END);
			*(g_eventBufferBodyPtr + 2) = (uint16)((*(g_tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_END);
			*(g_eventBufferBodyPtr + 3) = (uint16)((*(g_tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_END);

			// Advance the pointers
			g_eventBufferBodyPtr += 4;
			// Now handled outside of routine
			//g_tailOfPreTrigBuff += 4;
		}
		else // Normal samples in the triggered event
		{
			// Copy the channel data over from the pretrigger to the buffer
			*(g_eventBufferBodyPtr + 0) = *(g_tailOfPreTrigBuff + 0);
			*(g_eventBufferBodyPtr + 1) = *(g_tailOfPreTrigBuff + 1);
			*(g_eventBufferBodyPtr + 2) = *(g_tailOfPreTrigBuff + 2);
			*(g_eventBufferBodyPtr + 3) = *(g_tailOfPreTrigBuff + 3);

			// Advance the pointers
			g_eventBufferBodyPtr += 4;
			// Now handled outside of routine
			//g_tailOfPreTrigBuff += 4;
		}

		// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
		if (g_sensorInfoPtr->numOfChannels <= 4)
		{
			// Check if there are still PreTrigger data samples in the PreTrigger buffer
			if ((g_samplesInBody - g_isTriggered) < g_samplesInPretrig)
			{
				// Check if the end of the PreTrigger buffer has been reached
				if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;

				// Copy the PreTrigger data samples over to the PreTrigger data buffer
				*(g_eventBufferPretrigPtr + 0) = *(g_tailOfPreTrigBuff + 0);
				*(g_eventBufferPretrigPtr + 1) = *(g_tailOfPreTrigBuff + 1);
				*(g_eventBufferPretrigPtr + 2) = *(g_tailOfPreTrigBuff + 2);
				*(g_eventBufferPretrigPtr + 3) = *(g_tailOfPreTrigBuff + 3);

				// Advance the pointer (Don't advance g_tailOfPreTrigBuff since just reading pretrigger data)
				g_eventBufferPretrigPtr += 4;
			}
		}

		// Deincrement g_isTriggered since another event sample has been stored
		g_isTriggered--;

		// Check if all the event data from the PreTrigger buffer has been moved into the event buffer
		if (g_isTriggered == 0)
		{
			g_freeEventBuffers--;
			g_currentEventNumber++;
			g_calTestExpected++;

			if (g_currentEventNumber < g_maxEventBuffers)
			{
				g_eventBufferPretrigPtr = g_eventBufferBodyPtr + g_wordSizeInCal;
				g_eventBufferBodyPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig;
			}
			else
			{
				g_currentEventNumber = 0;
				g_eventBufferPretrigPtr = g_startOfEventBufferPtr;
				g_eventBufferBodyPtr = g_startOfEventBufferPtr + g_wordSizeInPretrig;
			}
		}
	}

	// Check if the end of the PreTrigger buffer has been reached
	if (g_tailOfPreTrigBuff >= g_endOfPreTrigBuff) g_tailOfPreTrigBuff = g_startOfPreTrigBuff;
}

//*****************************************************************************
// Function: moveComboBarIntervalDataToFile(void)
// Purpose : Transfer the data from the bar interval buffers into the event file
//*****************************************************************************
uint32 moveComboBarIntervalDataToFile(void)
{
	uint32 accumulatedBarIntervalCount = g_barIntervalCnt;

	// If Bar Intervals have been cached
	if (g_barIntervalCnt > 0)
	{
		// Reset the bar interval count
		g_barIntervalCnt = 0;

		fl_fwrite(g_comboBarIntervalWritePtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA), 1, g_comboDualCurrentEventFileHandle);		

		// Advance the Bar Interval global buffer pointer
		advanceBarIntervalBufPtr(WRITE_PTR);

		// Clear out the Bar Interval buffer to be used next
		byteSet(g_comboBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));

		// Count the total number of intervals captured.
		g_comboSumIntervalWritePtr->barIntervalsCaptured ++;
	}

	// Flag for the end of bargraph, used to indicate that a bar interval was stored, thus a summary should be too
	return (accumulatedBarIntervalCount);
}

//*****************************************************************************
// Function: moveComboSummaryIntervalDataToFile(void)
// Purpose : Transfer the data from the summary interval buffer into the event file
//*****************************************************************************
void moveComboSummaryIntervalDataToFile(void)
{
	float rFreq, vFreq, tFreq;

	rFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_comboSumIntervalWritePtr->r.frequency * 2) - 1));
	vFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_comboSumIntervalWritePtr->v.frequency * 2) - 1));
	tFreq = (float)((float)g_triggerRecord.trec.sample_rate / (float)((g_comboSumIntervalWritePtr->t.frequency * 2) - 1));

	// Calculate the Peak Displacement
	g_comboSumIntervalWritePtr->a.displacement = 0;
	g_comboSumIntervalWritePtr->r.displacement = (uint32)(g_comboSumIntervalWritePtr->r.peak * 1000000 / 2 / PI / rFreq);
	g_comboSumIntervalWritePtr->v.displacement = (uint32)(g_comboSumIntervalWritePtr->v.peak * 1000000 / 2 / PI / vFreq);
	g_comboSumIntervalWritePtr->t.displacement = (uint32)(g_comboSumIntervalWritePtr->t.peak * 1000000 / 2 / PI / tFreq);

	// Store timestamp for the end of the summary interval
	g_summaryCount++;
	g_comboSumIntervalWritePtr->summariesCaptured = g_summaryCount;
	g_comboSumIntervalWritePtr->intervalEnd_Time = getCurrentTime();
	g_comboSumIntervalWritePtr->batteryLevel =
		(uint32)(100.0 * convertedBatteryLevel(BATTERY_VOLTAGE));
	g_comboSumIntervalWritePtr->calcStructEndFlag = 0xEECCCCEE;	// End structure flag

	// Reset summary interval count
	g_summaryIntervalCnt = 0;

	fl_fwrite(g_comboSumIntervalWritePtr, sizeof(CALCULATED_DATA_STRUCT), 1, g_comboDualCurrentEventFileHandle);		

	// Move update the job totals.
	UpdateComboJobTotals(g_comboSumIntervalWritePtr);

	// Advance the Summary Interval global buffer pointer
	advanceSumIntervalBufPtr(WRITE_PTR);

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(g_comboSumIntervalWritePtr, 0, sizeof(CALCULATED_DATA_STRUCT));
	byteSet(&g_comboFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));
}

/*****************************************************************************
	Function:	CalculateComboData
	Purpose :	Calculate combo specific info from the raw A/D data that
				was copied into the event buffer
******************************************************************************/
uint8 CalculateComboData(void)
{
	// Temp variables, assigned as static to prevent storing on stack
	static uint16 aTemp, rTemp, vTemp, tTemp;
	static uint32 vsTemp;
	static uint16 aTempNorm, rTempNorm, vTempNorm, tTempNorm;
	static DATE_TIME_STRUCT aTempTime, rTempTime, vTempTime, tTempTime;
	static uint8 g_aJobFreqFlag = NO, g_rJobFreqFlag = NO, g_vJobFreqFlag = NO, g_tJobFreqFlag = NO;
	static uint8 g_aJobPeakMatch = NO, g_rJobPeakMatch = NO, g_vJobPeakMatch = NO, g_tJobPeakMatch = NO;

	uint8 gotDataFlag = NO;
	int32 processingCount = 500;
	uint16* dataReadStart;

	while ((g_bg430DataReadPtr != g_bg430DataWritePtr) && (processingCount-- > 0))
	{

		if (processingCount == 1)
		{
			if (g_bg430DataWritePtr < g_bg430DataReadPtr)
			{
				if (((g_bg430DataWritePtr - g_bg430DataStartPtr) +
					 (g_bg430DataEndPtr - g_bg430DataReadPtr)) > 500)
					processingCount = 500;
			}
			else
			{
				if ((g_bg430DataWritePtr - g_bg430DataReadPtr) > 500)
					processingCount = 500;
			}
		}

		dataReadStart = g_bg430DataReadPtr;

		// Make sure that we will not catch up to writing the data, almost impossible.
		if (g_bg430DataReadPtr == g_bg430DataWritePtr)
		{
			debug("ERROR 1a - Reading ptr equal to writing ptr.");
			g_bg430DataReadPtr = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		// Move from the pre-trigger buffer to our large ram buffer, but check for data wrapping.
		aTemp = (uint16)((*g_bg430DataReadPtr++) & DATA_MASK);
		if (g_bg430DataReadPtr > g_bg430DataEndPtr) g_bg430DataReadPtr = g_bg430DataStartPtr;

		// We have caught up to the end of the write with out it being completed.
		if (g_bg430DataReadPtr == g_bg430DataWritePtr)
		{
			debug("ERROR 1b - Reading ptr equal to writing ptr.");
			g_bg430DataReadPtr = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		rTemp = (uint16)((*g_bg430DataReadPtr++) & DATA_MASK);
		if (g_bg430DataReadPtr > g_bg430DataEndPtr) g_bg430DataReadPtr = g_bg430DataStartPtr;
		if (g_bg430DataReadPtr == g_bg430DataWritePtr)
		{
			debug("ERROR 1c - Reading ptr equal to writing ptr.");
			g_bg430DataReadPtr = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		vTemp = (uint16)((*g_bg430DataReadPtr++) & DATA_MASK);
		if (g_bg430DataReadPtr > g_bg430DataEndPtr) g_bg430DataReadPtr = g_bg430DataStartPtr;
		if (g_bg430DataReadPtr == g_bg430DataWritePtr)
		{
			debug("ERROR 1d - Reading ptr equal to writing ptr.");
			g_bg430DataReadPtr = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		tTemp = (uint16)((*g_bg430DataReadPtr++) & DATA_MASK);
		if (g_bg430DataReadPtr > g_bg430DataEndPtr) g_bg430DataReadPtr = g_bg430DataStartPtr;

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
		if (aTempNorm > g_comboBarIntervalWritePtr->aMax)
			g_comboBarIntervalWritePtr->aMax = aTempNorm;

		// ---------------
		// R, V, T channel
		// ---------------
		// Store the max R, V or T normalized value if a new max was found
		if (rTempNorm > g_comboBarIntervalWritePtr->rvtMax)
			g_comboBarIntervalWritePtr->rvtMax = rTempNorm;

		if (vTempNorm > g_comboBarIntervalWritePtr->rvtMax)
			g_comboBarIntervalWritePtr->rvtMax = vTempNorm;

		if (tTempNorm > g_comboBarIntervalWritePtr->rvtMax)
			g_comboBarIntervalWritePtr->rvtMax = tTempNorm;

		// ----------
		// Vector Sum
		// ----------
		// Store the max Vector Sum if a new max was found
		if (vsTemp > g_comboBarIntervalWritePtr->vsMax)
			g_comboBarIntervalWritePtr->vsMax = vsTemp;

		//=================================================
		// Summary Interval
		//=================================================
		// ---------
		// A channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (aTempNorm >= g_comboSumIntervalWritePtr->a.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (aTempNorm == g_comboSumIntervalWritePtr->a.peak)
			{
				// Sample matches current max, set match flag
				g_comboFreqCalcBuffer.a.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				aTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->a.peak = aTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->a_Time = getCurrentTime();
				// Reset match flag since a new peak was found
				g_comboFreqCalcBuffer.a.matchFlag = FALSE;
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
			g_comboFreqCalcBuffer.a.updateFlag = TRUE;
		}

		// ---------
		// R channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (rTempNorm >= g_comboSumIntervalWritePtr->r.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (rTempNorm == g_comboSumIntervalWritePtr->r.peak)
			{
				// Sample matches current max, set match flag
				g_comboFreqCalcBuffer.r.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				rTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->r.peak = rTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->r_Time = getCurrentTime();
				// Reset match flag since a new peak was found
				g_comboFreqCalcBuffer.r.matchFlag = FALSE;
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
			g_comboFreqCalcBuffer.r.updateFlag = TRUE;
		}

		// ---------
		// V channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (vTempNorm >= g_comboSumIntervalWritePtr->v.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (vTempNorm == g_comboSumIntervalWritePtr->v.peak)
			{
				// Sample matches current max, set match flag
				g_comboFreqCalcBuffer.v.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				vTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->v.peak = vTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->v_Time = getCurrentTime();
				// Reset match flag since a new peak was found
				g_comboFreqCalcBuffer.v.matchFlag = FALSE;
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
			g_comboFreqCalcBuffer.v.updateFlag = TRUE;
		}

		// ---------
		// T channel
		// ---------
		// Check if the current sample is equal or greater than the stored max
		if (tTempNorm >= g_comboSumIntervalWritePtr->t.peak)
		{
			// Check if the current max matches exactly to the stored max
			if (tTempNorm == g_comboSumIntervalWritePtr->t.peak)
			{
				// Sample matches current max, set match flag
				g_comboFreqCalcBuffer.t.matchFlag = TRUE;
				// Store timestamp in case this is the max (and count) we want to keep
				tTempTime = getCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->t.peak = tTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->t_Time = getCurrentTime();
				// Reset match flag since a new peak was found
				g_comboFreqCalcBuffer.t.matchFlag = FALSE;
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
			g_comboFreqCalcBuffer.t.updateFlag = TRUE;
		}

		// ----------
		// Vector Sum
		// ----------
		// Store the max Vector Sum if a new max was found
		if (vsTemp > g_comboSumIntervalWritePtr->vectorSumPeak)
		{
			// Store max vector sum
			g_comboSumIntervalWritePtr->vectorSumPeak = vsTemp;
			// Store timestamp
			g_comboSumIntervalWritePtr->vs_Time = getCurrentTime();

			if (vsTemp > g_vsJobPeak)
			{
				g_vsJobPeak = vsTemp;
			}
		}

		//=================================================
		// Freq Calc Information
		//=================================================
		// ---------
		// A channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_comboFreqCalcBuffer.a.sign ^ (aTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.a.sign = (uint16)(aTemp & SIGNBIT_MASK);

			// If the update flag was set, update freq count information
			if (g_comboFreqCalcBuffer.a.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_comboFreqCalcBuffer.a.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_comboFreqCalcBuffer.a.freq_count > g_comboSumIntervalWritePtr->a.frequency)
					{
						// Save the new count (lower freq)
						g_comboSumIntervalWritePtr->a.frequency = g_comboFreqCalcBuffer.a.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_comboSumIntervalWritePtr->a_Time = aTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_comboSumIntervalWritePtr->a.frequency = g_comboFreqCalcBuffer.a.freq_count;
				}

				// Reset flags
				g_comboFreqCalcBuffer.a.updateFlag = FALSE;
				g_comboFreqCalcBuffer.a.matchFlag = FALSE;
			}

			if (g_aJobFreqFlag == YES)
			{
				if (g_aJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.a.freq_count > g_aJobFreq)
						g_aJobFreq = g_comboFreqCalcBuffer.a.freq_count;

					g_aJobPeakMatch = NO;
				}
				else
				{
					g_aJobFreq = g_comboFreqCalcBuffer.a.freq_count;
				}

				g_aJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_comboFreqCalcBuffer.a.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_comboFreqCalcBuffer.a.freq_count++;
		}

		// ---------
		// R channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_comboFreqCalcBuffer.r.sign ^ (rTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.r.sign = (uint16)(rTemp & SIGNBIT_MASK);

			// If the update flag was set, update freq count information
			if (g_comboFreqCalcBuffer.r.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_comboFreqCalcBuffer.r.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_comboFreqCalcBuffer.r.freq_count > g_comboSumIntervalWritePtr->r.frequency)
					{
						// Save the new count (lower freq)
						g_comboSumIntervalWritePtr->r.frequency = g_comboFreqCalcBuffer.r.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_comboSumIntervalWritePtr->r_Time = rTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_comboSumIntervalWritePtr->r.frequency = g_comboFreqCalcBuffer.r.freq_count;
				}

				// Reset flags
				g_comboFreqCalcBuffer.r.updateFlag = FALSE;
				g_comboFreqCalcBuffer.r.matchFlag = FALSE;
			}

			if (g_rJobFreqFlag == YES)
			{
				if (g_rJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.r.freq_count > g_rJobFreq)
						g_rJobFreq = g_comboFreqCalcBuffer.r.freq_count;

					g_rJobPeakMatch = NO;
				}
				else
				{
					g_rJobFreq = g_comboFreqCalcBuffer.r.freq_count;
				}

				g_rJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_comboFreqCalcBuffer.r.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_comboFreqCalcBuffer.r.freq_count++;
		}

		// ---------
		// V channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_comboFreqCalcBuffer.v.sign ^ (vTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.v.sign = (uint16)(vTemp & SIGNBIT_MASK);

			// If the update flag was set, update freq count information
			if (g_comboFreqCalcBuffer.v.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_comboFreqCalcBuffer.v.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_comboFreqCalcBuffer.v.freq_count > g_comboSumIntervalWritePtr->v.frequency)
					{
						// Save the new count (lower freq)
						g_comboSumIntervalWritePtr->v.frequency = g_comboFreqCalcBuffer.v.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_comboSumIntervalWritePtr->v_Time = vTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_comboSumIntervalWritePtr->v.frequency = g_comboFreqCalcBuffer.v.freq_count;
				}

				// Reset flags
				g_comboFreqCalcBuffer.v.updateFlag = FALSE;
				g_comboFreqCalcBuffer.v.matchFlag = FALSE;
			}

			if (g_vJobFreqFlag == YES)
			{
				if (g_vJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.v.freq_count > g_vJobFreq)
						g_vJobFreq = g_comboFreqCalcBuffer.v.freq_count;

					g_vJobPeakMatch = NO;
				}
				else
				{
					g_vJobFreq = g_comboFreqCalcBuffer.v.freq_count;
				}

				g_vJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_comboFreqCalcBuffer.v.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_comboFreqCalcBuffer.v.freq_count++;
		}

		// ---------
		// T channel
		// ---------
		// Check if the stored sign comparison signals a zero crossing
		if (g_comboFreqCalcBuffer.t.sign ^ (tTemp & SIGNBIT_MASK))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.t.sign = (uint16)(tTemp & SIGNBIT_MASK);

			// If the update flag was set, update freq count information
			if (g_comboFreqCalcBuffer.t.updateFlag == TRUE)
			{
				// Check if the max values matched
				if (g_comboFreqCalcBuffer.t.matchFlag == TRUE)
				{
					// Check if current count is greater (lower freq) than stored count
					if (g_comboFreqCalcBuffer.t.freq_count > g_comboSumIntervalWritePtr->t.frequency)
					{
						// Save the new count (lower freq)
						g_comboSumIntervalWritePtr->t.frequency = g_comboFreqCalcBuffer.t.freq_count;
						// Save the timestamp corresponding to the lower freq
						g_comboSumIntervalWritePtr->t_Time = tTempTime;
					}
				}
				else // We had a new max, store count for freq calculation
				{
					// Move data to summary interval struct
					g_comboSumIntervalWritePtr->t.frequency = g_comboFreqCalcBuffer.t.freq_count;
				}

				// Reset flags
				g_comboFreqCalcBuffer.t.updateFlag = FALSE;
				g_comboFreqCalcBuffer.t.matchFlag = FALSE;
			}

			if (g_tJobFreqFlag == YES)
			{
				if (g_tJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.t.freq_count > g_tJobFreq)
						g_tJobFreq = g_comboFreqCalcBuffer.t.freq_count;

					g_tJobPeakMatch = NO;
				}
				else
				{
					g_tJobFreq = g_comboFreqCalcBuffer.t.freq_count;
				}

				g_tJobFreqFlag = NO;
			}

			// Reset count to 1 since we have a sample that's crossed zero boundary
			g_comboFreqCalcBuffer.t.freq_count = 1;
		}
		else
		{
			// Increment count since we haven't crossed a zero boundary
			g_comboFreqCalcBuffer.t.freq_count++;
		}

		//=================================================
		// End of Bar Interval
		//=================================================
		if (++g_barIntervalCnt >= (uint32)(g_triggerRecord.bgrec.barInterval * g_triggerRecord.trec.sample_rate))
		{
			moveComboBarIntervalDataToFile();

			//=================================================
			// End of Summary Interval
			//=================================================
			if (++g_summaryIntervalCnt >=
				(uint32)(g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval))
			{
				moveComboSummaryIntervalDataToFile();
			}
		}

	} // While != loop

	if (g_bg430DataWritePtr != g_bg430DataReadPtr)
	{
		return (BG_BUFFER_NOT_EMPTY);
	}
	else
	{
		return (BG_BUFFER_EMPTY);
	}
}

//*****************************************************************************
// Function: MoveStartOfComboEventRecordToFile
// Purpose : Move the combo Event Record into file
//*****************************************************************************
void MoveStartOfComboEventRecordToFile(void)
{
	// Get new file handle
	g_comboDualCurrentEventFileHandle = getNewEventFileHandle(g_currentEventNumber);
				
	if (g_comboDualCurrentEventFileHandle == NULL)
		debugErr("Failed to get a new file handle for the current Combo - Bargraph event!\n");

	// Write in the current but unfinished event record to provide an offset to start writing in the data
	fl_fwrite(&g_RamEventRecord, sizeof(EVT_RECORD), 1, g_comboDualCurrentEventFileHandle);

	// ns8100 - *NOTE* With Combo and keeping consistant with event numbers, save now instead of at the end of bargraph
	// Combo - Bargraph event created, inc event number for potential Combo - Waveform events
	storeCurrentEventNumber();		
}

//*****************************************************************************
// Function: MoveEndOfComboEventRecordToFile
// Purpose : Move the combo Event Record into the file
//*****************************************************************************
void MoveEndOfComboEventRecordToFile(void)
{
	// The following data will be filled in when the data has been moved over to flash.
	g_RamEventRecord.header.summaryChecksum = 0xAABB;
	g_RamEventRecord.header.dataChecksum = 0xCCDD;
	g_RamEventRecord.header.dataCompression = 0;

	g_RamEventRecord.summary.captured.endTime = getCurrentTime();

	// Make sure at the beginning of the file
	fl_fseek(g_comboDualCurrentEventFileHandle, 0, SEEK_SET);

	// Rewrite the event record
	fl_fwrite(&g_RamEventRecord, sizeof(EVT_RECORD), 1, g_comboDualCurrentEventFileHandle);

	fl_fclose(g_comboDualCurrentEventFileHandle);
	debug("Bargraph event file closed\n");

#if 0 // ns8100 - *NOTE* With Combo and keeping consistant with event numbers, save moved to start instead of at the end of bargraph
	// Store and increment the event number even if we do not save the event header information.
	storeCurrentEventNumber();
#endif
}

//*****************************************************************************
// Function:	advanceBarIntervalBufPtr
// Purpose :	handle advancing the bar interval buffer pointer
//*****************************************************************************
void advanceComboBarIntervalBufPtr(uint8 bufferType)
{
	if (bufferType == READ_PTR)
	{
		g_comboBarIntervalReadPtr++;
		if (g_comboBarIntervalReadPtr > g_comboBarIntervalEndPtr)
			g_comboBarIntervalReadPtr = &(g_comboBarInterval[0]);
	}
	else
	{
		g_comboBarIntervalWritePtr++;
		if (g_comboBarIntervalWritePtr > g_comboBarIntervalEndPtr)
			g_comboBarIntervalWritePtr = &(g_comboBarInterval[0]);
	}
}

//*****************************************************************************
// Function:	advanceSumIntervalBufPtr
// Purpose :	handle advancing the bar interval buffer pointer
//*****************************************************************************
void advanceComboSumIntervalBufPtr(uint8 bufferType)
{

	if (bufferType == READ_PTR)
	{
		g_comboSumIntervalReadPtr++;
		if (g_comboSumIntervalReadPtr > g_comboSumIntervalEndPtr)
			g_comboSumIntervalReadPtr = &(g_comboSummaryInterval[0]);
	}
	else
	{
		g_comboSumIntervalWritePtr++;
		if (g_comboSumIntervalWritePtr > g_comboSumIntervalEndPtr)
			g_comboSumIntervalWritePtr = &(g_comboSummaryInterval[0]);
	}
}

//*****************************************************************************
// FUNCTION:	void UpdateComboJobTotals(void)
// DESCRIPTION
//*****************************************************************************
void UpdateComboJobTotals(CALCULATED_DATA_STRUCT* sumIntervalPtr)
{
	// Update Combo Job Totals structure with most recent Summary Interval max's
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

#if 0 // Unused
//*****************************************************************************
// Function:	checkSpaceForBarSummaryInterval
// Purpose:
//*****************************************************************************
BOOLEAN checkSpaceForComboBarSummaryInterval(void)
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
#endif

//*****************************************************************************
// Function:	MoveComboWaveformEventToFile
// Purpose :
//*****************************************************************************
void MoveComboWaveformEventToFile(void)
{
	static FLASH_MOV_STATE flashMovState = FLASH_IDLE;
	static SUMMARY_DATA* sumEntry;
	static SUMMARY_DATA* ramSummaryEntry;
	static int32 sampGrpsLeft;
	static uint32 vectorSumTotal;

	uint16 normalizedData;
	uint16 i;
	uint16 sample;
	uint32 vectorSum;
	uint16 tempPeak;
	FLASH_USAGE_STRUCT flashStats;
	INPUT_MSG_STRUCT msg;

	if (g_freeEventBuffers < g_maxEventBuffers)
	{
		switch (flashMovState)
		{
			case FLASH_IDLE:
				if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
				{
					debugErr("Out of Ram Summary Entrys\n");
				}

				sumEntry = &g_summaryTable[g_currentEventBuffer];
				sumEntry->mode = WAVEFORM_MODE;

				// Initialize the freq data counts.
				sumEntry->waveShapeData.a.freq = 0;
				sumEntry->waveShapeData.r.freq = 0;
				sumEntry->waveShapeData.v.freq = 0;
				sumEntry->waveShapeData.t.freq = 0;
				flashMovState = FLASH_PRE;
				break;

			case FLASH_PRE:
				for (i = (uint16)g_samplesInPretrig; i != 0; i--)
				{
					// Store entire sample
					//storeData(g_currentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);
					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}
				flashMovState = FLASH_BODY_INT;
				break;

			case FLASH_BODY_INT:
				sampGrpsLeft = (int)(g_samplesInBody - 1);

				// A channel
				sample = *(g_currentEventSamplePtr + 0);
				sumEntry->waveShapeData.a.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				sumEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + 0);

				// R channel
				sample = *(g_currentEventSamplePtr + 1);
				tempPeak = sumEntry->waveShapeData.r.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				vectorSum = (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + 1);

				// V channel
				sample = *(g_currentEventSamplePtr + 2);
				tempPeak = sumEntry->waveShapeData.v.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				vectorSum += (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + 2);

				// T channel
				sample = *(g_currentEventSamplePtr + 3);
				tempPeak = sumEntry->waveShapeData.t.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				vectorSum += (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + 3);

				vectorSumTotal = (uint32)vectorSum;

				g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;

				flashMovState = FLASH_BODY;
				break;

			case FLASH_BODY:
				for (i = 0; ((i < g_triggerRecord.trec.sample_rate) && (sampGrpsLeft != 0)); i++)
				{
					sampGrpsLeft--;

					// A channel
					sample = *(g_currentEventSamplePtr + 0);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.a.peak)
					{
						sumEntry->waveShapeData.a.peak = normalizedData;
						sumEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + 0);
					}

					// R channel
					sample = *(g_currentEventSamplePtr + 1);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.r.peak)
					{
						sumEntry->waveShapeData.r.peak = normalizedData;
						sumEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + 1);
					}
					vectorSum = (uint32)(normalizedData * normalizedData);

					// V channel
					sample = *(g_currentEventSamplePtr + 2);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.v.peak)
					{
						sumEntry->waveShapeData.v.peak = normalizedData;
						sumEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + 2);
					}
					vectorSum += (normalizedData * normalizedData);

					// T channel
					sample = *(g_currentEventSamplePtr + 3);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.t.peak)
					{
						sumEntry->waveShapeData.t.peak = normalizedData;
						sumEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + 3);
					}
					vectorSum += (normalizedData * normalizedData);

					// Vector Sum
					if (vectorSum > vectorSumTotal)
					{
						vectorSumTotal = (uint32)vectorSum;
					}

					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}

				if (sampGrpsLeft == 0)
				{
					g_RamEventRecord.summary.calculated.vectorSumPeak = vectorSumTotal;

					flashMovState = FLASH_CAL;
				}
				break;

			case FLASH_CAL:
#if 0 // Does nothing
				// Loop 100 times
				for (i = (uint16)(g_samplesInCal / (g_triggerRecord.trec.sample_rate / 1024)); i != 0; i--)
				{
					// Advance the pointer using sample rate ratio to act as a filter to always scale down to a 1024 rate
					g_currentEventSamplePtr += ((g_triggerRecord.trec.sample_rate/1024) * g_sensorInfoPtr->numOfChannels);
				}
#endif

				sumEntry->waveShapeData.a.freq = CalcSumFreq(sumEntry->waveShapeData.a.peakPtr, (uint16)g_triggerRecord.trec.sample_rate);
				sumEntry->waveShapeData.r.freq = CalcSumFreq(sumEntry->waveShapeData.r.peakPtr, (uint16)g_triggerRecord.trec.sample_rate);
				sumEntry->waveShapeData.v.freq = CalcSumFreq(sumEntry->waveShapeData.v.peakPtr, (uint16)g_triggerRecord.trec.sample_rate);
				sumEntry->waveShapeData.t.freq = CalcSumFreq(sumEntry->waveShapeData.t.peakPtr, (uint16)g_triggerRecord.trec.sample_rate);

				completeRamEventSummary(ramSummaryEntry, sumEntry);

				// Get new event file handle
				g_currentEventFileHandle = getNewEventFileHandle(g_currentEventNumber);

				if (g_currentEventFileHandle == NULL)
				{
					debugErr("Failed to get a new file handle for the current Combo - Waveform event!\n");
				}					
				else // Write the file event to the SD card
				{
					// Write the event record header and summary
					fl_fwrite(&g_RamEventRecord, sizeof(EVT_RECORD), 1, g_currentEventFileHandle);

					// Write the event data, containing the pretrigger, event and cal
					fl_fwrite(g_currentEventStartPtr, g_wordSizeInEvent, 2, g_currentEventFileHandle);

					// Done writing the event file, close the file handle
					fl_fclose(g_currentEventFileHandle);
					debug("Event file closed\n");

					updateMonitorLogEntry();

					// After event numbers have been saved, store current event number in persistent storage.
					storeCurrentEventNumber();

					// Now store the updated event number in the universal ram storage.
					g_RamEventRecord.summary.eventNumber = g_currentEventNumber;
				}

				// Update event buffer count and pointers
				if (++g_currentEventBuffer == g_maxEventBuffers)
				{
					g_currentEventBuffer = 0;
					g_currentEventStartPtr = g_currentEventSamplePtr = g_startOfEventBufferPtr;
				}
				else
				{
					g_currentEventStartPtr = g_currentEventSamplePtr = g_startOfEventBufferPtr + (g_currentEventBuffer * g_wordSizeInEvent);
				}

				if (g_freeEventBuffers == g_maxEventBuffers)
				{
					clearSystemEventFlag(TRIGGER_EVENT);
				}

				g_lastCompletedRamSummary = ramSummaryEntry;

				g_printOutMode = WAVEFORM_MODE;
				raiseMenuEventFlag(RESULTS_MENU_EVENT);

				//debug("DataBuffs: Changing flash move state: %s\n", "FLASH_IDLE");
				flashMovState = FLASH_IDLE;
				g_freeEventBuffers++;

				if (getPowerControlState(LCD_POWER_ENABLE) == OFF)
				{
					assignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, displayTimerCallBack);
					assignSoftTimer(LCD_PW_ON_OFF_TIMER_NUM, (uint32)(g_helpRecord.lcd_timeout * TICKS_PER_MIN), lcdPwTimerCallBack);
				}

				// Check to see if there is room for another event, if not send a signal to stop monitoring
				if (g_helpRecord.flash_wrapping == NO)
				{
					getFlashUsageStats(&flashStats);

					if (flashStats.waveEventsLeft == 0)
					{
						msg.cmd = STOP_MONITORING_CMD;
						msg.length = 1;
						(*menufunc_ptrs[MONITOR_MENU])(msg);
					}
				}

				// Check if AutoDialout is enabled and signal the system if necessary
				checkAutoDialoutStatus();
			break;
		}
	}
	else
	{
		clearSystemEventFlag(TRIGGER_EVENT);

		// Check if not monitoring
		if (g_sampleProcessing != SAMPLING_STATE)
		{
 			// There were queued event buffers after monitoring was stopped
 			// Close the monitor log entry now since all events have been stored
			closeMonitorLogEntry();
		}
	}
}
