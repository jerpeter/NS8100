///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteHandler.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/RemoteHandler.c,v $
///	$Revision: 1.1 $
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
///	Externs
///----------------------------------------------------------------------------
extern uint8 g_disableDebugPrinting;
extern SYS_EVENT_STRUCT SysEvents_flags;
extern CMD_BUFFER_STRUCT* gp_ISRMessageBuffer;
extern MODEM_STATUS_STRUCT g_ModemStatus;
extern MODEM_SETUP_STRUCT modem_setup_rec;
extern REC_HELP_MN_STRUCT help_rec;				// Struct containing system parameters 
extern uint8 g_modemDataTransfered;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
uint16 g_CRLF = 0x0D0A;

// Index for the message pool.
uint8 readIndex;
uint8 writeIndex;
// Holds a pool of buffers for processing input from the craft port
CMD_BUFFER_STRUCT g_msgPool[CMD_MSG_POOL_SIZE];

const COMMAND_MESSAGE_STRUCT cmdMessageTable[ TOTAL_COMMAND_MESSAGES ] = {
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
// Function:	initInterruptBuffers
// Purpose :	
/********************************************************************/
void initInterruptBuffers(void)
{
	byteSet(gp_ISRMessageBuffer, 0, sizeof(CMD_BUFFER_STRUCT));
	gp_ISRMessageBuffer->status = CMD_MSG_NO_ERR;
	gp_ISRMessageBuffer->overRunCheck = 0xBADD;
	gp_ISRMessageBuffer->writePtr = gp_ISRMessageBuffer->readPtr = gp_ISRMessageBuffer->msg;
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
		
	if (g_ModemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

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
		if (	(YES == g_ModemStatus.systemIsLockedFlag) &&
			(YES == g_ModemStatus.xferMutex))
		{
			g_ModemStatus.xferState = NOP_CMD;
			g_ModemStatus.xferMutex = NO;
		}
	}

	else 
	{ 
		if (NO == g_ModemStatus.systemIsLockedFlag)
		{
			debug("System NOT Locked\n");
		
			// If the system is unlocked and a xfer command is not in progress 
			// look for the next command to complet. Else, toss the message.
			if (NO == g_ModemStatus.xferMutex)
			{
				for (cmdIndex = 0; cmdIndex < TOTAL_COMMAND_MESSAGES; cmdIndex++)
				{
					if ((cmdMsg->msg[0] == cmdMessageTable[ cmdIndex ].cmdChar1) &&
						(cmdMsg->msg[1] == cmdMessageTable[ cmdIndex ].cmdChar2) &&
						(cmdMsg->msg[2] == cmdMessageTable[ cmdIndex ].cmdChar3))
					{
						// Command successfully decoded, signal that data has been transfered
						g_modemDataTransfered = YES;

						cmdMessageTable[ cmdIndex ].cmdFunction(cmdMsg);
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

		
	if ((g_ModemStatus.testingFlag == YES) && (g_ModemStatus.testingPrintFlag == YES))
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
	if (0xBADD != g_msgPool[readIndex].overRunCheck)
	{
		g_msgPool[readIndex].overRunCheck = 0xBADD;
		overlayMessage(getLangText(STATUS_TEXT), "CRAFT MESSAGE OVERRUN", 0);
	}

	// NOTE: Need a different message if the command comming across contains
	// data or additional info other then the 3 char cmd field. Currently
	// assuming that only 3 char cmds are being sent. If data or a field is
	// sent with the incomming command we need to deal with it differently.

	cmdMessageHandler(&(g_msgPool[readIndex]));

	byteSet(g_msgPool[readIndex].msg, 0, sizeof(CMD_BUFFER_SIZE));
	g_msgPool[readIndex].size = 0;	
	g_msgPool[readIndex].readPtr = g_msgPool[readIndex].msg;	
	g_msgPool[readIndex].writePtr = g_msgPool[readIndex].msg;
	g_msgPool[readIndex].overRunCheck = 0xBADD;
	
	readIndex++;
	if (readIndex >= CMD_MSG_POOL_SIZE)
	{
		readIndex = 0;
	}

	// Are any more buffers filled?
	if (readIndex != writeIndex)
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
	if (g_ModemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

	// Check status and then reset it to no error.
	if (CMD_MSG_OVERFLOW_ERR == gp_ISRMessageBuffer->status)
	{
		gp_ISRMessageBuffer->status = CMD_MSG_NO_ERR;
		overlayMessage(getLangText(STATUS_TEXT), "MODEM SYNC FAILED", 0);
	}

	// Check status and then reset it to no error.
	if (0xBADD != gp_ISRMessageBuffer->overRunCheck)
	{
		gp_ISRMessageBuffer->overRunCheck = 0xBADD;
		overlayMessage(getLangText(STATUS_TEXT), "CRAFT OVERRUN ERROR", 0);
	}

	while (gp_ISRMessageBuffer->readPtr != gp_ISRMessageBuffer->writePtr)
	{
		debugRaw("<%c>",*gp_ISRMessageBuffer->readPtr);

		if ((*gp_ISRMessageBuffer->readPtr != 0x0A) &&
			(*gp_ISRMessageBuffer->readPtr != 0x0D))
		{
			*(g_msgPool[writeIndex].writePtr) = *gp_ISRMessageBuffer->readPtr;
			g_msgPool[writeIndex].writePtr++;
			g_msgPool[writeIndex].size++;

			// The buffer is full, go to the next buffer pool.
			if (g_msgPool[writeIndex].size >= (CMD_BUFFER_SIZE-2))
			{
				newPoolBuffer = YES;
			}
		}
		else
		{
			if (g_msgPool[writeIndex].size > 0)
			{
				newPoolBuffer = YES;
			}	
		}


		if (YES == newPoolBuffer)
		{
			// The message is now complete so go to the next message pool buffer.
			writeIndex++;
			if (writeIndex >= CMD_MSG_POOL_SIZE)
			{
				writeIndex = 0;
			}
			
			// Flag to indicate complete message to process
			raiseSystemEventFlag(CRAFT_PORT_EVENT);
		}
		
		*gp_ISRMessageBuffer->readPtr = 0x00;
		gp_ISRMessageBuffer->readPtr++;
		if (gp_ISRMessageBuffer->readPtr >=  (gp_ISRMessageBuffer->msg + CMD_BUFFER_SIZE))
		{			
			gp_ISRMessageBuffer->readPtr = gp_ISRMessageBuffer->msg;
		}
	}

	if ((g_ModemStatus.testingFlag == YES) && (g_ModemStatus.testingPrintFlag == YES))
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

	for (writeIndex = 0; writeIndex < CMD_MSG_POOL_SIZE; writeIndex++)
	{	
		g_msgPool[writeIndex].overRunCheck = 0xBADD;
		g_msgPool[writeIndex].readPtr = g_msgPool[writeIndex].msg;
		g_msgPool[writeIndex].writePtr = g_msgPool[writeIndex].msg;
	}
	
	// Initialize index to the start.
	writeIndex = readIndex = 0;

	return;
}

/********************************************************************/
// Function:	craftInitStatusFlags
// Purpose :	
/********************************************************************/
void craftInitStatusFlags(void)
{
	byteSet(&g_ModemStatus, 0, sizeof(MODEM_STATUS_STRUCT));

	// Modem and craft port specific flags.
	g_ModemStatus.connectionState = NOP_CMD;	// State flag to indicate which modem command to handle.

	// Check if the Modem setup record is valid and Modem status is yes
	if ((!modem_setup_rec.invalid) && (modem_setup_rec.modemStatus == YES))
	{
		// Signal that the modem is available
		g_ModemStatus.modemAvailable = YES;

		assignSoftTimer(MODEM_DELAY_TIMER_NUM, (MODEM_ATZ_DELAY), modemDelayTimerCallback);
	}
	else
	{
		// Signal that the modem is not available
		g_ModemStatus.modemAvailable = NO;
	}

	g_ModemStatus.craftPortRcvFlag = NO;	// Flag to indicate that incomming data has been received.
	g_ModemStatus.xferState = NOP_CMD;		// Flag for xmitting data to the craft.
	g_ModemStatus.xferMutex = NO;			// Flag to stop other message command from executing.
	g_ModemStatus.systemIsLockedFlag = YES;

	g_ModemStatus.ringIndicator = 0;
	g_ModemStatus.xferPrintState = help_rec.auto_print;
	
	// Modem is being tested/debugged set debug to true.
	g_ModemStatus.testingPrintFlag = g_disableDebugPrinting;		
	// Modem is being tested/debugged, set to print to the PC
	g_ModemStatus.testingFlag = MODEM_DEBUG_TEST_FLAG;
}

