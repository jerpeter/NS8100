///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#if 0
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
uint8 CalculateComboData(void);
void UpdateComboJobTotals(void);
void EndCombo(void);
void CompleteComboSummaryInterval(void);
void MoveComboBarIntervalDataToFile(void);
void MoveComboSummaryIntervalDataToFile(void);
void MoveComboWaveformEventToFile(void);
void MoveStartOfComboEventRecordToFile(void);
void MoveEndOfComboEventRecordToFile(void);
void AdvanceComboBarIntervalBufPtr(uint8);

#endif //_DATABUFFS_H_
#endif