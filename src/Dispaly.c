///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Dispaly.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:47 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Dispaly.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Typedefs.h"
#include "Display.h"
#include "Board.h"
#include "Common.h"
#include "PowerManagement.h"
#include "Mmc2114.h"
#include "Uart.h"
#include "Rec.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define DEFAULT_START_LINE 0
#define DEFAULT_X_LOC 0
#define	DEFAULT_Y_LOC 0

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern POWER_MANAGEMENT_STRUCT powerManagement;
extern uint8 contrast_value;
extern REC_HELP_MN_STRUCT help_rec;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
uint8 contrast_value;
uint8 g_LcdPowerState = ENABLED;

//  00C0_0000  reg_PORTA   Port A Output Data Register             Supervisor / User
//  00C0_0001  reg_PORTB   Port B Output Data Register             Supervisor / User
//  00C0_000C  reg_DDRA    Port A Data Direction Register          Supervisor / User
//  00C0_000D  reg_DDRB    Port B Data Direction Register          Supervisor / User
//  00C0_0018  reg_SETA    Port A Set Data Register                Supervisor / User
//  00C0_0019  reg_SETB    Port B Set Data Register                Supervisor / User
//  00C0_0024  reg_CLRA    Port A Clear Output Data Register       Supervisor / User
//  00C0_0025  reg_CLRB    Port B Clear Output Data Register       Supervisor / User

/****************************************
*	Function:	LcdInit
*	Purpose:	
****************************************/
void LcdInit(void)
{
#if 0 // convert to new hardware
	// Clear the data in Port C and Port D
	reg_CLRC.reg = 0x00;
	reg_CLRD.reg = 0x00;

	// Set Port C and Port D pins as outputs
	reg_DDRC.reg = 0xFF;
	reg_DDRD.reg = 0xFF;
#endif	
}

// *** NOTE ***
// reg_PortD converted to reg_PortA
//

/****************************************
*	Function:	LcdResetPulse
*	Purpose:	
****************************************/
void LcdResetPulse(void)
{
#if 0 // fix_ns8100
	// Reset is active low, put LCD in reset
	reg_PORTA.reg &= ~LCD_RESET_BIT;

	// Take LCD out of reset
	reg_PORTA.reg |= LCD_RESET_BIT;
#endif
}

/****************************************
*	Function:	LcdWrite
*	Purpose:	
****************************************/
void LcdWrite(uint8 mode, uint8 data, uint8 segment)
{

#if	1
#include "lcd.h"
extern uint16 lcd_port_image;
extern void soft_delay(volatile unsigned long int);

    volatile unsigned short *lcd = ((void *)AVR32_EBI_CS0_ADDRESS);
	uint8 lcd_register, display_half;

	if (mode == LCD_INSTRUCTION)
		lcd_register = COMMAND_REGISTER;
	else
		lcd_register = DATA_REGISTER;

	if (segment == LCD_SEGMENT1)
		display_half = FIRST_HALF_DISPLAY;
	else
		display_half = SECOND_HALF_DISPLAY;

    //Write data
    lcd_port_image = ((lcd_port_image & 0xFF00) | data);
    *lcd = lcd_port_image;

    if (lcd_register == COMMAND_REGISTER)
    {
        //Set RS low
        lcd_port_image &= ~LCD_RS;
        *lcd = lcd_port_image;
   }
   else
   {
       //Set RS high
       lcd_port_image |= LCD_RS;
       *lcd = lcd_port_image;
    }

    if (display_half == FIRST_HALF_DISPLAY)
   {
        //Set write low and CS2 low
        lcd_port_image &= (~LCD_READ_WRITE & ~LCD_CS2);
        *lcd = lcd_port_image;
    }
   else
   {
       //Set write low and CS1 low
       lcd_port_image &= (~LCD_READ_WRITE & ~LCD_CS1);
       *lcd = lcd_port_image;
   }

    //Set E high
    lcd_port_image |= LCD_ENABLE;
    *lcd = lcd_port_image;

    soft_delay(10);

    //Set E low
    lcd_port_image &= ~LCD_ENABLE;
    *lcd = lcd_port_image;

    soft_delay(10);

    //Set write, CS1, CS2 and address high
    lcd_port_image |= (LCD_READ_WRITE | LCD_CS1 | LCD_CS2 | LCD_RS);
    *lcd = lcd_port_image;

    soft_delay(10);
#endif

#if 0 

	uint16 commandAndDataWord = 0x0;

	if (segment == LCD_SEGMENT1)
	{
		commandAndDataWord |= LCD_CS1_BIT;
		commandAndDataWord &= ~LCD_CS2_BIT;
	}
	else // segment == LCD_SEGMENT2
	{
		commandAndDataWord &= ~LCD_CS1_BIT;
		commandAndDataWord |= LCD_CS2_BIT;
	}

	if (mode == LCD_DATA)
	{
		commandAndDataWord |= LCD_RS_BIT;
	}
	else // mode == LCD_INSTRUCTION
	{
		commandAndDataWord &= ~LCD_RS_BIT;
	}

	commandAndDataWord &= ~LCD_READ_WRITE_BIT;
	commandAndDataWord |= LCD_RESET_BIT;
		
	commandAndDataWord |= (data & 0x00FF);

#if 0
	// Set LCD data pins as output
	reg_DDRC.reg |= 0xFF; // Top bit is not connected, leave as output
#endif

	// Clock LCD data in by transitioning Enable from high to low
	*((uint16*)0xC0000000) = (commandAndDataWord | LCD_ENABLE_BIT);
	soft_usecWait(10);
	*((uint16*)0xC0000000) = (commandAndDataWord & ~LCD_ENABLE_BIT);
	soft_usecWait(10);
#endif
}

