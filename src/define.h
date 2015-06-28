///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

//-------------------------------------------------------------
// Global Defines
//-------------------------------------------------------------
#ifndef __DEFINE_H__
#define __DEFINE_H__

#define DEF_LITTLE		1
#define DEF_BIG			2

//-------------------------------------------------------------
//					Compile Target
//	!!!Comment out which ever platform you are NOT using!!!
//-------------------------------------------------------------
//#define TARGET_WINDOWS			1
#define TARGET_OTHER				1

//-------------------------------------------------------------
//					 Target Endian
//-------------------------------------------------------------
#define TARGET_ENDIAN			DEF_LITTLE
//#define TARGET_ENDIAN			DEF_BIG

//-------------------------------------------------------------
//					 Disk Configuration
//-------------------------------------------------------------
#define SOURCE_IDE_DRIVER					1		// Use custom hw storage driver
//#define SOURCE_WINDOWS_PHYSICAL_DRIVE		1		// Use Windows 2K/XP physical drive (caution!)
//#define SOURCE_MOUNT_FILE_AS_DRIVE		1		// Mount an image file of a FAT32 drive

// NOTE: The project does not come with any IDE/SD/CF driver:
// you must connect one to the stub functions in FAT32_Disk.c
// or use SOURCE_WINDOWS_PHYSICAL_DRIVE or SOURCE_MOUNT_FILE_AS_DRIVE

#ifdef SOURCE_WINDOWS_PHYSICAL_DRIVE
// If using SOURCE_WINDOWS_PHYSICAL_DRIVE and reading
// data directly to and from a PC drive:
// PhysicalDrive0 = 0, PhysicalDrive1 = 1, etc.
// See Disk Manager under Windows XP
	#define DISK_ID					1
#endif

#ifdef SOURCE_MOUNT_FILE_AS_DRIVE
// File to mount as a disk drive for testing on a PC
	#define DISK_MOUNT_FILE	".\\disk.bin"
#endif

//-------------------------------------------------------------
//					Structures/Typedefs
//-------------------------------------------------------------
// PC typedefs
#ifdef TARGET_WINDOWS

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <baseTsd.H>

	#ifndef BYTE
			typedef unsigned char BYTE;
	#endif

	#ifndef BOOL
			typedef int BOOL;
	#endif

	#ifndef UINT16
			typedef unsigned short UINT16;
	#endif

	//#ifndef UINT32
	//	typedef unsigned long UINT32;
	//#endif

	#ifndef TRUE
		#define TRUE 1
	#endif

	#ifndef FALSE
		#define FALSE 0
	#endif

	#ifndef NULL
		#define NULL 0
	#endif

	#define STRUCT_PACK


// Non PC typedefs
#else

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	#ifndef BYTE
			typedef unsigned char BYTE;
	#endif

	#ifndef UINT16
			typedef unsigned short UINT16;
	#endif

	#ifndef UINT32
		typedef unsigned long UINT32;
	#endif

	#ifndef BOOL
			typedef int BOOL;
	#endif

	#ifndef TRUE
		#define TRUE 1
	#endif

	#ifndef FALSE
		#define FALSE 0
	#endif

	#ifndef NULL
		#define NULL 0
	#endif

	#define STRUCT_PACK		//__attribute__ ((packed))
#endif

//-------------------------------------------------------------
//						Endian Specific
//-------------------------------------------------------------
// Little Endian
#if TARGET_ENDIAN == DEF_LITTLE
	#define GET_32BIT_WORD(buffer, location)	( ((UINT32)buffer[location+3]<<24) + ((UINT32)buffer[location+2]<<16) + ((UINT32)buffer[location+1]<<8) + (UINT32)buffer[location+0] )
	#define GET_16BIT_WORD(buffer, location)	( ((UINT16)buffer[location+1]<<8) + (UINT16)buffer[location+0] )

	#define SET_32BIT_WORD(buffer, location, value)	{ buffer[location+0] = (BYTE)((value)&0xFF); \
														buffer[location+1] = (BYTE)((value>>8)&0xFF); \
														buffer[location+2] = (BYTE)((value>>16)&0xFF); \
														buffer[location+3] = (BYTE)((value>>24)&0xFF); }

	#define SET_16BIT_WORD(buffer, location, value)	{ buffer[location+0] = (BYTE)((value)&0xFF); \
														buffer[location+1] = (BYTE)((value>>8)&0xFF); }
// Big Endian
#else
	#define GET_32BIT_WORD(buffer, location)	( ((UINT32)buffer[location+0]<<24) + ((UINT32)buffer[location+1]<<16) + ((UINT32)buffer[location+2]<<8) + (UINT32)buffer[location+3] )
	#define GET_16BIT_WORD(buffer, location)	( ((UINT16)buffer[location+0]<<8) + (UINT16)buffer[location+1] )

	#define SET_32BIT_WORD(buffer, location, value)	{ buffer[location+3] = (BYTE)((value)&0xFF); \
														buffer[location+2] = (BYTE)((value>>8)&0xFF); \
														buffer[location+1] = (BYTE)((value>>16)&0xFF); \
														buffer[location+0] = (BYTE)((value>>24)&0xFF); }

	#define SET_16BIT_WORD(buffer, location, value)	{ buffer[location+1] = (BYTE)((value)&0xFF); \
														buffer[location+0] = (BYTE)((value>>8)&0xFF); }
#endif

#endif
