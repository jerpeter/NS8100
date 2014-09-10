///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Flash.h"
#include "Uart.h"
#include "Common.h"
#include "EventProcessing.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 ChipErase(void)
{
	//uint16* addr = NULL;
	//uint32 i = 0;
	//uint16* flashPtr = (uint16*)FLASH_BASE_ADDR;

#if 0 // Old command set
	// Issue command to perform a chip erase (Addresses shifted, Processor A1 connected to Flash Device A0)
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00aa;
	addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
	*addr = 0x0055;
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x0080;
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00aa;
	addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
	*addr = 0x0055;
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x0010;

	// Check if flash command did not complete successfully
	if (FlashCmdCompletePolling((volatile uint16*)FLASH_BASE_ADDR)
		!= FLASH_OP_SUCCESS)
	{
		IssueFlashReset();
		debugErr("Flash ChipErase: CmdComplete failed.\n");
		return (FLASH_OP_ERROR);
	}

	// Check if flash data is not correct
	if (FlashDataPolling((volatile uint16*)FLASH_BASE_ADDR, 0xffff) != FLASH_OP_SUCCESS)
	{
		IssueFlashReset();
		debugErr("Flash ChipErase: DataPolling failed.\n");
		return (FLASH_OP_ERROR);
	}

	for (i = 0; i < TOTAL_FLASH_SIZE_x16; i++)
	{
		if (*flashPtr != 0xffff)
			debugErr("Flash ChipErase: Erase failure, address: 0x%x, data: 0x%x\n", flashPtr, *flashPtr);
		flashPtr++;
	}

#else // New command set
	// Check if an Atmel flash
	if(VerifyAtmelFlashDevice() == YES)
	{
		SectorErase((uint16*)FLASH_BASE_ADDR, TOTAL_FLASH_SECTORS);
	}
	else
	{
		SectorErase((uint16*)FLASH_BASE_ADDR, (15 + 1 + 8)); // 15 - 64K, 1 - 32K, 8 - 8K
	}
#endif

	return (FLASH_OP_SUCCESS);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 SectorErase(uint16* sectorAddr, uint16 numSectors)
{
	//uint16* addr = NULL;
	uint32 i = 0;
	uint16* flashPtr = NULL;
	uint32 sectorSize = 0;

	if (numSectors < 1)
	{
		debugWarn("Flash SectorErase: Number of sectors invalid.\n");
		return (FLASH_OP_ERROR);
	}

	while (numSectors--)

	{
		//debug("Flash SectorErase: Sector to erase = 0x%x\n", sectorAddr);

#if 0 // Old command set
		// Issue command to perform a sector erase (Addresses shifted, Processor A1 connected to Flash Device A0)
		addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
		*addr = 0x00aa;
		addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
		*addr = 0x0055;
		addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
		*addr = 0x0080;
		addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
		*addr = 0x00aa;
		addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
		*addr = 0x0055;
		addr = (uint16*)(sectorAddr);
		*addr = 0x0030;

		// Check if flash command did not complete successfully
		if (FlashCmdCompletePolling((volatile uint16*)sectorAddr) != FLASH_OP_SUCCESS)
		{
			IssueFlashReset();
			debugErr("Flash SectorErase: CmdComplete failed.\n");
			return (FLASH_OP_ERROR);
		}

		// Check if flash data is not correct
		if (FlashDataPolling((volatile uint16*)sectorAddr, 0xffff) != FLASH_OP_SUCCESS)
		{
			IssueFlashReset();
			debugErr("Flash SectorErase: DataPolling failed.\n");
			return (FLASH_OP_ERROR);
		}

#else // New command set

		//debug("Erasing at sector address: 0x%x\n", sectorAddr);
		// Clear status register
		*sectorAddr = 0x0050;

		// Issue first command to unlock the sector
		*sectorAddr = 0x0060;
		// Issue second command to the sector to complete
		*sectorAddr = 0x00D0;

		// Clear status register
		*sectorAddr = 0x0050;

		// Issue first command to erase a sector
		*sectorAddr = 0x0020;
		// Issue second command to the sector to complete
		*sectorAddr = 0x00D0;

		//debug("Waiting for flash ready response...\n", sectorAddr);
		// Loop on busy status
		waitWhileFlashOperationBusy();
#endif

		// Check if an Atmel flash
		if(VerifyAtmelFlashDevice() == YES)
		{
			// Check if in the Boot sector range
			if (sectorAddr < (uint16*)(FLASH_BASE_ADDR | FLASH_BOOT_SIZE_x8))
			{
				// Mask off lower address to get base address of the sector
				flashPtr = (uint16*)((uint32)sectorAddr & 0xfffff000);

				// Set the sector size to a boot sector
				sectorSize = FLASH_BOOT_SECTOR_SIZE_x16;
			}
			else // Data sector
			{
				// Mask off lower address to get base address of the sector
				flashPtr = (uint16*)((uint32)sectorAddr & 0xffff0000);

				// Set the sector size to a boot sector
				sectorSize = FLASH_SECTOR_SIZE_x16;
			}
		}
		else // LFH part
		{
			// Check if in the Boot sector range
			if (sectorAddr >= (uint16*)(0x809F0000))
			{
				// Mask off lower address to get base address of the sector
				flashPtr = (uint16*)((uint32)sectorAddr & 0xfffff000);

				// Set the sector size to a boot sector
				sectorSize = 0x1000;
			}
			else if (sectorAddr >= (uint16*)(0x809E8000))
			{
				// Mask off lower address to get base address of the sector
				flashPtr = (uint16*)((uint32)sectorAddr & 0xffff0000);

				// Set the sector size to a boot sector
				sectorSize = 0x8000;
			}
			else // Data sector
			{
				// Mask off lower address to get base address of the sector
				flashPtr = (uint16*)((uint32)sectorAddr & 0xffff0000);

				// Set the sector size to a boot sector
				sectorSize = 0x10000;
			}
		}

		//debug("Verifying data integrity...\n", sectorAddr);
		// Loop through the entire sector
		for (i = 0; i < sectorSize; i++)
		{
			// Check if a flash cell location is not erased
			if (*flashPtr != 0xffff)
			{
				debugErr("Flash SectorErase: Erase failure, address: 0x%x, data: 0x%x\n", flashPtr, *flashPtr);
				debugErr("Skipping rest of sector\n");
				break;
			}

			flashPtr++;
		}

		// If multiple sectors were specified, increment sector address by sector size
		sectorAddr += sectorSize;
	}

	return (FLASH_OP_SUCCESS);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int16 FlashWrite(uint16* dest, uint16* src, uint32 length)
{
#if 0 // ns7100
	uint32 x;

	for (x = 0; x < length; x++)
	{
		if (ProgramFlashWord((uint16*)(dest + x), src[x]) == FLASH_OP_ERROR)
			return (FLASH_OP_ERROR);
	}
#else // Remove warnings
	UNUSED(dest);
	UNUSED(src);
	UNUSED(length);
#endif

#if 0 // ns8100
extern FL_FILE* g_currentEventFileHandle;
	fl_fwrite(src, length, 1, g_currentEventFileHandle);
#endif

	return (FLASH_OP_SUCCESS);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 ProgramFlashWord(uint16* destAddr, uint16 data)
{
	//uint16* addr = NULL;
	uint16 currData = *destAddr;

	// Check if addr is not in valid range
	if (VerifyFlashAddr(destAddr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash ProgramFlashWord: Addr check failed, address: 0x%x is out of range\n", destAddr);
		return (FLASH_OP_ERROR);
	}

	// Check if trying to write/toggle any bits to 1's when the stored bits are 0
	if (((~currData) & data) != 0)
	{
		debugErr("Flash ProgramFlashWord: Data check failed, address: 0x%x, current data: 0x%x, new data: 0x%x\n",
			destAddr, currData, data);
		return (FLASH_OP_ERROR);
	}

#if 0 // Old command set
	// Issue command to program a cell (Addresses shifted, Processor A1 connected to Flash Device A0)
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00aa;
	addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
	*addr = 0x0055;
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00a0;
	addr = (uint16*)(destAddr);
	*addr = data;

	// Check if flash data is not correct
	if (FlashDataPolling((volatile uint16*)destAddr, data) != FLASH_OP_SUCCESS)
	{
		IssueFlashReset();
		debugErr("Flash ProgramFlashWord: DataPolling failed, address: 0x%x, data: 0x%x (data before write: 0x%x).\n",
			destAddr, data, currData);
		return (FLASH_OP_ERROR);
	}

#else // New command set
	// Clear status register
	*destAddr = 0x0050;

	// Issue first command to unlock the sector
	*destAddr = 0x0060;
	// Issue second command to the sector to complete
	*destAddr = 0x00D0;

	// Clear status register
	*destAddr = 0x0050;

	// Issue command to program a word
	*destAddr = 0x0040;
	// Issue data to location
	*destAddr = data;

	waitWhileFlashOperationBusy();
#endif

	return (FLASH_OP_SUCCESS);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 ProgramFlashByte(uint8* destAddr, uint8 data)
{
	//uint16* addr = NULL;
	uint16* wordAddr;
	uint16 wordData;
	uint16 checkWord;

	// Check if addr is not in valid range
	if (VerifyFlashAddr((uint16*)destAddr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash ProgramFlashWord: Addr check failed, address: 0x%x is out of range\n", destAddr);
		return (FLASH_OP_ERROR);
	}

	// Address must be even, so mask off odd address bit
	wordAddr = (uint16*)((uint32)destAddr & 0xFFFFFFFE);
	checkWord = *wordAddr;

	if ((uint32)destAddr & 0x00000001) // If addr is odd, check the low byte
	{
		if ((checkWord & 0x00ff) != 0x00ff)
		{
			debugErr("Flash ProgramFlashByte: Data check failed, address: 0x%x, data in flash: 0x%x\n",
				destAddr, (uint8)(checkWord & 0x00ff));
			return (FLASH_OP_ERROR);
		}

		// Write in current high byte data into the word data to write
		wordData = (uint16)(checkWord & 0xff00);
		// Add in the data to write
		wordData |= (uint16)data;
	}
	else // Addr is even, check the high byte
	{
		if ((checkWord & 0xff00) != 0xff00)
		{
			debugErr("Flash ProgramFlashByte: Data check failed, address: 0x%x, data in flash: 0x%x\n",
				destAddr, (uint8)(checkWord >> 8));
			return (FLASH_OP_ERROR);
		}

		// Write in current high byte data into the word data to write
		wordData = (uint16)(checkWord & 0x00ff);
		// Add in the data to write
		wordData |= (uint16)(data << 8);
	}

#if 0
	// Issue command to program a cell (Addresses shifted, Processor A1 connected to Flash Device A0)
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00aa;
	addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
	*addr = 0x0055;
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00a0;
	addr = (uint16*)(wordAddr);
	*addr = wordData;

	// Check if flash data is not correct
	if (FlashDataPolling((volatile uint16*)wordAddr, wordData) != FLASH_OP_SUCCESS)
	{
		IssueFlashReset();
		debugErr("Flash ProgramFlashByte: DataPolling failed, address: 0x%x, data: 0x%x (data before write: 0x%x).\n",
			wordAddr, wordData, checkWord);
		return (FLASH_OP_ERROR);
	}

#else
	// Clear status register
	*destAddr = 0x0050;

	// Issue first command to unlock the sector
	*destAddr = 0x0060;
	// Issue second command to the sector to complete
	*destAddr = 0x00D0;

	// Clear status register
	*wordAddr = 0x0050;

	// Issue command to program a word
	*wordAddr = 0x0040;
	// Issue data to location
	*wordAddr = data;

	waitWhileFlashOperationBusy();
#endif

	return (FLASH_OP_SUCCESS);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
void IssueFlashReset(void)
{
	uint16* addr;

	addr = (uint16*)(FLASH_BASE_ADDR);
	*addr = 0x00F0;
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
void waitWhileFlashOperationBusy(void)
{
	volatile uint16* addr = (volatile uint16*)FLASH_BASE_ADDR;

	// Write 70 to any location for status register select
	*addr = 0x0070;

	// Check if ready bit is set
	while (((volatile uint8)*addr & 0x80) == 0x00) {}

	// Clear the status register
	*addr = 0x0050;

	// Return to read state
	*addr = 0x00FF;
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 FlashCmdCompletePolling(volatile uint16* addr)
{
	uint16 savedVal;
	uint16 currentVal;
	uint32 counter = 0;

	// Check if addr is not in valid range
	if (VerifyFlashAddr((uint16*)addr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash ProgramFlashWord: Addr check failed, address: 0x%x is out of range\n", addr);
		return (FLASH_OP_ERROR);
	}

	savedVal = *addr;
	currentVal = *addr;

	// Continue while bit 6 is toggling
	while (((savedVal & 0x0040) != (currentVal & 0x0040)) && (counter < FLASH_TIMEOUT))
	{
		// check if bit 5 is high (1)
		if (currentVal & 0x0020)
		{
			savedVal = *addr;
			currentVal = *addr;

			// Check if not still toggling
			if ((savedVal & 0x0040) != (currentVal & 0x0040))
			{
				debugErr("Flash CmdComplete: Erase command failed\n");
				return (FLASH_OP_ERROR);
			}
			else
				return (FLASH_OP_SUCCESS);
		}

		// Small delay since erase operations are on the order of seconds to complete
		SoftUsecWait(1000);

		savedVal = currentVal;
		currentVal = *addr;

		counter++;
	}

	// Check if we maxed the counter while implies we timed out
	if (counter >= FLASH_TIMEOUT)
	{
		debugErr("Flash CmdComplete: Timed out\n");
		return (FLASH_OP_ERROR);
	}

	return (FLASH_OP_SUCCESS);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 FlashDataPolling(volatile uint16* addr, uint16 data)
{
	int32 counter = 0;
	int32 currentVal = 0;
	uint16 flashVal;

	// Check if addr is not in valid range
	if (VerifyFlashAddr((uint16*)addr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash ProgramFlashWord: Addr check failed, address: 0x%x is out of range\n", addr);
		return (FLASH_OP_ERROR);
	}

	flashVal = *addr;

	// Check if bit 7 is not the same
	while (((flashVal & 0x0080) != (data & 0x0080)) && (counter < FLASH_TIMEOUT))
	{
		// Check if bit 5 is high (1)
		if (currentVal & 0x0020)
		{
			flashVal = *addr;

			// Check if bit 7 is not the same
			if ((flashVal & 0x0080) != (data & 0x0080))
				return (FLASH_OP_ERROR);
			else
				return (FLASH_OP_SUCCESS);
		}

		flashVal = *addr;
		counter++;
	}

	// Check if we maxed the counter while implies we timed out
	if (counter >= FLASH_TIMEOUT)
		return (FLASH_OP_ERROR);

	return (FLASH_OP_SUCCESS);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 VerifyFlashAddr(uint16* addr)
{
	if (((uint32)addr >= FLASH_BASE_ADDR) &&
		((uint32)addr < FLASH_END_ADDR))
		return (FLASH_OP_SUCCESS);
	else
		return (FLASH_OP_ERROR);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
int16 VerifyFlashDevice(uint8 printResults)
{
	volatile uint16* addr = (uint16*)FLASH_BASE_ADDR;
	uint16 manfId;
	uint16 deviceCode;
	//uint16 protection;
	//uint16 i;

#if 0 // Old command set
	// Issue command to enter Product Id mode (Addresses shifted, Processor A1 connected to Flash Device A0)
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x00aa;
	addr = (uint16*)(FLASH_BASE_ADDR | 0x554);
	*addr = 0x0055;
	addr = (uint16*)(FLASH_BASE_ADDR | 0xAAA);
	*addr = 0x0090;
#else // New command set
	// Clear status register
	*addr = 0x0050;

	// Issue command to enter product id mode
	*addr = 0x0090;
#endif

	// Make address line a zero to select the Manufacturer Id
	addr = (uint16*)(FLASH_BASE_ADDR);
	// Read Manufacturer ID
	manfId = *addr;

	// Make the lowest address line a one to select the Device Id
	addr = (uint16*)(FLASH_BASE_ADDR | 0x02);
	// Read Device Code
	deviceCode = *addr;

	//debugRaw("Flash Device: Manufacturer ID: 0x%x, Device Code: 0x%x\n", manfId, deviceCode);

#if 0
	addr = (uint16*)(FLASH_BASE_ADDR + 0x04);
	for (i = 0; i < TOTAL_FLASH_SECTORS; i++)
	{
		protection = *addr;

		//debugRaw("  Sector %d at (%p) has protection bits: 0x%x\n", i, addr, protection);

		// Check if in the Boot sector range
		if (addr < (uint16*)(FLASH_BASE_ADDR | FLASH_BOOT_SIZE_x8))
		{
			addr += FLASH_BOOT_SECTOR_SIZE_x16;
		}
		else // Data sector
		{
			addr += FLASH_SECTOR_SIZE_x16;
		}
	}
#endif

	// Take the device out of Product Id mode
	addr = (uint16*)(FLASH_BASE_ADDR);
	*addr = 0x00F0;

	if(printResults == YES)
	{
		debugRaw("  Flash Manf ID: 0x%x, Device ID: 0x%x\n", manfId, deviceCode);
	}

	if ((manfId == FLASH_MANF_ID) && ((deviceCode == FLASH_DEVICE_CODE) ||
		(deviceCode == FLASH_DEVICE_CODE_1) || (deviceCode == FLASH_DEVICE_CODE_2)))
		return (FLASH_OP_SUCCESS);
	else
		return (FLASH_OP_ERROR);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0 // ns7100
uint8 VerifyAtmelFlashDevice(void)
{
	volatile uint16* addr = (uint16*)FLASH_BASE_ADDR;
	uint8 atmelFlashPart = NO;

	// Clear status register
	*addr = 0x0050;

	// Issue command to enter product id mode
	*addr = 0x0090;

	// Read Manf Id
	if(*addr == FLASH_MANF_ID)
	{
		atmelFlashPart = YES;
	}

	// Take the device out of Product Id mode
	*addr = 0x00F0;

	return (atmelFlashPart);
}
#endif

