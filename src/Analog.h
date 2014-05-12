///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Analog.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:06 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/Analog.h,v $
///	$Revision: 1.1 $
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

enum {
	SEISMIC_GAIN_LOW,
	SEISMIC_GAIN_HI,
	ACOUSTIC_GAIN_NORMAL,
	ACOUSTIC_GAIN_A_WEIGHTED
};

enum {
	ANALOG_CUTOFF_FREQ_1,
	ANALOG_CUTOFF_FREQ_2,
	ANALOG_CUTOFF_FREQ_3,
	ANALOG_CUTOFF_FREQ_4
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
void ReadAnalogData(SAMPLE_DATA_STRUCT* dataPtr);
void InitAnalogControl(void);
void WriteAnalogControl(uint8 data);
void SetAnalogCutoffFrequency(uint8 freq);
void SetSeismicGainSelect(uint8 seismicGain);
void SetAcousticGainSelect(uint8 acousticGain);
void SetCalSignalEnable(uint8 enable);
void SetCalSignal(uint8 data);
void GenerateCalSignal(void);
void GetChannelOffsets(void);
void GatherSampleData(void);

#endif //_ANALOG_H_
