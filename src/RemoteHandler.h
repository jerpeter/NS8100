///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteHandler.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/RemoteHandler.h,v $
///	$Revision: 1.1 $
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

