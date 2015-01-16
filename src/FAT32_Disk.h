#if 0
#ifndef FAT32_DISK_H_
#define FAT32_DISK_H_

#include "define.h"
#include "FAT32_opts.h"

BOOL FAT32_InitDrive();
BOOL FAT_ReadSector(UINT32 sector, BYTE *buffer);
BOOL FAT_WriteSector(UINT32 sector, BYTE *buffer);

#endif /*FAT32_DISK_H_*/
#endif