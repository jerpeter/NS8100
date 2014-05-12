///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Analog.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:46 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Analog.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Old_Board.h"
#include "Common.h"
#include "Analog.h"
#include "Summary.h"
#include "Record.h"
#include "InitDataBuffers.h"
#include "PowerManagement.h"
#include "Uart.h"
#include "Menu.h"
#include "rtc.h"
#include "tc.h"
#include "twi.h"
#include "spi.h"
#include "ad_test_menu.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static SAMPLE_DATA_STRUCT s_tempData;
static uint32 s_updateOffsetCount;
static uint32 s_rTotal;
static uint32 s_vTotal;
static uint32 s_tTotal;
static uint32 s_aTotal;
static uint32 s_rCount;
static uint32 s_vCount;
static uint32 s_tCount;
static uint32 s_aCount;

///----------------------------------------------------------------------------
///	Analog information
///----------------------------------------------------------------------------

// INA1 - R channel
// INA2 - T channel
// INB1 - V channel
// INB2 - A channel

// CNVST (A.L.) | A0 | A/B (A.L.) |    Read		| Address Select
// -------------------------------------------------------------
//	inactive	| 0  |  	1	  | INA1 (R)	|	0010 - 0x2
//	active		| 0  |  	0	  |	INB1 (V)	|	0000 - 0x0
//	inactive	| 1  |  	1	  | INA2 (T)	|	0110 - 0x6
//	active		| 1  |  	0	  |	INB2 (A)	|	0100 - 0x4
//--------------------------------------------------------------
// Processor Pin| A2 |	   A1	  |

///----------------------------------------------------------------------------
///	Function:	ReadAnalogData
///	Purpose:
///----------------------------------------------------------------------------
void ReadAnalogData(SAMPLE_DATA_STRUCT* dataPtr)
{
	uint16 channelConfigReadback;

#if 1
	if (g_adChannelConfig == FOUR_AD_CHANNELS_WITH_READBACK_AND_TEMP)
	{
		// Chan 0
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->r));
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &channelConfigReadback);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		if(channelConfigReadback != 0xe0d0) { /* Error */ }

		// Chan 1
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->t));
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &channelConfigReadback);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		if(channelConfigReadback != 0xe2d0) { /* Error */ }

		// Chan 2
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->v));
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &channelConfigReadback);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		if(channelConfigReadback != 0xe4d0) { /* Error */ }

		// Chan 3
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->a));
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &channelConfigReadback);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		if(channelConfigReadback != 0xe6d0) { /* Error */ }

		// Temp
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &g_currentTempReading);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &channelConfigReadback);
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		if(channelConfigReadback != 0xb6d0) { /* Error */ }
	}
	else // FOUR_AD_CHANNELS_NO_READBACK_NO_TEMP
	{
		// Chan 0
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->r));
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);

		// Chan 1
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->t));
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);

		// Chan 2
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->v));
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);

		// Chan 3
		spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
		spi_write(&AVR32_SPI0, 0x0000);	spi_read(&AVR32_SPI0, &(dataPtr->a));
		spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
	}	
#endif

#if 0
	uint8 delay = 0;

	// Set A0 mux to A1/B1 for R/V by clearing bit
	reg_PORTE.reg &= ~ANALOG_CONTROL_SHIFT;
	soft_usecWait(delay);

	// Set low to start conversion
	reg_PORTE.reg &= ~ANALOG_CONTROL_DATA;
	soft_usecWait(delay);
	reg_PORTE.reg |= ANALOG_CONTROL_DATA;
	soft_usecWait(delay);

	// Read R and V data
	dataPtr->r = *(uint16*)(ANALOG_ADDRESS + 0x02);
	dataPtr->v = *(uint16*)(ANALOG_ADDRESS + 0x00);

	soft_usecWait(delay);

	// Set A0 mux to A2/B2 for T/A by setting bit
	reg_PORTE.reg |= ANALOG_CONTROL_SHIFT;
	soft_usecWait(delay);

	// Set low to start conversion
	reg_PORTE.reg &= ~ANALOG_CONTROL_DATA;
	soft_usecWait(delay);
	reg_PORTE.reg |= ANALOG_CONTROL_DATA;
	soft_usecWait(delay);

	dataPtr->t = *(uint16*)(ANALOG_ADDRESS + 0x02);
	dataPtr->a = *(uint16*)(ANALOG_ADDRESS + 0x00);

	dataPtr->r -= g_channelOffset.r_16bit;
	dataPtr->v -= g_channelOffset.v_16bit;
	dataPtr->t -= g_channelOffset.t_16bit;
	dataPtr->a -= g_channelOffset.a_16bit;
