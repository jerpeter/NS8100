///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "InitDataBuffers.h"
#include "SysEvents.h"
#include "Summary.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "Record.h"
#include "Menu.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "EventProcessing.h"
#include "PowerManagement.h"
#include "RemoteCommon.h"
#include "FAT32_FileLib.h"
#include "usart.h"
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
void StartNewCombo(void)
{
	g_comboSummaryPtr = NULL;

	// Get the address and empty Ram summary
	if (GetRamSummaryEntry(&g_comboSummaryPtr) == FALSE)
	{
		debug("Out of Ram Summary Entrys\r\n");
		return;
	}

	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	ByteSet(&(g_comboBarInterval[0]), 0, (sizeof(BARGRAPH_BAR_INTERVAL_DATA) * NUM_OF_BAR_INTERVAL_BUFFERS));
	ByteSet(&(g_comboSummaryInterval[0]), 0, (SUMMARY_INTERVAL_SIZE_IN_BYTES * NUM_OF_SUM_INTERVAL_BUFFERS));

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	ByteSet(&g_comboFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));

	g_summaryCount = 0;
	g_oneSecondCnt = 0;			// Count the secs so that we can increment the sum capture rate.
	g_oneMinuteCount = 0;		// Flag to mark the end of a summary sample in terms of time.
	g_barIntervalCnt = 0;		// Count the number of samples that make up a bar interval.
	g_summaryIntervalCnt = 0;	// Count the number of bars that make up a summary interval.
	g_totalBarIntervalCnt = 0;

	g_comboBarIntervalWritePtr = &(g_comboBarInterval[0]);
	g_comboBarIntervalReadPtr = &(g_comboBarInterval[0]);
	g_comboSumIntervalWritePtr = &(g_comboSummaryInterval[0]);
	g_comboSumIntervalReadPtr = &(g_comboSummaryInterval[0]);

	// Clear out the Bar Interval buffer to be used next
	ByteSet(g_comboBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));
	// Clear out the Summary Interval and Freq Calc buffer to be used next
	ByteSet(g_comboSumIntervalWritePtr, 0, SUMMARY_INTERVAL_SIZE_IN_BYTES);

	MoveStartOfComboEventRecordToFile();

	// Update the current monitor log entry
	UpdateMonitorLogEntry();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void EndCombo(void)
{
	uint32 barIntervalsStored;

	while (CalculateComboData() == BG_BUFFER_NOT_EMPTY) {}

	// For the last time, put the data into the event buffer.
	barIntervalsStored = MoveComboBarIntervalDataToFile();

	// Check if bar intervals were stored or a summery interval is present
	if ((barIntervalsStored) || (g_summaryIntervalCnt))
	{
		MoveComboSummaryIntervalDataToFile();
	}

	MoveEndOfComboEventRecordToFile();
	g_fileProcessActiveUsbLockout = OFF;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0
void ProcessComboData(void)
{
	// In theory, the following would be processed
	// however they both advance the Pretrigger buffer pointer
	//ProcessComboSampleData();
	ProcessComboBargraphData();

	// Handle Pretrigger buffer pointer for circular buffer after processing both individual modes,
	//	neither of which advanced the pointer
	if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0
void ProcessComboDataSkipBargraphDuringCal(void)
{
	ProcessComboSampleData();

	if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0
void ProcessComboBargraphData(void)
{
	// Check to see if we have a chunk of ram buffer to write, otherwise check for data wrapping.
	if ((g_bargraphDataEndPtr - g_bargraphDataWritePtr) >= 4)
	{
		// Move from the Pretrigger buffer to our large ram buffer.
		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 0);
		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 1);
		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 2);
		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 3);

		// Check for the end and if so go to the top
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

	}
	else
	{
		// Move from the Pretrigger buffer to our large ram buffer, but check for data wrapping.
		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 0);
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 1);
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 2);
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;

		*g_bargraphDataWritePtr++ = *(g_tailOfPretriggerBuff + 3);
		if (g_bargraphDataWritePtr > g_bargraphDataEndPtr) g_bargraphDataWritePtr = g_bargraphDataStartPtr;
	}

	// Alert system that we have data in ram buffer, raise flag to calculate and move data to flash.
	raiseSystemEventFlag(BARGRAPH_EVENT);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0
