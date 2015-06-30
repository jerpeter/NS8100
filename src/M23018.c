///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "board.h"
#include "twi.h"
#include "m23018.h"
#include "Typedefs.h"
#include "Uart.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define	TWI_DATA_LENGTH		(sizeof(s_twiData)/sizeof(U8))

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static uint8 s_twiData[10];
static twi_package_t s_twiPacket;

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteMcp23018(unsigned char chip, unsigned char address, unsigned char data)
{
	//Set output latch a with 00
	s_twiData[0] = data;
	// TWI chip address to communicate with
	s_twiPacket.chip = chip;
	// TWI address/commands to issue to the other chip (node)
	s_twiPacket.addr = address;
	// Length of the TWI data address segment (1-3 bytes)
	s_twiPacket.addr_length = IO_ADDR_LGT;
	// Where to find the data to be written
	s_twiPacket.buffer = (void*) s_twiData;
	// How many bytes do we want to write
	s_twiPacket.length = 1;
	// perform a write access

	if (twi_master_write(&AVR32_TWI, &s_twiPacket) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write (single byte) to MCP23018\r\n");
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void WriteMcp23018Bytes(unsigned char chip, unsigned char address, unsigned char *data, unsigned char length)
{
	unsigned char count;

	//Set output latch a with 00
	for (count=0;count<=length;count++)
	{
		s_twiData[count] = data[count];
	}

	// TWI chip address to communicate with
	s_twiPacket.chip = chip;
	// TWI address/commands to issue to the other chip (node)
	s_twiPacket.addr = address;
	// Length of the TWI data address segment (1-3 bytes)
	s_twiPacket.addr_length = IO_ADDR_LGT;
	// Where to find the data to be written
	s_twiPacket.buffer = (void*) s_twiData;
	// How many bytes do we want to write
	s_twiPacket.length = length;
	// perform a write access

	if (twi_master_write(&AVR32_TWI, &s_twiPacket) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write (multiple bytes) to MCP23018\r\n");
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
unsigned char ReadMcp23018(unsigned char chip, unsigned char address)
{
	//Set output latch a with 00
	s_twiData[0] = 0;
	// TWI chip address to communicate with
	s_twiPacket.chip = chip;
	// TWI address/commands to issue to the other chip (node)
	s_twiPacket.addr = address;
	// Length of the TWI data address segment (1-3 bytes)
	s_twiPacket.addr_length = IO_ADDR_LGT;
	// Where to find the data to be written
	s_twiPacket.buffer = (void*) s_twiData;
	// How many bytes do we want to write
	s_twiPacket.length = 1;
	// perform a write access

	if (twi_master_read(&AVR32_TWI, &s_twiPacket) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write to MCP23018\r\n");
	}

	return(s_twiData[0]);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitMcp23018(void)
{
	// I/O Config
	WriteMcp23018(IO_ADDRESS_KPD, IOCONA, 0x20);
	WriteMcp23018(IO_ADDRESS_KPD, IOCONB, 0x20);

	// Port Value
	WriteMcp23018(IO_ADDRESS_KPD, GPIOA, 0x00);
	WriteMcp23018(IO_ADDRESS_KPD, GPIOB, 0x00);

	// Port Direction
	WriteMcp23018(IO_ADDRESS_KPD, IODIRA, 0x0F);
	WriteMcp23018(IO_ADDRESS_KPD, IODIRB, 0xFF);

	// Pullup (Open drain outputs only, without pullups you can't drive)
	WriteMcp23018(IO_ADDRESS_KPD, GPPUA, 0xFF);
	WriteMcp23018(IO_ADDRESS_KPD, GPPUB, 0xFF);

	// Polarity
	WriteMcp23018(IO_ADDRESS_KPD, IOPOLA, 0x0E);
	WriteMcp23018(IO_ADDRESS_KPD, IOPOLB, 0xFF);

	// Default Value
	WriteMcp23018(IO_ADDRESS_KPD, DEFVALA, 0x00);
	WriteMcp23018(IO_ADDRESS_KPD, DEFVALB, 0x00);

	// Interrupt on Change Compare
	WriteMcp23018(IO_ADDRESS_KPD, INTCONA, 0x00);
	WriteMcp23018(IO_ADDRESS_KPD, INTCONB, 0x00);

#if 0 // Prevent enabling interrupt on init (done at the end of Software Init now)
	// Interrupt Enable on Change
	WriteMcp23018(IO_ADDRESS_KPD, GPINTENA, 0x0F);
	WriteMcp23018(IO_ADDRESS_KPD, GPINTENB, 0xFF);
#endif

	// Clear any interrupt generation
	ReadMcp23018(IO_ADDRESS_KPD, INTFA);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOA);

	ReadMcp23018(IO_ADDRESS_KPD, INTFB);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOB);

	// clear the interrupt flag in the processor
	AVR32_EIC.ICR.int4 = 1;
	AVR32_EIC.ICR.int5 = 1;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void EnableMcp23018Interrupts(void)
{
	// Disable Interrupt Enable on Change
	WriteMcp23018(IO_ADDRESS_KPD, GPINTENA, 0x0F);
	WriteMcp23018(IO_ADDRESS_KPD, GPINTENB, 0xFF);

	// Clear any interrupt generation
	ReadMcp23018(IO_ADDRESS_KPD, INTFA);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOA);

	ReadMcp23018(IO_ADDRESS_KPD, INTFB);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisableMcp23018Interrupts(void)
{
	// Disable Interrupt Enable on Change
	WriteMcp23018(IO_ADDRESS_KPD, GPINTENA, 0x00);
	WriteMcp23018(IO_ADDRESS_KPD, GPINTENB, 0x00);

	// Clear any interrupt generation
	ReadMcp23018(IO_ADDRESS_KPD, INTFA);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOA);

	ReadMcp23018(IO_ADDRESS_KPD, INTFB);
	ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
}
