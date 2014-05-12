#ifndef FAT32_ACCESS_H_
#define FAT32_ACCESS_H_

#include "define.h"
#include "FAT32_Definitions.h"
#include "FAT32_opts.h"

//-----------------------------------------------------------------------------
//  Globals
//-----------------------------------------------------------------------------
struct 
{
 	   BYTE currentsector[512];
	   UINT32 SectorCurrentlyLoaded; // Initially Load to 0xffffffff;
	   UINT32 NextFreeCluster;
} FATFS_Internal;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
BOOL FAT32_InitFAT(void);
BOOL FAT32_SectorReader(UINT32 Startcluster, UINT32 offset);
BOOL FAT32_SectorWriter(UINT32 Startcluster, UINT32 offset);
void FAT32_ShowFATDetails(void);
UINT32 FAT32_GetRootCluster();
UINT32 FAT32_GetFileEntry(UINT32 Cluster, char *nametofind, FAT32_ShortEntry *sfEntry);
BOOL FAT32_SFNexists(UINT32 Cluster, char *shortname);
BOOL FAT32_UpdateFileLength(UINT32 Cluster, char *shortname, UINT32 fileLength);
BOOL FAT32_MarkFileDeleted(UINT32 Cluster, char *shortname);
void ListDirectory(UINT32 StartCluster);

#endif /*FAT32_ACCESS_H_*/
