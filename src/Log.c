///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Log.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:50 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Log.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include "Typedefs.h"
#include "Board.h"
#include "Record.h"
#include "Uart.h"
#include "Menu.h"
#include "SysEvents.h"
#include "Flash.h"
#include "TextTypes.h"
#include "FAT32_FileLib.h"

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

///----------------------------------------------------------------------------
///	Function:	initMonitorLog
///	Purpose:	
///----------------------------------------------------------------------------
void initMonitorLog(void)
{
	// Check if the table key is not valid or if the table index is not within range
	if ((__monitorLogTblKey != VALID_MONITOR_LOG_TABLE_KEY) || (__monitorLogTblIndex >= TOTAL_MONITOR_LOG_ENTRIES))
	{
		//debugPrint(RAW, "Clearing Monitor Log table\n");

		// Clear Monitor Log table
		byteSet(&__monitorLogTbl[0], 0x0, (sizeof(MONITOR_LOG_ENTRY_STRUCT) * TOTAL_MONITOR_LOG_ENTRIES));

		// Set the index to the first element
		__monitorLogTblIndex = 0;

		// Set the table key to be valid
		__monitorLogTblKey = VALID_MONITOR_LOG_TABLE_KEY;
		
		initMonitorLogUniqueEntryId();

		initMonitorLogTableFromLogFile();
	}
	// Check if the current monitor log entry is a partial entry (suggesting the entry was not closed)
	else if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		//debugPrint(RAW, "Found partial entry at Monitor Log table index: 0x%x, Now making Incomplete\n", __monitorLogTblIndex);

		// Complete entry by setting abnormal termination
		__monitorLogTbl[__monitorLogTblIndex].status = INCOMPLETE_LOG_ENTRY;
	}

	//debugPrint(RAW, "Monitor Log key: 0x%x, Monitor Log index: %d, Monitor Log Unique Entry Id: %d\n", 
	//			__monitorLogTblKey, __monitorLogTblIndex, __monitorLogUniqueEntryId);
}

///----------------------------------------------------------------------------
///	Function:	advanceMonitorLogIndex
///	Purpose:	
///----------------------------------------------------------------------------
void advanceMonitorLogIndex(void)
{
	__monitorLogTblIndex++;
	if (__monitorLogTblIndex >= TOTAL_MONITOR_LOG_ENTRIES)
		__monitorLogTblIndex = 0;
		
	//debugPrint(RAW, "Next Monitor Log table index: %d\n", __monitorLogTblIndex);
}

///----------------------------------------------------------------------------
///	Function:	getStartingMonitorLogTableIndex
///	Purpose:	
///----------------------------------------------------------------------------
uint16 getStartingMonitorLogTableIndex(void)
{
	if ((__monitorLogTblIndex + 1) >= TOTAL_MONITOR_LOG_ENTRIES)
		return (0);
	else
		return (uint16)(__monitorLogTblIndex + 1);
}

//*****************************************************************************
// Function:	getStartingEventNumberForCurrentMonitorLog
// Purpose:		
//*****************************************************************************
uint16 getStartingEventNumberForCurrentMonitorLog(void)
{
	return(__monitorLogTbl[__monitorLogTblIndex].startEventNumber);
}

//*****************************************************************************
// Function:	clearMonitorLogEntry
// Purpose:		
//*****************************************************************************
void clearMonitorLogEntry(void)
{
	// Set all log entries to all zero's, status to EMPTY_LOG_ENTRY, start and stop times INVALID
	byteSet(&__monitorLogTbl[__monitorLogTblIndex], 0x0, sizeof(MONITOR_LOG_ENTRY_STRUCT));

	//debugPrint(RAW, "Clearing entry at Monitor Log table index: %d\n", __monitorLogTblIndex);
}

