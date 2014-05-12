///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "sdramc.h"
#include "intc.h"
#include "usart.h"
#include "print_funcs.h"
#include "craft.h"
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
#include "Old_Board.h"
#include "Common.h"
#include "Display.h"
#include "Menu.h"
#include "Uart.h"
#include "spi.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "SysEvents.h"
#include "Record.h"
#include "InitDataBuffers.h"
#include "PowerManagement.h"
#include "SoftTimer.h"
#include "Keypad.h"
#include "RealTimeClock.h"
#include "RemoteHandler.h"
#include "TextTypes.h"
#include "RemoteCommon.h"
#include "twi.h"
#include "M23018.h"
#include "sd_mmc_spi.h"
#include "FAT32_Disk.h"
#include "FAT32_Access.h"
#include "adc.h"
#include "usb_task.h"
#include "device_mass_storage_task.h"
#include "usb_drv.h"
#include "FAT32_FileLib.h"
#include "srec.h"
#include "flashc.h"

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

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

//=================================================================================================
//	Function:	testSnippetsExecLoop
//=================================================================================================
void testSnippetsBeforeInit(void)
{
	#if 0 // Test (Enable serial and put processor in deep stop)
	gpio_clr_gpio_pin(AVR32_PIN_PB08);
	gpio_clr_gpio_pin(AVR32_PIN_PB09);

	// Setup debug serial port
	usart_options_t usart_1_rs232_options =
	{
		.baudrate = 115200,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};

	// Initialize it in RS232 mode.
	usart_init_rs232(&AVR32_USART1, &usart_1_rs232_options, FOSC0);

	// Init signals for ready to send and terminal ready
	SET_RTS; SET_DTR;

	debug("--- System Deep Stop ---\n");

	SLEEP(AVR32_PM_SMODE_DEEP_STOP);

	while (1) {;}
	#endif
}

