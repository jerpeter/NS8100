///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _ANALOG_H_
#define _ANALOG_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Summary.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define ANALOG_R_CHANNEL_SELECT		0x02
#define ANALOG_V_CHANNEL_SELECT		0x00
#define ANALOG_T_CHANNEL_SELECT		0x06
#define ANALOG_A_CHANNEL_SELECT		0x04
#define ANALOG_CONTROL_DATA			0x01
#define ANALOG_CONTROL_SHIFT		0x02
#define ANALOG_CONTROL_STORAGE		0x04

#define SAMPLE_RATE_1K_PIT_DIVIDER		2
#define SAMPLE_RATE_2K_PIT_DIVIDER		4
#define SAMPLE_RATE_4K_PIT_DIVIDER		8
#define SAMPLE_RATE_8K_PIT_DIVIDER		16
#define SAMPLE_RATE_16K_PIT_DIVIDER		32
#define SAMPLE_RATE_32K_PIT_DIVIDER		64
#define SAMPLE_RATE_64K_PIT_DIVIDER		128
#define SAMPLE_RATE_PIT_MODULUS			62500

#define CAL_SAMPLE_COUNT_FIRST_TRANSITION_HIGH	95
#define CAL_SAMPLE_COUNT_SECOND_TRANSITION_LOW	85
#define CAL_SAMPLE_COUNT_THIRD_TRANSITION_HIGH	65
#define CAL_SAMPLE_COUNT_FOURTH_TRANSITION_OFF	55

#define AD_SPI_0_CHIP_SELECT	0

#define AD_SPI					(&AVR32_SPI0)
#define AD_CTL_SPI				(&AVR32_SPI1)

#define AD_SPI_NPCS             0
#define AD_CTL_SPI_NPCS			3

#define GAIN_SELECT_x2	0x01
#define GAIN_SELECT_x4	0x00

#define CONSECUTIVE_TRIGGERS_THRESHOLD 2
#define CONSEC_EVENTS_WITHOUT_CAL_THRESHOLD 2 // Treating event + event as 1 consecutive, event + event + event as 2 consecutive
#define PENDING	2 // Anything above 1

#if 0 // Normal
#define AD_TEMP_COUNT_FOR_ADJUSTMENT	4
#else // Test new count
#define AD_TEMP_COUNT_FOR_ADJUSTMENT	16
#endif

enum {
	DEFAULT_CAL_BUFFER_INDEX = 0,
	ONCE_DELAYED_CAL_BUFFER_INDEX = 1,
	TWICE_DELAYED_CAL_BUFFER_INDEX = 2
};

enum {
	SEISMIC_GAIN_LOW,
	SEISMIC_GAIN_HIGH,
	ACOUSTIC_GAIN_NORMAL,
	ACOUSTIC_GAIN_A_WEIGHTED
};

enum {
	ANALOG_CUTOFF_FREQ_LOW,	// Filters ~500 HZ and above 
	ANALOG_CUTOFF_FREQ_1,	// Filters ~1000 HZ and above 
	ANALOG_CUTOFF_FREQ_2,	// Filters ~2000 HZ and above 
	ANALOG_CUTOFF_FREQ_3,	// Filters ~4000 HZ and above 
	ANALOG_CUTOFF_FREQ_4	// Filters ~14000 HZ and above 
};

enum {
	FOUR_AD_CHANNELS_WITH_READBACK_WITH_TEMP,
	FOUR_AD_CHANNELS_NO_READBACK_WITH_TEMP,
	FOUR_AD_CHANNELS_NO_READBACK_NO_TEMP
};

typedef union
{
	struct
	{
		bitfield calSignalEnable:			1;
		bitfield calSignal:					1;
		bitfield unused:					1;
		bitfield cutoffFreqSelectEnable:	1;
		bitfield cutoffFreqSelectHi:		1;
		bitfield cutoffFreqSelectLow:		1;
		bitfield acousticGainSelect:		1;
		bitfield seismicGainSelect:			1;
	} bit; 

	uint8 reg;
} ANALOG_CONTROL_STRUCT;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void GetAnalogConfigReadback(void);
void ReadAnalogData(SAMPLE_DATA_STRUCT* dataPtr);
void InitAnalogControl(void);
void WriteAnalogControl(uint16 data);
void SetAnalogCutoffFrequency(uint8 freq);
void SetSeismicGainSelect(uint8 seismicGain);
void SetAcousticGainSelect(uint8 acousticGain);
void SetCalSignalEnable(uint8 enable);
void SetCalSignal(uint8 data);
void GenerateCalSignal(void);
void GetChannelOffsets(uint32 sampleRate);
void UpdateChannelOffsetsForTempChange(void);
void GatherSampleData(void);
void adSetCalSignalLow(void);
void adSetCalSignalHigh(void);
void adSetCalSignalOff(void);

#endif //_ANALOG_H_
