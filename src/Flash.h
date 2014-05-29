///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _EXT_FLASH_H_
#define _EXT_FLASH_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define FLASH_BASE_ADDR		EXT_FLASH_ADDRESS
#define FLASH_OP_SUCCESS 	0
#define FLASH_OP_ERROR 		-1

// =================================================
// New Flash part: Atmel 2M low power flash x16 part
// -------------------------------------------------
#define FLASH_MANF_ID 0x001F		/* Atmel Flash */
#define FLASH_DEVICE_CODE 0x90C3
#define FLASH_DEVICE_CODE_1 0x00C0 	/* AT49BV/LV16X */
#define FLASH_DEVICE_CODE_2 0x00C2 	/* AT49BV/LV16XT */

#define TOTAL_FLASH_SIZE_x16 0x100000
#define TOTAL_FLASH_SIZE_x8  0x200000
#define FLASH_END_ADDR	(FLASH_BASE_ADDR + TOTAL_FLASH_SIZE_x8)

#define TOTAL_FLASH_SECTORS			39
#define TOTAL_FLASH_BOOT_SECTORS	8
#define TOTAL_FLASH_DATA_SECTORS	31

#define FLASH_BOOT_SIZE_x16			0x8000
#define FLASH_BOOT_SIZE_x8			0x10000

#define FLASH_BOOT_SECTOR_SIZE_x16	0x1000
#define FLASH_BOOT_SECTOR_SIZE_x8	0x2000
#define FLASH_SECTOR_SIZE_x16		0x8000
#define FLASH_SECTOR_SIZE_x8		0x10000

#define FLASH_TIMEOUT 200000

/*
	Details: MX29LV640BUTI-90
	=========================
	Size: 1M x 16 (2M x 8)
	Sector structure: 32K x 32 x 16 (64K x 32 x 8)
	Access time: 70ns
	Program time: 20us/word
	Erase time: 300mss/sector, 12s/chip
	Minimum 100,000 erase/program cycles
	20 year data retention
    Functions:
	|- Manf ID & Device ID
	|- Read
	|- Program (Byte/Word)
	|- Sector Erase
	|- Chip Erase
	|- Single Pulse Program
	|- Program Lock
	|- Status
	|- Config

	Sector Addresses: Atmel AT49BV/LV16X
	=====================================
	-- boot sectors --
	0  : 0x000000 -> 0x000FFF
	1  : 0x001000 -> 0x001FFF
		...
	6  : 0x006000 -> 0x006FFF
	7  : 0x007000 -> 0x007FFF
	-- end of boot sectors --

	-- data sectors --
	8  : 0x010000 -> 0x017FFF
	9  : 0x018000 -> 0x01FFFF
		 ...
	30 : 0x0F0000 -> 0x0F7FFF
	31 : 0x0F8000 -> 0x0FFFFF
	-- end of data sectors --
*/

// ====================================================
// Old Flash part: Macronix 8M low power flash x16 part
// ----------------------------------------------------
//#define FLASH_MANF_ID 0xC2 			/* Macronix Flash */
//#define FLASH_DEVICE_CODE 0x22CB 	/* MX29LV640BBTC-90 */

//#define TOTAL_FLASH_SIZE_x16 0x400000
//#define TOTAL_FLASH_SIZE_x8  0x800000
//#define FLASH_END_ADDR	(FLASH_BASE_ADDR + TOTAL_FLASH_SIZE_x8)

//#define TOTAL_FLASH_SECTORS			135
//#define TOTAL_FLASH_BOOT_SECTORS	8
//#define TOTAL_FLASH_DATA_SECTORS	127

//#define FLASH_BOOT_SIZE_x16			0x8000
//#define FLASH_BOOT_SIZE_x8			0x10000

//#define FLASH_BOOT_SECTOR_SIZE_x16	0x1000
//#define FLASH_BOOT_SECTOR_SIZE_x8	0x2000
//#define FLASH_SECTOR_SIZE_x16		0x8000
//#define FLASH_SECTOR_SIZE_x8		0x10000

//#define FLASH_TIMEOUT 200000

