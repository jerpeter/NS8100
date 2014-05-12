///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
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

// Function: handleVBD - View backlight delay.
void handleVBD(CMD_BUFFER_STRUCT* inCmd);

// Function: handleSBD - Set backlight delay.
void handleSBD(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVDT - View display timeout.
void handleVDT(CMD_BUFFER_STRUCT* inCmd);

// Function: handleSDT - Set display timeout.
void handleSDT(CMD_BUFFER_STRUCT* inCmd);

// Function: handleVCL - View contrast level.
void handleVCL(CMD_BUFFER_STRUCT* inCmd);

// Function: handleSCL - Set contrast level.
void handleSCL(CMD_BUFFER_STRUCT* inCmd);

// Init process
void modemInitProcess(void);

// Reset process, for the US Robotics
void modemResetProcess(void);

// Function: handleMRS - Modem reset.
void handleMRS(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMVS - Modem view settings.
void handleMVS(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMPO - Toggle modem on/off.
void handleMPO(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMMO - Toggle modem mode transmit/receive.
void handleMMO(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMNO - Toggle modem phone number A/B/C.
void handleMNO(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMTO - Toggle modem log on/off.
void handleMTO(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMSD - Modem set default initialization string.
void handleMSD(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMSR - Modem set receive initialization string.
void handleMSR(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMST - Modem set transmit initialization string.
void handleMST(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMSA - Modem set phone number A.
void handleMSA(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMSB - Modem set phone number B.
void handleMSB(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMSC - Modem set phone number C.
void handleMSC(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMVI - Modem view last call in detail.
void handleMVI(CMD_BUFFER_STRUCT* inCmd);

// Function: handleMVO - Modem view last call out detail.
void handleMVO(CMD_BUFFER_STRUCT* inCmd);

uint8 convertAscii2Binary(uint8 firstByte, uint8 secondByte);

#endif // _REMOTE_OPERATION_H_

