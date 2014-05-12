///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: EventProcessing.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:47 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/EventProcessing.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Old_Board.h"
#include "EventProcessing.h"
#include "Flash.h"
#include "Summary.h"
#include "Rec.h"
#include "Uart.h"
#include "Menu.h"
#include "Msgs430.h"
#include "TextTypes.h"
#include "NandFlash.h"
#include "FAT32_Definitions.h"
#include "FAT32_FileLib.h"
#include "FAT32_Access.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern uint32 __ramFlashSummaryTblKey;
extern SUMMARY_DATA __ramFlashSummaryTbl[TOTAL_RAM_SUMMARIES];
extern REC_EVENT_MN_STRUCT trig_rec;
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern EVT_RECORD g_RamEventRecord;
extern REC_HELP_MN_STRUCT help_rec;
extern MSGS430_UNION msgs430;
extern char appVersion[];
extern AUTODIALOUT_STRUCT __autoDialoutTbl;
extern uint16 eventDataBuffer[];

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
uint16* endFlashSectorPtr;
uint16 flashFullFlag;
uint8 noFlashWrapFlag = FALSE;
uint16 g_currentEventNumber = 1;
uint16 numOfFlashSummarys;
uint16 currFlashSummary;

///----------------------------------------------------------------------------
///	Statics
///----------------------------------------------------------------------------
static uint16* sFlashDataPtr;

//*****************************************************************************
// Function:	initRamSummaryTbl
// Purpose:
//*****************************************************************************
void initRamSummaryTbl(void)
{
	debug("Initializing ram summary table...\n");

	// Basically copying all FF's to the ram summary table
	byteSet(&__ramFlashSummaryTbl[0], 0xFF, (sizeof(SUMMARY_DATA) * TOTAL_RAM_SUMMARIES));

	__ramFlashSummaryTblKey = VALID_RAM_SUMMARY_TABLE_KEY;

	//debugRaw(" done.\n");
}

