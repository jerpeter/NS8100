///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
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
void WriteNandData(uint8 data);
void WriteNandCommand(uint8 data);
void WriteNandAddress(uint8 data);
void WriteNandFullAddress(uint32 data);
uint8 ReadNandData(void);
uint8 ReadNandAddress(uint8 type, uint32 address);
void ReadNandFlash(uint8* dest, uint32 address, uint32 length);
void WriteNandFlash(uint8* src, uint32 address, uint32 length);
uint8 ReadNandID(void);
void ReadNandStatus(void);
void waitWhileBusyNand(void);
void IssueNandReset(void);
void BlockEraseNand(uint32 blockAddress);
void PageProgramNand(uint8 type, uint32 address, uint8* data, uint16 length);
void CopyBackNand(uint32 readAddress, uint32 writeAddress);

#endif // _NAND_FLASH_H_
