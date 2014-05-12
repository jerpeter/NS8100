///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteOperation.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:59 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteOperation.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _REMOTE_OPERATION_H_
#define _REMOTE_OPERATION_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteCommon.h"
                            
///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
//==================================================
// Dummy commands
//==================================================

// Function: handleAAA - Dummy function for testing purposes.
void handleAAA(CMD_BUFFER_STRUCT* inCmd);

//==================================================
// Operating parameter commands
//==================================================

// Function: handleDCM - Download configuation memory.
void handleDCM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleUCM - Upload configuation memory.
void handleUCM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleDMM - Download modem configuation memory.
void handleDMM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleUMM - Upload modem configuation memory.
void handleUMM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVTI - View time.
void handleVTI(CMD_BUFFER_STRUCT* inCmd);

// Function: handleSTI - Set time.
void handleSTI(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVDA - View date.
void handleVDA(CMD_BUFFER_STRUCT* inCmd);

// Function: handleSDA - Set date.
void handleSDA(CMD_BUFFER_STRUCT* inCmd);

// Function: handleZRO - Zero sensors.
void handleZRO(CMD_BUFFER_STRUCT* inCmd);

// Function: handleTTO - Toggle test mode on/off.
void handleTTO(CMD_BUFFER_STRUCT* inCmd);

// Function: handleCAL - Calibrate sensors with cal pulse.
void handleCAL(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVOL - View on/off log.
void handleVOL(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVCG - View command log.
void handleVCG(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVSL - View summary log.
void handleVSL(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVEL - View event log.
void handleVEL(CMD_BUFFER_STRUCT* inCmd);

uint8 convertAscii2Binary(uint8 firstByte, uint8 secondByte);

#endif // _REMOTE_OPERATION_H_

