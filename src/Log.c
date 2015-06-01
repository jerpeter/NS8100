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
#include "TextTypes.h"
#include "FAT32_FileLib.h"
#include "string.h"

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
		//debugRaw("Clearing Monitor Log table\r\n");

		// Clear Monitor Log table
		memset(&__monitorLogTbl[0], 0, sizeof(__monitorLogTbl));

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
		//debugRaw("Found partial entry at Monitor Log table index: 0x%x, Now making Incomplete\r\n", __monitorLogTblIndex);

		// Complete entry by setting abnormal termination
		__monitorLogTbl[__monitorLogTblIndex].status = INCOMPLETE_LOG_ENTRY;
	}

	//debugRaw("Monitor Log key: 0x%x, Monitor Log index: %d, Monitor Log Unique Entry Id: %d\r\n",
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
		
	//debugRaw("Next Monitor Log table index: %d\r\n", __monitorLogTblIndex);
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
	memset(&__monitorLogTbl[__monitorLogTblIndex], 0, sizeof(MONITOR_LOG_ENTRY_STRUCT));

	//debugRaw("Clearing entry at Monitor Log table index: %d\r\n", __monitorLogTblIndex);
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

	//debugRaw("New Monitor Log entry with Unique Id: %d\r\n", __monitorLogUniqueEntryId);

	// Set the unique Monitor Log Entry number
	__monitorLogTbl[__monitorLogTblIndex].uniqueEntryId = __monitorLogUniqueEntryId;

	// Store the current entry number
	StoreMonitorLogUniqueEntryId();

	//debugRaw("Writing partial info to entry at Monitor Log table index: %d\r\n", __monitorLogTblIndex);

	// Set the elements to start a new log entry
	//debugRaw("Writing start time to Monitor Log entry at: 0x%x\r\n", &(__monitorLogTbl[__monitorLogTblIndex].startTime));
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
	__monitorLogTbl[__monitorLogTblIndex].airUnitsOfMeasure = g_unitConfig.unitsOfAir;
#else
	__monitorLogTbl[__monitorLogTblIndex].bitAccuracy = ((g_triggerRecord.trec.bitAccuracy < ACCURACY_10_BIT) || (g_triggerRecord.trec.bitAccuracy > ACCURACY_16_BIT)) ?
														ACCURACY_16_BIT : g_triggerRecord.trec.bitAccuracy;
	__monitorLogTbl[__monitorLogTblIndex].adjustForTempDrift = g_triggerRecord.trec.adjustForTempDrift;
