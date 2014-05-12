///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Ispi.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:50 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Ispi.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _ISPI_H_
#define _ISPI_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define SPI_CONTROL_ONE_REG_ADDR			0x00CB0000
#define SPI_CONTROL_TWO_REG_ADDR			0x00CB0001
#define SPI_BAUD_RATE_REG_ADDR				0x00CB0002
#define SPI_STATUS_REG_ADDR					0x00CB0003
#define SPI_DATA_REG_ADDR					0x00CB0005
#define SPI_PULLUP_REDUCED_DRV_REG_ADDR		0x00CB0006
#define SPI_PORT_DATA_REG_ADDR				0x00CB0007
#define SPI_PORT_DATA_DIR_REG_ADDR			0x00CB0008

#define IDLE_DATA 							0x0A
#define SAMPLE_BUF_SIZE  					750

enum
{
	// ISPI code defines
	ISPI_BEGIN_FLAG 	= 0xFE,
	ISPI_END_FLAG   	= 0xFE,
	ISPI_TRIGGER_FLAG 	= 0xFD,
	ISPI_CONTROL_BYTE 	= 0x01
};

	// CR1 register
#define ISPI_IRQ_ENABLE_BIT                  0x80
#define ISPI_SYSTEM_ENABLE_BIT               0x40
#define ISPI_WIRED_OR_MODE_BIT               0x20
#define ISPI_MASTER_BIT                      0x10
#define ISPI_CLOCK_POLARITY_BIT              0x08
#define ISPI_CLOCK_PHASE_BIT                 0x04
#define ISPI_SLAVE_SELECT_OUTPUT_ENA_BIT 	 0x02
#define ISPI_LSB_FIRST_ENABLE_BIT            0x01
	// CR2 register
#define ISPI_STOP_IN_DOZE_BIT                0x02
#define ISPI_SERIAL_PIN_CONTROL_BIT          0x01
	// SR register
#define ISPI_IRQ_BIT                         0x80
#define ISPI_WRITE_COLLISION_BIT             0x40
#define ISPI_MODE_FAULT_BIT                  0x10
	// PURD register
#define ISPI_REDUCED_DRIVE_CAPABILITY_BIT    0x10
#define ISPI_PULLUP_DEVICES_ENABLE_BIT       0x01
	// DDR register
#define ISPI_DDRSP3_BIT                      0x08
    
typedef enum
{
	ISPI_SLAVE_MODE,
	ISPI_MASTER_MODE
} OPERATIONAL_MODE_E;
    
typedef enum
{
	ISPI_LSB_FIRST,
	ISPI_MSB_FIRST
} BIT_XMIT_FIRST_MODE_E;
    
typedef enum
{
	ISPI_MODE_FAULT_INPUT,
	ISPI_GENERAL_PURPOSE_INPUT,
	ISPI_GENERAL_PURPOSE_OUTPUT,
	ISPI_SLAVE_SELECT_OUTPUT,
	ISPI_NA // Not applicatable is used in slave mode
} SLAVE_SELECT_OUTPUT_MODE_E;
    
typedef enum
{
	ISPI_IDLE,
	ISPI_RECEIVE,
	ISPI_SEND,
	ISPI_RECV_AND_SEND
} ISPI_STATE_E;

typedef union
{
	uint16 wordData;
	uint8  byteData[2];
} WORD_BYTE_UNION;
    
void ISPI_WriteISPI(uint8 data_);
uint16 ISPI_ReadISPI(void);
void ISPI_XmitTerminated(void);
void ISPI_IRQEnable(bool enabled_);
void ISPI_SystemEnable(bool enabled_);
void ISPI_OperationalMode(OPERATIONAL_MODE_E mode_);
void ISPI_BitXmitFirstMode(BIT_XMIT_FIRST_MODE_E mode_);
void ISPI_StopInDozeMode(bool enabled_);
void ISPI_BaudRate(uint8 baud_);
void ISPI_SlaveSelectOutputMode(SLAVE_SELECT_OUTPUT_MODE_E mode_);
void ISPI_ClearIRQ_Flag(void);
void ISPI_SetISPI_State(ISPI_STATE_E state_);
    
/*******************************************************************************
*  Prototypes
*******************************************************************************/
void ISPI_Init(void);
uint8 RetriveISPI_Data(uint16* data_);
uint32 ISPI_SendMsg(uint8 firstByte);
void ISPI_MsgComplete(void);
void ISPI_PollIRQ_Flag(void);

#endif // _ISPI_H_
