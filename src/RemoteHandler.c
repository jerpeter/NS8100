///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteHandler.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:58 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteHandler.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <string.h>
#include "Common.h"
#include "Uart.h"
#include "SysEvents.h"
#include "RemoteHandler.h"
#include "Menu.h"
#include "SoftTimer.h"
#include "Common.h"
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
static uint8 s_msgReadIndex;
static uint8 s_msgWriteIndex;
static const COMMAND_MESSAGE_STRUCT s_cmdMessageTable[ TOTAL_COMMAND_MESSAGES ] = {
	{ 'A', 'A', 'A', handleAAA },	// Dummy function call.
	{ 'M', 'R', 'S', handleMRS },		// Modem reset.

#if 0	
	{ 'M', 'V', 'S', handleMVS },		// Modem view settings.
	{ 'M', 'P', 'O', handleMPO },		// Toggle modem on/off.
	{ 'M', 'M', 'O', handleMMO },		// Toggle modem mode transmit/receive.
	{ 'M', 'N', 'O', handleMNO },		// Toggle modem phone number A/B/C.
	{ 'M', 'T', 'O', handleMTO },		// Toggle modem log on/off.
	{ 'M', 'S', 'D', handleMSD },		// Modem set default initialization string.
	{ 'M', 'S', 'R', handleMSR },		// Modem set receive initialization string.
	{ 'M', 'S', 'T', handleMST },		// Modem set transmit initialization string.
	{ 'M', 'S', 'A', handleMSA },		// Modem set phone number A.
	{ 'M', 'S', 'B', handleMSB },		// Modem set phone number B.
	{ 'M', 'S', 'C', handleMSC },		// Modem set phone number C.
	{ 'M', 'V', 'I', handleMVI },		// Modem view last call in detail.
	{ 'M', 'V', 'O', handleMVO },		// Modem view last call out detail.
	{ 'V', 'T', 'I', handleVTI },		// View time.
	{ 'S', 'T', 'I', handleSTI },		// Set time.
	{ 'V', 'D', 'A', handleVDA },		// View date.
	{ 'S', 'D', 'A', handleSDA },		// Set date.
#endif

	// Immediate commands
	{ 'U', 'N', 'L', handleUNL },		// Unlock unit.
	{ 'R', 'S', 'T', handleRST },		// Reset the unit.
	{ 'D', 'D', 'P', handleDDP },		// Disable Debug printing.
	{ 'D', 'A', 'I', handleDAI },		// Download App Image.

#if 0
	{ 'Z', 'R', 'O', handleZRO },		// Zero sensors.
	{ 'T', 'T', 'O', handleTTO },		// Toggle test mode on/off.
	{ 'C', 'A', 'L', handleCAL },		// Calibrate sensors with cal pulse.
	{ 'V', 'O', 'L', handleVOL },		// View on/off log.
	{ 'V', 'C', 'G', handleVCG },		// View command log.
	{ 'V', 'S', 'L', handleVSL },		// View summary log.
	{ 'V', 'E', 'L', handleVEL },		// View event log.
	{ 'E', 'S', 'M', handleESM },		// Erase summary memory.
	{ 'E', 'C', 'M', handleECM },		// Erase configuration memory.
	{ 'T', 'R', 'G', handleTRG },		// Trigger an event.
#endif

	{ 'V', 'M', 'L', handleVML },		// View Monitor log
	{ 'D', 'Q', 'M', handleDQM },		// Download summary memory.
	{ 'D', 'S', 'M', handleDSM },		// Download summary memory.
	{ 'D', 'E', 'M', handleDEM },		// Download event memory.
	{ 'E', 'E', 'M', handleEEM },		// Erase event memory.
	{ 'D', 'C', 'M', handleDCM },		// Download configuration memory.
	{ 'U', 'C', 'M', handleUCM },		// Upload configuration memory.
	{ 'D', 'M', 'M', handleDMM },		// Download modem configuration memory.
	{ 'U', 'M', 'M', handleUMM },		// Upload modem configuration memory.
	{ 'G', 'M', 'N', handleGMN },		// Start Monitoring waveform/bargraph/combo.
	{ 'H', 'L', 'T', handleHLT },		// Halt Monitoring waveform/bargraph/combo.
	{ 'G', 'A', 'D', handleGAD },		// Get Auto-Dialout/Download information
	{ 'G', 'F', 'S', handleGFS },		// Get Flash Stats
	{ 'Z', 'Z', 'Z', handleAAA }		// Help on menus.
};

/********************************************************************/
// Function:	initCraftInterruptBuffers
// Purpose :	
/********************************************************************/
void initCraftInterruptBuffers(void)
{
	byteSet(g_isrMessageBufferPtr, 0, sizeof(CMD_BUFFER_STRUCT));
	g_isrMessageBufferPtr->status = CMD_MSG_NO_ERR;
	g_isrMessageBufferPtr->overRunCheck = 0xBADD;
	g_isrMessageBufferPtr->writePtr = g_isrMessageBufferPtr->readPtr = g_isrMessageBufferPtr->msg;
}

