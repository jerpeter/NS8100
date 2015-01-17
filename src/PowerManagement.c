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
#include "navigation.h"

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
			//debug("Power Off: %s.\r\n", mode == ON ? "On" : "Off");
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
			//debug("Power Shutdown Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				state = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x80;
				WriteMcp23018(IO_ADDRESS_KPD, OLATA, state);
				s_powerManagement |= POWER_OFF_PROTECTION_ENABLE_BIT;
			} 
			else // (mode == OFF)
			{
				state = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
				state &= ~0x80;
				WriteMcp23018(IO_ADDRESS_KPD, OLATA, state);
				s_powerManagement &= ~(POWER_OFF_PROTECTION_ENABLE_BIT);
			}
			break;

		//----------------------------------------------------------------------------
		case ALARM_1_ENABLE: // Active high control
		//----------------------------------------------------------------------------
			//debug("Alarm 1 Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB06);
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB06);
			}
			break;

		//----------------------------------------------------------------------------
		case ALARM_2_ENABLE: // Active high control
		//----------------------------------------------------------------------------
			//debug("Alarm 2 Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB07);
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB07);
			}
			break;

		//----------------------------------------------------------------------------
		case LCD_POWER_ENABLE: // Active high control
		//----------------------------------------------------------------------------
			//debug("Lcd Sleep Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB21);
				g_LcdPowerState = ENABLED;
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB21);
				g_LcdPowerState = DISABLED;
			}
			break;

		//----------------------------------------------------------------------------
		case LCD_CONTRAST_ENABLE: // Active high control
		//----------------------------------------------------------------------------
			//debug("Lcd Contrast Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB22); 
			}				
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB22);
			}				
			break;

		//----------------------------------------------------------------------------
		case LCD_BACKLIGHT_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lcd Backlight Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_On();
			}				
			else // (mode == OFF)
			{
				Backlight_Off();
			}				
			break;

		//----------------------------------------------------------------------------
		case LCD_BACKLIGHT_HI_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Lcd Backlight Hi Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_High();
			}
			else // (mode == OFF)
			{
				Backlight_Low();
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_232_DRIVER_ENABLE: // Active low control
		//----------------------------------------------------------------------------
			//debug("Serial 232 Driver Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB08);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB08);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_232_RECEIVER_ENABLE: // Active low control
		//----------------------------------------------------------------------------
			//debug("Serial 232 Receiver Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB09);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB09);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_485_DRIVER_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Serial 485 Driver Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB00);
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB00);
			}
			break;

		//----------------------------------------------------------------------------
		case SERIAL_485_RECEIVER_ENABLE:
		//----------------------------------------------------------------------------
#if NS8100_ALPHA
			//debug("Serial 485 Receiver Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB31);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB31);
			}
#endif
			break;

		//----------------------------------------------------------------------------
		case USB_HOST_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Usb Host Sleep Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
			}
			else // (mode == OFF)
			{
				// fix_ns8100
			}
			break;

		//----------------------------------------------------------------------------
		case USB_DEVICE_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Usb Device Sleep Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
			}
			else // (mode == OFF)
			{
				// fix_ns8100
			}
			break;

		//----------------------------------------------------------------------------
		case LAN_SLEEP_ENABLE: // Active low control
		//----------------------------------------------------------------------------
			//debug("Lan Sleep Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB27);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB27);
			}
			break;

		//----------------------------------------------------------------------------
		case ANALOG_SLEEP_ENABLE:
		//----------------------------------------------------------------------------
			//debug("Analog Sleep Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
			}
			else // (mode == OFF)
			{
				// fix_ns8100
			}
			break;

		//----------------------------------------------------------------------------
		case RTC_TIMESTAMP: // Active low control
		//----------------------------------------------------------------------------
			//debug("RTC Timestamp Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB18);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB18);
			}
			break;

		//----------------------------------------------------------------------------
		case TRIGGER_OUT: // Active high control
		//----------------------------------------------------------------------------
			//debug("Trigger Out Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB05);
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB05);
			}
			break;

		//----------------------------------------------------------------------------
		case SDATA: // Active low control
		//----------------------------------------------------------------------------
			//debug("SDATA Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB02);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB02);
			}
			break;

		//----------------------------------------------------------------------------
		case ADATA: // Active low control
		//----------------------------------------------------------------------------
			//debug("ADATA Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB03);
			}
			else // (mode == OFF)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB03);
			}
			break;

		//----------------------------------------------------------------------------
		case USB_LED: // Active high control
		//----------------------------------------------------------------------------
			//debug("USB LED Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB28);
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB28);
			}
			break;

		//----------------------------------------------------------------------------
		case SD_POWER: // Active high control
		//----------------------------------------------------------------------------
			//debug("SD Power Enable: %s.\r\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB15);
			}
			else // (mode == OFF)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB15);
			}
			break;
	}

	// Set Power Control state for the option selected
	if (mode == ON)
	{
		s_powerManagement |= (1 << option);
	}
	else // (mode == OFF)
	{
		s_powerManagement &= ~(1 << option);
	}

#if 0 // Can no longer wait locally since this can be called from within an interrupt
	// Let hardware stabilize
	SoftUsecWait(10 * SOFT_MSECS);
#endif
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
	OverlayMessage(getLangText(STATUS_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), 0);
#if 0 // Removed debug log file due to inducing system problems
	debug("Dumping debug output to debug log file\r\n");
#endif

	debug("Adding On/Off Log timestamp\r\n");

#if 0 // Removed debug log file due to inducing system problems
	WriteDebugBufferToFile();
#endif

	AddOnOffLogTimestamp(OFF);

	// Make sure all open files are closed and data is flushed
	nav_exit();

	// Disable Power Off Protection
	PowerControl(POWER_OFF_PROTECTION_ENABLE, OFF);

	if (powerOffMode == SHUTDOWN_UNIT)
	{
		debug("Powering unit off (shutdown)...\r\n");

		// Shutdown application
		PowerControl(POWER_OFF, ON);
	}
	else
	{
		debug("Powering unit off (reboot)...\r\n");

		Disable_global_interrupt();
		AVR32_WDT.ctrl |= 0x00000001;
	}

	while (1) { /* do nothing */ };
}
