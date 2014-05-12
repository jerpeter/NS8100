///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#ifndef _SENSOR_H_
#define _SENSOR_H_


///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Common.h"
#include "Uart.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define SEISMIC_SENSOR	0x08 // Port bit
#define ACOUSTIC_SENSOR	0x10 // Port bit
#define READ_SENSOR		0x04 // Port bit

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void OneWireInit(void);
uint8 OneWireReset(uint8 sensor);
void OneWireWriteByte(uint8 sensor, uint8 dataByte);
uint8 OneWireReadByte(uint8 sensor);
void OneWireTest(uint8 sensor);
void OneWireFunctions(uint8 sensor);
uint8 OneWireReadROM(uint8 sensor);
uint8 OneWireReadMemory(uint8 sensor, uint8 address, uint8 length, uint8* data);
uint8 OneWireWriteScratchpad(uint8 sensor, uint8 address, uint8 length, uint8* data);
uint8 OneWireReadScratchpad(uint8 sensor, uint8 address, uint8 length, uint8* data);
uint8 OneWireCopyScratchpad(uint8 sensor);
uint8 OneWireWriteAppRegister(uint8 sensor, uint8 address, uint8 length, uint8* data);
uint8 OneWireReadStatusRegister(uint8 sensor, uint8* data);
uint8 OneWireReadAppRegister(uint8 sensor, uint8 address, uint8 length, uint8* data);
uint8 OneWireCopyAndLockAppRegister(uint8 sensor);

#endif //_SENSOR_H_
