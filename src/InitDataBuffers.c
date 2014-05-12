///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: InitDataBuffers.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:49 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/InitDataBuffers.c,v $
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
#include "ProcessCombo.h"
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

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
MSGS430_UNION msgs430;
uint16 preTrigBuff[PRE_TRIG_BUFF_SIZE_IN_WORDS];
uint16* startOfPreTrigBuff;
uint16* tailOfPreTrigBuff;
uint16* endOfPreTrigBuff;
uint16 gMaxEventBuffers;
uint16 gCurrentEventNumber;
FL_FILE* gCurrentEventFileHandle;
FL_FILE* gComboDualCurrentEventFileHandle;
uint16 gFreeEventBuffers;
uint16 gCalTestExpected;
uint32 gSamplesInBody;
uint32 gSamplesInPre;
uint32 gSamplesInCal;
uint32 gSamplesInEvent;
uint32 gWordSizeInPre;
uint32 gWordSizeInBody;
uint32 gWordSizeInCal;
uint32 gWordSizeInEvent;
uint16* startOfEventBufferPtr;
uint16* gEventBufferPrePtr;
uint16* gEventBufferBodyPtr;
uint16* gEventBufferCalPtr;
uint16* dgEventBufferCalPtr;
uint16* ddgEventBufferCalPtr;
//uint16* chaTwoEvtPrePtr;
//uint16* chaTwoEvtBodyPtr;
//uint16* chaTwoEvtCalPtr;
//uint16* dChaTwoEvtCalPtr;
//uint16* ddChaTwoEvtCalPtr;
uint16* gCurrentEventSamplePtr;
uint16 gCurrentEventBuffer;
SUMMARY_DATA summaryTable[MAX_RAM_SUMMARYS];
SUMMARY_DATA* gLastCompDataSum;
uint32 isTriggered = 0;
uint32 processingCal = 0;
uint16 eventsNotCompressed = 0; 
uint16* gp_bg430DataStart;
uint16* gp_bg430DataWrite;
uint16* gp_bg430DataRead;
uint16* gp_bg430DataEnd;
uint8 g_powerNoiseFlag = PRINTER_OFF;
uint8 g_doneTakingEvents = NO;
// fix_ns8100 - No aligned correctly
uint16 deadSpaceFix;
uint16 eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS];

