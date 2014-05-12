///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Sensor.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:00 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Sensor.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"
#include "Board.h"
#include "Sensor.h"
#include "Uart.h"
#include "Crc.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Function:	OneWireInit
///	Purpose:	Init the port data and direction
///----------------------------------------------------------------------------
void OneWireInit(void)
{
	// Already handled in basic processor init
	// Set port data to one (idle state)
	// reg_PORTE.reg |= (SEISMIC_SENSOR | ACOUSTIC_SENSOR);
	// Set data direction to outputs
	// reg_DDRE.reg |= (SEISMIC_SENSOR | ACOUSTIC_SENSOR);
	// Set data direction to input
	// reg_DDRQB.reg &= ~READ_SENSOR;
}

///----------------------------------------------------------------------------
///	Function:	OneWireReset
///	Purpose:	Send a reset on the bus
///----------------------------------------------------------------------------
#if 0
uint8 OneWireReset(uint8 sensor)
{
	//    500  30 110 (us)
	// __       _     ________
	//   |     | |   |
	//   |     | |   |
	//    -----   ---

	uint8 presenceDetect = NO;

	// Set data direction to output to drive a 0
	//reg_DDRE.reg |= sensor;
	reg_PORTE.reg &= ~sensor;

	// Hold low for 500us
	//soft_usecWait(400);
	soft_usecWait(500);

	// Release line (allow pullup to take affect)
	//reg_DDRE.reg &= ~sensor;
	reg_PORTE.reg |= sensor;

	// Wait 30us + 50us (80us total)
	//soft_usecWait(64);
	soft_usecWait(80);

	if ((reg_PORTQB.reg & READ_SENSOR) == LOW)
	{
		presenceDetect = YES;
	}

	// Wait 100us make sure device is not driving the line
	//soft_usecWait(80);
	soft_usecWait(100);

	return (presenceDetect);
}
#endif

///----------------------------------------------------------------------------
///	Function:	OneWireWriteByte
///	Purpose:	Write a byte on the OneWire bus
///----------------------------------------------------------------------------
#if 0
void OneWireWriteByte(uint8 sensor, uint8 data)
{
	uint8 i;

	// Loop through all the bits starting with LSB
	for (i = 0; i <= 7; i++)
	{
		// Set data direction to output to drive a 0
		//reg_DDRE.reg |= sensor;
		reg_PORTE.reg &= ~sensor;

		// Check if the bit is a 1
		if (data & 0x01)
		{
			// Hold low for 5us
			//soft_usecWait(4);
			soft_usecWait(5);

			// Release the line
			//reg_DDRE.reg &= ~sensor;
			reg_PORTE.reg |= sensor;

			// Wait for 65us, recovery time
			//soft_usecWait(52);
			soft_usecWait(65);
		}
		else
		{
			// Hold low for 65us
			//soft_usecWait(52);
			soft_usecWait(65);

			// Release the line
			//reg_DDRE.reg &= ~sensor;
			reg_PORTE.reg |= sensor;

			// Wait for 5us, recovery time
			//soft_usecWait(4);
			soft_usecWait(5);
		}

		// Shift the data over 1 bit
		data >>= 1;
	}
}
#endif

///----------------------------------------------------------------------------
///	Function:	OneWireReadByte
///	Purpose:	Read a byte on the OneWire bus
///----------------------------------------------------------------------------
#if 0
uint8 OneWireReadByte(uint8 sensor)
{
	uint8 data = 0;
	uint8 i;

	// Loop through all the bits starting with LSB
	for (i = 0; i <= 7; i++)
	{
		// Set data direction to output to drive a 0
		//reg_DDRE.reg |= sensor;
		reg_PORTE.reg &= ~sensor;

		// Hold low for 5us
		//soft_usecWait(4);
		soft_usecWait(5);

		// Release the line
		//reg_DDRE.reg &= ~sensor;
		reg_PORTE.reg |= sensor;

		// Wait for 5us
		//soft_usecWait(4);
		soft_usecWait(5);

		// Shift the data over 1 bit
		data >>= 1;

		// Check if the data bit is a 1
		if (reg_PORTQB.reg & READ_SENSOR)
		{
			// Or in a 1
			data |= 0x80;
		}
		else
		{
			// And in a zero
			data &= 0x7f;
		}

		// Hold for 60us, recovery time
		//soft_usecWait(48);
		soft_usecWait(60);
	}

	return (data);
}
#endif

