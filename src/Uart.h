///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Uart.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:02 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Uart.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _UART_H_
#define _UART_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h" 
#include "Mmc2114_InitVals.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define BAUD_TEST_57600		0x55	/* 57600 autobaud test character */
#define BAUD_TEST_38400		0x92	/* 38400 autobaud test character */
#define BAUD_TEST_19200		0x1C	/* 19200 autobaud test character */
#define BAUD_TEST_9600		0xE0	/*  9600 autobaud test character */
#define UART_BLOCK			0
#define UART_TIMEOUT		1
#define UART_TIMEOUT_COUNT	50000	// Approx 250ms to process the code to wait
#define UART_TIMED_OUT		0x80

enum {
	RX_ENABLE,
	RX_DISABLE,
	TX_ENABLE,
	TX_DISABLE
};

enum {
	CONVERT_DATA_TO_ASCII = 1,
	NO_CONVERSION
};

 /***********************************************************************/
 /*																		*/
 /*  In the first register, UMR1 is for reads and UMR2 is for writes.	*/
 /*																		*/
 /***********************************************************************/

#define UART_UMR1_RXRTS				(0x80)	/* Receive Request-to-Send	*/
#define UART_UMR1_RXIRQ				(0x40)	/* Receive Interrupt Select	*/
#define UART_UMR1_ERR				(0x20)	/* Error Mode			*/
#define UART_UMR1_PM_MULTI_ADDR		(0x1C)	/* Parity: Multidrop Adr Char	*/
#define UART_UMR1_PM_MULTI_DATA		(0x18)	/* Parity: Multidrop Data Char	*/
#define UART_UMR1_PM_NONE			(0x10)	/* Parity: None			*/
#define UART_UMR1_PM_FORCE_HI		(0x0C)	/* Parity: Force High		*/
#define UART_UMR1_PM_FORCE_LO		(0x08)	/* Parity: Force Low		*/
#define UART_UMR1_PM_ODD			(0x04)	/* Parity: Odd Parity		*/
#define UART_UMR1_PM_EVEN			(0x00)	/* Parity: Even Parity		*/
#define UART_UMR1_BC_5				(0x00)	/* 5 Bits Per Character		*/
#define UART_UMR1_BC_6				(0x01)	/* 6 Bits Per Character		*/
#define UART_UMR1_BC_7				(0x02)	/* 7 Bits Per Character		*/
#define UART_UMR1_BC_8				(0x03)	/* 8 Bits Per Character		*/

#define UART_UMR2_CM_NORMAL			(0x00)	/* Normal Channel Mode		*/
#define UART_UMR2_CM_ECHO			(0x40)	/* Automatic Echo Channel Mode	*/
#define UART_UMR2_CM_LOCAL_LOOP		(0x80)	/* Local Loopback Channel Mode	*/
#define UART_UMR2_CM_REMOTE_LOOP	(0xC0)	/* Remote Loopback Channel Mode	*/
#define UART_UMR2_TXRTS				(0x20)	/* Transmitter Ready-to-Send	*/
#define UART_UMR2_TXCTS				(0x10)	/* Transmitter Clear-to-Send	*/
#define UART_UMR2_STOP_BITS_1		(0x07)	/* 1 Stop Bits			*/
#define UART_UMR2_STOP_BITS_2		(0x0F)	/* 2 Stop Bits			*/
#define UART_UMR2_STOP_BITS(a)		((a)&0x0f)	/* Stop Bit Length	*/

 /***********************************************************************/
 /*																		*/
 /*  In this next register, USR is for reads and UCSR is for writes.	*/
 /*																		*/
 /***********************************************************************/

#define UART_USR_RB				(0x80)	/* Received Break		*/
#define UART_USR_FE				(0x40)	/* Framing Error		*/
#define UART_USR_PE			    (0x20)	/* Parity Error			*/
#define UART_USR_OE			    (0x10)	/* Overrun Error		*/
#define UART_USR_TXEMP			(0x08)	/* Transmitter Empty		*/
#define UART_USR_TXRDY			(0x04)	/* Transmitter Ready		*/
#define UART_USR_FFULL			(0x02)	/* FIFO Full			*/
#define UART_USR_RXRDY			(0x01)	/* Receiver Ready		*/

#define UART_UCSR_9600_BPS		(0xBB)	/* 9600 Baud w/ 3.6864 MHz clk	*/
#define UART_UCSR_TIMER			(0xDD)	/* Timer mode */
#define UART_UCSR_RCS(a)		(((a)&0x0f)<<4) /* Receiver Clk Select	*/
#define UART_UCSR_TCS(a)		((a)&0x0f) /* Transmitter Clock Select	*/

 /***********************************************************************/
 /*																		*/
 /*  In this next register, there is only UCR is for writes.			*/
 /*																		*/
 /***********************************************************************/

