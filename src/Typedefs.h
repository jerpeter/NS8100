///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Typedefs.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:02 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Typedefs.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#ifndef _PORTABLE_DATA_TYPES_
#define _PORTABLE_DATA_TYPES_

    typedef unsigned char  BOOLEAN;
    typedef unsigned char  INT8U;
    typedef signed   char  INT8S;
    typedef unsigned short INT16U;
    typedef signed   short INT16S;
    typedef unsigned int   INT32U;
    typedef signed   int   INT32S;
    typedef float          FP32;
    typedef double         FP64;

#endif // _PORTABLE_DATA_TYPES_

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

typedef unsigned			bitfield;		/* variable # of bits */
typedef unsigned char       BOOL;
typedef unsigned long       ADDRESS;

typedef unsigned char		uint8;  		/*  8 bits */
typedef unsigned short int	uint16; 		/* 16 bits */
typedef unsigned long int	uint32; 		/* 32 bits */
typedef signed char		    int8;   		/*  8 bits */
typedef signed short int	int16;  		/* 16 bits */
typedef signed long int		int32;  		/* 32 bits */

// ========================================================================
// This section is devoted to handling debug printing to the craft com port
// ========================================================================

// Global Debug Modes
#define ALL_DEBUG 			1
#define WARNINGS_AND_ERRORS	2
#define ERRORS				3
#define NO_DEBUG			4

// Debug levels
enum debugModes {RAW, NORM, WARN, ERR};

// Global Debug Mode
#define GLOBAL_DEBUG_PRINT_ENABLED ALL_DEBUG

// Print all debug statements
#if (GLOBAL_DEBUG_PRINT_ENABLED == ALL_DEBUG)
#define debugRaw(...) 	debugPrint(RAW, __VA_ARGS__)
#define debug(...)    	debugPrint(NORM, __VA_ARGS__)
#define debugWarn(...) 	debugPrint(WARN, __VA_ARGS__)
#define debugErr(...) 	debugPrint(ERR, __VA_ARGS__)
#define debugChar(x) 	debugPrintChar(x)

// Print just warning and error debug statements
#elif (GLOBAL_DEBUG_PRINT_ENABLED == WARNINGS_AND_ERRORS)
#define debugRaw(...) 	;
#define debug(...)    	;
#define debugWarn(...) 	debugPrint(WARN, __VA_ARGS__)
#define debugErr(...) 	debugPrint(ERR, __VA_ARGS__)
#define debugChar(x) 	;

// Print just error debug statements
#elif (GLOBAL_DEBUG_PRINT_ENABLED == ERRORS)
#define debugRaw(...) 	;
#define debug(...)    	;
#define debugWarn(...) 	;
#define debugErr(...) 	debugPrint(ERR, __VA_ARGS__)
#define debugChar(x) 	;

// Print no debug statements
#elif (GLOBAL_DEBUG_PRINT_ENABLED == NO_DEBUG)
#define debugRaw(...) 	;
#define debug(...)    	;
#define debugWarn(...) 	;
#define debugErr(...) 	;
#define debugChar(x) 	;

#endif // End of Global Debug subsection
// ========================================================================

#define thousands(x) ((x)/1000+'0')
#define hundreds(x)  (((x)/100)%10+'0')
#define tens(x)      (((x)/10) %10 +'0')
#define units(x)     ((x)%10+'0')

#define CLEAR_SCREEN printf("\x1b[2J")

/* The vector macro that puts the address of the interrupt
   service routine into the specified vector address */
#define vector(isr,address) (*(void **)(address)=(isr))
#define vec_offset 32    /* vectored interrupt vectors start at VBR + 32      */

#if 1
#ifndef NULL
	#define NULL 0x0
#endif
#endif

#ifndef bool
	typedef unsigned int bool;
#endif

enum {SLOW = 0, FAST};
enum {INPUT = 0, OUTPUT};
enum {VSS = 0, VDD};
enum {FAILED = 0, PASSED};

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NO
#define NO 0
#endif

#ifndef YES
#define YES 1
#endif

#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

#ifndef CLEAR
#define CLEAR 0
#endif

#ifndef SET
#define SET 1
#endif

#ifndef LOW
#define LOW 0
#endif

#ifndef HIGH
#define HIGH 1
#endif

#ifndef DISABLED
#define DISABLED 0
#endif

#ifndef ENABLED
#define ENABLED 1
#endif

#ifndef FAILED
#define FAILED 0
#endif

#ifndef PASSED
#define PASSED 1
#endif

#if 0
#ifndef FAIL
#define FAIL 0
#endif

#ifndef PASS
#define PASS 1
#endif
#endif

#define FOREVER while (TRUE)

#define SHOW_EVENT_STATE_CHANGES 0
/*
 * Routines and macros for accessing Input/Output devices
 */
#define cpu_iord_8(ADDR)        *((volatile uint8*)(ADDR))
#define cpu_iord_16(ADDR)       *((volatile uint16*)(ADDR))
#define cpu_iord_32(ADDR)       *((volatile uint32*)(ADDR))

#define cpu_iowr_8(ADDR,DATA)   *((volatile uint8*)(ADDR)) = (DATA)
#define cpu_iowr_16(ADDR,DATA)  *((volatile uint16*)(ADDR)) = (DATA)
#define cpu_iowr_32(ADDR,DATA)  *((volatile uint32*)(ADDR)) = (DATA)

/*#define NULL  0x00 */
#define EOT   0x04
#define ACK   0x06
#define XON   0x11
#define XOFF  0x13
#define NACK  0x15
#define CAN   0x18

enum MCORE_COMMAND
{
    M_TIME_COMMAND,
    M_PRINTER_COMMAND,
    M_TRIGGER_COMMAND,
    M_CALIBRATION_COMMAND
};

enum MCORE_STATUS
{
    M_ACK,
    M_NACK
};

enum ATOD_COMMAND
{
    A_DATA_COMMAND,
    A_PRINTER_COMMAND,
    A_TRIGGER_COMMAND,
    A_CALIBRATION_COMMAND
};

enum ATOD_STATUS
{
    A_ACK,
    A_NACK
};

#endif // _TYPEDEFS_H_
