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
///	Function Break
///----------------------------------------------------------------------------
void InitMonitorLog(void)
{
	// Check if the table key is not valid or if the table index is not within range
	if ((__monitorLogTblKey != VALID_MONITOR_LOG_TABLE_KEY) || (__monitorLogTblIndex >= TOTAL_MONITOR_LOG_ENTRIES))
	{
		//DebugPrint(RAW, "Clearing Monitor Log table\n");

		// Clear Monitor Log table
		ByteSet(&__monitorLogTbl[0], 0x0, (sizeof(MONITOR_LOG_ENTRY_STRUCT) * TOTAL_MONITOR_LOG_ENTRIES));

		// Set the index to the first element
		__monitorLogTblIndex = 0;

		// Set the table key to be valid
		__monitorLogTblKey = VALID_MONITOR_LOG_TABLE_KEY;
		
		InitMonitorLogUniqueEntryId();

		InitMonitorLogTableFromLogFile();
	}
	// Check if the current monitor log entry is a partial entry (suggesting the entry was not closed)
	else if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		//DebugPrint(RAW, "Found partial entry at Monitor Log table index: 0x%x, Now making Incomplete\n", __monitorLogTblIndex);

		// Complete entry by setting abnormal termination
		__monitorLogTbl[__monitorLogTblIndex].status = INCOMPLETE_LOG_ENTRY;
	}

	//DebugPrint(RAW, "Monitor Log key: 0x%x, Monitor Log index: %d, Monitor Log Unique Entry Id: %d\n",
	//			__monitorLogTblKey, __monitorLogTblIndex, __monitorLogUniqueEntryId);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdvanceMonitorLogIndex(void)
{
	__monitorLogTblIndex++;
	if (__monitorLogTblIndex >= TOTAL_MONITOR_LOG_ENTRIES)
		__monitorLogTblIndex = 0;
		
	//DebugPrint(RAW, "Next Monitor Log table index: %d\n", __monitorLogTblIndex);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetStartingMonitorLogTableIndex(void)
{
	if ((__monitorLogTblIndex + (unsigned)1) >= TOTAL_MONITOR_LOG_ENTRIES)
		return (0);
	else
		return (uint16)(__monitorLogTblIndex + 1);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetStartingEventNumberForCurrentMonitorLog(void)
{
	return(__monitorLogTbl[__monitorLogTblIndex].startEventNumber);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ClearMonitorLogEntry(void)
{
	// Set all log entries to all zero's, status to EMPTY_LOG_ENTRY, start and stop times INVALID
	ByteSet(&__monitorLogTbl[__monitorLogTblIndex], 0x0, sizeof(MONITOR_LOG_ENTRY_STRUCT));

	//DebugPrint(RAW, "Clearing entry at Monitor Log table index: %d\n", __monitorLogTblIndex);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void NewMonitorLogEntry(uint8 mode)
{
	//char spareBuffer[40];

	// Advance to the next log entry
	AdvanceMonitorLogIndex();

	// Clear out the log entry (if wrapped)
	ClearMonitorLogEntry();

	//DebugPrint(RAW, "New Monitor Log entry with Unique Id: %d\n", __monitorLogUniqueEntryId);

	// Set the unique Monitor Log Entry number
	__monitorLogTbl[__monitorLogTblIndex].uniqueEntryId = __monitorLogUniqueEntryId;

	// Store the current entry number
	StoreMonitorLogUniqueEntryId();

	//DebugPrint(RAW, "Writing partial info to entry at Monitor Log table index: %d\n", __monitorLogTblIndex);

	// Set the elements to start a new log entry
	//DebugPrint(RAW, "Writing start time to Monitor Log entry at: 0x%x\n", &(__monitorLogTbl[__monitorLogTblIndex].startTime));
	__monitorLogTbl[__monitorLogTblIndex].startTime = GetCurrentTime();
	__monitorLogTbl[__monitorLogTblIndex].startTime.valid = TRUE;
	__monitorLogTbl[__monitorLogTblIndex].mode = mode;
	__monitorLogTbl[__monitorLogTblIndex].startEventNumber = g_nextEventNumberToUse;
	__monitorLogTbl[__monitorLogTblIndex].status = PARTIAL_LOG_ENTRY;
	
#if 0 // 16-bit trigger level
	__monitorLogTbl[__monitorLogTblIndex].seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
#else // Bit accuracy adjusted trigger level
	if ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.seismicTriggerLevel == EXTERNAL_TRIGGER_CHAR))
	{
		__monitorLogTbl[__monitorLogTblIndex].seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
	}
	else
	{
		__monitorLogTbl[__monitorLogTblIndex].seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);
	}
#endif

#if 0 // Unit specific trigger level
	__monitorLogTbl[__monitorLogTblIndex].airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
#else // Bit accuracy adjusted trigger level as A/D count
	if ((g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.airTriggerLevel == EXTERNAL_TRIGGER_CHAR))
	{
		__monitorLogTbl[__monitorLogTblIndex].airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
	}
	else
	{
		__monitorLogTbl[__monitorLogTblIndex].airTriggerLevel = g_triggerRecord.trec.airTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);
	}
#endif

#if 0 // Field no longer needed
	__monitorLogTbl[__monitorLogTblIndex].airUnitsOfMeasure = g_helpRecord.unitsOfAir;
#else
	__monitorLogTbl[__monitorLogTblIndex].bitAccuracy = ((g_triggerRecord.trec.bitAccuracy < ACCURACY_10_BIT) || (g_triggerRecord.trec.bitAccuracy > ACCURACY_16_BIT)) ?
														ACCURACY_16_BIT : g_triggerRecord.trec.bitAccuracy;
	__monitorLogTbl[__monitorLogTblIndex].adjustForTempDrift = g_triggerRecord.trec.adjustForTempDrift;
#endif

	__monitorLogTbl[__monitorLogTblIndex].sensor_type = g_factorySetupRecord.sensor_type;
	__monitorLogTbl[__monitorLogTblIndex].sensitivity = g_triggerRecord.srec.sensitivity;

	//ByteSet(&spareBuffer[0], 0x0, sizeof(spareBuffer));
	//ConvertTimeStampToString(&spareBuffer[0], &__monitorLogTbl[__monitorLogTblIndex].startTime, REC_DATE_TIME_TYPE);
	//debug("\tStart Time: %s\n", (char*)&spareBuffer[0]);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateMonitorLogEntry()
{
	//DebugPrint(RAW, "Updating entry at Monitor Log table index: %d, event#: %d, total: %d\n", __monitorLogTblIndex, g_nextEventNumberToUse,
	//			(uint16)(g_nextEventNumberToUse - __monitorLogTbl[__monitorLogTblIndex].startEventNumber + 1));

	if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		// Set the number of events recorded
		__monitorLogTbl[__monitorLogTblIndex].eventsRecorded = (uint16)(g_nextEventNumberToUse - __monitorLogTbl[__monitorLogTblIndex].startEventNumber + 1);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CloseMonitorLogEntry()
{
	//char spareBuffer[40];

	//debug("Closing entry at Monitor Log table index: %d, total recorded: %d\n", __monitorLogTblIndex, __monitorLogTbl[__monitorLogTblIndex].eventsRecorded);

	if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		// Set the elements to close the current log entry
		//debug("Writing stop time to Monitor Log entry at: 0x%x\n", &(__monitorLogTbl[__monitorLogTblIndex].stopTime));
		__monitorLogTbl[__monitorLogTblIndex].stopTime = GetCurrentTime();
		__monitorLogTbl[__monitorLogTblIndex].stopTime.valid = TRUE;
		__monitorLogTbl[__monitorLogTblIndex].status = COMPLETED_LOG_ENTRY;

		AppendMonitorLogEntryFile();

		//ByteSet(&spareBuffer[0], 0x0, sizeof(spareBuffer));
		//ConvertTimeStampToString(&spareBuffer[0], &__monitorLogTbl[__monitorLogTblIndex].stopTime, REC_DATE_TIME_TYPE);
		//debug("\tStop Time: %s\n", (char*)&spareBuffer[0]);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitMonitorLogUniqueEntryId(void)
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

	GetRecordData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);

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
		
		SaveRecordData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);
	}
