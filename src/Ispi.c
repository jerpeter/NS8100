///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Ispi.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:50 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Ispi.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Ispi.h"
#include "Msgs430.h"
#include "MMC2114.h"
#include "Mmc2114_InitVals.h"
#include "SysEvents.h"
#include "Uart.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
volatile ISPI_STATE_E  _ISPI_State;
volatile uint8* _SPICR1  = (uint8*)SPI_CONTROL_ONE_REG_ADDR;
volatile uint8* _SPICR2  = (uint8*)SPI_CONTROL_TWO_REG_ADDR;
volatile uint8* _SPIBR   = (uint8*)SPI_BAUD_RATE_REG_ADDR;
volatile uint8* _SPISR   = (uint8*)SPI_STATUS_REG_ADDR;
volatile uint8* _SPIDR   = (uint8*)SPI_DATA_REG_ADDR;
volatile uint8* _SPIPURD = (uint8*)SPI_PULLUP_REDUCED_DRV_REG_ADDR;
volatile uint8* _SPIPORT = (uint8*)SPI_PORT_DATA_REG_ADDR;
volatile uint8* _SPIDDR  = (uint8*)SPI_PORT_DATA_DIR_REG_ADDR;

///----------------------------------------------------------------------------
///	Function:	ISPI_PollIRQ_Flag
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_PollIRQ_Flag(void)
{
	while ((*_SPISR & ISPI_IRQ_BIT) != ISPI_IRQ_BIT) {}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_WriteISPI
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_WriteISPI(uint8 data_)
{
	*_SPIDR = data_;
}

///----------------------------------------------------------------------------
///	Function:	ISPI_ReadISPI
///	Purpose:	
///----------------------------------------------------------------------------
uint16 ISPI_ReadISPI(void)
{
	return ((uint16)*_SPIDR);
}

///----------------------------------------------------------------------------
///	Function:	ISPI_XmitTerminated
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_XmitTerminated(void)
{
	// Empty
}

///----------------------------------------------------------------------------
///	Function:	ISPI_SystemEnable
///	Purpose:	
///----------------------------------------------------------------------------
inline void ISPI_SystemEnable(bool enable_)
{
	if (enable_ == TRUE)
	{
		*_SPICR1 |= ISPI_SYSTEM_ENABLE_BIT;
		*_SPIDDR = 0xFF;
	}
	else
	{
		*_SPICR1 &= ~ISPI_SYSTEM_ENABLE_BIT;
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_IRQEnable
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_IRQEnable(bool enabled_)
{
	if (enabled_ == TRUE)
	{
		*_SPICR1 |= ISPI_IRQ_ENABLE_BIT;
	}
	else
	{
		*_SPICR1 &= ~ISPI_IRQ_ENABLE_BIT;
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_StopInDozeMode
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_StopInDozeMode(bool enable_)
{
	if (enable_ == TRUE)
	{
		*_SPICR2 |= ISPI_STOP_IN_DOZE_BIT;
	}
	else
	{
		*_SPICR2 &= ~ISPI_STOP_IN_DOZE_BIT;
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_OperationalMode
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_OperationalMode(OPERATIONAL_MODE_E mode_)
{ 
	if (mode_ == ISPI_SLAVE_MODE)
	{
		*_SPICR1 &= ~ISPI_MASTER_BIT;
	}
	else
	{
		*_SPICR1 |= ISPI_MASTER_BIT;
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_BaudRate
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_BaudRate(uint8 baud_)
{
	*_SPIBR = baud_;
}

///----------------------------------------------------------------------------
///	Function:	ISPI_ClearIRQ_Flag
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_ClearIRQ_Flag(void)
{
	uint8 temp;

	temp = *_SPISR;
	temp = (uint8)ISPI_ReadISPI();
}

///----------------------------------------------------------------------------
///	Function:	ISPI_BitXmitFirstMode
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_BitXmitFirstMode(BIT_XMIT_FIRST_MODE_E mode_)
{
	if (mode_ == ISPI_LSB_FIRST)
	{
		*_SPICR1 |= ISPI_LSB_FIRST_ENABLE_BIT;
	}
	else
	{
		*_SPICR1 &= ~ISPI_LSB_FIRST_ENABLE_BIT;
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_SetISPI_State
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_SetISPI_State(ISPI_STATE_E state_)
{
	_ISPI_State = state_;
}

///----------------------------------------------------------------------------
///	Function:	ISPI_SlaveSelectOutputMode
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_SlaveSelectOutputMode(SLAVE_SELECT_OUTPUT_MODE_E slaveSelectOutputMode_)
{
	switch (slaveSelectOutputMode_)
	{
		case ISPI_MODE_FAULT_INPUT:
			*_SPICR1 &= ~ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT;
			*_SPIDDR &= ~ISPI_DDRSP3_BIT;
		break;

		case ISPI_GENERAL_PURPOSE_INPUT:
			*_SPICR1 |= ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT;
			*_SPIDDR &= ~ISPI_DDRSP3_BIT;
		break;

		case ISPI_GENERAL_PURPOSE_OUTPUT:
			*_SPICR1 &= ~ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT;
			*_SPIDDR |= ~ISPI_DDRSP3_BIT;
		break;

		case ISPI_SLAVE_SELECT_OUTPUT:
			*_SPICR1 |= ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT;
			*_SPIDDR |= ~ISPI_DDRSP3_BIT;
		break;

		default:
			// ISPI_NA will go through here
		break;     	  
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_Init
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_Init(void)
{
	OPERATIONAL_MODE_E operMode_ = ISPI_MASTER_MODE;
	uint8 baud_ = 0x01;
    BIT_XMIT_FIRST_MODE_E bitXmitMode_ = ISPI_MSB_FIRST;
	bool stopInDozeMode_ = TRUE;
	SLAVE_SELECT_OUTPUT_MODE_E slaveSelectOutputMode_ = ISPI_GENERAL_PURPOSE_OUTPUT;
	bool systemEnable_ = TRUE;
	bool irqEnable_ = FALSE;
	bool clkPhaseOpposite_ = FALSE;
	bool clkPolarityInverted_ = FALSE;
	bool reducedDriveEnabled_ = FALSE;
	bool pullupDevicesEnabled_ = FALSE;
	bool pinDriveOpenDrain_ = FALSE;
	bool serialPinBidirectional_ = FALSE;

	_ISPI_State = ISPI_IDLE;

	// Start off by disabling the SPI (in case it was configured or used in the bootloader)
	*_SPICR1 &= ~(ISPI_SYSTEM_ENABLE_BIT);

	//-------------------------------------------------------------
	//*_SPIDDR = 0xFF;

	//-------------------------------------------------------------
	if(bitXmitMode_ == ISPI_LSB_FIRST)
		*_SPICR1 |= ISPI_LSB_FIRST_ENABLE_BIT;
	else
		*_SPICR1 &= ~(ISPI_LSB_FIRST_ENABLE_BIT);

	//-------------------------------------------------------------
	switch(slaveSelectOutputMode_)
	{
		case ISPI_MODE_FAULT_INPUT:
			*_SPICR1 &= ~(ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT);
			*_SPIDDR &= ~(ISPI_DDRSP3_BIT);
		break;

		case ISPI_GENERAL_PURPOSE_INPUT:
			*_SPICR1 |= ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT;
			*_SPIDDR &= ~(ISPI_DDRSP3_BIT);
		break;

		case ISPI_GENERAL_PURPOSE_OUTPUT:
			*_SPICR1 &= ~(ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT);
			*_SPIDDR = 0x0E;
			*_SPIPORT = 0x08;
		break;

		case ISPI_SLAVE_SELECT_OUTPUT:
			*_SPICR1 |= ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT;
			*_SPIDDR |= ~(ISPI_DDRSP3_BIT);
		break;

		default:
			// ISPI_NA will go through here
		break;      	  
	}

	//-------------------------------------------------------------
	if(clkPhaseOpposite_ == TRUE)
		*_SPICR1 |= ISPI_CLOCK_PHASE_BIT;
	else
		*_SPICR1 &= ~(ISPI_CLOCK_PHASE_BIT);

	//-------------------------------------------------------------
	if(clkPolarityInverted_ == TRUE)
		*_SPICR1 |= ISPI_CLOCK_POLARITY_BIT;
	else
		*_SPICR1 &= ~(ISPI_CLOCK_POLARITY_BIT);

	//-------------------------------------------------------------
	if(operMode_ == ISPI_MASTER_MODE)
		*_SPICR1 |= ISPI_MASTER_BIT;
	else
		*_SPICR1 &= ~(ISPI_MASTER_BIT);

	//-------------------------------------------------------------
	if(pinDriveOpenDrain_ == TRUE)
		*_SPICR1 |= ISPI_WIRED_OR_MODE_BIT;
	else
		*_SPICR1 &= ~(ISPI_WIRED_OR_MODE_BIT);

	//-------------------------------------------------------------
	if(irqEnable_ == TRUE)
		*_SPICR1 |= ISPI_IRQ_ENABLE_BIT;
	else
		*_SPICR1 &= ~(ISPI_IRQ_ENABLE_BIT);

	//-------------------------------------------------------------
	if(stopInDozeMode_ == TRUE)
		*_SPICR2 |= ISPI_STOP_IN_DOZE_BIT;
	else
		*_SPICR2 &= ~(ISPI_STOP_IN_DOZE_BIT);

	//-------------------------------------------------------------
	if(serialPinBidirectional_ == TRUE)
		*_SPICR2 |= ISPI_SERIAL_PIN_CONTROL_BIT; 
	else
		*_SPICR2 &= ~(ISPI_SERIAL_PIN_CONTROL_BIT);

	//-------------------------------------------------------------
	*_SPIBR = baud_;

	//-------------------------------------------------------------
	if(reducedDriveEnabled_ == TRUE)
		*_SPIPURD |= ISPI_REDUCED_DRIVE_CAPABILITY_BIT;
	else
		*_SPIPURD &= ~(ISPI_REDUCED_DRIVE_CAPABILITY_BIT);

	//-------------------------------------------------------------
	if(pullupDevicesEnabled_ == TRUE)
		*_SPIPURD |= ISPI_PULLUP_DEVICES_ENABLE_BIT;
	else
		*_SPIPURD &= ~(ISPI_PULLUP_DEVICES_ENABLE_BIT);

	//-------------------------------------------------------------
	if(systemEnable_ == TRUE)
		*_SPICR1 |= ISPI_SYSTEM_ENABLE_BIT;
	else
		*_SPICR1 &= ~(ISPI_SYSTEM_ENABLE_BIT);

	//-------------------------------------------------------------
	// Dummy read
#if 0 // fix_ns8100
	uint8 data;

	data = reg_SPISR.reg;
	data = reg_SPIDR;
#endif
}

///----------------------------------------------------------------------------
///	Function:	RetriveISPI_Data
///	Purpose:	
///----------------------------------------------------------------------------
uint8 RetriveISPI_Data(uint16* data_)
{
	extern MSGS430_UNION msgs430;
	static uint16* msgPtr = (uint16*)&msgs430;
	static uint8 LSB = TRUE;
	uint8 emptyFlag;

	if (*msgPtr != 0xFFFF)
	{
		if (LSB == TRUE)
		{
			*data_ = (uint16)(*msgPtr++ & 0x00FF);
			LSB = FALSE;
		}
		else
		{
			*data_ = (uint16)((*msgPtr >> 8) & 0x00FF);
			LSB = TRUE;
		}

		emptyFlag = FALSE;
	}
	else
	{
		LSB = TRUE;
		*data_ = IDLE_DATA;
		msgPtr = (uint16*)&msgs430;
		emptyFlag = TRUE;
	}

	return (emptyFlag);
}

///----------------------------------------------------------------------------
///	Function:	ISPI_SendMsg
///	Purpose:	
///----------------------------------------------------------------------------
uint32 ISPI_SendMsg(uint8 cmdId)  
{
	extern MSGS430_UNION msgs430;
	WORD_BYTE_UNION* msgPtr;
	uint8 scratch;

	if (ISPI_GetISPI_State() == ISPI_IDLE)
	{
		switch (cmdId)
		{
			case START_TRIGGER_CMD:
				msgPtr = (WORD_BYTE_UNION*)&msgs430;

				while (msgPtr->wordData != 0xFFFF)
				{
					scratch = (uint8)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT);
					ISPI_WriteISPI(msgPtr->byteData[0]);

					while (scratch == ((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT)) {}

					scratch = (uint8)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT);
					ISPI_WriteISPI(msgPtr->byteData[1]);

					while (scratch == ((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT)) {}
					msgPtr++;
				}
			break;

			case STOP_TRIGGER_CMD:
			case ZERO_SENSORS_CMD:
			case MANUAL_CAL_PULSE_CMD:
				scratch = (uint8)((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT);
				ISPI_WriteISPI(cmdId);

				while (scratch == ((*(volatile uint8*)SPI_PORT_DATA_REG_ADDR) & ISPI_DDRSP3_BIT)) {}
			break;
		}

		ISPI_ClearIRQ_Flag();

		return (0); // PASSED
	}
	else
	{
		ISPI_ClearIRQ_Flag();
		return (1); // FAILED
	}
}

///----------------------------------------------------------------------------
///	Function:	ISPI_MsgComplete
///	Purpose:	
///----------------------------------------------------------------------------
void ISPI_MsgComplete(void)
{
	// Empty
}
