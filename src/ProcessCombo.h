///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _PROCESS_COMBO_H_
#define _PROCESS_COMBO_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Summary.h"
#include "ProcessBargraph.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void StartNewCombo(void);
void ProcessComboData(void);
void ProcessComboDataSkipBargraphDuringCal(void);
uint8 CalculateComboData(void);
void ProcessComboSampleData(void) ;
void ProcessComboBargraphData(void); 
void UpdateComboJobTotals(CALCULATED_DATA_STRUCT *);
void EndCombo(void);

uint32 moveComboBarIntervalDataToFile(void);
void moveComboSummaryIntervalDataToFile(void);
void MoveComboWaveformEventToFile(void);
void MoveStartOfComboEventRecordToFile(void);
void MoveEndOfComboEventRecordToFile(void);

void advanceComboBarIntervalBufPtr(uint8);
void advanceComboSumIntervalBufPtr(uint8);

#if 0 // Unused
BOOLEAN checkSpaceForComboBarSummaryInterval(void);
#endif

#endif //_DATABUFFS_H_