/****************************************
*	Function:	LcdRead
*	Purpose:	
****************************************/
#if 0
uint8 LcdRead(uint8 mode, uint8 segment)
{
	uint8 data = 0x00;

	if (segment == LCD_SEGMENT1)
	{
		reg_PORTA.reg |= LCD_CS1_BIT;
		reg_PORTA.reg &= ~LCD_CS2_BIT;
	}
	else // segment == LCD_SEGMENT2
	{
		reg_PORTA.reg &= ~LCD_CS1_BIT;
		reg_PORTA.reg |= LCD_CS2_BIT;
	}

	if (mode == LCD_DATA)
	{
		reg_PORTA.reg |= LCD_RS_BIT;
	}
	else // mode == LCD_INSTRUCTION
	{
		reg_PORTA.reg &= ~LCD_RS_BIT;
	}

	reg_PORTA.reg |= LCD_READ_WRITE_BIT;
	reg_PORTA.reg |= LCD_RESET_BIT;
	
	// Set Data Pins as input
#if 0 // Use with new hardware?
	reg_DDRD.reg &= 0x7F; // Top bit is least significant bit of data
	reg_DDRC.reg &= 0x80; // Top bit is not connexted, leave as output
#endif
	
	// Clock LCD data in by transitioning Enable from high to low
	reg_PORTA.reg |= LCD_ENABLE_BIT;
	soft_usecWait(1);
	reg_PORTA.reg &= ~LCD_ENABLE_BIT;
	soft_usecWait(1);

	// Read in the data for all but the most significant bit and shift (misalignment with Port D)
	data = (uint8)(reg_PORTC.reg << 1);

	// Check if the most significant bit is set
	if (reg_PORTA.reg && 0x80)
	{
		data |= 0x01;
	}
	else // Most significant bit is a zero
	{
		data &= 0xFE;
	}
	
	return (data);
}
#endif

#if 0
/****************************************
*	Function:	clockDataFromLcd
*	Purpose:	
****************************************/
inline uint8 clockDataFromLcd(uint8 lcdCmd)
{
	// Take Eanble high
    *((volatile uint8*)LCD_CMD_PORT) = lcdCmd;
    
	// Take Eanble low which will clock in the LCD data
    *((volatile uint8*)LCD_CMD_PORT) = (uint8)(lcdCmd & ~(LCD_ENABLE));

	// Read out LCD data
    return (*((volatile uint8*)LCD_DATA_PORT));
}
#endif

