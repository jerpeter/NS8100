///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: RemoteModem.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/RemoteModem.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <string.h>
#include "Rec.h"
#include "Old_Board.h"
#include "Uart.h"
#include "RemoteModem.h"
#include "SoftTimer.h"

extern uint8 g_CRLF;
extern uint8 g_disableDebugPrinting;
extern MODEM_SETUP_STRUCT modem_setup_rec;
extern MODEM_STATUS_STRUCT g_ModemStatus;

uint8 modemResetStage = 0;

//==================================================
// Modem commands
//==================================================

//==================================================
//	Procedure: modemInitProcess()
//	Description:
//
//	Input: void
//	Output: none
//--------------------------------------------------
void modemInitProcess(void)
{
	if (g_ModemStatus.testingFlag == YES) g_disableDebugPrinting = NO;

	debug("modemInitProcess\n");

#if 0 // fix_ns8100
	if (READ_DCD == NO_CONNECTION)
	{
		if (strlen(modem_setup_rec.reset) != 0)
		{
			uart_puts((char*)(modem_setup_rec.reset), CRAFT_COM_PORT);
			uart_puts((char*)&g_CRLF, CRAFT_COM_PORT);

			soft_usecWait(3 * SOFT_SECS);
		}

		if (strlen(modem_setup_rec.init) != 0)
		{
			uart_puts((char*)(modem_setup_rec.init), CRAFT_COM_PORT);
			uart_puts((char*)&g_CRLF, CRAFT_COM_PORT);
		}
	}
#endif

	// Assume connected.
	g_ModemStatus.numberOfRings = 0;
	g_ModemStatus.ringIndicator = 0;

	g_ModemStatus.connectionState = CONNECTED;
	g_ModemStatus.firstConnection = NOP_CMD;
}

//==================================================
//	Procedure: modemResetProcess()
//	Description:
//
//	Input: void
//	Output: none
//--------------------------------------------------
void modemResetProcess(void)
{
	if (g_ModemStatus.testingFlag == YES) g_disableDebugPrinting = NO;
	debug("handleMRC\n");

	g_ModemStatus.systemIsLockedFlag = YES;

#if 0 // fix_ns8100
	CLEAR_DTR;
#endif
	modemResetStage = 1;
	assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(15 * TICKS_PER_SEC), modemResetTimerCallback);
}

//==================================================
// Function: handleMRC
// Description:
// 		Modem reset.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMRS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);

	g_ModemStatus.systemIsLockedFlag = YES;

	if (YES == modem_setup_rec.modemStatus)
	{
		modemResetProcess();
	}

	return;
}

//==================================================
// Function: handleMVS
// Description:
// 		Modem view settings.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMVS(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMPO
// Description:
// 		Toggle modem on/off.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMPO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMMO
// Description:
// 		Toggle modem mode transmit/receive.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMMO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMNO
// Description:
// 		Toggle modem phone number A/B/C.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMNO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMTO
// Description:
// 		Toggle modem log on/off.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMTO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSD
// Description:
// 		Modem set default initialization string.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSR
// Description:
// 		Modem set receive initialization string.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSR(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMST
// Description:
// 		Modem set transmit initialization string.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMST(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSA
// Description:
// 		Modem set phone number A.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSA(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSB
// Description:
// 		Modem set phone number B.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSB(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMSC
// Description:
// 		Modem set phone number C.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMSC(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMVI
// Description:
// 		Modem view last call in detail.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMVI(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleMVO
// Description:
// 		Modem view last call out detail.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleMVO(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