//*****************************************************************************
// Function:	InitDataBuffs
// Purpose:		Setup up buffers for the specific mode selected by the user
//*****************************************************************************
void InitDataBuffs(uint8 op_mode)
{ 
	uint32 preTriggerSize;
	uint32 sampleRate;
	
	if (op_mode == MANUAL_CAL_MODE)
	{
		// Set sample rate to 1024 since it's fixed in the 430 for manual calibrations
		sampleRate = 1024;
	}
	else // Waveform, Bargraph, Combo
	{
		sampleRate = trig_rec.trec.sample_rate;
	}

	initEventRecord(&g_RamEventRecord, op_mode);

	// Setup the pre-trigger buffer pointers
	startOfPreTrigBuff = &(preTrigBuff[0]);
	tailOfPreTrigBuff = &(preTrigBuff[0]);

	// Set the PreTrigger size in words, 1/4 sec @ sample rate * number of channels, plus 1 sample
	preTriggerSize = ((uint32)(sampleRate / 4) * gp_SensorInfo->numOfChannels) + gp_SensorInfo->numOfChannels;
	// Set up the end of the pre trigger buffer
	endOfPreTrigBuff = &(preTrigBuff[preTriggerSize]);

	if ((op_mode == WAVEFORM_MODE) || (op_mode == MANUAL_CAL_MODE))
	{
		// Calculate samples for each section and total event
		gSamplesInBody = (uint32)(sampleRate * trig_rec.trec.record_time);
		gSamplesInPre  = (uint32)(sampleRate / 4);
		gSamplesInCal  = (uint32)((sampleRate / 1024) * 100);
		gSamplesInEvent  = gSamplesInPre + gSamplesInBody + gSamplesInCal;

		// Calculate word size for each section and total event, since buffer is an array of words
		gWordSizeInPre = gSamplesInPre * gp_SensorInfo->numOfChannels;
		gWordSizeInBody = gSamplesInBody * gp_SensorInfo->numOfChannels;
		gWordSizeInCal = gSamplesInCal * gp_SensorInfo->numOfChannels;
		gWordSizeInEvent = gWordSizeInPre + gWordSizeInBody + gWordSizeInCal;

		// Calculate total event buffers available
		gMaxEventBuffers = (uint16)(EVENT_BUFF_SIZE_IN_WORDS / (gWordSizeInPre + gWordSizeInBody + gWordSizeInCal));
		gFreeEventBuffers = gMaxEventBuffers;

		// Init starting event buffer pointers
		startOfEventBufferPtr = &(eventDataBuffer[0]);
		gCurrentEventSamplePtr = startOfEventBufferPtr; 
		gCurrentEventBuffer = 0;
		gCurrentEventNumber = 0;

		gEventBufferPrePtr = startOfEventBufferPtr;
		gEventBufferBodyPtr = gEventBufferPrePtr + gWordSizeInPre;
		gEventBufferCalPtr = gEventBufferPrePtr + gWordSizeInPre + gWordSizeInBody;

		// Init flags
		isTriggered = 0;
		processingCal = 0;
		gCalTestExpected = 0;
		g_doneTakingEvents = NO;
	}
	else if (op_mode == BARGRAPH_MODE)
	{		
		gp_bg430DataStart = &(eventDataBuffer[0]);
		gp_bg430DataWrite = &(eventDataBuffer[0]);
		gp_bg430DataRead = &(eventDataBuffer[0]);
		gp_bg430DataEnd = &(eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS - 4]);

		StartNewBargraph();
	}
	else if (op_mode == COMBO_MODE)
	{		
		// Waveform init
		// Calculate samples for each section and total event
		gSamplesInBody = (uint32)(sampleRate * trig_rec.trec.record_time);
		gSamplesInPre  = (uint32)(sampleRate / 4);
		gSamplesInCal  = (uint32)((sampleRate / 1024) * 100);
		gSamplesInEvent  = gSamplesInPre + gSamplesInBody + gSamplesInCal;

		// Calculate word size for each section and total event, since buffer is an array of words
		gWordSizeInPre = gSamplesInPre * gp_SensorInfo->numOfChannels;
		gWordSizeInBody = gSamplesInBody * gp_SensorInfo->numOfChannels;
		gWordSizeInCal = gSamplesInCal * gp_SensorInfo->numOfChannels;
		gWordSizeInEvent = gWordSizeInPre + gWordSizeInBody + gWordSizeInCal;

		// Calculate total event buffers available
		gMaxEventBuffers = (uint16)((EVENT_BUFF_SIZE_IN_WORDS - BARGRAPH_BUFFER_SIZE_OFFSET) / (gWordSizeInPre + gWordSizeInBody + gWordSizeInCal));
		gFreeEventBuffers = gMaxEventBuffers;

		// Init starting event buffer pointers
		startOfEventBufferPtr = &(eventDataBuffer[BARGRAPH_BUFFER_SIZE_OFFSET]);
		gCurrentEventSamplePtr = startOfEventBufferPtr; 
		gCurrentEventBuffer = 0;
		gCurrentEventNumber = 0;

		gEventBufferPrePtr = startOfEventBufferPtr;
		gEventBufferBodyPtr = gEventBufferPrePtr + gWordSizeInPre;
		gEventBufferCalPtr = gEventBufferPrePtr + gWordSizeInPre + gWordSizeInBody;

		// Init flags
		isTriggered = 0;
		processingCal = 0;
		gCalTestExpected = 0;
		g_doneTakingEvents = NO;

		// Bargraph init
		gp_bg430DataStart = &(eventDataBuffer[0]);
		gp_bg430DataWrite = &(eventDataBuffer[0]);
		gp_bg430DataRead = &(eventDataBuffer[0]);
		gp_bg430DataEnd = &(eventDataBuffer[BARGRAPH_BUFFER_SIZE_OFFSET - 4]);

		StartNewCombo();
	}
}

//*****************************************************************************
// Function:	CalcSumFreq
// Purpose:		
//*****************************************************************************
uint16 CalcSumFreq(uint16* workingPtr, uint16 sampleRate)
{
	uint16* samplePtr;
	uint32 dataCount = 0;
	uint16 freq = 0;

	samplePtr = workingPtr;

	if ((*samplePtr & DATA_MASK) >= POS_VALID_PEAK)
	{
		while (((*samplePtr & DATA_MASK) >= POS_CROSSOVER_BACKWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_START))
		{
			samplePtr -= 4;
			dataCount++;
		}
    
		samplePtr = workingPtr;
		while (((*samplePtr & DATA_MASK) >= POS_CROSSOVER_FORWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_END))
		{
			samplePtr += 4;
			dataCount++;
		}
	}
	else if ((*samplePtr & DATA_MASK) <= NEG_VALID_PEAK)
	{
		while (((*samplePtr & DATA_MASK) <= NEG_CROSSOVER_BACKWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_START))
		{
			samplePtr -= 4;
			dataCount++;
		}
    
		samplePtr = workingPtr;
		while (((*samplePtr & DATA_MASK) <= NEG_CROSSOVER_FORWARD) && ((*samplePtr & EMBEDDED_CMD) != EVENT_END))
		{
			samplePtr += 4;
			dataCount++;
		}
	}
	else
	{
		// Peak was less than 4 counts, assume to be noise
		return ((uint16)0);
	}
  
	// total counts between 0 crossings
	dataCount = (uint32)((dataCount - 1) * 2);

	// Whole part of freq shifted to make one decimal place.
	if (dataCount > 0)
	{
		freq = (uint16)(((float)(sampleRate * 10) / (float)dataCount));
	}

	return (freq);
}

//*****************************************************************************
// FUNCTION:	uint16 FixDataToZero(uint16 data_)
// DESCRIPTION
//*****************************************************************************
uint16 FixDataToZero(uint16 data_)
{
   if (data_ > 0x800)
     data_ = (uint16)(data_ - 0x0800);
   else
     data_ = (uint16)(0x0800 - data_);
     
   return (data_);  
}
