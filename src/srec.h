/*
 * srec.h
 *
 *  Created on: Apr 26, 2010
 *      Author: SW1
 */

#ifndef SREC_H_
#define SREC_H_

#include "typedefs.h"

#include "FAT32_Base.h"
#include "FAT32_Access.h"
#include "FAT32_Filelib.h"

//#define srec_h_Rev "$Revision: 1.1 $"

#if 0
#define    ACK     06
#define    NACK    21
#define    XOFF    19
#define    XON     17
#define    CAN     24
#endif
#define    CTRL_B  02

// srec lengths
#define SREC_LEN_LENGTH       1  /* Srecord length length   */
#define SREC_ADDR_LENGTH      4  /* Srecord address length  */
#define SREC_CKSUM_LENGTH     1  /* Srecord checksum length */

// srec type defines
#define SREC_HEADER           0  /* Srecord header block     */
#define SREC_DATA             3  /* Srecord data type 4-byte */
#define SREC_END              7  /* Srecord end type 4-byte  */

typedef struct
{
    uint8    RecordID;		 /* srecord id byte            */
    uint8    RecordType;	 /* record type 0-9            */
    uint8    Count[2];		 /* byte count byte            */
    uint8    Addr[8];        /* address array              */
    uint8    Data[128];	     /* data array                 */
    uint8    Checksum[2];    /* checksum byte              */
} ASCII_SREC_DATA;

typedef struct
{
    uint8    RecordType;	  /* record type 0-9            */
    uint8    Length;		     /* length byte count          */
    uint32   Address;        /* address array              */
    uint8    Data[64];	     /* data array                 */
    uint8    Checksum;       /* checksum byte              */
} RECORD_DATA;

typedef enum
{
   ENGLISH = 0,
   FRENCH =1,
   GERMAN =2,
   SPANISH =3,
   ITALIAN =4,
   NUM_LANGUAGES_IN_FIRMWARE
} eLanguageName;

typedef enum
{
   EXTERNAL_IMAGE = 0,
   INTERNAL_IMAGE =1,
} eImageSpace;

/***************************************************************/
/* srecord routines                                            */
/***************************************************************/
int            get_and_save_srec( FL_FILE* file );
int            unpack_srec(  FL_FILE* file  );
void           srec_get_line( ASCII_SREC_DATA *);
void           srec_file_get_line( ASCII_SREC_DATA *);
RECORD_DATA    srec_convert_line( ASCII_SREC_DATA linedata );
bool           srec_checksum( RECORD_DATA linedata );
void           srec_get_data( RECORD_DATA linedata, uint8 *data );

void           srec_ack( void );
void           srec_nack( void );

uint8  atoc( uint8 ch );
uint8  atonum( uint8 ch );
uint32 atoh_4( uint8 * ch );
uint8  atoh_2( uint8 * ch );
uint8  atoh_1( uint8 * ch );

void  srec_xOff( void );
void  srec_xOn( void );

#endif /* SREC_H_ */