//==================================================
//	Procedure: cmdMessageHandler()
//	Description: 
//		Start determining the type of command.
//	Input: CMD_BUFFER_STRUCT* inCmd
//	Output: none
//--------------------------------------------------
uint8 cmdMessageHandler(CMD_BUFFER_STRUCT* cmdMsg)
{
	uint8 cmdIndex = 0;
		
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

	debug("\nCMH:<%s>\n", cmdMsg->msg);

	// Fill in the data
	CHAR_UPPER_CASE(cmdMsg->msg[0]);
	CHAR_UPPER_CASE(cmdMsg->msg[1]);
	CHAR_UPPER_CASE(cmdMsg->msg[2]);		
		
	if ((cmdMsg->msg[0] == 'U') &&
		(cmdMsg->msg[1] == 'N') &&
		(cmdMsg->msg[2] == 'L'))
	{
		handleUNL(cmdMsg);
		
		// If the system is now locked and there is a xfer in progress, 
		// halt the xfer. This is used as a break command.
		if (	(YES == g_modemStatus.systemIsLockedFlag) &&
			(YES == g_modemStatus.xferMutex))
		{
			g_modemStatus.xferState = NOP_CMD;
			g_modemStatus.xferMutex = NO;
		}
	}

	else 
	{ 
		if (NO == g_modemStatus.systemIsLockedFlag)
		{
			debug("System NOT Locked\n");
		
			// If the system is unlocked and a xfer command is not in progress 
			// look for the next command to complet. Else, toss the message.
			if (NO == g_modemStatus.xferMutex)
			{
				for (cmdIndex = 0; cmdIndex < TOTAL_COMMAND_MESSAGES; cmdIndex++)
				{
					if ((cmdMsg->msg[0] == s_cmdMessageTable[ cmdIndex ].cmdChar1) &&
						(cmdMsg->msg[1] == s_cmdMessageTable[ cmdIndex ].cmdChar2) &&
						(cmdMsg->msg[2] == s_cmdMessageTable[ cmdIndex ].cmdChar3))
					{
						// Command successfully decoded, signal that data has been transfered
						g_modemDataTransfered = YES;

						s_cmdMessageTable[ cmdIndex ].cmdFunction(cmdMsg);
						break;
					}
				}
			}	
		}	
		else
		{
			debug("System IS Locked\n");
		}
	}

		
	if ((g_modemStatus.testingFlag == YES) && (g_modemStatus.testingPrintFlag == YES))
	{
		g_disableDebugPrinting = YES;
	}

	return (0);
}



//==================================================
//	Procedure: cmdMessageProcessing()
//	Description: 
//		Processing the incomming data string and determine if a 
//		valid cmd has been received. If it has handle the msg.
//	Input: CMD_BUFFER_STRUCT* inCmd
//	Output: none
//--------------------------------------------------
void cmdMessageProcessing()
{
	// Check if there is a potentially fatal error.
	if (0xBADD != g_msgPool[s_msgReadIndex].overRunCheck)
	{
		g_msgPool[s_msgReadIndex].overRunCheck = 0xBADD;
		overlayMessage(getLangText(STATUS_TEXT), "CRAFT MESSAGE OVERRUN", 0);
	}

	// NOTE: Need a different message if the command comming across contains
	// data or additional info other then the 3 char cmd field. Currently
	// assuming that only 3 char cmds are being sent. If data or a field is
	// sent with the incomming command we need to deal with it differently.

	cmdMessageHandler(&(g_msgPool[s_msgReadIndex]));

	byteSet(g_msgPool[s_msgReadIndex].msg, 0, sizeof(CMD_BUFFER_SIZE));
	g_msgPool[s_msgReadIndex].size = 0;	
	g_msgPool[s_msgReadIndex].readPtr = g_msgPool[s_msgReadIndex].msg;	
	g_msgPool[s_msgReadIndex].writePtr = g_msgPool[s_msgReadIndex].msg;
	g_msgPool[s_msgReadIndex].overRunCheck = 0xBADD;
	
	s_msgReadIndex++;
	if (s_msgReadIndex >= CMD_MSG_POOL_SIZE)
	{
		s_msgReadIndex = 0;
	}

	// Are any more buffers filled?
	if (s_msgReadIndex != s_msgWriteIndex)
	{
		// Flag to indicate complete message to process
		raiseSystemEventFlag(CRAFT_PORT_EVENT);
	}

	return;
}