#endif

#if 0
	dataPtr->r = *((uint16*)(ANALOG_ADDRESS | ANALOG_R_CHANNEL_SELECT));
	dataPtr->v = *((uint16*)(ANALOG_ADDRESS | ANALOG_V_CHANNEL_SELECT));

	// Delay 2.25 us before signaling another conversion

	dataPtr->t = *((uint16*)(ANALOG_ADDRESS | ANALOG_T_CHANNEL_SELECT));
	dataPtr->a = *((uint16*)(ANALOG_ADDRESS | ANALOG_A_CHANNEL_SELECT));
#endif
}

///----------------------------------------------------------------------------
///	Function:	InitAnalogControl
///	Purpose:
///----------------------------------------------------------------------------
void InitAnalogControl(void)
{
	g_analogControl.reg = 0x0;

	SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1);
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL);
}

///----------------------------------------------------------------------------
///	Function:	WriteADConfig
///	Purpose:
///----------------------------------------------------------------------------
void WriteADConfig(unsigned int config)
{
	spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
	spi_write(&AVR32_SPI0, ((unsigned short) config << 2));
	spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
}

///----------------------------------------------------------------------------
///	Function:	SetupADChannelConfig
///	Purpose:
///----------------------------------------------------------------------------
void SetupADChannelConfig(uint32 sampleRate)
{
	if (sampleRate <= 8192)
	{
		// Setup config for 4 Chan + Temp + Read back config + others
		WriteADConfig(0x39B4);
		
		g_adChannelConfig = FOUR_AD_CHANNELS_WITH_READBACK_AND_TEMP;
	}
	else // Sample rates above 8192 take too long to read back config and temp, so skip them
	{
		// Setup config for 4 Chan, No Temp, No read back
		WriteADConfig(0x39FF);

		g_adChannelConfig = FOUR_AD_CHANNELS_NO_READBACK_NO_TEMP;
	}

	//Delay for 1.2us at least
	soft_usecWait(2);

	spi_selectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);
	spi_write(&AVR32_SPI0, 0x0000);
    spi_unselectChip(&AVR32_SPI0, AD_SPI_0_CHIP_SELECT);

	soft_usecWait(2);
}

///----------------------------------------------------------------------------
///	Function:	WriteAnalogControl
///	Purpose:
///----------------------------------------------------------------------------
void WriteAnalogControl(uint16 control)
{
#if 0 // ns7100
	uint8 i = 0;
	uint8 mask = 0x80;

	// Make sure sclk bit is low
	reg_PORTE.reg &= ~ANALOG_CONTROL_SHIFT;

	// Write byte into shift register (Propogates D0 -> D7)
	for (i = 0; i < 8; i++)
	{
		if (data & mask)
		{
			reg_PORTE.reg |= ANALOG_CONTROL_DATA;
		}
		else
		{
			reg_PORTE.reg &= ~ANALOG_CONTROL_DATA;
		}

		// Clock a bit into the shift register by rising edge of shift pin
		reg_PORTE.reg |= ANALOG_CONTROL_SHIFT;
		reg_PORTE.reg &= ~ANALOG_CONTROL_SHIFT;

		mask >>= 1;
	}

	// Move shift register contents into storage register by rising edge of storage pin
	reg_PORTE.reg |= ANALOG_CONTROL_STORAGE;
	reg_PORTE.reg &= ~ANALOG_CONTROL_STORAGE;
#else // ns8100
	spi_selectChip(&AVR32_SPI1, AD_CTL_SPI_NPCS);
	spi_write(&AVR32_SPI1, (unsigned short) control);
    spi_unselectChip(&AVR32_SPI1, AD_CTL_SPI_NPCS);
#endif
}

