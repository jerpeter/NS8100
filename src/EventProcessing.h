///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: EventProcessing.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:47 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/EventProcessing.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _FLASHEVTS_H_
#define _FLASHEVTS_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Summary.h"
#include "Flash.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
// All address in words
//#define FLASH_BASE_ADDR          0x80000000
//#define FLASH_WORD_SIZE          0x0007FFFF
//#define FLASH_BYTE_SIZE          0x00100000

//#define BOOT_SECTOR_WORD_SIZE    0x00001000
//#define BOOT_SECTOR_BYTE_SIZE    0x00002000

//#define SA0_ADDR                 0x00000000
//#if 1 // Original
//#define SA1_ADDR                 0x00004000
//#define SA2_ADDR                 0x00006000
//#define SA3_ADDR                 0x00008000
//#else // Updated, but not fully tested
//#define SA1_ADDR                 0x00002000 // was 0x00004000
//#define SA2_ADDR                 0x00004000 // was 0x00006000
//#define SA3_ADDR                 0x00006000 // was 0x00008000
//#endif

// Updated (based on a 4MB flash part, with 63 data sectors(not including boot sectors))
//#define MAX_SAX                  63

//#define SAX_ADDR                 0x00010000
//#define SAX_WRD_SIZE             0x00008000
//#define SAX_BYTE_SIZE            0x00010000

#define FLASH_FULL_FLAG  			0x0000
#define EVENT_RECORD_START_FLAG		0xA55A
#define EVENT_RECORD_VERSION		0x0101
#define EVENT_MAJOR_VERSION_MASK	0xFF00

#define FIRST_FLASH_EVENT_DATA_SECTOR	0
#define TOTAL_FLASH_EVENT_DATA_SECTORS	(TOTAL_FLASH_DATA_SECTORS - FIRST_FLASH_EVENT_DATA_SECTOR)

#define LAST_RAM_SUMMARY_INDEX 			799 // Last index
#define TOTAL_RAM_SUMMARIES 			(LAST_RAM_SUMMARY_INDEX + 1) // 800 total, was 925 total

// Defines
#define FLASH_EVENT_START (FLASH_BASE_ADDR + FLASH_BOOT_SIZE_x8 + (FIRST_FLASH_EVENT_DATA_SECTOR * FLASH_SECTOR_SIZE_x8))
#define FLASH_EVENT_END   (FLASH_EVENT_START + (TOTAL_FLASH_EVENT_DATA_SECTORS * FLASH_SECTOR_SIZE_x8))

#define FLASH_EVENT_START_BOUNDRY_UPDATE(flashPtr)		\
	if (flashPtr <  (uint16*)FLASH_EVENT_START)			\
		flashPtr = (uint16*)((uint32)FLASH_EVENT_END - 	\
			(uint32)FLASH_EVENT_START - (uint32)flashPtr)

#define FLASH_EVENT_END_BOUNDRY_UPDATE(flashPtr) 							\
	if (flashPtr >=  (uint16*)FLASH_EVENT_END)								\
	{	flashPtr = (uint16*)((uint32)flashPtr - (uint32)FLASH_EVENT_END); 	\
		flashPtr = (uint16*)((uint32)FLASH_EVENT_START + (uint32)flashPtr); }

/*
typedef struct
{
	uint16 startFlag;
	uint16 recordVersion;
	uint16 headerLength;
	uint16 summaryLength;
	uint32 dataLength;
	uint16 dataCompression;
	uint16 summaryChecksum;
	uint16 dataChecksum;
	uint16 unused1;
	uint16 unused2;
	uint16 unused3;
} EVENT_HEADER_STRUCT;

typedef struct
{
	VERSION_INFO_STRUCT		version;
	PARAMETERS_STRUCT		parameters;
	CAPTURE_INFO_STRUCT		captured;
	CALCULATED_DATA_STRUCT	calculated;
	uint16 					eventNumber;
	uint8					mode;
	uint8					unused;
} EVENT_SUMMARY_STRUCT;

typedef struct
{
	EVENT_HEADER_STRUCT 		header;
	EVENT_SUMMARY_STRUCT		summary;
} EVT_RECORD;
*/

typedef struct
{
	uint32 sizeUsed;
	uint32 sizeFree;
	uint16 waveEventsLeft;
	uint16 barHoursLeft;
	uint16 manualCalsLeft;
	uint8 percentUsed;
	uint8 percentFree;
	BOOL wrapped;
	BOOL roomForBargraph;
} FLASH_USAGE_STRUCT;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void initRamSummaryTbl(void);
void copyValidFlashEventSummariesToRam(void);
void condenseRamSummaryTable(void);
void advFlashDataPtrToEventData(SUMMARY_DATA*);
uint8 InitFlashEvtBuff(void);
void InitFlashBuffs(void);
void initEventRecord(EVT_RECORD*, uint8 op_mode);
void initCurrentEventNumber(void);
uint16 getLastStoredEventNumber(void);
void storeCurrentEventNumber(void);
uint16 getUniqueEventNumber(SUMMARY_DATA* currentSummary);
uint16 GetFlashSumEntry(SUMMARY_DATA** sumEntryPtr);
void FillInFlashSummarys(SUMMARY_DATA* , SUMMARY_DATA*);
void ReclaimSpace(uint16* sectorAddr);
uint16* getFlashDataPointer(void);
void checkFlashDataPointer(void);
void storeData(uint16* dataPtr, uint16 dataWords);
FLASH_USAGE_STRUCT getFlashUsageStats(void);

#endif // _FLASHEVTS_H_
