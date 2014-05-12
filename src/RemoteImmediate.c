///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: RemoteImmediate.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:58 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteImmediate.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "RemoteCommon.h"
#include "RemoteImmediate.h"
#include "RemoteOperation.h"
#include "Old_Board.h"
#include "Uart.h"
#include "Menu.h"
#include "EventProcessing.h"
#include "Msgs430.h"
#include "PowerManagement.h"
#include "Menu.h"
#include "SysEvents.h"
#include "Crc.h"
#include "Minilzo.h"
#include "TextTypes.h"

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
static VMLx_XFER_STRUCT s_vmlXferStruct;

//==================================================
// Function: handleUNL
// Description:
// 		Unlock unit.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleUNL(CMD_BUFFER_STRUCT* inCmd)
{
	uint8 matchFlag = NO;
	uint8* dataStart = inCmd->msg + 3;
	char tempStr[UNLOCK_STR_SIZE];
	char unlStr[UNLOCK_STR_SIZE];
	char sendStr[UNLOCK_STR_SIZE*2];

	debug("handleUNL-user unlock code=<%s>\n", dataStart);

	byteSet(&tempStr[0], 0, sizeof(tempStr));

	sprintf(unlStr,"%04d", g_modemSetupRecord.unlockCode);

	if (0 == strncmp((const char*)(unlStr), (const char*)dataStart, UNLOCK_CODE_SIZE))
	{
		matchFlag = YES;
		strcpy(tempStr, (char*)UNL_USER_RESP_STRING);
	}
	else if (0 == strncmp((const char*)UNLOCK_CODE_DEFAULT, (const char*)dataStart, UNLOCK_CODE_SIZE))
	{
		matchFlag = YES;
		strcpy(tempStr, (char*)UNL_USER_RESP_STRING);
	}
	else if (0 == strncmp((const char*)UNLOCK_CODE_ADMIN, (const char*)dataStart, UNLOCK_CODE_SIZE))
	{
		matchFlag = YES;
		strcpy(tempStr, (char*)UNL_ADMIN_RESP_STRING);
	}
	else if (0 == strncmp((const char*)UNLOCK_CODE_PROG, (const char*)dataStart, UNLOCK_CODE_SIZE))
	{
		matchFlag = YES;
		strcpy(tempStr, (char*)UNL_PROG_RESP_STRING);
	}

	if (YES == matchFlag)
	{
		debug("handleUNL-MatchCode=<%s>\n", dataStart);

		byteSet(&sendStr[0], 0, sizeof(sendStr));
		if (YES == g_modemStatus.systemIsLockedFlag)
		{
			g_modemStatus.systemIsLockedFlag = NO;
			sprintf(sendStr,"%s0", tempStr);

			// Check to see if there is a binary flag set.
			// The default is to convert he data to ascii and transmit.
			if ('B' == inCmd->msg[UNLOCK_FLAG_LOC])
			{
				g_binaryXferFlag = NO_CONVERSION;
			}
			else
			{
				g_binaryXferFlag = CONVERT_DATA_TO_ASCII;
			}

			// Receiving successful unlock, update last successful connect time
			__autoDialoutTbl.lastConnectTime = getCurrentTime();
			__autoDialoutTbl.lastConnectTime.valid = YES;
		}
		else
		{
			g_modemStatus.systemIsLockedFlag = YES;
			g_binaryXferFlag = CONVERT_DATA_TO_ASCII;
			sprintf(sendStr,"%s1", tempStr);
		}

		modem_puts((uint8*)(sendStr), strlen(sendStr), CONVERT_DATA_TO_ASCII);
		modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

		g_modemStatus.xferState = NOP_CMD;
	}

	return;
}


//==================================================
// Function: handleRST
// Description:
// 		Reset the unit.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleRST(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	// Check if the unit is in monitor mode
	if (g_sampleProcessing == SAMPLING_STATE)
	{
		// Turn printing off
		g_helpRecord.auto_print = NO;

		stopMonitoring(g_triggerRecord.op_mode, FINISH_PROCESSING);
	}

	if(g_helpRecord.timer_mode == ENABLED)
	{
		// Disable Timer mode since restarting would force a prompt for user action
		g_helpRecord.timer_mode = DISABLED;
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		overlayMessage(getLangText(WARNING_TEXT), getLangText(TIMER_MODE_DISABLED_TEXT), (2 * SOFT_SECS));
	}

	PowerUnitOff(RESET_UNIT);

	return;
}

//==================================================
// Function: handleDDP
// Description:
// 		Disable Debug Printing, This would be a toggle.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleDDP(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	if (	g_disableDebugPrinting == YES)
	{
		g_disableDebugPrinting = NO;
	}
	else
	{
		g_disableDebugPrinting = YES;
	}

	return;
}

//==================================================
// Function: handleDAI
// Description:
// 		Download Application Image.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleDAI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	debug("handleDAI:Here\n");

	// If the function is valid then this call will never return, otherwise proceed as if we can't jump
	jumpToBootFunction();

	// Issue something to the user to alert them that the DAI command is not functional with this unit
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
	modem_puts((uint8*)DAI_ERR_RESP_STRING, sizeof(DAI_ERR_RESP_STRING), CONVERT_DATA_TO_ASCII);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	return;
}

