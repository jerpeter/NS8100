///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RemoteDisplay.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:58 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/RemoteDisplay.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "RemoteDisplay.h"

//==================================================
// Display commands
//==================================================

//==================================================
// Function: handleVBD
// Description: 
// 		View backlight delay.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSBD
// Description: 
// 		Set backlight delay.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSBD(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVDT
// Description: 
// 		View display timeout.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSDT
// Description: 
// 		Set display timeout.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSDT(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleVCL
// Description: 
// 		View contrast level.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleVCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

//==================================================
// Function: handleSCL
// Description: 
// 		Set contrast level.
// Input: CMD_BUFFER_STRUCT* inCmd
// Return: void
//--------------------------------------------------
void handleSCL(CMD_BUFFER_STRUCT* inCmd)
{
	UNUSED(inCmd);
	return;
}

