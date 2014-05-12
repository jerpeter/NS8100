///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: ProcessManualCal.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:57 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ProcessManualCal.c,v $
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
#include "FAT32_FileLib.h"

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
extern uint16* startOfEventBufferPtr;
extern uint16* gEventBufferPrePtr;
//extern uint16* chaTwoEvtPrePtr;
extern uint16* gEventBufferBodyPtr;
//extern uint16* chaTwoEvtBodyPtr;
extern uint16* gEventBufferCalPtr;
//extern uint16* chaTwoEvtCalPtr;
extern uint16* dgEventBufferCalPtr;
//extern uint16* dChaTwoEvtCalPtr;
extern uint16* ddgEventBufferCalPtr;
//extern uint16* ddChaTwoEvtCalPtr;
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

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------

//*****************************************************************************
// Function:	ProcessManuelCalPulse
// Purpose:		
//*****************************************************************************
void ProcessManuelCalPulse(void)
{
	DATE_TIME_STRUCT triggerTimeStamp;
	//SUMMARY_DATA* sumEntry;

	static uint8 manuelCalInProgess = FALSE;

	switch (*tailOfPreTrigBuff & EMBEDDED_CMD)
	{
		case CAL_START:
			manuelCalInProgess = TRUE;
			if (gFreeEventBuffers != 0)
			{
				g_RamEventRecord.summary.captured.eventTime = triggerTimeStamp = getCurrentTime();

				summaryTable[gCurrentEventNumber].linkPtr = gEventBufferPrePtr;

				*(gEventBufferPrePtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_START);
				*(gEventBufferPrePtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_START);
				*(gEventBufferPrePtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_START);
				*(gEventBufferPrePtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_START);

				gEventBufferPrePtr += 4;
				tailOfPreTrigBuff += 4;
			}
			break;

		case CAL_END:
			manuelCalInProgess = FALSE;

			*(gEventBufferPrePtr + 0) = (uint16)((*(tailOfPreTrigBuff + 0) & DATA_MASK) | EVENT_END);
			*(gEventBufferPrePtr + 1) = (uint16)((*(tailOfPreTrigBuff + 1) & DATA_MASK) | EVENT_END);
			*(gEventBufferPrePtr + 2) = (uint16)((*(tailOfPreTrigBuff + 2) & DATA_MASK) | EVENT_END);
			*(gEventBufferPrePtr + 3) = (uint16)((*(tailOfPreTrigBuff + 3) & DATA_MASK) | EVENT_END);

			gEventBufferPrePtr += 4;
			tailOfPreTrigBuff += 4;
			
			raiseSystemEventFlag(MANUEL_CAL_EVENT);
			manual_cal_flag = FALSE;
			manualCalSampleCount = 0;
			break;

		default:
			if (manuelCalInProgess == TRUE)
			{
				*(gEventBufferPrePtr + 0) = *(tailOfPreTrigBuff + 0);
				*(gEventBufferPrePtr + 1) = *(tailOfPreTrigBuff + 1);
				*(gEventBufferPrePtr + 2) = *(tailOfPreTrigBuff + 2);
				*(gEventBufferPrePtr + 3) = *(tailOfPreTrigBuff + 3);

				gEventBufferPrePtr += 4;
				tailOfPreTrigBuff += 4;
			}
			else
			{
				tailOfPreTrigBuff += gp_SensorInfo->numOfChannels;
			}
			break;
	}
  
	if (manual_cal_flag == FALSE)
	{
		gFreeEventBuffers--;
		gCurrentEventNumber++;
		if (gCurrentEventNumber < gMaxEventBuffers)
		{
				gEventBufferPrePtr = gEventBufferBodyPtr + gWordSizeInCal;
				gEventBufferBodyPtr = gEventBufferPrePtr + gWordSizeInPre;
		}
		else
		{
			gCurrentEventNumber = 0;
			gEventBufferPrePtr = startOfEventBufferPtr;
			gEventBufferBodyPtr = startOfEventBufferPtr + gWordSizeInPre;
		}
	}  

	// Check if the end of the PreTrigger buffer has been reached
	if (tailOfPreTrigBuff >= endOfPreTrigBuff) tailOfPreTrigBuff = startOfPreTrigBuff;
}

