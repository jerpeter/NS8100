///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: RomCopy.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:08 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/RomCopy.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "MMC2114.h"           /* target microcontroller register definitions */
#include "MMC2114_initvals.h"  /* project hardware translations */
#include "Common.h"

#if 0
///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
typedef uint32	size_t;

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern char __data_in_ROM_begin[]; // beginning of .data in ROM
extern char __data_in_RAM_begin[]; // beginning of .data in RAM
extern char __data_size[]; // .data size

///----------------------------------------------------------------------------
///	Globlas
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void copy_rom_section(void* dst, const void* src, uint32 size);
void board_chip_select_setup(void);
void startup_copy_rom_section(void);

//size_t (strlen)(const char* str);
//asm char* strcpy(register char* dst, register const char* src);
//asm void* memcpy(register void* dst, register const void* src, register size_t len);

/*
asm char* strcpy(register char* dst, register const char* src);
asm void* memcpy(register void* dst, register const void* src, register size_t len);
*/


/*
size_t (strlen)(const char* str)
{
	size_t	len = (uint32)-1;	
	do
		len++;
	while (*str++);	
	return (len);
}
*/


/*  strcpy.c	-	strcpy() and memcpy() routines for Motorola MCore series
 *	The compiler defines strcpy() and memcpy() as intrinsic functions so
 *	that it can detect the case where the number of bytes being copied is
 *	known at compile-time, and generate inline code.
 *	If the number of bytes being copied is not known at compile-time, calls to
 *	these intrinsics will link to the routines below.
 *	strcpy	-	linkable version of strcpy() intrinsic function
 *	If the compiler sees a call to the strcpy() intrinsic whose 2nd argument
 *	is not a char* literal, it will generate a call to this routine. We must
 *	copy the char* byte-by-byte as we do not know if the source and destination
 *	char*s are aligned to any particular boundary. */

/*
asm char* strcpy(register char* dst, register const char* src)
{
L1:	ld.b	r1,(src,0);
	addi	src,1;
	st.b	r1,(dst,0);
	tst		r1,r1;
	addi	dst,1;
	bt		L1;
	jmp		r15;
}
*/


/*	memcpy	-	linkable version of memcpy() intrinsic function
 *	If the compiler sees a call to the memcpy() intrinsic whose 3rd argument
 *	is not a constant-expression, it will generate a call to this routine. We must
 *	move the data byte-by-byte as we do not know if the source and destination
 *	are aligned to any particular boundary. */

/*
asm void* memcpy(register void* dst, register const void* src, register size_t len)
{
	tst		len,len;
	bf		L2;
L1:	ld.b	r1,(src,0);
	subi	len,1;
	st.b	r1,(dst,0);
	addi	src,1;
	addi	dst,1;
	tst		len,len;
	bt		L1;	
L2:
	jmp		r15
}
*/


/*	copy_rom_section	-	copy the exception/data vectors to their RAM destinations. */
void copy_rom_section(void* dst, const void* src, uint32 size)
{
    if (size && (dst != src)) 
    {
        byteCpy((uint8*)dst, (uint8*)src, size);   /* use MCore intrinsic */
    }
}


void startup_copy_rom_section(void)
{
    copy_rom_section(__data_in_RAM_begin, __data_in_ROM_begin, (uint32)__data_size);
}

/*	init_user	-	copy all the initialized non-constant data sections from ROM to RAM. */
void board_chip_select_setup(void)
{
     /* set up external ram chip select before ROM copy */
     /* Initialize Chip Select Module */
     reg_CSCR0.reg = init_CSCR0;         /* Chip Select 0 */
     reg_CSCR1.reg = init_CSCR1;         /* Chip Select 1 */
     reg_CSCR2.reg = init_CSCR2;         /* Chip Select 2 */
     reg_CSCR3.reg = init_CSCR3;         /* Chip Select 3 */
     
     
}
#endif
