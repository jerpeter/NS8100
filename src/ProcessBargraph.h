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
#define NUM_OF_BAR_INTERVAL_BUFFERS			(3600 + 1) // Max Bars = 1 sec bars for 1 hour (3600 seconds) + 1 spare
#define NUM_OF_SUM_INTERVAL_BUFFERS			1
#define NUM_OF_BAR_INTERVALS_TO_HOLD		60
#define NUM_OF_SUMMARY_INTERVALS_TO_HOLD	0

#define SUMMARY_INTERVAL_SIZE_IN_BYTES 	sizeof(CALCULATED_DATA_STRUCT)
#define SUMMARY_INTERVAL_SIZE_IN_WORDS 	((SUMMARY_INTERVAL_SIZE_IN_BYTES + 1) / 2)

#define BG_DATA_BUFFER_SIZE 			SAMPLE_RATE_8K * 4 * 60

#define COMBO_MODE_BAR_INTERVAL_SIZE		(NUM_OF_BAR_INTERVAL_BUFFERS * sizeof(BARGRAPH_BAR_INTERVAL_DATA))
#define COMBO_MODE_SUMMARY_INTERVAL_SIZE	(NUM_OF_SUM_INTERVAL_BUFFERS * sizeof(CALCULATED_DATA_STRUCT))
#define COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET	(COMBO_MODE_BAR_INTERVAL_SIZE + COMBO_MODE_SUMMARY_INTERVAL_SIZE + BG_DATA_BUFFER_SIZE)
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

enum{
	BARPGRAPH_SESSION_COMPLETE = 1,
	BARGRAPH_SESSION_IN_PROGRESS
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void StartNewBargraph(void);
void UpdateBargraphJobTotals(void);
void EndBargraph(void);
void MoveBarIntervalDataToFile(void);
void MoveSummaryIntervalDataToFile(void);
void CompleteSummaryInterval(void);
void MoveStartOfBargraphEventRecordToFile(void);
void MoveUpdatedBargraphEventRecordToFile(uint8 status);
void AdvanceBarIntervalBufPtr(uint8);
uint8 CalculateBargraphData(void);

#endif //_DATABUFFS_H_
