///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
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
#define MAX_SAMPLE_RATE				16384

#define SAMPLE_RATE_512		512
#define SAMPLE_RATE_1K		1024
#define SAMPLE_RATE_2K		2048
#define SAMPLE_RATE_4K		4096
#define SAMPLE_RATE_8K		8192
#define SAMPLE_RATE_16K		16384
#define SAMPLE_RATE_DEFAULT	1024

#define MAX_NUM_OF_CHANNELS			4
#define CHANNEL_DATA_IN_BYTES		2
#define MAX_CAL_SAMPLES				100
#define START_CAL_SIGNAL			(100 + 1)
#define MAX_DATA_PER_SAMPLE			(MAX_NUM_OF_CHANNELS * CHANNEL_DATA_IN_BYTES) // 8
#define MAX_DATA_PER_SECOND			(MAX_SAMPLE_RATE * MAX_DATA_PER_SAMPLE) // 64K

#define LARGEST_PRETIRGGER_SIZE_IN_BYTES	(MAX_DATA_PER_SECOND)
#define LARGEST_EVENT_SIZE_IN_BYTES			(MAX_DATA_PER_SECOND * 55) // Determined by max data and ram storage available
#define LARGEST_CAL_SIZE_IN_BYTES			(MAX_DATA_PER_SAMPLE * MAX_CAL_SAMPLES) // 1600

#define PRE_TRIG_BUFF_SIZE_IN_BYTES 	(LARGEST_PRETIRGGER_SIZE_IN_BYTES + MAX_DATA_PER_SAMPLE) // Max 1 second + 1 sample
#define PRE_TRIG_BUFF_SIZE_IN_WORDS 	(PRE_TRIG_BUFF_SIZE_IN_BYTES / 2)

#define EVENT_BUFF_SIZE_IN_BYTES    (LARGEST_EVENT_SIZE_IN_BYTES + LARGEST_PRETIRGGER_SIZE_IN_BYTES + LARGEST_CAL_SIZE_IN_BYTES)
#define EVENT_BUFF_SIZE_IN_WORDS    (((EVENT_BUFF_SIZE_IN_BYTES / sizeof(SAMPLE_DATA_STRUCT)) * sizeof(SAMPLE_DATA_STRUCT)) / 2)

typedef enum
{
	SUCCESS      	= 0x0000,
	EVT_BUF_FULL 	= 0x0001,
	SUMM_TBL_FULL 	= 0x0002
} FULL_FLAGS;

#if 0 // Removed
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
#endif

typedef enum
{
	FREQ_VALID_PEAK 			= 4,
	FREQ_CROSSOVER_BACKWARD 	= 2,
	FREQ_CROSSOVER_FORWARD 		= 1
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
	WAVE_INIT,
	WAVE_PRETRIG,
	WAVE_BODY_INIT,
	WAVE_BODY,
	WAVE_CAL_PULSE,
	WAVE_CALCULATE,
	WAVE_STORE,
	WAVE_COMPLETE
} WAVE_PROCESSING_STATE;

enum
{
	AVAILABLE = 1,
	EVENT_LOCK,
	CAL_PULSE_LOCK,
	FILE_LOCK,
	RTC_TIME_LOCK
};

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

enum
{
	ACCURACY_16_BIT = 16,
	ACCURACY_14_BIT = 14,
	ACCURACY_12_BIT = 12,
	ACCURACY_10_BIT = 10
};

#define AD_BIT_ACCURACY ACCURACY_16_BIT

#define ACCURACY_16_BIT_MIDPOINT 0x8000
#define ACCURACY_14_BIT_MIDPOINT 0x2000
#define ACCURACY_12_BIT_MIDPOINT 0x800
#define ACCURACY_10_BIT_MIDPOINT 0x200

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitDataBuffs(uint8 op_mode);
uint16 CalcSumFreq(uint16* dataPtr, uint32 sampleRate, uint16* startAddrPtr, uint16* endAddrPtr);
uint16 FixDataToZero(uint16 data_);

void ProcessWaveformData(void);
void MoveWaveformEventToFile(void);

void MoveManualCalToFile(void);

#endif //_DATABUFFS_H_
