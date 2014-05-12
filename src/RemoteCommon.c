///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: RemoteCommon.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/RemoteCommon.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Common.h"
#include "Uart.h"
#include "Rec.h"
#include "Old_Board.h"
#include "RemoteHandler.h"
#include "RemoteCommon.h"
#include "Crc.h"
#include "EventProcessing.h"
#include "SysEvents.h"
#include "SoftTimer.h"

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern uint32 __autoDialoutTblKey;
extern AUTODIALOUT_STRUCT __autoDialoutTbl;
extern uint8 	g_CRLF;
extern uint32 	g_XferCount;
extern uint32 	g_xmitCRC;
extern DEMx_XFER_STRUCT* gp_DemXferStruct;
extern MODEM_SETUP_STRUCT modem_setup_rec;
extern SYS_EVENT_STRUCT SysEvents_flags;
extern uint32 g_rtcSoftTimerTickCount;
extern MODEM_STATUS_STRUCT g_ModemStatus;
extern uint8 g_BinaryXferFlag;

//==================================================
// Globals
//==================================================
uint16 g_autoRetries = 0;
uint8 g_autoDialoutState = AUTO_DIAL_IDLE;
uint8 g_modemDataTransfered = NO;

//==================================================
// Functions
//==================================================

//==================================================
//	Procedure: parseIncommingMsgHeader()
//	Description:
//	Input: CMD_BUFFER_STRUCT* inCmd
//	Output: none
//--------------------------------------------------
uint8 parseIncommingMsgHeader(CMD_BUFFER_STRUCT* inCmd, COMMAND_MESSAGE_HEADER* incommingHdr)
{
	uint8 errCode = NO;
	char* msgPtr = (char*)inCmd->msg;			// A tempPtr into the message buffer.

	// clear the incomming header data.
	byteSet((uint8*)incommingHdr, 0, sizeof(COMMAND_MESSAGE_HEADER));

	if (strlen(msgPtr) >= HDR_CMD_LEN)
	{
		// Parse the string into a header data struct.
		byteCpy(incommingHdr->cmd, msgPtr, HDR_CMD_LEN);
		msgPtr += HDR_CMD_LEN;
	}

	if (strlen(msgPtr) >= HDR_TYPE_LEN)
	{
		byteCpy(incommingHdr->type, msgPtr, HDR_TYPE_LEN);
		msgPtr += HDR_TYPE_LEN;
	}

	if (strlen((char*)inCmd->msg) >= MESSAGE_HEADER_LENGTH)
	{
		byteCpy(incommingHdr->dataLength, msgPtr, HDR_DATALENGTH_LEN);
		msgPtr += HDR_DATALENGTH_LEN;

		byteCpy(incommingHdr->unitModel, msgPtr, HDR_UNITMODEL_LEN);
		msgPtr += HDR_UNITMODEL_LEN;

		byteCpy(incommingHdr->unitSn, msgPtr, HDR_SERIALNUMBER_LEN);
		msgPtr += HDR_SERIALNUMBER_LEN;

		byteCpy(incommingHdr->compressCrcFlags, msgPtr, HDR_COMPRESSCRC_LEN);
		msgPtr += HDR_COMPRESSCRC_LEN;

		byteCpy(incommingHdr->softwareVersion, msgPtr, HDR_SOFTWAREVERSION_LEN);
		msgPtr += HDR_SOFTWAREVERSION_LEN;

		byteCpy(incommingHdr->dataVersion, msgPtr, HDR_DATAVERSION_LEN);
		msgPtr += HDR_DATAVERSION_LEN;

		byteCpy(incommingHdr->spare, msgPtr, HDR_SPARE_LEN);
		msgPtr += HDR_SPARE_LEN;
	}
	else
	{
		errCode = YES;
	}

	return (errCode);
}

//==================================================
//	Procedure: parseIncommingMsgHeader()
//	Description:
//	Input: CMD_BUFFER_STRUCT* inCmd
//	Output: none
//--------------------------------------------------
uint8 parseIncommingMsgCmd(CMD_BUFFER_STRUCT* inCmd, COMMAND_MESSAGE_HEADER* incommingHdr)
{
	uint8 errCode = NO;
	char* msgPtr = (char*)inCmd->msg;			// A tempPtr into the message buffer.

	// clear the incomming header data.
	byteSet((uint8*)incommingHdr, 0, sizeof(COMMAND_MESSAGE_HEADER));

	if (strlen(msgPtr) >= HDR_CMD_LEN)
	{
		// Parse the string into a header data struct.
		byteCpy(incommingHdr->cmd, msgPtr, HDR_CMD_LEN);
		msgPtr += HDR_CMD_LEN;
	}
	else
	{
		errCode = YES;
	}

	return (errCode);
}