///----------------------------------------------------------------------------
///	Function:	adSetCalSignalLow
///	Purpose:
///----------------------------------------------------------------------------
void adSetCalSignalLow(void)
{
	g_analogControl.bit.calSignal = 0;
	g_analogControl.bit.calSignalEnable = 1;

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	adSetCalSignalHigh
///	Purpose:
///----------------------------------------------------------------------------
void adSetCalSignalHigh(void)
{
	g_analogControl.bit.calSignal = 1;
	g_analogControl.bit.calSignalEnable = 1;

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	adSetCalSignalOff
///	Purpose:
///----------------------------------------------------------------------------
void adSetCalSignalOff(void)
{
	g_analogControl.bit.calSignal = 0;
	g_analogControl.bit.calSignalEnable = 0;

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetAnalogCutoffFrequency
///	Purpose:
///----------------------------------------------------------------------------
void SetAnalogCutoffFrequency(uint8 freq)
{
	// Validated bit selection 8/20/2012
	
	switch (freq)
	{
		case ANALOG_CUTOFF_FREQ_LOW: // 500 Hz
			g_analogControl.bit.cutoffFreqSelectEnable = 1;
		break;

		case ANALOG_CUTOFF_FREQ_1: // 1K Hz
			g_analogControl.bit.cutoffFreqSelectLow = 1;
			g_analogControl.bit.cutoffFreqSelectHi = 1;
			g_analogControl.bit.cutoffFreqSelectEnable = 0;
		break;

		case ANALOG_CUTOFF_FREQ_2: // 2K Hz
			g_analogControl.bit.cutoffFreqSelectLow = 0;
			g_analogControl.bit.cutoffFreqSelectHi = 1;
			g_analogControl.bit.cutoffFreqSelectEnable = 0;
		break;

		case ANALOG_CUTOFF_FREQ_3: // 4K Hz
			g_analogControl.bit.cutoffFreqSelectLow = 1;
			g_analogControl.bit.cutoffFreqSelectHi = 0;
			g_analogControl.bit.cutoffFreqSelectEnable = 0;
		break;

		case ANALOG_CUTOFF_FREQ_4: // 14.3K Hz
			g_analogControl.bit.cutoffFreqSelectLow = 0;
			g_analogControl.bit.cutoffFreqSelectHi = 0;
			g_analogControl.bit.cutoffFreqSelectEnable = 0;
		break;
	}

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetSeismicGainSelect
///	Purpose:
///----------------------------------------------------------------------------
void SetSeismicGainSelect(uint8 seismicGain)
{
	if(seismicGain == SEISMIC_GAIN_LOW)
	{
		g_analogControl.bit.seismicGainSelect = 0;
	}
	else // seismicGain == SEISMIC_GAIN_HIGH
	{
		g_analogControl.bit.seismicGainSelect = 1;
	}

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetAcousticGainSelect
///	Purpose:
///----------------------------------------------------------------------------
void SetAcousticGainSelect(uint8 acousticGain)
{
	if(acousticGain == ACOUSTIC_GAIN_NORMAL)
	{
		g_analogControl.bit.acousticGainSelect = 0;
	}
	else // seismicGain == ACOUSTIC_GAIN_A_WEIGHTED
	{
		g_analogControl.bit.acousticGainSelect = 1;
	}

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetCalSignalEnable
///	Purpose:
///----------------------------------------------------------------------------
void SetCalSignalEnable(uint8 enable)
{
	if(enable == ON)
	{
		g_analogControl.bit.calSignalEnable = 1;
	}
	else // enable == OFF
	{
		g_analogControl.bit.calSignalEnable = 0;
	}

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetCalSignal
///	Purpose:
///----------------------------------------------------------------------------
void SetCalSignal(uint8 data)
{
	if(data)
	{
		g_analogControl.bit.calSignal = 1;
	}
	else // data == NULL
	{
		g_analogControl.bit.calSignal = 0;
	}

	WriteAnalogControl(g_analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	GenerateCalSignal
///	Purpose:
///----------------------------------------------------------------------------
void GenerateCalSignal(void)
{
	// Previous NS7100 timing was:
	// 1) Enable cal (cut off real channels) and delay 5ms
	// 2) Drive reference high for 10ms
	// 3) Drive reference low for 20ms
	// 4) Drive reference high for 10ms
	// 5) Disable cal and delay for 55ms and then off

	SetCalSignalEnable(ON);
	soft_usecWait(5 * SOFT_MSECS);
	SetCalSignal(ON);
	soft_usecWait(10 * SOFT_MSECS);
	SetCalSignal(OFF);
	soft_usecWait(20 * SOFT_MSECS);
	SetCalSignal(ON);
	soft_usecWait(10 * SOFT_MSECS);
	SetCalSignalEnable(OFF);
	soft_usecWait(55 * SOFT_MSECS);
}

///----------------------------------------------------------------------------
///	Function:	GetChannelOffsets
///	Purpose:
///----------------------------------------------------------------------------
void GetChannelOffsets(uint32 sampleRate)
{
	uint32 i = 0;
	uint32 timeDelay = (977 / (g_triggerRecord.trec.sample_rate / 512) / 2);
	uint8 powerAnalogDown = NO;

	// Check to see if the A/D is in sleep mode
	if (getPowerControlState(ANALOG_SLEEP_ENABLE) == ON)
	{
		// Power the A/D on to set the offsets
		powerControl(ANALOG_SLEEP_ENABLE, OFF);

		// Set flag to signal powering off the A/D when finished
		powerAnalogDown = YES;
	}

	// Reset offset values
	byteSet(&g_channelOffset, 0, sizeof(OFFSET_DATA_STRUCT));

	debug("Get Channel Offset: Read and pitch... (Address boundary: %s)\n", ((uint32)(&s_tempData) % 4 == 0) ? "YES" : "NO");
	// Read and pitch samples
	for (i = 0; i < (g_triggerRecord.trec.sample_rate * 2); i++)
	{
		ReadAnalogData(&s_tempData);

		//debug("Offset throw away data: 0x%x, 0x%x, 0x%x, 0x%x\n", s_tempData.r, s_tempData.v, s_tempData.t, s_tempData.a);

		// Delay equivalent to the time in between gathering samples for the current sample rate
		soft_usecWait(timeDelay);
	}

	// Initialize
	s_rTotal = 0;
	s_vTotal = 0;
	s_tTotal = 0;
	s_aTotal = 0;

	debug("Get Channel Offset: 1st Pass Read and sum...\n");
	// Read and sum samples
	for (i = 0; i < g_triggerRecord.trec.sample_rate; i++)
	{
		ReadAnalogData(&s_tempData);

		//debug("Offset sum data: 0x%x, 0x%x, 0x%x, 0x%x\n", s_tempData.r, s_tempData.v, s_tempData.t, s_tempData.a);

		s_rTotal += s_tempData.r;
		s_vTotal += s_tempData.v;
		s_tTotal += s_tempData.t;
		s_aTotal += s_tempData.a;

		// Delay equivalent to the time in between gathering samples for the current sample rate
		soft_usecWait(timeDelay);
	}

	// Average out the summations
	s_rTotal /= g_triggerRecord.trec.sample_rate;
	s_vTotal /= g_triggerRecord.trec.sample_rate;
	s_tTotal /= g_triggerRecord.trec.sample_rate;
	s_aTotal /= g_triggerRecord.trec.sample_rate;

	// Set the channel offsets
	g_channelOffset.r_offset = (int16)(s_rTotal - ACCURACY_16_BIT_MIDPOINT);
	g_channelOffset.v_offset = (int16)(s_vTotal - ACCURACY_16_BIT_MIDPOINT);
	g_channelOffset.t_offset = (int16)(s_tTotal - ACCURACY_16_BIT_MIDPOINT);
	g_channelOffset.a_offset = (int16)(s_aTotal - ACCURACY_16_BIT_MIDPOINT);

	debug("A/D Channel First Pass channel average: 0x%x, 0x%x, 0x%x, 0x%x\n", s_rTotal, s_vTotal, s_tTotal, s_aTotal);
	debug("A/D Channel First Pass channel offsets: %d, %d, %d, %d\n", g_channelOffset.r_offset, g_channelOffset.v_offset, g_channelOffset.t_offset, g_channelOffset.a_offset);

	// Seed totals to ensure count is non-zero (count initialized to 1 for this reason)
	s_rTotal = g_channelOffset.r_offset;
	s_vTotal = g_channelOffset.v_offset;
	s_tTotal = g_channelOffset.t_offset;
	s_aTotal = g_channelOffset.a_offset;
	
	// Seed counts
	s_rCount = 1;
	s_vCount = 1;
	s_tCount = 1;
	s_aCount = 1;

	debug("Get Channel Offset: 2nd Pass Read and sum...\n");
	// Read and sum samples
	for (i = 0; i < g_triggerRecord.trec.sample_rate; i++)
	{
		ReadAnalogData(&s_tempData);

		//debug("Offset sum data: 0x%x, 0x%x, 0x%x, 0x%x\n", s_tempData.r, s_tempData.v, s_tempData.t, s_tempData.a);

		if (((s_tempData.r - g_channelOffset.r_offset) > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
			(((s_tempData.r - g_channelOffset.r_offset) < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
		{
			s_rTotal += s_tempData.r;
			s_rCount++;
		}

		if (((s_tempData.v - g_channelOffset.v_offset) > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
			(((s_tempData.v - g_channelOffset.v_offset) < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
		{
			s_vTotal += s_tempData.v;
			s_vCount++;
		}

		if (((s_tempData.t - g_channelOffset.t_offset) > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
			(((s_tempData.t - g_channelOffset.t_offset) < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
		{
			s_tTotal += s_tempData.t;
			s_tCount++;
		}

		if (((s_tempData.a - g_channelOffset.a_offset) > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
			(((s_tempData.a - g_channelOffset.a_offset) < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
		{
			s_aTotal += s_tempData.a;
			s_aCount++;
		}

		// Delay equivalent to the time in between gathering samples for the current sample rate
		soft_usecWait(timeDelay);
	}

	// Average out the summations
	s_rTotal /= s_rCount;
	s_vTotal /= s_vCount;
	s_tTotal /= s_tCount;
	s_aTotal /= s_aCount;

	// Set the channel offsets
	g_channelOffset.r_offset = (int16)(s_rTotal - ACCURACY_16_BIT_MIDPOINT);
	g_channelOffset.v_offset = (int16)(s_vTotal - ACCURACY_16_BIT_MIDPOINT);
	g_channelOffset.t_offset = (int16)(s_tTotal - ACCURACY_16_BIT_MIDPOINT);
	g_channelOffset.a_offset = (int16)(s_aTotal - ACCURACY_16_BIT_MIDPOINT);

	debug("A/D Channel Second Pass channel average: 0x%x, 0x%x, 0x%x, 0x%x\n", s_rTotal, s_vTotal, s_tTotal, s_aTotal);
	debug("A/D Channel Second Pass channel offsets: %d, %d, %d, %d\n", g_channelOffset.r_offset, g_channelOffset.v_offset, g_channelOffset.t_offset, g_channelOffset.a_offset);

	// Establish a baseline temperature to compare while monitoring
	g_storedTempReading = g_currentTempReading;

	// If we had to power on the A/D, then power it off
	if (powerAnalogDown == YES)
	{
		powerControl(ANALOG_SLEEP_ENABLE, ON);
	}		
}

///----------------------------------------------------------------------------
///	Function:	UpdateChannelOffsetsForTempChange
///	Purpose:
///----------------------------------------------------------------------------
void UpdateChannelOffsetsForTempChange(void)
{
	// Make sure system isn't processing an event, not handling any system events but UPDATE_OFFSET_EVENT and not handling a manual cal pulse
	if ((g_busyProcessingEvent == NO) && (anySystemEventExcept(UPDATE_OFFSET_EVENT) == NO) && (g_triggerRecord.op_mode != MANUAL_CAL_MODE))
	{
		if (s_updateOffsetCount)
		{
			// Get data
			s_tempData.r = ((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->r;
			s_tempData.v = ((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->v;
			s_tempData.t = ((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->t;
			s_tempData.a = ((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->a;
		
			// Only capture samples that fall within the minimum trigger band (filtering spikes and seismic activity)
			if ((s_tempData.r > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
				((s_tempData.r < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
			{
				s_rTotal += s_tempData.r;
				s_rCount++;
			}

			if ((s_tempData.v > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
				((s_tempData.v < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
			{
				s_vTotal += s_tempData.v;
				s_vCount++;
			}

			if ((s_tempData.t > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
				((s_tempData.t < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
			{
				s_tTotal += s_tempData.t;
				s_tCount++;
			}

			if ((s_tempData.a > (ACCURACY_16_BIT_MIDPOINT - SEISMIC_TRIGGER_MIN_VALUE)) && 
				((s_tempData.a < (ACCURACY_16_BIT_MIDPOINT + SEISMIC_TRIGGER_MIN_VALUE))))
			{
				s_aTotal += s_tempData.a;
				s_aCount++;
			}
			
			// Done gathering offset data
			if (--s_updateOffsetCount == 0)
			{
				// Average out the summations
				s_rTotal /= s_rCount;
				s_vTotal /= s_vCount;
				s_tTotal /= s_tCount;
				s_aTotal /= s_aCount;

				debug("Temp change - A/D Channel offsets (old): %d, %d, %d, %d\n", g_channelOffset.r_offset, g_channelOffset.v_offset, g_channelOffset.t_offset, g_channelOffset.a_offset);

				// Set the channel offsets
				g_channelOffset.r_offset = (int16)(s_rTotal - ACCURACY_16_BIT_MIDPOINT);
				g_channelOffset.v_offset = (int16)(s_vTotal - ACCURACY_16_BIT_MIDPOINT);
				g_channelOffset.t_offset = (int16)(s_tTotal - ACCURACY_16_BIT_MIDPOINT);
				g_channelOffset.a_offset = (int16)(s_aTotal - ACCURACY_16_BIT_MIDPOINT);

				debug("Temp change - A/D Channel offsets (new): %d, %d, %d, %d\n", g_channelOffset.r_offset, g_channelOffset.v_offset, g_channelOffset.t_offset, g_channelOffset.a_offset);
				
				clearSystemEventFlag(UPDATE_OFFSET_EVENT);
			}
		}
		else // Start processing counter for new zero crossing
		{
			// Initialize the counter for checking samples
			s_updateOffsetCount = g_triggerRecord.trec.sample_rate;
			
			// Seed totals to ensure count is non-zero (count initialized to 1 for this reason)
			s_rTotal = g_channelOffset.r_offset;
			s_vTotal = g_channelOffset.v_offset;
			s_tTotal = g_channelOffset.t_offset;
			s_aTotal = g_channelOffset.a_offset;
			
			// Seed the counts
			s_rCount = 1;
			s_vCount = 1;
			s_tCount = 1;
			s_aCount = 1;
		}
	}
}

///----------------------------------------------------------------------------
///	Function:	GatherSampleData
///	Purpose:
///----------------------------------------------------------------------------
void GatherSampleData(void)
{
	uint8 trigFound = NO, alarm1Found = NO, alarm2Found = NO;
	static uint32 sampleCount = 0;
	static uint32 calSampleCount = 0;
	static uint8 recording = NO;
	static uint8 calPulse = NO;

	//debug("Tail of Quarter Sec Buffer: 0x%x\n", g_tailOfQuarterSecBuff);

	//debug("Read A/D Data\n");
	ReadAnalogData((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff);

	//debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x\n", *(g_tailOfQuarterSecBuff + 0), *(g_tailOfQuarterSecBuff + 1), *(g_tailOfQuarterSecBuff + 2), *(g_tailOfQuarterSecBuff + 3));

	// Convert the data to 12 bits to fit the format of the previous event record
	*(g_tailOfQuarterSecBuff + 0) >>= 4;
	*(g_tailOfQuarterSecBuff + 1) >>= 4;
	*(g_tailOfQuarterSecBuff + 2) >>= 4;
	*(g_tailOfQuarterSecBuff + 3) >>= 4;

	//debug("A/D Data Shifted: 0x%x, 0x%x, 0x%x, 0x%x\n", *(g_tailOfQuarterSecBuff + 0), *(g_tailOfQuarterSecBuff + 1), *(g_tailOfQuarterSecBuff + 2), *(g_tailOfQuarterSecBuff + 3));

	// Check if handling a manual calibration
	if (g_manualCalFlag == FALSE)
	{
		// Check if not recording an event and not handling a cal pulse
		if ((recording == NO) && (calPulse == NO))
		{
			if (*(g_tailOfQuarterSecBuff + 1) > ACCURACY_16_BIT_MIDPOINT)
			{
				if ((*(g_tailOfQuarterSecBuff + 1) - ACCURACY_16_BIT_MIDPOINT) > g_triggerRecord.trec.seismicTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 1)) > g_triggerRecord.trec.seismicTriggerLevel) trigFound = YES;
			}

			if (*(g_tailOfQuarterSecBuff + 2) > ACCURACY_16_BIT_MIDPOINT)
			{
				if ((*(g_tailOfQuarterSecBuff + 2) - ACCURACY_16_BIT_MIDPOINT) > g_triggerRecord.trec.seismicTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 2)) > g_triggerRecord.trec.seismicTriggerLevel) trigFound = YES;
			}

			if (*(g_tailOfQuarterSecBuff + 3) > ACCURACY_16_BIT_MIDPOINT)
			{
				if ((*(g_tailOfQuarterSecBuff + 3) - ACCURACY_16_BIT_MIDPOINT) > g_triggerRecord.trec.seismicTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 3)) > g_triggerRecord.trec.seismicTriggerLevel) trigFound = YES;
			}

			if (*(g_tailOfQuarterSecBuff + 0) > ACCURACY_16_BIT_MIDPOINT)
			{
				if ((*(g_tailOfQuarterSecBuff + 0) - ACCURACY_16_BIT_MIDPOINT) > g_airTriggerCount) trigFound = YES;
			}
			else
			{
				if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 0)) > g_airTriggerCount) trigFound = YES;
			}

			if ((trigFound == YES) && (recording == NO) && (calPulse == NO))
			{
				// Signal a tigger

				recording = YES;
				sampleCount = g_triggerRecord.trec.record_time * g_triggerRecord.trec.sample_rate;
			}
		}
		// Else check if we are still recording
		else if ((recording == YES) && (sampleCount))
		{
			sampleCount--;

			// Check if seismic is enabled for Alarm 1
			if ((g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH) || (g_helpRecord.alarm_one_mode == ALARM_MODE_SEISMIC))
			{
				if (*(g_tailOfQuarterSecBuff + 1) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 1) - ACCURACY_16_BIT_MIDPOINT) > g_helpRecord.alarm_one_seismic_lvl) alarm1Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 1)) > g_helpRecord.alarm_one_seismic_lvl) alarm1Found = YES;
				}

				if (*(g_tailOfQuarterSecBuff + 2) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 2) - ACCURACY_16_BIT_MIDPOINT) > g_helpRecord.alarm_one_seismic_lvl) alarm1Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 2)) > g_helpRecord.alarm_one_seismic_lvl) alarm1Found = YES;
				}

				if (*(g_tailOfQuarterSecBuff + 3) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 3) - ACCURACY_16_BIT_MIDPOINT) > g_helpRecord.alarm_one_seismic_lvl) alarm1Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 3)) > g_helpRecord.alarm_one_seismic_lvl) alarm1Found = YES;
				}
			}

			// Check if air is enabled for Alarm 1
			if ((g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH) || (g_helpRecord.alarm_one_mode == ALARM_MODE_AIR))
			{
				if (*(g_tailOfQuarterSecBuff + 0) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 0) - ACCURACY_16_BIT_MIDPOINT) > g_alarm1AirTriggerCount) alarm1Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 0)) > g_alarm1AirTriggerCount) alarm1Found = YES;
				}
			}

			// Check if seismic is enabled for Alarm 2
			if ((g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH) || (g_helpRecord.alarm_two_mode == ALARM_MODE_SEISMIC))
			{
				if (*(g_tailOfQuarterSecBuff + 1) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 1) - ACCURACY_16_BIT_MIDPOINT) > g_helpRecord.alarm_two_seismic_lvl) alarm2Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 1)) > g_helpRecord.alarm_two_seismic_lvl) alarm2Found = YES;
				}

				if (*(g_tailOfQuarterSecBuff + 2) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 2) - ACCURACY_16_BIT_MIDPOINT) > g_helpRecord.alarm_two_seismic_lvl) alarm2Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 2)) > g_helpRecord.alarm_two_seismic_lvl) alarm2Found = YES;
				}

				if (*(g_tailOfQuarterSecBuff + 3) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 3) - ACCURACY_16_BIT_MIDPOINT) > g_helpRecord.alarm_two_seismic_lvl) alarm2Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 3)) > g_helpRecord.alarm_two_seismic_lvl) alarm2Found = YES;
				}
			}

			// Check if air is enabled for Alarm 2
			if ((g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH) || (g_helpRecord.alarm_two_mode == ALARM_MODE_SEISMIC))
			{
				if (*(g_tailOfQuarterSecBuff + 0) > ACCURACY_16_BIT_MIDPOINT)
				{
					if ((*(g_tailOfQuarterSecBuff + 0) - ACCURACY_16_BIT_MIDPOINT) > g_alarm2AirTriggerCount) alarm2Found = YES;
				}
				else
				{
					if ((ACCURACY_16_BIT_MIDPOINT - *(g_tailOfQuarterSecBuff + 0)) > g_alarm2AirTriggerCount) alarm2Found = YES;
				}
			}

			// Check if Alarm 2 condition was met (Alarm 2 overrides Alarm 1)
			if (alarm2Found == YES)
			{
				// Signal an alarm
			}
			// Else check if Alarm 1 condition was met
			else if (alarm1Found == YES)
			{
				// Signal an alarm
			}
		}
		// Else check if we are still recording but handling the last sample
		else if ((recording == YES) && (sampleCount == 0))
		{
			recording = NO;
			calPulse = YES;
			calSampleCount = 100 * (g_triggerRecord.trec.sample_rate / 1024);

			SetCalSignalEnable(ON);
		}
		// Else check if a cal pulse is enabled
		else if ((calPulse == YES) && (calSampleCount))
		{
			if (calSampleCount == (100 * (g_triggerRecord.trec.sample_rate / 1024)))
			{
				SetCalSignalEnable(ON);
				SetCalSignal(ON);
			}

			if (calSampleCount == (91 * (g_triggerRecord.trec.sample_rate / 1024)))
				SetCalSignal(OFF);

			if (calSampleCount == (73 * (g_triggerRecord.trec.sample_rate / 1024)))
				SetCalSignal(ON);

			if (calSampleCount == (64 * (g_triggerRecord.trec.sample_rate / 1024)))
				SetCalSignalEnable(OFF);

			calSampleCount--;
		}
		else if ((calPulse == YES) && (calSampleCount == 0))
		{
			calPulse = NO;
		}
	}
	else //g_manualCalFlag == TRUE
	{
		g_manualCalSampleCount++;
	}
}
