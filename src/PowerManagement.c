///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: PowerManagement.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:55 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/PowerManagement.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include "Typedefs.h"
#include "Common.h"
#include "Old_Board.h"
#include "Uart.h"
#include "PowerManagement.h"
#include "Mmc2114.h"
#include "RealTimeClock.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern uint8 g_LcdPowerState;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
POWER_MANAGEMENT_STRUCT powerManagement;

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
				init_mcp23018(IO_ADDRESS_KPD);
				state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x40;
				write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
				powerManagement.reg |= POWER_OFF_BIT;
			}			
			else
			{
				// Unit is actually off
			}
			break;

		case POWER_SHUTDOWN_ENABLE:
			//debug("Power Shutdown Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				init_mcp23018(IO_ADDRESS_KPD);
				state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
				state &= ~0x80;
				write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
				powerManagement.reg |= POWER_SHUTDOWN_ENABLE_BIT;
			} 
			else
			{
				init_mcp23018(IO_ADDRESS_KPD);
				state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
				state |= 0x80;
				write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
				powerManagement.reg &= ~(POWER_SHUTDOWN_ENABLE_BIT);
			}
			break;

		case ALARM_1_ENABLE:
			//debug("Alarm 1 Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB06);
				powerManagement.reg |= ALARM_1_ENABLE_BIT;
			}
			else
			{
				gpio_set_gpio_pin(AVR32_PIN_PB06);
				powerManagement.reg &= ~(ALARM_1_ENABLE_BIT);
			}
			break;

		case ALARM_2_ENABLE:
			//debug("Alarm 2 Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB07);
				powerManagement.reg |= ALARM_2_ENABLE_BIT;
			}
			else
			{
				gpio_set_gpio_pin(AVR32_PIN_PB07);
				powerManagement.reg &= ~(ALARM_2_ENABLE_BIT);
			}
			break;

		case LCD_POWER_ENABLE:
			//debug("Lcd Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON) gpio_set_gpio_pin(AVR32_PIN_PB21); else gpio_clr_gpio_pin(AVR32_PIN_PB21);
			if (mode == ON)
			{
				g_LcdPowerState = ENABLED;
				powerManagement.reg |= LCD_POWER_ENABLE_BIT;
			}
			else // mode is OFF
			{
				g_LcdPowerState = DISABLED;
				powerManagement.reg &= ~(LCD_POWER_ENABLE_BIT);
			}
			break;

		case LCD_CONTRAST_ENABLE:
			//debug("Lcd Contrast Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				gpio_set_gpio_pin(AVR32_PIN_PB22); 
				powerManagement.reg |= LCD_CONTRAST_ENABLE_BIT;
			}				
			else
			{
				gpio_clr_gpio_pin(AVR32_PIN_PB22);
				powerManagement.reg &= ~(LCD_CONTRAST_ENABLE_BIT);
			}				
			break;

		case LCD_BACKLIGHT_ENABLE:
			//debug("Lcd Backlight Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_On();
				powerManagement.reg |= LCD_BACKLIGHT_ENABLE_BIT;
			}				
			else
			{
				Backlight_Off();
				powerManagement.reg &= ~(LCD_BACKLIGHT_ENABLE_BIT);
			}				
			break;

		case LCD_BACKLIGHT_HI_ENABLE:
			//debug("Lcd Backlight Hi Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
				Backlight_High();
				powerManagement.reg |= LCD_BACKLIGHT_HI_ENABLE_BIT;
			}
			else
			{
				Backlight_Low();
				powerManagement.reg &= ~(LCD_BACKLIGHT_HI_ENABLE_BIT);
			}
			break;

		case SERIAL_232_DRIVER_ENABLE:
			//debug("Serial 232 Driver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= SERIAL_232_DRIVER_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(SERIAL_232_DRIVER_ENABLE_BIT);
			}
			break;

		case SERIAL_232_RECEIVER_ENABLE:
			//debug("Serial 232 Receiver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= SERIAL_232_RECEIVER_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(SERIAL_232_RECEIVER_ENABLE_BIT);
			}
			break;

		case SERIAL_485_DRIVER_ENABLE:
			//debug("Serial 485 Driver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= SERIAL_485_DRIVER_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(SERIAL_485_DRIVER_ENABLE_BIT);
			}
			break;

		case SERIAL_485_RECEIVER_ENABLE:
			//debug("Serial 485 Receiver Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= SERIAL_485_RECEIVER_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(SERIAL_485_RECEIVER_ENABLE_BIT);
			}
			break;

		case USB_HOST_SLEEP_ENABLE:
			//debug("Usb Host Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= USB_HOST_SLEEP_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(USB_HOST_SLEEP_ENABLE_BIT);
			}
			break;

		case USB_DEVICE_SLEEP_ENABLE:
			//debug("Usb Device Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= USB_DEVICE_SLEEP_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(USB_DEVICE_SLEEP_ENABLE_BIT);
			}
			break;

		case LAN_SLEEP_ENABLE:
			//debug("Lan Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= LAN_SLEEP_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(LAN_SLEEP_ENABLE_BIT);
			}
			break;

		case ANALOG_SLEEP_ENABLE:
			//debug("Analog Sleep Enable: %s.\n", mode == ON ? "On" : "Off");
			if (mode == ON)
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg |= ANALOG_SLEEP_ENABLE_BIT;
			}
			else
			{
#if 0 // fix_ns8100
#endif
				powerManagement.reg &= ~(ANALOG_SLEEP_ENABLE_BIT);
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
			if (powerManagement.reg & POWER_OFF_BIT) state = ON;
			break;

		case POWER_SHUTDOWN_ENABLE:
			if (powerManagement.reg & POWER_SHUTDOWN_ENABLE_BIT) state = ON;
			break;

		case ALARM_1_ENABLE:
			if (powerManagement.reg & ALARM_1_ENABLE_BIT) state = ON;
			break;

		case ALARM_2_ENABLE:
			if (powerManagement.reg & ALARM_2_ENABLE_BIT) state = ON;
			break;

		case LCD_POWER_ENABLE:
			if (powerManagement.reg & LCD_POWER_ENABLE_BIT) state = ON;
			break;

		case LCD_CONTRAST_ENABLE:
			if (powerManagement.reg & LCD_CONTRAST_ENABLE_BIT) state = ON;
			break;

		case LCD_BACKLIGHT_ENABLE:
			if (powerManagement.reg & LCD_BACKLIGHT_ENABLE_BIT) state = ON;
			break;

		case LCD_BACKLIGHT_HI_ENABLE:
			if (powerManagement.reg & LCD_BACKLIGHT_HI_ENABLE_BIT) state = ON;
			break;

		case SERIAL_232_DRIVER_ENABLE:
			if (powerManagement.reg & SERIAL_232_DRIVER_ENABLE_BIT) state = ON;
			break;

		case SERIAL_232_RECEIVER_ENABLE:
			if (powerManagement.reg & SERIAL_232_RECEIVER_ENABLE_BIT) state = ON;
			break;

		case SERIAL_485_DRIVER_ENABLE:
			if (powerManagement.reg & SERIAL_485_DRIVER_ENABLE_BIT) state = ON;
			break;

		case SERIAL_485_RECEIVER_ENABLE:
			if (powerManagement.reg & SERIAL_485_RECEIVER_ENABLE_BIT) state = ON;
			break;

		case USB_HOST_SLEEP_ENABLE:
			if (powerManagement.reg & USB_HOST_SLEEP_ENABLE_BIT) state = ON;
			break;

		case USB_DEVICE_SLEEP_ENABLE:
			if (powerManagement.reg & USB_DEVICE_SLEEP_ENABLE_BIT) state = ON;
			break;

		case LAN_SLEEP_ENABLE:
			if (powerManagement.reg & LAN_SLEEP_ENABLE_BIT) state = ON;
			break;

		case ANALOG_SLEEP_ENABLE:
			if (powerManagement.reg & ANALOG_SLEEP_ENABLE_BIT) state = ON;
			break;
	}

	return (state);
}