/*
	Details: MX29LV640BUTI-90
	=========================
	Size: 4M x 16 (8M x 8)
	Sector structure: 32K x 128 x 16 (64K x 128 x 8)
	Access time: 90ns
	Program time: 11us/word, 45s/chip typical
	Erase time: 0.9s/sector, 45s/chip typical
	Minimum 100,000 erase/program cycles
	20 year data retention
    Functions:
	|- Manf ID & Device ID
	|- Read
	|- Program
	|- Sector Erase
	|- Chip Erase
	|- Acelerated Program
	|- Standby
	|- Output Disable
	|- Reset
	|- Sector Group Select
	|- Chip Unprotect
	|- Temporary Sector Group Unprotect

	Sector Addresses: MX29LV640BUTI-90
	==================================
	-- data sectors --
	0  : 0x000000 -> 0x007fff --\
	1  : 0x008000 -> 0x00ffff ---\__ Sector Group 0, 128K x16 (256K x8)
	2  : 0x010000 -> 0x017fff ---/
	3  : 0x018000 -> 0x01ffff --/
	4  : 0x020000 -> 0x027fff --\
	5  : 0x028000 -> 0x02ffff ---\__ Sector Group 1, 128K x16 (256K x8)
	6  : 0x030000 -> 0x037fff ---/
	7  : 0x038000 -> 0x03ffff --/
		...
	126  : 0x7f0000 -> 0x7f7fff
	127  : 0x7f8000 -> 0x7fffff
	-- end of data sectors --
*/

// ============================================
// Old Flash part: Macronix 4M fash x8/x16 part
// --------------------------------------------
//#define FLASH_MANF_ID 0xC2 		/* Macronix Flash */
//#define FLASH_DEVICE_CODE 0x22A8 	/* MX29LV320BTC-90 */

//#define TOTAL_FLASH_SIZE_x16 0x200000
//#define TOTAL_FLASH_SIZE_x8  0x400000
//#define TOTAL_FLASH_SECTORS	71
//#define TOTAL_FLASH_BOOT_SECTORS	8
//#define TOTAL_FLASH_DATA_SECTORS	63

//#define FLASH_BOOT_SECTOR_SIZE_x16	0x1000
//#define FLASH_BOOT_SECTOR_SIZE_x8		0x2000
//#define FLASH_DATA_SECTOR_SIZE_x16	0x8000
//#define FLASH_DATA_SECTOR_SIZE_x8		0x10000
//#define FLASH_END_ADDR	(FLASH_BASE_ADDR + TOTAL_FLASH_SIZE_x8)

//#define FLASH_TIMEOUT 100000

/*
	Details: MX29LV320BTC-90
	========================
	Size: 4M x 8 or 2M x 16
	Sector structure: 8K x 8 and 64K x 63
	Access time: 90ns
	Program time: 7us typical
	Erase time: 0.9s/sector, 35s/chip typical
    Functions:
	|- Read
	|- Reset
	|- Security Sector
	|- Device ID
	|- Program
	|- Chip Erase
	|- Sector Erase
	|- CFI Query
	|- Erase Suspend/Resume

	Sector Addresses: MX29LV320BTC-90
	=================================
	-- boot sectors --
	0  : 0x000000 -> 0x001fff
	1  : 0x002000 -> 0x003fff
		...
	6  : 0x00c000 -> 0x00dfff
	7  : 0x00e000 -> 0x00ffff
	-- end of boot sectors --

	-- data sectors --
	8  : 0x010000 -> 0x01ffff
	9  : 0x020000 -> 0x02ffff
		 ...
	62 : 0x380000 -> 0x38ffff
	63 : 0x390000 -> 0x39ffff
	-- end of data sectors --
*/

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
int16 ChipErase(void);
int16 SectorErase(uint16* sectorAddr, uint16 numSectors);
int16 FlashWrite(uint16* dest, uint16* src, uint32 length);
int16 ProgramFlashWord(uint16* destAddr, uint16 data);
int16 ProgramFlashByte(uint8* destAddr, uint8 data);
void IssueFlashReset(void);
void waitWhileFlashOperationBusy(void);
int16 VerifyFlashAddr(uint16* addr);
int16 VerifyFlashDevice(uint8 printResults);
int16 FlashCmdCompletePolling(volatile uint16* addr);
int16 FlashDataPolling(volatile uint16* addr, uint16 data);
uint8 VerifyAtmelFlashDevice(void);

#endif // _EXT_FLASH_H_
