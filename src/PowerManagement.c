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
#include "Typedefs.h"
#include "Common.h"
#include "Uart.h"
#include "powerManagement.h"
#include "RealTimeClock.h"
#include "gpio.h"
#include "M23018.h"
#include "lcd.h"

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
static uint32 s_powerManagement;

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerControl(POWER_MGMT_OPTIONS option, BOOLEAN mode)
{
	uint8 state;

	switch (option)
	{
		//----------------------------------------------------------------------------
		case POWER_OFF:
		//----------------------------------------------------------------------------
			//debug("Power Off: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				state = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x40;
				WriteMcp23018(IO_ADDRESS_KPD, OLATA, state);
				s_powerManagement |= POWER_OFF_BIT;
			}			
			else // (mode == OFF)
			{
				// Unit is actually off
			}
			break;

		//----------------------------------------------------------------------------
		case POWER_OFF_PROTECTION_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Power Shutdown Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				state = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x80;
				WriteMcp23018(IO_ADDRESS_KPD, OLATA, state);
				//debug("MCP23018 write complete\n");
				s_powerManagement |= POWER_OFF_PROTECTION_ENABLE_BIT;
			} 
			else // (mode == OFF)
			{
				state = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
				state &= ~0x80;
				WriteMcp23018(IO_ADDRESS_KPD, OLATA, state);
				//debug("MCP23018 write complete\n");
				s_powerManagement &= ~(POWER_OFF_PROTECTION_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case ALARM_1_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Alarm 1 Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB06);
				s_powerManagement |= ALARM_1_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB06);
				s_powerManagement &= ~(ALARM_1_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case ALARM_2_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Alarm 2 Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB07);
				s_powerManagement |= ALARM_2_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB07);
				s_powerManagement &= ~(ALARM_2_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case LCD_POWER_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lcd Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB21);
				g_LcdPowerState = ENABLED;
				s_powerManagement |= LCD_POWER_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB21);
				g_LcdPowerState = DISABLED;
				s_powerManagement &= ~(LCD_POWER_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case LCD_CONTRAST_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lcd Contrast Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB22); 
				s_powerManagement |= LCD_CONTRAST_ENABLE_BIT;
			}				
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB22);
				s_powerManagement &= ~(LCD_CONTRAST_ENABLE_BIT);
			}				
			break;

		//----------------------------------------------------------------------------
		case LCD_BACKLIGHT_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lcd Backlight Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_On();
				s_powerManagement |= LCD_BACKLIGHT_ENABLE_BIT;
			}				
			else // (mode == OFF)
			{
				Backlight_Off();
				s_powerManagement &= ~(LCD_BACKLIGHT_ENABLE_BIT);
			}				
			break;

		//----------------------------------------------------------------------------
		case LCD_BACKLIGHT_HI_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lcd Backlight Hi Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_High();
				s_powerManagement |= LCD_BACKLIGHT_HI_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				Backlight_Low();
				s_powerManagement &= ~(LCD_BACKLIGHT_HI_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_232_DRIVER_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Serial 232 Driver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB08);
				s_powerManagement |= SERIAL_232_DRIVER_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB08);
				s_powerManagement &= ~(SERIAL_232_DRIVER_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_232_RECEIVER_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Serial 232 Receiver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB09);
				s_powerManagement |= SERIAL_232_RECEIVER_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB09);
				s_powerManagement &= ~(SERIAL_232_RECEIVER_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_485_DRIVER_ENABLE:
		//----------------------------------------------------------------------------
		// fix_ns8100 - No direct control over 485 until next hardware spin which will likely be PB20 for driver/receiver combo
			//debug("Serial 485 Driver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				//gpio_clr_gpio_pin(AVR32_PIN_PB20);
				s_powerManagement |= SERIAL_485_DRIVER_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				//gpio_set_gpio_pin(AVR32_PIN_PB20);
				s_powerManagement &= ~(SERIAL_485_DRIVER_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_485_RECEIVER_ENABLE:
		//----------------------------------------------------------------------------
		// fix_ns8100 - No direct control over 485 until next hardware spin which will likely be PB20 for driver/receiver combo
			//debug("Serial 485 Receiver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				//gpio_clr_gpio_pin(AVR32_PIN_PB20);
				s_powerManagement |= SERIAL_485_RECEIVER_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				//gpio_clr_gpio_pin(AVR32_PIN_PB20);
				s_powerManagement &= ~(SERIAL_485_RECEIVER_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case USB_HOST_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Usb Host Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement |= USB_HOST_SLEEP_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				// fix_ns8100
				s_powerManagement &= ~(USB_HOST_SLEEP_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case USB_DEVICE_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Usb Device Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement |= USB_DEVICE_SLEEP_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				// fix_ns8100
				s_powerManagement &= ~(USB_DEVICE_SLEEP_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case LAN_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lan Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement |= LAN_SLEEP_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				// fix_ns8100
				s_powerManagement &= ~(LAN_SLEEP_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case ANALOG_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Analog Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement |= ANALOG_SLEEP_ENABLE_BIT;
			}
			else // (mode == OFF)
			{
				// fix_ns8100
				s_powerManagement &= ~(ANALOG_SLEEP_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case RTC_TIMESTAMP:
		//----------------------------------------------------------------------------
			//debug("RTC Timestamp Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB18);
				s_powerManagement |= RTC_TIMESTAMP_BIT;
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB18);
				s_powerManagement &= ~(RTC_TIMESTAMP_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case TRIGGER_OUT:
		//----------------------------------------------------------------------------
			//debug("Trigger Out Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB05);
				s_powerManagement |= TRIGGER_OUT_BIT;
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB05);
				s_powerManagement &= ~(TRIGGER_OUT_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case SDATA:
		//----------------------------------------------------------------------------
			//debug("SDATA Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB02);
				s_powerManagement |= SDATA_BIT;
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB02);
				s_powerManagement &= ~(SDATA_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case ADATA:
		//----------------------------------------------------------------------------
			//debug("ADATA Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB03);
				s_powerManagement |= ADATA_BIT;
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB03);
				s_powerManagement &= ~(ADATA_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case USB_LED:
		//----------------------------------------------------------------------------
			//debug("USB LED Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB28);
				s_powerManagement |= USB_LED_BIT;
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB28);
				s_powerManagement &= ~(USB_LED_BIT);
			}
			break;
	}

	// Let hardware stabilize
	SoftUsecWait(10 * SOFT_MSECS);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
BOOLEAN GetPowerControlState(POWER_MGMT_OPTIONS option)
{
	BOOLEAN state = OFF;

	if (s_powerManagement & (1 << option))
	{
		state = ON;
	}

	return (state);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerUnitOff(uint8 powerOffMode)
{
	if (powerOffMode == SHUTDOWN_UNIT)
	{
		debug("Powering unit off (shutdown)...\n");
	}
	else
	{
		debug("Powering unit off (reboot)...\n");
	}

	// Shutdown application
	PowerControl(POWER_OFF, ON);

	while (1) { /* do nothing */ };
}