#if 0
/****************************************
*	Function:	clockDataToLcd
*	Purpose:	
****************************************/
inline void clockDataToLcd(uint8 lcdCmd, uint8 lcdData)
{
	// Take Eanble high
    *((volatile uint8*)LCD_CMD_PORT) = lcdCmd;
    
	// Write in LCD data
    *((volatile uint8*)LCD_DATA_PORT) = lcdData;
    
	// Take Eanble low which will clock in the LCD data
    *((volatile uint8*)LCD_CMD_PORT) = (uint8)(lcdCmd & ~(LCD_ENABLE));
}
#endif

/****************************************
*	Function:	waitForLcdReady
*	Purpose:	
****************************************/
inline void waitForLcdReady(uint8 segment)
{
	UNUSED(segment);
	// Wait for Lcd busy flag to clear
	//while (LcdRead(LCD_INSTRUCTION, segment) & LCD_BUSY_FLAG) {};
}

#if 0
/****************************************
*	Function:	readLcdData
*	Purpose:	
****************************************/
inline uint8 readLcdData(uint8 segment)
{
	return (LcdRead(LCD_DATA, segment));
}
#endif

#if 0
/****************************************
*	Function:	writeLcdData
*	Purpose:	
****************************************/
inline void writeLcdData(uint8 lcdData, uint8 segment)
{
	LcdWrite(LCD_DATA, lcdData, segment);
	
	waitForLcdReady(segment);
}
#endif

/****************************************
*	Function:	setLcdStartLine
*	Purpose:	
****************************************/
inline void setLcdStartLine(uint8 lcdData, uint8 segment)
{
	// Or in Start Line instruction to LCD data
    lcdData |= (uint8)(LCD_START_LINE_INSTRUCTION);
	
	LcdWrite(LCD_INSTRUCTION, lcdData, segment);
	
	waitForLcdReady(segment);
}

/****************************************
*	Function:	setLcdXPosition
*	Purpose:	
****************************************/
inline void setLcdXPosition(uint8 lcdData, uint8 segment)
{
	// Or in X location instruction to the row selection in LCD data
    lcdData |= LCD_SET_PAGE;

	LcdWrite(LCD_INSTRUCTION, lcdData, segment);
		
	waitForLcdReady(segment);
}

/****************************************
*	Function:	setLcdYPosition
*	Purpose:	
****************************************/
inline void setLcdYPosition(uint8 lcdData, uint8 segment)
{
	// Or in Y location instruction to the column slection in LCD data
    lcdData |= LCD_SET_ADDRESS;
	
	LcdWrite(LCD_INSTRUCTION, lcdData, segment);

	waitForLcdReady(segment);
}

/****************************************
*	Function:	setLcdMode
*	Purpose:	
****************************************/
inline void setLcdMode(uint8 lcdData, uint8 segment)
{
	// Check if LCD data is set for turining on or off display
	if (lcdData == LCD_DISPLAY_ON)
	{
		// Write in Turn on Display command
		lcdData = (uint8)(LCD_DISPLAY_ON_INSTRUCTION);
	}
	else
	{ 
		// Write in Turn off Display command
		lcdData = (uint8)(LCD_DISPLAY_OFF_INSTRUCTION);
	}

	LcdWrite(LCD_INSTRUCTION, lcdData, segment);
	
	waitForLcdReady(segment);
}

/****************************************
*	Function:	DsplySetOrigin
*	Purpose:	
****************************************/
inline void setLcdOrigin(uint8 x, uint8 y, uint8 segment)
{
	// Set both X and Y coordinate positions
    setLcdXPosition(x, segment);
    setLcdYPosition(y, segment);
}

