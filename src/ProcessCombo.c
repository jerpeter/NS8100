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
#include "Rec.h"
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
//extern uint16* endFlashSectorPtr;
extern EVT_RECORD g_RamEventRecord;				// The event record in Ram, being filled for the current event.
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;// Sensor information, calculated when sensor parameters change.
extern REC_EVENT_MN_STRUCT trig_rec;		// Current trigger record structure in ram.
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern REC_HELP_MN_STRUCT help_rec;
extern MN_EVENT_STRUCT mn_event_flags;		// Event flags to indicate current calling menu
extern SYS_EVENT_STRUCT SysEvents_flags;	// System flags for main loop processing.
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);

// Print flag to indicate printing attempt or skip printing.
extern uint8 print_out_mode;
extern uint16* gp_bg430DataStart;
extern uint16* gp_bg430DataWrite;
extern uint16* gp_bg430DataRead;
extern uint16* gp_bg430DataEnd;

extern uint16* startOfPreTrigBuff;
extern uint16* tailOfPreTrigBuff;
extern uint16* endOfPreTrigBuff;
extern uint16 preTrigBuff[PRE_TRIG_BUFF_SIZE_IN_WORDS];
extern uint16 gMaxEventBuffers;
extern uint16 gCurrentEventNumber;
extern uint16 gFreeEventBuffers;
extern uint16 gCalTestExpected;
extern uint32 gSamplesInBody;
extern uint32 gSamplesInPre;
extern uint32 gSamplesInCal;
extern uint32 gSamplesInEvent;
extern uint32 gWordSizeInPre;
extern uint32 gWordSizeInBody;
extern uint32 gWordSizeInCal;
extern uint32 gWordSizeInEvent;
extern uint16* g_startOfEventBufferPtr;
extern uint16* gEventBufferPrePtr;
extern uint16* gEventBufferBodyPtr;
extern uint16* gEventBufferCalPtr;
extern uint16* dgEventBufferCalPtr;
extern uint16* ddgEventBufferCalPtr;
extern uint16* gCurrentEventSamplePtr;
extern uint16 gCurrentEventBuffer;
extern SUMMARY_DATA summaryTable[MAX_RAM_SUMMARYS];
extern SUMMARY_DATA* gLastCompDataSum;
extern uint32 isTriggered;
extern uint32 processingCal;
extern uint16 eventsNotCompressed;
extern uint8 g_doneTakingEvents;
extern uint16  eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS];
extern uint8 g_sampleProcessing;

// For combo processing, defined in ProcessBargraph
extern uint8 gOneMinuteCount;
extern uint32 gOneSecondCnt;
extern uint32 gBarIntervalCnt;
extern uint32 gTotalBarIntervalCnt;
extern uint32 gSummaryIntervalCnt;
extern uint16 gSummaryCnt;

// For combo processing, defined in ProcessBargraph
extern uint16 aImpulsePeak;
extern uint16 rImpulsePeak;
extern uint16 vImpulsePeak;
extern uint16 tImpulsePeak;
extern uint32 vsImpulsePeak;
extern uint16 impulseMenuCount;
extern uint16 aJobPeak;
extern uint16 aJobFreq;
extern uint16 rJobPeak;
extern uint16 rJobFreq;
extern uint16 vJobPeak;
extern uint16 vJobFreq;
extern uint16 tJobPeak;
extern uint16 tJobFreq;
extern uint32 vsJobPeak;

// ns8100
extern FL_FILE* gCurrentEventFileHandle;
extern uint16 g_currentEventNumber;
extern FL_FILE* gComboDualCurrentEventFileHandle;
extern uint16* gCurrentEventStartPtr;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
// This ptr points to the current/in use (ram) summary table entry. It is
// used to access the linkPtr which is really the flash event record Ptr.
// And the record ptr is the location in ram. i.e.
SUMMARY_DATA* gComboSummaryPtr = 0;
BARGRAPH_FREQ_CALC_BUFFER g_comboFreqCalcBuffer;