#endif

	__monitorLogTbl[__monitorLogTblIndex].sensor_type = g_factorySetupRecord.sensor_type;
	__monitorLogTbl[__monitorLogTblIndex].sensitivity = g_triggerRecord.srec.sensitivity;

	//memset(&spareBuffer[0], 0, sizeof(spareBuffer));
	//ConvertTimeStampToString(&spareBuffer[0], &__monitorLogTbl[__monitorLogTblIndex].startTime, REC_DATE_TIME_TYPE);
	//debug("\tStart Time: %s\r\n", (char*)&spareBuffer[0]);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateMonitorLogEntry()
{
	//debugRaw("Updating entry at Monitor Log table index: %d, event#: %d, total: %d\r\n", __monitorLogTblIndex, g_nextEventNumberToUse,
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

	//debug("Closing entry at Monitor Log table index: %d, total recorded: %d\r\n", __monitorLogTblIndex, __monitorLogTbl[__monitorLogTblIndex].eventsRecorded);

#if 0 // Reverting back to the original method of saving the event number after every event
	CURRENT_EVENT_NUMBER_STRUCT currentEventNumberRecord;

	// Check if the mode was Waveform which now postpones saving the current event number
	if (g_triggerRecord.op_mode == WAVEFORM_MODE)
	{
		// Set the current event number to be saved
		currentEventNumberRecord.currentEventNumber = (g_nextEventNumberToUse - 1);

		SaveRecordData(&currentEventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);

		debug("Saved Event ID: %d\r\n", (g_nextEventNumberToUse - 1));
	}
#endif

	if (__monitorLogTbl[__monitorLogTblIndex].status == PARTIAL_LOG_ENTRY)
	{
		// Set the elements to close the current log entry
		//debug("Writing stop time to Monitor Log entry at: 0x%x\r\n", &(__monitorLogTbl[__monitorLogTblIndex].stopTime));
		__monitorLogTbl[__monitorLogTblIndex].stopTime = GetCurrentTime();
		__monitorLogTbl[__monitorLogTblIndex].stopTime.valid = TRUE;
		__monitorLogTbl[__monitorLogTblIndex].status = COMPLETED_LOG_ENTRY;

		AppendMonitorLogEntryFile();

		//memset(&spareBuffer[0], 0, sizeof(spareBuffer));
		//ConvertTimeStampToString(&spareBuffer[0], &__monitorLogTbl[__monitorLogTblIndex].stopTime, REC_DATE_TIME_TYPE);
		//debug("\tStop Time: %s\r\n", (char*)&spareBuffer[0]);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitMonitorLogUniqueEntryId(void)
{
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

	debug("Total Monitor Log entries to date: %d, Current Monitor Log entry number: %d\r\n",
		(__monitorLogUniqueEntryId - 1), __monitorLogUniqueEntryId);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StoreMonitorLogUniqueEntryId(void)
{
	MONITOR_LOG_ID_STRUCT monitorLogRec;

	monitorLogRec.currentMonitorLogID = __monitorLogUniqueEntryId;

	SaveRecordData(&monitorLogRec, DEFAULT_RECORD, REC_UNIQUE_MONITOR_LOG_ID_TYPE);

	// Increment to a new Monitor Log Entry number
	__monitorLogUniqueEntryId++;
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
			debug("(ID: %03d) M: %d, Evt#: %d, S: %d, ST: 0x%x, AT: 0x%x, BA: %d, TA: %d, ST: %d, G: %d\r\n",
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
#include "fsaccess.h"
static char s_monitorLogFilename[] = "A:\\Logs\\MonitorLog.ns8";
static char s_monitorLogHumanReadableFilename[] = "A:\\Logs\\MonitorLogReadable.txt";
//static char testBufferFilename[] = "A:\\Logs\\TestBuffer.txt";
void AppendMonitorLogEntryFile(void)
{
	char modeString[10];
	char statusString[10];
	char startTimeString[20];
	char stopTimeString[20];
	char seisString[15];
	char airString[15];
	char sensorString[10];
	MONITOR_LOG_ENTRY_STRUCT *mle;
	float tempSesmicTriggerInUnits;
	float unitsDiv;
	uint32 airInUnits;
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Add monitor log entry");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

#if 1 // Atmel fat driver
		int monitorLogFile;
		monitorLogFile = open(s_monitorLogFilename, O_APPEND);
		if (monitorLogFile == -1)
		{
			nav_setcwd(s_monitorLogFilename, TRUE, TRUE);
			monitorLogFile = open(s_monitorLogFilename, O_APPEND);
		}
#else // Port fat driver
		FL_FILE* monitorLogFile;
		monitorLogFile = fl_fopen("C:\\Logs\\MonitorLog.ns8", "a+");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (monitorLogFile == -1)
#else // Port fat driver
		if (monitorLogFile == NULL)
#endif
		{
			debugErr("Monitor Log File not found\r\n");
			OverlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLog.ns8", 3 * SOFT_SECS);
		}
		else // Monitor log file contains entries
		{
			if (fsaccess_file_get_size(monitorLogFile) % sizeof(MONITOR_LOG_ENTRY_STRUCT) != 0)
			{
				debugWarn("Monitor Log File size does not comprise all whole entries\r\n");
			}

			debug("Writing Monitor log entry to log file...\r\n");

#if 1 // Atmel fat driver
			write(monitorLogFile, (uint8*)&(__monitorLogTbl[__monitorLogTblIndex]), sizeof(MONITOR_LOG_ENTRY_STRUCT));

			SetFileDateTimestamp(FS_DATE_LAST_WRITE);

			// Done writing, close the monitor log file
			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(monitorLogFile);
#else // Port fat driver
			fl_fwrite((uint8*)&(__monitorLogTbl[__monitorLogTblIndex]), sizeof(MONITOR_LOG_ENTRY_STRUCT), 1, monitorLogFile);

			// Done writing, close the monitor log file
			fl_fclose(monitorLogFile);
#endif

			debug("Monitor log entry appended to log file\r\n");
		}

#if 1 // Atmel fat driver
		int monitorLogHumanReadableFile;
		monitorLogHumanReadableFile = open(s_monitorLogHumanReadableFilename, O_APPEND);
		if (monitorLogHumanReadableFile == -1)
		{
			nav_setcwd(s_monitorLogHumanReadableFilename, TRUE, TRUE);
			monitorLogHumanReadableFile = open(s_monitorLogHumanReadableFilename, O_APPEND);
		}
#else // Port fat driver
		FL_FILE* monitorLogHumanReadableFile;
		monitorLogHumanReadableFile = fl_fopen("C:\\Logs\\MonitorLogReadable.txt", "a+");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (monitorLogHumanReadableFile == -1)
#else // Port fat driver
		if (monitorLogHumanReadableFile == NULL)
#endif
		{
			debugErr("Monitor Log Readable File not found\r\n");
			OverlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLogReadable.txt", 3 * SOFT_SECS);
		}
		else // File successfully created or opened
		{
			mle = &__monitorLogTbl[__monitorLogTblIndex];

			debug("Writing Monitor log entry to readable log file...\r\n");

			if (mle->mode == WAVEFORM_MODE) { strcpy((char*)&modeString, "Waveform"); }
			else if (mle->mode == BARGRAPH_MODE) { strcpy((char*)&modeString, "Bargraph"); }
			else if (mle->mode == COMBO_MODE) { strcpy((char*)&modeString, "Combo"); }

			if (mle->status == COMPLETED_LOG_ENTRY) { strcpy((char*)&statusString, "Completed"); }
			else if (mle->status == PARTIAL_LOG_ENTRY) { strcpy((char*)&statusString, "Partial"); }
			else if (mle->status == INCOMPLETE_LOG_ENTRY) { strcpy((char*)&statusString, "Incomplete"); }

			sprintf((char*)&startTimeString, "%02d-%02d-%02d %02d:%02d:%02d", mle->startTime.day, mle->startTime.month, mle->startTime.year,
					mle->startTime.hour, mle->startTime.min, mle->startTime.sec);
			sprintf((char*)&stopTimeString, "%02d-%02d-%02d %02d:%02d:%02d", mle->stopTime.day, mle->stopTime.month, mle->stopTime.year,
					mle->stopTime.hour, mle->stopTime.min, mle->stopTime.sec);

			if (g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) {strcpy((char*)seisString, "None"); }
			else
			{
				// Calculate the divider used for converting stored A/D peak counts to units of measure
				unitsDiv = (float)(g_bitAccuracyMidpoint * SENSOR_ACCURACY_100X_SHIFT * ((g_triggerRecord.srec.sensitivity == LOW) ? 2 : 4)) /
				(float)(g_factorySetupRecord.sensor_type);

				tempSesmicTriggerInUnits = (float)(g_triggerRecord.trec.seismicTriggerLevel >> g_bitShiftForAccuracy) / (float)unitsDiv;
				if ((g_factorySetupRecord.sensor_type != SENSOR_ACC) && (g_unitConfig.unitsOfMeasure == METRIC_TYPE)) { tempSesmicTriggerInUnits *= (float)METRIC; }

				sprintf((char*)seisString, "%05.2f %s", tempSesmicTriggerInUnits, (g_unitConfig.unitsOfMeasure == METRIC_TYPE ? "mm" : "in"));
			}

			if (g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR) {strcpy((char*)airString, "None"); }
			else
			{
				airInUnits = AirTriggerConvertToUnits(g_triggerRecord.trec.airTriggerLevel);
				if (g_unitConfig.unitsOfAir == MILLIBAR_TYPE) { sprintf((char*)airString, "%05.3f mB", ((float)airInUnits / 10000)); }
				else { sprintf((char*)airString, "%d dB", (uint16)airInUnits); }
			}

			if (g_factorySetupRecord.sensor_type == SENSOR_ACC) { strcpy((char*)&sensorString, "Acc"); }
			else { sprintf((char*)&sensorString, "%3.1f in", (float)g_factorySetupRecord.sensor_type / (float)204.8); }

			sprintf((char*)&g_spareBuffer, "Log ID: %03d --> Status: %10s, Mode: %8s, Start Time: %s, Stop Time: %s\r\n\tEvents: %3d, Start Evt #: %4d, "\
					"Seismic Trig: %10s, Air Trig: %11s\r\n\tBit Acc: %d, Temp Adjust: %3s, Sensor: %8s, Sensitivity: %4s\r\n\n",
					mle->uniqueEntryId, (char*)statusString, (char*)modeString, (char*)startTimeString, (char*)stopTimeString, mle->eventsRecorded, mle->startEventNumber,
					(char*)seisString, (char*)airString, mle->bitAccuracy, ((mle->adjustForTempDrift == YES) ? "YES" : "NO"),
					(char*)sensorString, ((mle->sensitivity == LOW) ? "LOW" : "HIGH"));

#if 1 // Atmel fat driver
			write(monitorLogHumanReadableFile, (uint8*)&g_spareBuffer, strlen((char*)g_spareBuffer));

			SetFileDateTimestamp(FS_DATE_LAST_WRITE);

			// Done writing, close the monitor log file
			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(monitorLogHumanReadableFile);
#else // Port fat driver
			fl_fwrite((uint8*)&g_spareBuffer, strlen((char*)g_spareBuffer), 1, monitorLogHumanReadableFile);

			// Done writing, close the monitor log file
			fl_fclose(monitorLogHumanReadableFile);
#endif

			debug("Monitor log readable entry appended to log file\r\n");
		}

		g_fileAccessLock = AVAILABLE;
	}

#if 0 // Test
	if (samplesCollected)
	{
		if (MessageBox("TEST BUFFER", "SAVE TEST BUFFER?", MB_YESNO) == MB_SECOND_CHOICE)
		{
			debug("Cancel save of test buffer\r\n");
			samplesCollected = 0;
			return;
		}

#if 1 // Atmel fat driver
		int testBufferFile;
		testBufferFile = open((char*)testBufferFilename, O_APPEND);
		if (testBufferFile == -1)
		{
			nav_setcwd((char*)testBufferFilename, TRUE, TRUE);
			testBufferFile = open((char*)testBufferFilename, O_APPEND);
		}
#else // Port fat driver
		FL_FILE* testBufferFile;
		testBufferFile = fl_fopen("C:\\Logs\\TestBuffer.txt", "a+");
#endif

		debug("Writing Test Buffer Data to log file...\r\n");

#if 1 // Atmel fat driver
		write(testBufferFile, (uint8*)&(g_eventDataBuffer), (samplesCollected * 2));

		// Done writing, close the monitor log file
		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(testBufferFile);
#else // Port fat driver
		fl_fwrite((uint8*)&(g_eventDataBuffer), (samplesCollected * 2), 1, testBufferFile);

		// Done writing, close the monitor log file
		fl_fclose(testBufferFile);
#endif

		samplesCollected = 0;

		debug("Test Buffer Data file complete\r\n");
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitMonitorLogTableFromLogFile(void)
{
	MONITOR_LOG_ENTRY_STRUCT monitorLogEntry;
	int32 bytesRead = 0;
	uint16 lowestId = 0;
	uint16 highestId = 0;
	uint16 foundIds = 0;
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Init monitor log");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

#if 1 // Atmel fat driver
		int monitorLogFile;
		monitorLogFile = open(s_monitorLogFilename, O_RDONLY);
#else // Port fat driver
		FL_FILE* monitorLogFile;
		monitorLogFile = fl_fopen("C:\\Logs\\MonitorLog.ns8", "r");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (monitorLogFile == -1)
#else // Port fat driver
		if (monitorLogFile == NULL)
#endif
		{
			debugWarn("Warning: Monitor Log File not found or has not yet been created\r\n");
		}
		// Verify file is big enough
		else if (fsaccess_file_get_size(monitorLogFile) < sizeof(MONITOR_LOG_ENTRY_STRUCT))
		{
			debugErr("Monitor Log File is corrupt\r\n");
			OverlayMessage("FILE NOT FOUND", "C:\\Logs\\MonitorLog.ns8", 3 * SOFT_SECS);
		}
		else // Monitor log file contains entries
		{
			OverlayMessage("MONITOR LOG", "INITIALIZING MONITOR LOG WITH SAVED ENTRIES", 1 * SOFT_SECS);

#if 1 // Atmel fat driver
			bytesRead = readWithSizeFix(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
#else // Port fat driver
			bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
#endif
			// Loop while data continues to be read from MONITOR log file
			while (bytesRead > 0)
			{
				if ((monitorLogEntry.status == COMPLETED_LOG_ENTRY) || (monitorLogEntry.status == INCOMPLETE_LOG_ENTRY))
				{
					//debug("Found Valid Monitor Log Entry with ID: %d\r\n", monitorLogEntry.uniqueEntryId);

					if (foundIds)
					{
						if (monitorLogEntry.uniqueEntryId < lowestId) { lowestId = monitorLogEntry.uniqueEntryId; }
						if (monitorLogEntry.uniqueEntryId > highestId) { highestId = monitorLogEntry.uniqueEntryId; }
					}
					else
					{
						lowestId = monitorLogEntry.uniqueEntryId;
						highestId = monitorLogEntry.uniqueEntryId;
					}

					foundIds++;
#if 0 // Test
					debug("(ID: %03d) M: %d, Evt#: %d, S: %d, ST: 0x%x, AT: 0x%x, BA: %d, TA: %d, ST: %d, G: %d\r\n",
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

#if 1 // Atmel fat driver
					bytesRead = readWithSizeFix(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
#else // Port fat driver
					bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
#endif
				}
			}

			// Done reading, close the monitor log file
#if 1 // Atmel fat driver
			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(monitorLogFile);
#else // Port fat driver
			fl_fclose(monitorLogFile);
#endif

			debug("Found Valid Monitor Log Entries, ID's: %d --> %d\r\n", lowestId, highestId);
		}

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static char s_onOffLogHumanReadableFilename[] = "A:\\Logs\\OnOffLogReadable.txt";
void AddOnOffLogTimestamp(uint8 onOffState)
{
	DATE_TIME_STRUCT time = GetCurrentTime();
	float extCharge = GetExternalVoltageLevelAveraged(EXT_CHARGE_VOLTAGE);
	char onOffStateString[6];
	char timeString[20];
	char extChargeString[8];

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Add On/Off log timestamp");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

#if 1 // Atmel fat driver
		int onOffLogHumanReadableFile;
		onOffLogHumanReadableFile = open(s_onOffLogHumanReadableFilename, O_APPEND);
		if (onOffLogHumanReadableFile == -1)
		{
			nav_setcwd(s_onOffLogHumanReadableFilename, TRUE, TRUE);
			onOffLogHumanReadableFile = open(s_onOffLogHumanReadableFilename, O_APPEND);
		}
#else // Port fat driver
		FL_FILE* onOffLogHumanReadableFile;
		onOffLogHumanReadableFile = fl_fopen("C:\\Logs\\OnOffLogReadable.txt", "a+");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (onOffLogHumanReadableFile == -1)
#else // Port fat driver
		if (onOffLogHumanReadableFile == NULL)
#endif
		{
			debugErr("On/Off Log File not found\r\n");
			OverlayMessage("FILE NOT FOUND", "C:\\Logs\\OnOffLogReadable.txt", 3 * SOFT_SECS);
		}
		else // File successfully created or opened
		{
			if (onOffState == ON) { strcpy((char*)onOffStateString, "On"); }
			else if (onOffState == OFF) { strcpy((char*)onOffStateString, "Off"); }
			else { strcpy((char*)onOffStateString, "J2B"); }

			if (extCharge > EXTERNAL_VOLTAGE_PRESENT) { sprintf((char*)&extChargeString, "%4.2fv", extCharge); }
			else { sprintf((char*)&extChargeString, "<none>"); }

			sprintf((char*)&timeString, "%02d-%02d-%02d %02d:%02d:%02d", time.day, time.month, time.year, time.hour, time.min, time.sec);

			sprintf((char*)&g_spareBuffer, "Unit <%s>: %3s, Version: %s, Mode: %6s, Time: %s, Battery: %3.2fv, Ext charge: %6s\r\n",
					(char*)&g_factorySetupRecord.serial_num, (char*)onOffStateString, (char*)g_buildVersion,
					((g_unitConfig.timerMode == ENABLED) ? "Timer" : "Normal"), (char*)timeString,
					GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE), extChargeString);

#if 1 // Atmel fat driver
			write(onOffLogHumanReadableFile, (uint8*)&g_spareBuffer, strlen((char*)g_spareBuffer));

			SetFileDateTimestamp(FS_DATE_LAST_WRITE);

			// Done writing, close the on/off log file
			g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
			close(onOffLogHumanReadableFile);
#else // Port fat driver
			fl_fwrite((uint8*)&g_spareBuffer, strlen((char*)g_spareBuffer), 1, onOffLogHumanReadableFile);

			// Done writing, close the on/off log file
			fl_fclose(onOffLogHumanReadableFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
static char s_debugLogFilename[] = "A:\\Logs\\DebugLogReadable.txt";
void WriteDebugBufferToFile(void)
{
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Write debug buffer");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;
		nav_select(FS_NAV_ID_DEFAULT);

		if (g_debugBufferCount)
		{
#if 1 // Atmel fat driver
			int debugLogFile;
			debugLogFile = open(s_debugLogFilename, O_APPEND);
			if (debugLogFile == -1)
			{
				nav_setcwd(s_debugLogFilename, TRUE, TRUE);
				debugLogFile = open(s_debugLogFilename, O_APPEND);
			}
#else // Port fat driver
			FL_FILE* debugLogFile;
			debugLogFile = fl_fopen("C:\\Logs\\DebugLogReadable.txt", "a+");
#endif

			// Verify file ID
#if 1 // Atmel fat driver
			if (debugLogFile == -1)
#else // Port fat driver
			if (debugLogFile == NULL)
#endif
			{
				debugErr("Debug Log File not found\r\n");
				OverlayMessage("FILE NOT FOUND", "C:\\Logs\\DebugLogReadable.txt", 3 * SOFT_SECS);
			}
			else // File successfully created or opened
			{
#if 1 // Atmel fat driver
				write(debugLogFile, (uint8*)&g_debugBuffer, g_debugBufferCount);

				SetFileDateTimestamp(FS_DATE_LAST_WRITE);

				// Done writing, close the debug log file
				g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
				close(debugLogFile);
#else // Port fat driver
				fl_fwrite((uint8*)&g_debugBuffer, g_debugBufferCount, 1, debugLogFile);

				// Done writing, close the debug log file
				fl_fclose(debugLogFile);
#endif
			}

			memset(&g_debugBuffer, 0, sizeof(g_debugBuffer));
			g_debugBufferCount = 0;
		}

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "navigation.h"
void SwitchDebugLogFile(void)
{
	uint8 status = PASSED;

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Rename debug file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

		nav_drive_set(0);
		nav_partition_mount();
		nav_select(FS_NAV_ID_DEFAULT);

		// Remove old run debug file (if it exists)
		if(nav_setcwd("A:\\Logs\\DebugLogLastRun.txt", TRUE, FALSE))
		{
			if(!nav_file_del(TRUE))
			{
				debugErr("Unable to delete old run debug file\r\n");
				status = FAILED;
			}
		}

		if (status == PASSED)
		{
			// Select source file
			if(nav_setcwd("A:\\Logs\\DebugLogReadable.txt", TRUE, FALSE))
			{
				// Rename file or directory
				if(!nav_file_rename("DebugLogLastRun.txt"))
				{
					debugErr("Unable to move debug log file to last run filename\r\n");
					status = FAILED;
				}
			}
			else
			{
				debugWarn("No debug file from last run\r\n");
				status = FAILED;
			}
		}

		if (status == PASSED) { debug("Debug log file moved to last run debug file\r\n"); }

		g_fileAccessLock = AVAILABLE;
	}
}
