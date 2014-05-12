/********************************************************
 Name          : M23018.c
 Author        : Joseph Getz
 Copyright     : Your copyright notice
 Description   : MCP23018 drivers
 **********************************************************/

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
#define  TWI_DATA_LENGTH        (sizeof(s_twiData)/sizeof(U8))

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static uint8 s_twiData[10];
static twi_package_t s_twiPacket;

///----------------------------------------------------------------------------
/// write_mcp23018
///----------------------------------------------------------------------------
void write_mcp23018(unsigned char chip, unsigned char address, unsigned char data)
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
	  twi_master_write(&AVR32_TWI, &s_twiPacket);
}

///----------------------------------------------------------------------------
/// write_mcp23018_bytes
///----------------------------------------------------------------------------
void write_mcp23018_bytes(unsigned char chip, unsigned char address,  unsigned char *data, unsigned char length)
{
    unsigned char count;

	//Set output latch a with 00
	for(count=0;count<=length;count++)
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
	if(twi_master_write(&AVR32_TWI, &s_twiPacket) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write to MCP23018\n");
	}
}

///----------------------------------------------------------------------------
/// read_mcp23018
///----------------------------------------------------------------------------
unsigned char read_mcp23018(unsigned char chip, unsigned char address)
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
	if(twi_master_read(&AVR32_TWI, &s_twiPacket) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write to MCP23018\n");
	}

	return(s_twiData[0]);
}

///----------------------------------------------------------------------------
/// init_mcp23018
///----------------------------------------------------------------------------
void init_mcp23018(unsigned char chip)
{
	// I/O Config
    write_mcp23018(chip, IOCONA, 0x20);
    write_mcp23018(chip, IOCONB, 0x20);

	// Port Value
	write_mcp23018(chip, GPIOA, 0x00);
    write_mcp23018(chip, GPIOB, 0x00);

	// Port Direction
    write_mcp23018(chip, IODIRA, 0x0F);
    write_mcp23018(chip, IODIRB, 0xFF);

	// Pullup (Open drain outputs only, without pullups you can't drive
    write_mcp23018(chip, GPPUA, 0xFF);
    write_mcp23018(chip, GPPUB, 0xFF);

	// Polarity
    write_mcp23018(chip, IOPOLA, 0x0E);
    write_mcp23018(chip, IOPOLB, 0xFF);

	// Default Value
    write_mcp23018(chip, DEFVALA, 0x00);
    write_mcp23018(chip, DEFVALB, 0x00);

	// Interrupt on Change Compare
	write_mcp23018(chip, INTCONA, 0x00);
	write_mcp23018(chip, INTCONB, 0x00);

	// Interrupt Enable on Change
    write_mcp23018(chip, GPINTENA, 0x0F);
    write_mcp23018(chip, GPINTENB, 0xFF);

	// Clear any interrupt generation
	read_mcp23018(IO_ADDRESS_KPD, INTFA);
	read_mcp23018(IO_ADDRESS_KPD, GPIOA);

	read_mcp23018(IO_ADDRESS_KPD, INTFB);
	read_mcp23018(IO_ADDRESS_KPD, GPIOB);

	// clear the interrupt flag in the processor
	AVR32_EIC.ICR.int4 = 1;
	AVR32_EIC.ICR.int5 = 1;
}