#endif

	debug("Total Monitor Log entries to date: %d, Current Monitor Log entry number: %d\n", 
		(__monitorLogUniqueEntryId - 1), __monitorLogUniqueEntryId);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StoreMonitorLogUniqueEntryId(void)
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
		//SectorErase(uniqueEntryIdStorePtr, 1);
		EraseParameterMemory(uniqueEntryIdOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		
		InitMonitorLogUniqueEntryId();
	}
	// Check if at the boundary of a flash sectors worth of unique Monitor Log Entry numbers
	else if (((__monitorLogUniqueEntryId - 1) % 4096) == 0)
	{
		// Check to make sure this isnt the initial (first) Monitor Log Entry number
		if (__monitorLogUniqueEntryId != 1)
		{
			//SectorErase(uniqueEntryIdStorePtr, 1);
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
			//ProgramFlashWord(uniqueEntryIdStorePtr, __monitorLogUniqueEntryId);
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

	SaveRecordData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);

	// Increment to a new Monitor Log Entry number
	__monitorLogUniqueEntryId++;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 GetNextMonitorLogEntry(uint16 uid, uint16 startIndex, uint16* tempIndex, MONITOR_LOG_ENTRY_STRUCT* logEntry)
{
	uint8 found = NO;

	// Loop while a new entry has not been found and the temp index does not equal the start (wrapped & finished)
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

#if 0 // Test
			debug("(ID: %03d) M: %d, Evt#: %d, S: %d, ST: 0x%x, AT: 0x%x, BA: %d, TA: %d, ST: %d, G: %d\n",
			logEntry->uniqueEntryId, logEntry->mode, logEntry->startEventNumber, logEntry->status,
			logEntry->seismicTriggerLevel, logEntry->airTriggerLevel, logEntry->bitAccuracy, logEntry->adjustForTempDrift,
			logEntry->sensor_type, logEntry->sensitivity);
#endif

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
///	Function Break
///----------------------------------------------------------------------------
uint16 NumOfNewMonitorLogEntries(uint16 uid)
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
///	Function Break
///----------------------------------------------------------------------------
void AppendMonitorLogEntryFile(void)
{
	FL_FILE* monitorLogFile;
	
	monitorLogFile = fl_fopen("C:\\Logs\\MonitorLog.ns8", "a+");

	// Verify file ID
	if (monitorLogFile == NULL)
	{
		debugErr("Error: Monitor Log File not found!\r\n");
		OverlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLog.ns8", 3 * SOFT_SECS);
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
///	Function Break
///----------------------------------------------------------------------------
void InitMonitorLogTableFromLogFile(void)
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
		OverlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLog.ns8", 3 * SOFT_SECS);
	}		
	else // Monitor log file contains entries
	{
		OverlayMessage("MONITOR LOG", "INITIALIZING MONITOR LOG WITH SAVED ENTRIES", 1 * SOFT_SECS);

		bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
		
		// Loop while data continues to be read from MONITOR log file
		while (bytesRead > 0)
		{
			if ((monitorLogEntry.status == COMPLETED_LOG_ENTRY) || (monitorLogEntry.status == INCOMPLETE_LOG_ENTRY))
			{
				debug("Found Valid Monitor Log Entry with ID: %d\n", monitorLogEntry.uniqueEntryId);

#if 0 // Test
				debug("(ID: %03d) M: %d, Evt#: %d, S: %d, ST: 0x%x, AT: 0x%x, BA: %d, TA: %d, ST: %d, G: %d\n",
						monitorLogEntry.uniqueEntryId, monitorLogEntry.mode, monitorLogEntry. startEventNumber, monitorLogEntry.status, 
						monitorLogEntry.seismicTriggerLevel, monitorLogEntry.airTriggerLevel, monitorLogEntry.bitAccuracy, monitorLogEntry.adjustForTempDrift,
						monitorLogEntry.sensor_type, monitorLogEntry.sensitivity);
/*
typedef struct
{
	uint16				uniqueEntryId;
	uint8				status;
	uint8				mode;
	DATE_TIME_STRUCT	startTime;
	DATE_TIME_STRUCT	stopTime;
	uint16				eventsRecorded;
	uint16				startEventNumber;
	uint32				seismicTriggerLevel;
	uint32				airTriggerLevel;
	uint8				bitAccuracy;
	uint8				adjustForTempDrift;
	uint16				sensor_type;
	uint32				sensitivity;
} MONITOR_LOG_ENTRY_STRUCT;

	__monitorLogTbl[__monitorLogTblIndex].startTime = GetCurrentTime();
	__monitorLogTbl[__monitorLogTblIndex].startTime.valid = TRUE;
	__monitorLogTbl[__monitorLogTblIndex].mode = mode;
	__monitorLogTbl[__monitorLogTblIndex].startEventNumber = g_nextEventNumberToUse;
	__monitorLogTbl[__monitorLogTblIndex].status = PARTIAL_LOG_ENTRY;
	__monitorLogTbl[__monitorLogTblIndex].seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
	__monitorLogTbl[__monitorLogTblIndex].airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
	__monitorLogTbl[__monitorLogTblIndex].sensor_type =  g_factorySetupRecord.sensor_type;
	__monitorLogTbl[__monitorLogTblIndex].sensitivity = g_triggerRecord.srec.sensitivity;
*/
#endif
				__monitorLogTbl[__monitorLogTblIndex] = monitorLogEntry;
			
				AdvanceMonitorLogIndex();

				bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
			}
		}

		// Done reading, close the monitor log file
		fl_fclose(monitorLogFile);
	}
}

