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
#include "FAT32_base.h"
#include "FAT32_Table.h"

//-----------------------------------------------------------------------------
//							FAT Sector Buffer
//-----------------------------------------------------------------------------
struct 
{
	BYTE Data[512];
	UINT32 Sector;
	BOOL Changed;
	UINT32 Reads;
	UINT32 Writes;
} FATBuffer;

#define FAT32_GET_32BIT_WORD(location)	( GET_32BIT_WORD(FATBuffer.Data, location) )
#define FAT32_SET_32BIT_WORD(location, value)	{ SET_32BIT_WORD(FATBuffer.Data, location, value); FATBuffer.Changed = TRUE; }

//-----------------------------------------------------------------------------
// FAT32_InitFatBuffer:
//-----------------------------------------------------------------------------
void FAT32_InitFatBuffer()
{
	FATBuffer.Sector = 0xFFFFFFFF;
	FATBuffer.Changed = FALSE;
	FATBuffer.Reads = 0;
	FATBuffer.Writes = 0;
	memset(FATBuffer.Data, 0x00, sizeof(FATBuffer.Data));
}
//-----------------------------------------------------------------------------
// FAT32_ReadFATSector: Read a FAT sector
//-----------------------------------------------------------------------------
BOOL FAT32_ReadFATSector(UINT32 sector)
{
	// Only do anything if sector not already loaded
	if ( (sector!=FATBuffer.Sector) )
	{
		// Writeback
		if (FATBuffer.Changed)
		{
			FATBuffer.Writes++;
			if (!FAT_WriteSector(FATBuffer.Sector, FATBuffer.Data))
				return FALSE;
		}

		FATBuffer.Sector = sector;
		FATBuffer.Changed = FALSE;

		// Read next sector
		if (!FAT_ReadSector(FATBuffer.Sector, FATBuffer.Data))
			return FALSE;

		FATBuffer.Reads++;
	}

	return TRUE;
}
//-----------------------------------------------------------------------------
// FAT32_WriteFATSector: Write a FAT sector
//-----------------------------------------------------------------------------
BOOL FAT32_WriteFATSector(UINT32 sector)
{
#ifdef FATBUFFER_IMMEDIATE_WRITEBACK
	FATBuffer.Sector = sector;
	FATBuffer.Changed = FALSE;
	FATBuffer.Writes++;
	return FAT_WriteSector(FATBuffer.Sector, FATBuffer.Data);
#else
	return TRUE;
#endif
}
//-----------------------------------------------------------------------------
// FAT32_ReadFATSector: Read a FAT sector
//-----------------------------------------------------------------------------
BOOL FAT32_PurgeFATBuffer()
{
#ifndef FATBUFFER_IMMEDIATE_WRITEBACK

	// Writeback
	if (FATBuffer.Changed) 
	{
		FATBuffer.Writes++;
		if (!FAT_WriteSector(FATBuffer.Sector, FATBuffer.Data))
			return FALSE;
		
		FATBuffer.Changed = FALSE;
	}

#endif

	return TRUE;
}

