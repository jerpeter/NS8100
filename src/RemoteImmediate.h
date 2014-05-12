///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _REMOTE_IMMEDIATE_H_
#define _REMOTE_IMMEDIATE_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteCommon.h"
#include "Summary.h"

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
// Function: handleUNL - Unlock unit.
void handleUNL(CMD_BUFFER_STRUCT* inCmd);

// Function: handleRST - Reset the unit.
void handleRST(CMD_BUFFER_STRUCT* inCmd);

// Function: handleDDP - Toggle debug printing.
void handleDDP(CMD_BUFFER_STRUCT* inCmd);

// Function: handleDAI - Download Application Image.
void handleDAI(CMD_BUFFER_STRUCT* inCmd);


// Function: handleESM - Erase summary memory.
void handleESM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleEEM - Erase event memory.
void handleEEM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleECM - Erase configuration memory.
void handleECM(CMD_BUFFER_STRUCT* inCmd);

// Function: handleECM - Start a Trigger.
void handleTRG(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVML - View Monitor Log
void handleVML(CMD_BUFFER_STRUCT* inCmd);
void sendVMLData(void);

// Function: handleUDE - Update last Downloaded Event number
void handleUDE(CMD_BUFFER_STRUCT* inCmd);

// Function: handleGAD - Get Auto-dialout/Download information
void handleGAD(CMD_BUFFER_STRUCT* inCmd);

// Function: handleGFS - Get flash stats
void handleGFS(CMD_BUFFER_STRUCT* inCmd);

// Function: handleDQS - Download Quick Summary Memory.
void handleDQM(CMD_BUFFER_STRUCT* inCmd);
uint8 sendDQMData(void);

// Function: handleDSM - Download summary memory.
void handleDSM(CMD_BUFFER_STRUCT* inCmd);
uint8 sendDSMData(void);

// Function: handleDEM - Download event memory.
void handleDEM(CMD_BUFFER_STRUCT* inCmd);
void prepareDEMDataToSend(EVT_RECORD*, COMMAND_MESSAGE_HEADER*);
uint8 sendDEMData(void);
uint8* sendDataNoFlashWrapCheck(uint8*, uint8*);
uint8* sendDataFlashWrapCheck(uint8*);

// Function: handleGMN - Start Monitoring waveform/bargraph/combo.
void handleGMN(CMD_BUFFER_STRUCT* inCmd);

// Function: handleHLP - Halt Monitoring waveform/bargraph/combo.
void handleHLT(CMD_BUFFER_STRUCT* inCmd);


void debugSummaryData(SUMMARY_DATA* ramTblElement);


#endif // _REMOTE_IMMEDIATE_H_

