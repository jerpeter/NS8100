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
#include "print_funcs.h"
#include "FAT32_Definitions.h"
#include "FAT32_Base.h"
#include "FAT32_Table.h"
#include "FAT32_Access.h"
#include "FAT32_Write.h"
#include "FAT32_Filestring.h"
#include "FAT32_Misc.h"

//-----------------------------------------------------------------------------
// FAT32_InitFAT: Load FAT32 Parameters
//-----------------------------------------------------------------------------
BOOL FAT32_InitFAT(void)
{
	FATFS_Internal.SectorCurrentlyLoaded = 0xFFFFFFFF;
	FATFS_Internal.NextFreeCluster = 0xFFFFFFFF;
	return FAT32_Init();
}
//-----------------------------------------------------------------------------
// FAT32_SectorReader: From the provided startcluster and sector offset
// Returns True if success, returns False if not (including if read out of range)
//-----------------------------------------------------------------------------
BOOL FAT32_SectorReader(UINT32 Startcluster, UINT32 offset)
{
	UINT32 SectortoRead = 0;
	UINT32 ClustertoRead = 0;
	UINT32 ClusterChain = 0;
	UINT32 i;
	UINT32 lba;

	// Set start of cluster chain to initial value
	ClusterChain = Startcluster;

	// Find parameters
	ClustertoRead = offset / FAT32.SectorsPerCluster;	  
	SectortoRead = offset - (ClustertoRead*FAT32.SectorsPerCluster);

	// Follow chain to find cluster to read
	for (i=0; i<ClustertoRead; i++)
		ClusterChain = FAT32_FindNextCluster(ClusterChain);

	// If end of cluster chain then return false
	if (ClusterChain==0xFFFFFFFF) 
		return FALSE;

	// Calculate sector address
	lba = FAT32_LBAofCluster(ClusterChain)+SectortoRead;

	// Else read sector if not already loaded
	if (lba!=FATFS_Internal.SectorCurrentlyLoaded)
	{
		FATFS_Internal.SectorCurrentlyLoaded = lba;
		return FAT_ReadSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);
	}
	else
		return TRUE;
}