#define UART_UCR_NONE			(0x00)	/* No Command			*/
#define UART_UCR_STOP_BREAK		(0x70)	/* Stop Break			*/
#define UART_UCR_START_BREAK	(0x60)	/* Start Break			*/
#define UART_UCR_RESET_BKCHGINT	(0x50)	/* Reset Break-Change Interrupt	*/
#define UART_UCR_RESET_ERROR	(0x40)	/* Reset Error Status		*/
#define UART_UCR_RESET_TX		(0x30)	/* Reset Transmitter		*/
#define UART_UCR_RESET_RX		(0x20)	/* Reset Receiver		*/
#define UART_UCR_RESET_MR		(0x10)	/* Reset Mode Register Pointer	*/
#define UART_UCR_TX_DISABLED	(0x08)	/* Transmitter Disabled		*/
#define UART_UCR_TX_ENABLED		(0x04)	/* Transmitter Enabled		*/
#define UART_UCR_RX_DISABLED	(0x02)	/* Receiver Disabled		*/
#define UART_UCR_RX_ENABLED		(0x01)	/* Receiver Enabled		*/

 /***********************************************************************/
 /*																		*/
 /*  In this next register, UIPCR is for reads and UACR is for writes.	*/
 /*																		*/
 /***********************************************************************/

#define UART_UIPCR_COS			(0x10)	/* Change-of-State at IPx input	*/
#define UART_UIPCR_CTS			(0x01)	/* Current State of CTS pin	*/

#define UART_UACR_BRG			(0x80)	/* Set 2 of Baud Rate Generator	*/
#define UART_UACR_CTMS_TIMER	(0x60)	/* Timer Mode and Source Select	*/
                                        /* Must set this mode and src	*/
#define UART_UACR_IEC			(0x01)	/* Input Enable Control		*/

 /***********************************************************************/
 /*																		*/
 /*  In this next register, UISR is for reads and UIMR is for writes.	*/
 /*																		*/
 /***********************************************************************/

#define UART_UISR_COS			(0x80)	/* Change-of-State at CTS input	*/
#define UART_UISR_DB			(0x04)	/* Receiver Has Detected Break	*/
#define UART_UISR_RXRDY			(0x02)	/* Receiver Ready or FIFO Full	*/
#define UART_UISR_TXRDY			(0x01)	/* Transmitter Ready		*/

#define UART_UIMR_COS			(0x80)	/* Enable Change-of-State Intpt	*/
#define UART_UIMR_DB			(0x04)	/* Enable Delta Break Interrupt	*/
#define UART_UIMR_FFULL			(0x02)	/* Enable FIFO Full Interrupt	*/
#define UART_UIMR_TXRDY			(0x01)	/* Enable Transmitter Rdy Intpt	*/

 /***********************************************************************/
 /*																	  	*/
 /*  The last few registers can only be accessed as a read or write     */
 /*  register.															*/
 /*																		*/
 /***********************************************************************/

#define UART_UIP_CTS			(0x01) /* Current State of CTS Input	*/
#define UART_UOP_RTS			(0x01) /* Sets All Bits on OP Bit Set	*/
#define UART_UOP0_RTS			(0x01) /* Clears All Bits on OP Bit Rst	*/

// Clear by writting a 1 (inactive)
#define CLEAR_DTR	(reg_TIM1PORT.reg |= 0x01)
// Set by writting a 0 (active)
#define SET_DTR		(reg_TIM1PORT.reg &= 0xFE)
// Clear by writting a 1 (inactive)
#define CLEAR_RTS	(reg_TIM1PORT.reg |= 0x02)
// Set by writting a 0 (active)
#define SET_RTS		(reg_TIM1PORT.reg &= 0xFD)
// Data Set Ready  a 0 = (active) - modem connected, 1 = modem not detected
#define READ_DSR 	((reg_TIM1PORT.reg >> 2) & 0x01)
// Data Carrier Detect  a 0 = (active) - modem connected, 1 = modem not detected
#define READ_DCD 	((reg_TIM1PORT.reg >> 3) & 0x01)
// Ring Indicator 1 = (active) - ring imcoming, 0 = no ring
#define READ_RI 	(reg_TIM2PORT.reg & 0x01)
// Returns CTS signal, 0 = Active - Clear To Send, 1 = Inactive - Not Clear To Send
#define READ_CTS 	((reg_TIM2PORT.reg >> 1) & 0x01)

enum {
	DCD_ACTIVE = 0,
	DCD_INACTIVE
};

enum {
	READY_TO_SEND = 0,
	NOT_READY_TO_SEND
};

enum {
	CONNECTION_ESTABLISHED = 0,
	NO_CONNECTION
};

enum {
	RING = 0,
	NO_RING
};

enum {
	MODEM_CONNECTED = 0,
	MODEM_NOT_CONNECTED
};

enum {
	MODEM_SEND_FAILED = 0,
	MODEM_SEND_SUCCESS
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
short craft(char* fmt, ...);
short debugPrint(uint8 mode, char* fmt, ...);
void debugPrintChar(uint8 charData);
char* uart_gets(char* s, int32 channel);
uint8 uart_getc(int32 channel, uint8 mode);
uint8 nibbleToA(uint8 hexData);
uint8 modem_putc(uint8 , uint8);
uint8 modem_puts(uint8* , uint32 , uint8);
void uart_puts(char* s, int32 channel);
void uart_write(void* b, int32 n, int32 channel);
void uart_putc(uint8 c, int32 channel);
void uart_init(uint32 BaudRate, int32 channel);
void uartControl(uint8 control, int8 channel);
uint16 auto_baud(int32 channel);
BOOLEAN uart_char_waiting(int32 channel);

#endif // _UART_H_
