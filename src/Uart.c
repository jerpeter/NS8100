///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Typedefs.h"
#include "Uart.h"
#include "Menu.h"
#include "RemoteCommon.h"
#include "PowerManagement.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
#if 0 // Original
static char s_uartBuffer[256];
#else // Make sure there is enough buffer space for comparing remote command 1K message sizes
static char s_uartBuffer[(1024 + 256)];
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 NibbleToA(uint8 hexData)
{
	uint8 hexNib = (uint8)(0x0F & hexData);

	if (hexNib <= 0x9)
	{
		// Got a hexNibble between '0' and '9', convert to the ascii representation.
		return (uint8)(hexNib + 0x30);
	}
	else if ((hexNib >= 0xA) && (hexNib <= 0xF))
	{
		// Got a hexNibble between 'A' and 'F', convert to the ascii representation.
		return (uint8)(hexNib + 0x37);
	}
	else // Should get here
	{
		return (0);
	}
}


///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ModemPutc(uint8 byteData, uint8 convertAsciiFlag)
{
	uint8 status = MODEM_SEND_FAILED;
	uint8 hexData;
	uint8 asciiData;
	volatile uint32 timeout = g_lifetimeHalfSecondTickCount + (25 * 2); //Set timeout to 25 secs

	// Make sure a modem is connected
	if (READ_DSR == MODEM_CONNECTED)
	{
		// Check clear to send and check if the timeout has been exceeded
		while ((NOT_READY_TO_SEND == READ_CTS) && (timeout > g_lifetimeHalfSecondTickCount))
		{
			// Check if the connection has been lost
			if (READ_DCD == NO_CONNECTION)
			{
				// Return immediately
				return (status);
			}
		}

		// Check if the timeout condition hasn't been exceeded yet
		if (timeout > g_lifetimeHalfSecondTickCount)
		{
			if (convertAsciiFlag == CONVERT_DATA_TO_ASCII)
			{
				// Convert the top nibble to hex
				hexData = (uint8)((0xF0 & byteData) >> 4);
				asciiData = NibbleToA(hexData);

				// Send the top nibble
				UartPutc(asciiData, CRAFT_COM_PORT);

				// Convert the bottom nibble to hex
				hexData = (uint8)(0x0F & byteData);
				asciiData = NibbleToA(hexData);

				// Send the bottom nibble
				UartPutc(asciiData, CRAFT_COM_PORT);
			}
			else
			{
				// Send the byte of data
				UartPutc(byteData, CRAFT_COM_PORT);
			}

			// Set status to success because data has been sent
			status = MODEM_SEND_SUCCESS;
		}
	}

	return (status);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ModemPuts(uint8* byteData, uint32 dataLength, uint8 convertAsciiFlag)
{
	uint32 dataDex;
	uint8* theData = byteData;

	// Sending modem data, signal that data is being transfered
	g_modemDataTransfered = YES;

	for (dataDex = 0; dataDex < dataLength; dataDex++)
	{
#if 0 // Exception testing (Prevent non-ISR soft loop watchdog from triggering)
		g_execCycles++;
#endif
		if (MODEM_SEND_FAILED == ModemPutc(*theData, convertAsciiFlag))
		{
			return (MODEM_SEND_FAILED);
		}

		theData++;
	}
	return (MODEM_SEND_SUCCESS);
}


///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "print_funcs.h"
#include "usart.h"
extern int usart_putchar(volatile avr32_usart_t *usart, int c);
extern int usart_write_char(volatile avr32_usart_t *usart, int c);
void UartPutc(uint8 c, int32 channel)
{
	uint32 retries = USART_DEFAULT_TIMEOUT;
	int status;

	if (channel == CRAFT_COM_PORT)
	{
#if 0 // Can't use this function without screwing up the craft since CRLF is being turned into CRCRLF
		usart_putchar(&AVR32_USART1, c);
#else // Localized version with retries
		//----------------------------------------------------------------------------------
		// Craft USART
		//----------------------------------------------------------------------------------
		status = usart_write_char(&AVR32_USART1, c);

		while ((status == USART_TX_BUSY) && (retries))
		{
			retries--;
			status = usart_write_char(&AVR32_USART1, c);
		}
#endif
	}
	else if (channel == RS485_COM_PORT)
	{
		// Add 485 logic
	}
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
	else if (channel == GLOBAL_DEBUG_PRINT_PORT)
	{
		//----------------------------------------------------------------------------------
		// Debug USART
		//----------------------------------------------------------------------------------
		// Check if the debug USART processor module is powered (USART0)
#if (GLOBAL_DEBUG_PRINT_ENABLED)
		// Dump the character to the serial port
		status = usart_write_char(&AVR32_USART0, c);

		while ((status == USART_TX_BUSY) && (retries))
		{
			retries--;
			status = usart_write_char(&AVR32_USART0, c);
		}
#endif

#if 0 // Removed debug log file due to inducing system problems
		//----------------------------------------------------------------------------------
		// Debug buffer
		//----------------------------------------------------------------------------------
		// Check if the buffer count doesn't equal the max (protect overrunning the buffer)
		if (g_debugBufferCount != sizeof(g_debugBuffer))
		{
			// Dump to the buffer
			g_debugBuffer[g_debugBufferCount++] = c;
		}
		// else do nothing (Assuming it's more important how we got here than where we're going)
#endif
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UartWrite(void* b, int32 n, int32 channel)
{
	char* s = (char*)b;
	while (n--)
	{
		if (*s == '\n')
		{
			UartPutc('\r', channel);
		}

		UartPutc(*s++, channel);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void UartPuts(char* s, int32 channel)
{
	while (*s)
	{
		if (*s == '\n')
		{
			UartPutc('\r', channel);
		}

		UartPutc(*s++, channel);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0
BOOLEAN UartCharWaiting(int32 channel)
{
	BOOLEAN charWaiting = FALSE;

	volatile MMC2114_IMM *imm = mmc2114_get_immp();

	if (channel == CRAFT_COM_PORT)
	{
		return ((imm->Sci1.SCISR1 & MMC2114_SCI_SCISR1_RDRF) != 0);
	}
	else if (channel == RS485_COM_PORT)
	{
		return ((imm->Sci2.SCISR1 & MMC2114_SCI_SCISR1_RDRF) != 0);
	}

	return (charWaiting);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#if 0
uint8 UartGetc(int32 channel, uint8 mode)
{
	volatile MMC2114_IMM *imm = mmc2114_get_immp();
	volatile uint32 uartTimeout = UART_TIMEOUT_COUNT;

	if (channel == CRAFT_COM_PORT)
	{
		while (!UartCharWaiting(channel))
		{
			if ((mode == UART_TIMEOUT) && (uartTimeout-- == 0))
				return (UART_TIMED_OUT);
		}

		return (imm->Sci1.SCIDRL);
	}
	else if (channel == RS485_COM_PORT)
	{
		while (!UartCharWaiting(channel))
		{
			if ((mode == UART_TIMEOUT) && (uartTimeout-- == 0))
				return (UART_TIMED_OUT);
		}

		return (imm->Sci2.SCIDRL);
	}

	return (0);
}
#endif

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
char* UartGets(char* s, int32 channel)
{
	char* b = s;
	BOOLEAN end = FALSE;
	int32 data;
	int32 count = 0;

	do
	{
		data = UartGetc(channel, UART_BLOCK);
		switch (data)
		{
			case '\b':
			case 0x7e:
				if (count)
				{
					count--;
					b--;
					if (channel != CRAFT_COM_PORT)
					{
						UartPuts("\b \b", channel);
					}
				}
				break;
			case '\r':
			case '\n':
				if (count)
				{
					*b = 0;
					if (channel != CRAFT_COM_PORT)
					{
						UartPuts("\r\n", channel);
					}
					end = TRUE;
				}
				break;
			case CAN_CHAR:
				*b = CAN_CHAR;
				*s = CAN_CHAR;
				end = TRUE;
				break;
			default:
				if (count < 255)
				{
					count++;
					*b++ = (char)data;
					if (channel != CRAFT_COM_PORT)
					{
						UartPutc((uint8)data, channel); // Echo the data back
					}
				}
				break;
		}
	} while (!end);
	if (*b != CAN_CHAR)
	{
		*b = 0;
	}
	return (s);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
short Craft(char* fmt, ...)
{
	va_list arg_ptr;
	short l;
#if 0 // Original
	char buf[256];
#else // Make sure there is enough buffer space for printing remote command 1K message sizes
	char buf[(1024 + 256)];
#endif
	static uint32 repeatingBuf = 0;
	char repeatCountStr[10];

	// Initialize arg_ptr to the begenning of the variable argument list
	va_start(arg_ptr, fmt);

	// Build the string in buf with the format fmt and arguments in arg_ptr
	l = (short)vsprintf(buf, fmt, arg_ptr);

	// Clean up. Invalidates arg_ptr from use again
	va_end(arg_ptr);

	// Check if the current string to be printed was different than the last
	if (strncmp(buf, s_uartBuffer, l))
	{
		// Check if the previous string repeated at all
		if (repeatingBuf > 0)
		{
			// Print the repeat count of the previous repeated string
			sprintf(repeatCountStr, "(%d)\n", (int)repeatingBuf);
			UartPuts(repeatCountStr, CRAFT_COM_PORT);

			// Reset the counter
			repeatingBuf = 0;
		}

		// Copy the new string into the global buffer
		strncpy(s_uartBuffer, buf, l);

		// Print the new string
		UartWrite(buf, l, CRAFT_COM_PORT);
	}
	else // Strings are equal
	{
		// Increment the repeat count
		repeatingBuf++;

		// Print a '!' (bang) so signify that the output was repeated
		UartPutc('!', CRAFT_COM_PORT);
	}

	// Return the number of characters
	return (l);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
short DebugPrint(uint8 mode, char* fmt, ...)
{
	va_list arg_ptr;
	short length = 0;
#if 0 // Original
	char buf[256];
#else // Make sure there is enough buffer space for printing remote command 1K message sizes
	char buf[(1024 + 256)];
#endif
	static uint32 repeatingBuf = 0;
	static uint8 strippedNewline = 0;
	char repeatCountStr[10];
	char timestampStr[8];
	int32 tempTime;

	//if (g_disableDebugPrinting == YES)
	//	return (0);

	// Initialize the buffer
	memset(&buf[0], 0, sizeof(buf));

	// Initialize arg_ptr to the beginning of the variable argument list
	va_start(arg_ptr, fmt);

	switch (mode)
	{
		case RAW: break;
		case NORM:	strcpy(buf, "(Debug |        ) "); break;
		case WARN:	strcpy(buf, "(Warn  -        ) "); break;
		case ERR:	strcpy(buf, "(Error *        ) "); break;
		default: break;
	}

	if (mode == RAW)
	{
		// Build the string in buf with the format fmt and arguments in arg_ptr
		length = (short)vsprintf(buf, fmt, arg_ptr);
	}
	else
	{
		// Build the string in buf with the format fmt and arguments in arg_ptr

		// Initialize the length to the number of chars in the debug string
		length = 18;

		// Offset the buf array to start after debug string section
		length += (short)vsprintf(&buf[length], fmt, arg_ptr);
	}

	// Clean up. Invalidates arg_ptr from use again
	va_end(arg_ptr);

	if (mode == RAW)
	{
		// Print the raw string
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
		UartWrite(buf, length, GLOBAL_DEBUG_PRINT_PORT);
#else
		UartWrite(buf, length, CRAFT_COM_PORT);
#endif
	}
	// Check if the current string to be printed was different than the last
	else if (strncmp(buf, s_uartBuffer, length))
	{
		// Check if the previous string repeated at all
		if (repeatingBuf > 0)
		{
			// Print the repeat count of the previous repeated string
			sprintf(repeatCountStr, "(%d)\n", (int)repeatingBuf);
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
			UartPuts(repeatCountStr, GLOBAL_DEBUG_PRINT_PORT);
#else
			UartPuts(repeatCountStr, CRAFT_COM_PORT);
#endif

			// Reset the counter
			repeatingBuf = 0;
		}
		else if (strippedNewline == YES)
		{
			// Issue a carrige return and a line feed
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
			UartPutc('\r', GLOBAL_DEBUG_PRINT_PORT);
			UartPutc('\n', GLOBAL_DEBUG_PRINT_PORT);
#else
			UartPutc('\r', CRAFT_COM_PORT);
			UartPutc('\n', CRAFT_COM_PORT);
#endif

			// Reset the flag
			strippedNewline = NO;
		}

		// Copy the new string into the global buffer
		strncpy(s_uartBuffer, buf, length);

		tempTime = g_lifetimeHalfSecondTickCount >> 1;

		// Put timestamp into a formatted string
		if (tempTime < 60)
		{
			sprintf(timestampStr, "%6ds", (int)tempTime);
		}
		else if (tempTime < 3600)
		{
			sprintf(timestampStr, "%2dm,%2ds", (int)(tempTime/60), (int)(tempTime%60));
		}
		else if (tempTime < 86400)
		{
			sprintf(timestampStr, "%2dh,%2dm", (int)(tempTime/3600), (int)((tempTime%3600)/60));
		}
		else
		{
			sprintf(timestampStr, "%2dd,%2dh", (int)(tempTime/86400), (int)((tempTime%86400)/3600));
		}

		// Add in the timestamp at print buffer location offset 9
		strncpy(&buf[9], timestampStr, 7);

		// For repeat '!' (bang) processing, look for an ending newline
		if (buf[length - 1] == '\n')
		{
			// Reduce length by one
			length--;

			// Check for a preceding carriage return
			if (buf[length - 1] == '\r')
			{
				length--;
			}

			// Strip trailing newline and replace with a null
			buf[length] = '\0';

			// Set the flag
			strippedNewline = YES;
		}
		// Check for an ending newline carriage return combo
		else if ((buf[length - 2] == '\n') && (buf[length - 1] == '\r'))
		{
			// Reduce length by one
			length -= 2;

			// Strip trailing newline and replace with a null
			buf[length] = '\0';

			// Set the flag
			strippedNewline = YES;
		}

		// Print the new string
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
		UartWrite(buf, length, GLOBAL_DEBUG_PRINT_PORT);
#else
		UartWrite(buf, length, CRAFT_COM_PORT);
#endif
	}
	else // Strings are equal
	{
		// Increment the repeat count
		repeatingBuf++;

		// Print a '!' (bang) so signify that the output was repeated
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
		UartPutc('!', GLOBAL_DEBUG_PRINT_PORT);
#else
		UartPutc('!', CRAFT_COM_PORT);
#endif
	}

	// Return the number of characters
	return (length);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DebugPrintChar(uint8 charData)
{
	if (g_disableDebugPrinting == NO)
	{
#if (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
		UartPutc(charData, GLOBAL_DEBUG_PRINT_PORT);
#else
		UartPutc(charData, CRAFT_COM_PORT);
#endif
	}
}
