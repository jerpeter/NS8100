///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: ProcessWaveform.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:57 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ProcessWaveform.c,v $
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
#include "Mmc2114.h"
#include "SysEvents.h"
#include "Rec.h"
#include "Menu.h"
#include "Summary.h"
#include "Flash.h"
#include "EventProcessing.h"
#include "Board.h"
#include "Rec.h"
#include "Uart.h"
#include "RealTimeClock.h"
#include "ProcessBargraph.h"
#include "PowerManagement.h"
#include "RemoteCommon.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern uint16* endFlashSectorPtr;
extern EVT_RECORD g_RamEventRecord;
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;
extern REC_EVENT_MN_STRUCT trig_rec;
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern REC_HELP_MN_STRUCT help_rec;
extern MN_EVENT_STRUCT mn_event_flags;
extern SYS_EVENT_STRUCT SysEvents_flags;
extern uint16 manual_cal_flag;
extern uint16 manualCalSampleCount;
extern uint8 print_out_mode;
extern uint8 g_sampleProcessing;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);

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
extern uint16* gp_bg430DataStart;
extern uint16* gp_bg430DataWrite;
extern uint16* gp_bg430DataRead;
extern uint16* gp_bg430DataEnd;

// ns8100
extern FL_FILE* gCurrentEventFileHandle;
extern uint16 g_currentEventNumber;
extern uint16* gCurrentEventStartPtr;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------

//*****************************************************************************
// Function:	ProcessWaveformData (Step 2)
// Purpose :	Copy A/D channel data from pretrigger into event buffers and
//				check for command nibble				
//*****************************************************************************
void ProcessWaveformData(void)
{
	//SUMMARY_DATA* sumEntry;
	static uint16 commandNibble;

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
#if 0 // ns8100 - Moved to handle only in the ISR
				case TRIG_THREE:
				case TRIG_TWO:
				case TRIG_ONE:
					//debugRaw("\nEvtBeg\n");
					
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
#endif
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
						tailOfPreTrigBuff += 4;

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
							tailOfPreTrigBuff += 4;
							
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
							tailOfPreTrigBuff += 4;
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
							tailOfPreTrigBuff += 4;
							break;
					}   
					break;

				default:
					// Advance the PreTrigger buffer the number of active channels
					tailOfPreTrigBuff += gp_SensorInfo->numOfChannels;
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
					tailOfPreTrigBuff += 4;
          
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
					tailOfPreTrigBuff += 4;
          
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
					tailOfPreTrigBuff += 4;

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
#if 0 // ns8100 - Moved to handle only in the ISR
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
#endif

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
			tailOfPreTrigBuff += 4;
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
			tailOfPreTrigBuff += 4;
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
			//debugRaw("\nEvtEnd\n");
			
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
// Function:	MoveWaveformEventToFlash
// Purpose :
//*****************************************************************************
void MoveWaveformEventToFlash(void)
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
				if (help_rec.flash_wrapping == NO)
				{
					getFlashUsageStats(&flashStats);
					
					if (flashStats.waveEventsLeft == 0)
					{
						// Release the buffer (in essence)
						gFreeEventBuffers++;
						
						// Can't store event, just return
						return;
					}
				}

				if (GetRamSummaryEntry(&ramSummaryEntry) == FALSE)
				{
					debugErr("Out of Flash Summary Entrys\n");
				}

#if 0 // ns7100
				// Set our flash event data pointer to the start of the flash event
				advFlashDataPtrToEventData(ramSummaryEntry);
#endif

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
					//debugRaw("\nEvent Pretrigger --> ");
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

				// Store entire sample
				//debugRaw("\nEvent Data --> ");
				//storeData(gCurrentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);
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

					// Store entire sample
					//debugRaw("\nEvent Data --> ");
					//storeData(gCurrentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);
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
					// Store entire sample
					//debugRaw("\nEvent Cal --> ");
					//storeData(gCurrentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);

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
					debugErr("Failed to get a new file handle for the current Waveform event!\n");
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
