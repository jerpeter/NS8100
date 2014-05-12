///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteHandler.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:58 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteHandler.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _REMOTE_HANDLER_H_
#define _REMOTE_HANDLER_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteCommon.h"
#include "RemoteModem.h"
#include "RemoteOperation.h"
#include "RemoteImmediate.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
//	Procedure: cmdMessageHandler() - Start determining the type of command.
uint8 cmdMessageHandler(CMD_BUFFER_STRUCT* inCmd);


//	Procedure: cmdMessageHandlerInit() - Initialize the buffer variables
void cmdMessageHandlerInit(void);

//	Procedure: cmdMessageProcessing() Processing the incomming data 
//	string and determine if a valid cmd has been received. 
void cmdMessageProcessing(void);
void processCraftData(void);
void craftInitStatusFlags(void);
void initInterruptBuffers(void);

#endif // _REMOTE_COMMON_H_