//==================================================
//	Procedure: buildMsgHeader()
//	Description:
//	Input: CMD_BUFFER_STRUCT* inCmd
//	Output: none
//--------------------------------------------------
void buildOutgoingHeaderBuffer(COMMAND_MESSAGE_HEADER* msgHdrData, uint8* msgHdrBuf)
{
	uint8* bufPtr = msgHdrBuf;

	byteSet(bufPtr, 0, MESSAGE_HEADER_LENGTH+1);

	// Parse the string into a header data struct.
	byteCpy(bufPtr, msgHdrData->cmd, HDR_CMD_LEN);
	bufPtr += HDR_CMD_LEN;

	byteCpy(bufPtr, msgHdrData->type, HDR_TYPE_LEN);
	bufPtr += HDR_TYPE_LEN;

	byteCpy(bufPtr, msgHdrData->dataLength, HDR_DATALENGTH_LEN);
	bufPtr += HDR_DATALENGTH_LEN;

	byteCpy(bufPtr, msgHdrData->unitModel, HDR_UNITMODEL_LEN);
	bufPtr += HDR_UNITMODEL_LEN;

	byteCpy(bufPtr, msgHdrData->unitSn, HDR_SERIALNUMBER_LEN);
	bufPtr += HDR_SERIALNUMBER_LEN;

	byteCpy(bufPtr, msgHdrData->compressCrcFlags, HDR_COMPRESSCRC_LEN);
	bufPtr += HDR_COMPRESSCRC_LEN;

	byteCpy(bufPtr, msgHdrData->softwareVersion, HDR_SOFTWAREVERSION_LEN);
	bufPtr += HDR_SOFTWAREVERSION_LEN;

	byteCpy(bufPtr, msgHdrData->dataVersion, HDR_DATAVERSION_LEN);
	bufPtr += HDR_DATAVERSION_LEN;

	byteCpy(bufPtr, msgHdrData->spare, HDR_SPARE_LEN);
	bufPtr += HDR_SPARE_LEN;

}


//==================================================
//	Procedure: buildOutgoingSimpleHeaderBuffer()
//	Description:
//	Input:
//	Output: none
//--------------------------------------------------
void buildOutgoingSimpleHeaderBuffer(uint8* msgHdrBuf,
	uint8* msgCmd, uint8* msgType, uint32 dataLength,
	uint8 verFlag, uint8 crcFlag)
{
	uint8* bufPtr = msgHdrBuf;
	//uint8 verData = 0;

	UNUSED(verFlag);
	UNUSED(crcFlag);

	byteSet(bufPtr, 0, MESSAGE_HEADER_SIMPLE_LENGTH);

	// Parse the string into a header data struct.
	byteCpy(bufPtr, msgCmd, HDR_CMD_LEN);
	bufPtr += HDR_CMD_LEN;

	byteCpy(bufPtr, msgType, HDR_TYPE_LEN);
	bufPtr += HDR_TYPE_LEN;

	buildIntDataField((char*)bufPtr, dataLength, FIELD_LEN_08);
	bufPtr += HDR_DATALENGTH_LEN;

	// Put in the version number and if the data message has a crcFlag.
	//*bufPtr++ = verFlag;
	//*bufPtr = crcFlag;
	*bufPtr++ = 0x46;
	*bufPtr = 0x46;
}


//==================================================
//	Procedure: sendErrorMsg()
//	Description:
//	Input:
//	Output: none
//--------------------------------------------------
void sendErrorMsg(uint8* msgCmd, uint8* msgType)
{
	uint32 msgCRC = 0;
	uint8 errHdr[MESSAGE_HEADER_SIMPLE_LENGTH];

	buildOutgoingSimpleHeaderBuffer(errHdr, msgCmd, msgType,
		MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);

	// Send Starting CRLF
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
	// Send Simple header
	modem_puts((uint8*)errHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);
	// Send Ending Footer
	modem_puts((uint8*)&msgCRC, 4, NO_CONVERSION);
	modem_puts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}