/****************************************
*	Function:	writeStringToLcd
*	Purpose:	Untested...
****************************************/
void writeStringToLcd(uint8* p, uint8 x, uint8 y, uint8 (*table_ptr)[2][10])
{
	// =========================================
	// Warning: This routine has not been tested
	// =========================================
	
	uint8* font_char_ptr;
	uint8 font_size;
	uint8 segment;
	uint8 pixel_byte;

	// Check if the LCD has power. If not, cant get status, so return
	if (g_LcdPowerState != ENABLED)
		return;

	/* THIS ROUTINE WAS FOR TESTING THE LCD ONLY */	
	segment = LCD_SEGMENT1;
	setLcdXPosition(x, segment);
	setLcdYPosition(y, segment);

	while (*p != '\0')
	{
		font_char_ptr = table_ptr[(*p)][0];	
		font_size = 6;
		pixel_byte = 0;

		while (pixel_byte<font_size)
		{
			if (segment == LCD_SEGMENT1)
			{
				if ((y + pixel_byte) > SEGMENT_ONE_BLOCK_BORDER) /*NOTE: BORDER is 63 its zero based 0 - 63 */
				{
					segment = LCD_SEGMENT2;
					setLcdXPosition(x, segment);

					setLcdYPosition((uint8)(y - 64), segment);
				}
			}
			else
			{
				if ((y + pixel_byte) > SEGMENT_TWO_BLOCK_BORDER) /* NOTE SEG2 BORDER IS 127 0 - 127 */
				{
					return; // NO WORD WRAP
				}
			}

			//writeLcdData(*(font_char_ptr + pixel_byte),segment);
			LcdWrite(LCD_DATA, *(font_char_ptr + pixel_byte), segment);
			waitForLcdReady(segment);

			pixel_byte++;
		}
		
		y = (uint8)(y + font_size);
		p++;
	}
}

/****************************************
*	Function:	writeMapToLcd
*	Purpose:	
****************************************/
void writeMapToLcd(uint8 (*mmap_ptr)[128])
{
    uint8 segment;
    uint8* pixel_byte_ptr;
    uint8 col_index;
    uint8 row_index;

#if 0 // fix_ns8100 // Skip while testing
	// Check if the LCD has power. If not, cant get status, so return
	if (g_LcdPowerState != ENABLED)
		return;
#endif

	row_index = 0;

    while (row_index < 8)
	{
	    col_index = 0;
	    pixel_byte_ptr = (uint8*)mmap_ptr[row_index];	
            
        segment = LCD_SEGMENT1;
		setLcdOrigin(row_index, DEFAULT_Y_LOC, segment);        

        while (col_index <= SEGMENT_ONE_BLOCK_BORDER)
        {
			// Normal command replaced by code below
			//writeLcdData(*(pixel_byte_ptr + col_index),segment);

			// Optimized writeLcdData operation for speed
			//clockDataToLcd(lcdCmdSeg1, *(pixel_byte_ptr + col_index));
			//while (clockDataFromLcd(lcdStatusSeg1) & LCD_BUSY_FLAG) {};
			// End of optimized writeLcdData operation

			LcdWrite(LCD_DATA, *(pixel_byte_ptr + col_index), segment);
			waitForLcdReady(segment);

			col_index++;
		}
		
        segment = LCD_SEGMENT2;
		setLcdOrigin(row_index, DEFAULT_Y_LOC, segment);        

        while (col_index <= SEGMENT_TWO_BLOCK_BORDER)
        {
			// Normal command replaced by code below
			//writeLcdData(*(pixel_byte_ptr + col_index),segment);

			// Optimized writeLcdData operation for speed
			//clockDataToLcd(lcdCmdSeg2, *(pixel_byte_ptr + col_index));
			//while (clockDataFromLcd(lcdStatusSeg2) & LCD_BUSY_FLAG) {};
			// End of optimized writeLcdData operation

			LcdWrite(LCD_DATA, *(pixel_byte_ptr + col_index), segment);
			waitForLcdReady(segment);

			col_index++;
		}

        row_index++;     
    }
}

/****************************************
*	Function:	initLcdDisplay
*	Purpose:	
****************************************/
void initLcdDisplay(void)
{    
	// Issue reset pulse to LCD display
	LcdResetPulse();
	
	// Make sure LCD is ready for commands
	waitForLcdReady(LCD_SEGMENT1);

	// Init LCD segment 1
	setLcdMode(LCD_DISPLAY_ON, LCD_SEGMENT1);
	setLcdStartLine(DEFAULT_START_LINE, LCD_SEGMENT1);
	setLcdOrigin(DEFAULT_X_LOC, DEFAULT_Y_LOC, LCD_SEGMENT1);

	// Init LCD segment 2
	setLcdMode(LCD_DISPLAY_ON, LCD_SEGMENT2);
	setLcdStartLine(DEFAULT_START_LINE, LCD_SEGMENT2);
	setLcdOrigin(DEFAULT_X_LOC, DEFAULT_Y_LOC, LCD_SEGMENT2);

	// Clear the LCD display
	clearLcdDisplay();
}