//==================================================
// Function: handleESM
// Description:
// 		Erase summary memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleESM(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleEEM
// Description:
// 		Erase event memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleEEM(CMD_BUFFER_STRUCT* inCmd)
{
	uint8 eemHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+1];
	uint8 returnCode = MSGTYPE_RESPONSE;
	uint32	msgCRC = 0;
	UNUSED(inCmd);


	if (SAMPLING_STATE == g_sampleProcessing)
	{
		// Error in message length
		returnCode = CFG_ERR_MONITORING_STATE;
	}
	else
	{
#if 0 // ns7100
		// Start erasing the sector. Re-Init the ram summary table and the flash buffers
		// Erase all the data sectors (leave the boots sectors alone)
		sectorErase((uint16*)(FLASH_BASE_ADDR + FLASH_SECTOR_SIZE_x8),
					TOTAL_FLASH_DATA_SECTORS);
#else // ns8100
		// fix_ns8100
#endif

		initRamSummaryTbl();
		InitFlashBuffs();
	}

	sprintf((char*)msgTypeStr, "%02d", returnCode);
	buildOutgoingSimpleHeaderBuffer((uint8*)eemHdr, (uint8*)"EEMx",
		(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
	// Send Simple header
	modem_puts((uint8*)eemHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);
	// Send Ending Footer
	modem_puts((uint8*)&msgCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	return;
}

//==================================================
// Function: handleECM
// Description:
// 		Erase configuration memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleECM(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleTRG
// Description:
// 		Trigger an event.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleTRG(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVML
// Description:
//--------------------------------------------------
void handleVML(CMD_BUFFER_STRUCT* inCmd)
{
	// Set the data pointer to start after the VML character data bytes
	uint16* dataPtr = (uint16*)(inCmd->msg + MESSAGE_HEADER_SIMPLE_LENGTH);

	// Save the last downloaded unique entry ID
	s_vmlXferStruct.lastDlUniqueEntryId = *dataPtr;

	// Init the start and temp monitor log table indicies
	s_vmlXferStruct.startMonitorLogTableIndex = getStartingMonitorLogTableIndex();
	s_vmlXferStruct.tempMonitorLogTableIndex = TOTAL_MONITOR_LOG_ENTRIES;

	// Set the transfer state command to the VML command
	g_modemStatus.xferState = VMLx_CMD;

	// Set the transfer state flag to the start with the header
	s_vmlXferStruct.xferStateFlag = HEADER_XFER_STATE;

	// Set the transfer mutex since the response will be handled on multiple passes
	g_modemStatus.xferMutex = YES;

	// Save off the printer state to allow the state to be reset when done with the command
	g_modemStatus.xferPrintState = g_helpRecord.auto_print;
}

//==================================================
// Function:	sendVMLData
// Description:
//--------------------------------------------------
void sendVMLData(void)
{
	uint32 dataLength;
	uint16 i = 0;
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	uint8 status;

	// Check if handling the header
	if (s_vmlXferStruct.xferStateFlag == HEADER_XFER_STATE)
	{
		// Transmit a carrige return line feed
		if (modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			// There was an error, so reset all global transfer and status fields
			s_vmlXferStruct.xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_modemStatus.xferState = NOP_CMD;
			g_transferCount = 0;

			return;
		}

		// Get the data length, which is the total number of new entries plus 1 (non-valid end 0xCC entry) times the log entry size
		dataLength = ((numOfNewMonitorLogEntries(s_vmlXferStruct.lastDlUniqueEntryId) + 1) * sizeof(MONITOR_LOG_ENTRY_STRUCT));

		// Signal a message response in the message type string
		sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);

		// Build the outgoing message header
		buildOutgoingSimpleHeaderBuffer((uint8*)&(s_vmlXferStruct.vmlHdr), (uint8*)"VMLx", (uint8*)msgTypeStr,
										(MESSAGE_SIMPLE_TOTAL_LENGTH + dataLength), COMPRESS_NONE, CRC_32BIT);

		// Calculate the CRC on the header
		g_transmitCRC = CalcCCITT32((uint8*)&(s_vmlXferStruct.vmlHdr), MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);

		// Send the header out the modem
		if (modem_puts((uint8*)&(s_vmlXferStruct.vmlHdr), MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag) == MODEM_SEND_FAILED)
		{
			// There was an error, so reset all global transfer and status fields
			s_vmlXferStruct.xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_modemStatus.xferState = NOP_CMD;
			g_transferCount = 0;

			return;
		}

		// Done with the header, move to data transfer
		s_vmlXferStruct.xferStateFlag = DATA_XFER_STATE;
	}
	else if (s_vmlXferStruct.xferStateFlag == DATA_XFER_STATE)
	{
		// Loop up to the total number of vml data log entries
		for (i = 0; i < VML_DATA_LOG_ENTRIES; i++)
		{
			// Get the next valid monitor log entry (routine will store it into the data buffer)
			status = getNextMonitorLogEntry(s_vmlXferStruct.lastDlUniqueEntryId, s_vmlXferStruct.startMonitorLogTableIndex,
											&(s_vmlXferStruct.tempMonitorLogTableIndex), &(s_vmlXferStruct.vmlData[i]));

			// Check if the status indicates that no more valid entries were found
			if (status == NO)
			{
				// Write all 0xCC's to a log entry to mark the end of the data (following a convention used in the DQM command)
				byteSet(&(s_vmlXferStruct.vmlData[i]), 0xCC, sizeof(MONITOR_LOG_ENTRY_STRUCT));

				// Reached the end of the data, set state to handle footer next
				s_vmlXferStruct.xferStateFlag = FOOTER_XFER_STATE;

				// Since we're done, add 1 to the total count of entries in the buffer
				i++;

				// Break out of the for loop since there are no more entries
				break;
			}
		}

		// Calculate the CRC on the data
		g_transmitCRC = CalcCCITT32((uint8*)&(s_vmlXferStruct.vmlData[0]), (i * sizeof(MONITOR_LOG_ENTRY_STRUCT)), g_transmitCRC);

		// Send the data out the modem
		if (modem_puts((uint8*)&(s_vmlXferStruct.vmlData[0]), (i * sizeof(MONITOR_LOG_ENTRY_STRUCT)), g_binaryXferFlag) == MODEM_SEND_FAILED)
		{
			// There was an error, so reset all global transfer and status fields
			s_vmlXferStruct.xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_modemStatus.xferState = NOP_CMD;
			g_transferCount = 0;

			return;
		}
	}
	else if (s_vmlXferStruct.xferStateFlag == FOOTER_XFER_STATE)
	{
		modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
		modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

		// Done with the command, reset all global transfer and status fields
		s_vmlXferStruct.xferStateFlag = NOP_XFER_STATE;
		g_modemStatus.xferState = NOP_CMD;
		g_modemStatus.xferMutex = NO;
		g_transferCount = 0;
	}
}

//==================================================
// Function: handleUDE
// Description:
//--------------------------------------------------
void handleUDE(CMD_BUFFER_STRUCT* inCmd)
{
	// Set the data pointer to start after the UDE character data bytes
	uint16* dataPtr = (uint16*)(inCmd->msg + MESSAGE_HEADER_SIMPLE_LENGTH);

	if(*dataPtr < g_currentEventNumber)
		__autoDialoutTbl.lastDownloadedEvent = *dataPtr;

	// Done with the command, reset all global transfer and status fields
	s_vmlXferStruct.xferStateFlag = NOP_XFER_STATE;
	g_modemStatus.xferState = NOP_CMD;
	g_modemStatus.xferMutex = NO;
	g_transferCount = 0;
}

//==================================================
// Function: handleGAD
// Description:
//--------------------------------------------------
void handleGAD(CMD_BUFFER_STRUCT* inCmd)
{
	uint32 dataLength;
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	uint8 gadHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 serialNumber[SERIAL_NUMBER_STRING_SIZE];
	AUTODIALOUT_STRUCT tempAutoDialout = __autoDialoutTbl;

	UNUSED(inCmd);

	byteSet(&serialNumber[0], 0, sizeof(serialNumber));
	strcpy((char*)&serialNumber[0], g_factorySetupRecord.serial_num);

	// Transmit a carrige return line feed
	if (modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	// Get the data length, which is the total number of new entries plus 1 (non-valid end 0xCC entry) times the log entry size
	dataLength = (SERIAL_NUMBER_STRING_SIZE + sizeof(AUTODIALOUT_STRUCT));

	// Signal a message response in the message type string
	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);

	// Build the outgoing message header
	buildOutgoingSimpleHeaderBuffer((uint8*)&(gadHdr), (uint8*)"GADx", (uint8*)msgTypeStr,
									(MESSAGE_SIMPLE_TOTAL_LENGTH + dataLength), COMPRESS_NONE, CRC_32BIT);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&(gadHdr), MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);

	// Send the header out the modem
	if (modem_puts((uint8*)&(gadHdr), MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&serialNumber[0], sizeof(serialNumber), g_transmitCRC);

	// Send the data out the modem
	if (modem_puts((uint8*)&serialNumber[0], sizeof(serialNumber), g_binaryXferFlag) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&tempAutoDialout, sizeof(AUTODIALOUT_STRUCT), g_transmitCRC);

	// Send the data out the modem
	if (modem_puts((uint8*)&tempAutoDialout, sizeof(AUTODIALOUT_STRUCT), g_binaryXferFlag) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Done with the command, reset all global transfer and status fields
	g_modemStatus.xferState = NOP_CMD;
	g_modemStatus.xferMutex = NO;
	g_transferCount = 0;

	return;
}

//==================================================
// Function: handleGFS
// Description:
//--------------------------------------------------
void handleGFS(CMD_BUFFER_STRUCT* inCmd)
{
	uint32 dataLength;
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	uint8 gfsHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	FLASH_USAGE_STRUCT usage;
	
	UNUSED(inCmd);

	getFlashUsageStats(&usage);

	// Transmit a carrige return line feed
	if (modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	// Get the data length, which is the total number of new entries plus 1 (non-valid end 0xCC entry) times the log entry size
	dataLength = sizeof(FLASH_USAGE_STRUCT);

	// Signal a message response in the message type string
	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);

	// Build the outgoing message header
	buildOutgoingSimpleHeaderBuffer((uint8*)&(gfsHdr), (uint8*)"GFSx", (uint8*)msgTypeStr,
									(MESSAGE_SIMPLE_TOTAL_LENGTH + dataLength), COMPRESS_NONE, CRC_32BIT);

	// Calculate the CRC on the header
	g_transmitCRC = CalcCCITT32((uint8*)&(gfsHdr), MESSAGE_HEADER_SIMPLE_LENGTH, SEED_32);

	// Send the header out the modem
	if (modem_puts((uint8*)&(gfsHdr), MESSAGE_HEADER_SIMPLE_LENGTH, g_binaryXferFlag) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	// Calculate the CRC on the data
	g_transmitCRC = CalcCCITT32((uint8*)&usage, sizeof(FLASH_USAGE_STRUCT), g_transmitCRC);

	// Send the data out the modem
	if (modem_puts((uint8*)&usage, sizeof(FLASH_USAGE_STRUCT), g_binaryXferFlag) == MODEM_SEND_FAILED)
	{
		// There was an error, so reset all global transfer and status fields
		g_modemStatus.xferMutex = NO;
		g_modemStatus.xferState = NOP_CMD;
		g_transferCount = 0;

		return;
	}

	modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Done with the command, reset all global transfer and status fields
	g_modemStatus.xferState = NOP_CMD;
	g_modemStatus.xferMutex = NO;
	g_transferCount = 0;

	return;
}

//==================================================
// Function: handleDQS
// Description:
// 		Download Quick Summary Memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleDQM(CMD_BUFFER_STRUCT* inCmd)
{

	// Download summary memory...
	uint16 idex;
	uint32 dataLength;						// Will hold the new data length of the message
	uint8 msgTypeStr[HDR_TYPE_LEN+2];
	uint8* dqmPtr;


	UNUSED(inCmd);

	// If the process is busy sending data, return;
	if (YES == g_modemStatus.xferMutex)
	{
		return;
	}

	byteSet((uint8*)g_dqmXferStructPtr, 0, sizeof(DQMx_XFER_STRUCT));

	// Determine the data size first. This size is needed for the header length.
	for (idex = 0; idex < TOTAL_RAM_SUMMARIES; idex++)
	{
		if (__ramFlashSummaryTbl[idex].fileEventNum != 0xFFFFFFFF)
		{
			g_dqmXferStructPtr->numOfRecs++;
		}
	}

	// Must have at least 1 record to signal the end of the data transmit.
	g_dqmXferStructPtr->numOfRecs++;

	// 4 is for the numOfRecs field.
	dataLength = 4 + (g_dqmXferStructPtr->numOfRecs * sizeof(DQMx_DATA_STRUCT));

	sprintf((char*)msgTypeStr, "%02d", MSGTYPE_RESPONSE);

	buildOutgoingSimpleHeaderBuffer((uint8*)g_dqmXferStructPtr->dqmHdr, (uint8*)"DQSx",
		(uint8*)msgTypeStr, (uint32)(MESSAGE_SIMPLE_TOTAL_LENGTH + dataLength),
		COMPRESS_NONE, CRC_32BIT);

	// Go to the start of the record length value.
	dqmPtr = (uint8*)g_dqmXferStructPtr->dqmHdr + MESSAGE_HEADER_SIMPLE_LENGTH;
	sprintf((char*)dqmPtr, "%04d", g_dqmXferStructPtr->numOfRecs);

	//-----------------------------------------------------------
	// Set the intial table index to the first element of the table
	g_dqmXferStructPtr->ramTableIndex = 0;
	g_dqmXferStructPtr->xferStateFlag = HEADER_XFER_STATE;		// This is the initial xfer state to start.
	g_modemStatus.xferState = DQMx_CMD;							// This is the xfer command state.
	g_modemStatus.xferMutex = YES;								// Mutex to prevent other commands.
	g_modemStatus.xferPrintState = g_helpRecord.auto_print;
	g_transferCount = 0;

	return;
}

//==================================================
// Function: sendDSMData
// Description:
// Input:
// Return: void
//--------------------------------------------------
uint8 sendDQMData(void)
{
	uint8 idex;
	uint8 xferState = DQMx_CMD;
	EVT_RECORD* eventRecord;

	// If the beginning of sending data, send the crlf.
	if (HEADER_XFER_STATE == g_dqmXferStructPtr->xferStateFlag)
	{
		if (MODEM_SEND_FAILED == modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION))
		{
			g_dqmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;
			return (NOP_CMD);
		}

		// g_transmitCRC will be the seed value for the rest of the CRC calculations.
		g_transmitCRC = CalcCCITT32((uint8*)g_dqmXferStructPtr->dqmHdr,
			(MESSAGE_HEADER_SIMPLE_LENGTH + 4), SEED_32);

		// Copy the hdr length plus the 4 of the record length.
		if (MODEM_SEND_FAILED == modem_puts((uint8*)g_dqmXferStructPtr->dqmHdr,
			(MESSAGE_HEADER_SIMPLE_LENGTH + 4), g_binaryXferFlag))
		{
			g_dqmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;
			return (NOP_CMD);
		}

		g_dqmXferStructPtr->xferStateFlag = SUMMARY_TABLE_SEARCH_STATE;
		g_transferCount = MESSAGE_HEADER_LENGTH + 4 + 2;
	}


	// xfer the event record structure.
	else if (SUMMARY_TABLE_SEARCH_STATE == g_dqmXferStructPtr->xferStateFlag)
	{
		if (g_dqmXferStructPtr->ramTableIndex >= TOTAL_RAM_SUMMARIES)
		{
			byteSet((uint8*)g_dqmXferStructPtr->dqmData, 0xCC, sizeof(DQMx_DATA_STRUCT));
			g_dqmXferStructPtr->dqmData[0].endFlag = 0xEE;

			g_transmitCRC = CalcCCITT32((uint8*)g_dqmXferStructPtr->dqmData,
				sizeof(DQMx_DATA_STRUCT), g_transmitCRC);

			if (MODEM_SEND_FAILED == modem_puts((uint8*)g_dqmXferStructPtr->dqmData,
				sizeof(DQMx_DATA_STRUCT), g_binaryXferFlag))
			{
				g_dqmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
				g_modemStatus.xferMutex = NO;
				g_transferCount = 0;
				return (NOP_CMD);
			}
			g_dqmXferStructPtr->xferStateFlag = FOOTER_XFER_STATE;
		}
		else
		{
			idex = 0;
			while ((idex < DQM_XFER_SIZE) &&
				(g_dqmXferStructPtr->ramTableIndex < TOTAL_RAM_SUMMARIES))
			{
				if (__ramFlashSummaryTbl[g_dqmXferStructPtr->ramTableIndex].fileEventNum != 0xFFFFFFFF)
				{
					// Assign it a new ptr, easier to read and handle.
#if 0 // ns7100
					eventRecord = (EVT_RECORD *)__ramFlashSummaryTbl[g_dqmXferStructPtr->ramTableIndex].linkPtr;
#else // ns8100
					static EVT_RECORD resultsEventRecord;
					eventRecord = &resultsEventRecord;
	
					//getEventFileInfo(__ramFlashSummaryTbl[g_dqmXferStructPtr->ramTableIndex].fileEventNum, &(resultsEventRecord.header), 
					//					&resultsEventRecord.summary, NO);
					getEventFileRecord(__ramFlashSummaryTbl[g_dqmXferStructPtr->ramTableIndex].fileEventNum, &resultsEventRecord);
#endif
					
					g_dqmXferStructPtr->dqmData[idex].dqmxFlag = 0xCC;
					g_dqmXferStructPtr->dqmData[idex].mode = eventRecord->summary.mode;
					g_dqmXferStructPtr->dqmData[idex].eventNumber = eventRecord->summary.eventNumber;
					byteCpy(g_dqmXferStructPtr->dqmData[idex].serialNumber,
						eventRecord->summary.version.serialNumber, SERIAL_NUMBER_STRING_SIZE);
					g_dqmXferStructPtr->dqmData[idex].eventTime = eventRecord->summary.captured.eventTime;
					g_dqmXferStructPtr->dqmData[idex].endFlag = 0xEE;
					idex++;
				}
				g_dqmXferStructPtr->ramTableIndex++;
			}

			g_transmitCRC = CalcCCITT32((uint8*)g_dqmXferStructPtr->dqmData,
				(idex * sizeof(DQMx_DATA_STRUCT)), g_transmitCRC);

			if (MODEM_SEND_FAILED == modem_puts((uint8*)g_dqmXferStructPtr->dqmData,
				(idex * sizeof(DQMx_DATA_STRUCT)), g_binaryXferFlag))
			{
				g_dqmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
				g_modemStatus.xferMutex = NO;
				g_transferCount = 0;
				return (NOP_CMD);
			}
		}
	}

	else if (FOOTER_XFER_STATE == g_dqmXferStructPtr->xferStateFlag)
	{
		modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
		modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
		g_dqmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
		xferState = NOP_CMD;
		g_modemStatus.xferMutex = NO;
		g_transferCount = 0;
	}

	return (xferState);
}

//==================================================
// Function: handleDSM
// Description:
// 		Download summary memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleDSM(CMD_BUFFER_STRUCT* inCmd)
{
	// Download summary memory...
	uint16 idex;
	uint16 numberOfRecs;					// Count the number of records to send.
	uint32 dataLength;						// Will hold the new data length of the message
	uint8  flagData = 0;

	// If the process is busy sending data, return;
	if (YES == g_modemStatus.xferMutex)
		return;

	if (YES == parseIncommingMsgHeader(inCmd, g_inCmdHeaderPtr))
	{
		return;
	}

	dataLength = 0;
	numberOfRecs = 0;

	// Determine the data size first. This size is needed for the header length.
	for (idex = 0; idex < TOTAL_RAM_SUMMARIES; idex++)
	{
		if (__ramFlashSummaryTbl[idex].fileEventNum != 0xFFFFFFFF)
		{
			dataLength += sizeof(EVENT_RECORD_DOWNLOAD_STRUCT);
			numberOfRecs++;
		}
	}

	// Now start building the outgoing header. Clear the outgoing header data.
	byteSet((uint8*)g_outCmdHeaderPtr, 0, sizeof(COMMAND_MESSAGE_HEADER));

	// Copy the existing header data into the outgoing buffer.
	byteCpy(g_outCmdHeaderPtr, g_inCmdHeaderPtr,  sizeof(COMMAND_MESSAGE_HEADER));

	// Start Building the outgoing header. Set the msg type to a one for a response message.
	sprintf((char*)g_outCmdHeaderPtr->type, "%02d", MSGTYPE_RESPONSE);

	buildIntDataField((char*)g_outCmdHeaderPtr->dataLength,
		(dataLength + MESSAGE_HEADER_LENGTH + MESSAGE_FOOTER_LENGTH + (DATA_FIELD_LEN)), FIELD_LEN_08);

	flagData = CRC_32BIT;
	flagData = (uint8)(flagData << 4);
	flagData = (uint8)((uint8)(flagData) | ((uint8)COMPRESS_NONE));
	sprintf((char*)g_outCmdHeaderPtr->compressCrcFlags,"%02x", flagData);

	// Create the message buffer from the outgoing header data.
	buildOutgoingHeaderBuffer(g_outCmdHeaderPtr, g_dsmXferStructPtr->msgHdr);

	// Fill in the number of records, fill in the data length
	byteSet(g_dsmXferStructPtr->numOfRecStr, 0, (DATA_FIELD_LEN+1));
	buildIntDataField((char*)g_dsmXferStructPtr->numOfRecStr, numberOfRecs, FIELD_LEN_06);

	//-----------------------------------------------------------
	// Set the intial table index to the first element of the table
	g_dsmXferStructPtr->tableIndex = 0;							// Index of current summary to xfer
	g_dsmXferStructPtr->tableIndexStart = 0;						// Start of summaries to xfer
	g_dsmXferStructPtr->tableIndexEnd = TOTAL_RAM_SUMMARIES;		// End of summaries to xfer
	g_dsmXferStructPtr->tableIndexInUse = TOTAL_RAM_SUMMARIES;	// Summary not to print incase of in use.

	g_dsmXferStructPtr->xferStateFlag = HEADER_XFER_STATE;	// This is the initail xfer state to start.
	g_modemStatus.xferState = DSMx_CMD;								// This is the xfer command state.
	g_modemStatus.xferMutex = YES;									// Mutex to prevent other commands.

	g_modemStatus.xferPrintState = g_helpRecord.auto_print;

	g_transferCount = 0;

	return;
}

//==================================================
// Function: sendDSMData
// Description:
// Input:
// Return: void
//--------------------------------------------------
uint8 sendDSMData(void)
{
	EVT_RECORD* eventRecord;
	uint8 xferState = DSMx_CMD;

	// If the beginning of sending data, send the crlf.
	if (HEADER_XFER_STATE == g_dsmXferStructPtr->xferStateFlag)
	{
		if (MODEM_SEND_FAILED == modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION))
		{
			g_dsmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;

			return (NOP_CMD);
		}

		// g_transmitCRC will be the seed value for the rest of the CRC calculations.
		g_transmitCRC = CalcCCITT32((uint8*)g_dsmXferStructPtr->msgHdr, MESSAGE_HEADER_LENGTH, SEED_32);

		if (MODEM_SEND_FAILED == modem_puts((uint8*)g_dsmXferStructPtr->msgHdr,
			MESSAGE_HEADER_LENGTH, g_binaryXferFlag))
		{
			g_dsmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;

			return (NOP_CMD);
		}

		g_transmitCRC = CalcCCITT32((uint8*)g_dsmXferStructPtr->numOfRecStr, FIELD_LEN_06, g_transmitCRC);

		if (MODEM_SEND_FAILED ==  modem_puts((uint8*)g_dsmXferStructPtr->numOfRecStr,
			FIELD_LEN_06, g_binaryXferFlag))
		{
			g_dsmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;

			return (NOP_CMD);
		}

		g_dsmXferStructPtr->xferStateFlag = SUMMARY_TABLE_SEARCH_STATE;
		g_transferCount = MESSAGE_HEADER_LENGTH + FIELD_LEN_06 + 2;
	}

	// xfer the event record structure.
	else if (SUMMARY_TABLE_SEARCH_STATE == g_dsmXferStructPtr->xferStateFlag)
	{
		// Loop for the first non-empty table summary.
		while ((g_dsmXferStructPtr->tableIndex < g_dsmXferStructPtr->tableIndexEnd) &&
				(__ramFlashSummaryTbl[g_dsmXferStructPtr->tableIndex].fileEventNum == 0xFFFFFFFF))
		{
			g_dsmXferStructPtr->tableIndex++;
		}

		// If not at the end of the table, xfer it over, else we are done and go to the footer state.
		if (g_dsmXferStructPtr->tableIndex < g_dsmXferStructPtr->tableIndexEnd)
		{
#if 0 // ns7100
			eventRecord = (EVT_RECORD *)__ramFlashSummaryTbl[g_dsmXferStructPtr->tableIndex].linkPtr;
#else // ns8100
			static EVT_RECORD resultsEventRecord;
			eventRecord = &resultsEventRecord;
	
			//getEventFileInfo(__ramFlashSummaryTbl[g_dsmXferStructPtr->tableIndex].fileEventNum, &(resultsEventRecord.header), 
			//					&resultsEventRecord.summary, NO);
			getEventFileRecord(__ramFlashSummaryTbl[g_dsmXferStructPtr->tableIndex].fileEventNum, &resultsEventRecord);
#endif
			
			g_dsmXferStructPtr->dloadEventRec.event = *eventRecord;
			g_dsmXferStructPtr->dloadEventRec.structureFlag = START_DLOAD_FLAG;
			g_dsmXferStructPtr->dloadEventRec.downloadDate = getCurrentTime();
			g_dsmXferStructPtr->dloadEventRec.endFlag = END_DLOAD_FLAG;

			// Setup the xfer structure ptrs.
			g_dsmXferStructPtr->startDloadPtr = (uint8*)&(g_dsmXferStructPtr->dloadEventRec);
			g_dsmXferStructPtr->dloadPtr = g_dsmXferStructPtr->startDloadPtr;
			g_dsmXferStructPtr->endDloadPtr = ((uint8*)g_dsmXferStructPtr->startDloadPtr +
				sizeof(EVENT_RECORD_DOWNLOAD_STRUCT));

			g_dsmXferStructPtr->tableIndex++;							// Increment for next time.
			g_dsmXferStructPtr->xferStateFlag = EVENTREC_XFER_STATE;	// state to xfer the record.
		}
		else
		{
			g_dsmXferStructPtr->xferStateFlag = FOOTER_XFER_STATE;
		}
	}

	// xfer the event record structure.
	else if (EVENTREC_XFER_STATE == g_dsmXferStructPtr->xferStateFlag)
	{
		g_dsmXferStructPtr->dloadPtr = sendDataNoFlashWrapCheck(g_dsmXferStructPtr->dloadPtr,
			g_dsmXferStructPtr->endDloadPtr);

		if (NULL == g_dsmXferStructPtr->dloadPtr)
		{
			g_dsmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;

			return (NOP_CMD);
		}

		if (g_dsmXferStructPtr->dloadPtr >= g_dsmXferStructPtr->endDloadPtr)
		{
			g_dsmXferStructPtr->xferStateFlag = SUMMARY_TABLE_SEARCH_STATE;
		}
	}

	else if (FOOTER_XFER_STATE == g_dsmXferStructPtr->xferStateFlag)
	{
		modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
		modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
		g_dsmXferStructPtr->xferStateFlag = NOP_XFER_STATE;
		xferState = NOP_CMD;
		g_modemStatus.xferMutex = NO;
		g_transferCount = 0;
	}

	return (xferState);
}

//==================================================
// Function: handleDEM
// Description:
// 		Download event memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleDEM(CMD_BUFFER_STRUCT* inCmd)
{
	uint16 idex;
	uint16 eventNumToSend;					// In case there is a specific record to print.
	uint32 dataLength;						// Will hold the new data length of the message
	uint16 i = 0, j = 0;
	uint32 msgCRC;
	uint32 inCRC;
	char msgTypeStr[8];
	uint8 rawData[5];
	uint8* rawDataPtr = &rawData[0];

	EVT_RECORD* eventRecord;

	debug("handleDEM:Entry\n");

	// If the process is busy sending data, return;
	if (YES == g_modemStatus.xferMutex)
	{
		return;
	}

	if (YES == parseIncommingMsgHeader(inCmd, g_inCmdHeaderPtr))
	{
		debug("handleDEM RTN Error.\n");
		return;
	}

	// Check if the CRC32 flag is set (2nd byte of Compress/CRC flags 2 byte field, stored as an ascii number)
	if((g_inCmdHeaderPtr->compressCrcFlags[1] - 0x30) == CRC_32BIT)
	{
		//Move the string data into the configuration structure. String is (2 * cfgSize)
		i = MESSAGE_HEADER_LENGTH;
		while((i < inCmd->size) && (i < (MESSAGE_HEADER_LENGTH + (sizeof(rawData) * 2))) && (i < CMD_BUFFER_SIZE))
		{
			*rawDataPtr++ = convertAscii2Binary(inCmd->msg[i], inCmd->msg[i + 1]);
			i += 2;
		}

		// Set i to the start of the Unit Model 8 byte field (which now contains the ascii equiv. of the CRC32 value)
		i = HDR_CMD_LEN + HDR_TYPE_LEN + HDR_DATALENGTH_LEN;

		// Get the CRC value from incoming command
		while((i < inCmd->size) && (i < CMD_BUFFER_SIZE) && (j < 4))
		{
			((uint8*)&inCRC)[j++] = convertAscii2Binary(inCmd->msg[i], inCmd->msg[i + 1]);

			// Set the byte fields to ASCII zero's
			inCmd->msg[i] = 0x30;
			inCmd->msg[i + 1] = 0x30;

			// Advance the pointer
			i += 2;
		}

		// Validate the CRC
		msgCRC = CalcCCITT32((uint8*)&(inCmd->msg[0]), MESSAGE_HEADER_LENGTH, SEED_32);
		msgCRC = CalcCCITT32((uint8*)&(rawData[0]), sizeof(rawData), msgCRC);

		// The CRC's don't match
		if(inCRC != msgCRC)
		{
			// Signal a bad CRC value
			sprintf((char*)msgTypeStr, "%02d", CFG_ERR_BAD_CRC);
			byteCpy(g_inCmdHeaderPtr->type, msgTypeStr, HDR_TYPE_LEN);

			// Send Starting CRLF
			modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

			// Calculate the CRC
			g_transmitCRC = CalcCCITT32((uint8*)&(g_inCmdHeaderPtr->cmd[0]), (uint32)(inCmd->size - 4), SEED_32);

			// Send Simple header
			modem_puts((uint8*)&(g_inCmdHeaderPtr->cmd[0]), (uint32)(inCmd->size - 4), NO_CONVERSION);

			// Send Ending Footer
			modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
			modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
			return;
		}

	}

#if 0 // Test code
	debugPrint(RAW, "Recieved DEM command: \n");
	for(i=0;i<inCmd->size;i++)
	{
		debugPrint(RAW, "(%d)%x ", i+1, inCmd->msg[i]);
	}
	debugPrint(RAW, "\n");

	debugPrint(RAW, "Command: <%s>, Len: %d\n", (char*)(g_inCmdHeaderPtr->cmd), HDR_CMD_LEN);
	debugPrint(RAW, "Message Type: 0x%x 0x%x, Len: %d\n", g_inCmdHeaderPtr->type[0], g_inCmdHeaderPtr->type[1], HDR_TYPE_LEN);
	debugPrint(RAW, "Data Length: <%s>, Len: %d\n", (char*)(g_inCmdHeaderPtr->dataLength), HDR_DATALENGTH_LEN);
	debugPrint(RAW, "Unit Model: <%s>, Len: %d\n", (char*)(g_inCmdHeaderPtr->unitModel), HDR_UNITMODEL_LEN);
	debugPrint(RAW, "Unit Serial #: <%s>, Len: %d\n", (char*)(g_inCmdHeaderPtr->unitSn), HDR_SERIALNUMBER_LEN);
	debugPrint(RAW, "Compress/CRC Flags: 0x%x 0x%x, Len: %d\n", g_inCmdHeaderPtr->compressCrcFlags[0], g_inCmdHeaderPtr->compressCrcFlags[1], HDR_COMPRESSCRC_LEN);
	debugPrint(RAW, "Software Version: 0x%x 0x%x, Len: %d\n", g_inCmdHeaderPtr->softwareVersion[0], g_inCmdHeaderPtr->softwareVersion[1], HDR_SOFTWAREVERSION_LEN);
	debugPrint(RAW, "Data Version: 0x%x 0x%x, Len: %d\n", g_inCmdHeaderPtr->dataVersion[0], g_inCmdHeaderPtr->dataVersion[1], HDR_DATAVERSION_LEN);
	debugPrint(RAW, "Spare: 0x%x 0x%x, Len: %d\n", g_inCmdHeaderPtr->spare[0], g_inCmdHeaderPtr->spare[1], HDR_SPARE_LEN);
#endif

	//-----------------------------------------------------------
	// We need to get the data length. This tells us if we have fields in the command
	// message to parse. The data length portion is length, minus the hdr and footer length.
	dataLength = dataLengthStrToUint32(g_inCmdHeaderPtr->dataLength);
 	dataLength = (uint32)(dataLength - MESSAGE_HEADER_LENGTH - MESSAGE_FOOTER_LENGTH);

	// Verify that this is a valid field and find the number and send it.
	if (dataLength == 6)
	{
		// Expecting a single field, so move to that location.
		eventNumToSend = getInt16Field(inCmd->msg + MESSAGE_HEADER_LENGTH);

		debug("eventNumToSend = %d \n",eventNumToSend);

		// Send each individual summary record.
		for (idex = 0; idex < TOTAL_RAM_SUMMARIES; idex++)
		{
			if (__ramFlashSummaryTbl[idex].fileEventNum != 0xFFFFFFFF)
			{
				// Check to see if this is the correct event number.
#if 0 // ns7100
				eventRecord = (EVT_RECORD *)__ramFlashSummaryTbl[idex].linkPtr;
#else // ns8100
				static EVT_RECORD resultsEventRecord;
				eventRecord = &resultsEventRecord;
	
				//getEventFileInfo(__ramFlashSummaryTbl[idex].fileEventNum, &(resultsEventRecord.header), 
				//					&resultsEventRecord.summary, YES);
				getEventFileRecord(__ramFlashSummaryTbl[idex].fileEventNum, &resultsEventRecord);
				cacheEventDataToRam(__ramFlashSummaryTbl[idex].fileEventNum, eventRecord->header.dataLength);
#endif

				if (eventRecord->summary.eventNumber == eventNumToSend)
				{
					prepareDEMDataToSend(eventRecord, g_inCmdHeaderPtr);

					if (g_demXferStructPtr->errorStatus == MODEM_SEND_SUCCESS)
					{
						// Update last downloaded event
						if (eventNumToSend > __autoDialoutTbl.lastDownloadedEvent)
							__autoDialoutTbl.lastDownloadedEvent = eventNumToSend;
					}

					break; // Send only one.
				}
			}
		}

		if (TOTAL_RAM_SUMMARIES == idex)
		{
			sendErrorMsg((uint8*)"DEMe", (uint8*)MSGTYPE_ERROR_NO_EVENT);
		}
	}

	return;
}

//==================================================
// Function: prepareDEMData
// Description:
// 		Download event memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
#include "InitDataBuffers.h"
void prepareDEMDataToSend(EVT_RECORD* eventRecord, COMMAND_MESSAGE_HEADER* g_inCmdHeaderPtr)
{
	uint32 dataLength;						// Will hold the new data length of the message
	uint32 eventRecCompressLength = 0;		// Will hold the length of the compressed event record data.
	uint32 eventDataCompressLength = 0;		// Will hold the length of the compressed event data.
	uint8  flagData = 0;


	// Clear out the xmit structures and intialize the flag and time fields.
	byteSet(&(g_demXferStructPtr->dloadEventRec), 0, sizeof(EVENT_RECORD_DOWNLOAD_STRUCT));
	g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
	g_demXferStructPtr->dloadEventRec.structureFlag = START_DLOAD_FLAG;
	g_demXferStructPtr->dloadEventRec.downloadDate = getCurrentTime();
	g_demXferStructPtr->dloadEventRec.event = *eventRecord;
	g_demXferStructPtr->dloadEventRec.endFlag = END_DLOAD_FLAG;
	g_demXferStructPtr->errorStatus = MODEM_SEND_SUCCESS;

	// Now start building the outgoing header. Get the intial values from
	// the incomming header. Clear the outgoing header data.
	byteSet(g_outCmdHeaderPtr, 0, sizeof(COMMAND_MESSAGE_HEADER));
	byteCpy(g_outCmdHeaderPtr, g_inCmdHeaderPtr,  sizeof(COMMAND_MESSAGE_HEADER));

	// Start Building the outgoing message header. Set the type to a one for a response message.
	sprintf((char*)g_outCmdHeaderPtr->type, "%02d", MSGTYPE_RESPONSE);

	//-----------------------------------------------------------
	// Get the length of the summary structure with the header and footer size.
	// Total number of callibration and event records.

	// Data length is total number of bytes of the uncompressed data.
	dataLength = (uint16)(MESSAGE_HEADER_LENGTH + MESSAGE_FOOTER_LENGTH +
		eventRecord->header.headerLength + eventRecord->header.summaryLength +
			eventRecord->header.dataLength);
	buildIntDataField((char*)g_outCmdHeaderPtr->dataLength, dataLength, FIELD_LEN_08);


	//-----------------------------------------------------------
	// Setup the xfer structure ptrs.
	g_demXferStructPtr->startDloadPtr = (uint8*)&(g_demXferStructPtr->dloadEventRec);
	g_demXferStructPtr->dloadPtr = g_demXferStructPtr->startDloadPtr;
	g_demXferStructPtr->endDloadPtr = ((uint8*)g_demXferStructPtr->startDloadPtr +
		sizeof(EVENT_RECORD_DOWNLOAD_STRUCT));


	dataLength = eventRecord->header.headerLength + eventRecord->header.summaryLength +
		eventRecord->header.dataLength;

	// Find and set the pointer to the begining of the event data.
#if 0 // ns7100
	g_demXferStructPtr->startDataPtr = (uint8*)eventRecord + sizeof(EVT_RECORD);
#else
extern uint16 g_eventDataBuffer[EVENT_BUFF_SIZE_IN_WORDS];
	g_demXferStructPtr->startDataPtr = (uint8*)(&g_eventDataBuffer[0]);
#endif

	// The data ptr is set to the start of data
	g_demXferStructPtr->dataPtr = g_demXferStructPtr->startDataPtr;

	// Is all the data contiguous? if it wraps then  we need to set the ptrs for the old method.
#if 0 // ns7100
	if( ( (uint32)(g_demXferStructPtr->startDataPtr) + (eventRecord->header.dataLength)) >= (uint32)FLASH_EVENT_END )
#else // ns8100
	if (0)
#endif
	{
		// Build the crc and copressed fields.
		flagData = CRC_32BIT;
		flagData = (uint8)(flagData << 4);
		flagData = (uint8)((uint8)(flagData) | ((uint8)COMPRESS_NONE));
		sprintf((char*)g_outCmdHeaderPtr->compressCrcFlags,"%02x", flagData);
		// Create the message buffer from the outgoing header data.
		buildOutgoingHeaderBuffer(g_outCmdHeaderPtr, g_demXferStructPtr->msgHdr);

		//----------------------------------------------------

		// data Length is the # of bytes to the end of flash
		dataLength = (uint32)((uint32)FLASH_EVENT_END - (uint32)g_demXferStructPtr->startDataPtr);

		// Find the end of flash location.
		g_demXferStructPtr->endDataPtr = (uint8*)((uint32)FLASH_EVENT_START +
			(uint32)(eventRecord->header.dataLength - dataLength));

		g_demXferStructPtr->xferStateFlag = HEADER_XFER_STATE;

		// Prepare the flags for the xfer command.
		g_modemStatus.xferMutex = YES;
		g_modemStatus.xferState = DEMx_CMD;
		g_modemStatus.xferPrintState = g_helpRecord.auto_print;
	}

	// It DOES NOT wrap in flash. Use .
	else
	{
		// Build the crc and copressed fields.
		flagData = CRC_32BIT;
		flagData = (uint8)(flagData << 4);
		flagData = (uint8)((uint8)(flagData) | ((uint8)COMPRESS_MINILZO));
		sprintf((char*)g_outCmdHeaderPtr->compressCrcFlags,"%02x", flagData);
		// Create the message buffer from the outgoing header data.
		buildOutgoingHeaderBuffer(g_outCmdHeaderPtr, g_demXferStructPtr->msgHdr);

		//----------------------------------------------------
		// Send CRLF start of msg.
		dataLength = 2;
		if (modem_puts((uint8*)&g_CRLF, dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}
		g_transferCount = 0;						// Do not count the first CRLF

		// Send the communications header
		dataLength = MESSAGE_HEADER_LENGTH;
		g_transmitCRC = CalcCCITT32((uint8*)g_demXferStructPtr->msgHdr, dataLength, SEED_32);
		if (modem_puts((uint8*)g_demXferStructPtr->msgHdr, dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}
		g_transferCount += dataLength;


#if 1
		// Compress and download the event record.
		g_demXferStructPtr->xmitSize = 0;
		eventRecCompressLength =
			lzo1x_1_compress((void*)g_demXferStructPtr->startDloadPtr, sizeof(EVENT_RECORD_DOWNLOAD_STRUCT));

		if (g_demXferStructPtr->xmitSize > 0)
		{
			g_transmitCRC = CalcCCITT32((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, g_transmitCRC);
			if (modem_puts((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, NO_CONVERSION) == MODEM_SEND_FAILED)
			{
				g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
			}
			g_transferCount += g_demXferStructPtr->xmitSize;
		}

		// Compress and download the event data.
		g_demXferStructPtr->xmitSize = 0;
		eventDataCompressLength = lzo1x_1_compress(
			(void*)(g_demXferStructPtr->startDataPtr), eventRecord->header.dataLength);

		if( g_demXferStructPtr->xmitSize > 0 )
		{
			g_transmitCRC = CalcCCITT32((uint8 *)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, g_transmitCRC);
			if(modem_puts((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, NO_CONVERSION) == MODEM_SEND_FAILED)
			{
				g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
			}
			g_transferCount += g_demXferStructPtr->xmitSize;
		}
#else
		// Compress and download the event record.
		dataLength = sizeof(uint32) + 12;
		g_transmitCRC = CalcCCITT32((uint8*)&(g_demXferStructPtr->dloadEventRec), dataLength, g_transmitCRC);
		if(modem_puts((uint8*)&(g_demXferStructPtr->dloadEventRec), dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}
		g_transferCount += dataLength;

		dataLength = eventRecord->header.headerLength + eventRecord->header.summaryLength +
			eventRecord->header.dataLength ;

		// Compress and download the event data.
		g_demXferStructPtr->xmitSize = 0;
		eventDataCompressLength = lzo1x_1_compress( (void*)eventRecord, dataLength );

		if( g_demXferStructPtr->xmitSize > 0 )
		{
			g_transmitCRC = CalcCCITT32((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, g_transmitCRC);
			if (modem_puts((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, NO_CONVERSION) == MODEM_SEND_FAILED)
			{
				g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
			}
			g_transferCount += g_demXferStructPtr->xmitSize;
		}
#endif

		dataLength = sizeof(uint32);
		// send the compressed size of the event record; sizeof(uint32) = 4.
		g_transmitCRC = CalcCCITT32((uint8*)&(eventRecCompressLength), dataLength, g_transmitCRC);
		if (modem_puts((uint8*)&(eventRecCompressLength), dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}
		g_transferCount += dataLength;

		// send the compressed size of the event data; sizeof(uint32) = 4.
		g_transmitCRC = CalcCCITT32((uint8*)&(eventDataCompressLength), dataLength, g_transmitCRC);
		if (modem_puts((uint8*)&(eventDataCompressLength), dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}
		g_transferCount += dataLength;

		// Total xmitted DataLength + size of the count the crc and the crlf.
		//g_transferCount += (dataLength + dataLength + dataLength + 2);	// Not counting last crlf

		g_transferCount += (dataLength + dataLength);
		g_transmitCRC = CalcCCITT32((uint8*)&(g_transferCount), dataLength, g_transmitCRC);
		if (modem_puts((uint8*)&(g_transferCount), dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}

		// CRC xmit
		if (modem_puts((uint8*)&g_transmitCRC, dataLength, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}

		// crlf xmit
		if (modem_puts((uint8*)&g_CRLF, sizeof(uint16), NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}

		debug("CRC=%d g_transferCount=%d \n", g_transmitCRC, g_transferCount);
	}

	return;
}


//==================================================
// Function: sendDEMData
// Description:
// Input:
// Return: void
//--------------------------------------------------
uint8 sendDEMData(void)
{
	uint8 xferState = DEMx_CMD;

	// If the beginning of sending data, send the crlf.
	if (HEADER_XFER_STATE == g_demXferStructPtr->xferStateFlag)
	{
		if (MODEM_SEND_FAILED == modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION))
		{
			g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;
			return (NOP_CMD);
		}

		// g_transmitCRC will be the seed value for the rest of the CRC calculations.
		g_transmitCRC = CalcCCITT32((uint8*)g_demXferStructPtr->msgHdr, MESSAGE_HEADER_LENGTH, SEED_32);

		if (MODEM_SEND_FAILED == modem_puts((uint8*)g_demXferStructPtr->msgHdr,
			MESSAGE_HEADER_LENGTH, g_binaryXferFlag))
		{
			g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;
			return (NOP_CMD);
		}

		g_demXferStructPtr->xferStateFlag = EVENTREC_XFER_STATE;
		g_transferCount = MESSAGE_HEADER_LENGTH + 2;
	}

	// xfer the event record structure.
	else if (EVENTREC_XFER_STATE == g_demXferStructPtr->xferStateFlag)
	{
		g_demXferStructPtr->dloadPtr = sendDataNoFlashWrapCheck(
			g_demXferStructPtr->dloadPtr, g_demXferStructPtr->endDloadPtr);

		if (NULL == g_demXferStructPtr->dloadPtr)
		{
			g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
			g_modemStatus.xferMutex = NO;
			g_transferCount = 0;
			return (NOP_CMD);
		}

		if (g_demXferStructPtr->dloadPtr >= g_demXferStructPtr->endDloadPtr)
		{
			g_demXferStructPtr->xferStateFlag = DATA_XFER_STATE;
		}
	}

	// xfer the event data.
	else if (DATA_XFER_STATE == g_demXferStructPtr->xferStateFlag)
	{
		// Does the end ptr wrap in flash? if not then continue;
		if (g_demXferStructPtr->dataPtr < g_demXferStructPtr->endDataPtr)
		{
			g_demXferStructPtr->dataPtr = sendDataNoFlashWrapCheck(
				g_demXferStructPtr->dataPtr, g_demXferStructPtr->endDataPtr);

			if (NULL == g_demXferStructPtr->dataPtr)
			{
				g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
				g_modemStatus.xferMutex = NO;
				g_transferCount = 0;
				return (NOP_CMD);
			}

			if (g_demXferStructPtr->dataPtr >= g_demXferStructPtr->endDataPtr)
			{
				g_demXferStructPtr->xferStateFlag = FOOTER_XFER_STATE;
			}
		}
		else	// The ptr does wrap in flash so the limit is the end of flash.
		{
			g_demXferStructPtr->dataPtr = sendDataFlashWrapCheck(
				g_demXferStructPtr->dataPtr);

			if (NULL == g_demXferStructPtr->dataPtr)
			{
				g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
				g_modemStatus.xferMutex = NO;
				g_transferCount = 0;
				return (NOP_CMD);
			}
		}
	}

	else if (FOOTER_XFER_STATE == g_demXferStructPtr->xferStateFlag)
	{
		debug("CRC=%d g_transferCount=%d \n", g_transmitCRC, g_transferCount+2);
		modem_puts((uint8*)&g_transmitCRC, 4, NO_CONVERSION);
		modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
		g_demXferStructPtr->xferStateFlag = NOP_XFER_STATE;
		xferState = NOP_CMD;
		g_modemStatus.xferMutex = NO;
		g_transferCount = 0;
	}

	return (xferState);
}

//==================================================
// Function: sendDataFlashWrapCheck
// Description:
// Input:
// Return: void
//--------------------------------------------------
uint8* sendDataFlashWrapCheck(uint8* xferPtr)
{
	uint32 xmitSize = XMIT_SIZE_MONITORING;

	if ((uint8*)(xferPtr + xmitSize) < (uint8*)FLASH_EVENT_END)
	{
		g_transmitCRC = CalcCCITT32((uint8*)xferPtr, xmitSize, g_transmitCRC);

		if (MODEM_SEND_FAILED == modem_puts(
			(uint8*)xferPtr, xmitSize, g_binaryXferFlag))
		{
			return (NULL);
		}

		xferPtr += xmitSize;
	}
	else
	{
		xmitSize = (uint32)((uint8*)FLASH_EVENT_END - (uint8*)xferPtr);

		g_transmitCRC = CalcCCITT32((uint8*)xferPtr, xmitSize, g_transmitCRC);

		if (MODEM_SEND_FAILED == modem_puts(
			(uint8*)xferPtr, xmitSize, g_binaryXferFlag))
		{
			return (NULL);
		}

		xferPtr = (uint8*)FLASH_EVENT_START;
	}

	g_transferCount += xmitSize;

	return (xferPtr);
}

//==================================================
// Function: sendDataNoFlashWrapCheck
// Description:
// Input:
// Return: void
//--------------------------------------------------
uint8* sendDataNoFlashWrapCheck(uint8* xferPtr, uint8* endPtr)
{
	uint32 xmitSize = XMIT_SIZE_MONITORING;

	if ((xferPtr + xmitSize) < endPtr)
	{
		g_transmitCRC = CalcCCITT32((uint8*)xferPtr, xmitSize, g_transmitCRC);

		if (MODEM_SEND_FAILED == modem_puts(
			(uint8*)xferPtr, xmitSize, g_binaryXferFlag))
		{
			return (NULL);
		}
	}
	else
	{
		xmitSize = (uint8)(endPtr - xferPtr);

		g_transmitCRC = CalcCCITT32((uint8*)xferPtr, xmitSize, g_transmitCRC);

		if (MODEM_SEND_FAILED == modem_puts(
			(uint8*)xferPtr, xmitSize, g_binaryXferFlag))
		{
			return (NULL);
		}
	}

	g_transferCount += xmitSize;
	xferPtr += xmitSize;

	return (xferPtr);
}



//==================================================
// Function: handleGMN
// Description: Start Monitoring waveform/bargraph/combo.
//
// Input: SUMMARY_DATA* ramTblElement
// Return: void
//--------------------------------------------------
void handleGMN(CMD_BUFFER_STRUCT* inCmd)
{
	INPUT_MSG_STRUCT mn_msg;
	uint8 gmnHdr[MESSAGE_HEADER_SIMPLE_LENGTH];

	uint8 nibble;
	uint8 tempBuff[4];
	uint16 readDex;
	uint16 buffDex;
	uint32 returnCode = CFG_ERR_NONE;
	uint32 msgCRC = 0;

	if (SAMPLING_STATE == g_sampleProcessing)
	{
		// Error in message length
		returnCode = CFG_ERR_MONITORING_STATE;
	}
	else if (inCmd->size < 18)
	{
		// Error in message length
		returnCode = CFG_ERR_MSG_LENGTH;
	}
	else
	{
		// Not larger then 1 but give some exta
		byteSet(tempBuff, 0, 4);

		readDex = 16;
		while ((readDex < inCmd->size) && (readDex < 18))
		{
			if ((inCmd->msg[readDex] >= 0x30) && (inCmd->msg[readDex] <= 0x39))
				nibble = (uint8)(inCmd->msg[readDex] - 0x30);
			else if ((inCmd->msg[readDex] >= 0x41) && (inCmd->msg[readDex] <= 0x46))
				nibble = (uint8)(inCmd->msg[readDex] - 0x37);

			buffDex = (uint16)((readDex-16)/2);
			tempBuff[buffDex] = (uint8)((uint8)(tempBuff[buffDex] << 4) + (uint8)nibble);

			readDex++;
		}

		switch (tempBuff[0])
		{
			case WAVEFORM_MODE:
			case BARGRAPH_MODE:
			case COMBO_MODE:
				// Good data
				g_triggerRecord.op_mode = tempBuff[0];
				g_activeMenu = MONITOR_MENU;
				ACTIVATE_MENU_WITH_DATA_MSG((uint32)g_triggerRecord.op_mode);
				(*menufunc_ptrs[g_activeMenu]) (mn_msg);
				returnCode = CFG_ERR_NONE;
				break;

			default:
				// Invalid trigger mode.
				returnCode = CFG_ERR_TRIGGER_MODE;
				break;
		}
	}

	sprintf((char*)tempBuff, "%02lu", returnCode);
	buildOutgoingSimpleHeaderBuffer((uint8*)gmnHdr, (uint8*)"GMNx",
		tempBuff, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
	// Send Simple header
	modem_puts((uint8*)gmnHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);
	// Send Ending Footer
	modem_puts((uint8*)&msgCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

//==================================================
// Function: handleHLT
// Description: Halt Monitoring waveform/bargraph/combo.
//
// Input: SUMMARY_DATA* ramTblElement
// Return: void
//--------------------------------------------------
void handleHLT(CMD_BUFFER_STRUCT* inCmd)
{
	INPUT_MSG_STRUCT mn_msg;
	uint8 hltHdr[MESSAGE_HEADER_SIMPLE_LENGTH];
	uint8 msgTypeStr[HDR_TYPE_LEN+1];
	uint8 resultCode = MSGTYPE_RESPONSE;
	uint32 msgCRC = 0;

	UNUSED(inCmd);

	if (SAMPLING_STATE == g_sampleProcessing)
	{
		// Stop 430 data transfers for the current mode and let the event processing handle the rest
		stopMonitoring(g_triggerRecord.op_mode, EVENT_PROCESSING);

		// Jump to the main menu
		g_activeMenu = MAIN_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
		resultCode = MSGTYPE_RESPONSE;
	}

	sprintf((char*)msgTypeStr, "%02d", resultCode);
	buildOutgoingSimpleHeaderBuffer((uint8*)hltHdr, (uint8*)"HLTx",
		(uint8*)msgTypeStr, MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
	// Send Simple header
	modem_puts((uint8*)hltHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);
	// Send Ending Footer
	modem_puts((uint8*)&msgCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);

	// Stop the processing.

}

//==================================================
// Function: debugSummaryData
// Description:
//
// Input: SUMMARY_DATA* ramTblElement
// Return: void
//--------------------------------------------------
void debugSummaryData(SUMMARY_DATA* ramTblElement)
{
	UNUSED(ramTblElement);
}

