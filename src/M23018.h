/********************************************************
 Name          : M23018.h
 Author        : Joseph Getz
 Copyright     : Your copyright notice
 Description   : MCP23018 drivers
 **********************************************************/
#ifndef M23018_H_
#define M23018_H_

#define IO_ADDRESS_BASE       0x20        // IO's TWI address
#define IO_ADDRESS_KPD        0x27        // IO's TWI address
#define IO_ADDR_LGT           1           // Address length of the IO chip
#define TWI_SPEED             400000      // Speed of TWI

#define IODIRA     0x00
#define IODIRB     0x01
#define IOPOLA     0x02
#define IOPOLB     0x03
#define GPINTENA   0x04
#define GPINTENB   0x05
#define DEFVALA    0x06
#define DEFVALB    0x07
#define INTCONA    0x08
#define INTCONB    0x09
#define IOCONA     0x0A
#define IOCONB     0x0B
#define GPPUA      0x0C
#define GPPUB      0x0D
#define INTFA      0x0E
#define INTFB      0x0F
#define INTCAPA    0x10
#define INTCAPB    0x11
#define GPIOA      0x12
#define GPIOB      0x13
#define OLATA      0x14
#define OLATB      0x15

#define GREEN_LED_PIN	0x10
#define RED_LED_PIN		0x20

void init_mcp23018(unsigned char chip);
void write_mcp23018(unsigned char chip, unsigned char address, unsigned char data);
void write_mcp23018_bytes(unsigned char chip, unsigned char address,  unsigned char *data, unsigned char length);
unsigned char read_mcp23018(unsigned char chip, unsigned char address);

#endif /* M23018_H_ */