//==================================================
// Function: getInt16Field
// Description:
// 		Download event memory.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
uint16 getInt16Field(uint8* dataPtr)
{
	uint16 int16Data = 0;
	uint8 dataStr[DATA_FIELD_LEN+1];

	byteSet(dataStr, 0, sizeof(dataStr));
	byteCpy(dataStr, dataPtr, DATA_FIELD_LEN);
	int16Data = (uint16)dataLengthStrToUint32(dataStr);


	return (int16Data);
}

//==================================================
//	Procedure: buildOutgoingDataLength()
//	Description:
//	Input: COMMAND_MESSAGE_HEADER* hdrData, uint32 dataLength
//	Output: none
//--------------------------------------------------
void buildIntDataField(char* strBuffer, uint32 data, uint8 fieldLen)
{
	if (fieldLen == FIELD_LEN_08)
	{
		sprintf((char*)strBuffer,"%08lu", data);
	}
	else if (fieldLen == FIELD_LEN_06)
	{
		sprintf((char*)strBuffer,"%06lu", data);
	}
	else if (fieldLen == FIELD_LEN_04)
	{
		sprintf((char*)strBuffer,"%04lu", data);
	}
	else if (fieldLen == FIELD_LEN_02)
	{
		sprintf((char*)strBuffer,"%02lu", data);
	}
}

//==================================================
//	Procedure: buildOutgoingDataLength()
//	Description:
//	Input: COMMAND_MESSAGE_HEADER* hdrData, uint32 dataLength
//	Output: none
//--------------------------------------------------
uint32 dataLengthStrToUint32(uint8* dataLengthStr)
{
	uint32 dataLength = 0;
	uint8 strDex = 0;
	uint8 dataLenBuf[HDR_DATALENGTH_LEN+1];

	uint8* dataStr = dataLengthStr;
	uint8* dataBuf = dataLenBuf;


	byteSet(&dataLenBuf[0], 0, sizeof(dataLenBuf));

	// Look and clear all leading zeros and non digits.
	while (((*dataStr <= '0') 	||
			(*dataStr >  '9')) 	&&
		 	(*dataStr != 0x00) 	&&
		 	(strDex < HDR_DATALENGTH_LEN))
	{
		strDex++;
		dataStr++;
	}

	// Copy over all valid digits to the buffer.
	while ((*dataStr != 0x00) && (strDex < HDR_DATALENGTH_LEN))
	{
		// Is it a valid digit, if so copy it over.
		if ((*dataStr >= '0') && (*dataStr <=  '9'))
		{
			*dataBuf = *dataStr;
			dataBuf++;
		}
		strDex++;
		dataStr++;
	}

	dataLength = (uint32)atoi((char*)dataLenBuf);

	return (dataLength);
}

