///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: ProcessCombo.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:56 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/ProcessCombo.h,v $
///	$Revision: 1.2 $
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
