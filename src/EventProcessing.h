///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _FLASHEVTS_H_
#define _FLASHEVTS_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Summary.h"
#include "FAT32_FileLib.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define FLASH_FULL_FLAG				0x0000
#define EVENT_RECORD_START_FLAG		0xA55A
#define EVENT_RECORD_VERSION		0x0101
#define EVENT_MAJOR_VERSION_MASK	0xFF00

#define FIRST_FLASH_EVENT_DATA_SECTOR	0
#define TOTAL_FLASH_EVENT_DATA_SECTORS	(TOTAL_FLASH_DATA_SECTORS - FIRST_FLASH_EVENT_DATA_SECTOR)

#define LAST_RAM_SUMMARY_INDEX 			799 // Last index
#define TOTAL_RAM_SUMMARIES 			(LAST_RAM_SUMMARY_INDEX + 1) // 800 total, was 925 total

// Defines
#define FLASH_EVENT_START	(FLASH_BASE_ADDR + FLASH_BOOT_SIZE_x8 + (FIRST_FLASH_EVENT_DATA_SECTOR * FLASH_SECTOR_SIZE_x8))
#define FLASH_EVENT_END		(FLASH_EVENT_START + (TOTAL_FLASH_EVENT_DATA_SECTORS * FLASH_SECTOR_SIZE_x8))

#define FLASH_EVENT_START_BOUNDRY_UPDATE(flashPtr)		\
	if (flashPtr < (uint16*)FLASH_EVENT_START)			\
		flashPtr = (uint16*)((uint32)FLASH_EVENT_END - 	\
			(uint32)FLASH_EVENT_START - (uint32)flashPtr)

#define FLASH_EVENT_END_BOUNDRY_UPDATE(flashPtr) 							\
	if (flashPtr >= (uint16*)FLASH_EVENT_END)								\
	{	flashPtr = (uint16*)((uint32)flashPtr - (uint32)FLASH_EVENT_END); 	\
		flashPtr = (uint16*)((uint32)FLASH_EVENT_START + (uint32)flashPtr); }

enum {
	EVENT_CACHE_FAILURE = 0,
	EVENT_CACHE_SUCCESS,
};

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
	//CREATE_EVENT_FILE_WITH_OVERWRITE,
	READ_EVENT_FILE,
	APPEND_EVENT_FILE,
	OVERWRITE_EVENT_FILE
} EVENT_FILE_OPTION;

typedef struct
{
	int16 file;
	uint16 totalEntries;
	uint16 validEntries;
	uint16 deletedEntries;
	uint16 currentEntryIndex;
	uint32 currentMonitorSessionStartSummary;
	SUMMARY_LIST_ENTRY_STRUCT cachedEntry;
} SUMMARY_LIST_FILE_DETAILS;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitRamSummaryTbl(void);
void CopyValidFlashEventSummariesToRam(void);
void CondenseRamSummaryTable(void);
uint8 InitFlashEvtBuff(void);
void InitFlashBuffs(void);
void InitEventRecord(uint8 opMode);
void InitCurrentEventNumber(void);
uint16 GetLastStoredEventNumber(void);
void StoreCurrentEventNumber(void);
void IncrementCurrentEventNumber(void);
uint16 GetUniqueEventNumber(SUMMARY_DATA* currentSummary);
uint16 GetRamSummaryEntry(SUMMARY_DATA** sumEntryPtr);
void CompleteRamEventSummary(SUMMARY_DATA* ramSummaryPtr);
uint16* GetFlashDataPointer(void);
void StoreData(uint16* dataPtr, uint16 dataWords);
void GetSDCardUsageStats(void);
void UpdateSDCardUsageStats(uint32 removeSize);
void GetEventFileInfo(uint16 eventNumber, EVENT_HEADER_STRUCT* eventHeaderPtr, EVENT_SUMMARY_STRUCT* eventSummaryPtr, BOOLEAN cacheDataToRamBuffer);
void GetEventFileRecord(uint16 eventNumber, EVT_RECORD* tempEventRecord);
void CacheEventDataToBuffer(uint16 eventNumber, uint8* dataBuffer, uint32 dataOffset, uint32 dataSize);
uint32 GetERDataSize(uint16 eventNumber);
void CacheERDataToBuffer(uint16 eventNumber, uint8* dataBuffer, uint32 dataOffset, uint32 dataSize);
void CacheEventDataToRam(uint16 eventNumber, uint32 dataSize);
uint8 CacheEventToRam(uint16 eventNumber, EVT_RECORD* eventRecordPtr);
BOOLEAN CheckValidEventFile(uint16 eventNumber);
void DeleteEventFileRecord(uint16 eventNumber);
void DeleteEventFileRecords(void);
void ReInitSdCardAndFat32(void);
inline void AdjustSampleForBitAccuracy(void);
void PowerDownSDCard(void);
void PowerUpSDCardAndInitFat32(void);
uint16 AirTriggerConvert(uint32 airTriggerToConvert);
uint32 AirTriggerConvertToUnits(uint32 airTriggerToConvert);

int OpenEventFile(uint16 eventNumber);
void CloseEventFile(int);
uint8 CheckCompressedEventDataFileExists(uint16 eventNumber);

void SetFileDateTimestamp(uint8 option);
int readWithSizeFix(int file, void* bufferPtr, uint32 length);

int GetEventFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option);
int GetERDataFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option);

void CacheResultsEventInfo(EVT_RECORD* eventRecordToCache);

void DumpSummaryListFileToEventBuffer(void);
SUMMARY_LIST_ENTRY_STRUCT* GetSummaryFromSummaryList(uint16 eventNumber);
void CacheNextSummaryListEntry(void);
void CachePreviousSummaryListEntry(void);
void CacheSummaryEntryByIndex(uint16 index);
void ParseAndCountSummaryListEntries(void);
void AddEventToSummaryList(EVT_RECORD* event);
void InitSummaryListFile(void);

void VerifyCacheEventToRam(uint16 eventNumber, char* subMessage);
void SaveRemoteEventDownloadStreamToFile(uint16 eventNumber);

#endif // _FLASHEVTS_H_