/****************************************
*	Function:	clearControlLinesLcdDisplay
*	Purpose:	
****************************************/
void clearControlLinesLcdDisplay(void)
{
	LcdWrite(LCD_INSTRUCTION, 0x00, LCD_SEGMENT1);
	LcdWrite(LCD_DATA, 0x00, LCD_SEGMENT1);
}

/****************************************
*	Function:	void clearLcdDisplay(void)
*	Purpose:	
****************************************/
void clearLcdDisplay(void)
{
	// Turn all of the LCD pixels off (0's), effectively clearing the display
    byteSet(&(mmap[0][0]), 0, sizeof(mmap));
    writeMapToLcd(mmap);
}

/****************************************
*	Function:	void fillLcdDisplay(void)
*	Purpose:	
****************************************/
void fillLcdDisplay(void)
{
	// Turn all of the LCD pixels on (1's), effectively filling the display
    byteSet(&(mmap[0][0]), 0xff, sizeof(mmap));
    writeMapToLcd(mmap);
}


/****************************************
*	Function:	void backLightState(LCD_BACKLIGHT_STATES state)
*	Purpose:	
****************************************/
void setNextLcdBacklightState(void)
{
	LCD_BACKLIGHT_STATES backlightState;

	// Get current backlight state
    backlightState = getLcdBacklightState();

	// Advance to next backlight state
	switch (backlightState)
	{
		case BACKLIGHT_OFF		: setLcdBacklightState(BACKLIGHT_DIM);		break;
		case BACKLIGHT_DIM		: setLcdBacklightState(BACKLIGHT_BRIGHT);	break;
		case BACKLIGHT_BRIGHT	: setLcdBacklightState(BACKLIGHT_OFF);		break;
	}
}

/****************************************
*	Function:	void backLightState(LCD_BACKLIGHT_STATES state)
*	Purpose:	
****************************************/
LCD_BACKLIGHT_STATES getLcdBacklightState(void)
{    
	if (getPowerControlState(LCD_BACKLIGHT_ENABLE) == ON)
	{
		if (getPowerControlState(LCD_BACKLIGHT_HI_ENABLE) == ON)
		{
			return (BACKLIGHT_BRIGHT);
		}
		else // getPowerControlState(LCD_BACKLIGHT_ENABLE) == OFF
		{
			return (BACKLIGHT_DIM);
		}
	}
	else
	{
		return (BACKLIGHT_OFF);
	}
}

/****************************************
*	Function:	void backLightState(LCD_BACKLIGHT_STATES state)
*	Purpose:	
****************************************/
void setLcdBacklightState(LCD_BACKLIGHT_STATES state)
{    
	switch (state)
	{
		case BACKLIGHT_OFF:
			powerControl(LCD_BACKLIGHT_ENABLE, OFF);
			powerControl(LCD_BACKLIGHT_HI_ENABLE, OFF);
		break;

		case BACKLIGHT_DIM:
			if (getPowerControlState(LCD_BACKLIGHT_ENABLE) == OFF)
			{
				powerControl(LCD_BACKLIGHT_ENABLE, ON);
			}

			powerControl(LCD_BACKLIGHT_HI_ENABLE, OFF);
		break;

		case BACKLIGHT_BRIGHT:
			if (getPowerControlState(LCD_BACKLIGHT_ENABLE) == OFF)
			{
				powerControl(LCD_BACKLIGHT_ENABLE, ON);
			}

			powerControl(LCD_BACKLIGHT_HI_ENABLE, ON);
		break;
	}
}

/*******************************************************************************
 *  Function:  		adjustLcdContrast
 *  Purpose :  		Adjust the contrast either lighter or darker
 *******************************************************************************/