///----------------------------------------------------------------------------
///	Function:	OneWireTest
///	Purpose:
///----------------------------------------------------------------------------
void OneWireTest(uint8 sensor)
{
	uint8 romData[8];
	uint8 i = 0;
	uint8 crc = 0;

	if (OneWireReset(sensor) == YES)
	{
		OneWireWriteByte(sensor, 0x33);

		for (i = 0; i < 8; i++)
		{
			romData[i] = OneWireReadByte(sensor);
		}

		crc = calcCrc8(&romData[0], 7, 0x00);

		debugPrint(RAW, "\nOne Wire Rom Data: ");

		for (i = 0; i < 8; i++)
		{
			debugPrint(RAW, "0x%x ", romData[i]);
		}

		if (crc == romData[7])
		{
			debugPrint(RAW, "(CRC: %x, success)\n", crc);
			OneWireFunctions(sensor);
		}
		else
		{
			debugPrint(RAW, "(CRC: %x, fail)\n", crc);
		}
	}
	else
	{
		debugPrint(RAW, "\nOne Wire device not found!\n");
	}
}

///----------------------------------------------------------------------------
///	Function:	OneWireFunctions
///	Purpose:
///----------------------------------------------------------------------------
void OneWireFunctions(uint8 sensor)
{
	uint8 i = 0;

	// Read Memory (0xF0), Address: 0x00 -> 0x1F (wrap)
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Read Memory\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read Memory
		OneWireWriteByte(sensor, 0xF0);

		// Address
		OneWireWriteByte(sensor, 0x00);

		debugPrint(RAW, "  Data:");

		// Data
		for (i = 0; i < 32; i++)
			debugPrint(RAW, "%x ", OneWireReadByte(sensor));

		debugPrint(RAW, "\n");

		OneWireReset(sensor);
	}
	else return;

	// Write Scratchpad (0x0F), Address: 0x00 -> 0x1F (wrap)
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Write Scratchpad\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Write Scratchpad
		OneWireWriteByte(sensor, 0x0F);

		// Address
		OneWireWriteByte(sensor, 0x00);

		// Data
		for (i = 0; i < 32; i++)
		{
			OneWireWriteByte(sensor, (uint8)((i + 1) * 2));
		}

		OneWireReset(sensor);
	}
	else return;

	// Read Scratchpad (0xAA), Address: 0x00 -> 0x1F (wrap)
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Read Scratchpad\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read Scratchpad
		OneWireWriteByte(sensor, 0xAA);

		// Address
		OneWireWriteByte(sensor, 0x00);

		debugPrint(RAW, "  Data:");

		// Data
		for (i = 0; i < 32; i++)
			debugPrint(RAW, "%x ", OneWireReadByte(sensor));

		debugPrint(RAW, "\n");

		OneWireReset(sensor);
	}
	else return;

	// Copy Scratchpad (0x55), Validation key: 0xA5, Data line held for 10ms
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Copy Scratchpad\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Copy Scratchpad
		OneWireWriteByte(sensor, 0x55);

		// Validation Key
		OneWireWriteByte(sensor, 0xA5);

		soft_usecWait(10 * SOFT_MSECS);

		OneWireReset(sensor);
	}
	else return;

	// Write Application Register (0x99), Address: 0x00 -> 0x07 (wrap)
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Write App Register\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Write App Register
		OneWireWriteByte(sensor, 0x99);

		// Address
		OneWireWriteByte(sensor, 0x00);

		// Data
		for (i = 0; i < 8; i++)
		{
			OneWireWriteByte(sensor, (uint8)((i + 1) * 4));
		}

		OneWireReset(sensor);
	}
	else return;

	// Read Status Register (0x66), Validation key: 0x00
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Read Status Register\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read Status Register
		OneWireWriteByte(sensor, 0x66);

		// Validation key
		OneWireWriteByte(sensor, 0x00);

		debugPrint(RAW, "  Data:");

		// Data
		debugPrint(RAW, "%x\n", OneWireReadByte(sensor));

		OneWireReset(sensor);
	}
	else return;

	// Read Application Register (0xC3), Address: 0x00 -> 0x07 (wrap)
	if (OneWireReset(sensor) == YES)
	{
		debugPrint(RAW, "Read App Register\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read App Register
		OneWireWriteByte(sensor, 0xC3);

		// Address
		OneWireWriteByte(sensor, 0x00);

		debugPrint(RAW, "  Data:");

		// Data
		for (i = 0; i < 8; i++)
		{
			debugPrint(RAW, "%x ", OneWireReadByte(sensor));
		}

		debugPrint(RAW, "\n");

		OneWireReset(sensor);
	}
	else return;

	// Copy and Lock Application Register (0x5A), Validation key: 0xA5, Only executed once
}