//-----------------------------------------------------------------------------
//						General FAT Table Operations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FAT32_FindNextCluster: Return Cluster number of next cluster in chain by 
// reading FAT table and traversing it. Return 0xffffffff for end of chain.
//-----------------------------------------------------------------------------
UINT32 FAT32_FindNextCluster(UINT32 Current_Cluster)
{
	UINT32 FAT_sector_offset, position;
	UINT32 nextcluster;

	// Why is '..' labelled with cluster 0 when it should be 2 ??
	if (Current_Cluster==0) Current_Cluster=2;

	// Find which sector of FAT table to read
	FAT_sector_offset = Current_Cluster / 128;

	// Read FAT sector into buffer
	if (!FAT32_ReadFATSector(FAT32.fat_begin_lba+FAT_sector_offset))
		return (FAT32_EOC_FLAG); 

	// Find 32 bit entry of current sector relating to cluster number 
	position = (Current_Cluster - (FAT_sector_offset * 128)) * 4; 

	// Read Next Clusters value from Sector Buffer
	nextcluster = FAT32_GET_32BIT_WORD((UINT16)position);	 

	// Mask out MS 4 bits (its 28bit addressing)
	nextcluster = nextcluster & 0x0FFFFFFF;		

	// If 0x0FFFFFFF then end of chain found
	if (nextcluster==0x0FFFFFFF) 
		return (FAT32_EOC_FLAG); 
	else 
	// Else return next cluster
		return (nextcluster);						 
} 
//-----------------------------------------------------------------------------
// FAT32_GetFsInfoNextCluster: Read the next free cluster from FS info block
//-----------------------------------------------------------------------------
UINT32 FAT32_GetFsInfoNextCluster()
{
	UINT32 nextFreeCluster = 0xFFFFFFFF;

	// Read FSINFO sector into buffer
	FAT32_ReadFATSector(FAT32.lba_begin+FAT32.fs_info_sector);

	// Get next free cluster
	nextFreeCluster = FAT32_GET_32BIT_WORD(492);

	return nextFreeCluster;
}
//-----------------------------------------------------------------------------
// FAT32_SetFsInfoNextCluster: Write the next free cluster to the FSINFO table
//-----------------------------------------------------------------------------
void FAT32_SetFsInfoNextCluster(UINT32 newValue)
{
	// Load sector to change it
	if (!FAT32_ReadFATSector(FAT32.lba_begin+FAT32.fs_info_sector))
		return ;

	// Change 
	FAT32_SET_32BIT_WORD(492, newValue);

	// Write back
	FAT32_WriteFATSector(FAT32.lba_begin+FAT32.fs_info_sector);	
}
//-----------------------------------------------------------------------------
// FAT32_FindBlankCluster: Find a free cluster entry by reading the FAT
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_FindBlankCluster(UINT32 StartCluster, UINT32 *FreeCluster)
{
	UINT32 FAT_sector_offset, position;
	UINT32 nextcluster;
	UINT32 Current_Cluster = StartCluster;

	do
	{
		// Find which sector of FAT table to read
		FAT_sector_offset = Current_Cluster / 128;

		if ( FAT_sector_offset < FAT32.fat_sectors)
		{
			// Read FAT sector into buffer
			FAT32_ReadFATSector(FAT32.fat_begin_lba+FAT_sector_offset);

			// Find 32 bit entry of current sector relating to cluster number 
			position = (Current_Cluster - (FAT_sector_offset * 128)) * 4; 

			// Read Next Clusters value from Sector Buffer
			nextcluster = FAT32_GET_32BIT_WORD((UINT16)position);	 

			// Mask out MS 4 bits (its 28bit addressing)
			nextcluster = nextcluster & 0x0FFFFFFF;		

			if (nextcluster !=0 )
				Current_Cluster++;
		}
		else
			// Otherwise, run out of FAT sectors to check...
			return FALSE;
	}
	while (nextcluster != 0x0);

	// Found blank entry
	*FreeCluster = Current_Cluster;
	return TRUE;
} 
#endif
//-----------------------------------------------------------------------------
// FAT32_SetClusterValue: Set a cluster link in the chain. NOTE: Immediate
// write (slow).
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_SetClusterValue(UINT32 Cluster, UINT32 NextCluster)
{
	UINT32 FAT_sector_offset, position;

	// Find which sector of FAT table to read
	FAT_sector_offset = Cluster / 128;

	// Read FAT sector into buffer
	FAT32_ReadFATSector(FAT32.fat_begin_lba+FAT_sector_offset);

	// Find 32 bit entry of current sector relating to cluster number 
	position = (Cluster - (FAT_sector_offset * 128)) * 4; 

	// Write Next Clusters value to Sector Buffer
	FAT32_SET_32BIT_WORD((UINT16)position, NextCluster);	 

	// Write FAT sector from buffer
	FAT32_WriteFATSector(FAT32.fat_begin_lba+FAT_sector_offset);	

	return TRUE;					 
} 
#endif
//-----------------------------------------------------------------------------
// FAT32_FreeClusterChain: Follow a chain marking each element as free
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_FreeClusterChain(UINT32 StartCluster)
{
	UINT32 last_cluster;
	UINT32 next_cluster = StartCluster;
	
	// Loop until end of chain
	while ( (next_cluster!=0xFFFFFFFF) && (next_cluster!=0x00000000) )
	{
		last_cluster = next_cluster;

		// Find next link
		next_cluster = FAT32_FindNextCluster(next_cluster);

		// Clear last link
		FAT32_SetClusterValue(last_cluster, 0x00000000);
	}

	return TRUE;
} 
#endif
//-----------------------------------------------------------------------------
// FAT32_AddClusterToEndofChain: Follow a chain marking and then add a new entry
// to the current tail.
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_AddClusterToEndofChain(UINT32 StartCluster, UINT32 newEntry)
{
	UINT32 last_cluster = 0xFFFFFFFF;
	UINT32 next_cluster = StartCluster;

	if (StartCluster == 0xFFFFFFFF)
		return FALSE;
	
	// Loop until end of chain
	while ( next_cluster!=0xFFFFFFFF )
	{
		last_cluster = next_cluster;

		// Find next link
		next_cluster = FAT32_FindNextCluster(next_cluster);

		if (next_cluster==0x00000000)
			return FALSE;
	}

	// Add link in for new cluster
	FAT32_SetClusterValue(last_cluster, newEntry);

	// Mark new cluster as end of chain
	FAT32_SetClusterValue(newEntry, 0xFFFFFFFF);

	return TRUE;
} 
#endif