///----------------------------------------------------------------------------
///	Function:	newMonitorLogEntry
///	Purpose:	
///----------------------------------------------------------------------------
void newMonitorLogEntry(uint8 mode)
{
	char spareBuffer[40];

	// Advance to the next log entry
	advanceMonitorLogIndex();

	// Clear out the log entry (if wrapped)
	clearMonitorLogEntry();

	//debugPrint(RAW, "New Monitor Log entry with Unique Id: %d\n", __monitorLogUniqueEntryId);

	// Set the unique Monitor Log Entry number
	__monitorLogTbl[__monitorLogTblIndex].uniqueEntryId = __monitorLogUniqueEntryId;

	// Store the current entry number
	storeMonitorLogUniqueEntryId();

	//debugPrint(RAW, "Writing partial info to entry at Monitor Log table index: %d\n", __monitorLogTblIndex);

	// Set the elements to start a new log entry
	//debugPrint(RAW, "Writing start time to Monitor Log entry at: 0x%x\n", &(__monitorLogTbl[__monitorLogTblIndex].startTime));
	__monitorLogTbl[__monitorLogTblIndex].startTime = getCurrentTime();
	__monitorLogTbl[__monitorLogTblIndex].startTime.valid = TRUE;
	__monitorLogTbl[__monitorLogTblIndex].mode = mode;
	__monitorLogTbl[__monitorLogTblIndex].startEventNumber = g_nextEventNumberToUse;
	__monitorLogTbl[__monitorLogTblIndex].status = PARTIAL_LOG_ENTRY;
	
	byteSet(&spareBuffer[0], 0x0, sizeof(spareBuffer));
	convertTimeStampToString(&spareBuffer[0], &__monitorLogTbl[__monitorLogTblIndex].startTime, REC_DATE_TIME_TYPE);
}

///----------------------------------------------------------------------------
///	Function:	updateMonitorLogEntry
///	Purpose:	
///----------------------------------------------------------------------------
void updateMonitorLogEntry()
{
	//debugPrint(RAW, "Updating entry at Monitor Log table index: %d, event#: %d, total: %d\n", __monitorLogTblIndex, g_nextEventNumberToUse, 
	//			(uint16)(g_nextEventNumberToUse - __monitorLogTbl[__monitorLogTblIndex].startEventNumber + 1));

	if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		// Set the elements to close the current log entry
		//__monitorLogTbl[__monitorLogTblIndex].endEventNumber = g_nextEventNumberToUse;
		__monitorLogTbl[__monitorLogTblIndex].eventsRecorded = (uint16)(g_nextEventNumberToUse - __monitorLogTbl[__monitorLogTblIndex].startEventNumber + 1);
	}
}

///----------------------------------------------------------------------------
///	Function:	closeMonitorLogEntry
///	Purpose:	
///----------------------------------------------------------------------------
void closeMonitorLogEntry()
{
	char spareBuffer[40];

	//debug("Closing entry at Monitor Log table index: %d, total recorded: %d\n", __monitorLogTblIndex, __monitorLogTbl[__monitorLogTblIndex].eventsRecorded);

	if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		// Set the elements to close the current log entry
		//debug("Writing stop time to Monitor Log entry at: 0x%x\n", &(__monitorLogTbl[__monitorLogTblIndex].stopTime));
		__monitorLogTbl[__monitorLogTblIndex].stopTime = getCurrentTime();
		__monitorLogTbl[__monitorLogTblIndex].stopTime.valid = TRUE;
		__monitorLogTbl[__monitorLogTblIndex].status = COMPLETED_LOG_ENTRY;

		appendMonitorLogEntryFile();

		byteSet(&spareBuffer[0], 0x0, sizeof(spareBuffer));
		convertTimeStampToString(&spareBuffer[0], &__monitorLogTbl[__monitorLogTblIndex].stopTime, REC_DATE_TIME_TYPE);
		//debug("\tStop Time: %s\n", (char*)&spareBuffer[0]);
	}
}