// A queue of buffers containing summary Interval data, so the
// summary interval data can be printed outside of the ISR context.
CALCULATED_DATA_STRUCT g_comboSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS];
CALCULATED_DATA_STRUCT* g_comboSumIntervalWritePtr = &(g_comboSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_comboSumIntervalReadPtr = &(g_comboSummaryInterval[0]);
CALCULATED_DATA_STRUCT* g_comboSumIntervalEndPtr = &(g_comboSummaryInterval[NUM_OF_SUM_INTERVAL_BUFFERS-1]);

// A queue of buffers containing bar Interval data, so the
// bar interval data can be printed outside of the ISR context.
BARGRAPH_BAR_INTERVAL_DATA g_comboBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS];
BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalWritePtr = &(g_comboBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalReadPtr = &(g_comboBarInterval[0]);
BARGRAPH_BAR_INTERVAL_DATA* g_comboBarIntervalEndPtr = &(g_comboBarInterval[NUM_OF_BAR_INTERVAL_BUFFERS - 1]);

// The following are the Event Data ram buffer pointers.
#if 0 // Unused
uint16* g_comboEventDataBuffer;
uint16* g_comboEventDataStartPtr;
uint16* g_comboEventDataWritePtr;
uint16* g_comboEventDataReadPtr;
uint16* g_comboEventDataEndPtr;
#endif

//*****************************************************************************
// Function:	StartNewCombo
// Purpose :
//*****************************************************************************
void StartNewCombo(void)
{
	gComboSummaryPtr = NULL;

	// Get the address and empty Ram summary
	if (GetRamSummaryEntry(&gComboSummaryPtr) == FALSE)
	{
		debug("Out of Ram Summary Entrys\n");
		return;
	}

	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_comboBarInterval[0]), 0, (sizeof(BARGRAPH_BAR_INTERVAL_DATA) * NUM_OF_BAR_INTERVAL_BUFFERS));
	byteSet(&(g_comboSummaryInterval[0]), 0, (SUMMARY_INTERVAL_SIZE_IN_BYTES * NUM_OF_SUM_INTERVAL_BUFFERS));

	// Clear out the Summary Interval and Freq Calc buffer to be used next
	byteSet(&g_comboFreqCalcBuffer, 0, sizeof(BARGRAPH_FREQ_CALC_BUFFER));

	gSummaryCnt = 0;
	gOneSecondCnt = 0;			// Count the secs so that we can increment the sum capture rate.
	gOneMinuteCount = 0;		// Flag to mark the end of a summary sample in terms of time.
	gBarIntervalCnt = 0;		// Count the number of samples that make up a bar interval.
	gSummaryIntervalCnt = 0;	// Count the number of bars that make up a summary interval.
	gTotalBarIntervalCnt = 0;

#if 0 // Unused
	// Initialize the Bar and Summary Interval buffer pointers to keep in sync
	byteSet(&(g_comboeventDataBufferer[0]), 0, (sizeof(uint16) * BG_DATA_BUFFER_SIZE));

	g_comboEventDataStartPtr = &(g_comboeventDataBufferer[0]);
	g_comboEventDataWritePtr = &(g_comboeventDataBufferer[0]);
	g_comboEventDataReadPtr = &(g_comboeventDataBufferer[0]);
	g_comboEventDataEndPtr = &(g_comboeventDataBufferer[BG_DATA_BUFFER_SIZE - 1]);
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
void EndCombo(void)
{
	uint32 barIntervalsStored;

	while (BG_BUFFER_NOT_EMPTY == CalculateComboData()) {}

	// For the last time, put the data into the event buffer.
	barIntervalsStored = moveComboBarIntervalDataToFile();

	// Check if bar intervals were stored or a summery interval is present
	if ((barIntervalsStored) || (gSummaryIntervalCnt))
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
	if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;
}