//==================================================
//	Procedure: processCraftData()
//	Description: 
//	Input: 
//	Output: none
//--------------------------------------------------
void processCraftData()			
{
	uint8 newPoolBuffer = NO;

	// Halt all debugging message when recieving data.	
	if (g_modemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

	// Check status and then reset it to no error.
	if (CMD_MSG_OVERFLOW_ERR == g_isrMessageBufferPtr->status)
	{
		g_isrMessageBufferPtr->status = CMD_MSG_NO_ERR;
		overlayMessage(getLangText(STATUS_TEXT), "MODEM SYNC FAILED", 0);
	}

	// Check status and then reset it to no error.
	if (0xBADD != g_isrMessageBufferPtr->overRunCheck)
	{
		g_isrMessageBufferPtr->overRunCheck = 0xBADD;
		overlayMessage(getLangText(STATUS_TEXT), "CRAFT OVERRUN ERROR", 0);
	}

	while (g_isrMessageBufferPtr->readPtr != g_isrMessageBufferPtr->writePtr)
	{
		debugRaw("<%c>",*g_isrMessageBufferPtr->readPtr);

		if ((*g_isrMessageBufferPtr->readPtr != 0x0A) &&
			(*g_isrMessageBufferPtr->readPtr != 0x0D))
		{
			*(g_msgPool[s_msgWriteIndex].writePtr) = *g_isrMessageBufferPtr->readPtr;
			g_msgPool[s_msgWriteIndex].writePtr++;
			g_msgPool[s_msgWriteIndex].size++;

			// The buffer is full, go to the next buffer pool.
			if (g_msgPool[s_msgWriteIndex].size >= (CMD_BUFFER_SIZE-2))
			{
				newPoolBuffer = YES;
			}
		}
		else
		{
			if (g_msgPool[s_msgWriteIndex].size > 0)
			{
				newPoolBuffer = YES;
			}	
		}


		if (YES == newPoolBuffer)
		{
			// The message is now complete so go to the next message pool buffer.
			s_msgWriteIndex++;
			if (s_msgWriteIndex >= CMD_MSG_POOL_SIZE)
			{
				s_msgWriteIndex = 0;
			}
			
			// Flag to indicate complete message to process
			raiseSystemEventFlag(CRAFT_PORT_EVENT);
		}
		
		*g_isrMessageBufferPtr->readPtr = 0x00;
		g_isrMessageBufferPtr->readPtr++;
		if (g_isrMessageBufferPtr->readPtr >=  (g_isrMessageBufferPtr->msg + CMD_BUFFER_SIZE))
		{			
			g_isrMessageBufferPtr->readPtr = g_isrMessageBufferPtr->msg;
		}
	}

	if ((g_modemStatus.testingFlag == YES) && (g_modemStatus.testingPrintFlag == YES))
	{
		g_disableDebugPrinting = YES;
	}
	
	return;
}

//==================================================
//	Procedure: cmdMessageHandlerInit()
//	Description: 
//		Initialize the buffer variables.
//	Input: CMD_BUFFER_STRUCT* inCmd
//	Output: none
//--------------------------------------------------
void cmdMessageHandlerInit()
{	
	// Clear and set up the addresses for the ptrs from the buffer array.
	byteSet(g_msgPool, 0, (sizeof(CMD_BUFFER_STRUCT) * CMD_MSG_POOL_SIZE));

	for (s_msgWriteIndex = 0; s_msgWriteIndex < CMD_MSG_POOL_SIZE; s_msgWriteIndex++)
	{	
		g_msgPool[s_msgWriteIndex].overRunCheck = 0xBADD;
		g_msgPool[s_msgWriteIndex].readPtr = g_msgPool[s_msgWriteIndex].msg;
		g_msgPool[s_msgWriteIndex].writePtr = g_msgPool[s_msgWriteIndex].msg;
	}
	
	// Initialize index to the start.
	s_msgWriteIndex = s_msgReadIndex = 0;

	return;
}

/********************************************************************/
// Function:	craftInitStatusFlags
// Purpose :	
/********************************************************************/
void craftInitStatusFlags(void)
{
	byteSet(&g_modemStatus, 0, sizeof(MODEM_STATUS_STRUCT));

	// Modem and craft port specific flags.
	g_modemStatus.connectionState = NOP_CMD;	// State flag to indicate which modem command to handle.

	// Check if the Modem setup record is valid and Modem status is yes
	if ((!g_modemSetupRecord.invalid) && (g_modemSetupRecord.modemStatus == YES))
	{
		// Signal that the modem is available
		g_modemStatus.modemAvailable = YES;

		assignSoftTimer(MODEM_DELAY_TIMER_NUM, (MODEM_ATZ_DELAY), modemDelayTimerCallback);
	}
	else
	{
		// Signal that the modem is not available
		g_modemStatus.modemAvailable = NO;
	}

	g_modemStatus.craftPortRcvFlag = NO;	// Flag to indicate that incomming data has been received.
	g_modemStatus.xferState = NOP_CMD;		// Flag for xmitting data to the craft.
	g_modemStatus.xferMutex = NO;			// Flag to stop other message command from executing.
	g_modemStatus.systemIsLockedFlag = YES;

	g_modemStatus.ringIndicator = 0;
	g_modemStatus.xferPrintState = g_helpRecord.auto_print;
	
	// Modem is being tested/debugged set debug to true.
	g_modemStatus.testingPrintFlag = g_disableDebugPrinting;		
	// Modem is being tested/debugged, set to print to the PC
	g_modemStatus.testingFlag = MODEM_DEBUG_TEST_FLAG;
}

