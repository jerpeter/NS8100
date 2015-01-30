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
#if 0
void ProcessWaveformData(void)
{
	//SUMMARY_DATA* sumEntry;

#if 0 // Removed
	//static uint16 commandNibble;

	// Store the command nibble for the first channel of data
	//commandNibble = (uint16)(*(g_tailOfPretriggerBuff) & EMBEDDED_CMD);
#endif

	// Check if still waiting for an event
	if (g_isTriggered == NO)
	{
		// Check if not processing a cal pulse
		if (g_processingCal == NO)
		{
			// Check the Command Nibble on the first channel (Acoustic)
			switch (0)//g_waveState)
			//switch (commandNibble)
			{
				case TRIG_ONE:
					// Check if there are still free event containers and we are still taking new events
					if ((g_freeEventBuffers != 0) && (g_doneTakingEvents == NO))
					{
						// Store the exact time we received the trigger data sample
						//g_pendingEventRecord.summary.captured.eventTime = GetCurrentTime();

						// Change global wave state flag
						//g_waveState = PENDING;

						// Set loop counter to 1 minus the total samples to be received in the event body (minus the trigger data sample)
						g_isTriggered = g_samplesInBody - 1;

						// Copy Pretrigger buffer data over to the Event body buffer
						*(g_eventBufferBodyPtr + 0) = *(g_tailOfPretriggerBuff + 0);
						*(g_eventBufferBodyPtr + 1) = *(g_tailOfPretriggerBuff + 1);
						*(g_eventBufferBodyPtr + 2) = *(g_tailOfPretriggerBuff + 2);
						*(g_eventBufferBodyPtr + 3) = *(g_tailOfPretriggerBuff + 3);

						// Advance the pointers
						g_eventBufferBodyPtr += 4;
						g_tailOfPretriggerBuff += 4;

						// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
						if (g_sensorInfoPtr->numOfChannels <= 4)
						{
							// Check if the end of the Pretrigger buffer buffer has been reached
							if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;

							// Copy Pretrigger buffer data over to the Event Pretrigger buffer
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
					// Set loop counter to 1 minus the total cal samples to be recieved (minus the cal start sample)
					g_processingCal = g_samplesInCal - 1;

					// Change global wave state flag
					//g_waveState = PENDING;

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
							g_tailOfPretriggerBuff += 4;
							
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
							g_tailOfPretriggerBuff += 4;
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
							g_tailOfPretriggerBuff += 4;
							break;
					}
					break;

				default:
					// Advance the Pretrigger buffer buffer the number of active channels
					g_tailOfPretriggerBuff += g_sensorInfoPtr->numOfChannels;
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
					g_tailOfPretriggerBuff += 4;
          
					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						//*(g_eventBufferCalPtr - 4) |= CAL_END;
						//*(g_eventBufferCalPtr - 3) |= CAL_END;
						//*(g_eventBufferCalPtr - 2) |= CAL_END;
						//*(g_eventBufferCalPtr - 1) |= CAL_END;

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
					g_tailOfPretriggerBuff += 4;
          
					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						//*(g_delayedOneEventBufferCalPtr - 4) |= CAL_END;	*(g_eventBufferCalPtr - 4) |= CAL_END;
						//*(g_delayedOneEventBufferCalPtr - 3) |= CAL_END;	*(g_eventBufferCalPtr - 3) |= CAL_END;
						//*(g_delayedOneEventBufferCalPtr - 2) |= CAL_END;	*(g_eventBufferCalPtr - 2) |= CAL_END;
						//*(g_delayedOneEventBufferCalPtr - 1) |= CAL_END;	*(g_eventBufferCalPtr - 1) |= CAL_END;

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
					g_tailOfPretriggerBuff += 4;

					if (g_processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						//*(g_delayedTwoEventBufferCalPtr - 4) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 4) |= CAL_END;	*(g_eventBufferCalPtr - 4) |= CAL_END;
						//*(g_delayedTwoEventBufferCalPtr - 3) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 3) |= CAL_END;	*(g_eventBufferCalPtr - 3) |= CAL_END;
						//*(g_delayedTwoEventBufferCalPtr - 2) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 2) |= CAL_END;	*(g_eventBufferCalPtr - 2) |= CAL_END;
						//*(g_delayedTwoEventBufferCalPtr - 1) |= CAL_END;	*(g_delayedOneEventBufferCalPtr - 1) |= CAL_END;	*(g_eventBufferCalPtr - 1) |= CAL_END;

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
			g_tailOfPretriggerBuff += 4;
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
			g_tailOfPretriggerBuff += 4;
		}

		// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
		if (g_sensorInfoPtr->numOfChannels <= 4)
		{
			// Check if there are still PreTrigger data samples in the Pretrigger buffer
			if ((g_samplesInBody - g_isTriggered) < g_samplesInPretrig)
			{
				// Check if the end of the Pretrigger buffer has been reached
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

		// Decrement g_isTriggered since another event sample has been stored
		g_isTriggered--;

		// Check if all the event data from the Pretrigger buffer has been moved into the event buffer
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

	// Check if the end of the Pretrigger buffer buffer has been reached
	if (g_tailOfPretriggerBuff >= g_endOfPretriggerBuff) g_tailOfPretriggerBuff = g_startOfPretriggerBuff;
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "fsaccess.h"
#include "M23018.h"
void MoveWaveformEventToFile(void)
{
	static WAVE_PROCESSING_STATE waveformProcessingState = WAVE_INIT;
	//static SUMMARY_DATA* sumEntry;
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
	uint8 keypadLedConfig;
	uint32 bytesWritten;
	uint32 remainingDataLength;

#if 1 // Atmel fat driver
	int waveformFileHandle = -1;
#else // Port fat driver
	FL_FILE* waveformFileHandle = NULL;
#endif

	if (g_freeEventBuffers < g_maxEventBuffers)
	{
		switch (waveformProcessingState)
		{
			case WAVE_INIT:
				if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
				{
					debugErr("Out of Ram Summary Entrys\r\n");
				}

				// Added temporarily to prevent SPI access issues
				// fix_ns8100
				g_pendingEventRecord.summary.captured.eventTime = g_startOfEventDateTimestampBufferPtr[g_eventBufferReadIndex]; //GetCurrentTime();

				if (getSystemEventState(EXT_TRIGGER_EVENT))
				{
					// Mark in the pending event record that this due to an External trigger signal
					g_pendingEventRecord.summary.captured.externalTrigger = YES;

					clearSystemEventFlag(EXT_TRIGGER_EVENT);
				}

#if 0 // Old
				sumEntry = &g_summaryTable[g_eventBufferReadIndex];
#endif

#if 1 // Need to clear out the Summary entry since it's initialized to all 0xFF's
				memset(ramSummaryEntry, 0, sizeof(SUMMARY_DATA));
#else // Old
				// Initialize the freq data counts.
				ramSummaryEntry->waveShapeData.a.freq = 0;
				ramSummaryEntry->waveShapeData.r.freq = 0;
				ramSummaryEntry->waveShapeData.v.freq = 0;
				ramSummaryEntry->waveShapeData.t.freq = 0;
#endif

				ramSummaryEntry->mode = WAVEFORM_MODE;
				waveformProcessingState = WAVE_PRETRIG;
				break;

			case WAVE_PRETRIG:
				for (i = g_samplesInPretrig; i != 0; i--)
				{
					if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}

				waveformProcessingState = WAVE_BODY_INIT;
				break;

			case WAVE_BODY_INIT:
				sampGrpsLeft = (g_samplesInBody - 1);

				if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

				// A channel
				sample = *(g_currentEventSamplePtr + A_CHAN_OFFSET);
				ramSummaryEntry->waveShapeData.a.peak = FixDataToZero(sample);
				ramSummaryEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + A_CHAN_OFFSET);

				// R channel
				sample = *(g_currentEventSamplePtr + R_CHAN_OFFSET);
				tempPeak = ramSummaryEntry->waveShapeData.r.peak = FixDataToZero(sample);
				vectorSum = (uint32)(tempPeak * tempPeak);
				ramSummaryEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + R_CHAN_OFFSET);

				// V channel
				sample = *(g_currentEventSamplePtr + V_CHAN_OFFSET);
				tempPeak = ramSummaryEntry->waveShapeData.v.peak = FixDataToZero(sample);
				vectorSum += (uint32)(tempPeak * tempPeak);
				ramSummaryEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + V_CHAN_OFFSET);

				// T channel
				sample = *(g_currentEventSamplePtr + T_CHAN_OFFSET);
				tempPeak = ramSummaryEntry->waveShapeData.t.peak = FixDataToZero(sample);
				vectorSum += (uint32)(tempPeak * tempPeak);
				ramSummaryEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + T_CHAN_OFFSET);

				vectorSumMax = (uint32)vectorSum;

				g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;

				waveformProcessingState = WAVE_BODY;
				break;

			case WAVE_BODY:
				for (i = 0; ((i < g_triggerRecord.trec.sample_rate) && (sampGrpsLeft != 0)); i++)
				{
					sampGrpsLeft--;

					if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();

					// A channel
					sample = *(g_currentEventSamplePtr + A_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > ramSummaryEntry->waveShapeData.a.peak)
					{
						ramSummaryEntry->waveShapeData.a.peak = normalizedData;
						ramSummaryEntry->waveShapeData.a.peakPtr = (g_currentEventSamplePtr + A_CHAN_OFFSET);
					}

					// R channel
					sample = *(g_currentEventSamplePtr + R_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > ramSummaryEntry->waveShapeData.r.peak)
					{
						ramSummaryEntry->waveShapeData.r.peak = normalizedData;
						ramSummaryEntry->waveShapeData.r.peakPtr = (g_currentEventSamplePtr + R_CHAN_OFFSET);
					}
					vectorSum = (uint32)(normalizedData * normalizedData);

					// V channel
					sample = *(g_currentEventSamplePtr + V_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > ramSummaryEntry->waveShapeData.v.peak)
					{
						ramSummaryEntry->waveShapeData.v.peak = normalizedData;
						ramSummaryEntry->waveShapeData.v.peakPtr = (g_currentEventSamplePtr + V_CHAN_OFFSET);
					}
					vectorSum += (normalizedData * normalizedData);

					// T channel
					sample = *(g_currentEventSamplePtr + T_CHAN_OFFSET);
					normalizedData = FixDataToZero(sample);
					if (normalizedData > ramSummaryEntry->waveShapeData.t.peak)
					{
						ramSummaryEntry->waveShapeData.t.peak = normalizedData;
						ramSummaryEntry->waveShapeData.t.peakPtr = (g_currentEventSamplePtr + T_CHAN_OFFSET);
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

					waveformProcessingState = WAVE_CAL_PULSE;
				}
				break;

			case WAVE_CAL_PULSE:
				for (i = g_samplesInCal; i != 0; i--)
				{
					if (g_bitShiftForAccuracy) AdjustSampleForBitAccuracy();
					
					g_currentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}

				waveformProcessingState = WAVE_CALCULATE;
				break;

			case WAVE_CALCULATE:
				startOfEventPtr = g_startOfEventBufferPtr + (g_eventBufferReadIndex * g_wordSizeInEvent);
				endOfEventDataPtr = startOfEventPtr + (g_wordSizeInPretrig + g_wordSizeInEvent);
				ramSummaryEntry->waveShapeData.a.freq = CalcSumFreq(ramSummaryEntry->waveShapeData.a.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);
				ramSummaryEntry->waveShapeData.r.freq = CalcSumFreq(ramSummaryEntry->waveShapeData.r.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);
				ramSummaryEntry->waveShapeData.v.freq = CalcSumFreq(ramSummaryEntry->waveShapeData.v.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);
				ramSummaryEntry->waveShapeData.t.freq = CalcSumFreq(ramSummaryEntry->waveShapeData.t.peakPtr, g_triggerRecord.trec.sample_rate, startOfEventPtr, endOfEventDataPtr);

#if 0 // Old
				CompleteRamEventSummary(ramSummaryEntry, sumEntry);
#else // Updated
				CompleteRamEventSummary(ramSummaryEntry);
#endif
				CacheResultsEventInfo((EVT_RECORD*)&g_pendingEventRecord);

				waveformProcessingState = WAVE_STORE;
				break;

			case WAVE_STORE:
				if ((g_spi1AccessLock == AVAILABLE) && (g_fileAccessLock == AVAILABLE))
				{
					g_spi1AccessLock = EVENT_LOCK;
					//g_fileAccessLock = FILE_LOCK;
					nav_select(FS_NAV_ID_DEFAULT);

					// Get new event file handle
					waveformFileHandle = GetEventFileHandle(g_pendingEventRecord.summary.eventNumber, CREATE_EVENT_FILE);

#if 1 // Atmel fat driver
					if (waveformFileHandle == -1)
#else // Port fat driver
					if (waveformFileHandle == NULL)
#endif
					{
						debugErr("Failed to get a new file handle for the current %s event\r\n", (g_triggerRecord.op_mode == WAVEFORM_MODE) ? "Waveform" : "Combo - Waveform");
					}					
					else // Write the file event to the SD card
					{
						sprintf((char*)&g_spareBuffer[0], "%s EVENT #%d BEING SAVED... (MAY TAKE TIME)",
								(g_triggerRecord.op_mode == WAVEFORM_MODE) ? "WAVEFORM" : "COMBO - WAVEFORM", g_pendingEventRecord.summary.eventNumber);
						OverlayMessage("EVENT COMPLETE", (char*)&g_spareBuffer[0], 0);

#if 1 // Atmel fat driver
						// Write the event record header and summary
						bytesWritten = write(waveformFileHandle, &g_pendingEventRecord, sizeof(EVT_RECORD));

						if (bytesWritten != sizeof(EVT_RECORD))
						{
							debugErr("Waveform Event Record written size incorrect (%d)\r\n", bytesWritten);
						}

						remainingDataLength = (g_wordSizeInEvent * 2);

						// Check if there are multiple chunks to write
						if (remainingDataLength > WAVEFORM_FILE_WRITE_CHUNK_SIZE)
						{
							// Get the current state of the keypad LED
							keypadLedConfig = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
						}

						while (remainingDataLength)
						{
							if (remainingDataLength > WAVEFORM_FILE_WRITE_CHUNK_SIZE)
							{
								// Write the event data, containing the Pretrigger, event and cal
								bytesWritten = write(waveformFileHandle, g_currentEventStartPtr, WAVEFORM_FILE_WRITE_CHUNK_SIZE);

								if (bytesWritten != WAVEFORM_FILE_WRITE_CHUNK_SIZE)
								{
									debugErr("Waveform Event Data written size incorrect (%d)\r\n", bytesWritten);
								}

								remainingDataLength -= WAVEFORM_FILE_WRITE_CHUNK_SIZE;
								g_currentEventStartPtr += (WAVEFORM_FILE_WRITE_CHUNK_SIZE / 2);

								// Quickly toggle the green LED to show status of saving a waveform event (while too busy to update LCD)
								if (keypadLedConfig & GREEN_LED_PIN) { keypadLedConfig &= ~GREEN_LED_PIN; }
								else { keypadLedConfig |= GREEN_LED_PIN; }

								WriteMcp23018(IO_ADDRESS_KPD, GPIOA, keypadLedConfig);
							}
							else // Remaining data size is less than the file write chunk size
							{
								// Write the event data, containing the Pretrigger, event and cal
								bytesWritten = write(waveformFileHandle, g_currentEventStartPtr, remainingDataLength);

								if (bytesWritten != remainingDataLength)
								{
									debugErr("Waveform Event Data written size incorrect (%d)\r\n", bytesWritten);
								}

								remainingDataLength = 0;
							}
						}

						SetFileDateTimestamp(FS_DATE_LAST_WRITE);

						// Done writing the event file, close the file handle
						close(waveformFileHandle);
#else // Port fat driver
						// Write the event record header and summary
						fl_fwrite(&g_pendingEventRecord, sizeof(EVT_RECORD), 1, waveformFileHandle);

						// Write the event data, containing the Pretrigger, event and cal
						fl_fwrite(g_currentEventStartPtr, g_wordSizeInEvent, 2, waveformFileHandle);

						// Done writing the event file, close the file handle
						fl_fclose(waveformFileHandle);
#endif
						debug("Waveform Event file closed\r\n");
					}

					AddEventToSummaryList(&g_pendingEventRecord);

					g_spi1AccessLock = AVAILABLE;
					waveformProcessingState = WAVE_COMPLETE;
				}
				break;

			case WAVE_COMPLETE:
				ramSummaryEntry->fileEventNum = g_pendingEventRecord.summary.eventNumber;
					
				UpdateMonitorLogEntry();

#if 0 // Prevent spam storing the event number if there are multiple events
				// After event numbers have been saved, store current event number in persistent storage.
				StoreCurrentEventNumber();
#else // Just increment the number and save when the monitor session is done
				IncrementCurrentEventNumber();
#endif
				UpdateSDCardUsageStats(sizeof(EVT_RECORD) + g_wordSizeInEvent);

				// Now store the updated event number in the universal ram storage.
				g_pendingEventRecord.summary.eventNumber = g_nextEventNumberToUse;

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

				// fix_ns8100 - Currently does nothing since freeing of buffer happens below and a check is at the start
				if (g_freeEventBuffers == g_maxEventBuffers)
				{
					clearSystemEventFlag(TRIGGER_EVENT);
				}

				g_lastCompletedRamSummaryIndex = ramSummaryEntry;

				if (g_triggerRecord.op_mode == WAVEFORM_MODE)
				{
					raiseMenuEventFlag(RESULTS_MENU_EVENT);
				}
				// else (g_triggerRecord.op_mode == COMBO_MODE)
				// Leave in monitor mode menu display processing for bargraph

				//debug("DataBuffs: Changing flash move state: %s\r\n", "WAVE_INIT");
				waveformProcessingState = WAVE_INIT;
				g_freeEventBuffers++;

				if (GetPowerControlState(LCD_POWER_ENABLE) == OFF)
				{
					AssignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, DisplayTimerCallBack);
					AssignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_unitConfig.lcdTimeout * TICKS_PER_MIN), LcdPwTimerCallBack);
				}

				// Check to see if there is room for another event, if not send a signal to stop monitoring
				if (g_unitConfig.flashWrapping == NO)
				{
					if (g_sdCardUsageStats.waveEventsLeft == 0)
					{
						msg.cmd = STOP_MONITORING_CMD;
						msg.length = 1;
						(*menufunc_ptrs[MONITOR_MENU])(msg);
					}
				}

				// Check if AutoDialout is enabled and signal the system if necessary
				CheckAutoDialoutStatus();
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