/*****************************************************************************
* Function:		ProcessComboBargraphData (Step 2)
* Purpose:		Copy A/D channel data from pretrigger buffer into event buffer
******************************************************************************/
void ProcessComboBargraphData(void)
{
	// Check to see if we have a chunk of ram buffer to write, otherwise check for data wrapping.
	if ((gp_bg430DataEnd - gp_bg430DataWrite) >= 4)
	{
		// Move from the pre-trigger buffer to our large ram buffer.
		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 0);
		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 1);
		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 2);
		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 3);

		// Check for the end and if so go to the top
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

	}
	else
	{
		// Move from the pre-trigger buffer to our large ram buffer, but check for data wrapping.
		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 0);
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 1);
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 2);
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;

		*gp_bg430DataWrite++ = *(tailOfPreTrigBuff + 3);
		if (gp_bg430DataWrite > gp_bg430DataEnd) gp_bg430DataWrite = gp_bg430DataStart;
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
	commandNibble = (uint16)(*(tailOfPreTrigBuff) & EMBEDDED_CMD);

	// Check if still waiting for an event
	if (isTriggered == NO)
	{
		// Check if not processing a cal pulse
		if (processingCal == NO)
		{
			// Check the Command Nibble on the first channel (Acoustic)
			switch (commandNibble)
			{
				case TRIG_ONE:
					// Check if there are still free event containers and we are still taking new events
					if ((gFreeEventBuffers != 0) && (g_doneTakingEvents == NO))
					{
						// Store the exact time we received the trigger data sample
						g_RamEventRecord.summary.captured.eventTime = getCurrentTime();

						// Set loop counter to 1 minus the total samples to be recieved in the event body (minus the trigger data sample)
						isTriggered = gSamplesInBody - 1;

#if 0 // ns7100
						// Save the link to the beginning of the pretrigger data
						summaryTable[gCurrentEventNumber].linkPtr = gEventBufferPrePtr;
#endif
						// Copy PreTrigger data over to the Event body buffer
						*(gEventBufferBodyPtr + 0) = *(tailOfPreTrigBuff + 0);
						*(gEventBufferBodyPtr + 1) = *(tailOfPreTrigBuff + 1);
						*(gEventBufferBodyPtr + 2) = *(tailOfPreTrigBuff + 2);
						*(gEventBufferBodyPtr + 3) = *(tailOfPreTrigBuff + 3);

						// Advance the pointers
						gEventBufferBodyPtr += 4;
						// Now handled outside of routine
						//tailOfPreTrigBuff += 4;

						// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
						if (gp_SensorInfo->numOfChannels <= 4)
						{
							// Check if the end of the PreTrigger buffer has been reached
							if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;

							// Copy PreTrigger data over to the Event PreTrigger buffer
							*(gEventBufferPrePtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_START);
							*(gEventBufferPrePtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_START);
							*(gEventBufferPrePtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_START);
							*(gEventBufferPrePtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_START);

							// Advance the pointer (Don't advance tailOfPreTrigBuff since just reading pretrigger data)
							gEventBufferPrePtr += 4;
						}
					}
					else
					{
						// Full or finished
					}
					break;

				case CAL_START:
					// Set loop counter to 1 minus the total cal samples to be recieved (minus the cal start sample)
					processingCal = gSamplesInCal - 1;

					switch (gCalTestExpected)
					{
						// Received a Cal pulse after the event
						case 1:
							// Copy PreTrigger data over to the Event Cal buffer
							*(gEventBufferCalPtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | CAL_START);
							*(gEventBufferCalPtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | CAL_START);
							*(gEventBufferCalPtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | CAL_START);
							*(gEventBufferCalPtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							gEventBufferCalPtr += 4;
							// Now handled outside of routine
							//tailOfPreTrigBuff += 4;

							break;

						// Received a Cal pulse which was delayed once (use/copy the cal for both events)
						case 2:
							// Set the pointer to the second event Cal buffer
							dgEventBufferCalPtr = gEventBufferCalPtr + gWordSizeInEvent;

							// Copy PreTrigger data over to the Event Cal buffers
							*(dgEventBufferCalPtr + 0) = *(gEventBufferCalPtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | CAL_START);
							*(dgEventBufferCalPtr + 1) = *(gEventBufferCalPtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | CAL_START);
							*(dgEventBufferCalPtr + 2) = *(gEventBufferCalPtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | CAL_START);
							*(dgEventBufferCalPtr + 3) = *(gEventBufferCalPtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							dgEventBufferCalPtr += 4;
							gEventBufferCalPtr += 4;
							// Now handled outside of routine
							//tailOfPreTrigBuff += 4;
							break;

						// Received a Cal pulse which was delayed twice (use/copy the cal for all three events)
						case 3:
							// Set the pointer to the second event Cal buffer
							dgEventBufferCalPtr = gEventBufferCalPtr + gWordSizeInEvent;

							// Set the pointer to the third event Cal buffer
							ddgEventBufferCalPtr = dgEventBufferCalPtr + gWordSizeInEvent;

							// Copy PreTrigger data over to the Event Cal buffers
							*(ddgEventBufferCalPtr + 0) = *(dgEventBufferCalPtr + 0) = *(gEventBufferCalPtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | CAL_START);
							*(ddgEventBufferCalPtr + 1) = *(dgEventBufferCalPtr + 1) = *(gEventBufferCalPtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | CAL_START);
							*(ddgEventBufferCalPtr + 2) = *(dgEventBufferCalPtr + 2) = *(gEventBufferCalPtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | CAL_START);
							*(ddgEventBufferCalPtr + 3) = *(dgEventBufferCalPtr + 3) = *(gEventBufferCalPtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | CAL_START);

							// Advance the pointers
							ddgEventBufferCalPtr += 4;
							dgEventBufferCalPtr += 4;
							gEventBufferCalPtr += 4;
							// Now handled outside of routine
							//tailOfPreTrigBuff += 4;
							break;
					}
					break;

				default:
					// Advance the PreTrigger buffer the number of active channels
					// Now handled outside of this routine
					//tailOfPreTrigBuff += gp_SensorInfo->numOfChannels;
					break;
			}
		}
		else // processingCal != 0
		{
			// Received another data sample, decrement the count
			processingCal--;

			switch (gCalTestExpected)
			{
				case 1:
					// Copy PreTrigger data over to the Event Cal buffer
					*(gEventBufferCalPtr + 0) = *(tailOfPreTrigBuff + 0);
					*(gEventBufferCalPtr + 1) = *(tailOfPreTrigBuff + 1);
					*(gEventBufferCalPtr + 2) = *(tailOfPreTrigBuff + 2);
					*(gEventBufferCalPtr + 3) = *(tailOfPreTrigBuff + 3);

					// Advance the pointers
					gEventBufferCalPtr += 4;
					// Now handled outside of routine
					//tailOfPreTrigBuff += 4;

					if (processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(gEventBufferCalPtr - 4) |= CAL_END;
						*(gEventBufferCalPtr - 3) |= CAL_END;
						*(gEventBufferCalPtr - 2) |= CAL_END;
						*(gEventBufferCalPtr - 1) |= CAL_END;

						gEventBufferCalPtr = gEventBufferPrePtr + gWordSizeInPre + gWordSizeInBody;

						//debugRaw("TE\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						gCalTestExpected = 0;
					}
					break;

				case 2:
					*(dgEventBufferCalPtr + 0) = *(gEventBufferCalPtr + 0) = *(tailOfPreTrigBuff + 0);
					*(dgEventBufferCalPtr + 1) = *(gEventBufferCalPtr + 1) = *(tailOfPreTrigBuff + 1);
					*(dgEventBufferCalPtr + 2) = *(gEventBufferCalPtr + 2) = *(tailOfPreTrigBuff + 2);
					*(dgEventBufferCalPtr + 3) = *(gEventBufferCalPtr + 3) = *(tailOfPreTrigBuff + 3);

					// Advance the pointers
					dgEventBufferCalPtr += 4;
					gEventBufferCalPtr += 4;
					// Now handled outside of routine
					//tailOfPreTrigBuff += 4;

					if (processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(gEventBufferCalPtr - 4) |= CAL_END;
						*(gEventBufferCalPtr - 3) |= CAL_END;
						*(gEventBufferCalPtr - 2) |= CAL_END;
						*(gEventBufferCalPtr - 1) |= CAL_END;

						gEventBufferCalPtr = gEventBufferPrePtr + gWordSizeInPre + gWordSizeInBody;

						//debugRaw("TE\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						gCalTestExpected = 0;
					}
					break;

				case 3:
					*(ddgEventBufferCalPtr + 0) = *(dgEventBufferCalPtr + 0) = *(gEventBufferCalPtr + 0) = *(tailOfPreTrigBuff + 0);
					*(ddgEventBufferCalPtr + 1) = *(dgEventBufferCalPtr + 1) = *(gEventBufferCalPtr + 1) = *(tailOfPreTrigBuff + 1);
					*(ddgEventBufferCalPtr + 2) = *(dgEventBufferCalPtr + 2) = *(gEventBufferCalPtr + 2) = *(tailOfPreTrigBuff + 2);
					*(ddgEventBufferCalPtr + 3) = *(dgEventBufferCalPtr + 3) = *(gEventBufferCalPtr + 3) = *(tailOfPreTrigBuff + 3);

					// Advance the pointers
					ddgEventBufferCalPtr += 4;
					dgEventBufferCalPtr += 4;
					gEventBufferCalPtr += 4;
					// Now handled outside of routine
					//tailOfPreTrigBuff += 4;

					if (processingCal == 0)
					{
						// Temp - Add CAL_END command flag
						*(gEventBufferCalPtr - 4) |= CAL_END;
						*(gEventBufferCalPtr - 3) |= CAL_END;
						*(gEventBufferCalPtr - 2) |= CAL_END;
						*(gEventBufferCalPtr - 1) |= CAL_END;

						gEventBufferCalPtr = gEventBufferPrePtr + gWordSizeInPre + gWordSizeInBody;

						//debugRaw("TE\n");
						raiseSystemEventFlag(TRIGGER_EVENT);
						gCalTestExpected = 0;
					}
					break;
			}
		}
	}
	else // isTriggered != 0
	{
		// Check if this is the last sample of the triggered event
		if ((isTriggered - 1) == 0)
		{
			// Copy the channel data over from the pretrigger to the buffer
			// Strip off the command nibble and mark the end of the event
			*(gEventBufferBodyPtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_END);
			*(gEventBufferBodyPtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_END);
			*(gEventBufferBodyPtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_END);
			*(gEventBufferBodyPtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_END);

			// Advance the pointers
			gEventBufferBodyPtr += 4;
			// Now handled outside of routine
			//tailOfPreTrigBuff += 4;
		}
		else // Normal samples in the triggered event
		{
			// Copy the channel data over from the pretrigger to the buffer
			*(gEventBufferBodyPtr + 0) = *(tailOfPreTrigBuff + 0);
			*(gEventBufferBodyPtr + 1) = *(tailOfPreTrigBuff + 1);
			*(gEventBufferBodyPtr + 2) = *(tailOfPreTrigBuff + 2);
			*(gEventBufferBodyPtr + 3) = *(tailOfPreTrigBuff + 3);

			// Advance the pointers
			gEventBufferBodyPtr += 4;
			// Now handled outside of routine
			//tailOfPreTrigBuff += 4;
		}

		// Check if the number of Active channels is less than or equal to 4 (one seismic sensor)
		if (gp_SensorInfo->numOfChannels <= 4)
		{
			// Check if there are still PreTrigger data samples in the PreTrigger buffer
			if ((gSamplesInBody - isTriggered) < gSamplesInPre)
			{
				// Check if the end of the PreTrigger buffer has been reached
				if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;

				// Copy the PreTrigger data samples over to the PreTrigger data buffer
				*(gEventBufferPrePtr + 0) = *(tailOfPreTrigBuff + 0);
				*(gEventBufferPrePtr + 1) = *(tailOfPreTrigBuff + 1);
				*(gEventBufferPrePtr + 2) = *(tailOfPreTrigBuff + 2);
				*(gEventBufferPrePtr + 3) = *(tailOfPreTrigBuff + 3);

				// Advance the pointer (Don't advance tailOfPreTrigBuff since just reading pretrigger data)
				gEventBufferPrePtr += 4;
			}
		}

		// Deincrement isTriggered since another event sample has been stored
		isTriggered--;

		// Check if all the event data from the PreTrigger buffer has been moved into the event buffer
		if (isTriggered == 0)
		{
			gFreeEventBuffers--;
			gCurrentEventNumber++;
			gCalTestExpected++;

			if (gCurrentEventNumber < gMaxEventBuffers)
			{
				gEventBufferPrePtr = gEventBufferBodyPtr + gWordSizeInCal;
				gEventBufferBodyPtr = gEventBufferPrePtr + gWordSizeInPre;
			}
			else
			{
				gCurrentEventNumber = 0;
				gEventBufferPrePtr = g_startOfEventBufferPtr;
				gEventBufferBodyPtr = g_startOfEventBufferPtr + gWordSizeInPre;
			}
		}
	}

	// Check if the end of the PreTrigger buffer has been reached
	if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;
}

//*****************************************************************************
// Function: moveComboBarIntervalDataToFile(void)
// Purpose : Transfer the data from the bar interval buffers into the event file
//*****************************************************************************
uint32 moveComboBarIntervalDataToFile(void)
{
	uint32 accumulatedBarIntervalCount = gBarIntervalCnt;

	// If Bar Intervals have been cached
	if (gBarIntervalCnt > 0)
	{
		// Reset the bar interval count
		gBarIntervalCnt = 0;

		fl_fwrite(g_comboBarIntervalWritePtr, sizeof(BARGRAPH_BAR_INTERVAL_DATA), 1, gComboDualCurrentEventFileHandle);		

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

	rFreq = (float)((float)trig_rec.trec.sample_rate / (float)((g_comboSumIntervalWritePtr->r.frequency * 2) - 1));
	vFreq = (float)((float)trig_rec.trec.sample_rate / (float)((g_comboSumIntervalWritePtr->v.frequency * 2) - 1));
	tFreq = (float)((float)trig_rec.trec.sample_rate / (float)((g_comboSumIntervalWritePtr->t.frequency * 2) - 1));

	// Calculate the Peak Displacement
	g_comboSumIntervalWritePtr->a.displacement = 0;
	g_comboSumIntervalWritePtr->r.displacement = (uint32)(g_comboSumIntervalWritePtr->r.peak * 1000000 / 2 / PI / rFreq);
	g_comboSumIntervalWritePtr->v.displacement = (uint32)(g_comboSumIntervalWritePtr->v.peak * 1000000 / 2 / PI / vFreq);
	g_comboSumIntervalWritePtr->t.displacement = (uint32)(g_comboSumIntervalWritePtr->t.peak * 1000000 / 2 / PI / tFreq);

	// Store timestamp for the end of the summary interval
	gSummaryCnt++;
	g_comboSumIntervalWritePtr->summariesCaptured = gSummaryCnt;
	g_comboSumIntervalWritePtr->intervalEnd_Time = getCurrentTime();
	g_comboSumIntervalWritePtr->batteryLevel =
		(uint32)(100.0 * convertedBatteryLevel(BATTERY_VOLTAGE));
	g_comboSumIntervalWritePtr->calcStructEndFlag = 0xEECCCCEE;	// End structure flag

	// Reset summary interval count
	gSummaryIntervalCnt = 0;

	fl_fwrite(g_comboSumIntervalWritePtr, sizeof(CALCULATED_DATA_STRUCT), 1, gComboDualCurrentEventFileHandle);		

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
	static uint8 aJobFreqFlag = NO, rJobFreqFlag = NO, vJobFreqFlag = NO, tJobFreqFlag = NO;
	static uint8 aJobPeakMatch = NO, rJobPeakMatch = NO, vJobPeakMatch = NO, tJobPeakMatch = NO;

	uint8 gotDataFlag = NO;
	int32 processingCount = 500;
	uint16* dataReadStart;

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
			debug("ERROR 1a - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		// Move from the pre-trigger buffer to our large ram buffer, but check for data wrapping.
		aTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;

		// We have caught up to the end of the write with out it being completed.
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debug("ERROR 1b - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		rTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debug("ERROR 1c - Reading ptr equal to writing ptr.");
			gp_bg430DataRead = dataReadStart;
			return (BG_BUFFER_NOT_EMPTY);
		}

		vTemp = (uint16)((*gp_bg430DataRead++) & DATA_MASK);
		if (gp_bg430DataRead > gp_bg430DataEnd) gp_bg430DataRead = gp_bg430DataStart;
		if (gp_bg430DataRead == gp_bg430DataWrite)
		{
			debug("ERROR 1d - Reading ptr equal to writing ptr.");
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

			if (aJobFreqFlag == YES)
			{
				if (aJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.a.freq_count > aJobFreq)
						aJobFreq = g_comboFreqCalcBuffer.a.freq_count;

					aJobPeakMatch = NO;
				}
				else
				{
					aJobFreq = g_comboFreqCalcBuffer.a.freq_count;
				}

				aJobFreqFlag = NO;
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

			if (rJobFreqFlag == YES)
			{
				if (rJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.r.freq_count > rJobFreq)
						rJobFreq = g_comboFreqCalcBuffer.r.freq_count;

					rJobPeakMatch = NO;
				}
				else
				{
					rJobFreq = g_comboFreqCalcBuffer.r.freq_count;
				}

				rJobFreqFlag = NO;
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

			if (vJobFreqFlag == YES)
			{
				if (vJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.v.freq_count > vJobFreq)
						vJobFreq = g_comboFreqCalcBuffer.v.freq_count;

					vJobPeakMatch = NO;
				}
				else
				{
					vJobFreq = g_comboFreqCalcBuffer.v.freq_count;
				}

				vJobFreqFlag = NO;
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

			if (tJobFreqFlag == YES)
			{
				if (tJobPeakMatch == YES)
				{
					if (g_comboFreqCalcBuffer.t.freq_count > tJobFreq)
						tJobFreq = g_comboFreqCalcBuffer.t.freq_count;

					tJobPeakMatch = NO;
				}
				else
				{
					tJobFreq = g_comboFreqCalcBuffer.t.freq_count;
				}

				tJobFreqFlag = NO;
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
		if (++gBarIntervalCnt >= (uint32)(trig_rec.bgrec.barInterval * trig_rec.trec.sample_rate))
		{
			moveComboBarIntervalDataToFile();

			//=================================================
			// End of Summary Interval
			//=================================================
			if (++gSummaryIntervalCnt >=
				(uint32)(trig_rec.bgrec.summaryInterval / trig_rec.bgrec.barInterval))
			{
				moveComboSummaryIntervalDataToFile();
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
// Function: MoveStartOfComboEventRecordToFile
// Purpose : Move the combo Event Record into file
//*****************************************************************************
void MoveStartOfComboEventRecordToFile(void)
{
	// Get new file handle
	gComboDualCurrentEventFileHandle = getNewEventFileHandle(g_currentEventNumber);
				
	if (gComboDualCurrentEventFileHandle == NULL)
		debugErr("Failed to get a new file handle for the current Combo - Bargraph event!\n");

	// Write in the current but unfinished event record to provide an offset to start writing in the data
	fl_fwrite(&g_RamEventRecord, sizeof(EVT_RECORD), 1, gComboDualCurrentEventFileHandle);

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
	fl_fseek(gComboDualCurrentEventFileHandle, 0, SEEK_SET);

	// Rewrite the event record
	fl_fwrite(&g_RamEventRecord, sizeof(EVT_RECORD), 1, gComboDualCurrentEventFileHandle);

	fl_fclose(gComboDualCurrentEventFileHandle);
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

	barIntervalSize = (sizeof(CALCULATED_DATA_STRUCT) + (((trig_rec.bgrec.summaryInterval / trig_rec.bgrec.barInterval) + 1) * 8));

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

	if (gFreeEventBuffers < gMaxEventBuffers)
	{
		switch (flashMovState)
		{
			case FLASH_IDLE:
				if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
				{
					debugErr("Out of Ram Summary Entrys\n");
				}

				sumEntry = &summaryTable[gCurrentEventBuffer];
				sumEntry->mode = WAVEFORM_MODE;

				// Initialize the freq data counts.
				sumEntry->waveShapeData.a.freq = 0;
				sumEntry->waveShapeData.r.freq = 0;
				sumEntry->waveShapeData.v.freq = 0;
				sumEntry->waveShapeData.t.freq = 0;
				flashMovState = FLASH_PRE;
				break;

			case FLASH_PRE:
				for (i = (uint16)gSamplesInPre; i != 0; i--)
				{
					// Store entire sample
					//storeData(gCurrentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);
					gCurrentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
				}
				flashMovState = FLASH_BODY_INT;
				break;

			case FLASH_BODY_INT:
				sampGrpsLeft = (int)(gSamplesInBody - 1);

				// A channel
				sample = *(gCurrentEventSamplePtr + 0);
				sumEntry->waveShapeData.a.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				sumEntry->waveShapeData.a.peakPtr = (gCurrentEventSamplePtr + 0);

				// R channel
				sample = *(gCurrentEventSamplePtr + 1);
				tempPeak = sumEntry->waveShapeData.r.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				vectorSum = (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.r.peakPtr = (gCurrentEventSamplePtr + 1);

				// V channel
				sample = *(gCurrentEventSamplePtr + 2);
				tempPeak = sumEntry->waveShapeData.v.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				vectorSum += (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.v.peakPtr = (gCurrentEventSamplePtr + 2);

				// T channel
				sample = *(gCurrentEventSamplePtr + 3);
				tempPeak = sumEntry->waveShapeData.t.peak = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
				vectorSum += (uint32)(tempPeak * tempPeak);
				sumEntry->waveShapeData.t.peakPtr = (gCurrentEventSamplePtr + 3);

				vectorSumTotal = (uint32)vectorSum;

				gCurrentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;

				flashMovState = FLASH_BODY;
				break;

			case FLASH_BODY:
				for (i = 0; ((i < trig_rec.trec.sample_rate) && (sampGrpsLeft != 0)); i++)
				{
					sampGrpsLeft--;

					// A channel
					sample = *(gCurrentEventSamplePtr + 0);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.a.peak)
					{
						sumEntry->waveShapeData.a.peak = normalizedData;
						sumEntry->waveShapeData.a.peakPtr = (gCurrentEventSamplePtr + 0);
					}

					// R channel
					sample = *(gCurrentEventSamplePtr + 1);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.r.peak)
					{
						sumEntry->waveShapeData.r.peak = normalizedData;
						sumEntry->waveShapeData.r.peakPtr = (gCurrentEventSamplePtr + 1);
					}
					vectorSum = (uint32)(normalizedData * normalizedData);

					// V channel
					sample = *(gCurrentEventSamplePtr + 2);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.v.peak)
					{
						sumEntry->waveShapeData.v.peak = normalizedData;
						sumEntry->waveShapeData.v.peakPtr = (gCurrentEventSamplePtr + 2);
					}
					vectorSum += (normalizedData * normalizedData);

					// T channel
					sample = *(gCurrentEventSamplePtr + 3);
					normalizedData = FixDataToZero((uint16)(sample & ~EMBEDDED_CMD));
					if (normalizedData > sumEntry->waveShapeData.t.peak)
					{
						sumEntry->waveShapeData.t.peak = normalizedData;
						sumEntry->waveShapeData.t.peakPtr = (gCurrentEventSamplePtr + 3);
					}
					vectorSum += (normalizedData * normalizedData);

					// Vector Sum
					if (vectorSum > vectorSumTotal)
					{
						vectorSumTotal = (uint32)vectorSum;
					}

					gCurrentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
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
				for (i = (uint16)(gSamplesInCal / (trig_rec.trec.sample_rate / 1024)); i != 0; i--)
				{
					// Advance the pointer using sample rate ratio to act as a filter to always scale down to a 1024 rate
					gCurrentEventSamplePtr += ((trig_rec.trec.sample_rate/1024) * gp_SensorInfo->numOfChannels);
				}
#endif

				sumEntry->waveShapeData.a.freq = CalcSumFreq(sumEntry->waveShapeData.a.peakPtr, (uint16)trig_rec.trec.sample_rate);
				sumEntry->waveShapeData.r.freq = CalcSumFreq(sumEntry->waveShapeData.r.peakPtr, (uint16)trig_rec.trec.sample_rate);
				sumEntry->waveShapeData.v.freq = CalcSumFreq(sumEntry->waveShapeData.v.peakPtr, (uint16)trig_rec.trec.sample_rate);
				sumEntry->waveShapeData.t.freq = CalcSumFreq(sumEntry->waveShapeData.t.peakPtr, (uint16)trig_rec.trec.sample_rate);

				completeRamEventSummary(ramSummaryEntry, sumEntry);

				// Get new event file handle
				gCurrentEventFileHandle = getNewEventFileHandle(g_currentEventNumber);

				if (gCurrentEventFileHandle == NULL)
				{
					debugErr("Failed to get a new file handle for the current Combo - Waveform event!\n");
				}					
				else // Write the file event to the SD card
				{
					// Write the event record header and summary
					fl_fwrite(&g_RamEventRecord, sizeof(EVT_RECORD), 1, gCurrentEventFileHandle);

					// Write the event data, containing the pretrigger, event and cal
					fl_fwrite(gCurrentEventStartPtr, gWordSizeInEvent, 2, gCurrentEventFileHandle);

					// Done writing the event file, close the file handle
					fl_fclose(gCurrentEventFileHandle);
					debug("Event file closed\n");

					updateMonitorLogEntry();

					// After event numbers have been saved, store current event number in persistent storage.
					storeCurrentEventNumber();

					// Now store the updated event number in the universal ram storage.
					g_RamEventRecord.summary.eventNumber = g_currentEventNumber;
				}

				// Update event buffer count and pointers
				if (++gCurrentEventBuffer == gMaxEventBuffers)
				{
					gCurrentEventBuffer = 0;
					gCurrentEventStartPtr = gCurrentEventSamplePtr = g_startOfEventBufferPtr;
				}
				else
				{
					gCurrentEventStartPtr = gCurrentEventSamplePtr = g_startOfEventBufferPtr + (gCurrentEventBuffer * gWordSizeInEvent);
				}

				if (gFreeEventBuffers == gMaxEventBuffers)
				{
					clearSystemEventFlag(TRIGGER_EVENT);
				}

				gLastCompDataSum = ramSummaryEntry;

				print_out_mode = WAVEFORM_MODE;
				raiseMenuEventFlag(RESULTS_MENU_EVENT);

				//debug("DataBuffs: Changing flash move state: %s\n", "FLASH_IDLE");
				flashMovState = FLASH_IDLE;
				gFreeEventBuffers++;

				if (getPowerControlState(LCD_POWER_ENABLE) == OFF)
				{
					assignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, displayTimerCallBack);
					assignSoftTimer(LCD_PW_ON_OFF_TIMER_NUM, (uint32)(help_rec.lcd_timeout * TICKS_PER_MIN), lcdPwTimerCallBack);
				}

				// Check to see if there is room for another event, if not send a signal to stop monitoring
				if (help_rec.flash_wrapping == NO)
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
