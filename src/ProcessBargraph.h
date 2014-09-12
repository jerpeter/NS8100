///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _PROCESS_BARGRAPH_H_
#define _PROCESS_BARGRAPH_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Summary.h"
#include "InitDataBuffers.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define NUM_OF_BAR_INTERVAL_BUFFERS		40
#define NUM_OF_SUM_INTERVAL_BUFFERS		4

#define MAX_BARGRAPH_HOURS 				24
#define MAX_SAMPLES_PER_HOUR 			600

#define SUMMARY_INTERVAL_SIZE_IN_BYTES 	sizeof(CALCULATED_DATA_STRUCT)
#define SUMMARY_INTERVAL_SIZE_IN_WORDS 	((SUMMARY_INTERVAL_SIZE_IN_BYTES + 1) / 2)

#define BG_DATA_BUFFER_SIZE 			SAMPLE_RATE_8K * 4 * 60

#define COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET	(((NUM_OF_BAR_INTERVAL_BUFFERS + NUM_OF_SUM_INTERVAL_BUFFERS) * sizeof(CALCULATED_DATA_STRUCT)) + BG_DATA_BUFFER_SIZE)
#define COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS	(((COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET / sizeof(SAMPLE_DATA_STRUCT)) * sizeof(SAMPLE_DATA_STRUCT)) / 2)

// Check if the current ptr goes past the end of the event(ram) buffer.
// If it does go past, start at the top of the ram buffer.
#define END_OF_EVENT_BUFFER_CHECK(CURRENT, START, END)	\
	if (CURRENT >= END) CURRENT = START
	
enum // Set unique values to the following types (actual value doesn't matter)
{
	READ_PTR,
	WRITE_PTR,
	BG_BUFFER_EMPTY,
	BG_BUFFER_NOT_EMPTY
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void StartNewBargraph(void);
void ProcessBargraphData(void);
uint8 CalculateBargraphData(void);
void UpdateBargraphJobTotals(CALCULATED_DATA_STRUCT *);
void EndBargraph(void);

uint32 MoveBarIntervalDataToFile(void);
void MoveSummaryIntervalDataToFile(void);

void MoveStartOfBargraphEventRecordToFlash(void);
void MoveEndOfBargraphEventRecordToFlash(void);

void AdvanceBarIntervalBufPtr(uint8);
void AdvanceSumIntervalBufPtr(uint8);

BOOLEAN CheckSpaceForBarSummaryInterval(void);

#endif //_DATABUFFS_H_
