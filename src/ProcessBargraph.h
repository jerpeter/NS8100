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
// Bargraph Bar Interval ranges
#define ONE_SEC_PRD 	 1
#define TEN_SEC_PRD 	10
#define TWENTY_SEC_PRD 	20
#define THIRTY_SEC_PRD 	30
#define FOURTY_SEC_PRD 	40
#define FIFTY_SEC_PRD 	50
#define SIXTY_SEC_PRD 	60

// Bargraph Summary Interval ranges
#define FIVE_MINUTE_INTVL	 		300
#define FIFTEEN_MINUTE_INTVL		900
#define THIRTY_MINUTE_INTVL			1800
#define ONE_HOUR_INTVL				3600
#define TWO_HOUR_INTVL				7200
#define FOUR_HOUR_INTVL				14400
#define EIGHT_HOUR_INTVL			28800
#define TWELVE_HOUR_INTVL			43200

#define MAX_NUM_OF_BAR_INTERVAL_BUFFERS			((TWELVE_HOUR_INTVL / ONE_SEC_PRD) + 1) // Max Bars possible per Summary + 1 spare

#define COMBO_BG_DATA_BUFFER_SIZE 				(SAMPLE_RATE_4K * (sizeof(SAMPLE_DATA_STRUCT)) * 60) // Cache up to 1 minute of data at max rate
#define COMBO_MODE_BAR_INTERVAL_SIZE			(MAX_NUM_OF_BAR_INTERVAL_BUFFERS * sizeof(BARGRAPH_BAR_INTERVAL_DATA))
#define COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET	(COMBO_MODE_BAR_INTERVAL_SIZE + COMBO_BG_DATA_BUFFER_SIZE)
#define COMBO_MODE_BARGRAPH_BUFFER_SIZE_WORDS	(((COMBO_MODE_BARGRAPH_BUFFER_SIZE_OFFSET / sizeof(SAMPLE_DATA_STRUCT)) * sizeof(SAMPLE_DATA_STRUCT)) / 2)

#define BAR_INTERVAL_ORIGINAL_DATA_TYPE_SIZE			8
#define BAR_INTERVAL_A_R_V_T_DATA_TYPE_SIZE				12
#define BAR_INTERVAL_A_R_V_T_WITH_FREQ_DATA_TYPE_SIZE	20

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

// Using a key within the data set to best fit the design for asynchronously identifying end of a Bar Interval
// Picked a pattern of alternating 1's and 0's, with the possibility of all 4 channels randomly being equal to this key at 1 chance in 18,446,744,073,709,551,616
#define BAR_INTERVAL_END_KEY_SAMPLE		{0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA}
#define BAR_INTERVAL_END_KEY			0xAAAA

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
