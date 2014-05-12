///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: PowerManagement.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/PowerManagement.h,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

#ifndef _POWER_MANAGEMENT_H_
#define _POWER_MANAGEMENT_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
typedef enum
{
	ALARM_2_ENABLE = 0,
	ALARM_1_ENABLE,
	SERIAL_485_RECEIVER_ENABLE,
	SERIAL_485_DRIVER_ENABLE,
	SERIAL_232_RECEIVER_ENABLE,
	SERIAL_232_DRIVER_ENABLE,
	POWER_SHUTDOWN_ENABLE,
	POWER_OFF,
	LCD_BACKLIGHT_HI_ENABLE,
	LCD_BACKLIGHT_ENABLE,
	LCD_CONTRAST_ENABLE,
	LCD_POWER_ENABLE,
	ANALOG_SLEEP_ENABLE,
	LAN_SLEEP_ENABLE,
	USB_DEVICE_SLEEP_ENABLE,
	USB_HOST_SLEEP_ENABLE
} POWER_MGMT_OPTIONS;

#define ALARM_2_ENABLE_BIT				0x8000	
#define ALARM_1_ENABLE_BIT				0x4000
#define SERIAL_485_RECEIVER_ENABLE_BIT	0x2000
#define SERIAL_485_DRIVER_ENABLE_BIT	0x1000
#define SERIAL_232_RECEIVER_ENABLE_BIT	0x0800
#define SERIAL_232_DRIVER_ENABLE_BIT	0x0400
#define POWER_OFF_BIT					0x0200
#define POWER_SHUTDOWN_ENABLE_BIT		0x0100
#define LCD_BACKLIGHT_HI_ENABLE_BIT		0x0080
#define LCD_BACKLIGHT_ENABLE_BIT		0x0040
#define LCD_CONTRAST_ENABLE_BIT			0x0020
#define LCD_POWER_ENABLE_BIT			0x0010
#define ANALOG_SLEEP_ENABLE_BIT			0x0008
#define LAN_SLEEP_ENABLE_BIT			0x0004
#define USB_DEVICE_SLEEP_ENABLE_BIT		0x0002
#define USB_HOST_SLEEP_ENABLE_BIT		0x0001

enum {
	SHUTDOWN_UNIT = 1,
	RESET_UNIT
};

typedef union
{
	struct
	{
		bitfield alarm2Enable:				1;
		bitfield alarm1Enable:				1;
		bitfield serial485ReceiverEnable:	1;
		bitfield serial485DriverEnable:		1;
		bitfield serial232ReceiverEnable:	1;
		bitfield serial232DriverEnable:		1;
		bitfield powerShutdownEnable:		1;
		bitfield powerOff:      			1;

		bitfield lcdBacklightHiEnable:		1;
		bitfield lcdBacklightEnable:		1;
		bitfield lcdContrastEnable:			1;
		bitfield lcdSleepEnable:			1;
		bitfield analogSleepEnable:			1;
		bitfield lanSleepEnable:			1;
		bitfield usbDeviceSleepEnable:		1;
		bitfield usbHostSleepEnable:		1;
	} bit;

	uint16 reg;
} POWER_MANAGEMENT_STRUCT;
                             
///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void powerControl(POWER_MGMT_OPTIONS option, BOOL mode);
BOOL getPowerControlState(POWER_MGMT_OPTIONS option);
void setMcorePwMgntDefaults(void);
void PowerUnitOff(uint8 powerOffMode);

#endif //_POWER_MANAGEMENT_H_
