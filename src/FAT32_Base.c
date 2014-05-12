//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                     FAT32 File IO Library
//                          V2.0
//                          Rob Riglar
//                    Copyright 2003 - 2007
//
//                  Email: rob@robriglar.com
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

//-----------------------------------------------------------------------------
// FAT32_FindLBABegin: This function is used to find the LBA Address of the first
// volume on the disc. Also checks are performed on the signature and identity
// codes to make sure the partition is FAT32.
//-----------------------------------------------------------------------------
UINT32 FAT32_FindFreeClusters( void )
{
   BYTE buffer[512];
   
   // Load MBR (LBA 0) into the 512 byte buffer
   if (!FAT_ReadSector(0, buffer))
      return FALSE;

   return (GET_32BIT_WORD(buffer, Free_Clusters_Position));
}


//-----------------------------------------------------------------------------
// FAT32_FindLBABegin: This function is used to find the LBA Address of the first
// volume on the disc. Also checks are performed on the signature and identity
// codes to make sure the partition is FAT32.
//-----------------------------------------------------------------------------
BOOL FAT32_FindLBABegin(BYTE *buffer, UINT32 *lbaBegin)
{
   if (buffer==NULL)
      return FALSE;

   // Load MBR (LBA 0) into the 512 byte buffer
   if (!FAT_ReadSector(0, buffer))
      return FALSE;

   // Make Sure 0x55 and 0xAA are at end of sector
   if (GET_16BIT_WORD(buffer, Signature_Position)!=Signature_Value)
      return FALSE;

   // TODO: Verify this

   // Check the partition type code
   switch(buffer[PARTITION1_TYPECODE_LOCATION])
   {
   case 0x0B: break;
   case 0x06: break;
   case 0x0C: break;
   case 0x0E: break;
   case 0x0F: break;
   case 0x05: break;
   default:
      if (buffer[PARTITION1_TYPECODE_LOCATION] > 0x06)
         return FALSE;
      break;
   }

   // Read LBA Begin for FAT32 File system is located for partition
   *lbaBegin=GET_32BIT_WORD(buffer, PARTITION1_LBA_BEGIN_LOCATION);

   // Return the LBA address of FAT table
   return TRUE;
}
//-----------------------------------------------------------------------------
// FAT32_LBAofCluster: This function converts a cluster number into a sector /
// LBA number.
//-----------------------------------------------------------------------------
UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number)
{
   return ((FAT32.cluster_begin_lba + ((Cluster_Number-2)*FAT32.SectorsPerCluster)));
}
//-----------------------------------------------------------------------------
// FAT32_Init: Uses FAT32_FindLBABegin to find the LBA for the volume,
// and loads into memory some specific details of the partitionw which are used
// in further calculations.
//-----------------------------------------------------------------------------
BOOL FAT32_Init(void)
{
   BYTE buffer[512];

   BYTE Number_of_FATS;
   BYTE i;
   UINT16 Reserved_Sectors;
   UINT32 LBA_BEGIN;
   UINT32 FATSz;
   UINT32 RootDirSectors;
   UINT32 TotSec;
   UINT32 DataSec;
   UINT32 CountofClusters;

   FAT32_InitFatBuffer();

   // Check Volume 1 and find LBA address
   if (FAT32_FindLBABegin(buffer, &LBA_BEGIN))
   {
      FAT32.lba_begin = LBA_BEGIN;

      // Load Volume 1 table into sector buffer
      if (!FAT_ReadSector(LBA_BEGIN, buffer))
         return FALSE;

      // Make sure there are 512 bytes per cluster
      if (GET_16BIT_WORD(buffer, 0x0B)!=0x200)
         return FALSE;

      // Load Parameters of FAT32
      FAT32.SectorsPerCluster = buffer[BPB_SecPerClus];
      Reserved_Sectors = GET_16BIT_WORD(buffer, BPB_RsvdSecCnt);
      Number_of_FATS = buffer[BPB_NumFATs];
      FAT32.fat_sectors = GET_32BIT_WORD(buffer, BPB_FAT32_FATSz32);
      FAT32.RootDir_First_Cluster = GET_32BIT_WORD(buffer, BPB_FAT32_RootClus);
      FAT32.fs_info_sector = GET_16BIT_WORD(buffer, BPB_FAT32_FSInfo);

      // Get volume label
        for(i=0;i<10;i++)
      {
         FAT32.volume_label[i] = buffer[BS_FAT32_VolLab + i];
      }

      // First FAT LBA address
      FAT32.fat_begin_lba = LBA_BEGIN + Reserved_Sectors;

      // The address of the first data cluster on this volume
      FAT32.cluster_begin_lba = FAT32.fat_begin_lba + (Number_of_FATS * FAT32.fat_sectors);

      if (GET_16BIT_WORD(buffer, 0x1FE)!=0xAA55) // This signature should be AA55
         return FALSE;

      // Calculate the root dir sectors
      RootDirSectors = ((GET_16BIT_WORD(buffer, BPB_RootEntCnt) * 32) + (GET_16BIT_WORD(buffer, BPB_BytsPerSec) - 1)) / GET_16BIT_WORD(buffer, BPB_BytsPerSec);

      if(GET_16BIT_WORD(buffer, BPB_FATSz16) != 0)
         FATSz = GET_16BIT_WORD(buffer, BPB_FATSz16);
      else
         FATSz = GET_32BIT_WORD(buffer, BPB_FAT32_FATSz32);

      if(GET_16BIT_WORD(buffer, BPB_TotSec16) != 0)
         TotSec = GET_16BIT_WORD(buffer, BPB_TotSec16);
      else
         TotSec = GET_32BIT_WORD(buffer, BPB_TotSec32);

      DataSec = TotSec - (GET_16BIT_WORD(buffer, BPB_RsvdSecCnt) + (buffer[BPB_NumFATs] * FATSz) + RootDirSectors);

      CountofClusters = DataSec / FAT32.SectorsPerCluster;

      if(CountofClusters < 4085)
         // Volume is FAT12
         return FALSE;
      else if(CountofClusters < 65525)
         // Volume is FAT16
         return FALSE;

      return TRUE;
    }
    else
      return FALSE;
}