///----------------------------------------------------------------------------
///	Function:	OneWireReadMemory
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireReadROM(uint8 sensor)
{
	uint8 status = FAILED;
	uint8 i = 0;
	uint8 romData[8];
	uint8 crc;

	// Read ROM
	if (OneWireReset(sensor) == YES)
	{
		// Read ROM command
		OneWireWriteByte(sensor, 0x33);

		for (i = 0; i < 8; i++)
		{
			romData[i] = OneWireReadByte(sensor);
		}

		crc = calcCrc8(&romData[0], 7, 0x00);

		debugPrint(RAW, "\nOne Wire Rom Data: ");

		for (i = 0; i < 8; i++)
		{
			debugPrint(RAW, "0x%x ", romData[i]);
		}

		if (crc == romData[7])
		{
			debugPrint(RAW, "(CRC: %x, success)\n", crc);
			OneWireFunctions(sensor);
		}
		else
		{
			debugPrint(RAW, "(CRC: %x, fail)\n", crc);
		}

		OneWireReset(sensor);

		status = PASSED;
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireReadMemory
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireReadMemory(uint8 sensor, uint8 address, uint8 length, uint8* data)
{
	uint8 status = FAILED;
	uint8 i = 0;

	if ((data != NULL) && (address <= 0x1F) && (length <= 32))
	{
		// Read Memory (0xF0), Address: 0x00 -> 0x1F (wrap)
		if (OneWireReset(sensor) == YES)
		{
			// Skip ROM
			OneWireWriteByte(sensor, 0xCC);

			// Read Memory
			OneWireWriteByte(sensor, 0xF0);

			// Address
			OneWireWriteByte(sensor, address);

			// Data
			for (i = 0; i < length; i++)
			{
				data[i] = OneWireReadByte(sensor);
			}

			OneWireReset(sensor);

			status = PASSED;
		}
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireWriteScratchpad
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireWriteScratchpad(uint8 sensor, uint8 address, uint8 length, uint8* data)
{
	uint8 status = FAILED;
	uint8 i = 0;

	if ((data != NULL) && (address <= 0x1F) && (length <= 32))
	{
		// Write Scratchpad (0x0F), Address: 0x00 -> 0x1F (wrap)
		if (OneWireReset(sensor) == YES)
		{
			// Skip ROM
			OneWireWriteByte(sensor, 0xCC);

			// Write Scratchpad
			OneWireWriteByte(sensor, 0x0F);

			// Address
			OneWireWriteByte(sensor, address);

			// Data
			for (i = 0; i < length; i++)
			{
				OneWireWriteByte(sensor, data[i]);
			}

			OneWireReset(sensor);

			status = PASSED;
		}
	}

	return(status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireReadScratchpad
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireReadScratchpad(uint8 sensor, uint8 address, uint8 length, uint8* data)
{
	uint8 status = FAILED;
	uint8 i = 0;

	if ((data != NULL) && (address <= 0x1F) && (length <= 32))
	{
		// Read Scratchpad (0xAA), Address: 0x00 -> 0x1F (wrap)
		if (OneWireReset(sensor) == YES)
		{
			// Skip ROM
			OneWireWriteByte(sensor, 0xCC);

			// Read Scratchpad
			OneWireWriteByte(sensor, 0xAA);

			// Address
			OneWireWriteByte(sensor, address);

			// Data
			for (i = 0; i < length; i++)
			{
				data[i] = OneWireReadByte(sensor);
			}

			OneWireReset(sensor);

			status = PASSED;
		}
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireCopyScratchpad
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireCopyScratchpad(uint8 sensor)
{
	uint8 status = FAILED;
	//uint8 i = 0;

	// Copy Scratchpad (0x55), Validation key: 0xA5, Data line held for 10ms
	if (OneWireReset(sensor) == YES)
	{
		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Copy Scratchpad
		OneWireWriteByte(sensor, 0x55);

		// Validation Key
		OneWireWriteByte(sensor, 0xA5);

		soft_usecWait(10 * SOFT_MSECS);

		OneWireReset(sensor);

		status = PASSED;
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireWriteAppRegister
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireWriteAppRegister(uint8 sensor, uint8 address, uint8 length, uint8* data)
{
	uint8 status = FAILED;
	uint8 i = 0;

	if ((data != NULL) && (address <= 0x07) && (length <= 8))
	{
		// Write Application Register (0x99), Address: 0x00 -> 0x07 (wrap)
		if (OneWireReset(sensor) == YES)
		{
			// Skip ROM
			OneWireWriteByte(sensor, 0xCC);

			// Write App Register
			OneWireWriteByte(sensor, 0x99);

			// Address
			OneWireWriteByte(sensor, address);

			// Data
			for (i = 0; i < 8; i++)
			{
				OneWireWriteByte(sensor, data[i]);
			}

			OneWireReset(sensor);

			status = PASSED;
		}
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireReadStatusRegister
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireReadStatusRegister(uint8 sensor, uint8* data)
{
	uint8 status = FAILED;
	//uint8 i = 0;

	if (data != NULL)
	{
		// Read Status Register (0x66), Validation key: 0x00
		if (OneWireReset(sensor) == YES)
		{
			// Skip ROM
			OneWireWriteByte(sensor, 0xCC);

			// Read Status Register
			OneWireWriteByte(sensor, 0x66);

			// Validation key
			OneWireWriteByte(sensor, 0x00);

			data[0] = OneWireReadByte(sensor);

			OneWireReset(sensor);

			status = PASSED;
		}
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireReadAppRegister
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireReadAppRegister(uint8 sensor, uint8 address, uint8 length, uint8* data)
{
	uint8 status = FAILED;
	uint8 i = 0;

	if ((data != NULL) && (address <= 0x07) && (length <= 8))
	{
		// Read Application Register (0xC3), Address: 0x00 -> 0x07 (wrap)
		if (OneWireReset(sensor) == YES)
		{
			// Skip ROM
			OneWireWriteByte(sensor, 0xCC);

			// Read App Register
			OneWireWriteByte(sensor, 0xC3);

			// Address
			OneWireWriteByte(sensor, address);

			// Data
			for (i = 0; i < length; i++)
			{
				data[i] = OneWireReadByte(sensor);
			}

			OneWireReset(sensor);

			status = PASSED;
		}
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function:	OneWireCopyAndLockAppRegister
///	Purpose:
///----------------------------------------------------------------------------
uint8 OneWireCopyAndLockAppRegister(uint8 sensor)
{
	uint8 status = FAILED;
	//uint8 i = 0;
	uint8 lockStatus = 0;

	if (OneWireReadStatusRegister(sensor, &lockStatus) == PASSED)
	{
		// Check if the App register is unlocked
		if (lockStatus == 0xFF)
		{
			// Copy and Lock Application Register (0x5A), Validation key: 0xA5, Only executed once
			if (OneWireReset(sensor) == YES)
			{
				// Skip ROM
				OneWireWriteByte(sensor, 0xCC);

				// Copy and Lock App Register
				OneWireWriteByte(sensor, 0x5A);

				// Validation Key
				OneWireWriteByte(sensor, 0xA5);

				soft_usecWait(10 * SOFT_MSECS);

				OneWireReset(sensor);

				status = PASSED;
			}
		}
	}

	return (status);
}
