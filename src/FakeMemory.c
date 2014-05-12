///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#if 0

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Common.h"
#include "Uart.h"
#include "Display.h"
#include "Menu.h"
#include "SoftTimer.h"
#include "PowerManagement.h"
#include "Keypad.h"
#include "SysEvents.h"
#include "Board.h"
#include "TextTypes.h"
#include "Record.h"
#include "EventProcessing.h"

// Carving up TABLE segment
/*
__monitorLogTblKey = 0x81800000;
__monitorLogTblIndex = 0x81800004;
__monitorLogUniqueEntryId = 0x81800006;
__monitorLogTbl = 0x81800008;
__autoDialoutTblKey = 0x81800FE8;
__autoDialoutTbl = 0x81800FEC;
__ramFlashSummaryTblKey = 0x81801000;
__ramFlashSummaryTbl = 0x81801004;
*/

#if 0
int ReadPSR(void)
{
	uint8 i = 0;

	i+=i;

	return i;
}

void WritePSR(int crap)
{
	uint8 i = 0;

	i+=crap;
}

void Wait(void)
{
	uint8 i = 0;

	i+=i;
}
#endif

#endif