//==================================================
//	Function:	writeCompressedData
//	Purpose:	Send out the compressed data byte
//--------------------------------------------------
void writeCompressedData(uint8 compressedData)
{
	gp_DemXferStruct->xmitBuffer[gp_DemXferStruct->xmitSize] = compressedData;
	gp_DemXferStruct->xmitSize++;

	if (gp_DemXferStruct->xmitSize >= XMIT_SIZE_MONITORING)
	{
		g_xmitCRC = CalcCCITT32((uint8*)gp_DemXferStruct->xmitBuffer, gp_DemXferStruct->xmitSize, g_xmitCRC);

		if (modem_puts((uint8*)gp_DemXferStruct->xmitBuffer, gp_DemXferStruct->xmitSize, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			gp_DemXferStruct->errorStatus = MODEM_SEND_FAILED;
		}

		g_XferCount += gp_DemXferStruct->xmitSize;
		gp_DemXferStruct->xmitSize = 0;
	}
}

//==================================================
//	Function:	initAutoDialout
//	Purpose:
//--------------------------------------------------
void initAutoDialout(void)
{
	// Check if the table key is not valid
	if (__autoDialoutTblKey != VALID_AUTODIALOUT_TABLE_KEY)
	{
		// Clear AutoDialout Log table
		byteSet(&__autoDialoutTbl, 0x0, sizeof(AUTODIALOUT_STRUCT));

		// No need to set the following since the memset takes care of this
		//__autoDialoutTbl.lastDownloadedEvent = 0;
		//__autoDialoutTbl.lastConnectTime.valid = FALSE;

		__autoDialoutTblKey = VALID_AUTODIALOUT_TABLE_KEY;
	}

	// Update the last stored event
	__autoDialoutTbl.lastStoredEvent = getLastStoredEventNumber();
}

//==================================================
//	Function:	checkAutoDialoutStatus
//	Purpose:
//--------------------------------------------------
void checkAutoDialoutStatus(void)
{
#if 0 // fix_ns8100
	if ((g_autoDialoutState == AUTO_DIAL_IDLE) && (READ_DCD == NO_CONNECTION) &&
		(modem_setup_rec.modemStatus == YES) && strlen((char*)&(modem_setup_rec.dial[0])) != 0)
	{
		raiseSystemEventFlag(AUTO_DIALOUT_EVENT);
	}
#endif
}

//==================================================
//	Function:	startAutoDialoutProcess
//	Purpose:
//--------------------------------------------------
void startAutoDialoutProcess(void)
{
#if 0 // fix_ns8100
	if (READ_DCD == NO_CONNECTION)
	{
		g_autoRetries = modem_setup_rec.retries;
		g_autoDialoutState = AUTO_DIAL_INIT;
	}
#endif
}

//==================================================
//	Function:	autoDialoutStateMachine
//	Purpose:
//--------------------------------------------------
void autoDialoutStateMachine(void)
{
	static uint32 timer = 0;
#if 0 // fix_ns8100
	CMD_BUFFER_STRUCT msg;
#endif

	switch (g_autoDialoutState)
	{
		//----------------------------------------------------------------
		// Send Dial string
		//----------------------------------------------------------------
		case AUTO_DIAL_INIT:
			// Issue dial command and dial string
			if((modem_setup_rec.dial[0] >= '0') && (modem_setup_rec.dial[0] <= '9'))
			{
				uart_puts((char *)"ATDT", CRAFT_COM_PORT);
			}
			uart_puts((char *)(modem_setup_rec.dial), CRAFT_COM_PORT);
			uart_puts((char *)&g_CRLF, CRAFT_COM_PORT);

			// Update timer to current tick count
			timer = g_rtcSoftTimerTickCount;

			// Advance to Connecting state
			g_autoDialoutState = AUTO_DIAL_CONNECTING;
		break;

		//----------------------------------------------------------------
		// Look for DCD
		//----------------------------------------------------------------
		case AUTO_DIAL_CONNECTING:
#if 0 // fix_ns8100
			// Check if a remote connection has been established
			if (READ_DCD == CONNECTION_ESTABLISHED)
			{
				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Connected state
				g_autoDialoutState = AUTO_DIAL_CONNECTED;
			}
			// Check if the timer has surpassed 1 minute
			else if ((g_rtcSoftTimerTickCount - timer) > (1 * TICKS_PER_MIN))
			{
				// Couldn't establish a connection, give up and retry later

				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Retry state
				g_autoDialoutState = AUTO_DIAL_RETRY;
			}
#endif
		break;

		//----------------------------------------------------------------
		// Send out GAD command
		//----------------------------------------------------------------
		case AUTO_DIAL_CONNECTED:
#if 0 // fix_ns8100
			// Check if the current connection has been established for 5 seconds
			if ((g_rtcSoftTimerTickCount - timer) > (5 * TICKS_PER_SEC))
			{
				// Make sure transfer flag is set to ascii
				g_BinaryXferFlag = NO_CONVERSION;

				// Send out GAD command (includes serial number and auto dialout parameters)
				handleGAD(&msg);

				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Response state
				g_autoDialoutState = AUTO_DIAL_RESPONSE;
			}
			// Check if we lose the remote connection
			else if (READ_DCD == NO_CONNECTION)
			{
				// Advance to Retry state
				g_autoDialoutState = AUTO_DIAL_RETRY;
			}
#endif
		break;

		//----------------------------------------------------------------
		// Look for system to be unlocked
		//----------------------------------------------------------------
		case AUTO_DIAL_RESPONSE:
#if 0 // fix_ns8100
			// Check if the system has been unlocked (thus successful receipt of an unlock command)
			if (g_ModemStatus.systemIsLockedFlag == NO)
			{
				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Active state
				g_autoDialoutState = AUTO_DIAL_ACTIVE;
			}
			// Check if more than 30 seconds have elapsed without a successful unlock command
			else if ((g_rtcSoftTimerTickCount - timer) > (30 * TICKS_PER_SEC))
			{
				// Send out GAD command again
				handleGAD(&msg);

				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Wait state
				g_autoDialoutState = AUTO_DIAL_WAIT;
			}
			// Check if we lose the remote connection
			else if (READ_DCD == NO_CONNECTION)
			{
				// Advance to Retry state
				g_autoDialoutState = AUTO_DIAL_RETRY;
			}
#endif
		break;

		//----------------------------------------------------------------
		// Wait for system to be unlocked (2nd attempt)
		//----------------------------------------------------------------
		case AUTO_DIAL_WAIT:
#if 0 // fix_ns8100
			// Check if the system has been unlocked (thus successful receipt of an unlock command)
			if (g_ModemStatus.systemIsLockedFlag == NO)
			{
				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Active state
				g_autoDialoutState = AUTO_DIAL_ACTIVE;
			}
			// Check if more than 30 seconds have elapsed without a successful unlock command (again, 2nd attempt)
			else if ((g_rtcSoftTimerTickCount - timer) > (30 * TICKS_PER_SEC))
			{
				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Retry state
				g_autoDialoutState = AUTO_DIAL_RETRY;
			}
			// Check if we lose the remote connection
			else if (READ_DCD == NO_CONNECTION)
			{
				// Advance to Retry state
				g_autoDialoutState = AUTO_DIAL_RETRY;
			}
#endif
		break;

		//----------------------------------------------------------------
		// Start retry handling
		//----------------------------------------------------------------
		case AUTO_DIAL_RETRY:
			// Check if retries have been exhausted
			if (g_autoRetries == 0)
			{
				// Advance to Finish state
				g_autoDialoutState = AUTO_DIAL_FINISH;
			}
			else // Keep trying
			{
				// Decrement retry count
				g_autoRetries--;

				// Unable to successfully connect to remote end, start retry with modem reset
				modemResetProcess();

				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Advance to Sleep state
				g_autoDialoutState = AUTO_DIAL_SLEEP;
			}
		break;

		//----------------------------------------------------------------
		// Sleep for variable retry time
		//----------------------------------------------------------------
		case AUTO_DIAL_SLEEP:
			// Check if the retry time has expired
			if ((g_rtcSoftTimerTickCount - timer) > (modem_setup_rec.retryTime * TICKS_PER_MIN))
			{
				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;

				// Start back at Init state
				g_autoDialoutState = AUTO_DIAL_INIT;
			}
		break;

		//----------------------------------------------------------------
		// Active connection
		//----------------------------------------------------------------
		case AUTO_DIAL_ACTIVE:
#if 0 // fix_ns8100
			// Check if modem data has been transfered (either sent or successful receipt of a message)
			if (g_modemDataTransfered == YES)
			{
				// Toggle the flag off
				g_modemDataTransfered = NO;

				// Update timer to current tick count
				timer = g_rtcSoftTimerTickCount;
			}
			// Check if data has not been transmitted in the last 5 minutes
			else if ((g_rtcSoftTimerTickCount - timer) > (5 * TICKS_PER_MIN))
			{
				// No data has been transfered in 5 minutes, tear down connection

				// Advance to Finish state
				g_autoDialoutState = AUTO_DIAL_FINISH;
			}
			// Check if we lose the remote connection
			else if (READ_DCD == NO_CONNECTION)
			{
				// Advance to Finish state
				g_autoDialoutState = AUTO_DIAL_FINISH;
			}
#endif
		break;

		//----------------------------------------------------------------
		// Finished with Auto dialout (either successful connection of failed retries)
		//----------------------------------------------------------------
		case AUTO_DIAL_FINISH:
			// Done with Auto Dialout processing, issue a modem reset
			modemResetProcess();

			// Place in Idle state
			g_autoDialoutState = AUTO_DIAL_IDLE;
		break;

		//----------------------------------------------------------------
		// Idle
		//----------------------------------------------------------------
		case AUTO_DIAL_IDLE:
			// Do nothing
		break;
	}
}
