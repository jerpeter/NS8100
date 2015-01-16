#if 0
#ifndef FAT32_MUISC_H_
#define FAT32_MUISC_H_

#include "define.h"
#include "FAT32_Definitions.h"
#include "FAT32_opts.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define MAX_LONGFILENAME_ENTRIES	20

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
struct 
{
	   // Long File Name Structure (max 260 LFN length)
	   BYTE String[MAX_LONGFILENAME_ENTRIES][13];
	   BYTE no_of_strings;
} FAT32_LFN;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void FATMisc_ClearLFN(BOOL wipeTable);
void FATMisc_CacheLFN(BYTE *entryBuffer);
void FATMisc_GetLFNCache(BYTE *strOut);
int FATMisc_If_LFN_TextOnly(FAT32_ShortEntry *entry);
int FATMisc_If_LFN_Invalid(FAT32_ShortEntry *entry);
int FATMisc_If_LFN_Exists(FAT32_ShortEntry *entry);
int FATMisc_If_noLFN_SFN_Only(FAT32_ShortEntry *entry);
int FATMisc_If_dir_entry(FAT32_ShortEntry *entry);
int FATMisc_If_file_entry(FAT32_ShortEntry *entry);
int FATMisc_LFN_to_entry_count(char *filename);
void FATMisc_LFN_to_lfn_entry(char *filename, BYTE *buffer, int entry, BYTE sfnChk);
void FATMisc_Create_sfn_entry(char *shortfilename, UINT32 size, UINT32 startCluster, FAT32_ShortEntry *entry);
BOOL FATMisc_CreateSFN(char *sfn_output, char *filename);
BOOL FATMisc_GenerateTail(char *sfn_output, char *sfn_input, UINT32 tailNum);

#endif /*FAT32_MUISC_H_*/
#endif