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
#include "Mmc2114.h"
#include "Analog.h"
#include "Summary.h"
#include "Rec.h"
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
extern uint16* startOfPreTrigBuff;
extern uint16* tailOfPreTrigBuff;
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;
extern uint16* endOfPreTrigBuff;
extern REC_EVENT_MN_STRUCT trig_rec;
extern uint16 manual_cal_flag;
extern uint16 manualCalSampleCount;
extern REC_HELP_MN_STRUCT help_rec;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
ANALOG_CONTROL_STRUCT analogControl;
OFFSET_DATA_STRUCT g_channelOffset;

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
	uint16 trash;
#if 1
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &(dataPtr->r));
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &trash);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);

    // Chan 1
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &(dataPtr->v));
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &trash);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);

    // Chan 2
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &(dataPtr->t));
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &trash);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);

    // Chan 3
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &(dataPtr->a));
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &trash);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);

    // Temp
    spi_selectChip(AD_SPI, AD_SPI_NPCS);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &trash);
    spi_write(AD_SPI, 0x0000);
    spi_read(AD_SPI, &trash);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);
#endif

#if 0 // fix_ns8100
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

	dataPtr->r -= g_channelOffset.r;
	dataPtr->v -= g_channelOffset.v;
	dataPtr->t -= g_channelOffset.t;
	dataPtr->a -= g_channelOffset.a;
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
	SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1);
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL);
}

///----------------------------------------------------------------------------
///	Function:	WriteAnalogControl
///	Purpose:
///----------------------------------------------------------------------------
void WriteAnalogControl(uint8 data)
{
#if 0 // fix_ns8100
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
#endif
}

