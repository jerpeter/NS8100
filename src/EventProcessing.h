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
#include "FAT32_FileLib.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
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

typedef struct
{
	uint32 sizeUsed;
	uint32 sizeFree;
	uint16 waveEventsLeft;
	uint16 barHoursLeft;
	uint16 manualCalsLeft;
	uint8 percentUsed;
	uint8 percentFree;
	BOOLEAN wrapped;
	BOOLEAN roomForBargraph;
} FLASH_USAGE_STRUCT;

typedef enum {
	CREATE_EVENT_FILE,
	READ_EVENT_FILE,
	APPEND_EVENT_FILE	
} EVENT_FILE_OPTION;
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
uint16 GetRamSummaryEntry(SUMMARY_DATA** sumEntryPtr);
void completeRamEventSummary(SUMMARY_DATA* , SUMMARY_DATA*);
void ReclaimSpace(uint16* sectorAddr);
uint16* getFlashDataPointer(void);
void checkFlashDataPointer(void);
void storeData(uint16* dataPtr, uint16 dataWords);
void getFlashUsageStats(FLASH_USAGE_STRUCT* usage);
void getEventFileInfo(uint16 eventNumber, EVENT_HEADER_STRUCT* eventHeaderPtr, EVENT_SUMMARY_STRUCT* eventSummaryPtr, BOOLEAN cacheDataToRamBuffer);
void getEventFileRecord(uint16 eventNumber, EVT_RECORD* tempEventRecord);
void cacheEventDataToRam(uint16 eventNumber, uint32 dataSize);
BOOLEAN validEventFile(uint16 eventNumber);
FL_FILE* getEventFileHandle(uint16 eventNumber, EVENT_FILE_OPTION option);
void deleteEventFileRecord(uint16 eventNumber);
void deleteEventFileRecords(void);

#endif // _FLASHEVTS_H_
