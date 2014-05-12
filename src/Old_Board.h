///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Old_Board.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:55 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Old_Board.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

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

/* System Bus Clock Info */
#define SYSCLK          	(32 * 1000000) //32000000
#define	SYSTEM_CLOCK		(SYSCLK / 1000000)		/* system bus frequency in MHz */
#define PERIOD				(1000 / SYSTEM_CLOCK)	/* system bus period in ns */

/* Flash Device Info */
#define SST39LV800						/* Flash Device */
#define SST39LV800_16BIT				/* Flash port size */
#define UPDBUG_SEC				(0)			/* dBug sector */
#define UPUSER_SEC				(1)			/* user sector */
#define PARAM_SEC				(10)		/* param sector */
#define PARAM_ADDR				(0xF8000)	/* param sector addr */

/* Memory Map Base Info */
#define IMM_ADDRESS				(0x00C00000)
#define IO_BASE_ADDRESS			(0x81200000)

/* Memory device base and size defines */
#define INT_FLASH_ADDRESS		(0x00000000)
#define INT_FLASH_SIZE			(0x00040000)
#define INT_RAM_ADDRESS			(0x00800000)
#define INT_RAM_SIZE			(0x00008000)
#define EXT_FLASH_ADDRESS		(0x80800000)
#define EXT_FLASH_SIZE			(0x00200000)
#define EXT_SRAM_ADDRESS		(0x80000000)
#define EXT_SRAM_SIZE			(0x00800000)

/* peripheral base and size defines */
#define PERIPHERAL_SIZE			(0x00040000)

#define ANALOG_ADDRESS			(IO_BASE_ADDRESS)							// 0x81200000
#define DATA_ADDRESS			(ANALOG_ADDRESS + PERIPHERAL_SIZE)			// 0x81240000
#define RTC_ADDRESS				(DATA_ADDRESS + PERIPHERAL_SIZE)			// 0x81280000
#define KEYPAD_ADDRESS			(RTC_ADDRESS + PERIPHERAL_SIZE)				// 0x812C0000
#define POWER_CONTROL_ADDRESS	(KEYPAD_ADDRESS + PERIPHERAL_SIZE)			// 0x81300000
#define USB_ADDRESS				(POWER_CONTROL_ADDRESS + PERIPHERAL_SIZE)	// 0x81340000
#define LAN_ADDRESS				(USB_ADDRESS + PERIPHERAL_SIZE)				// 0x81380000
#define LAN_ADDRESS_MEMORY		(LAN_ADDRESS + PERIPHERAL_SIZE)				// 0x813C0000

/* Memory defines */
#define ADDRESS_16M_PROBE   	(0x00800000)
#define ADDRESS_4M_PROBE    	(0x00100000)
#define MEMORY_16M          	(0x01000000)
#define MEMORY_4M           	(0x00400000)
#define MEMORY_1M           	(0x00100000)
#define DRAM_PRIME_ADDRESS1 	(0x00000000)
#define DRAM_PRIME_ADDRESS2 	(0x00000004)
#define DRAM_PRIME_ADDRESS3 	(0x00000008)
#define DRAM_PRIME_ADDRESS4 	(0x0000000C)
#define DRAM_PRIME_ADDRESS5 	(0x00000010)
#define DRAM_PRIME_ADDRESS6 	(0x00000014)
#define DRAM_PRIME_ADDRESS7 	(0x00000018)
#define DRAM_PRIME_ADDRESS8 	(0x0000001C)
#define MEM_TEST_PATTERN_1  	(0xA55A9669)
#define MEM_TEST_PATTERN_2  	(0xF0F0F0F0)
#define MAX_CODE_SIZE			(0x00040000)

/* MMC2114 INTERNAL ADDRESSES */
#define DTOA_RESULT_START			(0x000A0280)
#define DTOA_EXT_CHARGE_RESULT_1	(0x000A0280)
#define DTOA_BATT_VOLT_RESULT_1		(0x000A0282)
#define DTOA_EXT_CHARGE_RESULT_2	(0x000A0284)
#define DTOA_BATT_VOLT_RESULT_2		(0x000A0286)

/* Interrupt Info */
#define ETHERNET_VECTOR			28
#define TIMER_BENCHMARK_VECTOR	26
#define TIMER_NETWORK_VECTOR	25

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

#define REFERENCE_VOLTAGE       3.3
#define BATT_RESOLUTION         1024  // 10-bit resolution

#define BATT_RESISTOR_RATIO     	((604 + 301)/301)
#define EXT_CHARGE_RESISTOR_RATION	((3000 + 200)/200)

/* Macro that returns a pointer to the Internal Memory Map */
#define mmc2114_get_immp()      ((MMC2114_IMM *)(IMM_ADDRESS))

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

#endif /* _BOARD_H_ */
