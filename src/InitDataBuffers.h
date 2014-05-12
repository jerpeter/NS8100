///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: InitDataBuffers.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:50 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/InitDataBuffers.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _DATABUFFS_H_
#define _DATABUFFS_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Summary.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define MIN_SAMPLE_RATE				512
#define MAX_SAMPLE_RATE				8192
#define MAX_NUM_OF_CHANNELS			4
#define CHANNEL_DATA_IN_BYTES		2
#define MAX_CAL_SAMPLES				100 /*(100 / (1024 / MIN_SAMPLE_RATE))*/
#define START_CAL_SIGNAL			(100 + 1)
#define MAX_DATA_PER_SAMPLE			(MAX_NUM_OF_CHANNELS * CHANNEL_DATA_IN_BYTES) // 8
#define MAX_DATA_PER_SECOND			(MAX_SAMPLE_RATE * MAX_DATA_PER_SAMPLE) // 64K

#define LARGEST_EVENT_SIZE_IN_BYTES			(MAX_DATA_PER_SECOND * 100) // 655360 = MAX_DATA_PER_SECOND * 5 seconds
#define LARGEST_PRETRIGGER_SIZE_IN_BYTES	(MAX_DATA_PER_SECOND / 4) // 32768 = MAX_DATA_PER_SECOND * 1/4 second
#define LARGEST_CAL_SIZE_IN_BYTES			(MAX_DATA_PER_SAMPLE * MAX_CAL_SAMPLES) // 1600
#define SPARE_SIZE_IN_BYTES					24000 // Leftover to match old event buffer size

#define PRE_TRIG_BUFF_SIZE_IN_BYTES 	(LARGEST_PRETRIGGER_SIZE_IN_BYTES + MAX_DATA_PER_SAMPLE) // 1/4 second + 1 sample
#define PRE_TRIG_BUFF_SIZE_IN_WORDS 	(PRE_TRIG_BUFF_SIZE_IN_BYTES / 2)

#define EVENT_BUFF_SIZE_IN_BYTES    (LARGEST_EVENT_SIZE_IN_BYTES + LARGEST_PRETRIGGER_SIZE_IN_BYTES + LARGEST_CAL_SIZE_IN_BYTES + SPARE_SIZE_IN_BYTES)
#define EVENT_BUFF_SIZE_IN_WORDS    (EVENT_BUFF_SIZE_IN_BYTES / 2)

typedef enum
{
	SUCCESS      	= 0x0000,
	EVT_BUF_FULL 	= 0x0001,
	SUMM_TBL_FULL 	= 0x0002
} FULL_FLAGS;

typedef enum
{
	SAMPLE_DATA 	= 0x0000,
	TRIG_ONE    	= 0x1000,
	TRIG_TWO    	= 0x2000,
	TRIG_THREE  	= 0x3000,
	EVENT_END   	= 0x4000,
	EVENT_START 	= 0x5000,
	CAL_START   	= 0x6000,
	CAL_END     	= 0x7000,
	EVENT_END_START = 0x8000,
	EVENT_START_2ND = 0x9000,
	EMBEDDED_CMD  	= 0xF000,
	DATA_MASK		= 0x0FFF,
	SIGNBIT_MASK	= 0x0800
} EMBEDDED_CMDS;

typedef enum
{
	POS_VALID_PEAK 			= 0x0804,
	POS_CROSSOVER_BACKWARD 	= 0x0802,
	POS_CROSSOVER_FORWARD 	= 0x0801,
	NEG_VALID_PEAK 			= 0x07FC,
	NEG_CROSSOVER_BACKWARD 	= 0x07FE,
	NEG_CROSSOVER_FORWARD 	= 0x07FF
} CROSSOVER_LEVELS;

typedef enum
{
	MOV_BEGINNING = 1,
	MOV_ABORT,
	MOV_CHANNEL_ONE,
	MOV_CHANNEL_TWO,
	MOV_CHANNEL_THREE,
	MOV_CHANNEL_FOUR,
	MOV_CHANNEL_FIVE,
	MOV_CHANNEL_SIX,
	MOV_CHANNEL_SEVEN,
	MOV_CHANNEL_EIGHT
} MOV_STATE;

typedef enum
{
	FLASH_IDLE,		// EVENT_PROCESSING_IDLE
	FLASH_PRE,		// EVENT_PRE_PROCESSING (combine pre + int)
	FLASH_BODY_INT, // EVENT_CALCULATIONS
	FLASH_BODY,		// EVENT_POST_PROCESSING
	FLASH_CAL		// EVENT_STORAGE
} FLASH_MOV_STATE;	// EVENT_PROCESSING_STATE

typedef enum
{
	AVAILABLE = 0,
	EVENT_LOCK,
	CAL_PULSE_LOCK
} SPI_LOCK_STATE;
typedef union
{
	uint16 sampleWord[8];
	uint8  sampleByte[16];
} ISPI_PACKET;

enum
{
	A_CHAN_OFFSET = 0,
	R_CHAN_OFFSET = 1,
	V_CHAN_OFFSET = 2,
	T_CHAN_OFFSET = 3
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitDataBuffs(uint8 op_mode);
uint16 CalcSumFreq(uint16*, uint16);
uint16 FixDataToZero(uint16 data_);

void ProcessWaveformData(void);
void MoveWaveformEventToFlash(void);

void ProcessManuelCalPulse(void);
void MoveManuelCalToFlash(void);

#endif //_DATABUFFS_H_