void adjustLcdContrast(CONTRAST_ADJUSTMENT adjust)
{
	switch (adjust)
	{
		case LIGHTER:
			// Check if an increment in contrast will exceed the max
			if ((contrast_value + CONTRAST_STEPPING) <= MAX_CONTRAST)
			{
				contrast_value += CONTRAST_STEPPING;
			} 

			setLcdContrast(contrast_value);
			break;
		
		case DARKER:
			// Check if an deincrement in contrast will exceed the min
			if ((contrast_value - CONTRAST_STEPPING) >= MIN_CONTRAST)
			{
				contrast_value -= CONTRAST_STEPPING;
			} 

			setLcdContrast(contrast_value);
			break;
	}
	
	saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
}

/*******************************************************************************
 *  Function:  		set_lcd_contrast
 *  Description:	This function sets the LCD contrast voltage on the
 *                  LCD display by writing a number to the contrast
 *                  voltage generation circuit.
 *******************************************************************************/
void setLcdContrast(uint8 cmd)
{
	//uint16* powerManagementPort = (uint16*)POWER_CONTROL_ADDRESS;
	uint32 i;

	// Check if lcd contrast adjustment is out of visable range
	if (cmd > DEFAULT_MAX_CONTRAST)
	{
		contrast_value = DEFAULT_MAX_CONTRAST;
		return;
	}

	// ADJ  CTRL 
	//  0    0 --> Wiper(counter) stays where is was set, -V is off
	//  0    1 --> Wiper(counter) stays where is was set, -V is on
	//  1    0 --> Wiper(counter) is reset to midpoint/mid-level/32 and -V is off
	//  RE   1 --> Wiper(counter) is incremented (wraps on high boundary 64) and -V is on

	// New Board
	// ---------
	// ADJ  is Power Management bit LCD_CONTRAST_ENABLE
	// CTRL is Power Management bit LCD_POWER_ENABLE
	// RE is a rising edge

	// Old Board
	// ---------
	// ADJ  is reg_PORTE.reg bit 4
	// CTRL is powerManagement.bit.lcs
	// RE is a rising edge

	// see if less than half or more
	if (cmd < 32)
	{
		// less than half so run to max and then wrap
		cmd = (uint8)(cmd + 32);
	}
	else
	{
		// more than half so just add difference from half to desired position
		cmd = (uint8)(cmd - 32);  
	}

	// Section to reset the Wiper(counter)
	//reg_PORTE.reg |= 0x04;      // Set adjust high
	//soft_usecWait(LCD_ACCESS_DELAY);
	//powerManagement.bit.lcdContrastEnable = OFF;
	//*powerManagementPort = powerManagement.reg; // Set ctrl low
	//soft_usecWait(1000);
	// Enables wiper(counter) adjustment
	//powerManagement.bit.lcdContrastEnable = ON;
	//*powerManagementPort = powerManagement.reg; // Set ctrl high
	//soft_usecWait(LCD_ACCESS_DELAY);
	//reg_PORTE.reg &= ~0x04;      // Set adjust low
	//soft_usecWait(LCD_ACCESS_DELAY);

	// Section to reset the Wiper(counter)
	powerControl(LCD_CONTRAST_ENABLE, ON); // Set adjust high
	powerControl(LCD_POWER_ENABLE, OFF); // Set control low
	soft_usecWait(LCD_ACCESS_DELAY); // Delay

	// Enables wiper(counter) adjustment
	powerControl(LCD_POWER_ENABLE, ON); // Set control high	
	soft_usecWait(LCD_ACCESS_DELAY); // Delay

	powerControl(LCD_CONTRAST_ENABLE, OFF); // Set adjust low
	soft_usecWait(LCD_ACCESS_DELAY); // Delay

	// Section to adjust the wiper(counter)
	for (i = 0; i < cmd; i++)
	{
		//reg_PORTE.reg |= 0x04;   // Set adjust high
		//soft_usecWait(LCD_ACCESS_DELAY);
		//reg_PORTE.reg &= ~0x04;  // Set adjust low
		//soft_usecWait(LCD_ACCESS_DELAY);
		
		powerControl(LCD_CONTRAST_ENABLE, ON); // Set adjust high
		soft_usecWait(LCD_ACCESS_DELAY);
		
		powerControl(LCD_CONTRAST_ENABLE, OFF); // Set adjust low
		soft_usecWait(LCD_ACCESS_DELAY);
	}
}
