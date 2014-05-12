///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Flash.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:48 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Flash.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Old_Board.h"
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

/*==================================================
 * Procedure: chipErase()
 * Description: Erase the entire flash chip
 * Input: None
 * Output: Status
 */
#if 0 // ns7100
int16 chipErase(void)
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
	if (flashCmdCompletePolling((volatile uint16*)FLASH_BASE_ADDR)
		!= FLASH_OP_SUCCESS)
	{
		issueReset();
		debugErr("Flash chipErase: CmdComplete failed.\n");
		return (FLASH_OP_ERROR);
	}

	// Check if flash data is not correct
	if (flashDataPolling((volatile uint16*)FLASH_BASE_ADDR, 0xffff) != FLASH_OP_SUCCESS)
	{
		issueReset();
		debugErr("Flash chipErase: DataPolling failed.\n");
		return (FLASH_OP_ERROR);
	}

	for (i = 0; i < TOTAL_FLASH_SIZE_x16; i++)
	{
		if (*flashPtr != 0xffff)
			debugErr("Flash chipErase: Erase failure, address: 0x%x, data: 0x%x\n", flashPtr, *flashPtr);
		flashPtr++;
	}

#else // New command set
	// Check if an Atmel flash
	if(verifyAtmelFlashDevice() == YES)
	{
		sectorErase((uint16*)FLASH_BASE_ADDR, TOTAL_FLASH_SECTORS);
	}
	else
	{
		sectorErase((uint16*)FLASH_BASE_ADDR, (15 + 1 + 8)); // 15 - 64K, 1 - 32K, 8 - 8K
	}
#endif

	return (FLASH_OP_SUCCESS);
}
#endif

/*==================================================
 * Procedure: sectorErase()
 * Description: Erase a sector on the flash chip.
 * Input: Sector number
 * Output: None
 */
