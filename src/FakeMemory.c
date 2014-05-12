#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Mmc2114.h"
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
#include "Rec.h"
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
  
uint32 __monitorLogTblKey;
uint16 __monitorLogTblIndex;
uint16 __monitorLogUniqueEntryId;
MONITOR_LOG_ENTRY_STRUCT __monitorLogTbl[TOTAL_MONITOR_LOG_ENTRIES];
uint32 __autoDialoutTblKey;
AUTODIALOUT_STRUCT __autoDialoutTbl;
uint32 __ramFlashSummaryTblKey;
SUMMARY_DATA __ramFlashSummaryTbl[TOTAL_RAM_SUMMARIES];

//uint32 __data_in_RAM_begin[1];
//uint32 __data_in_ROM_begin[1];

//uint32 __SPI_CONTROL_ONE_REG_ADDR[1];
//uint32 __SPI_CONTROL_TWO_REG_ADDR[1];
//uint32 __SPI_BAUD_RATE_REG_ADDR[1];
//uint32 __SPI_STATUS_REG_ADDR[1];
//uint32 __SPI_DATA_REG_ADDR[1];
//uint32 __SPI_PULLUP_REDUCED_DRV_REG_ADDR[1];
//uint32 __SPI_PORT_DATA_REG_ADDR[1];
//uint32 __SPI_PORT_DATA_DIR_REG_ADDR[1];

//uint32 __internal_ram[1024];
//uint32 __stack_end[1];
//uint32 __stack_begin[1];
//uint32 __heap_addr[1];
//uint32 __heap_end[1];
//uint32 __heap_size[1];
//uint32 __data_size[1];
//uint32 __data_begin[1];
//uint32 __data_ROM_begin[1];

#if 1
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
