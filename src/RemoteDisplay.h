///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteDisplay.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:58 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteDisplay.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _REMOTE_DISPLAY_H_
#define _REMOTE_DISPLAY_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteCommon.h"
                            
///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
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
 
#endif // _REMOTE_DISPLAY_H_