#if 0 // ns7100
int16 sectorErase(uint16* sectorAddr, uint16 numSectors)
{
	//uint16* addr = NULL;
	uint32 i = 0;
	uint16* flashPtr = NULL;
	uint32 sectorSize = 0;

	if (numSectors < 1)
	{
		debugWarn("Flash sectorErase: Number of sectors invalid.\n");
		return (FLASH_OP_ERROR);
	}

	while (numSectors--)

	{
		//debug("Flash sectorErase: Sector to erase = 0x%x\n", sectorAddr);

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
		if (flashCmdCompletePolling((volatile uint16*)sectorAddr) != FLASH_OP_SUCCESS)
		{
			issueReset();
			debugErr("Flash sectorErase: CmdComplete failed.\n");
			return (FLASH_OP_ERROR);
		}

		// Check if flash data is not correct
		if (flashDataPolling((volatile uint16*)sectorAddr, 0xffff) != FLASH_OP_SUCCESS)
		{
			issueReset();
			debugErr("Flash sectorErase: DataPolling failed.\n");
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
		if(verifyAtmelFlashDevice() == YES)
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
				debugErr("Flash sectorErase: Erase failure, address: 0x%x, data: 0x%x\n", flashPtr, *flashPtr);
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

/*******************************************************************************
*	Function:	flashWrite
*	Purpose: Original routine, with the guts replace by the new flash function
*******************************************************************************/
int16 flashWrite(uint16* dest, uint16* src, uint32 length)
{
#if 0 // ns7100
	uint32 x;

	for (x = 0; x < length; x++)
	{
		if (programWord((uint16*)(dest + x), src[x]) == FLASH_OP_ERROR)
			return (FLASH_OP_ERROR);
	}
#endif

#if 0 // ns8100
extern FL_FILE* g_currentEventFileHandle;
	fl_fwrite(src, length, 1, g_currentEventFileHandle);
#endif

	return (FLASH_OP_SUCCESS);
}

/*==================================================
 * Procedure: programWord()
 * Description: Program one word in flash
 * Input: Program address, word data
 * Output: None
 */
#if 0 // ns7100
int16 programWord(uint16* destAddr, uint16 data)
{
	//uint16* addr = NULL;
	uint16 currData = *destAddr;

	// Check if addr is not in valid range
	if (verifyFlashAddr(destAddr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash programWord: Addr check failed, address: 0x%x is out of range\n", destAddr);
		return (FLASH_OP_ERROR);
	}

	// Check if trying to write/toggle any bits to 1's when the stored bits are 0
	if (((~currData) & data) != 0)
	{
		debugErr("Flash programWord: Data check failed, address: 0x%x, current data: 0x%x, new data: 0x%x\n",
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
	if (flashDataPolling((volatile uint16*)destAddr, data) != FLASH_OP_SUCCESS)
	{
		issueReset();
		debugErr("Flash programWord: DataPolling failed, address: 0x%x, data: 0x%x (data before write: 0x%x).\n",
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

/*==================================================
 * Procedure: programByte()
 * Description: Program one byte in flash to retrofit old code
 * Input: Program address, byte data
 * Output: Status
 */
#if 0 // ns7100
int16 programByte(uint8* destAddr, uint8 data)
{
	//uint16* addr = NULL;
	uint16* wordAddr;
	uint16 wordData;
	uint16 checkWord;

	// Check if addr is not in valid range
	if (verifyFlashAddr((uint16*)destAddr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash programWord: Addr check failed, address: 0x%x is out of range\n", destAddr);
		return (FLASH_OP_ERROR);
	}

	// Address must be even, so mask off odd address bit
	wordAddr = (uint16*)((uint32)destAddr & 0xFFFFFFFE);
	checkWord = *wordAddr;

	if ((uint32)destAddr & 0x00000001) // If addr is odd, check the low byte
	{
		if ((checkWord & 0x00ff) != 0x00ff)
		{
			debugErr("Flash programByte: Data check failed, address: 0x%x, data in flash: 0x%x\n",
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
			debugErr("Flash programByte: Data check failed, address: 0x%x, data in flash: 0x%x\n",
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
	if (flashDataPolling((volatile uint16*)wordAddr, wordData) != FLASH_OP_SUCCESS)
	{
		issueReset();
		debugErr("Flash programByte: DataPolling failed, address: 0x%x, data: 0x%x (data before write: 0x%x).\n",
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

/*==================================================
 * Procedure: issueReset()
 * Description: Issue the reset command to the flash device
 * Input: None
 * Output: None
 */
#if 0 // ns7100
void issueReset(void)
{
	uint16* addr;

	addr = (uint16*)(FLASH_BASE_ADDR);
	*addr = 0x00F0;
}
#endif

/*==================================================
 * Procedure: waitWhileFlashOperationBusy()
 * Description:
 * Input: None
 * Output: None
 */
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

/*==================================================
 * Procedure: flashCmdCompletePolling()
 * Description: Check to make sure the flash command has completed
 * Input: Addr (to chip, or sector, or specific addr)
 * Output: Status
 */
#if 0 // ns7100
int16 flashCmdCompletePolling(volatile uint16* addr)
{
	uint16 savedVal;
	uint16 currentVal;
	uint32 counter = 0;

	// Check if addr is not in valid range
	if (verifyFlashAddr((uint16*)addr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash programWord: Addr check failed, address: 0x%x is out of range\n", addr);
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
		soft_usecWait(1000);

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

/*==================================================
 * Procedure: flashDataPolling()
 * Description: Check to make sure the data had been written to the flash successfully
 * Input: Addr, Data
 * Output: Status
 */
#if 0 // ns7100
int16 flashDataPolling(volatile uint16* addr, uint16 data)
{
	int32 counter = 0;
	int32 currentVal = 0;
	uint16 flashVal;

	// Check if addr is not in valid range
	if (verifyFlashAddr((uint16*)addr) != FLASH_OP_SUCCESS)
	{
		debugErr("Flash programWord: Addr check failed, address: 0x%x is out of range\n", addr);
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

/*==================================================
 * Procedure: verifyFlashAddr()
 * Description: Check to make sure the flash address is valid
 * Input: Addr
 * Output: Status
 */
#if 0 // ns7100
int16 verifyFlashAddr(uint16* addr)
{
	if (((uint32)addr >= FLASH_BASE_ADDR) &&
		((uint32)addr < FLASH_END_ADDR))
		return (FLASH_OP_SUCCESS);
	else
		return (FLASH_OP_ERROR);
}
#endif

/*==================================================
 * Procedure: verifyFlashDevice()
 * Description: Verify the flash device is a Macronix with the appropriate type
 * Input: None
 * Output: Status
 */
#if 0 // ns7100
int16 verifyFlashDevice(uint8 printResults)
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

	//debugPrint(RAW, "Flash Device: Manufacturer ID: 0x%x, Device Code: 0x%x\n", manfId, deviceCode);

#if 0
	addr = (uint16*)(FLASH_BASE_ADDR + 0x04);
	for (i = 0; i < TOTAL_FLASH_SECTORS; i++)
	{
		protection = *addr;

		//debugPrint(RAW, "  Sector %d at (%p) has protection bits: 0x%x\n", i, addr, protection);

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
		debugPrint(RAW, "  Flash Manf ID: 0x%x, Device ID: 0x%x\n", manfId, deviceCode);
	}

	if ((manfId == FLASH_MANF_ID) && ((deviceCode == FLASH_DEVICE_CODE) ||
		(deviceCode == FLASH_DEVICE_CODE_1) || (deviceCode == FLASH_DEVICE_CODE_2)))
		return (FLASH_OP_SUCCESS);
	else
		return (FLASH_OP_ERROR);
}
#endif

/*==================================================
 * Procedure: verifyAtmelFlashDevice()
 * Description: Verify the flash device is a Macronix with the appropriate type
 * Input: None
 * Output: Status
 */
#if 0 // ns7100
uint8 verifyAtmelFlashDevice(void)
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