//*****************************************************************************
// Function: copyValidFlashEventSummariesToRam
// Purpose: When the ram summary table is garbage, copy flash summaries to ram
//*****************************************************************************
void copyValidFlashEventSummariesToRam(void)
{
	uint16 ramSummaryIndex = 0;
	uint16 eventMajorVersion = (EVENT_RECORD_VERSION & EVENT_MAJOR_VERSION_MASK);

#if 1 //---------------------------------------------------------------------------------------
	FAT32_DIRLIST* dirList = (FAT32_DIRLIST*)&(eventDataBuffer[0]);
	uint16 entriesFound = 0;
	char fileName[50]; // Should only be short filenames, 8.3 format + directory
	FL_FILE* eventFile;
	EVENT_HEADER_STRUCT fileEventHeader;
	EVENT_SUMMARY_STRUCT fileSummary;
	unsigned long int dirStartCluster;
	
	debug("Copying valid SD event summaries to ram...\n");

	fl_directory_start_cluster("C:\\Events", &dirStartCluster);
    ListDirectory(dirStartCluster, dirList, NO);

	//debug("CVFESR: Dirctory list created\n");

	while(dirList[entriesFound].type != FAT32_END_LIST)
	{
		//debug("CVFESR: Handling Next Entry...\n");

		if (dirList[entriesFound].type == FAT32_FILE)
		{
			//debug("CVFESR: Found File Entry...\n");

			sprintf(fileName, "C:\\Events\\%s", dirList[entriesFound].name);
			eventFile = fl_fopen(fileName, "r");

			if (eventFile == NULL)
				debugErr("Error: Event File %s not found!\r\n", dirList[entriesFound].name);
			else if (eventFile->filelength < sizeof(EVENT_HEADER_STRUCT))
				debugErr("Error: Event File %s is corrupt!\r\n", dirList[entriesFound].name);
			else
			{
				//debug("CVFESR: Reading File Entry Header...\n");

				fl_fread(eventFile, (uint8*)&fileEventHeader, sizeof(EVENT_HEADER_STRUCT));

				// If we find the EVENT_RECORD_START_FLAG followed by the encodeFlag2, then assume this is the start of an event
				if ((fileEventHeader.startFlag == EVENT_RECORD_START_FLAG) &&
					((fileEventHeader.recordVersion & EVENT_MAJOR_VERSION_MASK) == eventMajorVersion) &&
					(fileEventHeader.headerLength == sizeof(EVENT_HEADER_STRUCT)))
				{
					debug("Found Valid Event File: %s", dirList[entriesFound].name);

					fl_fread(eventFile, (uint8*)&fileSummary, sizeof(EVENT_SUMMARY_STRUCT));

					//__ramFlashSummaryTbl[ramSummaryIndex].fileEventNum = fileSummary.eventNumber;
					__ramFlashSummaryTbl[ramSummaryIndex].linkPtr = (void*)((uint32)(fileSummary.eventNumber));

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
	
#else //---------------------------------------------------------------------------------------
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
			//debugPrint(RAW, "*** Found Event (%d) at: 0x%x with sizes: 0x%x, 0x%x, 0x%x, event #: %d, mode: 0x%x\n",
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
				//debugPrint(RAW, "\nDiscovered a Flash Event that wraps.\n");
				//debugPrint(RAW, "Assuming the end of events to find.\n");
				break;
			}

			flashEventPtr = (EVT_RECORD*)((uint32)flashEventPtr + eventSize);

			//debugPrint(RAW, "Next flashEventPtr (0x%x) Start Flag: 0x%x\n", flashEventPtr, flashEventPtr->header.startFlag);
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
		debugRaw(" done (none discovered).\n");
	}
	else
	{
		debugRaw(" done (discovered %d).\n", ramSummaryIndex);
	}
}

//*****************************************************************************
// Function: condenseRamSummaryTable
// Purpose: Condense all the entries to the begenning of the table since they are indexed sequentially
//*****************************************************************************
void condenseRamSummaryTable(void)
{
	uint16 i = 0, j = 0;

	debug("Condensing ram summary table...\n");

	// Find the empty summary entries between the valid entries and condense the ram summary table
	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		// Check if we have an empty summary entry
		if (__ramFlashSummaryTbl[i].linkPtr == (uint16*)0xFFFFFFFF)
		{
			// Index (i) is the current empty summary entry

			// Search through the rest of the ram summary table for a valid entry to move up
			for (j = (uint16)(i + 1); j < TOTAL_RAM_SUMMARIES; j++)
			{
				// If the current summary entry is valid, the move it to the empty entry
				if (__ramFlashSummaryTbl[j].linkPtr != (uint16*)0xFFFFFFFF)
				{
					// Copy the valid summary entry to the current empty entry
					__ramFlashSummaryTbl[i] = __ramFlashSummaryTbl[j];

					// Set the link pointer to FF's to make the algorithm think the entry is empty
					__ramFlashSummaryTbl[j].linkPtr = (uint16*)0xFFFFFFFF;
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

//*****************************************************************************
// Function:	advFlashDataPtrToEventData
// Purpose:
//*****************************************************************************
void advFlashDataPtrToEventData(SUMMARY_DATA* currentSummary)
{
	// Check if the event record offset to start storing data is beyond the end of flash event storage
	if (((uint32)sFlashDataPtr + sizeof(EVT_RECORD)) >= FLASH_EVENT_END)
	{
		// Erase the starting sector for the flash event storage
		ReclaimSpace((uint16*)FLASH_EVENT_START);

		// Set the end of flash sector address to the end address of the sector
		endFlashSectorPtr = (uint16*)FLASH_EVENT_START + FLASH_SECTOR_SIZE_x16;

		// Set linkPtr to now point to the start of flash event storage
		currentSummary->linkPtr = (uint16*)FLASH_EVENT_START;

		// Set our flash event data pointer to the start of the flash event plus the event record offset
	    sFlashDataPtr = (uint16*)(FLASH_EVENT_START + sizeof(EVT_RECORD));
	}
	else
	{
		// Advance the flash data pointer by the size of the event record header and summary
		sFlashDataPtr = (uint16*)((uint32)sFlashDataPtr + sizeof(EVT_RECORD));
	}
}

//*****************************************************************************
// FUNCTION
//  void InitFlashEvtBuff(void)
//*****************************************************************************
uint8 InitFlashEvtBuff(void)
{
	EVT_RECORD* storedEventPtr;
	EVT_RECORD* tempEventPtr;
	uint16* tempEncodeLinePtr = NULL;
	//uint8 commentSize = 0;
	uint16 i = 0;
	uint32 eventSize = 0;
	uint16 highestEventNumber = 0;
	//EVT_RECORD storedEventRecord;
	//EVT_RECORD tempEventRecord;
	//EVT_RECORD* tempEventPtrAddress = NULL;

	// Initialize the current ram summary index
	currFlashSummary = 0;
	endFlashSectorPtr = NULL;

	//storedEventPtr = &storedEventRecord;
	//tempEventPtr = &tempEventRecord;

	// Find the first free ram summary entry (table should be condensed, all used entries up front)
	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		if (__ramFlashSummaryTbl[i].linkPtr == (uint16*)0xFFFFFFFF)
		{
			currFlashSummary = i;
			break;
		}
	}

	// If the number of ram summarys is zero, initialize the pointers to the start of the flash event table
	if (numOfFlashSummarys == 0)
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

	endFlashSectorPtr = (uint16*)(((uint32)tempEventPtr & 0xFFFF0000) + FLASH_SECTOR_SIZE_x8);
	//endFlashSectorPtr = (uint16*)(((uint32)tempEventPtrAddress & 0xFFFF0000) + FLASH_SECTOR_SIZE_x8);

	debug("End of Flash sector at: %p\n", endFlashSectorPtr);

#if 1
	tempEncodeLinePtr = (uint16*)tempEventPtr;
	//tempEncodeLinePtr = (uint16*)tempEventPtrAddress;

	// Now check that the next flash location to the end of the sector is empty
	while (tempEncodeLinePtr < endFlashSectorPtr)
	{
		if (*tempEncodeLinePtr != 0xFFFF)
		{
			debugWarn("Next flash event storage location (to the end of the sector) is not empty!\n");

			// Handle partial event
			debugWarn("Handling partial flash event error...\n");

			ReclaimSpace(tempEncodeLinePtr);
			condenseRamSummaryTable();

			// Count the number of summary structures with a valid (non-FF) link pointer
			numOfFlashSummarys = 0;
			for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
			{
				if (__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF)
				{
					numOfFlashSummarys++;
				}
			}

			//debugRaw(" done.\n");
			return (FAILED);
		}

		tempEncodeLinePtr++;
	}
#endif

	sFlashDataPtr = (uint16*)tempEventPtr;

	debug("Current Flash Event Pointer: %p\n", sFlashDataPtr);
	debug("Current Flash Sector boundary: %p\n", endFlashSectorPtr);

	return (PASSED);
}

//*****************************************************************************
// FUNCTION
//  void InitFlashBuffs(void)
//*****************************************************************************
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
	numOfFlashSummarys = 0;

	//debug("Flash Event start: 0x%x End: 0x%x\n", FLASH_EVENT_START, FLASH_EVENT_END);

	// Init Ram Summary Table - Check to see if the ram summary table is valid
	if (__ramFlashSummaryTblKey != VALID_RAM_SUMMARY_TABLE_KEY)
	{
		// Table is invalid, key is missing
		//debugPrint(RAW, "Ram Summary Key not found.\n");
		dataInRamGarbage = TRUE;
	}
	else // Verify and count every ram summary link that points to an event
	{
		// Count the number of summary structures with a valid (non-FF) linkPtr
		for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
		{
			// Check if the link pointer points to something
			if (__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF)
			{
				// Check to make sure ram values arent garbage
				// linkPtr should be even (and long-bounded), and point to somewhere within the flash event structure
				if (((uint32)__ramFlashSummaryTbl[i].linkPtr & 0x00000001) ||
					((uint32)__ramFlashSummaryTbl[i].linkPtr & 0x00000003) ||
					((uint32)__ramFlashSummaryTbl[i].linkPtr < FLASH_EVENT_START) ||
					((uint32)__ramFlashSummaryTbl[i].linkPtr > FLASH_EVENT_END))
				{
					// This signals a big warning
					//debugPrint(RAW, "Data in Ram Summary Table is garbage.\n");

					// Assume the whole table is garbage, so just recreate it
					dataInRamGarbage = TRUE;
					break;
				}
				else if(*(__ramFlashSummaryTbl[i].linkPtr) != EVENT_RECORD_START_FLAG)
				{
					//debugPrint(RAW, "Ram summary (%d) points to an invalid event.\n", i+1);
					__ramFlashSummaryTbl[i].linkPtr = (uint16*)0xFFFFFFFF;
				}
				else
				{
					// Inc the total number of events found
					numOfFlashSummarys++;

					// Check if any empty entries were found before or while counting events
					if(foundEmptyRamSummaryEntry == YES)
						condenseTable = YES;
				}

#if 0 // fix_ns8100
				ReadNandFlash((uint8*)&eventStartData, (uint32)__ramFlashSummaryTbl[i].linkPtr, sizeof(eventStartData));
				if (eventStartData != EVENT_RECORD_START_FLAG)
				{
					//debugPrint(RAW, "Ram summary (%d) points to an invalid event.\n", i+1);
					__ramFlashSummaryTbl[i].linkPtr = (uint16*)0xFFFFFFFF;
				}
				else
				{
					// Inc the total number of events found
					numOfFlashSummarys++;

					// Check if any empty entries were found before or while counting events
					if (foundEmptyRamSummaryEntry == YES)
						condenseTable = YES;
				}
#endif
			}
			else
				foundEmptyRamSummaryEntry = YES;
		}
	}

	// Check if we found garbage in the ram summary table
	if (dataInRamGarbage == TRUE)
	{
		// Re-init the table
		initRamSummaryTbl();

#if 1 // fix_ns8100
		// Find all flash events and recreate ram summary entries for them
		copyValidFlashEventSummariesToRam();
#endif
		// Re-count the number of summary structures with a valid (non-FF) link pointer
		numOfFlashSummarys = 0;
		for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
		{
			if (__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF)
			{
				numOfFlashSummarys++;
			}
		}
	}
	else if (condenseTable == YES)
	{
		// Remove any empty entries and sort oldest first
		condenseRamSummaryTable();
	}

	debug("Number of Event Summaries found in RAM: %d\n", numOfFlashSummarys);

	// Initialize some global values and check if successful
	if (InitFlashEvtBuff() == FAILED)
	{
		// Failed (encountered and handled a partial flash event)
		// Now everything is right with the world, need to initialize global values again
		InitFlashEvtBuff();
	}
}

//*****************************************************************************
// FUNCTION:	initEventRecord
// DESCRIPTION:
//*****************************************************************************
void initEventRecord(EVT_RECORD* eventRec, uint8 op_mode)
{
	uint8 idex;

	byteSet(eventRec, 0xFF, sizeof(EVT_RECORD));

	//--------------------------------
	// EVENT_HEADER_STRUCT  -
	eventRec->header.startFlag = (uint16)EVENT_RECORD_START_FLAG;
	eventRec->header.recordVersion = (uint16)EVENT_RECORD_VERSION;
	eventRec->header.headerLength = (uint16)sizeof(EVENT_HEADER_STRUCT);
	eventRec->header.summaryLength = (uint16)sizeof(EVENT_SUMMARY_STRUCT);

	// The following data will be filled in when the data has been moved over to flash.
	// dataCompression, dataLength, summaryChecksum, dataChecksum;

	//-----------------------
	// EVENT_SUMMARY_STRUCT - Fill in the event VERSION_INFO_STRUCT data
	eventRec->summary.mode = op_mode;
	eventRec->summary.eventNumber = (uint16)g_currentEventNumber;

	//-----------------------
	// EVENT_SUMMARY_STRUCT - Fill in the event VERSION_INFO_STRUCT data

	byteSet(&(eventRec->summary.version.modelNumber[0]), 0, MODEL_STRING_SIZE);
	byteCpy(&(eventRec->summary.version.modelNumber[0]), &(factory_setup_rec.serial_num[0]), 15);
	byteSet(&(eventRec->summary.version.serialNumber[0]), 0, SERIAL_NUMBER_STRING_SIZE);
	byteCpy(&(eventRec->summary.version.serialNumber[0]), &(factory_setup_rec.serial_num[0]), 15);
	byteSet(&(eventRec->summary.version.softwareVersion[0]), 0, VERSION_STRING_SIZE);
	byteCpy(&(eventRec->summary.version.softwareVersion[0]), &(appVersion[0]), VERSION_STRING_SIZE - 1);

	//-----------------------
	// EVENT_SUMMARY_STRUCT - Fill in the event PARAMETERS_STRUCT data

	eventRec->summary.parameters.bitAccuracy = 12;
	eventRec->summary.parameters.numOfChannels = 4;
	// 8 is the total number of records to copy over.
	for (idex = 0; idex < 8; idex++)
	{
		eventRec->summary.parameters.channel[idex].type = msgs430.startMsg430.channel[idex].channel_type;
		eventRec->summary.parameters.channel[idex].input = msgs430.startMsg430.channel[idex].channel_num;
		eventRec->summary.parameters.channel[idex].group = msgs430.startMsg430.channel[idex].group_num;

		if (op_mode == MANUAL_CAL_MODE)
		{
			eventRec->summary.parameters.channel[idex].options = GAIN_SELECT_x2;
		}
		else // Waveform, Bargraph, Combo
		{
			eventRec->summary.parameters.channel[idex].options = msgs430.startMsg430.channel[idex].options;
		}
	}

	if (eventRec->summary.mode == MANUAL_CAL_MODE)
	{
		eventRec->summary.parameters.sampleRate = 1024;
	}
	else
	{
		eventRec->summary.parameters.sampleRate = (uint16)trig_rec.trec.sample_rate;
	}

	eventRec->summary.parameters.aWeighting = (uint8)factory_setup_rec.aweight_option;
	eventRec->summary.parameters.seismicSensorType = (uint16)factory_setup_rec.sensor_type;
	eventRec->summary.parameters.airSensorType = (uint16)0x0;
	eventRec->summary.parameters.distToSource = (uint32)(trig_rec.trec.dist_to_source * 100.0);
	eventRec->summary.parameters.weightPerDelay = (uint32)(trig_rec.trec.weight_per_delay * 100.0);

	if ((eventRec->summary.mode == WAVEFORM_MODE) || (eventRec->summary.mode == MANUAL_CAL_MODE))
	{
		eventRec->summary.captured.batteryLevel = (uint32)(100.0 * convertedBatteryLevel(BATTERY_VOLTAGE));
		eventRec->summary.captured.endTime = getCurrentTime();
		eventRec->summary.captured.printerStatus = (uint8)(help_rec.auto_print);
		eventRec->summary.captured.calDate = factory_setup_rec.cal_date;

		eventRec->summary.parameters.numOfSamples = (uint16)(trig_rec.trec.sample_rate * trig_rec.trec.record_time);
		eventRec->summary.parameters.preBuffNumOfSamples = (uint16)(trig_rec.trec.sample_rate / 4);
		eventRec->summary.parameters.calDataNumOfSamples = (uint16)(CALIBRATION_NUMBER_OF_SAMPLES);
	}

	if ((eventRec->summary.mode == MANUAL_CAL_MODE) || (eventRec->summary.mode == BARGRAPH_MODE))
	{
		eventRec->summary.parameters.numOfSamples = 0;
		eventRec->summary.parameters.preBuffNumOfSamples = 0;
	}

	if (eventRec->summary.mode == BARGRAPH_MODE)
	{
		eventRec->summary.parameters.calDataNumOfSamples = 0;
		eventRec->summary.parameters.activeChannels = trig_rec.berec.barChannel;
	}

	byteSet(&(eventRec->summary.parameters.companyName[0]), 0, COMPANY_NAME_STRING_SIZE);
	byteCpy(&(eventRec->summary.parameters.companyName[0]), &(trig_rec.trec.client[0]), COMPANY_NAME_STRING_SIZE - 1);
	byteSet(&(eventRec->summary.parameters.seismicOperator[0]), 0, SEISMIC_OPERATOR_STRING_SIZE);
	byteCpy(&(eventRec->summary.parameters.seismicOperator[0]), &(trig_rec.trec.oper[0]), SEISMIC_OPERATOR_STRING_SIZE - 1);
	byteSet(&(eventRec->summary.parameters.sessionLocation[0]), 0, SESSION_LOCATION_STRING_SIZE);
	byteCpy(&(eventRec->summary.parameters.sessionLocation[0]), &(trig_rec.trec.loc[0]), SESSION_LOCATION_STRING_SIZE - 1);
	byteSet(&(eventRec->summary.parameters.sessionComments[0]), 0, SESSION_COMMENTS_STRING_SIZE);
	byteCpy(&(eventRec->summary.parameters.sessionComments[0]), &(trig_rec.trec.comments[0]), sizeof(trig_rec.trec.comments));

	// Waveform specific
	eventRec->summary.parameters.seismicTriggerLevel = (uint32)trig_rec.trec.seismicTriggerLevel;
	eventRec->summary.parameters.airTriggerLevel = (uint32)trig_rec.trec.soundTriggerLevel;
	eventRec->summary.parameters.recordTime = (uint32)trig_rec.trec.record_time;

	// Bargraph specific
	eventRec->summary.parameters.barInterval = (uint16)trig_rec.bgrec.barInterval;
	eventRec->summary.parameters.summaryInterval = (uint16)trig_rec.bgrec.summaryInterval;

	// SUMMARY_EVENT_PARAMETERS -
	// Fill in the event Parameter data structure at the end of the event.
	// Fill in the data length, data checksum and summary checksum at the end of the event.
}

//*****************************************************************************
// FUNCTION
//  void InitUniqueEventNumber(void)
//*****************************************************************************
void initCurrentEventNumber(void)
{
#if 0 // fix_ns8100
	// Get the starting address of the 2nd boot sector (base + boot sector size)
	//uint16* uniqueEventNumberStartPtr = (uint16*)(FLASH_BASE_ADDR + FLASH_BOOT_SECTOR_SIZE_x8);
	uint16 uniqueEventNumberOffset = (FLASH_BOOT_SECTOR_SIZE_x8);
	uint16 eventNumber = 0;

#if 0 // ns7100
	// Get the end sector address (Start of the 3rd boot sector)
	//uint16* endPtr = (uint16*)(FLASH_BASE_ADDR + (FLASH_BOOT_SECTOR_SIZE_x8 * 2));
	uint16 endOffset = (FLASH_BOOT_SECTOR_SIZE_x8 * 2);

	GetParameterMemory((uint8*)&eventNumber, uniqueEventNumberOffset, sizeof(eventNumber));

	// Check if the first location is 0xFF which indicates the unit hasn't recorded any events
	if (eventNumber == 0xFFFF)
	{
		g_currentEventNumber = 1;
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

		// Get the current event number which is the las in the list
		GetParameterMemory((uint8*)&eventNumber, uniqueEventNumberOffset, sizeof(eventNumber));

		// Set the Current Event number to the last event number stored plus 1
		g_currentEventNumber = (uint16)(eventNumber + 1);
	}
#else
	GetParameterMemory((uint8*)&eventNumber, uniqueEventNumberOffset, sizeof(eventNumber));

	// Check if the storage location is 0xFF which indicates the unit hasn't recorded any events
	if (eventNumber == 0xFFFF)
	{
		g_currentEventNumber = 1;
	}
	else
	{
		// Set the Current Event number to the last event number stored plus 1
		g_currentEventNumber = (uint16)(eventNumber + 1);
	}
#endif
#endif

#if 1 // fix_ns8100
	g_currentEventNumber = 1;
#endif

	debug("Total Events stored: %d, Current Event number: %d\n",
		(g_currentEventNumber - 1), g_currentEventNumber);
}

//*****************************************************************************
// FUNCTION
//  void getLastStoredEventNumber(void)
//*****************************************************************************
uint16 getLastStoredEventNumber(void)
{
	return ((uint16)(g_currentEventNumber - 1));
}

//*****************************************************************************
// FUNCTION
//  void storeCurrentEventNumber(void)
//*****************************************************************************
void storeCurrentEventNumber(void)
{
	//uint16* uniqueEventStorePtr = (uint16*)(FLASH_BASE_ADDR + FLASH_BOOT_SECTOR_SIZE_x8);
	uint16 uniqueEventStoreOffset = FLASH_BOOT_SECTOR_SIZE_x8;

#if 0
	uint16 positionsToAdvance = 0;
	uint16 eventNumber = 0;

	// If the Current Event Number is 0, then we have wrapped (>65535) in which case we will reinitialize
	if (g_currentEventNumber == 0)
	{
		//sectorErase(uniqueEventStorePtr, 1);
		EraseParameterMemory(uniqueEventStoreOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		initCurrentEventNumber();
	}
	// Check if at the boundary of a flash sectors worth of Unique Event numbers
	else if (((g_currentEventNumber - 1) % 4096) == 0)
	{
		// Check to make sure this isnt the initial (first) event number
		if (g_currentEventNumber != 1)
		{
			//sectorErase(uniqueEventStorePtr, 1);
			EraseParameterMemory(uniqueEventStoreOffset, FLASH_BOOT_SECTOR_SIZE_x8);
		}
	}
	else // Still room to store Unique Event numbers
	{
		// Get the positions to advance based on the current event number mod'ed by the storage size in event numbers
		positionsToAdvance = (uint16)((g_currentEventNumber - 1) % 4096);

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
			//programWord(uniqueEventStorePtr, g_currentEventNumber);
			SaveParameterMemory((uint8*)&g_currentEventNumber, uniqueEventStoreOffset, sizeof(g_currentEventNumber));

			// Store as the last Event recorded in AutoDialout table
			__autoDialoutTbl.lastStoredEvent = g_currentEventNumber;

			// Increment to a new Event number
			g_currentEventNumber++;
			debug("Unique Event numbers stored: %d, Current Event number: %d\n",
				(g_currentEventNumber - 1), g_currentEventNumber);

			return;
		}
	}

	// If we get here, then we failed a validation check
	debugErr("Unique Event number storage doesnt match Current Event Number. (0x%x, %d)\n",
				uniqueEventStoreOffset, g_currentEventNumber);
#else
	// Store the Current Event number as the newest Unique Event number
	SaveParameterMemory((uint8*)&g_currentEventNumber, uniqueEventStoreOffset, sizeof(g_currentEventNumber));

	// Store as the last Event recorded in AutoDialout table
	__autoDialoutTbl.lastStoredEvent = g_currentEventNumber;

	// Increment to a new Event number
	g_currentEventNumber++;
	debug("Unique Event numbers stored: %d, Current Event number: %d\n",
		(g_currentEventNumber - 1), g_currentEventNumber);

	return;
#endif
}

//*****************************************************************************
// FUNCTION
//  uint16 getUniqueEventNumber(SUMMARY_DATA*)
//*****************************************************************************
uint16 getUniqueEventNumber(SUMMARY_DATA* currentSummary)
{
	EVT_RECORD* currentStoredEvent = (EVT_RECORD*)currentSummary->linkPtr;

	return (currentStoredEvent->summary.eventNumber);
}

//*****************************************************************************
// FUNCTION
//  uint16 GetFlashSumEntry(SUMMARY_DATA* entryPtr)
//
// DESCRIPTION
//  general description of function.
//
// INPUTS
//  inputs to the function
//
// HISTORY
//  DATE         NAME              REMARKS
//   5/11/2003 - Paul Hankins      - Initial Coding
//*****************************************************************************
uint16 GetFlashSumEntry(SUMMARY_DATA** sumEntryPtr)
{
	uint16 success = TRUE;

	if ((numOfFlashSummarys == LAST_RAM_SUMMARY_INDEX) && (noFlashWrapFlag == TRUE))
	{
		*sumEntryPtr = 0;
		success = FALSE;
	}
	else
	{
		// Check if we have reached the max num of summarys and wrapping is enabled
		if ((numOfFlashSummarys == LAST_RAM_SUMMARY_INDEX) && (noFlashWrapFlag == FALSE))
		{
			// Clear out oldest summary
			if (currFlashSummary == LAST_RAM_SUMMARY_INDEX)
			{
				__ramFlashSummaryTbl[0].linkPtr = (uint16*)0xFFFFFFFF;
			}
			else
			{
				__ramFlashSummaryTbl[currFlashSummary + 1].linkPtr = (uint16*)0xFFFFFFFF;
			}
		}

		// Check if the current flash pointer address is long-bounded
		if ((uint32)sFlashDataPtr & 0x00000003)
		{
			// If the address is not long-bounded, make it so
			sFlashDataPtr = (uint16*)(((uint32)sFlashDataPtr & 0xFFFFFFFC) + 1);
		}

		// Check if we can store the event record structure (not including raw data) in the current sector
		if (((uint8*)sFlashDataPtr + sizeof(EVT_RECORD)) >= (uint8*)endFlashSectorPtr)
		{
			// Check if we are at the end of the flash buffer (special case)
			if (endFlashSectorPtr >= (uint16*)FLASH_EVENT_END)
			{
				// Reset the end of flash sector ptr and the current flash pointer address
				endFlashSectorPtr = (uint16*)FLASH_EVENT_START;
				sFlashDataPtr = (uint16*)FLASH_EVENT_START;
			}

			// Need to clear space, issue a sector erase
			ReclaimSpace(endFlashSectorPtr);
			// Incrememnt the flash sector boundary to the next sector
			endFlashSectorPtr += FLASH_SECTOR_SIZE_x16;
		}

		// Set the flash pointer address in the ram summary
		__ramFlashSummaryTbl[currFlashSummary].linkPtr = (uint16*)sFlashDataPtr;

		// Set the current summary pointer to the current ram summary
		*sumEntryPtr = &__ramFlashSummaryTbl[currFlashSummary];

		debug("Ram Summary Table Entry %d: Event pointer: 0x%x\n", currFlashSummary, sFlashDataPtr);

		// Increment the flash summary value.
		currFlashSummary++;

		if (currFlashSummary > LAST_RAM_SUMMARY_INDEX)
		{
			currFlashSummary = 0;
		}

		if (numOfFlashSummarys != LAST_RAM_SUMMARY_INDEX)
		{
			numOfFlashSummarys++;
		}
	}

	debug("Number of Event Summaries in RAM: %d\n", numOfFlashSummarys);

	return (success);
}

/*****************************************************************************
  FUNCTION
		void FillFlashSummarys(SUMMARY_DATA* flashSummPtr, SUMMARY_DATA* ramSummPtr)

  DESCRIPTION
	Write in the following structures into flash. Fill them in with data from the ram
	record and then write to flash.	The flashSummPtr->linkPtr points to the valid flash
	location. flashSummPtr is a structure in ram.

  INPUTS
	SUMMARY_DATA* flashSummPtr 	- location of the flash memory
	SUMMARY_DATA* ramSummPtr	- location of the ram memory
*****************************************************************************/
void FillInFlashSummarys(SUMMARY_DATA* flashSummPtr, SUMMARY_DATA* ramSummPtr)
{
	//uint8 i = 0;
	uint32 eventDataSize = 0;
	EVT_RECORD* flashEventRecord = (EVT_RECORD*)flashSummPtr->linkPtr;

	//--------------------------------
	// EVT_RECORD -

	// Check if Waveform or Manual Cal mode
	if ((g_RamEventRecord.summary.mode == WAVEFORM_MODE) || (g_RamEventRecord.summary.mode == MANUAL_CAL_MODE))
	{
		// Fill in calculated data (Bargraph data filled in at the end of bargraph)
		g_RamEventRecord.summary.calculated.a.peak = ramSummPtr->waveShapeData.a.peak;
		g_RamEventRecord.summary.calculated.r.peak = ramSummPtr->waveShapeData.r.peak;
		g_RamEventRecord.summary.calculated.v.peak = ramSummPtr->waveShapeData.v.peak;
		g_RamEventRecord.summary.calculated.t.peak = ramSummPtr->waveShapeData.t.peak;

		g_RamEventRecord.summary.calculated.a.frequency = ramSummPtr->waveShapeData.a.freq;
		g_RamEventRecord.summary.calculated.r.frequency = ramSummPtr->waveShapeData.r.freq;
		g_RamEventRecord.summary.calculated.v.frequency = ramSummPtr->waveShapeData.v.freq;
		g_RamEventRecord.summary.calculated.t.frequency = ramSummPtr->waveShapeData.t.freq;

		g_RamEventRecord.summary.calculated.a.displacement = 0;

		// Calculate Displacement as PPV/(2 * PI * Freq) with 1000000 to shift to keep accuracy and the 10 to adjust the frequency
		g_RamEventRecord.summary.calculated.r.displacement =
			(uint32)(ramSummPtr->waveShapeData.r.peak * 1000000 / 2 / PI / ramSummPtr->waveShapeData.r.freq * 10);
		g_RamEventRecord.summary.calculated.v.displacement =
			(uint32)(ramSummPtr->waveShapeData.v.peak * 1000000 / 2 / PI / ramSummPtr->waveShapeData.v.freq * 10);
		g_RamEventRecord.summary.calculated.t.displacement =
			(uint32)(ramSummPtr->waveShapeData.t.peak * 1000000 / 2 / PI / ramSummPtr->waveShapeData.t.freq * 10);
	}

	//--------------------------------
	// EVENT_HEADER_STRUCT  -
	g_RamEventRecord.header.summaryChecksum = 0;
	g_RamEventRecord.header.dataChecksum = 0;

	// *** Fix bug with summary.parameters.numOfSamples bug where a value can be larger than the uint16 it holds
	if ((g_RamEventRecord.summary.mode == WAVEFORM_MODE) && ((g_RamEventRecord.summary.parameters.sampleRate *
		g_RamEventRecord.summary.parameters.recordTime) > 0xFFFF))
	{
		// Store the event data size in bytes, It's (number of channels) * (pre + event + cal sample) * 2 bytes
		g_RamEventRecord.header.dataLength = (uint32)((g_RamEventRecord.summary.parameters.numOfChannels *
								 			 (g_RamEventRecord.summary.parameters.preBuffNumOfSamples +
							 				 (g_RamEventRecord.summary.parameters.sampleRate *
											 g_RamEventRecord.summary.parameters.recordTime) +
											 g_RamEventRecord.summary.parameters.calDataNumOfSamples)) * 2);
	}
	else
	{
		// Store the event data size in bytes, It's (number of channels) * (pre + event + cal sample) * 2 bytes
		g_RamEventRecord.header.dataLength = (uint32)((g_RamEventRecord.summary.parameters.numOfChannels *
								 			 (g_RamEventRecord.summary.parameters.preBuffNumOfSamples +
							 				 g_RamEventRecord.summary.parameters.numOfSamples +
											 g_RamEventRecord.summary.parameters.calDataNumOfSamples)) * 2);
	}

	//--------------------------------
	// Get the structure size as the number of words. The following will round up.
	eventDataSize = (sizeof(EVT_RECORD) + 1) / 2;
	flashWrite((uint16*)flashEventRecord, (uint16*)&(g_RamEventRecord), eventDataSize);

	updateMonitorLogEntry();

	// After event numbers have been saved, store current event number in persistent storage.
	storeCurrentEventNumber();

	// Now store the updated event number in the universal ram storage.
	g_RamEventRecord.summary.eventNumber = g_currentEventNumber;
}

//******************************************************************************
// Function:	ReclaimSpace
//******************************************************************************
void ReclaimSpace(uint16* sectorAddr)
{
	uint8 needToEraseSector = NO;
	uint16 tempNumOfFlashSummarys = numOfFlashSummarys;
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
			numOfFlashSummarys--;
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

		overlayMessage(getLangText(STATUS_TEXT), &message[0], (2 * SOFT_SECS));
	}

	// Check if number of summaries changed
	if (numOfFlashSummarys == tempNumOfFlashSummarys)
		debug("Number of Event Summaries now in RAM: %d\n", numOfFlashSummarys);

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

		if (help_rec.flash_wrapping == NO)
		{
			overlayMessage(getLangText(WARNING_TEXT), "WRAPPING DISABLED BUT FORCED TO ERASE SOME FLASH", (5 * SOFT_SECS));
		}

		sectorErase(sectorAddr, 1);
	}
}

/*******************************************************************************
*	Function:	getFlashDataPointer
*	Purpose:
*******************************************************************************/
uint16* getFlashDataPointer(void)
{
	return (sFlashDataPtr);
}

/*******************************************************************************
*	Function:	checkFlashDataPointer
*	Purpose:
*******************************************************************************/
void checkFlashDataPointer(void)
{
	// Check if current flash data pointer is equal to or past the flash sector boundary
	if (sFlashDataPtr >= endFlashSectorPtr)
	{
		// Check if the current flash data pointer is equal to or past the end of the flash memory
		if (endFlashSectorPtr == (uint16*)FLASH_EVENT_END)
		{
			// Reset pointers
			endFlashSectorPtr = (uint16*)FLASH_EVENT_START;
			sFlashDataPtr = (uint16*)FLASH_EVENT_START;
		}

		// Erase the current sector
		ReclaimSpace(endFlashSectorPtr);

		// Set the flash sector boundary to the beginning of the next sector
		endFlashSectorPtr = endFlashSectorPtr + FLASH_SECTOR_SIZE_x16;
	}
}

/*******************************************************************************
*	Function:	storeData
*	Purpose:
*******************************************************************************/
void storeData(uint16* dataPtr, uint16 dataWords)
{
#if 0 // fix_ns8100
	uint8 i = 0;

	for (i = 0; i < dataWords; i++)
	{
		checkFlashDataPointer();
		programWord(sFlashDataPtr++, *(dataPtr + i));
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

/*******************************************************************************
*	Function:	flashUsage
*	Purpose:
*******************************************************************************/
FLASH_USAGE_STRUCT getFlashUsageStats(void)
{
	FLASH_USAGE_STRUCT usage;
	uint32 waveSize;
	uint32 barSize;
	uint32 manualCalSize;
	uint32 newBargraphMinSize;
	uint16 i = 0;

	waveSize = sizeof(EVT_RECORD) + (100 * 8) + (uint32)(trig_rec.trec.sample_rate * 2) +
				(uint32)(trig_rec.trec.sample_rate * trig_rec.trec.record_time * 8);
	barSize = (uint32)(((3600 * 8) / trig_rec.bgrec.barInterval) + (sizeof(EVT_RECORD) / 24) +
				((3600 * sizeof(CALCULATED_DATA_STRUCT)) / trig_rec.bgrec.summaryInterval));
	manualCalSize = sizeof(EVT_RECORD) + (100 * 8);
	newBargraphMinSize = (manualCalSize + sizeof(EVT_RECORD) + (sizeof(CALCULATED_DATA_STRUCT) * 2) +
							(((trig_rec.bgrec.summaryInterval / trig_rec.bgrec.barInterval) + 1) * 8 * 2));


	usage.sizeUsed = (uint32)sFlashDataPtr - FLASH_EVENT_START;
	usage.sizeFree = FLASH_EVENT_END - (uint32)sFlashDataPtr;
	usage.percentUsed = (uint8)((usage.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
	usage.percentFree = (uint8)(100 - usage.percentUsed);
	usage.waveEventsLeft = (uint16)(usage.sizeFree / waveSize);
	usage.barHoursLeft = (uint16)(usage.sizeFree / barSize);
	usage.manualCalsLeft = (uint16)(usage.sizeFree / manualCalSize);
	usage.wrapped = NO;
	usage.roomForBargraph = (usage.sizeFree > newBargraphMinSize) ? YES : NO;

	for (i = 0; i < TOTAL_RAM_SUMMARIES; i++)
	{
		if ((__ramFlashSummaryTbl[i].linkPtr != (uint16*)0xFFFFFFFF) && (__ramFlashSummaryTbl[i].linkPtr > sFlashDataPtr))
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
		usage.sizeUsed = (FLASH_EVENT_END - FLASH_EVENT_START) - ((uint32)endFlashSectorPtr - (uint32)sFlashDataPtr);
		usage.sizeFree = ((uint32)endFlashSectorPtr - (uint32)sFlashDataPtr);
		usage.percentUsed = (uint8)(((uint32)usage.sizeUsed * 100) / (FLASH_EVENT_END - FLASH_EVENT_START));
		usage.percentFree = (uint8)(100 - usage.percentUsed);
		usage.waveEventsLeft = (uint16)(usage.sizeFree / waveSize);
		usage.barHoursLeft = (uint16)(usage.sizeFree / barSize);
		usage.manualCalsLeft = (uint16)(usage.sizeFree / manualCalSize);
		usage.roomForBargraph = (usage.sizeFree > newBargraphMinSize) ? YES : NO;
	}

	return (usage);
}

