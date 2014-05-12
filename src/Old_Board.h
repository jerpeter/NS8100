///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

#if 1

#ifndef _BOARD_H_
#define _BOARD_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define CONFIG_VERSION		"1.6"
#define CONFIG_DATE			"05-18-2003"

/* Uart Info */
#define CRAFT_BAUDRATE	115200 //14400 //38400
#define CRAFT_COM_PORT	0
#define RS485_BAUDRATE	2000000 //19200
#define RS485_COM_PORT	1

/* define max and min limits */
#define MAX_BUTTONS				15

/* Board level defines */
#define MAX_NACKS               25
#define MAX_ZONES               2
#define DEFAULT_ACTIVE_BUTTON   6
#define EE_ACTIVE_BUTTON        7

/* Battery Level defines */
#define BATT_MAX_VOLTS 			6.5
#define BATT_MIN_VOLTS 			4.0

#define REFERENCE_VOLTAGE       (float)3.3
#define BATT_RESOLUTION         (float)1024  // 10-bit resolution


#if 0 // ns7100
#define BATT_RESISTOR_RATIO     	((604 + 301)/301)
#define EXT_CHARGE_RESISTOR_RATIO	((3000 + 200)/200)
#else // ns8100
#define VOLTAGE_RATIO_BATT			(float)3
#define VOLTAGE_RATIO_EXT_CHARGE    (float)16.05
#endif

#if 0 // ns7100
/* Macro that returns a pointer to the Internal Memory Map */
#define mmc2114_get_immp()      ((MMC2114_IMM *)(IMM_ADDRESS))
#endif

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

#endif /* _BOARD_H_ */

#endif
