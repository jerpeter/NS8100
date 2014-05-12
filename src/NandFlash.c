///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: NandFlash.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:54 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/NandFlash.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Old_Board.h"
#include "Common.h"
#include "Typedefs.h"
#include "NandFlash.h"
#include "Uart.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define NAND_FLASH_ADDRESS			DATA_ADDRESS
#define NAND_TOTAL_BOCKS			4096
#define NAND_TOTAL_PAGES			32
#define NAND_TOTAL_A_BYTES			256
#define NAND_TOTAL_B_BYTES			256
#define NAND_TOTAL_C_BYTES			16
#define NAND_TOTAL_BYTES_IN_PAGE	(NAND_TOTAL_A_BYTES + NAND_TOTAL_B_BYTES + NAND_TOTAL_C_BYTES)

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void writeNandData(uint8 data)
{
	*(uint8*)(NAND_FLASH_ADDRESS + NAND_DATA_ADDRESS) = data;
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void writeNandCommand(uint8 data)
{
	*(uint8*)(NAND_FLASH_ADDRESS + NAND_COMMAND_ADDRESS) = data;
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void writeNandAddress(uint8 data)
{
	*(uint8*)(NAND_FLASH_ADDRESS + NAND_ADDRESS_ADDRESS) = data;
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void writeNandFullAddress(uint32 data)
{
	writeNandAddress((uint8)(data));
	writeNandAddress((uint8)(data >> 9));
	writeNandAddress((uint8)(data >> 17));
	writeNandAddress((uint8)(data >> 25));
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void writeNandAddressParts(uint8 block, uint8 page, uint8 byte);
void writeNandAddressParts(uint8 block, uint8 page, uint8 byte)
{
	writeNandAddress((uint8)(byte));
	writeNandAddress((uint8)((block << 5) | page));
	writeNandAddress((uint8)(block >> 3));
	writeNandAddress((uint8)(block >> 11));
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
uint8 readNandData(void)
{
	uint8 data;

	data = *(uint8*)(NAND_FLASH_ADDRESS + NAND_DATA_ADDRESS);

	return (data);
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
uint8 readNandAddress(uint8 type, uint32 address)
{
	writeNandCommand(type);

	soft_usecWait(50);

	writeNandFullAddress(address);

	//soft_usecWait(12);
	soft_usecWait(50);

	return (readNandData());
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void ReadNandFlash(uint8* dest, uint32 address, uint32 length)
{
	uint32 i = 0;

	for (i=0; i<length; i++)
	{
		*dest = readNandAddress((uint8)(address & 0x10), address);

		dest++;
		address++;
	}
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void WriteNandFlash(uint8* src, uint32 address, uint32 length)
{
	uint32 i = 0;

	for (i=0; i<length; i++)
	{
		pageProgramNand((uint8)(address & 0x10), address, src, 1);

		src++;
		address++;
	}
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
uint8 readNandID(void)
{
	uint8 manfCode;
	uint8 deviceCode;

	writeNandCommand(0x90);
	writeNandFullAddress(0x00);

	soft_usecWait(10);

	manfCode = readNandData();
	deviceCode = readNandData();

	debugPrint(RAW, "Nand Flash Device: Manufacturer Code: 0x%x, Device Code: 0x%x\n", manfCode, deviceCode);

	if (manfCode == 0x20)
		return (PASSED);
	else
		return (FAILED);
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void readNandStatus(void)
{
	uint8 status;

	writeNandCommand(0x70);

	soft_usecWait(1);

	status = readNandData();

	debugPrint(RAW, "Reading Staus register returns data: 0x%x\n  ", status);
	debugPrint(RAW, "Write Protect: %s, Device Status: %s, Last Operation: %s\n  ",
				(status & NAND_WRITE_PROTECTION_DISABLED_BIT) ? "Disabled" : "Enabled",
				(status & NAND_DEVICE_READY_BIT) ? "Ready" : "Busy",
				(status & NAND_OPERATION_FAILED) ? "Failed" : "Successful");
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void waitWhileBusyNand(void)
{
	writeNandCommand(0x70);

	//soft_usecWait(1);
	soft_usecWait(50);

	while ((readNandData() & NAND_DEVICE_READY_BIT) == 0x00)
	{
		// Do nothing
	}
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void issueNandReset(void)
{
	writeNandCommand(0xFF);
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void blockEraseNand(uint32 blockAddress)
{
	writeNandCommand(0x60);

	writeNandAddress((uint8)(blockAddress >> 9));
	writeNandAddress((uint8)(blockAddress >> 17));
	writeNandAddress((uint8)(blockAddress >> 25));

	writeNandCommand(0xD0);
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void pageProgramNand(uint8 type, uint32 address, uint8* data, uint16 length)
{
	uint16 i = 0;

	writeNandCommand(type);

	soft_usecWait(50);

	writeNandCommand(0x80);

	soft_usecWait(50);

	writeNandFullAddress(address);

	soft_usecWait(50);

	for(i = 0; i < length; i++)
	{
		writeNandData(data[i]);

		soft_usecWait(50);
	}

	writeNandCommand(0x10);

	soft_usecWait(500);
}

///----------------------------------------------------------------------------
///	Function:
/// Purpose:
///----------------------------------------------------------------------------
void copyBackNand(uint32 readAddress, uint32 writeAddress)
{
	//uint16 i = 0;

	writeNandCommand(0x00);

	writeNandFullAddress(readAddress);

	writeNandCommand(0x8A);

	writeNandFullAddress(writeAddress);

	writeNandCommand(0x10);
}

