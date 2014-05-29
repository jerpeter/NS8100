///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
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
///	Function Break
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
///	Function Break
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
	//SoftUsecWait(400);
	SoftUsecWait(500);

	// Release line (allow pullup to take affect)
	//reg_DDRE.reg &= ~sensor;
	reg_PORTE.reg |= sensor;

	// Wait 30us + 50us (80us total)
	//SoftUsecWait(64);
	SoftUsecWait(80);

	if ((reg_PORTQB.reg & READ_SENSOR) == LOW)
	{
		presenceDetect = YES;
	}

	// Wait 100us make sure device is not driving the line
	//SoftUsecWait(80);
	SoftUsecWait(100);

	return (presenceDetect);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
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
			//SoftUsecWait(4);
			SoftUsecWait(5);

			// Release the line
			//reg_DDRE.reg &= ~sensor;
			reg_PORTE.reg |= sensor;

			// Wait for 65us, recovery time
			//SoftUsecWait(52);
			SoftUsecWait(65);
		}
		else
		{
			// Hold low for 65us
			//SoftUsecWait(52);
			SoftUsecWait(65);

			// Release the line
			//reg_DDRE.reg &= ~sensor;
			reg_PORTE.reg |= sensor;

			// Wait for 5us, recovery time
			//SoftUsecWait(4);
			SoftUsecWait(5);
		}

		// Shift the data over 1 bit
		data >>= 1;
	}
}
#endif

///----------------------------------------------------------------------------
///	Function Break
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
		//SoftUsecWait(4);
		SoftUsecWait(5);

		// Release the line
		//reg_DDRE.reg &= ~sensor;
		reg_PORTE.reg |= sensor;

		// Wait for 5us
		//SoftUsecWait(4);
		SoftUsecWait(5);

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
		//SoftUsecWait(48);
		SoftUsecWait(60);
	}

	return (data);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
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

		crc = CalcCrc8(&romData[0], 7, 0x00);

		DebugPrint(RAW, "\nOne Wire Rom Data: ");

		for (i = 0; i < 8; i++)
		{
			DebugPrint(RAW, "0x%x ", romData[i]);
		}

		if (crc == romData[7])
		{
			DebugPrint(RAW, "(CRC: %x, success)\n", crc);
			OneWireFunctions(sensor);
		}
		else
		{
			DebugPrint(RAW, "(CRC: %x, fail)\n", crc);
		}
	}
	else
	{
		DebugPrint(RAW, "\nOne Wire device not found!\n");
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void OneWireFunctions(uint8 sensor)
{
	uint8 i = 0;

	// Read Memory (0xF0), Address: 0x00 -> 0x1F (wrap)
	if (OneWireReset(sensor) == YES)
	{
		DebugPrint(RAW, "Read Memory\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read Memory
		OneWireWriteByte(sensor, 0xF0);

		// Address
		OneWireWriteByte(sensor, 0x00);

		DebugPrint(RAW, "  Data:");

		// Data
		for (i = 0; i < 32; i++)
			DebugPrint(RAW, "%x ", OneWireReadByte(sensor));

		DebugPrint(RAW, "\n");

		OneWireReset(sensor);
	}
	else return;

	// Write Scratchpad (0x0F), Address: 0x00 -> 0x1F (wrap)
	if (OneWireReset(sensor) == YES)
	{
		DebugPrint(RAW, "Write Scratchpad\n");

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
		DebugPrint(RAW, "Read Scratchpad\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read Scratchpad
		OneWireWriteByte(sensor, 0xAA);

		// Address
		OneWireWriteByte(sensor, 0x00);

		DebugPrint(RAW, "  Data:");

		// Data
		for (i = 0; i < 32; i++)
			DebugPrint(RAW, "%x ", OneWireReadByte(sensor));

		DebugPrint(RAW, "\n");

		OneWireReset(sensor);
	}
	else return;

	// Copy Scratchpad (0x55), Validation key: 0xA5, Data line held for 10ms
	if (OneWireReset(sensor) == YES)
	{
		DebugPrint(RAW, "Copy Scratchpad\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Copy Scratchpad
		OneWireWriteByte(sensor, 0x55);

		// Validation Key
		OneWireWriteByte(sensor, 0xA5);

		SoftUsecWait(10 * SOFT_MSECS);

		OneWireReset(sensor);
	}
	else return;

	// Write Application Register (0x99), Address: 0x00 -> 0x07 (wrap)
	if (OneWireReset(sensor) == YES)
	{
		DebugPrint(RAW, "Write App Register\n");

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
		DebugPrint(RAW, "Read Status Register\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read Status Register
		OneWireWriteByte(sensor, 0x66);

		// Validation key
		OneWireWriteByte(sensor, 0x00);

		DebugPrint(RAW, "  Data:");

		// Data
		DebugPrint(RAW, "%x\n", OneWireReadByte(sensor));

		OneWireReset(sensor);
	}
	else return;

	// Read Application Register (0xC3), Address: 0x00 -> 0x07 (wrap)
	if (OneWireReset(sensor) == YES)
	{
		DebugPrint(RAW, "Read App Register\n");

		// Skip ROM
		OneWireWriteByte(sensor, 0xCC);

		// Read App Register
		OneWireWriteByte(sensor, 0xC3);

		// Address
		OneWireWriteByte(sensor, 0x00);

		DebugPrint(RAW, "  Data:");

		// Data
		for (i = 0; i < 8; i++)
		{
			DebugPrint(RAW, "%x ", OneWireReadByte(sensor));
		}

		DebugPrint(RAW, "\n");

		OneWireReset(sensor);
	}
	else return;

	// Copy and Lock Application Register (0x5A), Validation key: 0xA5, Only executed once
}

///----------------------------------------------------------------------------
///	Function Break
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

		crc = CalcCrc8(&romData[0], 7, 0x00);

		DebugPrint(RAW, "\nOne Wire Rom Data: ");

		for (i = 0; i < 8; i++)
		{
			DebugPrint(RAW, "0x%x ", romData[i]);
		}

		if (crc == romData[7])
		{
			DebugPrint(RAW, "(CRC: %x, success)\n", crc);
			OneWireFunctions(sensor);
		}
		else
		{
			DebugPrint(RAW, "(CRC: %x, fail)\n", crc);
		}

		OneWireReset(sensor);

		status = PASSED;
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function Break
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
///	Function Break
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
///	Function Break
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
///	Function Break
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

		SoftUsecWait(10 * SOFT_MSECS);

		OneWireReset(sensor);

		status = PASSED;
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function Break
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
///	Function Break
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
///	Function Break
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
///	Function Break
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

				SoftUsecWait(10 * SOFT_MSECS);

				OneWireReset(sensor);

				status = PASSED;
			}
		}
	}

	return (status);
}
