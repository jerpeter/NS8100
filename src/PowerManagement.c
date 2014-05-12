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
#include "Old_Board.h"
#include "Uart.h"
#include "powerManagement.h"
#include "RealTimeClock.h"

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
static POWER_MANAGEMENT_STRUCT s_powerManagement;

/****************************************
*	Function:		powerControl
*	Purpose:
****************************************/
#include "gpio.h"
#include "M23018.h"
#include "lcd.h"
void powerControl(POWER_MGMT_OPTIONS option, BOOLEAN mode)
{
	// NS7100
	//uint16* powerManagementPort = (uint16*)POWER_CONTROL_ADDRESS;
	//uint16* powerManagementPort = &emptyStorage;
	uint8 state;

	switch (option)
	{
		case POWER_OFF:
			//debug("Power Off: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // Skip re-init since it was fond to be causing a lockup
				init_mcp23018(IO_ADDRESS_KPD);
#endif				
				state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x40;
				write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
				s_powerManagement.reg |= POWER_OFF_BIT;
			}			
			else
			{
				// Unit is actually off
			}
			break;

		case POWER_OFF_PROTECTION_ENABLE:
			//debug("Power Shutdown Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // Re-init causing lockup. No longer needed
				//init_mcp23018(IO_ADDRESS_KPD);
				//debug("MCP23018 re-init complete\n");
#endif
				state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x80;
				write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
				//debug("MCP23018 write complete\n");
				s_powerManagement.reg |= POWER_OFF_PROTECTION_ENABLE_BIT;
			} 
			else
			{
#if 0 // Re-init causing lockup. No longer needed
				//init_mcp23018(IO_ADDRESS_KPD);
				//debug("MCP23018 re-init complete\n");
#endif
				state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
				state &= ~0x80;
				write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
				//debug("MCP23018 write complete\n");
				s_powerManagement.reg &= ~(POWER_OFF_PROTECTION_ENABLE_BIT);
			}
			break;

		case ALARM_1_ENABLE:
			//debug("Alarm 1 Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB06);
				s_powerManagement.reg |= ALARM_1_ENABLE_BIT;
			}
			else
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB06);
				s_powerManagement.reg &= ~(ALARM_1_ENABLE_BIT);
			}
			break;

		case ALARM_2_ENABLE:
			//debug("Alarm 2 Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB07);
				s_powerManagement.reg |= ALARM_2_ENABLE_BIT;
			}
			else
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB07);
				s_powerManagement.reg &= ~(ALARM_2_ENABLE_BIT);
			}
			break;

		case LCD_POWER_ENABLE:
			//debug("Lcd Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON) gpio_set_gpio_pin(AVR32_PIN_PB21); else gpio_clr_gpio_pin(AVR32_PIN_PB21);
			if (mode == ON)
			{
				g_LcdPowerState = ENABLED;
				s_powerManagement.reg |= LCD_POWER_ENABLE_BIT;
			}
			else // mode is OFF
			{
				g_LcdPowerState = DISABLED;
				s_powerManagement.reg &= ~(LCD_POWER_ENABLE_BIT);
			}
			break;

		case LCD_CONTRAST_ENABLE:
			//debug("Lcd Contrast Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB22); 
				s_powerManagement.reg |= LCD_CONTRAST_ENABLE_BIT;
			}				
			else
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB22);
				s_powerManagement.reg &= ~(LCD_CONTRAST_ENABLE_BIT);
			}				
			break;

		case LCD_BACKLIGHT_ENABLE:
			//debug("Lcd Backlight Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_On();
				s_powerManagement.reg |= LCD_BACKLIGHT_ENABLE_BIT;
			}				
			else
			{
				Backlight_Off();
				s_powerManagement.reg &= ~(LCD_BACKLIGHT_ENABLE_BIT);
			}				
			break;

		case LCD_BACKLIGHT_HI_ENABLE:
			//debug("Lcd Backlight Hi Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_High();
				s_powerManagement.reg |= LCD_BACKLIGHT_HI_ENABLE_BIT;
			}
			else
			{
				Backlight_Low();
				s_powerManagement.reg &= ~(LCD_BACKLIGHT_HI_ENABLE_BIT);
			}
			break;

		case SERIAL_232_DRIVER_ENABLE:
			//debug("Serial 232 Driver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= SERIAL_232_DRIVER_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(SERIAL_232_DRIVER_ENABLE_BIT);
			}
			break;

		case SERIAL_232_RECEIVER_ENABLE:
			//debug("Serial 232 Receiver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= SERIAL_232_RECEIVER_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(SERIAL_232_RECEIVER_ENABLE_BIT);
			}
			break;

		case SERIAL_485_DRIVER_ENABLE:
			//debug("Serial 485 Driver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= SERIAL_485_DRIVER_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(SERIAL_485_DRIVER_ENABLE_BIT);
			}
			break;

		case SERIAL_485_RECEIVER_ENABLE:
			//debug("Serial 485 Receiver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= SERIAL_485_RECEIVER_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(SERIAL_485_RECEIVER_ENABLE_BIT);
			}
			break;

		case USB_HOST_SLEEP_ENABLE:
			//debug("Usb Host Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= USB_HOST_SLEEP_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(USB_HOST_SLEEP_ENABLE_BIT);
			}
			break;

		case USB_DEVICE_SLEEP_ENABLE:
			//debug("Usb Device Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= USB_DEVICE_SLEEP_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(USB_DEVICE_SLEEP_ENABLE_BIT);
			}
			break;

		case LAN_SLEEP_ENABLE:
			//debug("Lan Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= LAN_SLEEP_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(LAN_SLEEP_ENABLE_BIT);
			}
			break;

		case ANALOG_SLEEP_ENABLE:
			//debug("Analog Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				// fix_ns8100
				s_powerManagement.reg |= ANALOG_SLEEP_ENABLE_BIT;
			}
			else
			{
				// fix_ns8100
				s_powerManagement.reg &= ~(ANALOG_SLEEP_ENABLE_BIT);
			}
			break;
	}

	// Let hardware stabilize
	soft_usecWait(10 * SOFT_MSECS);
}

