///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "EventProcessing.h"
#include "Common.h"
#include "Summary.h"
#include "Record.h"
#include "Uart.h"
#include "Menu.h"
#include "TextTypes.h"
#include "FAT32_Definitions.h"
#include "FAT32_FileLib.h"
#include "FAT32_Access.h"
#include "sd_mmc_spi.h"
#include "PowerManagement.h"
#include "Sensor.h"
#include "lcd.h"
#include "navigation.h"
#include "fsaccess.h"
#include "fat.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static uint16* s_flashDataPtr;
static uint8 s_noFlashWrapFlag = FALSE;
static uint16* s_endFlashSectorPtr;
static uint16 s_numOfFlashSummarys;
static uint16 s_currFlashSummary;
static char s_summaryListFileName[] = "A:\\Logs\\SummaryList.bin";

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitRamSummaryTbl(void)
{
	debug("Initializing ram summary table...\r\n");

	// Basically copying all FF's to the ram summary table
	memset(&__ramFlashSummaryTbl[0], 0xFF, sizeof(__ramFlashSummaryTbl));

	__ramFlashSummaryTblKey = VALID_RAM_SUMMARY_TABLE_KEY;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CopyValidFlashEventSummariesToRam(void)
{
	uint16 ramSummaryIndex = 0;

	OverlayMessage("SUMMARY LIST", "INITIALIZING SUMMARY LIST WITH STORED EVENT INFO", 1 * SOFT_SECS);
	debug("Copying valid SD event summaries to ram...\r\n");

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Copy summaries to RAM");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);
		nav_setcwd("A:\\Events\\", TRUE, TRUE);
		nav_filelist_reset();

		while (nav_filelist_set(0 , FS_FIND_NEXT))
		{
#if 1 // New faster method to grab event number from event name
			if (!nav_file_isdir())
			{
				nav_file_name((FS_STRING)&g_spareBuffer[0], 80, FS_NAME_GET, FALSE);

				char* fileName = (char*)&g_spareBuffer[0];

				uint8 j;
				char eventNumBuffer[10];

				j = 3;
				memset(&eventNumBuffer, 0, sizeof(eventNumBuffer));

				while (fileName[j] != '.')
				{
					eventNumBuffer[j-3] = fileName[j];
					j++;
				}

				__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum = atoi((char*)eventNumBuffer);

				//debug("Event File: %s produces RST-fen: %d\r\n", fileName, __ramFlashSummaryTbl[ramSummaryIndex].fileEventNum);

				// Check to make sure ram summary index doesnt get out of range, if so reset to zero
				if (++ramSummaryIndex >= TOTAL_RAM_SUMMARIES)
				{
					ramSummaryIndex = 0;
				}
#else // Old method to open and check each event file
				int eventFile;
				uint16 eventMajorVersion = (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK);
				EVT_RECORD tempEventRecord;

				//_______________________________________________________________________________
				//___Handle the next file found
				eventFile = open(fileName, O_RDONLY);

				if (eventFile == -1)
				{
					debugErr("Event File %s not found\r\n", fileName);
					OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
				}
				else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
				{
					debugErr("Event File %s is corrupt\r\n", fileName);
					OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
				}
				else
				{
					//fl_fread(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVENT_HEADER_STRUCT));
					readWithSizeFix(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVT_RECORD));

					// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
					if ((tempEventRecord.header.startFlag == EVENT_RECORD_START_FLAG) &&
					((tempEventRecord.header.recordVersion & EVENT_MAJOR_VERSION_MASK) == eventMajorVersion) &&
					(tempEventRecord.header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
					{
						//fl_fread(eventFile, (uint8*)&tempEventRecord.summary, sizeof(EVENT_SUMMARY_STRUCT));

#if EXTENDED_DEBUG
						debug("Found Valid Event File: %s, with Event #: %d, Mode: %d, Size: %s, RSI: %d\r\n",
						fileName, tempEventRecord.summary.eventNumber, tempEventRecord.summary.mode,
						(fsaccess_file_get_size(eventFile) == (tempEventRecord.header.headerLength + tempEventRecord.header.summaryLength +
						tempEventRecord.header.dataLength)) ? "Correct" : "Incorrect", ramSummaryIndex);
#endif
						__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum = tempEventRecord.summary.eventNumber;

						// Check to make sure ram summary index doesnt get out of range, if so reset to zero
						if (++ramSummaryIndex >= TOTAL_RAM_SUMMARIES)
						{
							ramSummaryIndex = 0;
						}
					}
					else
					{
						debugWarn("Event File: %s is not valid for this unit.\r\n", fileName);
					}

					g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
					close(eventFile);
#endif
			}
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}

	//debug("Done copying events to summary (discovered %d)\r\n", ramSummaryIndex);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DumpSummaryListFileToEventBuffer(void)
{
	SUMMARY_LIST_ENTRY_STRUCT* summaryListCache = (SUMMARY_LIST_ENTRY_STRUCT*)&g_eventDataBuffer[SUMMARY_LIST_CACHE_OFFSET];
	//uint16 cachedEntries = 0;

	if (g_summaryList.totalEntries < SUMMARY_LIST_CACHE_ENTRIES_LIMIT)
	{
		nav_select(FS_NAV_ID_DEFAULT);

		if (nav_setcwd(s_summaryListFileName, TRUE, FALSE))
		{
			g_summaryList.file = open(s_summaryListFileName, O_RDONLY);

			debug("Dumping Summary list with file size: %d\r\n", nav_file_lgt());

			readWithSizeFix(g_summaryList.file, summaryListCache, nav_file_lgt());
			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(g_summaryList.file);
		}
		else
		{
			debugErr("File access problem: Dump Summary List\r\n");
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AddEventToSummaryList(EVT_RECORD* event)
{
	memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

	g_summaryList.cachedEntry.eventNumber = event->summary.eventNumber;
	g_summaryList.cachedEntry.mode = event->summary.mode;
	g_summaryList.cachedEntry.subMode = event->summary.subMode;
	g_summaryList.cachedEntry.channelSummary.a = event->summary.calculated.a;
	g_summaryList.cachedEntry.channelSummary.r = event->summary.calculated.r;
	g_summaryList.cachedEntry.channelSummary.v = event->summary.calculated.v;
	g_summaryList.cachedEntry.channelSummary.t = event->summary.calculated.t;
	g_summaryList.cachedEntry.eventTime = event->summary.captured.eventTime;
	memcpy(g_summaryList.cachedEntry.serialNumber, event->summary.version.serialNumber, SERIAL_NUMBER_STRING_SIZE);
	g_summaryList.cachedEntry.seismicSensorType = event->summary.parameters.seismicSensorType;
	g_summaryList.cachedEntry.sampleRate = event->summary.parameters.sampleRate;
	g_summaryList.cachedEntry.unitsOfMeasure = event->summary.parameters.seismicUnitsOfMeasure;
	g_summaryList.cachedEntry.unitsOfAir = event->summary.parameters.airUnitsOfMeasure;
	g_summaryList.cachedEntry.gainSelect = event->summary.parameters.channel[0].options;
	g_summaryList.cachedEntry.bitAccuracy = event->summary.parameters.bitAccuracy;
	g_summaryList.cachedEntry.vectorSumPeak = event->summary.calculated.vectorSumPeak;

	nav_select(FS_NAV_ID_DEFAULT);

	if (nav_setcwd(s_summaryListFileName, TRUE, TRUE))
	{
		g_summaryList.file = open(s_summaryListFileName, O_APPEND);
		//file_seek(0, FS_SEEK_END);
		write(g_summaryList.file, &g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT));
		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(g_summaryList.file);
	}
	else
	{
		debugErr("File access problem: Add Event to Summary list\r\n");
	}

	//nav_select(FS_NAV_ID_DEFAULT);

	debug("Added Event: %d to Summary List file\r\n", g_summaryList.cachedEntry.eventNumber);

	g_summaryList.totalEntries++;
	g_summaryList.validEntries++;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheNextSummaryListEntry(void)
{
	SUMMARY_LIST_ENTRY_STRUCT* summaryListCache = (SUMMARY_LIST_ENTRY_STRUCT*)&g_eventDataBuffer[SUMMARY_LIST_CACHE_OFFSET];

	// Check if able to use the summary list cache generated by the dump summary file to cache call
	if ((g_summaryList.totalEntries < SUMMARY_LIST_CACHE_ENTRIES_LIMIT) &&
		((g_summaryListMenuActive == YES) || ((g_modemStatus.xferState == DQMx_CMD) && (g_sampleProcessing == IDLE_STATE))))
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		while (++g_summaryList.currentEntryIndex < g_summaryList.totalEntries)
		{
			memcpy(&g_summaryList.cachedEntry, &summaryListCache[g_summaryList.currentEntryIndex], sizeof(SUMMARY_LIST_ENTRY_STRUCT));

			if (g_summaryList.cachedEntry.eventNumber)
			{
				break;
			}
		}

		if (g_summaryList.currentEntryIndex == g_summaryList.totalEntries)
		{
			debug("End of Summary list cache\r\n");
		}
	}
	else
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		nav_select(FS_NAV_ID_DEFAULT);

		if (nav_setcwd(s_summaryListFileName, TRUE, FALSE))
		{
			g_summaryList.file = open(s_summaryListFileName, O_RDONLY);

			while (file_seek(++g_summaryList.currentEntryIndex * sizeof(SUMMARY_LIST_ENTRY_STRUCT), FS_SEEK_SET) == TRUE)
			{
				readWithSizeFix(g_summaryList.file, &g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT));

				if (g_summaryList.cachedEntry.eventNumber)
				{
					break;
				}

				if (g_summaryList.currentEntryIndex == g_summaryList.totalEntries)
				{
					debug("End of Summary list entries\r\n");
					break;
				}
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(g_summaryList.file);
		}
		else
		{
			debugErr("File access problem: Cache Next Summary List\r\n");
		}

		//nav_select(FS_NAV_ID_DEFAULT);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CachePreviousSummaryListEntry(void)
{
	SUMMARY_LIST_ENTRY_STRUCT* summaryListCache = (SUMMARY_LIST_ENTRY_STRUCT*)&g_eventDataBuffer[SUMMARY_LIST_CACHE_OFFSET];

	// Check if able to use the summary list cache generated by the dump summary file to cache call
	if ((g_summaryList.totalEntries < SUMMARY_LIST_CACHE_ENTRIES_LIMIT) &&
		((g_summaryListMenuActive == YES) || ((g_modemStatus.xferState == DQMx_CMD) && (g_sampleProcessing == IDLE_STATE))))
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		if (g_summaryList.currentEntryIndex)
		{
			while (--g_summaryList.currentEntryIndex)
			{
				memcpy(&g_summaryList.cachedEntry, &summaryListCache[g_summaryList.currentEntryIndex], sizeof(SUMMARY_LIST_ENTRY_STRUCT));

				if (g_summaryList.cachedEntry.eventNumber)
				{
					break;
				}
			}
		}
		else
		{
			memcpy(&g_summaryList.cachedEntry, &summaryListCache[0], sizeof(SUMMARY_LIST_ENTRY_STRUCT));
		}

		if (g_summaryList.currentEntryIndex == 0)
		{
			debug("Start of Summary list cache\r\n");
		}
	}
	else
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		nav_select(FS_NAV_ID_DEFAULT);

		if (nav_setcwd(s_summaryListFileName, TRUE, FALSE))
		{
			g_summaryList.file = open(s_summaryListFileName, O_RDONLY);

			if (g_summaryList.currentEntryIndex)
			{
				while (file_seek(--g_summaryList.currentEntryIndex * sizeof(SUMMARY_LIST_ENTRY_STRUCT), FS_SEEK_SET) == TRUE)
				{
					readWithSizeFix(g_summaryList.file, &g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT));

					if (g_summaryList.cachedEntry.eventNumber)
					{
						break;
					}

					if (g_summaryList.currentEntryIndex == 0)
					{
						debug("Start of Summary list entries\r\n");
						break;
					}
				}
			}
			else
			{
				file_seek(0, FS_SEEK_SET);
				readWithSizeFix(g_summaryList.file, &g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT));
				debug("Start of Summary list entries\r\n");
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(g_summaryList.file);
		}
		else
		{
			debugErr("File access problem: Cache Previous Summary List\r\n");
		}

		//nav_select(FS_NAV_ID_DEFAULT);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheSummaryEntryByIndex(uint16 index)
{
	SUMMARY_LIST_ENTRY_STRUCT* summaryListCache = (SUMMARY_LIST_ENTRY_STRUCT*)&g_eventDataBuffer[SUMMARY_LIST_CACHE_OFFSET];

	// Check if able to use the summary list cache generated by the dump summary file to cache call
	if ((g_summaryList.totalEntries < SUMMARY_LIST_CACHE_ENTRIES_LIMIT) &&
		((g_summaryListMenuActive == YES) || ((g_modemStatus.xferState == DQMx_CMD) && (g_sampleProcessing == IDLE_STATE))))
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		while (index < g_summaryList.totalEntries)
		{
			memcpy(&g_summaryList.cachedEntry, &summaryListCache[index], sizeof(SUMMARY_LIST_ENTRY_STRUCT));

			// Check if a valid entry was found
			if (g_summaryList.cachedEntry.eventNumber)
			{
				g_summaryList.currentEntryIndex = index;
				break;
			}
			else // Deleted entry, move to next entry
			{
				//debug("Skipping deleted entry\r\n");
				index++;
			}
		}

		// Check if no entry was found
		if (index == g_summaryList.totalEntries)
		{
			debugErr("Cache Summary Entry reached end\r\n");
		}
	}
	else // Resort to file access
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		GetSpi1MutexLock(SDMMC_LOCK);

		nav_select(FS_NAV_ID_DEFAULT);

		if (nav_setcwd(s_summaryListFileName, TRUE, FALSE))
		{
			g_summaryList.file = open(s_summaryListFileName, O_RDONLY);

			if (index < g_summaryList.totalEntries)
			{
				while (file_seek((index * sizeof(SUMMARY_LIST_ENTRY_STRUCT)), FS_SEEK_SET) == TRUE)
				{
					readWithSizeFix(g_summaryList.file, &g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT));

					// Check if no entry was found
					if (g_summaryList.cachedEntry.eventNumber == 0)
					{
						//debug("Skipping deleted entry\r\n");
						index++;
					}
					else
					{
						g_summaryList.currentEntryIndex = index;
						break;
					}
				}
			}

			// Check if no entry was found
			if (g_summaryList.cachedEntry.eventNumber == 0)
			{
				debugErr("Cache Summary Entry failed using index: %d\r\n", index);
			}
			else
			{
				//debug("Caching Summary File event: %d\r\n", g_summaryList.cachedEntry.eventNumber);
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(g_summaryList.file);
		}
		else
		{
			debugErr("File access problem: Cache Summary Entry by Index\r\n");
		}

		//nav_select(FS_NAV_ID_DEFAULT);

		ReleaseSpi1MutexLock();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
SUMMARY_LIST_ENTRY_STRUCT* GetSummaryFromSummaryList(uint16 eventNumber)
{
	uint32 summaryListIndex = 0;
	uint16 summaryListIndexEventNumber;

	// Check if the current cached entry is not already loaded with the current event request
	if (eventNumber != g_summaryList.cachedEntry.eventNumber)
	{
		// Clear the cached entry
		memset(&g_summaryList.cachedEntry, 0, sizeof(g_summaryList.cachedEntry));

		nav_select(FS_NAV_ID_DEFAULT);

		if (nav_setcwd(s_summaryListFileName, TRUE, FALSE))
		{
			g_summaryList.file = open(s_summaryListFileName, O_RDONLY);

			while (file_seek(summaryListIndex * sizeof(SUMMARY_LIST_ENTRY_STRUCT), FS_SEEK_SET) == TRUE)
			{
				readWithSizeFix(g_summaryList.file, &summaryListIndexEventNumber, 2);

				if (summaryListIndexEventNumber == eventNumber)
				{
					file_seek(summaryListIndex * sizeof(SUMMARY_LIST_ENTRY_STRUCT), FS_SEEK_SET);
					readWithSizeFix(g_summaryList.file, &g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT));

					break;
				}
				else
				{
					summaryListIndex++;
				}
			}

			// Check if no entry was found
			if (g_summaryList.cachedEntry.eventNumber == 0)
			{
				debugErr("No Summary List entry found for Event Number: %d\r\n", eventNumber);
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(g_summaryList.file);
		}
		else
		{
			debugErr("File access problem: Get Summary from Summary List\r\n");
		}

		//nav_select(FS_NAV_ID_DEFAULT);
	}

	return (&g_summaryList.cachedEntry);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ParseAndCountSummaryListEntries(void)
{
	file_seek(0, FS_SEEK_SET);
	while (readWithSizeFix(g_summaryList.file, (uint8*)&g_summaryList.cachedEntry, sizeof(SUMMARY_LIST_ENTRY_STRUCT)) != -1)
	{
		if (g_summaryList.cachedEntry.eventNumber)
		{
			g_summaryList.validEntries++;
		}
		else
		{
			g_summaryList.deletedEntries++;
		}
	}

	if (g_summaryList.totalEntries != (g_summaryList.validEntries + g_summaryList.deletedEntries))
	{
		debugErr("Summary List Parse and Count showing incorrect amount of total entries\r\n");
	}
	else
	{
		debug("Summary List file: Valid Entires: %d, Deleted Entries: %d (Total: %d)\r\n", g_summaryList.validEntries, g_summaryList.deletedEntries, g_summaryList.totalEntries);
	}

	//nav_select(FS_NAV_ID_DEFAULT);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitSummaryListFile(void)
{
	memset(&g_summaryList, 0, sizeof(g_summaryList));

	if (nav_setcwd(s_summaryListFileName, TRUE, TRUE))
	{
		g_summaryList.file = open(s_summaryListFileName, O_RDWR);

		if (nav_file_lgt() % sizeof(SUMMARY_LIST_ENTRY_STRUCT) == 0)
		{
			g_summaryList.totalEntries = (nav_file_lgt() / sizeof(SUMMARY_LIST_ENTRY_STRUCT));
			g_summaryList.currentMonitorSessionStartSummary = nav_file_lgt();
		}
		else
		{
			debugErr("Summary List file contains a corrupted entry\r\n");
		}

		ParseAndCountSummaryListEntries();
	}
	else
	{
		ReportFileAccessProblem(s_summaryListFileName);
	}

	g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
	close(g_summaryList.file);
	//nav_select(FS_NAV_ID_DEFAULT);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CondenseRamSummaryTable(void)
{
	uint16 i = 0, j = 0;

	debug("Condensing ram summary table...\r\n");

	// Find the empty summary entries between the valid entries and condense the ram summary table
	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		// Check if we have an empty summary entry
		if (__ramFlashSummaryTbl[i].fileEventNum == 0xFFFFFFFF)
		{
			// Index (i) is the current empty summary entry

			// Search through the rest of the ram summary table for a valid entry to move up
			for (j = (uint16)(i + 1); j < TOTAL_RAM_SUMMARIES; j++)
			{
				// If the current summary entry is valid, the move it to the empty entry
				if (__ramFlashSummaryTbl[j].fileEventNum != 0xFFFFFFFF)
				{
					// Copy the valid summary entry to the current empty entry
					__ramFlashSummaryTbl[i] = __ramFlashSummaryTbl[j];

					// Set the link pointer to FF's to make the algorithm think the entry is empty
					__ramFlashSummaryTbl[j].fileEventNum = 0xFFFFFFFF;
					break;
				}
			}

			// If we didnt find a valid summary entry and hit the end of the table, we are done.
			if (j == TOTAL_RAM_SUMMARIES)
			{
				//debugRaw(" done.\r\n");
				return;
			}
		}
	}

	//debugRaw(" done.\r\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 InitFlashEvtBuff(void)
{
	uint16 i = 0;

	// Initialize the current ram summary index
	s_currFlashSummary = 0;
	s_endFlashSectorPtr = NULL;

	// Find the first free ram summary entry (table should be condensed, all used entries up front)
	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		if (__ramFlashSummaryTbl[i].fileEventNum == 0xFFFFFFFF)
		{
			s_currFlashSummary = i;
			break;
		}
	}

	return (PASSED);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitFlashBuffs(void)
{
	uint16 i;
	uint8 dataInRamGarbage = FALSE;
	//uint16* nextEmptyFlashLocation = NULL;
	//uint16* tempFlashEventPtr = NULL;
	//uint16* endOfSector = NULL;
	uint8 foundEmptyRamSummaryEntry = NO;
	uint8 condenseTable = NO;
	//uint16 eventStartData = 0;

	// Initialize the number of ram summarys
	s_numOfFlashSummarys = 0;

	//debug("Flash Event start: 0x%x End: 0x%x\r\n", FLASH_EVENT_START, FLASH_EVENT_END);

	// Init Ram Summary Table - Check to see if the ram summary table is valid
	if (__ramFlashSummaryTblKey != VALID_RAM_SUMMARY_TABLE_KEY)
	{
		// Table is invalid, key is missing
		//debugRaw("Ram Summary Key not found.\r\n");
		dataInRamGarbage = TRUE;
	}
	else // Verify and count every ram summary link that points to an event
	{
		// Count the number of summary structures with a valid (non-FF) linkPtr
		for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
		{
			// Check if the link pointer points to something
			if (__ramFlashSummaryTbl[i].fileEventNum != 0xFFFFFFFF)
			{
				// Check to make sure ram values arent garbage
				if ((uint32)(__ramFlashSummaryTbl[i].fileEventNum) >= g_nextEventNumberToUse)
				{
					// This signals a big warning
					//debugRaw("Data in Ram Summary Table is garbage.\r\n");

					// Assume the whole table is garbage, so just recreate it
					dataInRamGarbage = TRUE;
					break;
				}
				else if (CheckValidEventFile((uint16)((uint32)(__ramFlashSummaryTbl[i].fileEventNum))) == NO)
				{
					//debugRaw("Ram summary (%d) points to an invalid event.\r\n", i+1);
					__ramFlashSummaryTbl[i].fileEventNum = 0xFFFFFFFF;
				}
				else
				{
					// Inc the total number of events found
					s_numOfFlashSummarys++;

					// Check if any empty entries were found before or while counting events
					if (foundEmptyRamSummaryEntry == YES)
						condenseTable = YES;
				}
			}
			else
				foundEmptyRamSummaryEntry = YES;
		}
	}

	// Check if we found garbage in the ram summary table
	if (dataInRamGarbage == TRUE)
	{
		// Re-init the table
		InitRamSummaryTbl();

#if 1 // Test removing an old mechanism since it's an extremely slow process for large numbers of files
		// Find all flash events and recreate ram summary entries for them
		CopyValidFlashEventSummariesToRam();
#endif

		// Re-count the number of summary structures with a valid (non-FF) link pointer
		s_numOfFlashSummarys = 0;
		for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
		{
			if (__ramFlashSummaryTbl[i].fileEventNum != 0xFFFFFFFF)
			{
				s_numOfFlashSummarys++;
			}
		}
	}
	else if (condenseTable == YES)
	{
		// Remove any empty entries and sort oldest first
		CondenseRamSummaryTable();
	}

	debug("Number of Event Summaries found in RAM: %d\r\n", s_numOfFlashSummarys);

	// Initialize some global values and check if successful
	if (InitFlashEvtBuff() == FAILED)
	{
		// Failed (encountered and handled a partial flash event)
		// Now everything is right with the world, need to initialize global values again
		InitFlashEvtBuff();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ClearAndFillInCommonRecordInfo(EVT_RECORD* eventRec)
{
	uint8 i;

	memset(eventRec, 0x00, sizeof(EVT_RECORD));
	memset(eventRec->summary.parameters.unused, 0xAF, sizeof(eventRec->summary.parameters.unused));
	memset(eventRec->summary.captured.unused, 0xBF, sizeof(eventRec->summary.captured.unused));
	memset(eventRec->summary.calculated.unused, 0xCF, sizeof(eventRec->summary.calculated.unused));

	//--------------------------------
	eventRec->header.startFlag = (uint16)EVENT_RECORD_START_FLAG;
	eventRec->header.recordVersion = (uint16)EVENT_RECORD_VERSION;
	eventRec->header.headerLength = (uint16)sizeof(EVENT_HEADER_STRUCT);
	eventRec->header.summaryLength = (uint16)sizeof(EVENT_SUMMARY_STRUCT);
	//--------------------------------
	eventRec->summary.parameters.sampleRate = (uint16)g_triggerRecord.trec.sample_rate;
	//--------------------------------
	eventRec->summary.captured.endTime = GetCurrentTime();
	eventRec->summary.captured.batteryLevel = (uint32)(100.0 * GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
	eventRec->summary.captured.printerStatus = g_unitConfig.autoPrint;
	eventRec->summary.captured.externalTrigger = NO;
	eventRec->summary.captured.comboEventsRecordedDuringSession = 0;
	eventRec->summary.captured.comboEventsRecordedStartNumber = 0;
	eventRec->summary.captured.comboEventsRecordedEndNumber = 0;
	eventRec->summary.captured.comboBargraphEventNumberLink = 0;
	//--------------------------------
	eventRec->summary.parameters.calibrationDateSource = g_currentCalibration.source;
	memset(&(eventRec->summary.captured.calDateTime), 0, sizeof(eventRec->summary.captured.calDateTime));
	ConvertCalDatetoDateTime(&eventRec->summary.captured.calDateTime, &g_currentCalibration.date);
	eventRec->summary.parameters.seismicSensorCurrentCalDate = g_seismicSmartSensorMemory.currentCal.calDate;
	eventRec->summary.parameters.acousticSensorCurrentCalDate = g_acousticSmartSensorMemory.currentCal.calDate;
	// Store Unit cal date in smart sensor field if the selected source is a smart sensor cal date (to prevent duplication of data and losing the Unit cal date)
	if (g_currentCalibration.source == SEISMIC_SMART_SENSOR_CAL_DATE) { eventRec->summary.parameters.seismicSensorCurrentCalDate = g_factorySetupRecord.calDate; }
	else if (g_currentCalibration.source == ACOUSTIC_SMART_SENSOR_CAL_DATE) { eventRec->summary.parameters.acousticSensorCurrentCalDate = g_factorySetupRecord.calDate; }
	//-----------------------
	memset(&(eventRec->summary.parameters.seismicSensorSerialNumber[0]), 0, SENSOR_SERIAL_NUMBER_SIZE);
	memcpy(&(eventRec->summary.parameters.seismicSensorSerialNumber[0]), &(g_seismicSmartSensorMemory.serialNumber[0]), SENSOR_SERIAL_NUMBER_SIZE);
	eventRec->summary.parameters.seismicSensorFacility = g_seismicSmartSensorMemory.currentCal.calFacility;
	eventRec->summary.parameters.seismicSensorInstrument = g_seismicSmartSensorMemory.currentCal.calInstrument;
	memset(&(eventRec->summary.parameters.acousticSensorSerialNumber[0]), 0, SENSOR_SERIAL_NUMBER_SIZE);
	memcpy(&(eventRec->summary.parameters.acousticSensorSerialNumber[0]), &(g_acousticSmartSensorMemory.serialNumber[0]), SENSOR_SERIAL_NUMBER_SIZE);
	eventRec->summary.parameters.acousticSensorFacility = g_acousticSmartSensorMemory.currentCal.calFacility;
	eventRec->summary.parameters.acousticSensorInstrument = g_acousticSmartSensorMemory.currentCal.calInstrument;
	//-----------------------
	memset(&(eventRec->summary.parameters.companyName[0]), 0, COMPANY_NAME_STRING_SIZE);
	memcpy(&(eventRec->summary.parameters.companyName[0]), &(g_triggerRecord.trec.client[0]), COMPANY_NAME_STRING_SIZE - 1);
	memset(&(eventRec->summary.parameters.seismicOperator[0]), 0, SEISMIC_OPERATOR_STRING_SIZE);
	memcpy(&(eventRec->summary.parameters.seismicOperator[0]), &(g_triggerRecord.trec.oper[0]), SEISMIC_OPERATOR_STRING_SIZE - 1);
	memset(&(eventRec->summary.parameters.sessionLocation[0]), 0, SESSION_LOCATION_STRING_SIZE);
	memcpy(&(eventRec->summary.parameters.sessionLocation[0]), &(g_triggerRecord.trec.loc[0]), SESSION_LOCATION_STRING_SIZE - 1);
	memset(&(eventRec->summary.parameters.sessionComments[0]), 0, SESSION_COMMENTS_STRING_SIZE);
	memcpy(&(eventRec->summary.parameters.sessionComments[0]), &(g_triggerRecord.trec.comments[0]), sizeof(g_triggerRecord.trec.comments));
	//-----------------------
	memset(&(eventRec->summary.version.modelNumber[0]), 0, MODEL_STRING_SIZE);
	memcpy(&(eventRec->summary.version.modelNumber[0]), &(g_factorySetupRecord.serial_num[0]), 15);
	memset(&(eventRec->summary.version.serialNumber[0]), 0, SERIAL_NUMBER_STRING_SIZE);
	memcpy(&(eventRec->summary.version.serialNumber[0]), &(g_factorySetupRecord.serial_num[0]), 15);
	memset(&(eventRec->summary.version.softwareVersion[0]), 0, VERSION_STRING_SIZE);
	memcpy(&(eventRec->summary.version.softwareVersion[0]), (void*)&g_buildVersion[0], strlen(g_buildVersion));
	memset(&(eventRec->summary.version.seismicSensorRom), 0, sizeof(g_seismicSmartSensorRom));
	memcpy(&(eventRec->summary.version.seismicSensorRom), (void*)&g_seismicSmartSensorRom, sizeof(g_seismicSmartSensorRom));
	memset(&(eventRec->summary.version.acousticSensorRom), 0, sizeof(g_acousticSmartSensorRom));
	memcpy(&(eventRec->summary.version.acousticSensorRom), (void*)&g_acousticSmartSensorRom, sizeof(g_acousticSmartSensorRom));

	//-----------------------
	eventRec->summary.parameters.bitAccuracy = ((g_triggerRecord.trec.bitAccuracy < ACCURACY_10_BIT) || (g_triggerRecord.trec.bitAccuracy > ACCURACY_16_BIT)) ? 
												ACCURACY_16_BIT : g_triggerRecord.trec.bitAccuracy;
	eventRec->summary.parameters.numOfChannels = NUMBER_OF_CHANNELS_DEFAULT;
	eventRec->summary.parameters.aWeighting = ((uint8)g_factorySetupRecord.aweight_option & (uint8)g_unitConfig.airScale); // Equals 1 if enabled (1) and scale is A-weighting (1)
	eventRec->summary.parameters.seismicSensorType = g_factorySetupRecord.sensor_type;
	eventRec->summary.parameters.airSensorType = SENSOR_MICROPHONE;

	eventRec->summary.parameters.adjustForTempDrift = g_triggerRecord.trec.adjustForTempDrift;
	eventRec->summary.parameters.seismicUnitsOfMeasure = g_unitConfig.unitsOfMeasure;
	eventRec->summary.parameters.airUnitsOfMeasure = g_unitConfig.unitsOfAir;
	eventRec->summary.parameters.distToSource = (uint32)(g_triggerRecord.trec.dist_to_source * 100.0);
	eventRec->summary.parameters.weightPerDelay = (uint32)(g_triggerRecord.trec.weight_per_delay * 100.0);
	//-----------------------
	eventRec->summary.parameters.channel[0].type = ACOUSTIC_CHANNEL_TYPE;
	eventRec->summary.parameters.channel[0].input = 4;
	eventRec->summary.parameters.channel[1].type = RADIAL_CHANNEL_TYPE;
	eventRec->summary.parameters.channel[1].input = 1;
	eventRec->summary.parameters.channel[2].type = VERTICAL_CHANNEL_TYPE;
	eventRec->summary.parameters.channel[2].input = 3;
	eventRec->summary.parameters.channel[3].type = TRANSVERSE_CHANNEL_TYPE;
	eventRec->summary.parameters.channel[3].input = 2;

	for (i = 0; i < 4; i++) // First seismic group
	{
		eventRec->summary.parameters.channel[i].group = SEISMIC_GROUP_1;
		eventRec->summary.parameters.channel[i].options = (g_triggerRecord.srec.sensitivity == LOW) ? GAIN_SELECT_x2 : GAIN_SELECT_x4;
	}

	for (i = 4; i < 8; i++) // Second seismic group
	{
		eventRec->summary.parameters.channel[i].group = SEISMIC_GROUP_2;
		eventRec->summary.parameters.channel[i].type = 0;
		eventRec->summary.parameters.channel[i].input = DISABLED;
		eventRec->summary.parameters.channel[i].options = 0;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitEventRecord(uint8 opMode)
{
	EVT_RECORD* eventRec;
	uint8 idex;
	float tempSesmicTriggerInUnits;
	float unitsDiv;

	if ((opMode == WAVEFORM_MODE) || (opMode == MANUAL_CAL_MODE) || (opMode == COMBO_MODE))
	{
		eventRec = &g_pendingEventRecord;		
		ClearAndFillInCommonRecordInfo(eventRec);

		eventRec->summary.mode = opMode;
		if (opMode == COMBO_MODE) { eventRec->summary.subMode = WAVEFORM_MODE; }
		eventRec->summary.eventNumber = (uint16)g_nextEventNumberToUse;

		eventRec->summary.parameters.numOfSamples = (uint16)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time);
		eventRec->summary.parameters.preBuffNumOfSamples = (uint16)(g_triggerRecord.trec.sample_rate / g_unitConfig.pretrigBufferDivider);
		eventRec->summary.parameters.calDataNumOfSamples = (uint16)(CALIBRATION_NUMBER_OF_SAMPLES);

		// Reset parameters for the special calibration mode
		if (opMode == MANUAL_CAL_MODE)
		{
			eventRec->summary.parameters.sampleRate = MANUAL_CAL_DEFAULT_SAMPLE_RATE;
			eventRec->summary.parameters.bitAccuracy = ACCURACY_16_BIT;
			eventRec->summary.parameters.numOfSamples = 0;
			eventRec->summary.parameters.preBuffNumOfSamples = 0;
			eventRec->summary.parameters.seismicTriggerLevel = 0;
			eventRec->summary.parameters.airTriggerLevel = 0;
			eventRec->summary.parameters.recordTime = 0;
			for (idex = 0; idex < 8; idex++) { eventRec->summary.parameters.channel[idex].options = GAIN_SELECT_x2; }
		}
		else // ((opMode == WAVEFORM_MODE) || (opMode == COMBO_MODE))
		{
			eventRec->summary.parameters.recordTime = (uint32)g_triggerRecord.trec.record_time;

			if ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.seismicTriggerLevel == EXTERNAL_TRIGGER_CHAR) ||
				(g_triggerRecord.trec.airTriggerLevel == MANUAL_TRIGGER_CHAR))
			{
				eventRec->summary.parameters.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;

				debug("Seismic trigger in units: No Trigger\r\n");
			}
			else // Seismic trigger is valid
			{
				eventRec->summary.parameters.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);

				// Calculate the divider used for converting stored A/D peak counts to units of measure
				unitsDiv = (float)(g_bitAccuracyMidpoint * SENSOR_ACCURACY_100X_SHIFT * ((g_triggerRecord.srec.sensitivity == LOW) ? 2 : 4)) /
				(float)(g_factorySetupRecord.sensor_type);

				tempSesmicTriggerInUnits = (float)(g_triggerRecord.trec.seismicTriggerLevel >> g_bitShiftForAccuracy) / (float)unitsDiv;

				if ((g_factorySetupRecord.sensor_type != SENSOR_ACC) && (g_unitConfig.unitsOfMeasure == METRIC_TYPE))
				{
					tempSesmicTriggerInUnits *= (float)METRIC;
				}

				debug("Seismic trigger in units: %05.2f %s\r\n", tempSesmicTriggerInUnits, (g_unitConfig.unitsOfMeasure == METRIC_TYPE ? "mm" : "in"));
				eventRec->summary.parameters.seismicTriggerInUnits = (uint32)(tempSesmicTriggerInUnits * 100);
			}

			if ((g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.airTriggerLevel == EXTERNAL_TRIGGER_CHAR) ||
				(g_triggerRecord.trec.airTriggerLevel == MANUAL_TRIGGER_CHAR))
			{
				eventRec->summary.parameters.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;

				debug("Air trigger in units: No Trigger\r\n");
			}
			else // Air trigger is valid
			{
				eventRec->summary.parameters.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);

				eventRec->summary.parameters.airTriggerInUnits = AirTriggerConvertToUnits(g_triggerRecord.trec.airTriggerLevel);

				if (g_unitConfig.unitsOfAir == MILLIBAR_TYPE)
				{
					debug("Air trigger in units: %05.3f mB\r\n", (float)(eventRec->summary.parameters.airTriggerInUnits / 10000));
				}
				else // (g_unitConfig.unitsOfAir == DECIBEL_TYPE)
				{
					debug("Air trigger in units: %d dB\r\n", eventRec->summary.parameters.airTriggerInUnits);
				}
			}
		}	
	}

	if ((opMode == BARGRAPH_MODE) || (opMode == COMBO_MODE))
	{
		eventRec = &g_pendingBargraphRecord;		
		ClearAndFillInCommonRecordInfo(eventRec);

		eventRec->summary.mode = opMode;
		if (opMode == COMBO_MODE) { eventRec->summary.subMode = BARGRAPH_MODE; }
		eventRec->summary.eventNumber = (uint16)g_nextEventNumberToUse;

		eventRec->summary.captured.eventTime = GetCurrentTime();

		eventRec->summary.parameters.barInterval = (uint16)g_triggerRecord.bgrec.barInterval;
		eventRec->summary.parameters.summaryInterval = (uint16)g_triggerRecord.bgrec.summaryInterval;
		eventRec->summary.parameters.numOfSamples = 0;
		eventRec->summary.parameters.preBuffNumOfSamples = 0;
		eventRec->summary.parameters.calDataNumOfSamples = 0;
		eventRec->summary.parameters.activeChannels = g_triggerRecord.berec.barChannel;
	}	
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitCurrentEventNumber(void)
{
	CURRENT_EVENT_NUMBER_STRUCT currentEventNumberRecord;

	GetRecordData(&currentEventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);

	if (!currentEventNumberRecord.invalid)
	{
		// Set the Current Event number to the last event number stored plus 1
		g_nextEventNumberToUse = currentEventNumberRecord.currentEventNumber + 1;
	}
	else // record is invalid
	{
		g_nextEventNumberToUse = 1;

		// Don't save as 1 since no event has been recorded; The eventual save will validate the event number
	}

	debug("Stored Event ID: %d, Next Event ID to use: %d\r\n", (g_nextEventNumberToUse - 1), g_nextEventNumberToUse);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetLastStoredEventNumber(void)
{
	return ((uint16)(g_nextEventNumberToUse - 1));
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StoreCurrentEventNumber(void)
{
	CURRENT_EVENT_NUMBER_STRUCT currentEventNumberRecord;

	// Store the Current Event number as the newest Unique Event number
	currentEventNumberRecord.currentEventNumber = g_nextEventNumberToUse;
	SaveRecordData(&currentEventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);

	// Store as the last Event recorded in AutoDialout table
	__autoDialoutTbl.lastStoredEvent = g_nextEventNumberToUse;

	// Increment to a new Event number
	g_nextEventNumberToUse++;
	debug("Saved Event ID: %d, Next Event ID to use: %d\r\n", (g_nextEventNumberToUse - 1), g_nextEventNumberToUse);

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void IncrementCurrentEventNumber(void)
{
	CURRENT_EVENT_NUMBER_STRUCT currentEventNumberRecord;

	// Save the Current Event number as the newest Unique Event number
	currentEventNumberRecord.currentEventNumber = g_nextEventNumberToUse;

	// Save as the last Event recorded in AutoDialout table
	__autoDialoutTbl.lastStoredEvent = g_nextEventNumberToUse;

	// Increment to a new Event number
	g_nextEventNumberToUse++;
	debug("Current Event ID: %d, Next Event ID to use: %d\r\n", (g_nextEventNumberToUse - 1), g_nextEventNumberToUse);

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetUniqueEventNumber(SUMMARY_DATA* currentSummary)
{
	// Event number is stored in ram summary
	return currentSummary->fileEventNum;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void GetEventFileInfo(uint16 eventNumber, EVENT_HEADER_STRUCT* eventHeaderPtr, EVENT_SUMMARY_STRUCT* eventSummaryPtr, BOOLEAN cacheDataToRamBuffer)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	EVENT_HEADER_STRUCT fileEventHeader;
	EVENT_SUMMARY_STRUCT fileSummary;
	int eventFile;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Check event file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);

		// Verify file ID
		if (eventFile == -1)
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
			readWithSizeFix(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));

			// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
			if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
			((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
			{
				debug("Found Valid Event File: %s\r\n", fileName);

				readWithSizeFix(eventFile, (uint8*)&fileSummary, sizeof(EVENT_SUMMARY_STRUCT));
			
				if (cacheDataToRamBuffer == YES)
				{
					readWithSizeFix(eventFile, (uint8*)&g_eventDataBuffer[0], (fsaccess_file_get_size(eventFile) - (sizeof(EVENT_HEADER_STRUCT) - sizeof(EVENT_SUMMARY_STRUCT))));
				}
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(eventFile);
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}

	if (eventHeaderPtr != NULL)
	{
		memcpy(eventHeaderPtr, &fileEventHeader, sizeof(EVENT_HEADER_STRUCT));
	}

	if (eventSummaryPtr != NULL)
	{
		memcpy(eventSummaryPtr, &fileSummary, sizeof(EVENT_SUMMARY_STRUCT));
	}	
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int OpenEventFile(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	int eventFile = 0;
	uint8 dummy;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Get event file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);

		//g_fileAccessLock = SDMMC_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);

		// Dummy read to cache first sector and set up globals
		dummy = file_getc();
		file_seek(0, FS_SEEK_SET);
	}

	return (eventFile);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CloseEventFile(int eventFile)
{
	g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
	close(eventFile);

	//g_fileAccessLock = AVAILABLE;
	ReleaseSpi1MutexLock();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 CheckCompressedEventDataFileExists(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	uint8 fileExistStatus = NO;

	GetSpi1MutexLock(SDMMC_LOCK);

	nav_select(FS_NAV_ID_DEFAULT);

	sprintf(fileName, "A:\\ERData\\Evt%d.nsD", eventNumber);

	// Check if the Compressed data file is not present
	if (nav_setcwd(fileName, TRUE, FALSE) == TRUE)
	{
		fileExistStatus = YES;
	}

	//g_fileAccessLock = AVAILABLE;
	ReleaseSpi1MutexLock();
	
	return (fileExistStatus);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void GetEventFileRecord(uint16 eventNumber, EVT_RECORD* eventRecord)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	int eventFile;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Get event file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);

		//g_fileAccessLock = SDMMC_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);

		// Verify file ID
		if (eventFile == -1)
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
			readWithSizeFix(eventFile, (uint8*)eventRecord, sizeof(EVT_RECORD));

			// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
			if ((eventRecord->header.startFlag == EVENT_RECORD_START_FLAG) &&
			((eventRecord->header.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(eventRecord->header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
			{
				debug("Found Valid Event File: %s\r\n", fileName);
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(eventFile);
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DeleteEventFileRecord(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Delete event file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		if (nav_setcwd(fileName, TRUE, FALSE) == TRUE)
		{
			if (nav_file_del(TRUE) == FALSE)
			{
				OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
			}
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DeleteEventFileRecords(void)
{
	uint16 eventsDeleted = 0;
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	char popupText[50];

	debug("Deleting Events...\r\n");

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Delete multiple events");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);

		// Handle removing event files
		if (nav_setcwd("A:\\Events\\", TRUE, FALSE) == TRUE)
		{
			//nav_filelist_reset();

			while (nav_filelist_set(0 , FS_FIND_NEXT))
			{
				nav_file_getname(&fileName[0], 50);
				sprintf(popupText, "REMOVING %s", fileName);
				OverlayMessage(getLangText(STATUS_TEXT), popupText, 0);

				// Delete file or directory
				if (nav_file_del(FALSE) == FALSE)
				{
					nav_file_getname(&fileName[0], 50);
					OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
					//break;
				}
				else
				{
					eventsDeleted++;
#if 0 // Exception testing (Prevent non-ISR soft loop watchdog from triggering)
					g_execCycles++;
#endif
				}
			}
		}

		// Handle removing compressed event data files
		if (nav_setcwd("A:\\ERData\\", TRUE, FALSE) == TRUE)
		{
			//nav_filelist_reset();

			while (nav_filelist_set(0 , FS_FIND_NEXT))
			{
				nav_file_getname(&fileName[0], 50);
				sprintf(popupText, "REMOVING %s", fileName);
				OverlayMessage(getLangText(STATUS_TEXT), popupText, 0);

				// Delete file or directory
				if (nav_file_del(FALSE) == FALSE)
				{
					nav_file_getname(&fileName[0], 50);
					OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
					//break;
				}
				else
				{
					eventsDeleted++;
#if 0 // Exception testing (Prevent non-ISR soft loop watchdog from triggering)
					g_execCycles++;
#endif
				}
			}
		}

		// Re-create directory
		if (nav_setcwd("A:\\ERData\\", TRUE, FALSE) == FALSE)
		{
			debugErr("Unable to access or create the Events directory\r\n");
			OverlayMessage(getLangText(ERROR_TEXT), "UNABLE TO ACCESS EVENTS DIR", 3 * SOFT_SECS);
		}

		// Re-create directory
		if (nav_setcwd("A:\\Events\\", TRUE, FALSE) == FALSE)
		{
			debugErr("Unable to access or create the Events directory\r\n");
			OverlayMessage(getLangText(ERROR_TEXT), "UNABLE TO ACCESS EVENTS DIR", 3 * SOFT_SECS);
		}

		if (nav_setcwd(s_summaryListFileName, TRUE, FALSE) == TRUE)
		{
			if (nav_file_del(FALSE) == FALSE)
			{
				nav_file_getname(&fileName[0], 50);
				OverlayMessage(fileName, "UNABLE TO DELETE SUMMARY LIST", 3 * SOFT_SECS);
			}
		}

		InitSummaryListFile();

		sprintf(popupText, "REMOVED %d EVENTS", eventsDeleted);
		OverlayMessage("DELETE EVENTS", popupText, 3 * SOFT_SECS);

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheEventDataToBuffer(uint16 eventNumber, uint8* dataBuffer, uint32 dataOffset, uint32 dataSize)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	int eventFile;

	GetSpi1MutexLock(SDMMC_LOCK);
	//g_fileAccessLock = SDMMC_LOCK;

	nav_select(FS_NAV_ID_DEFAULT);

	sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
	eventFile = open(fileName, O_RDONLY);

	// Verify file ID
	if (eventFile == -1)
	{
		debugErr("Event File %s not found\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}
	else
	{
		file_seek(dataOffset, FS_SEEK_SET);
		readWithSizeFix(eventFile, dataBuffer, dataSize);

		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(eventFile);
	}

	//g_fileAccessLock = AVAILABLE;
	ReleaseSpi1MutexLock();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint32 GetERDataSize(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	uint32 size = 0;
	int eventFile;

	GetSpi1MutexLock(SDMMC_LOCK);
	//g_fileAccessLock = SDMMC_LOCK;

	nav_select(FS_NAV_ID_DEFAULT);

	sprintf(fileName, "A:\\ERData\\Evt%d.nsD", eventNumber);
	eventFile = open(fileName, O_RDONLY);

	// Verify file ID
	if (eventFile == -1)
	{
		debugErr("ER Data File %s not found\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}
	else
	{
		size = nav_file_lgt();

		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(eventFile);
	}

	//g_fileAccessLock = AVAILABLE;
	ReleaseSpi1MutexLock();

	return (size);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheERDataToBuffer(uint16 eventNumber, uint8* dataBuffer, uint32 dataOffset, uint32 dataSize)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	int eventFile;

	GetSpi1MutexLock(SDMMC_LOCK);
	//g_fileAccessLock = SDMMC_LOCK;

	nav_select(FS_NAV_ID_DEFAULT);

	sprintf(fileName, "A:\\ERData\\Evt%d.nsD", eventNumber);
	eventFile = open(fileName, O_RDONLY);

	// Verify file ID
	if (eventFile == -1)
	{
		debugErr("ER Data File %s not found\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}
	else
	{
		file_seek(dataOffset, FS_SEEK_SET);
		readWithSizeFix(eventFile, dataBuffer, dataSize);

		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(eventFile);
	}

	//g_fileAccessLock = AVAILABLE;
	ReleaseSpi1MutexLock();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheEventDataToRam(uint16 eventNumber, uint32 dataSize)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	int eventFile;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Cache event data");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);

		// Verify file ID
		if (eventFile == -1)
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
			file_seek(sizeof(EVT_RECORD), FS_SEEK_SET);
			readWithSizeFix(eventFile, (uint8*)&g_eventDataBuffer[0], dataSize);

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(eventFile);
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#define VERIFY_CHUNK_SIZE	1024
void VerifyCacheEventToRam(uint16 eventNumber, char* subMessage)
{
	uint16 i = 0;
	uint32 remainingData = 0;
	uint32 index = 0;
	uint32 readCount = 0;
	uint16 errorCount = 0;
	uint8* dataPtr = (uint8*)&g_eventDataBuffer[0];
	uint8* verifyPtr = (uint8*)&g_spareBuffer[(SPARE_BUFFER_SIZE - VERIFY_CHUNK_SIZE - 1)];
	int eventFile;
	char fileName[50];
	char message[50];

	sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);

	nav_select(FS_NAV_ID_DEFAULT);

	eventFile = open(fileName, O_RDONLY);

	file_seek(0, FS_SEEK_SET);
	remainingData = nav_file_lgt();

	debug("Verify Cache data bytes: %d\r\n", remainingData);

	while (remainingData)
	{
		if (remainingData > VERIFY_CHUNK_SIZE)
		{
			readCount = read(eventFile, verifyPtr, VERIFY_CHUNK_SIZE);

			if (readCount != VERIFY_CHUNK_SIZE)
			{
				debugErr("CacheEventToRam Read Data size incorrect (%d)\r\n", readCount);
				close (eventFile);
				return;
			}

			for (i = 0; i < VERIFY_CHUNK_SIZE; i++)
			{
				if (verifyPtr[i] != dataPtr[index])
				{
					debugErr("CacheEventToRam verification failed: %x != %x, index: %d\r\n", verifyPtr[i], dataPtr[index], index);

					if (errorCount++ > 1024) { debugErr("Too many errors, bailing\r\n\r\n"); close (eventFile); return; }
				}

				index++;
			}

			remainingData -= VERIFY_CHUNK_SIZE;
		}
		else // Remaining data size is less than the access limit
		{
			readCount = read(eventFile, &verifyPtr[0], remainingData);

			if (readCount != remainingData)
			{
				debugErr("CacheEventToRam Read Data size incorrect (%d)\r\n", readCount);
				close (eventFile);
				return;
			}

			for (i = 0; i < remainingData; i++)
			{
				if (verifyPtr[(index % VERIFY_CHUNK_SIZE)] != dataPtr[index])
				{
					debugErr("CacheEventToRam verification failed: %x != %x, index: %d\r\n", verifyPtr[(index % 8192)], dataPtr[index], index);

					if (errorCount++ > 1024) { debugErr("Too many errors, bailing\r\n\r\n"); close (eventFile); return; }
				}

				index++;
			}

			remainingData = 0;
		}
	}

	if (errorCount)
	{
		sprintf(message, "EVENT %d CACHE VERIFY FAILED (%s SEND)", eventNumber, subMessage);
		MessageBox(getLangText(ERROR_TEXT), message, MB_OK);
	}

	close (eventFile);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 CacheEventToRam(uint16 eventNumber, EVT_RECORD* eventRecordPtr)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	int eventFile;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Cache event");
		return EVENT_CACHE_FAILURE;
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);

		// Verify file ID
		if (eventFile == -1)
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
			ReleaseSpi1MutexLock();
			return EVENT_CACHE_FAILURE;
		}
		// Verify file is big enough
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVT_RECORD))
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
			ReleaseSpi1MutexLock();
			return EVENT_CACHE_FAILURE;
		}
		else
		{
			readWithSizeFix(eventFile, (uint8*)eventRecordPtr, sizeof(EVT_RECORD));
			readWithSizeFix(eventFile, (uint8*)&g_eventDataBuffer[0], (fsaccess_file_get_size(eventFile) - sizeof(EVT_RECORD)));
			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(eventFile);
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();

		return EVENT_CACHE_SUCCESS;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
BOOLEAN CheckValidEventFile(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	EVENT_HEADER_STRUCT fileEventHeader;
	BOOLEAN validFile = NO;
	int eventFile;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Check valid event");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		GetSpi1MutexLock(SDMMC_LOCK);
		//g_fileAccessLock = SDMMC_LOCK;

		nav_select(FS_NAV_ID_DEFAULT);

		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);

		// Verify file ID
		if (eventFile == -1)
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
			readWithSizeFix(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));

			// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
			if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
				((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
				(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
			{
				//debug("Found Valid Event File: %s", fileName);

				validFile = YES;
			}

			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(eventFile);
		}

		//g_fileAccessLock = AVAILABLE;
		ReleaseSpi1MutexLock();
	}

	return(validFile);
}

#define SD_MMC_SPI_NPCS	2
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ReInitSdCardAndFat32(void)
{
	// Power off the SD card
	PowerControl(SD_POWER, OFF);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	// Power on the SD Card
	PowerControl(SD_POWER, ON);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	// Check if SD Detect pin 
	if (gpio_get_pin_value(AVR32_PIN_PA02) == SDMMC_CARD_DETECTED)
	{
		GetSpi1MutexLock(SDMMC_LOCK);

		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		// Init the NAV and select the SD MMC Card
		nav_reset();
		nav_select(FS_NAV_ID_DEFAULT);

		// Check if the drive select was successful
		if (nav_drive_set(0) == TRUE)
		{
			// Check if the partition mount was unsuccessful (otherwise passes through without an error case)
			if (nav_partition_mount() == FALSE)
			{
				// Error case
				debugErr("FAT32 SD Card mount failed\r\n");
				OverlayMessage("ERROR", "FAILED TO MOUNT SD CARD!", 0);
			}
		}
		else // Error case
		{
			debugErr("FAT32 SD Card drive select failed\r\n");
			OverlayMessage("ERROR", "FAILED TO SELECT SD CARD DRIVE!", 0);
		}

		ReleaseSpi1MutexLock();
	}
	else
	{
		debugErr("\n\nSD Card not detected\r\n");
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerDownSDCard(void)
{
	debugRaw("\n Powering down SD Card... ");

	// Power off the SD card
	PowerControl(SD_POWER, OFF);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	debugRaw("done.\r\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerUpSDCardAndInitFat32(void)
{
	debugRaw("\nPowering up SD Card and ReInit Fat32... \r\n");

	// Power on the SD Card
	PowerControl(SD_POWER, ON);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	// Check if SD Detect pin 
	if (gpio_get_pin_value(AVR32_PIN_PA02) == SDMMC_CARD_DETECTED)
	{
		GetSpi1MutexLock(SDMMC_LOCK);

		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		// Init the NAV and select the SD MMC Card
		nav_reset();
		nav_select(FS_NAV_ID_DEFAULT);

		// Check if the drive select was successful
		if (nav_drive_set(0) == TRUE)
		{
			// Check if the partition mount was unsuccessful (otherwise passes through without an error case)
			if (nav_partition_mount() == FALSE)
			{
				// Error case
				debugErr("FAT32 SD Card mount failed\r\n");
				OverlayMessage("ERROR", "FAILED TO MOUNT SD CARD!", 0);
			}
		}
		else // Error case
		{
			debugErr("FAT32 SD Card drive select failed\r\n");
			OverlayMessage("ERROR", "FAILED TO SELECT SD CARD DRIVE!", 0);
		}

		ReleaseSpi1MutexLock();
	}
	else
	{
		debugErr("\n\nSD Card not detected\r\n");
	}

	debugRaw("done.\r\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void SetFileDateTimestamp(uint8 option)
{
	char dateTimeBuffer[20];
	DATE_TIME_STRUCT dateTime;

	// Set the creation date and time
	dateTime = GetCurrentTime();

	// ASCII string date/time format to write: "YYYYMMDDHHMMSSMS" = year, month, day, hour, min, sec, ms
	sprintf((char*)&dateTimeBuffer[0], "%04d%02d%02d%02d%02d%02d%02d", (dateTime.year + 2000), dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec, dateTime.hundredths);

	if (option == FS_DATE_CREATION)
	{
		nav_file_dateset((char*)&dateTimeBuffer[0], FS_DATE_CREATION);
	}
	else
	{
		nav_file_dateset((char*)&dateTimeBuffer[0], FS_DATE_LAST_WRITE);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int GetEventFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option)
{
	char* fileName = (char*)&g_spareBuffer[0];

	int fileHandle;
	int fileOption;

	sprintf(fileName, "A:\\Events\\Evt%d.ns8", newFileEventNumber);

	// Set the file option flags for the file process
	switch (option)
	{
		case CREATE_EVENT_FILE:
		debug("File to create: %s\r\n", fileName);
		fileOption = (O_CREAT | O_WRONLY);
		break;

		case READ_EVENT_FILE:
		debug("File to read: %s\r\n", fileName);
		fileOption = O_RDONLY;
		break;

		case APPEND_EVENT_FILE:
		debug("File to append: %s\r\n", fileName);
		fileOption = O_APPEND;
		break;

		case OVERWRITE_EVENT_FILE:
		debug("File to overwrite: %s\r\n", fileName);
		fileOption = O_RDWR;
		break;
	}

	fileHandle = open(fileName, fileOption);

	// Check if trying to create a new event file and one already exists
	if ((fileHandle == -1) && (option == CREATE_EVENT_FILE))
	{
		// Delete the existing file and check if the operation was successful
		if (nav_file_del(TRUE) == TRUE)
		{
			fileHandle = open(fileName, fileOption);
		}
	}

	if (option == CREATE_EVENT_FILE)
	{
		SetFileDateTimestamp(FS_DATE_CREATION);
	}

	return (fileHandle);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int GetERDataFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option)
{
	char* fileName = (char*)&g_spareBuffer[0];

	int fileHandle;
	int fileOption;

	sprintf(fileName, "A:\\ERData\\Evt%d.nsD", newFileEventNumber);

	// Set the file option flags for the file process
	switch (option)
	{
		case CREATE_EVENT_FILE: debug("File to create: %s\r\n", fileName); fileOption = (O_CREAT | O_WRONLY); break;
		case READ_EVENT_FILE: debug("File to read: %s\r\n", fileName); fileOption = O_RDONLY; break;
		case APPEND_EVENT_FILE: debug("File to append: %s\r\n", fileName); fileOption = O_APPEND; break;
		case OVERWRITE_EVENT_FILE: debug("File to overwrite: %s\r\n", fileName); fileOption = O_RDWR; break;
	}

	fileHandle = open(fileName, fileOption);

	// Check if trying to create a new event file and one already exists
	if ((fileHandle == -1) && (option == CREATE_EVENT_FILE))
	{
		// Delete the existing file and check if the operation was successful
		if (nav_file_del(TRUE) == TRUE)
		{
			fileHandle = open(fileName, fileOption);
		}
	}

	if (option == CREATE_EVENT_FILE)
	{
		SetFileDateTimestamp(FS_DATE_CREATION);
	}

	return (fileHandle);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetRamSummaryEntry(SUMMARY_DATA** sumEntryPtr)
{
	uint16 success = TRUE;

	if ((s_numOfFlashSummarys == LAST_RAM_SUMMARY_INDEX) && (s_noFlashWrapFlag == TRUE))
	{
		*sumEntryPtr = 0;
		success = FALSE;
	}
	else
	{
		// Check if we have reached the max num of summarys and wrapping is enabled
		if ((s_numOfFlashSummarys == LAST_RAM_SUMMARY_INDEX) && (s_noFlashWrapFlag == FALSE))
		{
			// Clear out oldest summary
			if (s_currFlashSummary == LAST_RAM_SUMMARY_INDEX)
			{
				__ramFlashSummaryTbl[0].fileEventNum = 0xFFFFFFFF;
			}
			else
			{
				__ramFlashSummaryTbl[s_currFlashSummary + 1].fileEventNum = 0xFFFFFFFF;
			}
		}

		// Set the flash pointer address in the ram summary
		__ramFlashSummaryTbl[s_currFlashSummary].fileEventNum = g_nextEventNumberToUse;

		// Set the current summary pointer to the current ram summary
		*sumEntryPtr = &__ramFlashSummaryTbl[s_currFlashSummary];

		debug("Ram Summary Table Entry %d: Event ID: %d\r\n", s_currFlashSummary, g_nextEventNumberToUse);

		// Increment the flash summary value.
		s_currFlashSummary++;

		if (s_currFlashSummary > LAST_RAM_SUMMARY_INDEX)
		{
			s_currFlashSummary = 0;
		}

		if (s_numOfFlashSummarys != LAST_RAM_SUMMARY_INDEX)
		{
			s_numOfFlashSummarys++;
		}
	}

	debug("Number of Event Summaries in RAM: %d\r\n", s_numOfFlashSummarys);

	return (success);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
inline void AdjustSampleForBitAccuracy(void)
{
	// Shift the sample data to adjust for the lower accuracy
	*(g_currentEventSamplePtr) >>= g_bitShiftForAccuracy;
	*(g_currentEventSamplePtr + 1) >>= g_bitShiftForAccuracy;
	*(g_currentEventSamplePtr + 2) >>= g_bitShiftForAccuracy;
	*(g_currentEventSamplePtr + 3) >>= g_bitShiftForAccuracy;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CompleteRamEventSummary(SUMMARY_DATA* ramSummaryPtr)
{
	//--------------------------------
	// Complete the Summary (used for Wave, Cal, Combo-Wave
	//--------------------------------

	if (g_pendingEventRecord.summary.mode == COMBO_MODE)
	{
		g_pendingEventRecord.summary.captured.comboEventsRecordedDuringSession = (g_pendingEventRecord.summary.eventNumber - g_pendingBargraphRecord.summary.eventNumber);
		g_pendingEventRecord.summary.captured.comboEventsRecordedStartNumber = (g_pendingBargraphRecord.summary.eventNumber + 1);
		g_pendingEventRecord.summary.captured.comboEventsRecordedEndNumber = g_pendingEventRecord.summary.eventNumber;
		g_pendingEventRecord.summary.captured.comboBargraphEventNumberLink = g_pendingBargraphRecord.summary.eventNumber;
	}

	debug("Copy calculated from Waveform buffer to global ram event record\r\n");
		
	// Fill in calculated data (Bargraph data filled in at the end of bargraph)
	g_pendingEventRecord.summary.calculated.a.peak = ramSummaryPtr->waveShapeData.a.peak;
	g_pendingEventRecord.summary.calculated.r.peak = ramSummaryPtr->waveShapeData.r.peak;
	g_pendingEventRecord.summary.calculated.v.peak = ramSummaryPtr->waveShapeData.v.peak;
	g_pendingEventRecord.summary.calculated.t.peak = ramSummaryPtr->waveShapeData.t.peak;

	debug("Newly stored peaks: a:%04x r:%04x v:%04x t:%04x\r\n", 
			g_pendingEventRecord.summary.calculated.a.peak, 
			g_pendingEventRecord.summary.calculated.r.peak,
			g_pendingEventRecord.summary.calculated.v.peak,
			g_pendingEventRecord.summary.calculated.t.peak);

	g_pendingEventRecord.summary.calculated.a.frequency = ramSummaryPtr->waveShapeData.a.freq;
	g_pendingEventRecord.summary.calculated.r.frequency = ramSummaryPtr->waveShapeData.r.freq;
	g_pendingEventRecord.summary.calculated.v.frequency = ramSummaryPtr->waveShapeData.v.freq;
	g_pendingEventRecord.summary.calculated.t.frequency = ramSummaryPtr->waveShapeData.t.freq;

	debug("Newly stored freq: a:%d r:%d v:%d t:%d\r\n", 
			g_pendingEventRecord.summary.calculated.a.frequency, 
			g_pendingEventRecord.summary.calculated.r.frequency,
			g_pendingEventRecord.summary.calculated.v.frequency,
			g_pendingEventRecord.summary.calculated.t.frequency);

	// Calculate Displacement as PPV/(2 * PI * Freq) with 1000000 to shift to keep accuracy and the 10 to adjust the frequency
	g_pendingEventRecord.summary.calculated.a.displacement = 0;

	if (ramSummaryPtr->waveShapeData.r.freq != 0)
	{
		g_pendingEventRecord.summary.calculated.r.displacement = (uint32)(ramSummaryPtr->waveShapeData.r.peak * 1000000 / 2 / PI / ramSummaryPtr->waveShapeData.r.freq * 10);
	}
	else { g_pendingEventRecord.summary.calculated.r.displacement = 0; }

	if (ramSummaryPtr->waveShapeData.v.freq != 0)
	{
		g_pendingEventRecord.summary.calculated.v.displacement = (uint32)(ramSummaryPtr->waveShapeData.v.peak * 1000000 / 2 / PI / ramSummaryPtr->waveShapeData.v.freq * 10);
	}
	else { g_pendingEventRecord.summary.calculated.v.displacement = 0; }

	if (ramSummaryPtr->waveShapeData.t.freq != 0)
	{
		g_pendingEventRecord.summary.calculated.t.displacement = (uint32)(ramSummaryPtr->waveShapeData.t.peak * 1000000 / 2 / PI / ramSummaryPtr->waveShapeData.t.freq * 10);
	}
	else { g_pendingEventRecord.summary.calculated.t.displacement = 0; }

	// Calculate Peak Acceleration as (2 * PI * PPV * Freq) / 1G, where 1G = 386.4in/sec2 or 9814.6 mm/sec2, using 1000 to shift to keep accuracy
	// The divide by 10 at the end to adjust the frequency, since freq stored as freq * 10
	// Not dividing by 1G at this time. Before displaying Peak Acceleration, 1G will need to be divided out
	g_pendingEventRecord.summary.calculated.a.acceleration = 0;

	g_pendingEventRecord.summary.calculated.r.acceleration = (uint32)(ramSummaryPtr->waveShapeData.r.peak * 1000 * 2 * PI * ramSummaryPtr->waveShapeData.r.freq / 10);
	g_pendingEventRecord.summary.calculated.v.acceleration = (uint32)(ramSummaryPtr->waveShapeData.v.peak * 1000 * 2 * PI * ramSummaryPtr->waveShapeData.v.freq / 10);
	g_pendingEventRecord.summary.calculated.t.acceleration = (uint32)(ramSummaryPtr->waveShapeData.t.peak * 1000 * 2 * PI * ramSummaryPtr->waveShapeData.t.freq / 10);

	//--------------------------------
	g_pendingEventRecord.header.summaryChecksum = 0;
	g_pendingEventRecord.header.dataChecksum = 0;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16* GetFlashDataPointer(void)
{
	return (s_flashDataPtr);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void GetSDCardUsageStats(void)
{
	uint32 waveSize;
	uint32 barSize;
	uint32 manualCalSize;

	waveSize = sizeof(EVT_RECORD) + (100 * 8) + (uint32)(g_triggerRecord.trec.sample_rate * 2) +
				(uint32)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time * 8);
	barSize = (uint32)(((3600 * 8) / g_triggerRecord.bgrec.barInterval) + (sizeof(EVT_RECORD) / 24) +
				((3600 * sizeof(CALCULATED_DATA_STRUCT)) / g_triggerRecord.bgrec.summaryInterval));
	manualCalSize = sizeof(EVT_RECORD) + (100 * 8);

#if NS8100_ORIGINAL_PROTOTYPE
	sd_mmc_spi_get_capacity();

	g_sdCardUsageStats.sizeUsed = 0;
	g_sdCardUsageStats.sizeFree = capacity - g_sdCardUsageStats.sizeUsed;
	g_sdCardUsageStats.percentUsed = (uint8)((g_sdCardUsageStats.sizeUsed * 100) / capacity);
	g_sdCardUsageStats.percentFree = (uint8)(100 - g_sdCardUsageStats.percentUsed);
#else // (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
	nav_select(FS_NAV_ID_DEFAULT);
	nav_drive_set(0);

	if (!nav_partition_mount()) // Mount drive
	{
		debugErr("SD MMC Card: Unable to mount volume\r\n");
	}

	g_sdCardUsageStats.clusterSizeInBytes = (fs_g_nav.u8_BPB_SecPerClus * SECTOR_SIZE_IN_BYTES);
	g_sdCardUsageStats.sizeFree = ((nav_partition_freespace() * SECTOR_SIZE_IN_BYTES) - RESERVED_FILESYSTEM_SIZE_IN_BYTES);
	g_sdCardUsageStats.sizeUsed = (nav_partition_space() * SECTOR_SIZE_IN_BYTES) - g_sdCardUsageStats.sizeFree;
	g_sdCardUsageStats.percentFree = (uint8)(nav_partition_freespace_percent());
	g_sdCardUsageStats.percentUsed = (uint8)(100 - g_sdCardUsageStats.percentFree);
#endif
	g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
	g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
	g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);

#if DISABLED_BUT_FIX_FOR_NS8100 // Fill in at some point
	if (g_unitConfig.flashWrapping == YES)
	{
		// Recalc parameters
		g_sdCardUsageStats.sizeUsed = (FLASH_EVENT_END - FLASH_EVENT_START) - ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.sizeFree = ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.percentUsed = (uint8)(((uint32)g_sdCardUsageStats.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
		g_sdCardUsageStats.percentFree = (uint8)(100 - g_sdCardUsageStats.percentUsed);
		g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
		g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
		g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateSDCardUsageStats(uint32 removeSize)
{
	uint32 removeSizeByAllocationUnit;
	uint32 waveSize;
	uint32 barSize;
	uint32 manualCalSize;
	uint32 s_addedSize = 0;

	// Adjust size down to a allocation unit (cluster) boundary
	removeSizeByAllocationUnit = ((removeSize / g_sdCardUsageStats.clusterSizeInBytes) * g_sdCardUsageStats.clusterSizeInBytes);

	// Check if remove size doesn't exactly match the allocation unit (cluster) size
	if (removeSize % g_sdCardUsageStats.clusterSizeInBytes)
	{
		// Remainder size fills up a partial allocation unit (cluster) but need to count the entire allocation unit (cluster)
		removeSizeByAllocationUnit += g_sdCardUsageStats.clusterSizeInBytes;
	}

	s_addedSize += removeSizeByAllocationUnit;

	// Check if the current running added size is above 100 MB or if the remaining size is less than 50 MB
	if ((s_addedSize > (100 * ONE_MEGABYTE_SIZE)) || (g_sdCardUsageStats.sizeFree < (50 * ONE_MEGABYTE_SIZE)))
	{
		// Reset added size in either case
		s_addedSize = 0;

		// Do official recalculation
		GetSDCardUsageStats();
	}
	else
	{
		waveSize = sizeof(EVT_RECORD) + (100 * 8) + (uint32)(g_triggerRecord.trec.sample_rate * 2) +
					(uint32)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time * 8);
		barSize = (uint32)(((3600 * 8) / g_triggerRecord.bgrec.barInterval) + (sizeof(EVT_RECORD) / 24) +
					((3600 * sizeof(CALCULATED_DATA_STRUCT)) / g_triggerRecord.bgrec.summaryInterval));
		manualCalSize = sizeof(EVT_RECORD) + (100 * 8);

		g_sdCardUsageStats.sizeFree -= removeSizeByAllocationUnit;
		g_sdCardUsageStats.sizeUsed += removeSizeByAllocationUnit;
		g_sdCardUsageStats.percentFree = (uint8)((float)((float)g_sdCardUsageStats.sizeFree / (float)(g_sdCardUsageStats.sizeFree + g_sdCardUsageStats.sizeUsed)) * 100);
		g_sdCardUsageStats.percentUsed = (uint8)(100 - g_sdCardUsageStats.percentFree);

		g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
		g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
		g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
	}

#if DISABLED_BUT_FIX_FOR_NS8100 // Fill in at some point
	if (g_unitConfig.flashWrapping == YES)
	{
		// Recalc parameters
		g_sdCardUsageStats.sizeUsed = (FLASH_EVENT_END - FLASH_EVENT_START) - ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.sizeFree = ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.percentUsed = (uint8)(((uint32)g_sdCardUsageStats.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
		g_sdCardUsageStats.percentFree = (uint8)(100 - g_sdCardUsageStats.percentUsed);
		g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
		g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
		g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int readWithSizeFix(int file, void* bufferPtr, uint32 length)
{
	uint32 remainingByteLengthToRead = length;
	uint8* readLocationPtr = (uint8*)bufferPtr;
	int readCount = 0;

	while (remainingByteLengthToRead)
	{
		if (remainingByteLengthToRead > ATMEL_FILESYSTEM_ACCESS_LIMIT)
		{
			readCount = read(file, readLocationPtr, ATMEL_FILESYSTEM_ACCESS_LIMIT);

			if (readCount != ATMEL_FILESYSTEM_ACCESS_LIMIT)
			{
				if (readCount != -1) { debugErr("Atmel Read Data size incorrect (%d)\r\n", readCount); }
				return (-1);
			}

			remainingByteLengthToRead -= ATMEL_FILESYSTEM_ACCESS_LIMIT;
			readLocationPtr += ATMEL_FILESYSTEM_ACCESS_LIMIT;
		}
		else // Remaining data size is less than the access limit
		{
			readCount = read(file, readLocationPtr, remainingByteLengthToRead);

			if (readCount != (int)remainingByteLengthToRead)
			{
				if (readCount != -1) { debugErr("Atmel Read Data size incorrect (%d)\r\n", readCount); }
				return (-1);
			}

			remainingByteLengthToRead = 0;
		}
	}

	return (length);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int writeWithSizeFix(int file, void* bufferPtr, uint32 length)
{
	uint32 remainingByteLengthToWrite = length;
	uint8* writeLocationPtr = (uint8*)bufferPtr;
	int writeCount = 0;

	while (remainingByteLengthToWrite)
	{
		if (remainingByteLengthToWrite > ATMEL_FILESYSTEM_ACCESS_LIMIT)
		{
			writeCount = write(file, writeLocationPtr, ATMEL_FILESYSTEM_ACCESS_LIMIT);

			if (writeCount != ATMEL_FILESYSTEM_ACCESS_LIMIT)
			{
				if (writeCount != -1) { debugErr("Atmel Read Data size incorrect (%d)\r\n", readCount); }
				return (-1);
			}

			remainingByteLengthToWrite -= ATMEL_FILESYSTEM_ACCESS_LIMIT;
			writeLocationPtr += ATMEL_FILESYSTEM_ACCESS_LIMIT;
		}
		else // Remaining data size is less than the access limit
		{
			writeCount = write(file, writeLocationPtr, remainingByteLengthToWrite);

			if (writeCount != (int)remainingByteLengthToWrite)
			{
				if (writeCount != -1) { debugErr("Atmel Read Data size incorrect (%d)\r\n", readCount); }
				return (-1);
			}

			remainingByteLengthToWrite = 0;
		}
	}

	return (length);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void SaveRemoteEventDownloadStreamToFile(uint16 eventNumber)
{
#if 1
	UNUSED(eventNumber);
#else // Test
	int fileHandle;
	uint32 remainingDataLength;
	uint32 bytesWritten = 0;

	// Split the buffer in half, and use the 2nd half for shadow copy
	uint8* dataPtr = (uint8*)&g_eventDataBuffer[(EVENT_BUFF_SIZE_IN_WORDS / 2)];

	// Get new event file handle
	fileHandle = GetEventFileHandle(eventNumber, CREATE_EVENT_FILE);

	if (fileHandle == -1)
	{
		debugErr("Failed to get a new file handle for the current %s event\r\n", (g_triggerRecord.opMode == WAVEFORM_MODE) ? "Waveform" : "Combo - Waveform");
	}
	else // Write the file event to the SD card
	{
		sprintf((char*)&g_spareBuffer[0], "EVENT #%d MODEM DL SHADOW COPY BEING SAVED...", eventNumber);
		OverlayMessage("SHADOW COPY", (char*)&g_spareBuffer[0], 0);

		remainingDataLength = g_spareIndex;

		while (remainingDataLength)
		{
			if (remainingDataLength > WAVEFORM_FILE_WRITE_CHUNK_SIZE)
			{
				// Write the event data, containing the Pretrigger, event and cal
				bytesWritten = write(fileHandle, dataPtr, WAVEFORM_FILE_WRITE_CHUNK_SIZE);

				if (bytesWritten != WAVEFORM_FILE_WRITE_CHUNK_SIZE)
				{
					debugErr("Remote Event Download to file write size incorrect (%d)\r\n", bytesWritten);
				}

				remainingDataLength -= WAVEFORM_FILE_WRITE_CHUNK_SIZE;
				dataPtr += (WAVEFORM_FILE_WRITE_CHUNK_SIZE);
			}
			else // Remaining data size is less than the file write chunk size
			{
				// Write the event data, containing the Pretrigger, event and cal
				bytesWritten = write(fileHandle, dataPtr, remainingDataLength);

				if (bytesWritten != remainingDataLength)
				{
					debugErr("Remote Event Download to file write size incorrect (%d)\r\n", bytesWritten);
				}

				remainingDataLength = 0;
			}
		}

		SetFileDateTimestamp(FS_DATE_LAST_WRITE);

		// Done writing the event file, close the file handle
		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(fileHandle);
		fat_cache_flush();
	}
#endif
}
