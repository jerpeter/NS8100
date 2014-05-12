///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteModem.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/RemoteModem.h,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

#ifndef _REMOTE_MODEM_H_
#define _REMOTE_MODEM_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteCommon.h"

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
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


#endif // _REMOTE_MODEM_H_