///----------------------------------------------------------------------------
///	Function:	SetAnalogCutoffFrequency
///	Purpose:
///----------------------------------------------------------------------------
void SetAnalogCutoffFrequency(uint8 freq)
{
	switch (freq)
	{
		case ANALOG_CUTOFF_FREQ_1:
			analogControl.bit.cutoffFreqSelectLow = 0;
			analogControl.bit.cutoffFreqSelectHi = 0;
		break;

		case ANALOG_CUTOFF_FREQ_2:
			analogControl.bit.cutoffFreqSelectLow = 1;
			analogControl.bit.cutoffFreqSelectHi = 0;
		break;

		case ANALOG_CUTOFF_FREQ_3:
			analogControl.bit.cutoffFreqSelectLow = 0;
			analogControl.bit.cutoffFreqSelectHi = 1;
		break;

		case ANALOG_CUTOFF_FREQ_4:
			analogControl.bit.cutoffFreqSelectLow = 1;
			analogControl.bit.cutoffFreqSelectHi = 1;
		break;
	}

	// Set enable to decode channel pin selection
	analogControl.bit.cutoffFreqSelectEnable = 1;
	WriteAnalogControl(analogControl.reg);

	// Clear enable to make channel pins a don't care
	analogControl.bit.cutoffFreqSelectEnable = 0;
	WriteAnalogControl(analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetSeismicGainSelect
///	Purpose:
///----------------------------------------------------------------------------
void SetSeismicGainSelect(uint8 seismicGain)
{
	if(seismicGain == SEISMIC_GAIN_LOW)
	{
		analogControl.bit.seismicGainSelect = 0;
	}
	else // seismicGain == SEISMIC_GAIN_HI
	{
		analogControl.bit.seismicGainSelect = 1;
	}

	WriteAnalogControl(analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetAcousticGainSelect
///	Purpose:
///----------------------------------------------------------------------------
void SetAcousticGainSelect(uint8 acousticGain)
{
	if(acousticGain == ACOUSTIC_GAIN_NORMAL)
	{
		analogControl.bit.acousticGainSelect = 0;
	}
	else // seismicGain == ACOUSTIC_GAIN_A_WEIGHTED
	{
		analogControl.bit.acousticGainSelect = 1;
	}

	WriteAnalogControl(analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetCalSignalEnable
///	Purpose:
///----------------------------------------------------------------------------
void SetCalSignalEnable(uint8 enable)
{
	if(enable == ON)
	{
		analogControl.bit.calSignalEnable = 1;
	}
	else // enable == OFF
	{
		analogControl.bit.calSignalEnable = 0;
	}

	WriteAnalogControl(analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	SetCalSignal
///	Purpose:
///----------------------------------------------------------------------------
void SetCalSignal(uint8 data)
{
	if(data)
	{
		analogControl.bit.calSignal = 1;
	}
	else // data == NULL
	{
		analogControl.bit.calSignal = 0;
	}

	WriteAnalogControl(analogControl.reg);
}

///----------------------------------------------------------------------------
///	Function:	GenerateCalSignal
///	Purpose:
///----------------------------------------------------------------------------
void GenerateCalSignal(void)
{
	// Previous NS7100 timing was:
	// 1) Enable cal (cut off real channels)
	// 2) Drive reference high for 9ms
	// 3) Drive reference low for 18ms
	// 4) Drive reference high for 9ms
	// 5) Disable cal and delay for 64ms and then off

	SetCalSignalEnable(ON);

	SetCalSignal(ON);
	soft_usecWait(9 * SOFT_MSECS);
	SetCalSignal(OFF);
	soft_usecWait(18 * SOFT_MSECS);
	SetCalSignal(ON);
	soft_usecWait(9 * SOFT_MSECS);

	SetCalSignalEnable(OFF);

	soft_usecWait(64 * SOFT_MSECS);
}

///----------------------------------------------------------------------------
///	Function:	GetChannelOffsets
///	Purpose:
///----------------------------------------------------------------------------
void GetChannelOffsets(void)
{
	uint8 i = 0;
	SAMPLE_DATA_STRUCT tempData;
	uint8 powerAnalogDown = NO;
	uint32 delay = 100 * (8192 / trig_rec.trec.sample_rate);
	uint32 rTotal = 0;
	uint32 vTotal = 0;
	uint32 tTotal = 0;
	uint32 aTotal = 0;

	// Check to see if the A/D is already powered on
	if (getPowerControlState(ANALOG_SLEEP_ENABLE) == YES)
	{
		// Power the A/D on to set the offsets
		powerControl(ANALOG_SLEEP_ENABLE, OFF);

		// Set flag to signal powering off the A/D when finished
		powerAnalogDown = YES;
	}

	// Reset offset values
	g_channelOffset.r = 0;
	g_channelOffset.v = 0;
	g_channelOffset.t = 0;
	g_channelOffset.a = 0;

	// Read and pitch 50 samples
	for (i=0;i<50;i++)
	{
		ReadAnalogData(&tempData);

		//debug("Offset throw away data: 0x%x, 0x%x, 0x%x, 0x%x\n", tempData.r, tempData.v, tempData.t, tempData.a);

		// Delay equivalent to the time in between gathering samples for the current sample rate
		soft_usecWait(delay);
	}

	// Read and sum 50 samples
	for (i=0;i<50;i++)
	{
		ReadAnalogData(&tempData);

		//debug("Offset sum data: 0x%x, 0x%x, 0x%x, 0x%x\n", tempData.r, tempData.v, tempData.t, tempData.a);

		rTotal += tempData.r;
		vTotal += tempData.v;
		tTotal += tempData.t;
		aTotal += tempData.a;

		// Delay equivalent to the time in between gathering samples for the current sample rate
		soft_usecWait(delay);
	}

	// Average out the summations
	rTotal /= 50;
	vTotal /= 50;
	tTotal /= 50;
	aTotal /= 50;

	debug("A/D Channel offset average: 0x%x, 0x%x, 0x%x, 0x%x\n", rTotal, vTotal, tTotal, aTotal);

	// Set the channel offsets
	g_channelOffset.r = (int16)(rTotal - 0x8000);
	g_channelOffset.v = (int16)(vTotal - 0x8000);
	g_channelOffset.t = (int16)(tTotal - 0x8000);
	g_channelOffset.a = (int16)(aTotal - 0x8000);

	debug("A/D Channel offsets: %d, %d, %d, %d\n", g_channelOffset.r, g_channelOffset.v, g_channelOffset.t, g_channelOffset.a);

	// If we had to power on the A/D, then power it off
	if (powerAnalogDown == YES)
		powerControl(ANALOG_SLEEP_ENABLE, ON);
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

	//debug("Tail of Pretrigger: 0x%x\n", tailOfPreTrigBuff);

	//debug("Read A/D Data\n");
	ReadAnalogData((SAMPLE_DATA_STRUCT*)tailOfPreTrigBuff);

	//debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x\n", *(tailOfPreTrigBuff + 0), *(tailOfPreTrigBuff + 1), *(tailOfPreTrigBuff + 2), *(tailOfPreTrigBuff + 3));

	// Convert the data to 12 bits to fit the format of the previous event record
	*(tailOfPreTrigBuff + 0) >>= 4;
	*(tailOfPreTrigBuff + 1) >>= 4;
	*(tailOfPreTrigBuff + 2) >>= 4;
	*(tailOfPreTrigBuff + 3) >>= 4;

	//debug("A/D Data Shifted: 0x%x, 0x%x, 0x%x, 0x%x\n", *(tailOfPreTrigBuff + 0), *(tailOfPreTrigBuff + 1), *(tailOfPreTrigBuff + 2), *(tailOfPreTrigBuff + 3));

	// Check if handling a manual calibration
	if (manual_cal_flag == FALSE)
	{
		// Check if not recording an event and not handling a cal pulse
		if ((recording == NO) && (calPulse == NO))
		{
			if (*(tailOfPreTrigBuff + 1) > 0x0800)
			{
				if ((*(tailOfPreTrigBuff + 1) - 0x800) > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((0x800 - *(tailOfPreTrigBuff + 1)) > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
			}

			if (*(tailOfPreTrigBuff + 2) > 0x0800)
			{
				if ((*(tailOfPreTrigBuff + 2) - 0x800) > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((0x800 - *(tailOfPreTrigBuff + 2)) > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
			}

			if (*(tailOfPreTrigBuff + 3) > 0x0800)
			{
				if ((*(tailOfPreTrigBuff + 3) - 0x800) > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((0x800 - *(tailOfPreTrigBuff + 3)) > trig_rec.trec.seismicTriggerLevel) trigFound = YES;
			}

			if (*(tailOfPreTrigBuff + 0) > 0x0800)
			{
				if ((*(tailOfPreTrigBuff + 0) - 0x800) > trig_rec.trec.soundTriggerLevel) trigFound = YES;
			}
			else
			{
				if ((0x800 - *(tailOfPreTrigBuff + 0)) > trig_rec.trec.soundTriggerLevel) trigFound = YES;
			}

			if ((trigFound == YES) && (recording == NO) && (calPulse == NO))
			{
				// Add command nibble to signal a tigger
				*(tailOfPreTrigBuff + 0) |= TRIG_ONE;
				*(tailOfPreTrigBuff + 1) |= TRIG_ONE;
				*(tailOfPreTrigBuff + 2) |= TRIG_ONE;
				*(tailOfPreTrigBuff + 3) |= TRIG_ONE;

				recording = YES;
				sampleCount = trig_rec.trec.record_time * trig_rec.trec.sample_rate;
			}
		}
		// Else check if we are still recording
		else if ((recording == YES) && (sampleCount))
		{
			sampleCount--;

			// Check if seismic is enabled for Alarm 1
			if ((help_rec.alarm_one_mode == ALARM_MODE_BOTH) || (help_rec.alarm_one_mode == ALARM_MODE_SEISMIC))
			{
				if (*(tailOfPreTrigBuff + 1) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 1) - 0x800) > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 1)) > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}

				if (*(tailOfPreTrigBuff + 2) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 2) - 0x800) > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 2)) > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}

				if (*(tailOfPreTrigBuff + 3) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 3) - 0x800) > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 3)) > help_rec.alarm_one_seismic_lvl) alarm1Found = YES;
				}
			}

			// Check if air is enabled for Alarm 1
			if ((help_rec.alarm_one_mode == ALARM_MODE_BOTH) || (help_rec.alarm_one_mode == ALARM_MODE_AIR))
			{
				if (*(tailOfPreTrigBuff + 0) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 0) - 0x800) > help_rec.alarm_one_air_lvl) alarm1Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 0)) > help_rec.alarm_one_air_lvl) alarm1Found = YES;
				}
			}

			// Check if seismic is enabled for Alarm 2
			if ((help_rec.alarm_two_mode == ALARM_MODE_BOTH) || (help_rec.alarm_two_mode == ALARM_MODE_SEISMIC))
			{
				if (*(tailOfPreTrigBuff + 1) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 1) - 0x800) > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 1)) > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}

				if (*(tailOfPreTrigBuff + 2) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 2) - 0x800) > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 2)) > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}

				if (*(tailOfPreTrigBuff + 3) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 3) - 0x800) > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 3)) > help_rec.alarm_two_seismic_lvl) alarm2Found = YES;
				}
			}

			// Check if air is enabled for Alarm 2
			if ((help_rec.alarm_two_mode == ALARM_MODE_BOTH) || (help_rec.alarm_two_mode == ALARM_MODE_SEISMIC))
			{
				if (*(tailOfPreTrigBuff + 0) > 0x0800)
				{
					if ((*(tailOfPreTrigBuff + 0) - 0x800) > help_rec.alarm_two_air_lvl) alarm2Found = YES;
				}
				else
				{
					if ((0x800 - *(tailOfPreTrigBuff + 0)) > help_rec.alarm_two_air_lvl) alarm2Found = YES;
				}
			}

			// Check if Alarm 2 condition was met (Alarm 2 overrides Alarm 1)
			if (alarm2Found == YES)
			{
				// Add command nibble to signal a tigger
				*(tailOfPreTrigBuff + 0) |= TRIG_THREE;
				*(tailOfPreTrigBuff + 1) |= TRIG_THREE;
				*(tailOfPreTrigBuff + 2) |= TRIG_THREE;
				*(tailOfPreTrigBuff + 3) |= TRIG_THREE;
			}
			// Else check if Alarm 1 condition was met
			else if (alarm1Found == YES)
			{
				// Add command nibble to signal a tigger
				*(tailOfPreTrigBuff + 0) |= TRIG_TWO;
				*(tailOfPreTrigBuff + 1) |= TRIG_TWO;
				*(tailOfPreTrigBuff + 2) |= TRIG_TWO;
				*(tailOfPreTrigBuff + 3) |= TRIG_TWO;
			}
		}
		// Else check if we are still recording but handling the last sample
		else if ((recording == YES) && (sampleCount == 0))
		{
			recording = NO;
			calPulse = YES;
			calSampleCount = 100 * (trig_rec.trec.sample_rate / 1024);

			// Start Cal
			*(tailOfPreTrigBuff + 0) |= CAL_START;
			*(tailOfPreTrigBuff + 1) |= CAL_START;
			*(tailOfPreTrigBuff + 2) |= CAL_START;
			*(tailOfPreTrigBuff + 3) |= CAL_START;

			SetCalSignalEnable(ON);
		}
		// Else check if a cal pulse is enabled
		else if ((calPulse == YES) && (calSampleCount))
		{
			if (calSampleCount == (100 * (trig_rec.trec.sample_rate / 1024)))
			{
				SetCalSignalEnable(ON);
				SetCalSignal(ON);
			}

			if (calSampleCount == (91 * (trig_rec.trec.sample_rate / 1024)))
				SetCalSignal(OFF);

			if (calSampleCount == (73 * (trig_rec.trec.sample_rate / 1024)))
				SetCalSignal(ON);

			if (calSampleCount == (64 * (trig_rec.trec.sample_rate / 1024)))
				SetCalSignalEnable(OFF);

			calSampleCount--;
		}
		else if ((calPulse == YES) && (calSampleCount == 0))
		{
			calPulse = NO;
		}
	}
	else //manual_cal_flag == TRUE
	{
		if (manualCalSampleCount == 0)
		{
			// Start Cal
			*(tailOfPreTrigBuff + 0) |= CAL_START;
			*(tailOfPreTrigBuff + 1) |= CAL_START;
			*(tailOfPreTrigBuff + 2) |= CAL_START;
			*(tailOfPreTrigBuff + 3) |= CAL_START;
		}

		if (manualCalSampleCount == 99)
		{
			// Stop Cal
			*(tailOfPreTrigBuff + 0) |= CAL_END;
			*(tailOfPreTrigBuff + 1) |= CAL_END;
			*(tailOfPreTrigBuff + 2) |= CAL_END;
			*(tailOfPreTrigBuff + 3) |= CAL_END;
		}

		manualCalSampleCount++;
	}
}