void ProcessComboSampleData(void)
{
	//SUMMARY_DATA* sumEntry;
	static uint16 commandNibble;

	// Store the command nibble for the first channel of data
	commandNibble = (uint16)(*(g_tailOfPretriggerBuff) & EMBEDDED_CMD);

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
						g_pendingEventRecord.summary.captured.eventTime = GetCurrentTime();

						// Set loop counter to 1 minus the total samples to be received in the event body (minus the trigger data sample)
						g_isTriggered = g_samplesInBody - 1;

						// Copy Pretrigger buffer data over to the Event body buffer
						*(g_eventBufferBodyPtr + 0) = *(g_tailOfPretriggerBuff + 0);
						*(g_eventBufferBodyPtr + 1) = *(g_tailOfPretriggerBuff + 1);
						*(g_eventBufferBodyPtr + 2) = *(g_tailOfPretriggerBuff + 2);
						*(g_eventBufferBodyPtr + 3) = *(g_tailOfPretriggerBuff + 3);

						// Advance the pointers
						g_eventBufferBodyPtr += 4;
						// Now handled outside of routine
						//g_tailOfPretriggerBuff += 4;

						// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
						if (g_sensorInfoPtr->numOfChannels <= 4)
						{
							// Check if the end of the Pretrigger buffer has been reached
							if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;

							// Copy Pretrigger buffer data over to the event Pretrigger buffer
							*(g_eventBufferPretrigPtr + 0) = (uint16)((*(g_tailOfPretriggerBuff + 0) & DATA_MASK) | EVENT_START);
							*(g_eventBufferPretrigPtr + 1) = (uint16)((*(g_tailOfPretriggerBuff + 1) & DATA_MASK) | EVENT_START);
							*(g_eventBufferPretrigPtr + 2) = (uint16)((*(g_tailOfPretriggerBuff + 2) & DATA_MASK) | EVENT_START);
							*(g_eventBufferPretrigPtr + 3) = (uint16)((*(g_tailOfPretriggerBuff + 3) & DATA_MASK) | EVENT_START);

							// Advance the pointer (Don't advance g_tailOfPretriggerBuff since just reading Pretrigger buffer data)
							g_eventBufferPretrigPtr += 4;
						}
					}
					else
					{
						// Full or finished
					}
					break;

				case CAL_START:
					// Set loop counter to 1 minus the total cal samples to be received (minus the cal start sample)
					g_processingCal = g_samplesInCal - 1;

					switch (g_calTestExpected)
					{
						// Received a Cal pulse after the event
						case 1:
							// Copy Pretrigger buffer data over to the Event Cal buffer
							*(g_eventBufferCalPtr + 0) = (uint16)((*(g_tailOfPretriggerBuff + 0) & DATA_MASK) | CAL_START);
							*(g_eventBufferCalPtr + 1) = (uint16)((*(g_tailOfPretriggerBuff + 1) & DATA_MASK) | CAL_START);
							*(g_eventBufferCalPtr + 2) = (uint16)((*(g_tailOfPretriggerBuff + 2) & DATA_MASK) | CAL_START);
							*(g_eventBufferCalPtr + 3) = (uint16)((*(g_tailOfPretriggerBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							g_eventBufferCalPtr += 4;
							// Now handled outside of routine
							//g_tailOfPretriggerBuff += 4;

							break;

						// Received a Cal pulse which was delayed once (use/copy the cal for both events)
						case 2:
							// Set the pointer to the second event Cal buffer
							g_delayedOneEventBufferCalPtr = g_eventBufferCalPtr + g_wordSizeInEvent;

							// Copy Pretrigger buffer data over to the Event Cal buffers
							*(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = (uint16)((*(g_tailOfPretriggerBuff + 0) & DATA_MASK) | CAL_START);
							*(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = (uint16)((*(g_tailOfPretriggerBuff + 1) & DATA_MASK) | CAL_START);
							*(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = (uint16)((*(g_tailOfPretriggerBuff + 2) & DATA_MASK) | CAL_START);
							*(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = (uint16)((*(g_tailOfPretriggerBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							g_delayedOneEventBufferCalPtr += 4;
							g_eventBufferCalPtr += 4;
							// Now handled outside of routine
							//g_tailOfPretriggerBuff += 4;
							break;

						// Received a Cal pulse which was delayed twice (use/copy the cal for all three events)
						case 3:
							// Set the pointer to the second event Cal buffer
							g_delayedOneEventBufferCalPtr = g_eventBufferCalPtr + g_wordSizeInEvent;

							// Set the pointer to the third event Cal buffer
							g_delayedTwoEventBufferCalPtr = g_delayedOneEventBufferCalPtr + g_wordSizeInEvent;

							// Copy Pretrigger buffer data over to the Event Cal buffers
							*(g_delayedTwoEventBufferCalPtr + 0) = *(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = (uint16)((*(g_tailOfPretriggerBuff + 0) & DATA_MASK) | CAL_START);
							*(g_delayedTwoEventBufferCalPtr + 1) = *(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = (uint16)((*(g_tailOfPretriggerBuff + 1) & DATA_MASK) | CAL_START);
							*(g_delayedTwoEventBufferCalPtr + 2) = *(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = (uint16)((*(g_tailOfPretriggerBuff + 2) & DATA_MASK) | CAL_START);
							*(g_delayedTwoEventBufferCalPtr + 3) = *(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = (uint16)((*(g_tailOfPretriggerBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							g_delayedTwoEventBufferCalPtr += 4;
							g_delayedOneEventBufferCalPtr += 4;
							g_eventBufferCalPtr += 4;
							// Now handled outside of routine
							//g_tailOfPretriggerBuff += 4;
							break;
					}
					break;

				default:
					// Advance the Pretrigger buffer buffer the number of active channels
					// Now handled outside of this routine
					//g_tailOfPretriggerBuff += g_sensorInfoPtr->numOfChannels;
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
					// Copy Pretrigger buffer data over to the Event Cal buffer
					*(g_eventBufferCalPtr + 0) = *(g_tailOfPretriggerBuff + 0);
					*(g_eventBufferCalPtr + 1) = *(g_tailOfPretriggerBuff + 1);
					*(g_eventBufferCalPtr + 2) = *(g_tailOfPretriggerBuff + 2);
					*(g_eventBufferCalPtr + 3) = *(g_tailOfPretriggerBuff + 3);

					// Advance the pointers
					g_eventBufferCalPtr += 4;
					// Now handled outside of routine
					//g_tailOfPretriggerBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(g_eventBufferCalPtr - 4) |= CAL_END;
						*(g_eventBufferCalPtr - 3) |= CAL_END;
						*(g_eventBufferCalPtr - 2) |= CAL_END;
						*(g_eventBufferCalPtr - 1) |= CAL_END;

						g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

						//debugRaw("TE\r\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						g_calTestExpected = 0;
					}
					break;

				case 2:
					*(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = *(g_tailOfPretriggerBuff + 0);
					*(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = *(g_tailOfPretriggerBuff + 1);
					*(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = *(g_tailOfPretriggerBuff + 2);
					*(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = *(g_tailOfPretriggerBuff + 3);

					// Advance the pointers
					g_delayedOneEventBufferCalPtr += 4;
					g_eventBufferCalPtr += 4;
					// Now handled outside of routine
					//g_tailOfPretriggerBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(g_delayedOneEventBufferCalPtr - 4) |= CAL_END;	*(g_eventBufferCalPtr - 4) |= CAL_END;
						*(g_delayedOneEventBufferCalPtr - 3) |= CAL_END;	*(g_eventBufferCalPtr - 3) |= CAL_END;
						*(g_delayedOneEventBufferCalPtr - 2) |= CAL_END;	*(g_eventBufferCalPtr - 2) |= CAL_END;
						*(g_delayedOneEventBufferCalPtr - 1) |= CAL_END;	*(g_eventBufferCalPtr - 1) |= CAL_END;

						g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

						//debugRaw("TE\r\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						g_calTestExpected = 0;
					}
					break;

				case 3:
					*(g_delayedTwoEventBufferCalPtr + 0) = *(g_delayedOneEventBufferCalPtr + 0) = *(g_eventBufferCalPtr + 0) = *(g_tailOfPretriggerBuff + 0);
					*(g_delayedTwoEventBufferCalPtr + 1) = *(g_delayedOneEventBufferCalPtr + 1) = *(g_eventBufferCalPtr + 1) = *(g_tailOfPretriggerBuff + 1);
					*(g_delayedTwoEventBufferCalPtr + 2) = *(g_delayedOneEventBufferCalPtr + 2) = *(g_eventBufferCalPtr + 2) = *(g_tailOfPretriggerBuff + 2);
					*(g_delayedTwoEventBufferCalPtr + 3) = *(g_delayedOneEventBufferCalPtr + 3) = *(g_eventBufferCalPtr + 3) = *(g_tailOfPretriggerBuff + 3);

					// Advance the pointers
					g_delayedTwoEventBufferCalPtr += 4;
					g_delayedOneEventBufferCalPtr += 4;
					g_eventBufferCalPtr += 4;
					// Now handled outside of routine
					//g_tailOfPretriggerBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(g_delayedTwoEventBufferCalPtr - 4) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 4) |= CAL_END;	*(g_eventBufferCalPtr - 4) |= CAL_END;
						*(g_delayedTwoEventBufferCalPtr - 3) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 3) |= CAL_END;	*(g_eventBufferCalPtr - 3) |= CAL_END;
						*(g_delayedTwoEventBufferCalPtr - 2) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 2) |= CAL_END;	*(g_eventBufferCalPtr - 2) |= CAL_END;
						*(g_delayedTwoEventBufferCalPtr - 1) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 1) |= CAL_END;	*(g_eventBufferCalPtr - 1) |= CAL_END;

						g_eventBufferCalPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig + g_wordSizeInBody;

						//debugRaw("TE\r\n");
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
			// Copy the channel data over from the Pretrigger buffer to the event buffer
			// Strip off the command nibble and mark the end of the event
			*(g_eventBufferBodyPtr + 0) = (uint16)((*(g_tailOfPretriggerBuff + 0) & DATA_MASK) | EVENT_END);
			*(g_eventBufferBodyPtr + 1) = (uint16)((*(g_tailOfPretriggerBuff + 1) & DATA_MASK) | EVENT_END);
			*(g_eventBufferBodyPtr + 2) = (uint16)((*(g_tailOfPretriggerBuff + 2) & DATA_MASK) | EVENT_END);
			*(g_eventBufferBodyPtr + 3) = (uint16)((*(g_tailOfPretriggerBuff + 3) & DATA_MASK) | EVENT_END);

			// Advance the pointers
			g_eventBufferBodyPtr += 4;
			// Now handled outside of routine
			//g_tailOfPretriggerBuff += 4;
		}
		else // Normal samples in the triggered event
		{
			// Copy the channel data over from the Pretrigger buffer to the event buffer
			*(g_eventBufferBodyPtr + 0) = *(g_tailOfPretriggerBuff + 0);
			*(g_eventBufferBodyPtr + 1) = *(g_tailOfPretriggerBuff + 1);
			*(g_eventBufferBodyPtr + 2) = *(g_tailOfPretriggerBuff + 2);
			*(g_eventBufferBodyPtr + 3) = *(g_tailOfPretriggerBuff + 3);

			// Advance the pointers
			g_eventBufferBodyPtr += 4;
			// Now handled outside of routine
			//g_tailOfPretriggerBuff += 4;
		}

		// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
		if (g_sensorInfoPtr->numOfChannels <= 4)
		{
			// Check if there are still PreTrigger data samples in the Pretrigger buffer
			if ((g_samplesInBody - g_isTriggered) < g_samplesInPretrig)
			{
				// Check if the end of the Pretrigger buffer buffer has been reached
				if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;

				// Copy the Pretrigger buffer data samples over to the Pretrigger data buffer
				*(g_eventBufferPretrigPtr + 0) = *(g_tailOfPretriggerBuff + 0);
				*(g_eventBufferPretrigPtr + 1) = *(g_tailOfPretriggerBuff + 1);
				*(g_eventBufferPretrigPtr + 2) = *(g_tailOfPretriggerBuff + 2);
				*(g_eventBufferPretrigPtr + 3) = *(g_tailOfPretriggerBuff + 3);

				// Advance the pointer (Don't advance g_tailOfPretriggerBuff since just reading Pretrigger buffer data)
				g_eventBufferPretrigPtr += 4;
			}
		}

		// De-increment g_isTriggered since another event sample has been stored
		g_isTriggered--;

		// Check if all the event data from the Pretrigger buffer buffer has been moved into the event buffer
		if (g_isTriggered == 0)
		{
			g_freeEventBuffers--;
			g_eventBufferWriteIndex++;
			g_calTestExpected++;

			if (g_eventBufferWriteIndex < g_maxEventBuffers)
			{
				g_eventBufferPretrigPtr = g_eventBufferBodyPtr + g_wordSizeInCal;
				g_eventBufferBodyPtr = g_eventBufferPretrigPtr + g_wordSizeInPretrig;
			}
			else
			{
				g_eventBufferWriteIndex = 0;
				g_eventBufferPretrigPtr = g_startOfEventBufferPtr;
				g_eventBufferBodyPtr = g_startOfEventBufferPtr + g_wordSizeInPretrig;
			}
		}
	}

	// Check if the end of the Pretrigger buffer has been reached
	if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint32 MoveComboBarIntervalDataToFile(void)
{
	uint32 accumulatedBarIntervalCount = g_barIntervalCnt;

	// If Bar Intervals have been cached
	if (g_barIntervalCnt > 0)
	{
		// Reset the bar interval count
		g_barIntervalCnt = 0;

		fl_fwrite(g_comboBarIntervalWritePtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA), 1, g_comboDualCurrentEventFileHandle);		
		g_pendingBargraphRecord.header.dataLength += sizeof(BARGRAPH_BAR_INTERVAL_DATA);

		// Advance the Bar Interval global buffer pointer
		AdvanceBarIntervalBufPtr(WRITE_PTR);

		// Clear out the Bar Interval buffer to be used next
		ByteSet(g_comboBarIntervalWritePtr, 0, sizeof(BARGRAPH_BAR_INTERVAL_DATA));

		// Count the total number of intervals captured.
		g_comboSumIntervalWritePtr->barIntervalsCaptured++;
	}

	// Flag for the end of bargraph, used to indicate that a bar interval was stored, thus a summary should be too
	return (accumulatedBarIntervalCount);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MoveComboSummaryIntervalDataToFile(void)
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
	g_comboSumIntervalWritePtr->intervalEnd_Time = GetCurrentTime();
	g_comboSumIntervalWritePtr->batteryLevel =
		(uint32)(100.0 * GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
	g_comboSumIntervalWritePtr->calcStructEndFlag = 0xEECCCCEE;	// End structure flag

	// Reset summary interval count
	g_summaryIntervalCnt = 0;

	fl_fwrite(g_comboSumIntervalWritePtr, sizeof(CALCULATED_DATA_STRUCT), 1, g_comboDualCurrentEventFileHandle);		
	g_pendingBargraphRecord.header.dataLength += sizeof(CALCULATED_DATA_STRUCT);

	// Move update the job totals.
	UpdateComboJobTotals(g_comboSumIntervalWritePtr);

	// Advance the Summary Interval global buffer pointer
	AdvanceSumIntervalBufPtr(WRITE_PTR);

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	ByteSet(g_comboSumIntervalWritePtr, 0, sizeof(CALCULATED_DATA_STRUCT));
	ByteSet(&g_comboFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 CalculateComboData(void)
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
				aTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->a.peak = aTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->a_Time = GetCurrentTime();
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
				rTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->r.peak = rTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->r_Time = GetCurrentTime();
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
				vTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->v.peak = vTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->v_Time = GetCurrentTime();
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
				tTempTime = GetCurrentTime();
			}
			else
			{
				// Copy over new max
				g_comboSumIntervalWritePtr->t.peak = tTempNorm;
				// Update timestamp
				g_comboSumIntervalWritePtr->t_Time = GetCurrentTime();
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
			g_comboSumIntervalWritePtr->vs_Time = GetCurrentTime();

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
		if (g_comboFreqCalcBuffer.a.sign ^ (currentDataSample.a & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.a.sign = (uint16)(currentDataSample.a & g_bitAccuracyMidpoint);

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
		if (g_comboFreqCalcBuffer.r.sign ^ (currentDataSample.r & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.r.sign = (uint16)(currentDataSample.r & g_bitAccuracyMidpoint);

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
		if (g_comboFreqCalcBuffer.v.sign ^ (currentDataSample.v & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.v.sign = (uint16)(currentDataSample.v & g_bitAccuracyMidpoint);

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
		if (g_comboFreqCalcBuffer.t.sign ^ (currentDataSample.t & g_bitAccuracyMidpoint))
		{
			// Store new sign for future zero crossing comparisons
			g_comboFreqCalcBuffer.t.sign = (uint16)(currentDataSample.t & g_bitAccuracyMidpoint);

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
			MoveComboBarIntervalDataToFile();

			//=================================================
			// End of Summary Interval
			//=================================================
			if (++g_summaryIntervalCnt >=
				(uint32)(g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval))
			{
				MoveComboSummaryIntervalDataToFile();
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
void MoveStartOfComboEventRecordToFile(void)
{
	// Get new file handle
	g_comboDualCurrentEventFileHandle = GetEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);
				
	if (g_comboDualCurrentEventFileHandle == NULL) { debugErr("Failed to get a new file handle for the current Combo - Bargraph event\r\n"); }

	// Write in the current but unfinished event record to provide an offset to start writing in the data
	fl_fwrite(&g_pendingBargraphRecord, sizeof(EVT_RECORD), 1, g_comboDualCurrentEventFileHandle);

	// ns8100 - *NOTE* With Combo and keeping consistant with event numbers, save now instead of at the end of bargraph
	// Combo - Bargraph event created, inc event number for potential Combo - Waveform events
	StoreCurrentEventNumber();
	
	// Update the Waveform pending record with the new event number to use
	g_pendingEventRecord.summary.eventNumber = g_nextEventNumberToUse;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MoveEndOfComboEventRecordToFile(void)
{
	// The following data will be filled in when the data has been moved over to flash.
	g_pendingBargraphRecord.header.summaryChecksum = 0xAABB;
	g_pendingBargraphRecord.header.dataChecksum = 0xCCDD;
	g_pendingBargraphRecord.header.dataCompression = 0;

	g_pendingBargraphRecord.summary.captured.endTime = GetCurrentTime();

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

	// Make sure at the beginning of the file
	fl_fseek(g_comboDualCurrentEventFileHandle, 0, SEEK_SET);

	// Rewrite the event record
	fl_fwrite(&g_pendingBargraphRecord, sizeof(EVT_RECORD), 1, g_comboDualCurrentEventFileHandle);

	fl_fclose(g_comboDualCurrentEventFileHandle);
	debug("Bargraph event file closed\r\n");

#if 0 // ns8100 - *NOTE* With Combo and keeping consistant with event numbers, save moved to start instead of at the end of bargraph
	// Store and increment the event number even if we do not save the event header information.
	StoreCurrentEventNumber();
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdvanceComboBarIntervalBufPtr(uint8 bufferType)
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdvanceComboSumIntervalBufPtr(uint8 bufferType)
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateComboJobTotals(CALCULATED_DATA_STRUCT* sumIntervalPtr)
{
	// Update Combo Job Totals structure with most recent Summary Interval max's
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

#if 0 // Unused
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
BOOLEAN CheckSpaceForComboBarSummaryInterval(void)
{
	uint32 barIntervalSize;
	BOOLEAN spaceLeft;
	
	barIntervalSize = (sizeof(CALCULATED_DATA_STRUCT) + (((g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval) + 1) * 8));

	if (g_flashUsageStats.sizeFree > barIntervalSize)
		spaceLeft = YES;
	else
		spaceLeft = NO;

	return (spaceLeft);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MoveComboWaveformEventToFile(void)
{
	static FLASH_MOV_STATE waveformProcessingState = FLASH_IDLE;
	static SUMMARY_DATA* sumEntry;
	static SUMMARY_DATA* ramSummaryEntry;
	static int32 sampGrpsLeft;
	static uint32 vectorSumMax;

	uint16 normalizedData;
	uint32 i;
	uint16 sample;
	uint32 vectorSum;
	uint16 tempPeak;
	INPUT_MSG_STRUCT msg;
	uint16* startOfEventPtr;
	uint16* endOfEventDataPtr;

	if (g_freeEventBuffers < g_maxEventBuffers)
	{
		switch (waveformProcessingState)
		{
			case FLASH_IDLE:
				if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
				{
					debugErr("Out of Ram Summary Entrys\r\n");
				}

				// Added temporarily to prevent SPI access issues
				// fix_ns8100
				g_pendingEventRecord.summary.captured.eventTime = GetCurrentTime();

				if (getSystemEventState(EXT_TRIGGER_EVENT))
				{
					// Mark in the pending event record that this due to an External trigger signal
					g_pendingEventRecord.summary.captured.externalTrigger = YES;

					clearSystemEventFlag(EXT_TRIGGER_EVENT);
				}

				sumEntry = &g_summaryTable[g_eventBufferReadIndex];
				sumEntry->mode = WAVEFORM_MODE;

				// Initialize the freq data counts.
				sumEntry->waveShapeData.a.freq = 0;
				sumEntry->waveShapeData.r.freq = 0;
				sumEntry->waveShapeData.v.freq = 0;
				sumEntry->waveShapeData.t.freq = 0;

				waveformProcessingState = FLASH_PRETRIG;
				break;

			case FLASH_PRETRIG:
				for (i = g_samplesInPretrig; i != 0; i--)
				{
					if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}

				waveformProcessingState = FLASH_BODY_INT;
				break;

			case FLASH_BODY_INT:
				sampGrpsLeft = (g_samplesInBody - 1);

				if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

				// A channel
				sample = *(g_currentEventSamplePtr + A_CHAN_OFFSET);
				sumEntry->waveShapeData.a.peak = FixDataToZero(sample);
				sumEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + A_CHAN_OFFSET);

				// R channel
				sample = *(g_currentEventSamplePtr + R_CHAN_OFFSET);
				tempPeak = sumEntry->waveShapeData.r.peak = FixDataToZero(sample);
				vectorSum = (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + R_CHAN_OFFSET);

				// V channel
				sample = *(g_currentEventSamplePtr + V_CHAN_OFFSET);
				tempPeak = sumEntry->waveShapeData.v.peak = FixDataToZero(sample);
				vectorSum += (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + V_CHAN_OFFSET);

				// T channel
				sample = *(g_currentEventSamplePtr + T_CHAN_OFFSET);
				tempPeak = sumEntry->waveShapeData.t.peak = FixDataToZero(sample);
				vectorSum += (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + T_CHAN_OFFSET);

				vectorSumMax = (uint32)vectorSum;

				g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;

				waveformProcessingState = FLASH_BODY;
				break;

			case FLASH_BODY:
				for (i = 0; ((i < g_triggerRecord.trec.sample_rate) && (sampGrpsLeft != 0)); i++)
				{
					sampGrpsLeft--;

					if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

					// A channel
					sample = *(g_currentEventSamplePtr + A_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > sumEntry->waveShapeData.a.peak)
					{
						sumEntry->waveShapeData.a.peak = normalizedData;
						sumEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + A_CHAN_OFFSET);
					}

					// R channel
					sample = *(g_currentEventSamplePtr + R_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > sumEntry->waveShapeData.r.peak)
					{
						sumEntry->waveShapeData.r.peak = normalizedData;
						sumEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + R_CHAN_OFFSET);
					}
					vectorSum = (uint32)(normalizedData * normalizedData);

					// V channel
					sample = *(g_currentEventSamplePtr + V_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > sumEntry->waveShapeData.v.peak)
					{
						sumEntry->waveShapeData.v.peak = normalizedData;
						sumEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + V_CHAN_OFFSET);
					}
					vectorSum += (normalizedData * normalizedData);

					// T channel
					sample = *(g_currentEventSamplePtr + T_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > sumEntry->waveShapeData.t.peak)
					{
						sumEntry->waveShapeData.t.peak = normalizedData;
						sumEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + T_CHAN_OFFSET);
					}
					vectorSum += (normalizedData * normalizedData);

					// Vector Sum
					if (vectorSum > vectorSumMax)
					{
						vectorSumMax = (uint32)vectorSum;
					}

					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}

				if (sampGrpsLeft == 0)
				{
					g_pendingEventRecord.summary.calculated.vectorSumPeak = vectorSumMax;

					waveformProcessingState = FLASH_CAL;
				}
				break;

			case FLASH_CAL:
				for (i = g_samplesInCal; i != 0; i--)
				{
					if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();
					
					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}

				waveformProcessingState = FLASH_STORE;
				break;
				
			case FLASH_STORE:
				if (g_spi1AccessLock == AVAILABLE)
				{
					g_spi1AccessLock = EVENT_LOCK;

					startOfEventPtr = g_startOfEventBufferPtr + (g_eventBufferReadIndex * g_wordSizeInEvent);
					endOfEventDataPtr = startOfEventPtr + (g_wordSizeInPretrig + g_wordSizeInEvent);
					sumEntry->waveShapeData.a.freq = CalcSumFreq(sumEntry->waveShapeData.a.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);
					sumEntry->waveShapeData.r.freq = CalcSumFreq(sumEntry->waveShapeData.r.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);
					sumEntry->waveShapeData.v.freq = CalcSumFreq(sumEntry->waveShapeData.v.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);
					sumEntry->waveShapeData.t.freq = CalcSumFreq(sumEntry->waveShapeData.t.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);

					CompleteRamEventSummary(ramSummaryEntry, sumEntry);
					CacheResultsEventInfo((EVT_RECORD*)&g_pendingEventRecord);

					// Get new event file handle
					g_currentEventFileHandle = GetEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);

					if (g_currentEventFileHandle == NULL)
					{
						debugErr("Failed to get a new file handle for the current Combo - Waveform event\r\n");

						//ReInitSdCardAndFat32();
						//g_currentEventFileHandle = GetEventFileHandle(g_nextEventNumberToUse, CREATE_EVENT_FILE);
					}					
					else // Write the file event to the SD card
					{
						sprintf((char*)&g_spareBuffer[0], "COMBO WAVEFORM EVENT #%d BEING SAVED... (MAY TAKE TIME)", g_nextEventNumberToUse);
						OverlayMessage("EVENT COMPLETE", (char*)&g_spareBuffer[0], 0);

						// Write the event record header and summary
						fl_fwrite(&g_pendingEventRecord, sizeof(EVT_RECORD), 1, g_currentEventFileHandle);

						// Write the event data, containing the Pretrigger, event and cal
						fl_fwrite(g_currentEventStartPtr, g_wordSizeInEvent, 2, g_currentEventFileHandle);

						// Done writing the event file, close the file handle
						fl_fclose(g_currentEventFileHandle);
						debug("Event file closed\r\n");

						ramSummaryEntry->fileEventNum = g_pendingEventRecord.summary.eventNumber;
					
						UpdateMonitorLogEntry();

						// After event numbers have been saved, store current event number in persistent storage.
						StoreCurrentEventNumber();

						// Now store the updated event number in the universal ram storage.
						g_pendingEventRecord.summary.eventNumber = g_nextEventNumberToUse;
					}

					// Update event buffer count and pointers
					if (++g_eventBufferReadIndex == g_maxEventBuffers)
					{
						g_eventBufferReadIndex = 0;
						g_currentEventStartPtr = g_currentEventSamplePtr = g_startOfEventBufferPtr;
					}
					else
					{
						g_currentEventStartPtr = g_currentEventSamplePtr = g_startOfEventBufferPtr + (g_eventBufferReadIndex * g_wordSizeInEvent);
					}

					if (g_freeEventBuffers == g_maxEventBuffers)
					{
						clearSystemEventFlag(TRIGGER_EVENT);
					}

					g_lastCompletedRamSummaryIndex = ramSummaryEntry;

#if 0 // Leave in monitor mode menu display processing for bargraph
					raiseMenuEventFlag(RESULTS_MENU_EVENT);
#endif

					//debug("DataBuffs: Changing flash move state: %s\r\n", "FLASH_IDLE");
					waveformProcessingState = FLASH_IDLE;
					g_freeEventBuffers++;

					if (GetPowerControlState(LCD_POWER_ENABLE) == OFF)
					{
						AssignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, DisplayTimerCallBack);
						AssignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_unitConfig.lcdTimeout * TICKS_PER_MIN), LcdPwTimerCallBack);
					}

					// Check to see if there is room for another event, if not send a signal to stop monitoring
					if (g_unitConfig.flashWrapping == NO)
					{
						if (g_flashUsageStats.waveEventsLeft == 0)
						{
							msg.cmd = STOP_MONITORING_CMD;
							msg.length = 1;
							(*menufunc_ptrs[MONITOR_MENU])(msg);
						}
					}

					// Check if AutoDialout is enabled and signal the system if necessary
					CheckAutoDialoutStatus();

					g_spi1AccessLock = AVAILABLE;
				}				
			break;
		}
	}
	else
	{
		clearSystemEventFlag(TRIGGER_EVENT);

		// Check if not monitoring
		if (g_sampleProcessing != ACTIVE_STATE)
		{
 			// There were queued event buffers after monitoring was stopped
 			// Close the monitor log entry now since all events have been stored
			CloseMonitorLogEntry();
		}
	}
}
