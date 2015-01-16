#if 0
#ifndef FAT32_TABLE_H_
#define FAT32_TABLE_H_

#include "define.h"
#include "FAT32_opts.h"

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void FAT32_InitFatBuffer();
BOOL FAT32_ReadFATSector(UINT32 sector);
BOOL FAT32_WriteFATSector(UINT32 sector);
BOOL FAT32_PurgeFATBuffer();
UINT32 FAT32_FindNextCluster(UINT32 Current_Cluster);
UINT32 FAT32_GetFsInfoNextCluster();
void FAT32_SetFsInfoNextCluster(UINT32 newValue);
BOOL FAT32_FindBlankCluster(UINT32 StartCluster, UINT32 *FreeCluster);
BOOL FAT32_SetClusterValue(UINT32 Cluster, UINT32 NextCluster);
BOOL FAT32_AddClusterToEndofChain(UINT32 StartCluster, UINT32 newEntry);
BOOL FAT32_FreeClusterChain(UINT32 StartCluster);

#endif /*FAT32_TABLE_H_*/
#endif