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
#include "Flash.h"
#include "Summary.h"
#include "Record.h"
#include "Uart.h"
#include "Menu.h"
#include "TextTypes.h"
#include "NandFlash.h"
#include "FAT32_Definitions.h"
#include "FAT32_FileLib.h"
#include "FAT32_Access.h"
#include "sd_mmc_spi.h"
#include "FAT32_Disk.h"

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
	debug("Initializing ram summary table...\n");

	// Basically copying all FF's to the ram summary table
	ByteSet(&__ramFlashSummaryTbl[0], 0xFF, (sizeof(SUMMARY_DATA) * TOTAL_RAM_SUMMARIES));

	__ramFlashSummaryTblKey = VALID_RAM_SUMMARY_TABLE_KEY;

	//debugRaw(" done.\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "lcd.h"
#define TOTAL_DOTS	4
void CopyValidFlashEventSummariesToRam(void)
{
	uint16 ramSummaryIndex = 0;
	uint16 eventMajorVersion = (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK);

	OverlayMessage("SUMMARY LIST", "INITIALIZING SUMMARY LIST WITH STORED EVENT INFO", 1 * SOFT_SECS);

#if 1 // ns8100 ---------------------------------------------------------------------------------------
	FAT32_DIRLIST* dirList = (FAT32_DIRLIST*)&(g_eventDataBuffer[0]);
	uint16 entriesFound = 0;
	char* fileName = (char*)&g_spareBuffer[0];
	FL_FILE* eventFile;
	EVT_RECORD tempEventRecord;
	unsigned long int dirStartCluster;
	uint8 dotBuff[TOTAL_DOTS];
	static uint8 dotState = 0;
	char overlayText[50];
	uint8 i = 0;
	
	debug("Copying valid SD event summaries to ram...\n");

	fl_directory_start_cluster("C:\\Events", &dirStartCluster);
    ListDirectory(dirStartCluster, dirList, NO);

	while(dirList[entriesFound].type != FAT32_END_LIST)
	{
		if (dirList[entriesFound].type == FAT32_FILE)
		{
			//_______________________________________________________________________________
			//___Create the idea of progress with scrolling dots
			ByteSet(&overlayText[0], 0, sizeof(overlayText));
			ByteSet(&dotBuff[0], 0, sizeof(dotBuff));
	
			for (i = 0; i < dotState; i++) { dotBuff[i] = '.'; }
			for (; i < (TOTAL_DOTS - 1); i++) { dotBuff[i] = ' '; }

			if (++dotState >= TOTAL_DOTS) { dotState = 0; }

			sprintf((char*)&overlayText[0], "INITIALIZING SUMMARY LIST WITH STORED EVENT INFO%s", (char*)&dotBuff[0]);
			OverlayMessage("SUMMARY LIST", (char*)&overlayText[0], 0);

			//_______________________________________________________________________________
			//___Handle the next file found
			sprintf(fileName, "C:\\Events\\%s", dirList[entriesFound].name);
			eventFile = fl_fopen(fileName, "r");

			if (eventFile == NULL)
			{
				debugErr("Error: Event File %s not found!\r\n", dirList[entriesFound].name);
				OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
			}			
			else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
			{
				debugErr("Error: Event File %s is corrupt!\r\n", dirList[entriesFound].name);
				OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
			}			
			else
			{
				//fl_fread(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVENT_HEADER_STRUCT));
				fl_fread(eventFile, (uint8*)&tempEventRecord.header, sizeof(EVT_RECORD));

				// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
				if ((tempEventRecord.header.startFlag == EVENT_RECORD_START_FLAG) &&
					((tempEventRecord.header.recordVersion & EVENT_MAJOR_VERSION_MASK) == eventMajorVersion) &&
					(tempEventRecord.header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
				{
					//fl_fread(eventFile, (uint8*)&tempEventRecord.summary, sizeof(EVENT_SUMMARY_STRUCT));

					debug("Found Valid Event File: %s, with Event #: %d, Mode: %d, Size: %s, RSI: %d\n", 
							dirList[entriesFound].name, tempEventRecord.summary.eventNumber, tempEventRecord.summary.mode, 
							(eventFile->filelength == (tempEventRecord.header.headerLength + tempEventRecord.header.summaryLength + 
							tempEventRecord.header.dataLength)) ? "Correct" : "Incorrect", ramSummaryIndex);
					
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
		}

		entriesFound++;
	}
	
#else // ns7100 ---------------------------------------------------------------------------------------
	uint16 eventHeaderSize = sizeof(EVENT_HEADER_STRUCT);
	uint32 eventSize = 0;
	EVT_RECORD* flashEventPtr = (EVT_RECORD*)FLASH_EVENT_START;

	debug("Copying valid flash event summaries to ram...");

	// Start with begenning of flash event table
	while (flashEventPtr < (EVT_RECORD*)FLASH_EVENT_END)
	{
		// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
		if ((flashEventPtr->header.startFlag == EVENT_RECORD_START_FLAG) &&
			((flashEventPtr->header.recordVersion & EVENT_MAJOR_VERSION_MASK) == eventMajorVersion) &&
			(flashEventPtr->header.headerLength == eventHeaderSize))
		{
			//DebugPrint(RAW, "*** Found Event (%d) at: 0x%x with sizes: 0x%x, 0x%x, 0x%x, event #: %d, mode: 0x%x\n",
			//	ramSummaryIndex+1, flashEventPtr, flashEventPtr->header.headerLength,
			//	flashEventPtr->header.summaryLength, flashEventPtr->header.dataLength,
			//	flashEventPtr->summary.eventNumber, flashEventPtr->summary.mode);

			__ramFlashSummaryTbl[ramSummaryIndex].linkPtr = (uint16*)flashEventPtr;

			// Check to make sure ram summary index doesnt get out of range, if so reset to zero
			if (++ramSummaryIndex >= TOTAL_RAM_SUMMARIES)
			{
				ramSummaryIndex = 0;
			}

			eventSize = (flashEventPtr->header.headerLength + flashEventPtr->header.summaryLength +
						flashEventPtr->header.dataLength);

			if (((uint32)flashEventPtr + eventSize) >= FLASH_EVENT_END)
			{
				//DebugPrint(RAW, "\nDiscovered a Flash Event that wraps.\n");
				//DebugPrint(RAW, "Assuming the end of events to find.\n");
				break;
			}

			flashEventPtr = (EVT_RECORD*)((uint32)flashEventPtr + eventSize);

			//DebugPrint(RAW, "Next flashEventPtr (0x%x) Start Flag: 0x%x\n", flashEventPtr, flashEventPtr->header.startFlag);
		}

		// Else increment the flash event pointer, move two words, scanning memory.
		else
		{
			flashEventPtr = (EVT_RECORD*)((uint32)flashEventPtr + 2);
		}

	}
#endif

	if (ramSummaryIndex == 0)
	{
		debugRaw(" done (none discovered).");
	}
	else
	{
		debugRaw(" done (discovered %d).", ramSummaryIndex);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CondenseRamSummaryTable(void)
{
	uint16 i = 0, j = 0;

	debug("Condensing ram summary table...\n");

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
				//debugRaw(" done.\n");
				return;
			}
		}
	}

	//debugRaw(" done.\n");
}

#if 0 // ns7100
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AdvFlashDataPtrToEventData(SUMMARY_DATA* currentSummary)
{
	// Check if the event record offset to start storing data is beyond the end of flash event storage
	if (((uint32)s_flashDataPtr + sizeof(EVT_RECORD)) >= FLASH_EVENT_END)
	{
		// Erase the starting sector for the flash event storage
		ReclaimSpace((uint16*)FLASH_EVENT_START);

		// Set the end of flash sector address to the end address of the sector
		s_endFlashSectorPtr = (uint16*)FLASH_EVENT_START + FLASH_SECTOR_SIZE_x16;

		// Set linkPtr to now point to the start of flash event storage
		currentSummary->linkPtr = (uint16*)FLASH_EVENT_START;

		// Set our flash event data pointer to the start of the flash event plus the event record offset
	    s_flashDataPtr = (uint16*)(FLASH_EVENT_START + sizeof(EVT_RECORD));
	}
	else
	{
		// Advance the flash data pointer by the size of the event record header and summary
		s_flashDataPtr = (uint16*)((uint32)s_flashDataPtr + sizeof(EVT_RECORD));
	}
}
#endif

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

#if 0 // ns7100
	//EVT_RECORD* storedEventPtr;
	//EVT_RECORD* tempEventPtr;
	//uint16* tempEncodeLinePtr = NULL;
	//uint8 commentSize = 0;
	//uint32 eventSize = 0;
	//uint16 highestEventNumber = 0;
	//EVT_RECORD storedEventRecord;
	//EVT_RECORD tempEventRecord;
	//EVT_RECORD* tempEventPtrAddress = NULL;

	//storedEventPtr = &storedEventRecord;
	//tempEventPtr = &tempEventRecord;

	// If the number of ram summarys is zero, initialize the pointers to the start of the flash event table
	if (s_numOfFlashSummarys == 0)
	{
		tempEventPtr = (EVT_RECORD *)FLASH_EVENT_START;
	}
	else // We have ram valid summarys
	{
		// Find the newest event/highest event number
		i = 0;
		while ((__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF) && (i < TOTAL_RAM_SUMMARIES))
		{
			storedEventPtr = (EVT_RECORD*)__ramFlashSummaryTbl[i].linkPtr;
			//ReadNandFlash((uint8*)&storedEventRecord, (uint32)__ramFlashSummaryTbl[i].linkPtr, sizeof(storedEventRecord));

			if (storedEventPtr->summary.eventNumber > highestEventNumber)
			{
				highestEventNumber = storedEventPtr->summary.eventNumber;

				tempEventPtr = storedEventPtr;
				//tempEventRecord = storedEventRecord;
				//tempEventPtrAddress = (EVT_RECORD*)__ramFlashSummaryTbl[i].linkPtr;
			}

			i++;
		}

		debug("Highest Event Number found: %d, at: %p\n", highestEventNumber, tempEventPtr);

		eventSize = (tempEventPtr->header.headerLength + tempEventPtr->header.summaryLength +
					(uint32)(tempEventPtr->header.dataLength));

		if (eventSize & 0x00000001)
		{
			debugErr("Total Event size is an odd size suggesting an error!\n");
			debug("Event sizes: 0x%x (%d), 0x%x (%d), 0x%x (%d)\n",
					tempEventPtr->header.headerLength, tempEventPtr->header.headerLength,
					tempEventPtr->header.summaryLength, tempEventPtr->header.summaryLength,
					tempEventPtr->header.dataLength, tempEventPtr->header.dataLength);
			eventSize += 1;
			debug("Fixing event size. New event size: 0x%x (%d)\n", eventSize, eventSize);
		}

		tempEventPtr = (EVT_RECORD *)((uint32)tempEventPtr + eventSize);
		//tempEventPtrAddress = (EVT_RECORD *)((uint32)tempEventPtrAddress + eventSize);

		debug("End of Event found at: %p\n", tempEventPtr);
		//debug("End of Event found at: %p\n", tempEventPtrAddress);

		// Check to make sure the next empty flash location isnt past the end of the table
		if (tempEventPtr >= (EVT_RECORD*)FLASH_EVENT_END)
		//if (tempEventPtrAddress >= (EVT_RECORD*)FLASH_EVENT_END)
		{
			// Next empty location address needs to be wrapped
			tempEventPtr = (EVT_RECORD*)((uint32)tempEventPtr - (FLASH_EVENT_END - FLASH_EVENT_START));
			//tempEventPtrAddress = (EVT_RECORD*)((uint32)tempEventPtr - (FLASH_EVENT_END - FLASH_EVENT_START));

			debug("Past the end of Flash at: %p, Setting end of event at: %p\n", (uint16*)FLASH_EVENT_END, tempEventPtr);
			//debug("Past the end of Flash at: %p, Setting end of event at: %p\n", (uint16*)FLASH_EVENT_END, tempEventPtrAddress);
		}
	}

	s_endFlashSectorPtr = (uint16*)(((uint32)tempEventPtr & 0xFFFF0000) + FLASH_SECTOR_SIZE_x8);
	//s_endFlashSectorPtr = (uint16*)(((uint32)tempEventPtrAddress & 0xFFFF0000) + FLASH_SECTOR_SIZE_x8);

	debug("End of Flash sector at: %p\n", s_endFlashSectorPtr);
#endif

#if 0
	tempEncodeLinePtr = (uint16*)tempEventPtr;
	//tempEncodeLinePtr = (uint16*)tempEventPtrAddress;

	// Now check that the next flash location to the end of the sector is empty
	while (tempEncodeLinePtr < s_endFlashSectorPtr)
	{
		if (*tempEncodeLinePtr != 0xFFFF)
		{
			debugWarn("Next flash event storage location (to the end of the sector) is not empty!\n");

			// Handle partial event
			debugWarn("Handling partial flash event error...\n");

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

			//debugRaw(" done.\n");
			return (FAILED);
		}

		tempEncodeLinePtr++;
	}

	s_flashDataPtr = (uint16*)tempEventPtr;

	debug("Current Flash Event Pointer: %p\n", s_flashDataPtr);
	debug("Current Flash Sector boundary: %p\n", s_endFlashSectorPtr);
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

	//debug("Flash Event start: 0x%x End: 0x%x\n", FLASH_EVENT_START, FLASH_EVENT_END);

	// Init Ram Summary Table - Check to see if the ram summary table is valid
	if (__ramFlashSummaryTblKey != VALID_RAM_SUMMARY_TABLE_KEY)
	{
		// Table is invalid, key is missing
		//DebugPrint(RAW, "Ram Summary Key not found.\n");
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
					//DebugPrint(RAW, "Data in Ram Summary Table is garbage.\n");

					// Assume the whole table is garbage, so just recreate it
					dataInRamGarbage = TRUE;
					break;
				}
				else if(CheckValidEventFile((uint16)((uint32)(__ramFlashSummaryTbl[i].fileEventNum))) == NO)
				{
					//DebugPrint(RAW, "Ram summary (%d) points to an invalid event.\n", i+1);
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

		// Find all flash events and recreate ram summary entries for them
		CopyValidFlashEventSummariesToRam();

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

	debug("Number of Event Summaries found in RAM: %d\n", s_numOfFlashSummarys);

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

#if 0 // ns7100
	ByteSet(eventRec, 0xFF, sizeof(EVT_RECORD));
#else // ns8100
	ByteSet(eventRec, 0x00, sizeof(EVT_RECORD));
	ByteSet(eventRec->summary.parameters.unused, 0xFF, sizeof(eventRec->summary.parameters.unused));
	ByteSet(eventRec->summary.captured.unused, 0xFF, sizeof(eventRec->summary.captured.unused));
	ByteSet(eventRec->summary.calculated.unused, 0xFF, sizeof(eventRec->summary.calculated.unused));
#endif
	//--------------------------------
	eventRec->header.startFlag = (uint16)EVENT_RECORD_START_FLAG;
	eventRec->header.recordVersion = (uint16)EVENT_RECORD_VERSION;
	eventRec->header.headerLength = (uint16)sizeof(EVENT_HEADER_STRUCT);
	eventRec->header.summaryLength = (uint16)sizeof(EVENT_SUMMARY_STRUCT);
	//-----------------------
	eventRec->summary.parameters.sampleRate = (uint16)g_triggerRecord.trec.sample_rate;
	//-----------------------
	eventRec->summary.captured.endTime = GetCurrentTime();
	eventRec->summary.captured.batteryLevel = (uint32)(100.0 * GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE));
	eventRec->summary.captured.printerStatus = (uint8)(g_helpRecord.autoPrint);
	eventRec->summary.captured.calDate = g_factorySetupRecord.cal_date;
	//-----------------------
	ByteSet(&(eventRec->summary.parameters.companyName[0]), 0, COMPANY_NAME_STRING_SIZE);
	ByteCpy(&(eventRec->summary.parameters.companyName[0]), &(g_triggerRecord.trec.client[0]), COMPANY_NAME_STRING_SIZE - 1);
	ByteSet(&(eventRec->summary.parameters.seismicOperator[0]), 0, SEISMIC_OPERATOR_STRING_SIZE);
	ByteCpy(&(eventRec->summary.parameters.seismicOperator[0]), &(g_triggerRecord.trec.oper[0]), SEISMIC_OPERATOR_STRING_SIZE - 1);
	ByteSet(&(eventRec->summary.parameters.sessionLocation[0]), 0, SESSION_LOCATION_STRING_SIZE);
	ByteCpy(&(eventRec->summary.parameters.sessionLocation[0]), &(g_triggerRecord.trec.loc[0]), SESSION_LOCATION_STRING_SIZE - 1);
	ByteSet(&(eventRec->summary.parameters.sessionComments[0]), 0, SESSION_COMMENTS_STRING_SIZE);
	ByteCpy(&(eventRec->summary.parameters.sessionComments[0]), &(g_triggerRecord.trec.comments[0]), sizeof(g_triggerRecord.trec.comments));
	//-----------------------
	ByteSet(&(eventRec->summary.version.modelNumber[0]), 0, MODEL_STRING_SIZE);
	ByteCpy(&(eventRec->summary.version.modelNumber[0]), &(g_factorySetupRecord.serial_num[0]), 15);
	ByteSet(&(eventRec->summary.version.serialNumber[0]), 0, SERIAL_NUMBER_STRING_SIZE);
	ByteCpy(&(eventRec->summary.version.serialNumber[0]), &(g_factorySetupRecord.serial_num[0]), 15);
	ByteSet(&(eventRec->summary.version.softwareVersion[0]), 0, VERSION_STRING_SIZE);
	ByteCpy(&(eventRec->summary.version.softwareVersion[0]), &(g_appVersion[0]), VERSION_STRING_SIZE - 1);
	//-----------------------
	eventRec->summary.parameters.bitAccuracy = ((g_triggerRecord.trec.bitAccuracy < ACCURACY_10_BIT) || (g_triggerRecord.trec.bitAccuracy > ACCURACY_16_BIT)) ? 
												ACCURACY_16_BIT : g_triggerRecord.trec.bitAccuracy;
	eventRec->summary.parameters.numOfChannels = NUMBER_OF_CHANNELS_DEFAULT;
	eventRec->summary.parameters.aWeighting = (uint8)g_factorySetupRecord.aweight_option;
	eventRec->summary.parameters.seismicSensorType = g_factorySetupRecord.sensor_type;
#if 0 // Port lost change
	eventRec->summary.parameters.airSensorType = (uint16)0x0;
#else // Updated
   eventRec->summary.parameters.airSensorType = (uint16)g_helpRecord.unitsOfAir;
#endif
	eventRec->summary.parameters.adjustForTempDrift = g_triggerRecord.trec.adjustForTempDrift;
	eventRec->summary.parameters.seismicUnitsOfMeasure = g_helpRecord.unitsOfMeasure;
	eventRec->summary.parameters.airUnitsOfMeasure = g_helpRecord.unitsOfAir;
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
		eventRec->summary.eventNumber = (uint16)g_nextEventNumberToUse;

		eventRec->summary.parameters.numOfSamples = (uint16)(g_triggerRecord.trec.sample_rate * g_triggerRecord.trec.record_time);
#if 0 // Fixed Pretrigger size
		eventRec->summary.parameters.preBuffNumOfSamples = (uint16)(g_triggerRecord.trec.sample_rate / 4);
#else // Variable Pretrigger size
		eventRec->summary.parameters.preBuffNumOfSamples = (uint16)(g_triggerRecord.trec.sample_rate / g_helpRecord.pretrigBufferDivider);
#endif
		eventRec->summary.parameters.calDataNumOfSamples = (uint16)(CALIBRATION_NUMBER_OF_SAMPLES);

		if ((op_mode == WAVEFORM_MODE) || (op_mode == COMBO_MODE))
		{
#if 0 // Normal
			eventRec->summary.parameters.seismicTriggerLevel = (uint32)g_triggerRecord.trec.seismicTriggerLevel;
#else // Adjust trigger for bit accuracy
			if ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) || (g_triggerRecord.trec.seismicTriggerLevel == EXTERNAL_TRIGGER_CHAR))
			{
				eventRec->summary.parameters.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel;
			}
			else
			{
				eventRec->summary.parameters.seismicTriggerLevel = g_triggerRecord.trec.seismicTriggerLevel / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);
			}
#endif

#if 0 // Normal
			eventRec->summary.parameters.airTriggerLevel = (uint32)g_triggerRecord.trec.airTriggerLevel;
#else // Adjust trigger for bit accuracy. WARNING: Only valid if air trigger gets converted to an A/D trigger count
			if ((g_airTriggerCount == NO_TRIGGER_CHAR) || (g_airTriggerCount == EXTERNAL_TRIGGER_CHAR))
			{
				eventRec->summary.parameters.airTriggerLevel = g_airTriggerCount;
			}
			else
			{
				eventRec->summary.parameters.airTriggerLevel = g_airTriggerCount / (ACCURACY_16_BIT_MIDPOINT / g_bitAccuracyMidpoint);
			}
#endif

			// Calculate the divider used for converting stored A/D peak counts to units of measure
			unitsDiv = (float)(g_bitAccuracyMidpoint * SENSOR_ACCURACY_100X_SHIFT * ((g_triggerRecord.srec.sensitivity == LOW) ? 2 : 4)) / 
						(float)(g_factorySetupRecord.sensor_type);

			tempSesmicTriggerInUnits = (float)(g_triggerRecord.trec.seismicTriggerLevel >> g_bitShiftForAccuracy) / (float)unitsDiv;

			if ((g_factorySetupRecord.sensor_type != SENSOR_ACC) && (g_helpRecord.unitsOfMeasure == METRIC_TYPE))
			{
				tempSesmicTriggerInUnits *= (float)METRIC;
			}

			debug("Seismic trigger in units: %05.2f %s\n", tempSesmicTriggerInUnits, (g_helpRecord.unitsOfMeasure == METRIC_TYPE ? "mm" : "in"));
			eventRec->summary.parameters.seismicTriggerInUnits = (uint32)(tempSesmicTriggerInUnits * 100);

			eventRec->summary.parameters.airTriggerInUnits = (uint32)g_triggerRecord.trec.airTriggerLevel;

			eventRec->summary.parameters.recordTime = (uint32)g_triggerRecord.trec.record_time;
		}	
		else // (op_mode == MANUAL_CAL_MODE)
		{ 
			eventRec->summary.parameters.sampleRate = MANUAL_CAL_DEFAULT_SAMPLE_RATE;
			eventRec->summary.parameters.numOfSamples = 0;
			eventRec->summary.parameters.preBuffNumOfSamples = 0;
			eventRec->summary.parameters.seismicTriggerLevel = 0;
			eventRec->summary.parameters.airTriggerLevel = 0;
			eventRec->summary.parameters.recordTime = 0;
			for (idex = 0; idex < 8; idex++) { eventRec->summary.parameters.channel[idex].options = GAIN_SELECT_x2; }
		}

		if (op_mode == COMBO_MODE) { eventRec->summary.subMode = WAVEFORM_MODE; }
	}

	if ((op_mode == BARGRAPH_MODE) || (op_mode == COMBO_MODE))
	{
		eventRec = &g_pendingBargraphRecord;		
		ClearAndFillInCommonRecordInfo(eventRec);

		eventRec->summary.mode = op_mode;
		eventRec->summary.eventNumber = (uint16)g_nextEventNumberToUse;

		eventRec->summary.captured.eventTime = GetCurrentTime();

		eventRec->summary.parameters.barInterval = (uint16)g_triggerRecord.bgrec.barInterval;
		eventRec->summary.parameters.summaryInterval = (uint16)g_triggerRecord.bgrec.summaryInterval;
		eventRec->summary.parameters.numOfSamples = 0;
		eventRec->summary.parameters.preBuffNumOfSamples = 0;
		eventRec->summary.parameters.calDataNumOfSamples = 0;
		eventRec->summary.parameters.activeChannels = g_triggerRecord.berec.barChannel;
		
		if (op_mode == COMBO_MODE) { eventRec->summary.subMode = BARGRAPH_MODE; }
	}	
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitCurrentEventNumber(void)
{
#if 0 // ns7100
	// Get the end sector address (Start of the 3rd boot sector)
	//uint16* endPtr = (uint16*)(FLASH_BASE_ADDR + (FLASH_BOOT_SECTOR_SIZE_x8 * 2));
	uint16 endOffset = (FLASH_BOOT_SECTOR_SIZE_x8 * 2);

	GetParameterMemory((uint8*)&eventNumber, uniqueEventNumberOffset, sizeof(eventNumber));

	// Check if the first location is 0xFF which indicates the unit hasn't recorded any events
	if (eventNumber == 0xFFFF)
	{
		g_nextEventNumberToUse = 1;
	}
	else // Find the last stored event number location
	{
		// Get the next locations value
		GetParameterMemory((uint8*)&eventNumber, (uint16)(uniqueEventNumberOffset + 1), sizeof(eventNumber));

		// Loop until the next location is empty and not past the end
		while ((eventNumber != 0xFFFF) && ((uniqueEventNumberOffset + 1) < endOffset))
		{
			// Increment address to known valid event number
			uniqueEventNumberOffset++;

			// Get the next locations value
			GetParameterMemory((uint8*)&eventNumber, (uint16)(uniqueEventNumberOffset + 1), sizeof(eventNumber));
		}

		// Get the current event number which is the last in the list
		GetParameterMemory((uint8*)&eventNumber, uniqueEventNumberOffset, sizeof(eventNumber));

		// Set the Current Event number to the last event number stored plus 1
		g_nextEventNumberToUse = (uint16)(eventNumber + 1);
	}
#endif

#if 1 // Use config to hold Current Event Number
	CURRENT_EVENT_NUMBER_STRUCT currentEventNumberRecord;

	GetRecordData(&currentEventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);

	// Check if the Factory Setup Record is valid
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
#endif

	debug("Stored Event ID: %d, Next Event ID to use: %d\n", (g_nextEventNumberToUse - 1), g_nextEventNumberToUse);
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
#if 0
	//uint16* uniqueEventStorePtr = (uint16*)(FLASH_BASE_ADDR + FLASH_BOOT_SECTOR_SIZE_x8);
	uint16 uniqueEventStoreOffset = FLASH_BOOT_SECTOR_SIZE_x8;
	uint16 positionsToAdvance = 0;
	uint16 eventNumber = 0;

	// If the Current Event Number is 0, then we have wrapped (>65535) in which case we will reinitialize
	if (g_nextEventNumberToUse == 0)
	{
		//SectorErase(uniqueEventStorePtr, 1);
		EraseParameterMemory(uniqueEventStoreOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		InitCurrentEventNumber();
	}
	// Check if at the boundary of a flash sectors worth of Unique Event numbers
	else if (((g_nextEventNumberToUse - 1) % 4096) == 0)
	{
		// Check to make sure this isnt the initial (first) event number
		if (g_nextEventNumberToUse != 1)
		{
			//SectorErase(uniqueEventStorePtr, 1);
			EraseParameterMemory(uniqueEventStoreOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		}
	}
	else // Still room to store Unique Event numbers
	{
		// Get the positions to advance based on the current event number mod'ed by the storage size in event numbers
		positionsToAdvance = (uint16)((g_nextEventNumberToUse - 1) % 4096);

		// Set the offset to the event number positions adjusted to bytes
		uniqueEventStoreOffset += (positionsToAdvance * 2);
	}

	GetParameterMemory((uint8*)&eventNumber, uniqueEventStoreOffset, sizeof(eventNumber));

	// Check to make sure Current location is empty
	if (eventNumber == 0xFFFF)
	{
		GetParameterMemory((uint8*)&eventNumber, (uint16)(uniqueEventStoreOffset - 1), sizeof(eventNumber));

		// Check if the first location to be used or if a valid Unique Number
		// preceeded the Current Event Number to be stored
		if ((positionsToAdvance == 0) || (eventNumber != 0xFFFF))
		{
			// Store the Current Event number as the newest Unique Event number
			//ProgramFlashWord(uniqueEventStorePtr, g_nextEventNumberToUse);
			SaveParameterMemory((uint8*)&g_nextEventNumberToUse, uniqueEventStoreOffset, sizeof(g_nextEventNumberToUse));

			// Store as the last Event recorded in AutoDialout table
			__autoDialoutTbl.lastStoredEvent = g_nextEventNumberToUse;

			// Increment to a new Event number
			g_nextEventNumberToUse++;
			debug("Unique Event numbers stored: %d, Current Event number: %d\n",
				(g_nextEventNumberToUse - 1), g_nextEventNumberToUse);

			return;
		}
	}

	// If we get here, then we failed a validation check
	debugErr("Unique Event number storage doesnt match Current Event Number. (0x%x, %d)\n",
				uniqueEventStoreOffset, g_nextEventNumberToUse);
#endif // end of ns7100

#if 1 // Updated to use config
	CURRENT_EVENT_NUMBER_STRUCT currentEventNumberRecord;

	// Store the Current Event number as the newest Unique Event number
	currentEventNumberRecord.currentEventNumber = g_nextEventNumberToUse;
	SaveRecordData(&currentEventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);

	// Store as the last Event recorded in AutoDialout table
	__autoDialoutTbl.lastStoredEvent = g_nextEventNumberToUse;

	// Increment to a new Event number
	g_nextEventNumberToUse++;
	debug("Saved Event ID: %d, Next Event ID to use: %d\n", (g_nextEventNumberToUse - 1), g_nextEventNumberToUse);

	return;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetUniqueEventNumber(SUMMARY_DATA* currentSummary)
{
#if 0 // ns7100	
	EVT_RECORD* currentStoredEvent = (EVT_RECORD*)currentSummary->linkPtr;
	return (currentStoredEvent->summary.eventNumber);
#endif

#if 1 // Event number is stored in ram summary
	return currentSummary->fileEventNum;
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void GetEventFileInfo(uint16 eventNumber, EVENT_HEADER_STRUCT* eventHeaderPtr, EVENT_SUMMARY_STRUCT* eventSummaryPtr, BOOLEAN cacheDataToRamBuffer)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	FL_FILE* eventFile;
	EVENT_HEADER_STRUCT fileEventHeader;
	EVENT_SUMMARY_STRUCT fileSummary;
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
	eventFile = fl_fopen(fileName, "r");

	// Verify file ID
	if (eventFile == NULL)
	{
		debugErr("Error: Event File %s not found!\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}	
	// Verify file is big enough
	else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
	{
		debugErr("Error: Event File %s is corrupt!\r\n", fileName);
		OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
	}	
	else
	{
		fl_fread(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));

		// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
		if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
			((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
		{
			debug("Found Valid Event File: %s\n", fileName);

			fl_fread(eventFile, (uint8*)&fileSummary, sizeof(EVENT_SUMMARY_STRUCT));
			
			if (cacheDataToRamBuffer == YES)
			{
				fl_fread(eventFile, (uint8*)&g_eventDataBuffer[0], 
							(eventFile->filelength - (sizeof(EVENT_HEADER_STRUCT) - sizeof(EVENT_SUMMARY_STRUCT))));
			}
		}

		fl_fclose(eventFile);
	}

	if (eventHeaderPtr != NULL)
	{
		ByteCpy(eventHeaderPtr, &fileEventHeader, sizeof(EVENT_HEADER_STRUCT));
	}

	if (eventSummaryPtr != NULL)
	{
		ByteCpy(eventSummaryPtr, &fileSummary, sizeof(EVENT_SUMMARY_STRUCT));
	}	
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void GetEventFileRecord(uint16 eventNumber, EVT_RECORD* eventRecord)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	FL_FILE* eventFile;
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
	eventFile = fl_fopen(fileName, "r");

	// Verify file ID
	if (eventFile == NULL)
	{
		debugErr("Error: Event File %s not found!\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}	
	// Verify file is big enough
	else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
	{
		debugErr("Error: Event File %s is corrupt!\r\n", fileName);
		OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
	}	
	else
	{
		fl_fread(eventFile, (uint8*)eventRecord, sizeof(EVT_RECORD));

		// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
		if ((eventRecord->header.startFlag == EVENT_RECORD_START_FLAG) &&
			((eventRecord->header.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(eventRecord->header.headerLength == sizeof(EVENT_HEADER_STRUCT)))
		{
			debug("Found Valid Event File: %s\n", fileName);
		}

		fl_fclose(eventFile);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DeleteEventFileRecord(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
	if (fl_remove(fileName) == -1)
	{
		OverlayMessage(fileName, "UNABLE TO DELETE EVENT", 3 * SOFT_SECS);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DeleteEventFileRecords(void)
{
	FAT32_DIRLIST* dirList = (FAT32_DIRLIST*)&(g_eventDataBuffer[0]);
	uint16 entriesIndex = 0;
	uint16 eventsDeleted = 0;
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	char popupText[50];
	unsigned long int dirStartCluster;
	
	debug("Deleting Events...\n");

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

	sprintf(popupText, "REMOVED %d EVENTS", eventsDeleted);
	OverlayMessage("DELETE EVENTS", popupText, 3 * SOFT_SECS);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheEventDataToRam(uint16 eventNumber, uint32 dataSize)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	FL_FILE* eventFile;
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
	eventFile = fl_fopen(fileName, "r");

	// Verify file ID
	if (eventFile == NULL)
	{
		debugErr("Error: Event File %s not found!\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}	
	// Verify file is big enough
	else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
	{
		debugErr("Error: Event File %s is corrupt!\r\n", fileName);
		OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
	}	
	else
	{
		fl_fclose(eventFile);
		eventFile = fl_fopen(fileName, "r");

		fl_fseek(eventFile, sizeof(EVT_RECORD), SEEK_SET);
		fl_fread(eventFile, (uint8*)&g_eventDataBuffer[0], dataSize);

		fl_fclose(eventFile);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheEventToRam(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	FL_FILE* eventFile;
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
	eventFile = fl_fopen(fileName, "r");

	// Verify file ID
	if (eventFile == NULL)
	{
		debugErr("Error: Event File %s not found!\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}	
	// Verify file is big enough
	else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
	{
		debugErr("Error: Event File %s is corrupt!\r\n", fileName);
		OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
	}	
	else
	{
		fl_fread(eventFile, (uint8*)&g_eventDataBuffer[0], eventFile->filelength);
		fl_fclose(eventFile);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
BOOLEAN CheckValidEventFile(uint16 eventNumber)
{
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	FL_FILE* eventFile;
	EVENT_HEADER_STRUCT fileEventHeader;
	BOOLEAN validFile = NO;
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", eventNumber);
	eventFile = fl_fopen(fileName, "r");

	// Verify file ID
	if (eventFile == NULL)
	{
		debugErr("Error: Event File %s not found!\r\n", fileName);
		OverlayMessage("FILE NOT FOUND", fileName, 3 * SOFT_SECS);
	}		
	// Verify file is big enough
	else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
	{
		debugErr("Error: Event File %s is corrupt!\r\n", fileName);
		OverlayMessage("FILE CORRUPT", fileName, 3 * SOFT_SECS);
	}		
	else
	{
		fl_fread(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));

		// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
		if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
			((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK)) &&
			(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
		{
			//debug("Found Valid Event File: %s", fileName);

			validFile =  YES;
		}

		fl_fclose(eventFile);
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
	gpio_clr_gpio_pin(AVR32_PIN_PB15);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	// Power on the SD Card
	gpio_set_gpio_pin(AVR32_PIN_PB15);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	// Check if SD Detect pin 
	if (gpio_get_pin_value(AVR32_PIN_PA02) == ON)
	{
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}
	}
	else
	{
		debugErr("\n\nSD Card not detected!\n");
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerDownSDCard(void)
{
	debugRaw("\n Powering down SD Card... ");

	// Power off the SD card
	gpio_clr_gpio_pin(AVR32_PIN_PB15);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	debugRaw("done.\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerUpSDCardAndInitFat32(void)
{
	debugRaw("\nPowering up SD Card and ReInit Fat32... \n");

	// Power on the SD Card
	gpio_set_gpio_pin(AVR32_PIN_PB15);

	// Wait for power to propagate
	SoftUsecWait(10 * SOFT_MSECS);

	// Check if SD Detect pin 
	if (gpio_get_pin_value(AVR32_PIN_PA02) == ON)
	{
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}
	}
	else
	{
		debugErr("\n\nSD Card not detected!\n");
	}

	debugRaw("done.\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
FL_FILE* GetEventFileHandle(uint16 newFileEventNumber, EVENT_FILE_OPTION option)
{
	FL_FILE* fileHandle;
	char* fileName = (char*)&g_spareBuffer[0];
	char fileOption[3];
	
	sprintf(fileName, "C:\\Events\\Evt%d.ns8", newFileEventNumber);

	switch (option)
	{
		case CREATE_EVENT_FILE:
			debug("File to create: %s\n", fileName);
			strcpy(&fileOption[0], "w");
			break;
			
		case READ_EVENT_FILE:
			strcpy(&fileOption[0], "r");
			break;
			
		case APPEND_EVENT_FILE:
			strcpy(&fileOption[0], "a+");
			break;
	}

	// Check if trying to create a file but the filename exists
	if (option == CREATE_EVENT_FILE)
	{
		fileHandle = fl_fopen(fileName, "r");
		
		// File alraedy exists
		if (fileHandle != NULL)
		{
			fl_fclose(fileHandle);
			fl_remove(fileName);
		}
	}

	return (fl_fopen(fileName, fileOption));
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

#if 0 // ns7100
		// Check if the current flash pointer address is long-bounded
		if ((uint32)s_flashDataPtr & 0x00000003)
		{
			// If the address is not long-bounded, make it so
			s_flashDataPtr = (uint16*)(((uint32)s_flashDataPtr & 0xFFFFFFFC) + 1);
		}

		// Check if we can store the event record structure (not including raw data) in the current sector
		if (((uint8*)s_flashDataPtr + sizeof(EVT_RECORD)) >= (uint8*)s_endFlashSectorPtr)
		{
			// Check if we are at the end of the flash buffer (special case)
			if (s_endFlashSectorPtr >= (uint16*)FLASH_EVENT_END)
			{
				// Reset the end of flash sector ptr and the current flash pointer address
				s_endFlashSectorPtr = (uint16*)FLASH_EVENT_START;
				s_flashDataPtr = (uint16*)FLASH_EVENT_START;
			}

			// Need to clear space, issue a sector erase
			ReclaimSpace(s_endFlashSectorPtr);
			// Incrememnt the flash sector boundary to the next sector
			s_endFlashSectorPtr += FLASH_SECTOR_SIZE_x16;
		}

		// Set the flash pointer address in the ram summary
		__ramFlashSummaryTbl[s_currFlashSummary].linkPtr = (uint16*)s_flashDataPtr;

		// Set the current summary pointer to the current ram summary
		*sumEntryPtr = &(__ramFlashSummaryTbl[s_currFlashSummary]);

		debug("Ram Summary Table Entry %d: Event pointer: 0x%x\n", s_currFlashSummary, s_flashDataPtr);
#endif

#if 1 // ns8100
		// Set the flash pointer address in the ram summary
		__ramFlashSummaryTbl[s_currFlashSummary].fileEventNum = g_nextEventNumberToUse;

		// Set the current summary pointer to the current ram summary
		*sumEntryPtr = &__ramFlashSummaryTbl[s_currFlashSummary];

		debug("Ram Summary Table Entry %d: Event ID: %d\n", s_currFlashSummary, g_nextEventNumberToUse);
#endif

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

	debug("Number of Event Summaries in RAM: %d\n", s_numOfFlashSummarys);

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
void CompleteRamEventSummary(SUMMARY_DATA* flashSummPtr, SUMMARY_DATA* ramSummPtr)
{
	//--------------------------------
	// EVT_RECORD -

	// Only called for Wave, Cal, C-Wave, all settings are common
#if 0
	if ((g_pendingEventRecord.summary.mode == WAVEFORM_MODE) || (g_pendingEventRecord.summary.mode == MANUAL_CAL_MODE) || 
			(g_pendingEventRecord.summary.mode == COMBO_MODE))
	{
#endif

	debug("Copy calculated from Waveform buffer to global ram event record\n");
		
	// Fill in calculated data (Bargraph data filled in at the end of bargraph)
	g_pendingEventRecord.summary.calculated.a.peak = flashSummPtr->waveShapeData.a.peak = ramSummPtr->waveShapeData.a.peak;
	g_pendingEventRecord.summary.calculated.r.peak = flashSummPtr->waveShapeData.r.peak = ramSummPtr->waveShapeData.r.peak;
	g_pendingEventRecord.summary.calculated.v.peak = flashSummPtr->waveShapeData.v.peak = ramSummPtr->waveShapeData.v.peak;
	g_pendingEventRecord.summary.calculated.t.peak = flashSummPtr->waveShapeData.t.peak = ramSummPtr->waveShapeData.t.peak;

	debug("Newly stored peaks: a:%04x r:%04x v:%04x t:%04x\n", 
			g_pendingEventRecord.summary.calculated.a.peak, 
			g_pendingEventRecord.summary.calculated.r.peak,
			g_pendingEventRecord.summary.calculated.v.peak,
			g_pendingEventRecord.summary.calculated.t.peak);

	g_pendingEventRecord.summary.calculated.a.frequency = flashSummPtr->waveShapeData.a.freq = ramSummPtr->waveShapeData.a.freq;
	g_pendingEventRecord.summary.calculated.r.frequency = flashSummPtr->waveShapeData.r.freq = ramSummPtr->waveShapeData.r.freq;
	g_pendingEventRecord.summary.calculated.v.frequency = flashSummPtr->waveShapeData.v.freq = ramSummPtr->waveShapeData.v.freq;
	g_pendingEventRecord.summary.calculated.t.frequency = flashSummPtr->waveShapeData.t.freq = ramSummPtr->waveShapeData.t.freq;

	debug("Newly stored freq: a:%d r:%d v:%d t:%d\n", 
			g_pendingEventRecord.summary.calculated.a.frequency, 
			g_pendingEventRecord.summary.calculated.r.frequency,
			g_pendingEventRecord.summary.calculated.v.frequency,
			g_pendingEventRecord.summary.calculated.t.frequency);

	// Calculate Displacement as PPV/(2 * PI * Freq) with 1000000 to shift to keep accuracy and the 10 to adjust the frequency
	g_pendingEventRecord.summary.calculated.a.displacement = 0;

	g_pendingEventRecord.summary.calculated.r.displacement = (uint32)(ramSummPtr->waveShapeData.r.peak * 1000000 / 2 / PI / ramSummPtr->waveShapeData.r.freq * 10);
	g_pendingEventRecord.summary.calculated.v.displacement = (uint32)(ramSummPtr->waveShapeData.v.peak * 1000000 / 2 / PI / ramSummPtr->waveShapeData.v.freq * 10);
	g_pendingEventRecord.summary.calculated.t.displacement = (uint32)(ramSummPtr->waveShapeData.t.peak * 1000000 / 2 / PI / ramSummPtr->waveShapeData.t.freq * 10);

#if 1 // Updated (Port lost change)	
	// Calculate Peak Acceleration as (2 * PI * PPV * Freq) / 1G, where 1G = 386.4in/sec2 or 9814.6 mm/sec2, using 1000 to shift to keep accuracy
	// The divide by 10 at the end to adjust the frequency, since freq stored as freq * 10
	// Not dividing by 1G at this time. Before displaying Peak Acceleration, 1G will need to be divided out
	g_pendingEventRecord.summary.calculated.a.acceleration = 0;

	g_pendingEventRecord.summary.calculated.r.acceleration = (uint32)(ramSummPtr->waveShapeData.r.peak * 1000 * 2 * PI * ramSummPtr->waveShapeData.r.freq / 10);
	g_pendingEventRecord.summary.calculated.v.acceleration = (uint32)(ramSummPtr->waveShapeData.v.peak * 1000 * 2 * PI * ramSummPtr->waveShapeData.v.freq / 10);
	g_pendingEventRecord.summary.calculated.t.acceleration = (uint32)(ramSummPtr->waveShapeData.t.peak * 1000 * 2 * PI * ramSummPtr->waveShapeData.t.freq / 10);
#endif

	//} End of now if 0 defined check

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

#if 0 // ns7100
	//uint8 i = 0;
	uint32 eventDataSize = 0;

	EVT_RECORD* flashEventRecord = (EVT_RECORD*)flashSummPtr->linkPtr;

	// Get the structure size as the number of words. The following will round up.
	eventDataSize = (sizeof(EVT_RECORD) + 1) / 2;

	FlashWrite((uint16*)flashEventRecord, (uint16*)&(g_pendingEventRecord), eventDataSize);
#endif
}

#if 0 // ns7100
///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ReclaimSpace(uint16* sectorAddr)
{
	uint8 needToEraseSector = NO;
	uint16 tempNumOfFlashSummarys = s_numOfFlashSummarys;
	uint16 i = 0;
	uint16 firstErasedEventNumber = 0;
	uint16 lastErasedEventNumber = 0;
	char message[50];

	// If sectorAddr isnt the begenning address, make it so
	uint16* tempSectorAddr = (uint16*)((uint32)sectorAddr & 0xffff0000);

	// Sector addresses should all be data sectors
	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		if ((__ramFlashSummaryTbl[i].linkPtr >= tempSectorAddr) &&
			(__ramFlashSummaryTbl[i].linkPtr < (tempSectorAddr + FLASH_SECTOR_SIZE_x16)))
		{
			if (firstErasedEventNumber == 0)
				firstErasedEventNumber = ((EVT_RECORD*)__ramFlashSummaryTbl[i].linkPtr)->summary.eventNumber;
			lastErasedEventNumber = ((EVT_RECORD*)__ramFlashSummaryTbl[i].linkPtr)->summary.eventNumber;

			__ramFlashSummaryTbl[i].linkPtr = (uint16*)0xFFFFFFFF;
			debug("Removing ram summary entry: %d\n", i);

			needToEraseSector = YES;

			// Keep the count of summaries accurate
			s_numOfFlashSummarys--;
		}
	}

	if (firstErasedEventNumber != 0)
	{
		if (firstErasedEventNumber != lastErasedEventNumber)
		{
			sprintf(&message[0], "ERASING OLDEST EVENTS FOR SPACE (%d-%d)", firstErasedEventNumber, lastErasedEventNumber);
		}
		else
		{
			sprintf(&message[0], "ERASING OLDEST EVENT FOR SPACE (%d)", firstErasedEventNumber);
		}

		OverlayMessage(getLangText(STATUS_TEXT), &message[0], (2 * SOFT_SECS));
	}

	// Check if number of summaries changed
	if (s_numOfFlashSummarys == tempNumOfFlashSummarys)
		debug("Number of Event Summaries now in RAM: %d\n", s_numOfFlashSummarys);

	// Since sector data may contain all data (not pointed to by a ram summary), check to make sure it's empty
	for (i = 0; i < FLASH_SECTOR_SIZE_x16; i++)
		if (sectorAddr[i] != 0xFFFF)
		{
			needToEraseSector = YES;
			break;
		}

	if (needToEraseSector == YES)
	{
		debug("Attempting to reclaim space from flash event table (sector addr: 0x%x)\n", sectorAddr);

		if (g_helpRecord.flashWrapping == NO)
		{
			OverlayMessage(getLangText(WARNING_TEXT), "WRAPPING DISABLED BUT FORCED TO ERASE SOME FLASH", (5 * SOFT_SECS));
		}

		SectorErase(sectorAddr, 1);
	}
}
#endif

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
#if 0 // ns7100
void CheckFlashDataPointer(void)
{
	// Check if current flash data pointer is equal to or past the flash sector boundary
	if (s_flashDataPtr >= s_endFlashSectorPtr)
	{
		// Check if the current flash data pointer is equal to or past the end of the flash memory
		if (s_endFlashSectorPtr == (uint16*)FLASH_EVENT_END)
		{
			// Reset pointers
			s_endFlashSectorPtr = (uint16*)FLASH_EVENT_START;
			s_flashDataPtr = (uint16*)FLASH_EVENT_START;
		}

		// Erase the current sector
		ReclaimSpace(s_endFlashSectorPtr);

		// Set the flash sector boundary to the beginning of the next sector
		s_endFlashSectorPtr = s_endFlashSectorPtr + FLASH_SECTOR_SIZE_x16;
	}
}
#endif

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
void GetFlashUsageStats(FLASH_USAGE_STRUCT* usage)
{
	//FLASH_USAGE_STRUCT usage;
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

    sd_mmc_spi_get_capacity();

	usage->sizeUsed = 0;
	usage->sizeFree = capacity - usage->sizeUsed;
	usage->percentUsed = (uint8)((usage->sizeUsed * 100) / capacity);
	usage->percentFree = (uint8)(100 - usage->percentUsed);
	usage->waveEventsLeft = (uint16)(usage->sizeFree / waveSize);
	usage->barHoursLeft = (uint16)(usage->sizeFree / barSize);
	usage->manualCalsLeft = (uint16)(usage->sizeFree / manualCalSize);
	usage->wrapped = NO;
	usage->roomForBargraph = (usage->sizeFree > newBargraphMinSize) ? YES : NO;

#if 0 // ns7100
	uint16 i = 0;

	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		if ((__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF) && (__ramFlashSummaryTbl[i].linkPtr > s_flashDataPtr))
		{
			// Have a valid event after the current Flash data pointer, thus found non-empty flash sectors
			usage.wrapped = YES;

			// Break out of the for loop
			break;
		}
	}

	if (usage.wrapped == YES)
	{
		// Recalc parameters
		usage.sizeUsed = (FLASH_EVENT_END - FLASH_EVENT_START) - ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		usage.sizeFree = ((uint32)s_endFlashSectorPtr - (uint32)s_flashDataPtr);
		usage.percentUsed = (uint8)(((uint32)usage.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
		usage.percentFree = (uint8)(100 - usage.percentUsed);
		usage.waveEventsLeft = (uint16)(usage.sizeFree / waveSize);
		usage.barHoursLeft = (uint16)(usage.sizeFree / barSize);
		usage.manualCalsLeft = (uint16)(usage.sizeFree / manualCalSize);
		usage.roomForBargraph = (usage.sizeFree > newBargraphMinSize) ? YES : NO;
	}
#endif
}

