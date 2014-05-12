///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: NandFlash.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/NandFlash.h,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

#ifndef _NAND_FLASH_H_
#define _NAND_FLASH_H_ 

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define NAND_WRITE_PROTECTION_DISABLED_BIT	0x80
#define NAND_DEVICE_READY_BIT				0x40
#define NAND_OPERATION_FAILED				0x01

#define NAND_DATA_ADDRESS		0x00
#define NAND_COMMAND_ADDRESS	0x02
#define NAND_ADDRESS_ADDRESS	0x04

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void writeNandData(uint8 data);
void writeNandCommand(uint8 data);
void writeNandAddress(uint8 data);
void writeNandFullAddress(uint32 data);
uint8 readNandData(void);
uint8 readNandAddress(uint8 type, uint32 address);
void ReadNandFlash(uint8* dest, uint32 address, uint32 length);
void WriteNandFlash(uint8* src, uint32 address, uint32 length);
uint8 readNandID(void);
void readNandStatus(void);
void waitWhileBusyNand(void);
void issueNandReset(void);
void blockEraseNand(uint32 blockAddress);
void pageProgramNand(uint8 type, uint32 address, uint8* data, uint16 length);
void copyBackNand(uint32 readAddress, uint32 writeAddress);

#endif // _NAND_FLASH_H_
