#if 0
#ifndef FAT32_WRITE_H_
#define FAT32_WRITE_H_

#include "define.h"
#include "FAT32_Definitions.h"
#include "FAT32_opts.h"

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
BOOL FAT32_AddFileEntry(UINT32 dirCluster, char *filename, char *shortfilename, UINT32 startCluster, UINT32 size);
BOOL FAT32_AddFreeSpaceToChain(UINT32 *startCluster);
BOOL FAT32_AllocateFreeSpace(BOOL newFile, UINT32 *startCluster, UINT32 size);


#endif /*FAT32_WRITE_H_*/
#endif