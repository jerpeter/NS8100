#ifndef FAT32_OPTS_H_
#define FAT32_OPTS_H_

//-------------------------------------------------------------
// Configuration
//-------------------------------------------------------------

// Max filename Length 
#define MAX_LONG_FILENAME					260

// Max open files (reduce to lower memory requirements)
#define MAX_OPEN_FILES						4

// Writes to FAT are done immediately
#define FATBUFFER_IMMEDIATE_WRITEBACK		1

// Include support for writing files
#define INCLUDE_WRITE_SUPPORT				1

#endif /*FAT32_OPTS_H_*/