///----------------------------------------------------------------------------
///	Function:	initMonitorLogUniqueEntryId
///	Purpose:	
///----------------------------------------------------------------------------
void initMonitorLogUniqueEntryId(void)
{
#if 0 // ns7100
	uint16 uniqueEntryIdOffset = (FLASH_BOOT_SECTOR_SIZE_x8 + 2);
	uint16 uniqueEntryId = 0;
	uint16 uniqueEntryIdOffset = (FLASH_BOOT_SECTOR_SIZE_x8 * 2);

	// Get the end address (Start of the 4th boot sector)
	//uint16* endPtr = (uint16*)(FLASH_BASE_ADDR + (FLASH_BOOT_SECTOR_SIZE_x8 * 3));
	uint16 endOffset = (FLASH_BOOT_SECTOR_SIZE_x8 * 3);
		
	GetParameterMemory((uint8*)&uniqueEntryId, uniqueEntryIdOffset, sizeof(uniqueEntryId));

	// Check if the first location is 0xFF which indicates the unit hasn't recorded any monitor log entries
	if (uniqueEntryId == 0xFFFF)
	{
		__monitorLogUniqueEntryId = 1;
	}
	else // Find the last stored Monitor Log entry number location
	{
		GetParameterMemory((uint8*)&uniqueEntryId, (uint16)(uniqueEntryIdOffset + 1), sizeof(uniqueEntryId));

		// Loop until the next location is empty and not past the end
		while ((uniqueEntryId != 0xFFFF) && ((uniqueEntryIdOffset + 1) < endOffset))
		{
			// Increment address
			uniqueEntryIdOffset++;

			GetParameterMemory((uint8*)&uniqueEntryId, (uint16)(uniqueEntryIdOffset + 1), sizeof(uniqueEntryId));
		}

		GetParameterMemory((uint8*)&uniqueEntryId, uniqueEntryIdOffset, sizeof(uniqueEntryId));

		// Set the Monitor Log Entry number to the last stored Monitor Log number stored plus 1
		__monitorLogUniqueEntryId = (uint16)(uniqueEntryId + 1);
	}
#else // ns8100
	MONITOR_LOG_ID_STRUCT monitorLogRec;

	getRecData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);

	// Check if the Monitor Log Record is valid
	if (!monitorLogRec.invalid)
	{
		if (monitorLogRec.currentMonitorLogID == 0xFFFF)
		{
			__monitorLogUniqueEntryId = 1;
		}
		else
		{
			// Set the Current Event number to the last event number stored plus 1
			__monitorLogUniqueEntryId = (monitorLogRec.currentMonitorLogID + 1);
		}
	}
	else // record is invalid
	{
		__monitorLogUniqueEntryId = 1;
		monitorLogRec.currentMonitorLogID = __monitorLogUniqueEntryId;
		
		saveRecData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);
	}
#endif

	debug("Total Monitor Log entries to date: %d, Current Monitor Log entry number: %d\n", 
		(__monitorLogUniqueEntryId - 1), __monitorLogUniqueEntryId);
}

