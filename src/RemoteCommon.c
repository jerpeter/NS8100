///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Common.h"
#include "Uart.h"
#include "Record.h"
#include "RemoteHandler.h"
#include "RemoteCommon.h"
#include "Crc.h"
#include "EventProcessing.h"
#include "SysEvents.h"
#include "SoftTimer.h"

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
uint8 ParseIncommingMsgHeader(CMD_BUFFER_STRUCT* inCmd, COMMAND_MESSAGE_HEADER* incommingHdr)
{
	uint8 errCode = NO;
	char* msgPtr = (char*)inCmd->msg;			// A tempPtr into the message buffer.

	// clear the incomming header data.
	memset((uint8*)incommingHdr, 0, sizeof(COMMAND_MESSAGE_HEADER));

	if (strlen(msgPtr) >= HDR_CMD_LEN)
	{
		// Parse the string into a header data struct.
		memcpy(incommingHdr->cmd, msgPtr, HDR_CMD_LEN);
		msgPtr += HDR_CMD_LEN;
	}

	if (strlen(msgPtr) >= HDR_TYPE_LEN)
	{
		memcpy(incommingHdr->type, msgPtr, HDR_TYPE_LEN);
		msgPtr += HDR_TYPE_LEN;
	}

	if (strlen((char*)inCmd->msg) >= MESSAGE_HEADER_LENGTH)
	{
		memcpy(incommingHdr->dataLength, msgPtr, HDR_DATALENGTH_LEN);
		msgPtr += HDR_DATALENGTH_LEN;

		memcpy(incommingHdr->unitModel, msgPtr, HDR_UNITMODEL_LEN);
		msgPtr += HDR_UNITMODEL_LEN;

		memcpy(incommingHdr->unitSn, msgPtr, HDR_SERIALNUMBER_LEN);
		msgPtr += HDR_SERIALNUMBER_LEN;

		memcpy(incommingHdr->compressCrcFlags, msgPtr, HDR_COMPRESSCRC_LEN);
		msgPtr += HDR_COMPRESSCRC_LEN;

		memcpy(incommingHdr->softwareVersion, msgPtr, HDR_SOFTWAREVERSION_LEN);
		msgPtr += HDR_SOFTWAREVERSION_LEN;

		memcpy(incommingHdr->dataVersion, msgPtr, HDR_DATAVERSION_LEN);
		msgPtr += HDR_DATAVERSION_LEN;

		memcpy(incommingHdr->spare, msgPtr, HDR_SPARE_LEN);
		msgPtr += HDR_SPARE_LEN;
	}
	else
	{
		errCode = YES;
	}

	return (errCode);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ParseIncommingMsgCmd(CMD_BUFFER_STRUCT* inCmd, COMMAND_MESSAGE_HEADER* incommingHdr)
{
	uint8 errCode = NO;
	char* msgPtr = (char*)inCmd->msg;			// A tempPtr into the message buffer.

	// clear the incomming header data.
	memset((uint8*)incommingHdr, 0, sizeof(COMMAND_MESSAGE_HEADER));

	if (strlen(msgPtr) >= HDR_CMD_LEN)
	{
		// Parse the string into a header data struct.
		memcpy(incommingHdr->cmd, msgPtr, HDR_CMD_LEN);
		msgPtr += HDR_CMD_LEN;
	}
	else
	{
		errCode = YES;
	}

	return (errCode);
}


///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void BuildOutgoingHeaderBuffer(COMMAND_MESSAGE_HEADER* msgHdrData, uint8* msgHdrBuf)
{
	uint8* bufPtr = msgHdrBuf;

	memset(bufPtr, 0, MESSAGE_HEADER_LENGTH+1);

	// Parse the string into a header data struct.
	memcpy(bufPtr, msgHdrData->cmd, HDR_CMD_LEN);
	bufPtr += HDR_CMD_LEN;

	memcpy(bufPtr, msgHdrData->type, HDR_TYPE_LEN);
	bufPtr += HDR_TYPE_LEN;

	memcpy(bufPtr, msgHdrData->dataLength, HDR_DATALENGTH_LEN);
	bufPtr += HDR_DATALENGTH_LEN;

	memcpy(bufPtr, msgHdrData->unitModel, HDR_UNITMODEL_LEN);
	bufPtr += HDR_UNITMODEL_LEN;

	memcpy(bufPtr, msgHdrData->unitSn, HDR_SERIALNUMBER_LEN);
	bufPtr += HDR_SERIALNUMBER_LEN;

	memcpy(bufPtr, msgHdrData->compressCrcFlags, HDR_COMPRESSCRC_LEN);
	bufPtr += HDR_COMPRESSCRC_LEN;

	memcpy(bufPtr, msgHdrData->softwareVersion, HDR_SOFTWAREVERSION_LEN);
	bufPtr += HDR_SOFTWAREVERSION_LEN;

	memcpy(bufPtr, msgHdrData->dataVersion, HDR_DATAVERSION_LEN);
	bufPtr += HDR_DATAVERSION_LEN;

	memcpy(bufPtr, msgHdrData->spare, HDR_SPARE_LEN);
	bufPtr += HDR_SPARE_LEN;

}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void BuildOutgoingSimpleHeaderBuffer(uint8* msgHdrBuf,
	uint8* msgCmd, uint8* msgType, uint32 dataLength,
	uint8 verFlag, uint8 crcFlag)
{
	uint8* bufPtr = msgHdrBuf;
	//uint8 verData = 0;

	UNUSED(verFlag);
	UNUSED(crcFlag);

	memset(bufPtr, 0, MESSAGE_HEADER_SIMPLE_LENGTH);

	// Parse the string into a header data struct.
	memcpy(bufPtr, msgCmd, HDR_CMD_LEN);
	bufPtr += HDR_CMD_LEN;

	memcpy(bufPtr, msgType, HDR_TYPE_LEN);
	bufPtr += HDR_TYPE_LEN;

	BuildIntDataField((char*)bufPtr, dataLength, FIELD_LEN_08);
	bufPtr += HDR_DATALENGTH_LEN;

	// Put in the version number and if the data message has a crcFlag.
	//*bufPtr++ = verFlag;
	//*bufPtr = crcFlag;
	*bufPtr++ = 0x46;
	*bufPtr = 0x46;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void SendErrorMsg(uint8* msgCmd, uint8* msgType)
{
	uint32 msgCRC = 0;
	uint8 errHdr[MESSAGE_HEADER_SIMPLE_LENGTH];

	BuildOutgoingSimpleHeaderBuffer(errHdr, msgCmd, msgType,
		MESSAGE_SIMPLE_TOTAL_LENGTH, COMPRESS_NONE, CRC_NONE);

	// Send Starting CRLF
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);
	// Send Simple header
	ModemPuts((uint8*)errHdr, MESSAGE_HEADER_SIMPLE_LENGTH, CONVERT_DATA_TO_ASCII);
	// Send Ending Footer
	ModemPuts((uint8*)&msgCRC, 4, NO_CONVERSION);
	ModemPuts((uint8*)&g_CRLF, 2, NO_CONVERSION);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint16 GetInt16Field(uint8* dataPtr)
{
	uint16 int16Data = 0;
	uint8 dataStr[DATA_FIELD_LEN+1];

	memset(dataStr, 0, sizeof(dataStr));
	memcpy(dataStr, dataPtr, DATA_FIELD_LEN);
	int16Data = (uint16)DataLengthStrToUint32(dataStr);


	return (int16Data);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void BuildIntDataField(char* strBuffer, uint32 data, uint8 fieldLen)
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint32 DataLengthStrToUint32(uint8* dataLengthStr)
{
	uint32 dataLength = 0;
	uint8 strDex = 0;
	uint8 dataLenBuf[HDR_DATALENGTH_LEN+1];

	uint8* dataStr = dataLengthStr;
	uint8* dataBuf = dataLenBuf;


	memset(&dataLenBuf[0], 0, sizeof(dataLenBuf));

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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteCompressedData(uint8 compressedData)
{
	g_demXferStructPtr->xmitBuffer[g_demXferStructPtr->xmitSize] = compressedData;
	g_demXferStructPtr->xmitSize++;

	if (g_demXferStructPtr->xmitSize >= XMIT_SIZE_MONITORING)
	{
		g_transmitCRC = CalcCCITT32((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, g_transmitCRC);

		if (ModemPuts((uint8*)g_demXferStructPtr->xmitBuffer, g_demXferStructPtr->xmitSize, NO_CONVERSION) == MODEM_SEND_FAILED)
		{
			g_demXferStructPtr->errorStatus = MODEM_SEND_FAILED;
		}

		g_transferCount += g_demXferStructPtr->xmitSize;
		g_demXferStructPtr->xmitSize = 0;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitAutoDialout(void)
{
	// Check if the table key is not valid
	if (__autoDialoutTblKey != VALID_AUTODIALOUT_TABLE_KEY)
	{
		// Clear AutoDialout Log table
		memset(&__autoDialoutTbl, 0, sizeof(__autoDialoutTbl));

		// No need to set the following since the memset takes care of this
		//__autoDialoutTbl.lastDownloadedEvent = 0;
		//__autoDialoutTbl.lastConnectTime.valid = FALSE;

		__autoDialoutTblKey = VALID_AUTODIALOUT_TABLE_KEY;
	}

	// Update the last stored event
	__autoDialoutTbl.lastStoredEvent = GetLastStoredEventNumber();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CheckAutoDialoutStatus(void)
{
	if ((g_autoDialoutState == AUTO_DIAL_IDLE) && (READ_DCD == NO_CONNECTION) &&
		(g_modemSetupRecord.modemStatus == YES) && strlen((char*)&(g_modemSetupRecord.dial[0])) != 0)
	{
		raiseSystemEventFlag(AUTO_DIALOUT_EVENT);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void StartAutoDialoutProcess(void)
{
	if (READ_DCD == NO_CONNECTION)
	{
		g_autoRetries = g_modemSetupRecord.retries;
		g_autoDialoutState = AUTO_DIAL_INIT;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AutoDialoutStateMachine(void)
{
	static uint32 timer = 0;
	CMD_BUFFER_STRUCT msg;

	switch (g_autoDialoutState)
	{
		//----------------------------------------------------------------
		// Send Dial string
		//----------------------------------------------------------------
		case AUTO_DIAL_INIT:
			// Issue dial command and dial string
			if((g_modemSetupRecord.dial[0] >= '0') && (g_modemSetupRecord.dial[0] <= '9'))
			{
				UartPuts((char *)"ATDT", CRAFT_COM_PORT);
			}
			UartPuts((char *)(g_modemSetupRecord.dial), CRAFT_COM_PORT);
			UartPuts((char *)&g_CRLF, CRAFT_COM_PORT);

			// Update timer to current tick count
			timer = g_rtcSoftTimerTickCount;

			// Advance to Connecting state
			g_autoDialoutState = AUTO_DIAL_CONNECTING;
		break;

		//----------------------------------------------------------------
		// Look for DCD
		//----------------------------------------------------------------
		case AUTO_DIAL_CONNECTING:
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
		break;

		//----------------------------------------------------------------
		// Send out GAD command
		//----------------------------------------------------------------
		case AUTO_DIAL_CONNECTED:
			// Check if the current connection has been established for 5 seconds
			if ((g_rtcSoftTimerTickCount - timer) > (5 * TICKS_PER_SEC))
			{
				// Make sure transfer flag is set to ascii
				g_binaryXferFlag = NO_CONVERSION;

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
		break;

		//----------------------------------------------------------------
		// Look for system to be unlocked
		//----------------------------------------------------------------
		case AUTO_DIAL_RESPONSE:
			// Check if the system has been unlocked (thus successful receipt of an unlock command)
			if (g_modemStatus.systemIsLockedFlag == NO)
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
		break;

		//----------------------------------------------------------------
		// Wait for system to be unlocked (2nd attempt)
		//----------------------------------------------------------------
		case AUTO_DIAL_WAIT:
			// Check if the system has been unlocked (thus successful receipt of an unlock command)
			if (g_modemStatus.systemIsLockedFlag == NO)
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
				ModemResetProcess();

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
			if ((g_rtcSoftTimerTickCount - timer) > (g_modemSetupRecord.retryTime * TICKS_PER_MIN))
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
		break;

		//----------------------------------------------------------------
		// Finished with Auto dialout (either successful connection of failed retries)
		//----------------------------------------------------------------
		case AUTO_DIAL_FINISH:
			// Done with Auto Dialout processing, issue a modem reset
			ModemResetProcess();

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
