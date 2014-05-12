///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _REMOTE_HANDLER_H_
#define _REMOTE_HANDLER_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteCommon.h"
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
void initCraftInterruptBuffers(void);

#endif // _REMOTE_COMMON_H_

