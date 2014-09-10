///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Common.h"
#include "Typedefs.h"
#include "NandFlash.h"
#include "Uart.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#if 0 // ns7100
#define NAND_FLASH_ADDRESS			DATA_ADDRESS
#else
#define NAND_FLASH_ADDRESS			0x0
#endif
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
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteNandData(uint8 data)
{
	*(uint8*)(NAND_FLASH_ADDRESS + NAND_DATA_ADDRESS) = data;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteNandCommand(uint8 data)
{
	*(uint8*)(NAND_FLASH_ADDRESS + NAND_COMMAND_ADDRESS) = data;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteNandAddress(uint8 data)
{
	*(uint8*)(NAND_FLASH_ADDRESS + NAND_ADDRESS_ADDRESS) = data;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteNandFullAddress(uint32 data)
{
	WriteNandAddress((uint8)(data));
	WriteNandAddress((uint8)(data >> 9));
	WriteNandAddress((uint8)(data >> 17));
	WriteNandAddress((uint8)(data >> 25));
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteNandAddressParts(uint8 block, uint8 page, uint8 byte);
void WriteNandAddressParts(uint8 block, uint8 page, uint8 byte)
{
	WriteNandAddress((uint8)(byte));
	WriteNandAddress((uint8)((block << 5) | page));
	WriteNandAddress((uint8)(block >> 3));
	WriteNandAddress((uint8)(block >> 11));
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ReadNandData(void)
{
	uint8 data;

	data = *(uint8*)(NAND_FLASH_ADDRESS + NAND_DATA_ADDRESS);

	return (data);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ReadNandAddress(uint8 type, uint32 address)
{
	WriteNandCommand(type);

	SoftUsecWait(50);

	WriteNandFullAddress(address);

	//SoftUsecWait(12);
	SoftUsecWait(50);

	return (ReadNandData());
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ReadNandFlash(uint8* dest, uint32 address, uint32 length)
{
	uint32 i = 0;

	for (i=0; i<length; i++)
	{
		*dest = ReadNandAddress((uint8)(address & 0x10), address);

		dest++;
		address++;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteNandFlash(uint8* src, uint32 address, uint32 length)
{
	uint32 i = 0;

	for (i=0; i<length; i++)
	{
		PageProgramNand((uint8)(address & 0x10), address, src, 1);

		src++;
		address++;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ReadNandID(void)
{
	uint8 manfCode;
	uint8 deviceCode;

	WriteNandCommand(0x90);
	WriteNandFullAddress(0x00);

	SoftUsecWait(10);

	manfCode = ReadNandData();
	deviceCode = ReadNandData();

	debugRaw("Nand Flash Device: Manufacturer Code: 0x%x, Device Code: 0x%x\n", manfCode, deviceCode);

	if (manfCode == 0x20)
		return (PASSED);
	else
		return (FAILED);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ReadNandStatus(void)
{
	uint8 status;

	WriteNandCommand(0x70);

	SoftUsecWait(1);

	status = ReadNandData();

	debugRaw("Reading Staus register returns data: 0x%x\n  ", status);
	debugRaw("Write Protect: %s, Device Status: %s, Last Operation: %s\n  ",
				(status & NAND_WRITE_PROTECTION_DISABLED_BIT) ? "Disabled" : "Enabled",
				(status & NAND_DEVICE_READY_BIT) ? "Ready" : "Busy",
				(status & NAND_OPERATION_FAILED) ? "Failed" : "Successful");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void waitWhileBusyNand(void)
{
	WriteNandCommand(0x70);

	//SoftUsecWait(1);
	SoftUsecWait(50);

	while ((ReadNandData() & NAND_DEVICE_READY_BIT) == 0x00)
	{
		// Do nothing
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void IssueNandReset(void)
{
	WriteNandCommand(0xFF);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void BlockEraseNand(uint32 blockAddress)
{
	WriteNandCommand(0x60);

	WriteNandAddress((uint8)(blockAddress >> 9));
	WriteNandAddress((uint8)(blockAddress >> 17));
	WriteNandAddress((uint8)(blockAddress >> 25));

	WriteNandCommand(0xD0);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PageProgramNand(uint8 type, uint32 address, uint8* data, uint16 length)
{
	uint16 i = 0;

	WriteNandCommand(type);

	SoftUsecWait(50);

	WriteNandCommand(0x80);

	SoftUsecWait(50);

	WriteNandFullAddress(address);

	SoftUsecWait(50);

	for(i = 0; i < length; i++)
	{
		WriteNandData(data[i]);

		SoftUsecWait(50);
	}

	WriteNandCommand(0x10);

	SoftUsecWait(500);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CopyBackNand(uint32 readAddress, uint32 writeAddress)
{
	//uint16 i = 0;

	WriteNandCommand(0x00);

	WriteNandFullAddress(readAddress);

	WriteNandCommand(0x8A);

	WriteNandFullAddress(writeAddress);

	WriteNandCommand(0x10);
}

