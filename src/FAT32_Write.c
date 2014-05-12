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
#include "FAT32_Definitions.h"
#include "FAT32_Base.h"
#include "FAT32_Table.h"
#include "FAT32_Access.h"
#include "FAT32_Write.h"
#include "FAT32_Filestring.h"
#include "FAT32_Misc.h"

#ifdef INCLUDE_WRITE_SUPPORT
//-----------------------------------------------------------------------------
// FAT32_AddFreeSpaceToChain: Allocate another cluster of free space to the end
// of a files cluster chain.
//-----------------------------------------------------------------------------
BOOL FAT32_AddFreeSpaceToChain(UINT32 *startCluster)
{
	UINT32 nextcluster;

	// Set the next free cluster hint to unknown
	FAT32_SetFsInfoNextCluster(0xFFFFFFFF); 

	// Start looking for free clusters from the beginning
	if (FAT32_FindBlankCluster(2, &nextcluster))
	{
		// Point last to this
		FAT32_SetClusterValue(*startCluster, nextcluster);
		
		// Point this to end of file
		FAT32_SetClusterValue(nextcluster, FAT32_EOC_FLAG);

		// Adjust argument reference
		*startCluster = nextcluster;

		return TRUE;
	}
	else
		return FALSE;
}
//-----------------------------------------------------------------------------
// FAT32_AllocateFreeSpace: Add an ammount of free space to a file either from
// 'startCluster' if newFile = false, or allocating a new start to the chain if
// newFile = true.
//-----------------------------------------------------------------------------
BOOL FAT32_AllocateFreeSpace(BOOL newFile, UINT32 *startCluster, UINT32 size)
{
	UINT32 clusterSize;
	UINT32 clusterCount;
	UINT32 nextcluster;

	if (size==0)
		return FALSE;

	// Set the next free cluster hint to unknown
	FAT32_SetFsInfoNextCluster(0xFFFFFFFF); 

	// Work out size and clusters
	clusterSize = FAT32.SectorsPerCluster * 512;
	clusterCount = (size / clusterSize);

	// If any left over
	if (size-(clusterSize*clusterCount))
		clusterCount++;

	// Allocated first link in the chain if a new file
	if (newFile)
	{
		if (!FAT32_FindBlankCluster(2, &nextcluster))
			return FALSE;

		// If this is all that is needed then all done
		if (clusterCount==1)
		{
			FAT32_SetClusterValue(nextcluster, FAT32_EOC_FLAG);
			*startCluster = nextcluster;
			return TRUE;
		}
	}
	// Allocate from end of current chain (startCluster is end of chain)
	else
		nextcluster = *startCluster;

	while (clusterCount)
	{
		if (!FAT32_AddFreeSpaceToChain(&nextcluster))
			return FALSE;

		clusterCount--;
	}

	return TRUE;
}
//-----------------------------------------------------------------------------
// FAT32_FindFreeOffset: Find a free space in the directory for a new entry 
// which takes up 'entryCount' blocks (or allocate some more)
//-----------------------------------------------------------------------------
BOOL FAT32_FindFreeOffset(UINT32 dirCluster, int entryCount, UINT32 *pSector, BYTE *pOffset)
{
	BYTE item=0;
	UINT16 recordoffset = 0;
	int currentCount = entryCount;
	BYTE i=0;
	int x=0;

	BOOL firstFound = FALSE;

	if (entryCount==0)
		return FALSE;

	// Main cluster following loop
	while (TRUE)
	{
		// Read sector
		if (FAT32_SectorReader(dirCluster, x++)) 
		{
			// Analyse Sector
			for (item=0; item<=15;item++)
			{
				// Create the multiplier for sector access
				recordoffset = (32*item);

				// If looking for the last used directory entry
				if (firstFound==FALSE)
				{
					if (FATFS_Internal.currentsector[recordoffset]==0x00)
					{
						firstFound = TRUE;

						// Store start
						*pSector = x-1;
						*pOffset = item;

						currentCount--;
					}
				}
				// Check that there are enough free entries left
				else
				{
					// If everthing fits
					if (currentCount==0)
						return TRUE;
					else
						currentCount--;
				}

			} // End of for
		} // End of if
		// Run out of free space in the directory, allocate some more
		else
		{
			UINT32 newCluster;

			// Get a new cluster for directory
			if (!FAT32_FindBlankCluster(2, &newCluster))
				return FALSE;

			// Add cluster to end of directory tree
			if (!FAT32_AddClusterToEndofChain(dirCluster, newCluster))
				return FALSE;

			// Erase new directory cluster
			memset(FATFS_Internal.currentsector, 0x00, 512);
			for (i=0;i<FAT32.SectorsPerCluster;i++)
			{
				if (!FAT32_SectorWriter(newCluster, i))
					return FALSE;
			}

			// If non of the name fitted on previous sectors
			if (firstFound==FALSE) 
			{
				// Store start
				*pSector = (x-1);
				*pOffset = 0;
				firstFound = TRUE;
			}

			return TRUE;
		}
	} // End of while loop

	return FALSE;
}
//-----------------------------------------------------------------------------
// FAT32_AddFileEntry: Add a directory entry to a location found by FindFreeOffset
//-----------------------------------------------------------------------------
BOOL FAT32_AddFileEntry(UINT32 dirCluster, char *filename, char *shortfilename, UINT32 startCluster, UINT32 size)
{
	BYTE item=0;
	UINT16 recordoffset = 0;
	BYTE i=0;
	UINT32 x=0;
	int entryCount = FATMisc_LFN_to_entry_count(filename);
	FAT32_ShortEntry shortEntry;
	BOOL dirtySector = FALSE;

	UINT32 dirSector = 0;
	BYTE dirOffset = 0;
	BOOL foundEnd = FALSE;

	BYTE checksum;
	BYTE *pSname;

	if (entryCount==0)
		return FALSE;

	// Find space in the directory for this filename (or allocate some more)
	if (!FAT32_FindFreeOffset(dirCluster, entryCount, &dirSector, &dirOffset))
		return FALSE;

	// Generate checksum of short filename
	pSname = (BYTE*)shortfilename;
	checksum = 0;
	for (i=11; i!=0; i--) checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + *pSname++;

	// Main cluster following loop
	while (TRUE)
	{
		// Read sector
		if (FAT32_SectorReader(dirCluster, x++)) 
		{
			// Analyse Sector
			for (item=0; item<=15;item++)
			{
				// Create the multiplier for sector access
				recordoffset = (32*item);

				// If the start position for the entry has been found
				if (foundEnd==FALSE)
					if ( (dirSector==(x-1)) && (dirOffset==item) )
						foundEnd = TRUE;

				// Start adding filename
				if (foundEnd)
				{				
					if (entryCount==0)
					{
						// Short filename
						FATMisc_Create_sfn_entry(shortfilename, size, startCluster, &shortEntry);
						memcpy(&FATFS_Internal.currentsector[recordoffset], &shortEntry, sizeof(shortEntry));

						// Writeback
						return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);
					}
					else
					{
						entryCount--;

						// Copy entry to directory buffer
						FATMisc_LFN_to_lfn_entry(filename, &FATFS_Internal.currentsector[recordoffset], entryCount, checksum); 
						dirtySector = TRUE;
					}
				}
			} // End of if

			// Write back to disk before loading another sector
			if (dirtySector)
			{
				if (!FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector))
					return FALSE;

				dirtySector = FALSE;
			}
		} 
		else
			return FALSE;
	} // End of while loop

	return FALSE;
}
#endif

