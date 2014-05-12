#ifndef FAT32_BASE_H_
#define FAT32_BASE_H_

#include "define.h"
#include "FAT32_opts.h"
#include "FAT32_Disk.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
struct
{
      // Filesystem globals
      BYTE SectorsPerCluster;
      UINT32 cluster_begin_lba;
      UINT32 RootDir_First_Cluster;
      UINT32 fat_begin_lba;
       UINT32 filenumber;
      UINT16 fs_info_sector;
      UINT32 lba_begin;
      UINT32 fat_sectors;
      UINT32 free_space;
      BYTE volume_label[11];
} FAT32;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
UINT32 FAT32_FindFreeClusters(void);
BOOL FAT32_FindLBABegin(BYTE *buffer, UINT32 *lbaBegin);
UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number);
BOOL FAT32_Init(void);

#endif /*FAT32_BASE_H_*/