//-----------------------------------------------------------------------------
// FAT32_SectorWriter: Write to the provided startcluster and sector offset
// Returns True if success, returns False if not 
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_SectorWriter(UINT32 Startcluster, UINT32 offset)
{
 	UINT32 SectortoWrite = 0;
	UINT32 ClustertoWrite = 0;
	UINT32 ClusterChain = 0;
	UINT32 LastClusterChain = 0xFFFFFFFF;
	UINT32 i;
	
	// Set start of cluster chain to initial value
	ClusterChain = Startcluster;

	// Find parameters
	ClustertoWrite = offset / FAT32.SectorsPerCluster;	  
	SectortoWrite = offset - (ClustertoWrite*FAT32.SectorsPerCluster);

	// Follow chain to find cluster to read
	for (i=0; i<ClustertoWrite; i++)
	{
		// Find next link in the chain
		LastClusterChain = ClusterChain;
	  	ClusterChain = FAT32_FindNextCluster(ClusterChain);

		// Dont keep following a dead end
		if (ClusterChain==0xFFFFFFFF)
			break;
	}

	// If end of cluster chain 
	if (ClusterChain==0xFFFFFFFF) 
	{
		// Add another cluster to the last good cluster chain
		if (!FAT32_AddFreeSpaceToChain(&LastClusterChain))
			return FALSE;

		ClusterChain = LastClusterChain;
	}

	// Calculate write address
	FATFS_Internal.SectorCurrentlyLoaded = FAT32_LBAofCluster(ClusterChain)+SectortoWrite;

	// Write to disk
	return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);
}
#endif
//-----------------------------------------------------------------------------
// FAT32_ShowFATDetails: Show the details about the filesystem
//-----------------------------------------------------------------------------
void FAT32_ShowFATDetails(void)
{
	BYTE i;
	
	print_dbg("    Volume Label...................");
	for(i=0;i<10;i++)
	{
		print_dbg_char(FAT32.volume_label[i]);
	}
    print_dbg("\r\n");
	
	print_dbg("    FAT 1st cluster................");
//    print_dbg("\r\nCurrent Disc FAT details\r\n------------------------\r\nRoot Dir First Cluster = ");   
    //printf("0x%x",FAT32.RootDir_First_Cluster);
    print_dbg_ulong(FAT32.RootDir_First_Cluster);
    print_dbg("\r\n");

    print_dbg("    FAT Begin LBA..................");
//    print_dbg("\r\nFAT Begin LBA = ");
	//printf("0x%x",FAT32.fat_begin_lba);
	print_dbg_ulong(FAT32.fat_begin_lba);
	print_dbg("\r\n");

    print_dbg("    Cluster Begin LBA..............");
//	print_dbg("\r\nCluster Begin LBA = ");
	//printf("0x%x",FAT32.cluster_begin_lba);
	print_dbg_ulong(FAT32.cluster_begin_lba);
	print_dbg("\r\n");

    print_dbg("    Sectors per cluster............");
//	print_dbg("\r\nSectors Per Cluster = ");
	//printf("%d",FAT32.SectorsPerCluster);
	print_dbg_ulong(FAT32.SectorsPerCluster);
	print_dbg("\r\n");

//	print_dbg("\r\n\r\nFormula for conversion from Cluster num to LBA is;");
//	print_dbg("\r\nLBA = (cluster_begin_lba + ((Cluster_Number-2)*SectorsPerCluster)))\r\n");
}
//-----------------------------------------------------------------------------
// FAT32_GetRootCluster: Get the root dir cluster
//-----------------------------------------------------------------------------
UINT32 FAT32_GetRootCluster()
{
	return FAT32.RootDir_First_Cluster;
}
//-------------------------------------------------------------
// FAT32_GetFileEntry: Find the file entry for a filename
//-------------------------------------------------------------
UINT32 FAT32_GetFileEntry(UINT32 Cluster, char *nametofind, FAT32_ShortEntry *sfEntry)
{
	BYTE item=0;
	UINT16 recordoffset = 0;
	BYTE i=0;
	int x=0;
	char LongFilename[MAX_LONG_FILENAME];
	char ShortFilename[13];
	FAT32_ShortEntry *directoryEntry;

	FATMisc_ClearLFN(TRUE);

	// Main cluster following loop
	while (TRUE)
	{
		// Read sector
		if (FAT32_SectorReader(Cluster, x++)) // If sector read was successfull
		{
			// Analyse Sector
			for (item=0; item<=15;item++)
			{
				// Create the multiplier for sector access
				recordoffset = (32*item);

				// Overlay directory entry over buffer
				directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);

				// Long File Name Text Found
				if (FATMisc_If_LFN_TextOnly(directoryEntry) ) 
					FATMisc_CacheLFN(FATFS_Internal.currentsector+recordoffset);

				// If Invalid record found delete any long file name information collated
				else if (FATMisc_If_LFN_Invalid(directoryEntry) ) 
					FATMisc_ClearLFN(FALSE);

				// Normal SFN Entry and Long text exists 
				else if (FATMisc_If_LFN_Exists(directoryEntry) ) 
				{
					FATMisc_GetLFNCache((BYTE*)LongFilename);

					// Compare names to see if they match
					if (FileString_Compare(LongFilename, nametofind)) 
					{
						memcpy(sfEntry,directoryEntry,sizeof(FAT32_ShortEntry));
						return TRUE;
					}

		 			FATMisc_ClearLFN(FALSE);
				}

				// Normal Entry, only 8.3 Text		 
				else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )
				{
					memset(ShortFilename, 0, sizeof(ShortFilename));

					// Copy name to string
					for (i=0; i<8; i++) 
						ShortFilename[i] = directoryEntry->Name[i];

					// If not . or .. entry
					if (ShortFilename[0]!='.')
                        ShortFilename[8] = '.';
					else
						ShortFilename[8] = ' ';

					// Extension
					for (i=8; i<11; i++) 
						ShortFilename[i+1] = directoryEntry->Name[i];
		  			
					// Compare names to see if they match
					if (FileString_Compare(ShortFilename, nametofind)) 
					{
						memcpy(sfEntry,directoryEntry,sizeof(FAT32_ShortEntry));
						return TRUE;
					}

					FATMisc_ClearLFN(FALSE);
				}
			} // End of if
		} 
		else
			break;
	} // End of while loop

	return FALSE;
}
//-------------------------------------------------------------
// FAT32_SFNexists: Check if a short filename exists.
// NOTE: shortname is XXXXXXXXYYY not XXXXXXXX.YYY
//-------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_SFNexists(UINT32 Cluster, char *shortname)
{
	BYTE item=0;
	UINT16 recordoffset = 0;
	int x=0;
	FAT32_ShortEntry *directoryEntry;

	// Main cluster following loop
	while (TRUE)
	{
		// Read sector
		if (FAT32_SectorReader(Cluster, x++)) // If sector read was successfull
		{
			// Analyse Sector
			for (item=0; item<=15;item++)
			{
				// Create the multiplier for sector access
				recordoffset = (32*item);

				// Overlay directory entry over buffer
				directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);

				// Long File Name Text Found
				if (FATMisc_If_LFN_TextOnly(directoryEntry) ) 
					;

				// If Invalid record found delete any long file name information collated
				else if (FATMisc_If_LFN_Invalid(directoryEntry) ) 
					;

				// Normal Entry, only 8.3 Text		 
				else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )
				{
					if (strncmp((const char*)directoryEntry->Name, shortname, 11)==0)
						return TRUE;
				}
			} // End of if
		} 
		else
			break;
	} // End of while loop

	return FALSE;
}
#endif
//-------------------------------------------------------------
// FAT32_UpdateFileLength: Find a SFN entry and update it 
// NOTE: shortname is XXXXXXXXYYY not XXXXXXXX.YYY
//-------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_UpdateFileLength(UINT32 Cluster, char *shortname, UINT32 fileLength)
{
	BYTE item=0;
	UINT16 recordoffset = 0;
	int x=0;
	FAT32_ShortEntry *directoryEntry;

	// Main cluster following loop
	while (TRUE)
	{
		// Read sector
		if (FAT32_SectorReader(Cluster, x++)) // If sector read was successfull
		{
			// Analyse Sector
			for (item=0; item<=15;item++)
			{
				// Create the multiplier for sector access
				recordoffset = (32*item);

				// Overlay directory entry over buffer
				directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);

				// Long File Name Text Found
				if (FATMisc_If_LFN_TextOnly(directoryEntry) ) 
					;

				// If Invalid record found delete any long file name information collated
				else if (FATMisc_If_LFN_Invalid(directoryEntry) ) 
					;

				// Normal Entry, only 8.3 Text		 
				else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )
				{
					if (strncmp((const char*)directoryEntry->Name, shortname, 11)==0)
					{
						directoryEntry->FileSize[0] = (BYTE)((fileLength >> 0) & 0xFF);
						directoryEntry->FileSize[1] = (BYTE)((fileLength >> 8) & 0xFF);
						directoryEntry->FileSize[2] = (BYTE)((fileLength >> 16) & 0xFF);
						directoryEntry->FileSize[3] = (BYTE)((fileLength >> 24) & 0xFF);
						//directoryEntry->FileSize = fileLength;
						// TODO: Update last write time

						// Update sfn entry
						memcpy((BYTE*)(FATFS_Internal.currentsector+recordoffset), (BYTE*)directoryEntry, sizeof(FAT32_ShortEntry));					

						// Write sector back
						return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);
					}
				}
			} // End of if
		} 
		else
			break;
	} // End of while loop

	return FALSE;
}
#endif
//-------------------------------------------------------------
// FAT32_MarkFileDeleted: Find a SFN entry and mark if as deleted 
// NOTE: shortname is XXXXXXXXYYY not XXXXXXXX.YYY
//-------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
BOOL FAT32_MarkFileDeleted(UINT32 Cluster, char *shortname)
{
	BYTE item=0;
	UINT16 recordoffset = 0;
	int x=0;
	FAT32_ShortEntry *directoryEntry;

	// Main cluster following loop
	while (TRUE)
	{
		// Read sector
		if (FAT32_SectorReader(Cluster, x++)) // If sector read was successfull
		{
			// Analyse Sector
			for (item=0; item<=15;item++)
			{
				// Create the multiplier for sector access
				recordoffset = (32*item);

				// Overlay directory entry over buffer
				directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);

				// Long File Name Text Found
				if (FATMisc_If_LFN_TextOnly(directoryEntry) ) 
					;

				// If Invalid record found delete any long file name information collated
				else if (FATMisc_If_LFN_Invalid(directoryEntry) ) 
					;

				// Normal Entry, only 8.3 Text		 
				else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )
				{
					if (strncmp((const char *)directoryEntry->Name, shortname, 11)==0)
					{
						// Mark as deleted
						directoryEntry->Name[0] = 0xE5; 

						// Update sfn entry
						memcpy((BYTE*)(FATFS_Internal.currentsector+recordoffset), (BYTE*)directoryEntry, sizeof(FAT32_ShortEntry));					

						// Write sector back
						return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);
					}
				}
			} // End of if
		} 
		else
			break;
	} // End of while loop

	return FALSE;
}
#endif
//-----------------------------------------------------------------------------
// ListDirectory: Using starting cluster number of a directory and the FAT,
//				  list all directories and files 
//-----------------------------------------------------------------------------
void ListDirectory(UINT32 StartCluster, FAT32_DIRLIST* dirList, BOOL displayResults)
{
//	BYTE buffer[128];
	BYTE i,item;
	UINT16 recordoffset;
	BYTE LFNIndex=0;
	UINT32 x=0;
	FAT32_ShortEntry *directoryEntry;
	char LongFilename[MAX_LONG_FILENAME];
	char ShortFilename[13];
    char tempBuffer[50];
    unsigned int  dirFiles = 0;
    unsigned int  dirSize = 0;
    unsigned int  dirDirs = 0;
    unsigned long int  freeSpace;
    unsigned long tempData;
	UINT16 totalEntriesFound = 0;

    FAT32.filenumber=0;
	//print_dbg("\r\nNo.             Filename\r\n");
	if (displayResults)
		print_dbg("\r\n");

	FATMisc_ClearLFN(TRUE);
	
	while (TRUE)
	{
		// If data read OK
		if (FAT32_SectorReader(StartCluster, x++))
		{
			LFNIndex=0;

			// Maximum of 15 directory entries
			for (item=0; item<=15;item++)
			{
				// Increase directory offset 
				recordoffset = (32*item);

				// Overlay directory entry over buffer
				directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);
		 
				// Long File Name Text Found
				if ( FATMisc_If_LFN_TextOnly(directoryEntry) )   
					FATMisc_CacheLFN(FATFS_Internal.currentsector+recordoffset);
				 	 
				// If Invalid record found delete any long file name information collated
				else if ( FATMisc_If_LFN_Invalid(directoryEntry) ) 	
					FATMisc_ClearLFN(FALSE);

				// Normal SFN Entry and Long text exists 
				else if (FATMisc_If_LFN_Exists(directoryEntry) ) 
				{
					FAT32.filenumber++; //File / Dir Count

					// Get text
					FATMisc_GetLFNCache((BYTE*)LongFilename);

		 			if (FATMisc_If_dir_entry(directoryEntry)) 
		 			{
						if (displayResults)
		 					print_dbg("\r\n    01/01/2008  12:00p    <DIR>  ");
		                sprintf(tempBuffer,"%s",LongFilename);
						if (displayResults)
							print_dbg(tempBuffer);
    					dirDirs++;

#if 1
						if (dirList != NULL)
						{
							//memset(&dirList[totalEntriesFound], 0, sizeof(FAT32_DIRLIST));
							
							//print_dbg(" (Found long name dir)");
							dirList[totalEntriesFound].type = FAT32_DIR;
							sprintf(dirList[totalEntriesFound].name, "%s", LongFilename);
							totalEntriesFound++;
						}
#endif
		 			}
    				if (FATMisc_If_file_entry(directoryEntry)) 
    				{
						if (displayResults)
	    					print_dbg("\r\n    01/01/2008  12:00p    ");
    					tempData = ((directoryEntry->FileSize[0] << 0) & 0x000000FF);
      					tempData += ((directoryEntry->FileSize[1] << 8) & 0x0000FF00);
      					tempData += ((directoryEntry->FileSize[2] << 16)& 0x00FF0000);
      					tempData += ((directoryEntry->FileSize[3] << 24)& 0xFF000000);
   	                    sprintf(tempBuffer,"%8lu  %s",tempData, LongFilename);
						if (displayResults)
	   	            		print_dbg(tempBuffer);
    					dirFiles++;
    					dirSize += tempData;

#if 1
						if (dirList != NULL)
						{
							//memset(&dirList[totalEntriesFound], 0, sizeof(FAT32_DIRLIST));
							
							//print_dbg(" (Found long name file)");
							dirList[totalEntriesFound].type = FAT32_FILE;
							dirList[totalEntriesFound].size = tempData;
							sprintf(dirList[totalEntriesFound].name, "%s", LongFilename);
							totalEntriesFound++;
						}
#endif
    				}

					// Print Filename
					//printf("%d - %s [%d bytes] (0x%08lx)",FAT32.filenumber, LongFilename, directoryEntry->FileSize, (directoryEntry->FstClusHI<<16)|directoryEntry->FstClusLO);
					//print_dbg_ulong(FAT32.filenumber);
					//print_dbg(" - ");
                    //sprintf(tempBuffer,"%s",LongFilename);
            		//print_dbg(tempBuffer);
            		//print_dbg("\t");
					//print_dbg_ulong(directoryEntry->FileSize);
            		//print_dbg(" bytes");
            		//print_dbg(" [");
					//print_dbg_ulong((directoryEntry->FstClusHI<<16)|directoryEntry->FstClusLO);
            		//print_dbg("]");
                                
					FATMisc_ClearLFN(FALSE);
				}
				 
				// Normal Entry, only 8.3 Text		 
				else if ( FATMisc_If_noLFN_SFN_Only(directoryEntry) )
				{
       				FATMisc_ClearLFN(FALSE);
					FAT32.filenumber++; //File / Dir Count
					
		 			if (FATMisc_If_dir_entry(directoryEntry))
		 			{
		 				//print_dbg("\r\n    01/01/2008  12:00p    <DIR>     ");
		                sprintf(tempBuffer,"%s",directoryEntry->Name);
						if (displayResults)
			            	print_dbg(tempBuffer);
    					dirDirs++;

#if 1
						if (dirList != NULL)
						{
							//memset(&dirList[totalEntriesFound], 0, sizeof(FAT32_DIRLIST));
							
							//print_dbg(" (Found short name dir)");
							dirList[totalEntriesFound].type = FAT32_DIR;
							sprintf(dirList[totalEntriesFound].name, "%s", directoryEntry->Name);
							totalEntriesFound++;
						}
#endif
		 			}
    				if (FATMisc_If_file_entry(directoryEntry))
    				{
    					memset(ShortFilename, 0, sizeof(ShortFilename));
    					// Copy name to string
    					for (i=0; i<8; i++) 
    						ShortFilename[i] = directoryEntry->Name[i];
    					// If not . or .. entry
    					if (ShortFilename[0]!='.')
                            ShortFilename[8] = '.';
    					else
    						ShortFilename[8] = ' ';
    					// Extension
    					for (i=8; i<11; i++) 
    						ShortFilename[i+1] = directoryEntry->Name[i];

						if (displayResults)
	    					print_dbg("\r\n    01/01/2008  12:00p    ");
    					tempData = ((directoryEntry->FileSize[0] << 0) & 0x000000FF);
      					tempData += ((directoryEntry->FileSize[1] << 8) & 0x0000FF00);
      					tempData += ((directoryEntry->FileSize[2] << 16)& 0x00FF0000);
      					tempData += ((directoryEntry->FileSize[3] << 24)& 0xFF000000);
   	                    sprintf(tempBuffer,"%8lu  %s",tempData, ShortFilename);
						if (displayResults)
	   	            		print_dbg(tempBuffer);
    					dirFiles++;
    					dirSize += tempData;

#if 1
						if (dirList != NULL)
						{
							//memset(&dirList[totalEntriesFound], 0, sizeof(FAT32_DIRLIST));
							
							//print_dbg(" (Found short name file)");
							dirList[totalEntriesFound].type = FAT32_FILE;
							sprintf(dirList[totalEntriesFound].name, "%s", ShortFilename);
							dirList[totalEntriesFound].size = tempData;
							totalEntriesFound++;
						}
#endif
    				}

		  			
					// Print Filename
//					printf("%d - %s",FAT32.filenumber, ShortFilename);
//					print_dbg_ulong(FAT32.filenumber);
//					print_dbg(" - ");
//                    sprintf(tempBuffer,"%s",ShortFilename);
//            		print_dbg(tempBuffer);
//            		print_dbg(" [");
//					print_dbg_ulong((directoryEntry->FstClusHI<<16)|directoryEntry->FstClusLO);
//            		print_dbg("]");
					 					
				}
			}// end of for
		}
		else
			break;
	}

	if (dirList != NULL)
	{
		if (displayResults)
			print_dbg("\r\nMark last list entry the End\r\n");
		memset(&dirList[totalEntriesFound], 0, sizeof(FAT32_DIRLIST));
		dirList[totalEntriesFound].type = FAT32_END_LIST;
	}

    sprintf(tempBuffer,"\n\r\n\r    %5d file(s)     %8d bytes\n\r",dirFiles, dirSize);
	if (displayResults)
		print_dbg(tempBuffer);
	freeSpace = FAT32_FindFreeClusters();   
	sprintf(tempBuffer,"    %5d dir(s)    %10lu free bytes\n\r",dirDirs, freeSpace);
	if (displayResults)
		print_dbg(tempBuffer);
} 

#endif