/****************************************
*	Function:	getPowerControlState
*	Purpose:
****************************************/
BOOLEAN getPowerControlState(POWER_MGMT_OPTIONS option)
{
	BOOLEAN state = OFF;

	switch (option)
	{
		case POWER_OFF:
			if (s_powerManagement.reg & POWER_OFF_BIT) state = ON;
			break;

		case POWER_OFF_PROTECTION_ENABLE:
			if (s_powerManagement.reg & POWER_OFF_PROTECTION_ENABLE_BIT) state = ON;
			break;

		case ALARM_1_ENABLE:
			if (s_powerManagement.reg & ALARM_1_ENABLE_BIT) state = ON;
			break;

		case ALARM_2_ENABLE:
			if (s_powerManagement.reg & ALARM_2_ENABLE_BIT) state = ON;
			break;

		case LCD_POWER_ENABLE:
			if (s_powerManagement.reg & LCD_POWER_ENABLE_BIT) state = ON;
			break;

		case LCD_CONTRAST_ENABLE:
			if (s_powerManagement.reg & LCD_CONTRAST_ENABLE_BIT) state = ON;
			break;

		case LCD_BACKLIGHT_ENABLE:
			if (s_powerManagement.reg & LCD_BACKLIGHT_ENABLE_BIT) state = ON;
			break;

		case LCD_BACKLIGHT_HI_ENABLE:
			if (s_powerManagement.reg & LCD_BACKLIGHT_HI_ENABLE_BIT) state = ON;
			break;

		case SERIAL_232_DRIVER_ENABLE:
			if (s_powerManagement.reg & SERIAL_232_DRIVER_ENABLE_BIT) state = ON;
			break;

		case SERIAL_232_RECEIVER_ENABLE:
			if (s_powerManagement.reg & SERIAL_232_RECEIVER_ENABLE_BIT) state = ON;
			break;

		case SERIAL_485_DRIVER_ENABLE:
			if (s_powerManagement.reg & SERIAL_485_DRIVER_ENABLE_BIT) state = ON;
			break;

		case SERIAL_485_RECEIVER_ENABLE:
			if (s_powerManagement.reg & SERIAL_485_RECEIVER_ENABLE_BIT) state = ON;
			break;

		case USB_HOST_SLEEP_ENABLE:
			if (s_powerManagement.reg & USB_HOST_SLEEP_ENABLE_BIT) state = ON;
			break;

		case USB_DEVICE_SLEEP_ENABLE:
			if (s_powerManagement.reg & USB_DEVICE_SLEEP_ENABLE_BIT) state = ON;
			break;

		case LAN_SLEEP_ENABLE:
			if (s_powerManagement.reg & LAN_SLEEP_ENABLE_BIT) state = ON;
			break;

		case ANALOG_SLEEP_ENABLE:
			if (s_powerManagement.reg & ANALOG_SLEEP_ENABLE_BIT) state = ON;
			break;
	}

	return (state);
}

