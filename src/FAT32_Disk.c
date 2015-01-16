#if 0
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//					        FAT32 File IO Library
//								    V2.0
// 	  							 Rob Riglar
//						    Copyright 2003 - 2007
//
//   					  Email: rob@robriglar.com
//
//-----------------------------------------------------------------------------
//
// This file is part of FAT32 File IO Library.
//
// FAT32 File IO Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// FAT32 File IO Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FAT32 File IO Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include "FAT32_Disk.h"
#include "sd_mmc_spi.h"

#ifdef SOURCE_WINDOWS_PHYSICAL_DRIVE
	extern BOOL ReadDiskSector(int drive, unsigned long startinglogicalsector, int numberofsectors, BYTE *buffer);
	extern BOOL WriteDiskSector(int drive, unsigned long startinglogicalsector, int numberofsectors, BYTE *buffer);
#endif

//-----------------------------------------------------------------------------
// FAT32_InitDrive: Initialise the choosen source
//-----------------------------------------------------------------------------
BOOL FAT32_InitDrive()
{
// File as disk drive
#ifdef SOURCE_MOUNT_FILE_AS_DRIVE

	return TRUE;

#endif

// Windows physical drive
#ifdef SOURCE_WINDOWS_PHYSICAL_DRIVE

	return TRUE;

#endif

// Hardware IDE driver
#ifdef SOURCE_IDE_DRIVER

	return TRUE;

#endif
}
//-----------------------------------------------------------------------------
// FAT_ReadSector: Read a sector from disk
//-----------------------------------------------------------------------------
BOOL FAT_ReadSector(UINT32 sector, BYTE *buffer)
{
// File as disk drive
#ifdef SOURCE_MOUNT_FILE_AS_DRIVE

	FILE *f = fopen(DISK_MOUNT_FILE, "rb");
	if (f!=NULL)
	{
		fseek(f , (sector*512) , SEEK_SET );
		fread(buffer,1,512,f);
		fclose(f);
		return TRUE;
	}
	else
		return FALSE;

#endif

// Windows physical drive
#ifdef SOURCE_WINDOWS_PHYSICAL_DRIVE

	return ReadDiskSector(DISK_ID, sector, 1, buffer);

#endif

// Hardware IDE driver
#ifdef SOURCE_IDE_DRIVER

	// Include sector reading code for your storage device
	sd_mmc_spi_read_open( sector );
	sd_mmc_spi_read_sector_to_ram( buffer );
    return TRUE;

#endif

	return FALSE;
}
//-----------------------------------------------------------------------------
// FAT_WriteSector: Write a sector to disk
//-----------------------------------------------------------------------------
BOOL FAT_WriteSector(UINT32 sector, BYTE *buffer)
{
// File as disk drive
#ifdef SOURCE_MOUNT_FILE_AS_DRIVE
	FILE *f = fopen(DISK_MOUNT_FILE, "rb+");
	if (f!=NULL)
	{
		fseek(f , (sector*512) , SEEK_SET );
		fwrite (buffer , 1 , 512, f );
		fclose(f);
		return TRUE;
	}
	else
		return FALSE;
#endif

// Windows physical drive
#ifdef SOURCE_WINDOWS_PHYSICAL_DRIVE

	return WriteDiskSector(DISK_ID, sector, 1, buffer);

#endif

// Hardware IDE driver
#ifdef SOURCE_IDE_DRIVER

	// Include sector writing code for your storage device
	sd_mmc_spi_write_open( sector );
	sd_mmc_spi_write_sector_from_ram( buffer );
    return TRUE;

#endif

	return FALSE;
}
#endif