///----------------------------------------------------------------------------
///	Function:	storeMonitorLogUniqueEntryId
///	Purpose:	
///----------------------------------------------------------------------------
void storeMonitorLogUniqueEntryId(void)
{
#if 0 // ns7100
	uint16 uniqueEntryIdOffset = (FLASH_BOOT_SECTOR_SIZE_x8 + 2);

	// Get the starting address of the 3rd boot sector (base + boot sector size * 2)
	//uint16* uniqueEntryIdStorePtr = (uint16*)(FLASH_BASE_ADDR + (FLASH_BOOT_SECTOR_SIZE_x8 * 2));
	uint16 uniqueEntryIdOffset = (FLASH_BOOT_SECTOR_SIZE_x8 * 2);

	uint16 positionsToAdvance = 0;
	uint16 uniqueEntryId = 0;

	// If the Monitor Log Entry number is 0, then we have wrapped (>65535) in which case we will reinitialize
	if (__monitorLogUniqueEntryId == 0)
	{
		//sectorErase(uniqueEntryIdStorePtr, 1);
		EraseParameterMemory(uniqueEntryIdOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		
		initMonitorLogUniqueEntryId();
	}
	// Check if at the boundary of a flash sectors worth of unique Monitor Log Entry numbers
	else if (((__monitorLogUniqueEntryId - 1) % 4096) == 0)
	{
		// Check to make sure this isnt the initial (first) Monitor Log Entry number
		if (__monitorLogUniqueEntryId != 1)
		{
			//sectorErase(uniqueEntryIdStorePtr, 1);
			EraseParameterMemory(uniqueEntryIdOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		}	
	}
	else // Still room to store unique Monitor Log Entry numbers
	{
		// Get the positions to advance based on the current monitor log unique id mod'ed by the storage size in id numbers
		positionsToAdvance = (uint16)((__monitorLogUniqueEntryId - 1) % 4096);

		// Set the offset to the event number positions adjusted to bytes
		uniqueEntryIdOffset += (positionsToAdvance * 2);
	}

	GetParameterMemory((uint8*)&uniqueEntryId, uniqueEntryIdOffset, sizeof(uniqueEntryId));

	// Check to make sure Current location is empty
	if (uniqueEntryId == 0xFFFF)
	{
		GetParameterMemory((uint8*)&uniqueEntryId, (uint16)(uniqueEntryIdOffset - 1), sizeof(uniqueEntryId));
	
		// Check if the first location to be used or if a valid unique Monitor Log Entry number
		// preceeded the current Monitor Log Entry number to be stored
		if ((positionsToAdvance == 0) || (uniqueEntryId != 0xFFFF))
		{
			// Store the current Monitor Log Entry number as the newest unique entry number
			//programWord(uniqueEntryIdStorePtr, __monitorLogUniqueEntryId);
			SaveParameterMemory((uint8*)&__monitorLogUniqueEntryId, uniqueEntryIdOffset, sizeof(__monitorLogUniqueEntryId));
			
			// Increment to a new Monitor Log Entry number
			__monitorLogUniqueEntryId++;
			
			return;
		}
	}

	// If we get here, then we failed a validation check
	debugErr("Unique Monitor Log Entry number storage doesnt match Current Monitor Log Entry number. (0x%x, %d)\n", 
				uniqueEntryIdOffset, __monitorLogUniqueEntryId);
#else // ns8100
	MONITOR_LOG_ID_STRUCT monitorLogRec;

	monitorLogRec.currentMonitorLogID = __monitorLogUniqueEntryId;

	saveRecData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);

	// Increment to a new Monitor Log Entry number
	__monitorLogUniqueEntryId++;
#endif
}

///----------------------------------------------------------------------------
///	Function:	getNextMonitorLogEntry
///	Purpose:	
///----------------------------------------------------------------------------
uint8 getNextMonitorLogEntry(uint16 uid, uint16 startIndex, uint16* tempIndex, MONITOR_LOG_ENTRY_STRUCT* logEntry)
{
	uint8 found = NO;

	// Loop while a new entry has not been found and the temp index does not equal the start (wrapped & finsished)
	while ((found == NO) && ((*tempIndex) != startIndex))
	{
		// Check if initial condition
		if ((*tempIndex) == TOTAL_MONITOR_LOG_ENTRIES)
		{
			// Set the temp index to the start index
			*tempIndex = startIndex;
		}

		// Check if the entry at the temp index is complete or incomplete and newer than the comparison unique entry Id 
		if (((__monitorLogTbl[(*tempIndex)].status == COMPLETED_LOG_ENTRY) || 
			(__monitorLogTbl[(*tempIndex)].status == INCOMPLETE_LOG_ENTRY)) && 
			(__monitorLogTbl[(*tempIndex)].uniqueEntryId > uid))
		{
			// Copy the monitor log entry over to the log entry buffer
			*logEntry = __monitorLogTbl[(*tempIndex)];

			// Set the found flag to mark that an entry was discovered
			found = TRUE;
		}

		// Increment the temp index
		(*tempIndex)++;
		if (*tempIndex >= TOTAL_MONITOR_LOG_ENTRIES)
			*tempIndex = 0;
	}
	
	return (found);
}

///----------------------------------------------------------------------------
///	Function:	numOfNewMonitorLogEntries
///	Purpose:	
///----------------------------------------------------------------------------
uint16 numOfNewMonitorLogEntries(uint16 uid)
{
	uint16 total = 0;
	uint16 i = 0;

	//uint16 startIndex = __monitorLogTblIndex;
	//uint16 tempIndex = __monitorLogTblIndex;
	
	for (i = 0; i < TOTAL_MONITOR_LOG_ENTRIES; i++)
	{
		if(((__monitorLogTbl[i].status == COMPLETED_LOG_ENTRY) ||
			(__monitorLogTbl[i].status == INCOMPLETE_LOG_ENTRY)) && 
			(__monitorLogTbl[i].uniqueEntryId > uid))
		{
			total++;
		}
	}

	return (total);
}

///----------------------------------------------------------------------------
///	Function:	appendMonitorLogEntryFile
///	Purpose:	
///----------------------------------------------------------------------------
void appendMonitorLogEntryFile(void)
{
	FL_FILE* monitorLogFile;
	
	monitorLogFile = fl_fopen("C:\\Logs\\MonitorLog.ns8", "a+");

	// Verify file ID
	if (monitorLogFile == NULL)
	{
		debugErr("Error: Monitor Log File not found!\r\n");
		overlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLog.ns8", 3 * SOFT_SECS);
	}		
	else // Monitor log file contains entries
	{
		if (monitorLogFile->filelength % sizeof(MONITOR_LOG_ENTRY_STRUCT) != 0)
		{
			debugWarn("Warning: Monitor Log File size does not comprise all whole entries!\r\n");
		}

		debug("Writing Monitor log entry to log file...\n");

		fl_fwrite((uint8*)&(__monitorLogTbl[__monitorLogTblIndex]), sizeof(MONITOR_LOG_ENTRY_STRUCT), 1, monitorLogFile);

		// Done reading, close the monitor log file
		fl_fclose(monitorLogFile);
		
		debug("Monitor log entry appended to log file\n");
	}
}

///----------------------------------------------------------------------------
///	Function:	initMonitorLogTableFromLogFile
///	Purpose:	
///----------------------------------------------------------------------------
void initMonitorLogTableFromLogFile(void)
{
	FL_FILE* monitorLogFile;
	MONITOR_LOG_ENTRY_STRUCT monitorLogEntry;
	int32 bytesRead = 0;
	
	monitorLogFile = fl_fopen("C:\\Logs\\MonitorLog.ns8", "r");

	// Verify file ID
	if (monitorLogFile == NULL)
	{
		debugWarn("Warning: Monitor Log File not found or has not yet been created\r\n");
	}		
	// Verify file is big enough
	else if (monitorLogFile->filelength < sizeof(MONITOR_LOG_ENTRY_STRUCT))
	{
		debugErr("Error: Monitor Log File is corrupt!\r\n");
		overlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLog.ns8", 3 * SOFT_SECS);
	}		
	else // Monitor log file contains entries
	{
		overlayMessage("MONITOR LOG", "INITIALIZING MONITOR LOG WITH SAVED ENTRIES", 1 * SOFT_SECS);

		bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
		
		// Loop while data continues to be read from MONITOR log file
		while (bytesRead > 0)
		{
			if ((monitorLogEntry.status == COMPLETED_LOG_ENTRY) || (monitorLogEntry.status == INCOMPLETE_LOG_ENTRY))
			{
				debug("Found Valid Monitor Log Entry with ID: %d\n", monitorLogEntry.uniqueEntryId);

				__monitorLogTbl[__monitorLogTblIndex] = monitorLogEntry;
			
				advanceMonitorLogIndex();

				bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
			}
		}

		// Done reading, close the monitor log file
		fl_fclose(monitorLogFile);
	}
}