//*****************************************************************************
// Function:	MoveManuelCalToFlash
// Purpose:
//*****************************************************************************
void MoveManuelCalToFlash(void)
{
	static SUMMARY_DATA* sumEntry;
	static SUMMARY_DATA* flashSumEntry;
	uint16 i;
	uint16 sample;
	uint16 normalizedData;
	uint16 hiA = 0, hiR = 0, hiV = 0, hiT = 0;
	uint16 lowA = 0xFFF, lowR = 0xFFF, lowV = 0xFFF, lowT = 0xFFF;

	if (gFreeEventBuffers < gMaxEventBuffers)
	{
		if (GetFlashSumEntry(&flashSumEntry) == FALSE)
		{
			debugErr("Out of Flash Summary Entrys\n");
		}

#if 0 // ns7100
		// Set our flash event data pointer to the start of the flash event
		advFlashDataPtrToEventData(flashSumEntry);
#endif

#if 1 // ns8100
		// Get new file handle
		gCurrentEventFileHandle = getNewEventFileHandle(g_currentEventNumber);
				
		if (gCurrentEventFileHandle == NULL)
			debugErr("Failed to get a new file handle for the current Waveform event!\n");

		// Seek to beginning of where data will be stored in event
		fl_fseek(gCurrentEventFileHandle, sizeof(EVT_RECORD), SEEK_SET);
#endif

		sumEntry = &summaryTable[gCurrentEventBuffer];
		sumEntry->mode = MANUAL_CAL_MODE;

		// Initialize the freq data counts.
		sumEntry->waveShapeData.a.freq = 0;
		sumEntry->waveShapeData.r.freq = 0;
		sumEntry->waveShapeData.v.freq = 0;
		sumEntry->waveShapeData.t.freq = 0;

		for (i = (uint16)gSamplesInCal; i != 0; i--)
		{
			//=========================================================
			// First channel - A
			sample = (uint16)((*(gCurrentEventSamplePtr + 0)) & DATA_MASK);

			if (sample > hiA) hiA = sample;
			if (sample < lowA) lowA = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.a.peak)
			{
				sumEntry->waveShapeData.a.peak = normalizedData;
				sumEntry->waveShapeData.a.peakPtr = (gCurrentEventSamplePtr + 0);
			}

			//=========================================================
			// Second channel - R
			sample = (uint16)((*(gCurrentEventSamplePtr + 1)) & DATA_MASK);

			if (sample > hiR) hiR = sample;
			if (sample < lowR) lowR = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.r.peak)
			{
				sumEntry->waveShapeData.r.peak = normalizedData;
				sumEntry->waveShapeData.r.peakPtr = (gCurrentEventSamplePtr + 1);
			}

			//=========================================================
			// Third channel - V
			sample = (uint16)((*(gCurrentEventSamplePtr + 2)) & DATA_MASK);

			if (sample > hiV) hiV = sample;
			if (sample < lowV) lowV = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.v.peak)
			{
				sumEntry->waveShapeData.v.peak = normalizedData;
				sumEntry->waveShapeData.v.peakPtr = (gCurrentEventSamplePtr + 2);
			}

			//=========================================================
			// Fourth channel - T
			sample = (uint16)((*(gCurrentEventSamplePtr + 3)) & DATA_MASK);

			if (sample > hiT) hiT = sample;
			if (sample < lowT) lowT = sample;

			normalizedData = FixDataToZero(sample);

			if (normalizedData > sumEntry->waveShapeData.t.peak)
			{
				sumEntry->waveShapeData.t.peak = normalizedData;
				sumEntry->waveShapeData.t.peakPtr = (gCurrentEventSamplePtr + 3);
			}

			// Store entire sample
#if 0 // ns7100
			storeData(gCurrentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT);
#else // ns8100
			fl_fwrite(gCurrentEventSamplePtr, NUMBER_OF_CHANNELS_DEFAULT, 2, gCurrentEventFileHandle);
#endif
			
			gCurrentEventSamplePtr += NUMBER_OF_CHANNELS_DEFAULT;
		}

		if (++gCurrentEventBuffer == gMaxEventBuffers)
		{
			gCurrentEventBuffer = 0;
			gCurrentEventSamplePtr = startOfEventBufferPtr;
		}
		else
		{
			gCurrentEventSamplePtr = startOfEventBufferPtr + (gCurrentEventBuffer * gWordSizeInEvent);
		}

		sumEntry->waveShapeData.a.peak = (uint16)(hiA - lowA + 1);
		sumEntry->waveShapeData.r.peak = (uint16)(hiR - lowR + 1);
		sumEntry->waveShapeData.v.peak = (uint16)(hiV - lowV + 1);
		sumEntry->waveShapeData.t.peak = (uint16)(hiT - lowT + 1);

		sumEntry->waveShapeData.a.freq = CalcSumFreq(sumEntry->waveShapeData.a.peakPtr, 1024);
		sumEntry->waveShapeData.r.freq = CalcSumFreq(sumEntry->waveShapeData.r.peakPtr, 1024);   
		sumEntry->waveShapeData.v.freq = CalcSumFreq(sumEntry->waveShapeData.v.peakPtr, 1024);   
		sumEntry->waveShapeData.t.freq = CalcSumFreq(sumEntry->waveShapeData.t.peakPtr, 1024);       

		FillInFlashSummarys(flashSumEntry, sumEntry);

		clearSystemEventFlag(MANUEL_CAL_EVENT);

		gLastCompDataSum = flashSumEntry;

		// Set printout mode to allow the results menu processing to know this is a manual cal pulse
		print_out_mode = MANUAL_CAL_MODE;
		raiseMenuEventFlag(RESULTS_MENU_EVENT);

		gFreeEventBuffers++;
	}
	else
	{
		clearSystemEventFlag(MANUEL_CAL_EVENT);
	}
}
