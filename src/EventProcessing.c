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
#if 0 // Port fat driver
#include "FAT32_Disk.h"
#endif

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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitRamSummaryTbl(void)
{
	debug("Initializing ram summary table...\r\n");

	// Basically copying all FF's to the ram summary table
	memset(&__ramFlashSummaryTbl[0], 0xFF, (sizeof(SUMMARY_DATA) * TOTAL_RAM_SUMMARIES));

	__ramFlashSummaryTblKey = VALID_RAM_SUMMARY_TABLE_KEY;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "lcd.h"
#include "navigation.h"
#include "fsaccess.h"
#define TOTAL_DOTS	4
void CopyValidFlashEventSummariesToRam(void)
{
	uint16 ramSummaryIndex = 0;

#if 1 // Atmel fat driver
#else // Port fat driver
	uint8 dotBuff[TOTAL_DOTS];
	char overlayText[50];
	static uint8 dotState = 0;
	uint8 i = 0;
	unsigned long int dirStartCluster;
	uint16 entriesFound = 0;
	FAT32_DIRLIST* dirList = (FAT32_DIRLIST*)&(g_eventDataBuffer[0]);
#endif

	OverlayMessage("SUMMARY LIST", "INITIALIZING SUMMARY LIST WITH STORED EVENT INFO", 1 * SOFT_SECS);
	debug("Copying valid SD event summaries to ram...\r\n");

	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Copy summaries to RAM");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		nav_setcwd("A:\\Events\\", TRUE, FALSE);
		nav_filelist_reset();

		while (nav_filelist_set(0 , FS_FIND_NEXT))
		{
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
#if 0
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
					read(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVT_RECORD));

					// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
					if ((tempEventRecord.header.startFlag == EVENT_RECORD_START_FLAG) &&
					((tempEventRecord.header.recordVersion & EVENT_MAJOR_VERSION_MASK) == eventMajorVersion) &&
					(tempEventRecord.header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
					{
						//fl_fread(eventFile, (uint8*)&tempEventRecord.summary, sizeof(EVENT_SUMMARY_STRUCT));

#if 0
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

					close(eventFile);
#endif
			}
		}
#else // Port fat driver
		fl_directory_start_cluster("C:\\Events", &dirStartCluster);
		ListDirectory(dirStartCluster, dirList, NO);

		while(dirList[entriesFound].type != FAT32_END_LIST)
		{
			if (dirList[entriesFound].type == FAT32_FILE)
			{
				//_______________________________________________________________________________
				//___Create the idea of progress with scrolling dots
				memset(&overlayText[0], 0, sizeof(overlayText));
				memset(&dotBuff[0], 0, sizeof(dotBuff));
	
				for (i = 0; i < dotState; i++) { dotBuff[i] = '.'; }
				for (; i < (TOTAL_DOTS - 1); i++) { dotBuff[i] = ' '; }

				if (++dotState >= TOTAL_DOTS) { dotState = 0; }

#if 1 // Test not updating LCD
				sprintf((char*)&overlayText[0], "INITIALIZING SUMMARY LIST WITH STORED EVENT INFO%s", (char*)&dotBuff[0]);
				OverlayMessage("SUMMARY LIST", (char*)&overlayText[0], 0);
#endif

#if 0 // Test
				char eventNumBuffer[6];
				uint8 j;

				j = 3;
				memset(&eventNumBuffer, 0, sizeof(eventNumBuffer));

				while (dirList[entriesFound].name[j] != '.')
				{
					eventNumBuffer[j-3] = dirList[entriesFound].name[j];
					j++;
				}

				__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum = atoi((char*)eventNumBuffer);

				// Check to make sure ram summary index doesnt get out of range, if so reset to zero
				if (++ramSummaryIndex >= TOTAL_RAM_SUMMARIES)
				{
					ramSummaryIndex = 0;
				}
#else // Normal, but extremely slow
				uint16 eventMajorVersion = (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK);
				char* fileName = (char*)&g_spareBuffer[0];
				FL_FILE* eventFile;
				EVT_RECORD tempEventRecord;

				//_______________________________________________________________________________
				//___Handle the next file found
				sprintf(fileName, "C:\\Events\\%s", dirList[entriesFound].name);
				eventFile = fl_fopen(fileName, "r");

				if (eventFile == NULL)
				{
					debugErr("Event File %s not found\r\n", dirList[entriesFound].name);
					OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
				}
				else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
				{
					debugErr("Event File %s is corrupt\r\n", dirList[entriesFound].name);
					OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
				}
				else
				{
					debug("Event File: %s\r\n", dirList[entriesFound].name);

					//fl_fread(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVENT_HEADER_STRUCT));
					fl_fread(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVT_RECORD));

					// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
					if ((tempEventRecord.header.startFlag == EVENT_RECORD_START_FLAG) &&
						((tempEventRecord.header.recordVersion & EVENT_MAJOR_VERSION_MASK) == eventMajorVersion) &&
						(tempEventRecord.header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
					{
						//fl_fread(eventFile, (uint8*)&tempEventRecord.summary, sizeof(EVENT_SUMMARY_STRUCT));

#if 0
						debug("Found Valid Event File: %s, with Event #: %d, Mode: %d, Size: %s, RSI: %d\r\n",
								dirList[entriesFound].name, tempEventRecord.summary.eventNumber, tempEventRecord.summary.mode,
								(eventFile->filelength == (tempEventRecord.header.headerLength + tempEventRecord.header.summaryLength +
								tempEventRecord.header.dataLength)) ? "Correct" : "Incorrect", ramSummaryIndex);
#endif				
						__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum = tempEventRecord.summary.eventNumber;

#if 0
						char popupText[50];
						ClearLCDscreen();
						sprintf(popupText, "Adding Event #%d to RAM Summary Table (%d)",
								(int)__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum,
								(int)tempEventRecord.summary.eventNumber);
						OverlayMessage(fileName, popupText, 200 * SOFT_MSECS);
#endif
						// Check to make sure ram summary index doesnt get out of range, if so reset to zero
						if (++ramSummaryIndex >= TOTAL_RAM_SUMMARIES)
						{
							ramSummaryIndex = 0;
						}
					}
					else
					{
						debugWarn("Event File: %s is not valid for this unit.\r\n", dirList[entriesFound].name);
					}

					fl_fclose(eventFile);
				}
#endif
			}

			entriesFound++;
		}
#endif
		g_fileAccessLock = AVAILABLE;
	}

	//debug("Done copying events to summary (discovered %d)\r\n", ramSummaryIndex);
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

#if 0
	tempEncodeLinePtr = (uint16*)tempEventPtr;
	//tempEncodeLinePtr = (uint16*)tempEventPtrAddress;

	// Now check that the next flash location to the end of the sector is empty
	while (tempEncodeLinePtr < s_endFlashSectorPtr)
	{
		if (*tempEncodeLinePtr != 0xFFFF)
		{
			debugWarn("Next flash event storage location (to the end of the sector) is not empty\r\n");

			// Handle partial event
			debugWarn("Handling partial flash event error...\r\n");

			ReclaimSpace(tempEncodeLinePtr);
			CondenseRamSummaryTable();

			// Count the number of summary structures with a valid (non-FF) link pointer
			s_numOfFlashSummarys = 0;
			for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
			{
				if (__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF)
				{
					s_numOfFlashSummarys++;
				}
			}

			//debugRaw(" done.\r\n");
			return (FAILED);
		}

		tempEncodeLinePtr++;
	}

	s_flashDataPtr = (uint16*)tempEventPtr;

	debug("Current Flash Event Pointer: %p\r\n", s_flashDataPtr);
	debug("Current Flash Sector boundary: %p\r\n", s_endFlashSectorPtr);
#endif

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
				if((uint32)(__ramFlashSummaryTbl[i].fileEventNum) >= g_nextEventNumberToUse)
				{
					// This signals a big warning
					//debugRaw("Data in Ram Summary Table is garbage.\r\n");

					// Assume the whole table is garbage, so just recreate it
					dataInRamGarbage = TRUE;
					break;
				}
				else if(CheckValidEventFile((uint16)((uint32)(__ramFlashSummaryTbl[i].fileEventNum))) == NO)
				{
					//debugRaw("Ram summary (%d) points to an invalid event.\r\n", i+1);
					__ramFlashSummaryTbl[i].fileEventNum = 0xFFFFFFFF;
				}
				else
				{
					// Inc the total number of events found
					s_numOfFlashSummarys++;

					// Check if any empty entries were found before or while counting events
					if(foundEmptyRamSummaryEntry == YES)
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
	eventRec->summary.captured.printerStatus = (uint8)(g_unitConfig.autoPrint);
	eventRec->summary.captured.calDate = g_factorySetupRecord.cal_date;
	eventRec->summary.captured.externalTrigger = NO;
	eventRec->summary.captured.comboEventsRecordedDuringSession = 0;
	eventRec->summary.captured.comboEventsRecordedStartNumber = 0;
	eventRec->summary.captured.comboEventsRecordedEndNumber = 0;
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
void InitEventRecord(uint8 op_mode)
{
	EVT_RECORD* eventRec;
	uint8 idex;
	float tempSesmicTriggerInUnits;
	float unitsDiv;

	if ((op_mode == WAVEFORM_MODE) || (op_mode == MANUAL_CAL_MODE) || (op_mode == COMBO_MODE))
	{
		eventRec = &g_pendingEventRecord;		
		ClearAndFillInCommonRecordInfo(eventRec);

		eventRec->summary.mode = op_mode;
		if (op_mode == COMBO_MODE) { eventRec->summary.subMode = WAVEFORM_MODE; }
		eventRec->summary.eventNumber = (uint16)g_nextEventNumberToUse;

		eventRec->summary.parameters.numOfSamples = (uint16)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time);
		eventRec->summary.parameters.preBuffNumOfSamples = (uint16)(g_triggerRecord.trec.sample_rate / g_unitConfig.pretrigBufferDivider);
		eventRec->summary.parameters.calDataNumOfSamples = (uint16)(CALIBRATION_NUMBER_OF_SAMPLES);

		// Reset parameters for the special calibration mode
		if (op_mode == MANUAL_CAL_MODE)
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
		else // ((op_mode == WAVEFORM_MODE) || (op_mode == COMBO_MODE))
		{
			eventRec->summary.parameters.recordTime = (uint32)g_triggerRecord.trec.record_time;

#if 0 // Old - Fixed method
			eventRec->summary.parameters.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
#else // New - Adjust trigger for bit accuracy
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
#endif

#if 0 // Old - Fixed method
			eventRec->summary.parameters.airTriggerLevel = g_triggerRecord.trec.airTriggerLevel;
#else // New - Adjust trigger for bit accuracy
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
#endif
		}	
	}

	if ((op_mode == BARGRAPH_MODE) || (op_mode == COMBO_MODE))
	{
		eventRec = &g_pendingBargraphRecord;		
		ClearAndFillInCommonRecordInfo(eventRec);

		eventRec->summary.mode = op_mode;
		if (op_mode == COMBO_MODE) { eventRec->summary.subMode = BARGRAPH_MODE; }
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

#if 0 // Bad logic - Don't want to set 1 since no event has been recorded, and the eventual save will validate the record
		currentEventNumberRecord.currentEventNumber = g_nextEventNumberToUse;
		SaveRecordData(&currentEventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);
#endif
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
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Check event file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		int eventFile;
		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);
#else // Port fat driver
		FL_FILE* eventFile;
		sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = fl_fopen(fileName, "r");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (eventFile == -1)
#else // Port fat driver
		if (eventFile == NULL)
#endif
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
#if 1 // Atmel fat driver
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
#else // Port fat driver
		else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
#endif
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
#if 1 // Atmel fat driver
			read(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));
#else // Port fat driver
			fl_fread(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));
#endif

			// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
			if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
			((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
			{
				debug("Found Valid Event File: %s\r\n", fileName);

#if 1 // Atmel fat driver
				read(eventFile, (uint8*)&fileSummary, sizeof(EVENT_SUMMARY_STRUCT));
#else // Port fat driver
				fl_fread(eventFile, (uint8*)&fileSummary, sizeof(EVENT_SUMMARY_STRUCT));
#endif
			
				if (cacheDataToRamBuffer == YES)
				{
#if 1 // Atmel fat driver
					read(eventFile, (uint8*)&g_eventDataBuffer[0], (fsaccess_file_get_size(eventFile) - (sizeof(EVENT_HEADER_STRUCT) - sizeof(EVENT_SUMMARY_STRUCT))));
#else // Port fat driver
					fl_fread(eventFile, (uint8*)&g_eventDataBuffer[0], (eventFile->filelength - (sizeof(EVENT_HEADER_STRUCT) - sizeof(EVENT_SUMMARY_STRUCT))));
#endif
				}
			}

#if 1 // Atmel fat driver
			close(eventFile);
#else // Port fat driver
			fl_fclose(eventFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
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
void GetEventFileRecord(uint16 eventNumber, EVT_RECORD* eventRecord)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Get event file");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		int eventFile;
		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);
#else // Port fat driver
		FL_FILE* eventFile;
		sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = fl_fopen(fileName, "r");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (eventFile == -1)
#else // Port fat driver
		if (eventFile == NULL)
#endif
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
#if 1 // Atmel fat driver
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
#else // Port fat driver
		else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
#endif
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
#if 1 // Atmel fat driver
			read(eventFile, (uint8*)eventRecord, sizeof(EVT_RECORD));
#else // Port fat driver
			fl_fread(eventFile, (uint8*)eventRecord, sizeof(EVT_RECORD));
#endif

			// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
			if ((eventRecord->header.startFlag == EVENT_RECORD_START_FLAG) &&
			((eventRecord->header.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(eventRecord->header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
			{
				debug("Found Valid Event File: %s\r\n", fileName);
			}

#if 1 // Atmel fat driver
			close(eventFile);
#else // Port fat driver
			fl_fclose(eventFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
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
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		if (nav_setcwd(fileName, TRUE, FALSE) == TRUE)
		{
			if (nav_file_del(TRUE) == FALSE)
			{
				OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
			}
		}
#else // Port fat driver
		sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
		if (fl_remove(fileName) == -1)
		{
			OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
		}
#endif

		g_fileAccessLock = AVAILABLE;
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
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
	while (nav_setcwd("A:\\Events", TRUE, FALSE) == TRUE)
	{
		// Delete file or directory
		if(nav_file_del(FALSE) == FALSE)
		{
			nav_file_getname(&fileName[0], 50);
			OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
			break;
		}
		else
		{
			eventsDeleted++;
		}
	}
#else // Port fat driver
		FAT32_DIRLIST* dirList = (FAT32_DIRLIST*)&(g_eventDataBuffer[0]);
		uint16 entriesIndex = 0;
		unsigned long int dirStartCluster;

		fl_directory_start_cluster("C:\\Events", &dirStartCluster);
		ListDirectory(dirStartCluster, dirList, NO);

		while(dirList[entriesIndex].type != FAT32_END_LIST)
		{
			if (dirList[entriesIndex].type == FAT32_FILE)
			{
				sprintf(fileName, "C:\\Events\\%s", dirList[entriesIndex].name);
				if (fl_remove(fileName) == -1)
				{
					OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
				}
				else
				{
					eventsDeleted++;
				}
			}

			entriesIndex++;
		}
#endif

		sprintf(popupText, "REMOVED %d EVENTS", eventsDeleted);
		OverlayMessage("DELETE EVENTS", popupText, 3 * SOFT_SECS);

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheEventDataToRam(uint16 eventNumber, uint32 dataSize)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Cache event data");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		int eventFile;
		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);
#else // Port fat driver
		FL_FILE* eventFile;
		sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = fl_fopen(fileName, "r");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (eventFile == -1)
#else // Port fat driver
		if (eventFile == NULL)
#endif
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
#if 1 // Atmel fat driver
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
#else // Port fat driver
		else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
#endif
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
#if 1 // Atmel fat driver
			file_seek(sizeof(EVT_RECORD), FS_SEEK_SET);
			read(eventFile, (uint8*)&g_eventDataBuffer[0], dataSize);

			close(eventFile);
#else // Port fat driver
			fl_fclose(eventFile);
			eventFile = fl_fopen(fileName, "r");

			fl_fseek(eventFile, sizeof(EVT_RECORD), SEEK_SET);
			fl_fread(eventFile, (uint8*)&g_eventDataBuffer[0], dataSize);

			fl_fclose(eventFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheEventToRam(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Cache event");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		int eventFile;
		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);
#else // Port fat driver
		FL_FILE* eventFile;
		sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = fl_fopen(fileName, "r");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (eventFile == -1)
#else // Port fat driver
		if (eventFile == NULL)
#endif
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
#if 1 // Atmel fat driver
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
#else // Port fat driver
		else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
#endif
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
#if 1 // Atmel fat driver
			read(eventFile, (uint8*)&g_eventDataBuffer[0], fsaccess_file_get_size(eventFile));
			close(eventFile);
#else // Port fat driver
			fl_fread(eventFile, (uint8*)&g_eventDataBuffer[0], eventFile->filelength);
			fl_fclose(eventFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
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
	
	if (g_fileAccessLock != AVAILABLE)
	{
		ReportFileSystemAccessProblem("Check valid event");
	}
	else // (g_fileAccessLock == AVAILABLE)
	{
		//g_fileAccessLock = FILE_LOCK;

#if 1 // Atmel fat driver
		int eventFile;
		sprintf(fileName, "A:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = open(fileName, O_RDONLY);
#else // Port fat driver
		FL_FILE* eventFile;
		sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
		eventFile = fl_fopen(fileName, "r");
#endif

		// Verify file ID
#if 1 // Atmel fat driver
		if (eventFile == -1)
#else // Port fat driver
		if (eventFile == NULL)
#endif
		{
			debugErr("Event File %s not found\r\n", fileName);
			OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
		}
		// Verify file is big enough
#if 1 // Atmel fat driver
		else if (fsaccess_file_get_size(eventFile) < sizeof(EVENT_HEADER_STRUCT))
#else // Port fat driver
		else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
#endif
		{
			debugErr("Event File %s is corrupt\r\n", fileName);
			OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
		}
		else
		{
#if 1 // Atmel fat driver
			read(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));
#else // Port fat driver
			fl_fread(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));
#endif

			// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
			if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
				((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
				(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
			{
				//debug("Found Valid Event File: %s", fileName);

				validFile =  YES;
			}

#if 1 // Atmel fat driver
			close(eventFile);
#else // Port fat driver
			fl_fclose(eventFile);
#endif
		}

		g_fileAccessLock = AVAILABLE;
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
	if (gpio_get_pin_value(AVR32_PIN_PA02) == ON)
	{
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

#if 1 // Atmel fat driver
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

#else // Port fat driver
		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed\r\n");
			OverlayMessage("ERROR", "FAILED TO INIT FILE SYSTEM ON SD CARD!", 0);
		}
#endif
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
	if (gpio_get_pin_value(AVR32_PIN_PA02) == ON)
	{
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

#if 1 // Atmel fat driver
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

#else // Port fat driver
		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed\r\n");
			OverlayMessage("ERROR", "FAILED TO INIT FILE SYSTEM ON SD CARD!", 0);
		}
#endif
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
#if 1 // Atmel fat driver
int GetEventFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option)
#else // Port fat driver
FL_FILE* GetEventFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option)
#endif
{
	char* fileName = (char*)&g_spareBuffer[0];

#if 1 // Atmel fat driver
	int fileHandle;
	int fileOption;

	sprintf(fileName, "A:\\Events\\Evt%d.ns8", newFileEventNumber);
#else // Port fat driver
	FL_FILE* fileHandle;
	char fileOption[3];

	sprintf(fileName, "C:\\Events\\Evt%d.ns8", newFileEventNumber);
#endif

	// Set the file option flags for the file process
	switch (option)
	{
		case CREATE_EVENT_FILE:
			debug("File to create: %s\r\n", fileName);
#if 1 // Atmel fat driver
			fileOption = (O_CREAT | O_WRONLY);
#else // Port fat driver
			strcpy(&fileOption[0], "w");
#endif
			break;
			
		case READ_EVENT_FILE:
			debug("File to read: %s\r\n", fileName);
#if 1 // Atmel fat driver
			fileOption = O_RDONLY;
#else // Port fat driver
			strcpy(&fileOption[0], "r");
#endif
			break;
			
		case APPEND_EVENT_FILE:
			debug("File to append: %s\r\n", fileName);
#if 1 // Atmel fat driver
			fileOption = O_APPEND;
#else // Port fat driver
			strcpy(&fileOption[0], "a+");
#endif
			break;

		case OVERWRITE_EVENT_FILE:
			debug("File to overwrite: %s\r\n", fileName);
#if 1 // Atmel fat driver
			fileOption = O_RDWR;
#else // Port fat driver
			strcpy(&fileOption[0], "r+");
#endif
			break;
	}

#if 1 // New with Atmel fat driver
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

#else // Old
	// Check if trying to create a file but the filename exists
	if (option == CREATE_EVENT_FILE)
	{
#if 1 // Atmel fat driver
		fileHandle = open(fileName, O_RDONLY);

		// File already exists
		if (fileHandle != -1)
		{
			close(fileHandle);
			nav_file_del(TRUE);
		}
#else // Port fat driver
		fileHandle = fl_fopen(fileName, "r");
		
		// File already exists
		if (fileHandle != NULL)
		{
			fl_fclose(fileHandle);
			fl_remove(fileName);
		}
#endif
	}

#if 1 // Atmel fat driver
	return (open(fileName, fileOption));
#else // Port fat driver
	return (fl_fopen(fileName, fileOption));
#endif
#endif
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
#if 0 // Old
void CompleteRamEventSummary(SUMMARY_DATA* flashSummPtr, SUMMARY_DATA* ramSummPtr)
#else
void CompleteRamEventSummary(SUMMARY_DATA* ramSummaryPtr)
#endif
{
	//--------------------------------
	// EVT_RECORD -

	// Only called for Wave, Cal, C-Wave, all settings are common
#if 0
	if ((g_pendingEventRecord.summary.mode == WAVEFORM_MODE) || (g_pendingEventRecord.summary.mode == MANUAL_CAL_MODE) || 
			(g_pendingEventRecord.summary.mode == COMBO_MODE))
	{
#endif

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
	// EVENT_HEADER_STRUCT  -
	g_pendingEventRecord.header.summaryChecksum = 0;
	g_pendingEventRecord.header.dataChecksum = 0;

#if 0 // ns8100 - Replaced with data length calculated at init
	// *** Fix bug with summary.parameters.numOfSamples bug where a value can be larger than the uint16 it holds
	if (((g_pendingEventRecord.summary.mode == WAVEFORM_MODE) || (g_pendingEventRecord.summary.mode == COMBO_MODE)) && 
			((g_pendingEventRecord.summary.parameters.sampleRate * g_pendingEventRecord.summary.parameters.recordTime) > 0xFFFF))
	{
		// Store the event data size in bytes, It's (number of channels) * (pre + event + cal sample) * 2 bytes
		g_pendingEventRecord.header.dataLength = (uint32)((g_pendingEventRecord.summary.parameters.numOfChannels *
								 			 (g_pendingEventRecord.summary.parameters.preBuffNumOfSamples +
							 				 (g_pendingEventRecord.summary.parameters.sampleRate *
											 g_pendingEventRecord.summary.parameters.recordTime) +
											 g_pendingEventRecord.summary.parameters.calDataNumOfSamples)) * 2);
	}
	else
	{
		// Store the event data size in bytes, It's (number of channels) * (pre + event + cal sample) * 2 bytes
		g_pendingEventRecord.header.dataLength = (uint32)((g_pendingEventRecord.summary.parameters.numOfChannels *
								 			 (g_pendingEventRecord.summary.parameters.preBuffNumOfSamples +
							 				 g_pendingEventRecord.summary.parameters.numOfSamples +
											 g_pendingEventRecord.summary.parameters.calDataNumOfSamples)) * 2);
	}
#endif

	//--------------------------------
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
#if 0 // Unused
void StoreData(uint16* dataPtr, uint16 dataWords)
{
#if 0
	uint8 i = 0;

	for (i = 0; i < dataWords; i++)
	{
		CheckFlashDataPointer();
		ProgramFlashWord(s_flashDataPtr++, *(dataPtr + i));
	}
#endif

#if 0
	debugRaw("R: %4x V: %4x T: %4x A: %4x ", 
				((SAMPLE_DATA_STRUCT*)dataPtr)->r,
				((SAMPLE_DATA_STRUCT*)dataPtr)->v,
				((SAMPLE_DATA_STRUCT*)dataPtr)->t,
				((SAMPLE_DATA_STRUCT*)dataPtr)->a);
#endif
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "navigation.h"
void GetSDCardUsageStats(void)
{
	uint32 waveSize;
	uint32 barSize;
	uint32 manualCalSize;
	uint32 newBargraphMinSize;

	waveSize = sizeof(EVT_RECORD) + (100 * 8) + (uint32)(g_triggerRecord.trec.sample_rate * 2) +
				(uint32)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time * 8);
	barSize = (uint32)(((3600 * 8) / g_triggerRecord.bgrec.barInterval) + (sizeof(EVT_RECORD) / 24) +
				((3600 * sizeof(CALCULATED_DATA_STRUCT)) / g_triggerRecord.bgrec.summaryInterval));
	manualCalSize = sizeof(EVT_RECORD) + (100 * 8);
	newBargraphMinSize = (manualCalSize + sizeof(EVT_RECORD) + (sizeof(CALCULATED_DATA_STRUCT) * 2) +
							(((g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval) + 1) * 8 * 2));

#if NS8100_ORIGINAL
    sd_mmc_spi_get_capacity();

	g_sdCardUsageStats.sizeUsed = 0;
	g_sdCardUsageStats.sizeFree = capacity - g_sdCardUsageStats.sizeUsed;
	g_sdCardUsageStats.percentUsed = (uint8)((g_sdCardUsageStats.sizeUsed * 100) / capacity);
	g_sdCardUsageStats.percentFree = (uint8)(100 - g_sdCardUsageStats.percentUsed);
#else // NS8100_ALPHA
	if(!nav_partition_mount()) // Mount drive
	{
		debugErr("SD MMC Card: Unable to mount volume\r\n");
	}

	g_sdCardUsageStats.sizeFree = (nav_partition_freespace() << FS_SHIFT_B_TO_SECTOR);
	g_sdCardUsageStats.sizeUsed = (nav_partition_space() << FS_SHIFT_B_TO_SECTOR) - g_sdCardUsageStats.sizeFree;
	g_sdCardUsageStats.percentFree = (uint8)(nav_partition_freespace_percent());
	g_sdCardUsageStats.percentUsed = (uint8)(100 - g_sdCardUsageStats.percentFree);
#endif
	g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
	g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
	g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
	g_sdCardUsageStats.wrapped = NO;
	g_sdCardUsageStats.roomForBargraph = (g_sdCardUsageStats.sizeFree > newBargraphMinSize) ? YES : NO;

#if 0 // Fill in at some point
	if (g_sdCardUsageStats.wrapped == YES)
	{
		// Recalc parameters
		g_sdCardUsageStats.sizeUsed = (FLASH_EVENT_END - FLASH_EVENT_START) - ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.sizeFree = ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.percentUsed = (uint8)(((uint32)g_sdCardUsageStats.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
		g_sdCardUsageStats.percentFree = (uint8)(100 - g_sdCardUsageStats.percentUsed);
		g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
		g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
		g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
		g_sdCardUsageStats.roomForBargraph = (g_sdCardUsageStats.sizeFree > newBargraphMinSize) ? YES : NO;
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UpdateSDCardUsageStats(uint32 removeSize)
{
	uint32 waveSize;
	uint32 barSize;
	uint32 manualCalSize;
	uint32 newBargraphMinSize;

	waveSize = sizeof(EVT_RECORD) + (100 * 8) + (uint32)(g_triggerRecord.trec.sample_rate * 2) +
	(uint32)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time * 8);
	barSize = (uint32)(((3600 * 8) / g_triggerRecord.bgrec.barInterval) + (sizeof(EVT_RECORD) / 24) +
	((3600 * sizeof(CALCULATED_DATA_STRUCT)) / g_triggerRecord.bgrec.summaryInterval));
	manualCalSize = sizeof(EVT_RECORD) + (100 * 8);
	newBargraphMinSize = (manualCalSize + sizeof(EVT_RECORD) + (sizeof(CALCULATED_DATA_STRUCT) * 2) +
	(((g_triggerRecord.bgrec.summaryInterval / g_triggerRecord.bgrec.barInterval) + 1) * 8 * 2));

	g_sdCardUsageStats.sizeFree -= removeSize;
	g_sdCardUsageStats.sizeUsed += removeSize;
	g_sdCardUsageStats.percentFree = (uint8)((float)((float)g_sdCardUsageStats.sizeFree / (float)(g_sdCardUsageStats.sizeFree + g_sdCardUsageStats.sizeUsed)) * 100);
	g_sdCardUsageStats.percentUsed = (uint8)(100 - g_sdCardUsageStats.percentFree);

	g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
	g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
	g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
	g_sdCardUsageStats.wrapped = NO;
	g_sdCardUsageStats.roomForBargraph = (g_sdCardUsageStats.sizeFree > newBargraphMinSize) ? YES : NO;

#if 0 // Fill in at some point
	if (g_sdCardUsageStats.wrapped == YES)
	{
		// Recalc parameters
		g_sdCardUsageStats.sizeUsed = (FLASH_EVENT_END - FLASH_EVENT_START) - ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.sizeFree = ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		g_sdCardUsageStats.percentUsed = (uint8)(((uint32)g_sdCardUsageStats.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
		g_sdCardUsageStats.percentFree = (uint8)(100 - g_sdCardUsageStats.percentUsed);
		g_sdCardUsageStats.waveEventsLeft = (uint16)(g_sdCardUsageStats.sizeFree / waveSize);
		g_sdCardUsageStats.barHoursLeft = (uint16)(g_sdCardUsageStats.sizeFree / barSize);
		g_sdCardUsageStats.manualCalsLeft = (uint16)(g_sdCardUsageStats.sizeFree / manualCalSize);
		g_sdCardUsageStats.roomForBargraph = (g_sdCardUsageStats.sizeFree > newBargraphMinSize) ? YES : NO;
	}
#endif
}