/****************************************
*	Function:		setMcorePwMgntDefaults
*	Purpose:
****************************************/
#if 0 // ns7100
void setMcorePwMgntDefaults(void)
{
	uint16* powerManagementPort = (uint16*)POWER_CONTROL_ADDRESS;

	//debug("Power Defaults Set\n");
	s_powerManagement.bit.alarm2Enable = 			ON;
	s_powerManagement.bit.alarm1Enable = 			ON;
	s_powerManagement.bit.serial485ReceiverEnable = OFF;
	s_powerManagement.bit.serial485DriverEnable = 	OFF;
	s_powerManagement.bit.serial232ReceiverEnable = ON;
	s_powerManagement.bit.serial232DriverEnable = 	ON;
	s_powerManagement.bit.powerShutdownEnable = 	OFF;
	s_powerManagement.bit.powerOff = 				OFF;
	s_powerManagement.bit.lcdBacklightHiEnable = 	OFF;
	s_powerManagement.bit.lcdBacklightEnable = 		OFF;
	s_powerManagement.bit.lcdContrastEnable = 		OFF;
	s_powerManagement.bit.lcdSleepEnable = 			OFF;
	s_powerManagement.bit.analogSleepEnable = 		ON;
	s_powerManagement.bit.lanSleepEnable = 			ON;
	s_powerManagement.bit.usbDeviceSleepEnable = 	ON;
	s_powerManagement.bit.usbHostSleepEnable = 		ON;

	//s_powerManagement.reg = 0xCC0F;

	*powerManagementPort = s_powerManagement.reg;

	soft_usecWait(10 * SOFT_MSECS);
}
#endif

/****************************************
*	Function:	void PowerUnitOff(uint8)
*	Purpose:
****************************************/
void PowerUnitOff(uint8 powerOffMode)
{
#if 0 // ns7100
	uint16* powerManagementPort = (uint16*)POWER_CONTROL_ADDRESS;
#endif

	if (powerOffMode == SHUTDOWN_UNIT)
	{
		debug("Powering unit off (shutdown)...\n");
#if 0 // ns7100
		RTC_ENABLES.bit.periodicIntEnable = OFF;
		RTC_ENABLES.bit.powerFailIntEnable = OFF;
#endif
	}
	else
	{
		debug("Powering unit off (reboot)...\n");
	}

	// Shutdown application
	//s_powerManagement.bit.powerOff = ON;
	powerControl(POWER_OFF, ON);

#if 0 // ns7100
	*powerManagementPort = s_powerManagement.reg;
	soft_usecWait(10 * SOFT_MSECS);
#endif

	while (1) { /* do nothing */ };
}
