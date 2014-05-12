/********************************************************
 Name          : M23018.c
 Author        : Joseph Getz
 Copyright     : Your copyright notice
 Description   : MCP23018 drivers
 **********************************************************/
#include "board.h"
#include "twi.h"
#include "m23018.h"
#include "Typedefs.h"
#include "Uart.h"

U8 twi_data[10];
#define  TWI_DATA_LENGTH        (sizeof(twi_data)/sizeof(U8))

twi_package_t packet, packet_received;

void write_mcp23018(unsigned char chip, unsigned char address, unsigned char data)
{
	  //Set output latch a with 00
	  twi_data[0] = data;
	  // TWI chip address to communicate with
	  packet.chip = chip;
	  // TWI address/commands to issue to the other chip (node)
	  packet.addr = address;
	  // Length of the TWI data address segment (1-3 bytes)
	  packet.addr_length = IO_ADDR_LGT;
	  // Where to find the data to be written
	  packet.buffer = (void*) twi_data;
	  // How many bytes do we want to write
	  packet.length = 1;
	  // perform a write access
	  twi_master_write(&AVR32_TWI, &packet);
}

void write_mcp23018_bytes(unsigned char chip, unsigned char address,  unsigned char *data, unsigned char length)
{
    unsigned char count;

	//Set output latch a with 00
	for(count=0;count<=length;count++)
	{
	    twi_data[count] = data[count];
	}
	// TWI chip address to communicate with
	packet.chip = chip;
	// TWI address/commands to issue to the other chip (node)
	packet.addr = address;
	// Length of the TWI data address segment (1-3 bytes)
	packet.addr_length = IO_ADDR_LGT;
	// Where to find the data to be written
	packet.buffer = (void*) twi_data;
	// How many bytes do we want to write
	packet.length = length;
	// perform a write access
	if(twi_master_write(&AVR32_TWI, &packet) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write to MCP23018\n");
	}
}
unsigned char read_mcp23018(unsigned char chip, unsigned char address)
{
	//Set output latch a with 00
	twi_data[0] = 0;
	// TWI chip address to communicate with
	packet.chip = chip;
	// TWI address/commands to issue to the other chip (node)
	packet.addr = address;
	// Length of the TWI data address segment (1-3 bytes)
	packet.addr_length = IO_ADDR_LGT;
	// Where to find the data to be written
	packet.buffer = (void*) twi_data;
	// How many bytes do we want to write
	packet.length = 1;
	// perform a write access
	if(twi_master_read(&AVR32_TWI, &packet) != TWI_SUCCESS)
	{
		debugErr("TWI: Failure to write to MCP23018\n");
	}

	return(twi_data[0]);
}

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