//=================================================================================================
//	Function:	testSnippetsAfterInit
//=================================================================================================
void testSnippetsAfterInit(void)
{

	#if 0
	CMD_BUFFER_STRUCT inCmd;
	inCmd.msg[MESSAGE_HEADER_SIMPLE_LENGTH] = 0;
	handleVML(&inCmd);
	#endif

	#if 0 // Craft Test
	Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
	Menu_Functions = (unsigned long *)Main_Menu_Functions;
	Menu_String = (unsigned char *)&Main_Menu_Text;
	#endif

	#if 0 // Clear Internal RAM static variables
	uint32 i = 0x28;
	while (i)
	{
		*((uint8*)i--) = 0;
	}
	#endif

	#if 0 // Test (Effective CS Low for SDMMC)
	while (1)
	{
		debug("SPI1 SDMMC CS Active (0)\n");
		spi_selectChip(&AVR32_SPI1, 2);
		spi_write(&AVR32_SPI1, 0x0000);
		soft_usecWait(5 * SOFT_SECS);


		debug("SPI1 SDMMC CS Inactive (1)\n");
		spi_unselectChip(&AVR32_SPI1, 2);
		soft_usecWait(5 * SOFT_SECS);
	}
	#endif

	#if 0 // Test (SDMMC Reset to enable standby power)
	uint8 r1;
	uint32 retry = 0;
	uint32 timedAccess = 0;
	FL_FILE* monitorLogFile;
	MONITOR_LOG_ENTRY_STRUCT monitorLogEntry;
	int32 bytesRead = 0;

	while (1)
	{
		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);
		
		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------
		
		//---Init and idle-------------------------------------------------------------------
		debug("SDMMC Init and idle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------
		
		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------

		//---Init and FAT32 Cycle-------------------------------------------------------------------
		debug("SDMMC Init and FAT32 Init cycle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		timedAccess = g_rtcSoftTimerTickCount;
		
		while (g_rtcSoftTimerTickCount < (timedAccess + 10))
		{
			FAT32_InitDrive();
			if (FAT32_InitFAT() == FALSE)
			{
				debugErr("FAT32 Initialization failed!\n\r");
			}
		}
		//----------------------------------------------------------------------

		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------

		//---Init and Read cycle-------------------------------------------------------------------
		debug("SDMMC Init and Read cycle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}

		timedAccess = g_rtcSoftTimerTickCount;
		
		while (g_rtcSoftTimerTickCount < (timedAccess + 10))
		{
			monitorLogFile = fl_fopen("C:\\Logs\\TestLogRead.ns8", "r");
			if (monitorLogFile == NULL) { debugErr("Test Read file not found!\n"); }
			bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
			while (bytesRead > 0) { bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT)); }
			fl_fclose(monitorLogFile);
		}
		//----------------------------------------------------------------------

		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------

		//---Init and Write cycle-------------------------------------------------------------------
		debug("SDMMC Init and Write cycle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}

		timedAccess = g_rtcSoftTimerTickCount;
		
		while (g_rtcSoftTimerTickCount < (timedAccess + 10))
		{
			monitorLogFile = fl_fopen("C:\\Logs\\TestLogWrite.ns8", "a+");
			if (monitorLogFile == NULL) { debugErr("Test Write file not opened!\n"); }
			for (retry = 0; retry < 250; retry++)
			{
				fl_fwrite((uint8*)&(__monitorLogTbl[__monitorLogTblIndex]), sizeof(MONITOR_LOG_ENTRY_STRUCT), 1, monitorLogFile);
			}
			fl_fclose(monitorLogFile);
		}
		//----------------------------------------------------------------------
	}
	#endif

	#if 0 // Test (Ineffective CS Low for SDMMC)
	gpio_enable_gpio_pin(AVR32_PIN_PA14);
	gpio_enable_gpio_pin(AVR32_PIN_PA18);
	gpio_enable_gpio_pin(AVR32_PIN_PA19);
	gpio_enable_gpio_pin(AVR32_PIN_PA20);

	while (1)
	{
		debug("SPI1 CS's 0\n");
		//gpio_clr_gpio_pin(AVR32_PIN_PA14);
		//gpio_clr_gpio_pin(AVR32_PIN_PA18);
		gpio_clr_gpio_pin(AVR32_PIN_PA19);
		//gpio_clr_gpio_pin(AVR32_PIN_PA20);

		spi_selectChip(&AVR32_SPI1, 2);
		// Small delay before the RTC device is accessible
		soft_usecWait(500);
		unsigned int i;
		for (i = 0; i < 10000; i++)
		spi_write(&AVR32_SPI1, 0x0000);
		spi_unselectChip(&AVR32_SPI1, 2);

		__monitorLogTblKey = 0;
		initMonitorLog();

		soft_usecWait(3 * SOFT_SECS);

		debug("SPI1 CS's 1\n");
		//gpio_set_gpio_pin(AVR32_PIN_PB14);
		gpio_set_gpio_pin(AVR32_PIN_PB18);
		//gpio_set_gpio_pin(AVR32_PIN_PB19);
		//gpio_set_gpio_pin(AVR32_PIN_PB20);

		soft_usecWait(3 * SOFT_SECS);
	}

	#endif

	#if 0 // Test (Timer mode)
	EnableRtcAlarm(0, 0, 0, 0);

	static RTC_MEM_MAP_STRUCT rtcMap;
	static uint8 clearAlarmFlag = 0x03; //0xEF; // Logic 0 on the bit will clear Alarm flag, bit 4
	static uint32 counter = 0;

	rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);
	
	if (rtcMap.control_2 & 0x10)
	{
		debug("RTC Alarm Flag indicates alarm condition raised. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
		
		rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
		debug("RTC Alarm Flag being cleared\n");

		rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);

		if (rtcMap.control_2 & 0x10)
		{
			debugWarn("RTC Alarm flag was not cleared successfully! (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
		}
		else
		{
			debug("RTC Alarm Flag cleared. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
		}
	}
	else
	{
		rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
		debug("RTC Alarm Flag does not show an alarm condition. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
	}
	#endif

	#if 0 // Test (LCD off and Proc stop)
	debug("\n--- System Init Complete ---\n");
	soft_usecWait(10 * SOFT_SECS);
	displayTimerCallBack();
	lcdPwTimerCallBack();
	soft_usecWait(10 * SOFT_SECS);
	debug("--- System Deep Stop ---\n");

	SLEEP(AVR32_PM_SMODE_DEEP_STOP);

	while (1) {;}
	#endif

	#if 0 // Test (Keypad logic test)
	uint8 keypadState;
	while (1 == 1)
	{
		keypadState = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
		if (keypadState & GREEN_LED_PIN)
		debug("Green LED Active\n");
		if (keypadState & RED_LED_PIN)
		debug("Red LED Active\n");

		debug("Turning on the Green LED only\n");
		keypadState |= GREEN_LED_PIN;
		keypadState &= ~RED_LED_PIN;
		write_mcp23018(IO_ADDRESS_KPD, GPIOA, keypadState);

		soft_usecWait(3 * SOFT_SECS);

		keypadState = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
		if (keypadState & GREEN_LED_PIN)
		debug("Green LED Active\n");
		if (keypadState & RED_LED_PIN)
		debug("Red LED Active\n");

		debug("Turning on the Red LED only\n");
		keypadState &= ~GREEN_LED_PIN;
		keypadState |= RED_LED_PIN;
		write_mcp23018(IO_ADDRESS_KPD, GPIOA, keypadState);

		soft_usecWait(3 * SOFT_SECS);

		keypadState = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
		if (keypadState & GREEN_LED_PIN)
		debug("Green LED Active\n");
		if (keypadState & RED_LED_PIN)
		debug("Red LED Active\n");

		debug("Turning off both LEDs\n");
		keypadState &= ~GREEN_LED_PIN;
		keypadState &= ~RED_LED_PIN;
		write_mcp23018(IO_ADDRESS_KPD, GPIOA, keypadState);

		soft_usecWait(3 * SOFT_SECS);
	}
	#endif
}

//=================================================================================================
//	Function:	testSnippetsExecLoop
//=================================================================================================
void testSnippetsExecLoop(void)
{
	#if 0 // Test (Timer mode)
	if (counter)
	{
		counter--;
		
		if (counter == 0)
		{
			rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
			debug("RTC Alarm Flag being cleared\n");

			rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);

			if (rtcMap.control_2 & 0x10)
			{
				debugWarn("RTC Alarm flag was not cleared successfully! (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
			}
			else
			{
				debug("RTC Alarm Flag cleared. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
			}
		}
	}
	else
	{
		rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);
		
		if (rtcMap.control_2 & 0x10)
		{
			debug("RTC Alarm Flag indicates alarm condition raised. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
			counter = 1000000;
		}
	}
	
	#endif

	#if 0 // Test (Display temperature change readings)
	static uint16 s_tempReading = 0;
	
	// 0x49 to 0x4c
	if (abs((int)(g_currentTempReading - s_tempReading)) > 4)
	{
		debug("Temp update on change, Old: 0x%x, New: 0x%x\n", s_tempReading, g_currentTempReading);
		
		s_tempReading = g_currentTempReading;
	}
	#endif

}