/****************************************
*	Function:		setMcorePwMgntDefaults
*	Purpose:
****************************************/
void setMcorePwMgntDefaults(void)
{
	uint16* powerManagementPort = (uint16*)POWER_CONTROL_ADDRESS;

	//debug("Power Defaults Set\n");
	powerManagement.bit.alarm2Enable = 				ON;
	powerManagement.bit.alarm1Enable = 				ON;
	powerManagement.bit.serial485ReceiverEnable = 	OFF;
	powerManagement.bit.serial485DriverEnable = 	OFF;
	powerManagement.bit.serial232ReceiverEnable = 	ON;
	powerManagement.bit.serial232DriverEnable = 	ON;
	powerManagement.bit.powerShutdownEnable = 		OFF;
	powerManagement.bit.powerOff = 					OFF;
	powerManagement.bit.lcdBacklightHiEnable = 		OFF;
	powerManagement.bit.lcdBacklightEnable = 		OFF;
	powerManagement.bit.lcdContrastEnable = 		OFF;
	powerManagement.bit.lcdSleepEnable = 			OFF;
	powerManagement.bit.analogSleepEnable = 		ON;
	powerManagement.bit.lanSleepEnable = 			ON;
	powerManagement.bit.usbDeviceSleepEnable = 		ON;
	powerManagement.bit.usbHostSleepEnable = 		ON;

	//powerManagement.reg = 0xCC0F;

	*powerManagementPort = powerManagement.reg;

	soft_usecWait(10 * SOFT_MSECS);
}

/****************************************
*	Function:	void PowerUnitOff(uint8)
*	Purpose:
****************************************/
void PowerUnitOff(uint8 powerOffMode)
{
	uint16* powerManagementPort = (uint16*)POWER_CONTROL_ADDRESS;

	if (powerOffMode == SHUTDOWN_UNIT)
	{
		debug("Powering unit off (shutdown)...\n");
		RTC_ENABLES.bit.periodicIntEnable = OFF;
		RTC_ENABLES.bit.powerFailIntEnable = OFF;
	}
	else
	{
		debug("Powering unit off (reboot)...\n");
	}

	// Shutdown application
	//powerManagement.bit.powerOff = ON;
	powerControl(POWER_OFF, ON);

	*powerManagementPort = powerManagement.reg;
	soft_usecWait(10 * SOFT_MSECS);

	while (1) { /* do nothing */ };
}
