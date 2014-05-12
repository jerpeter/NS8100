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
#include "ad_test_menu.h"
#include "rtc_test_menu.h"
#include "usb_test_menu.h"
#include "sd_mmc_test_menu.h"
#include "eeprom_test_menu.h"
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

extern void InitKeypad(void);
extern void InitSystemHardware_NS8100(void);
extern void InitInterrupts_NS8100(void);
extern void InitSoftwareSettings_NS8100(void);
extern void testSnippetsAfterInit(void);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
//void InitSystemHardware(void);
void InitInterrupts(void);
void InitSoftwareSettings(void);
void SystemEventManager(void);
void MenuEventManager(void);
void CraftManager(void);
void MessageManager(void);
void FactorySetupManager(void);
void Setup_8100_EIC_External_RTC_ISR(void);
void Setup_8100_EIC_Keypad_ISR(void);
void Setup_8100_EIC_System_ISR(void);
void Setup_8100_Soft_Timer_Tick_ISR(void);
void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM);
void Setup_8100_Usart_RS232_ISR(void);

//-----------------------------------------------------------------------------
uint8 craft_g_input_buffer[120];
BOOLEAN processCraftCmd = NO;
void craftTestMenuThruDebug(void)
{
	int count;
    int character;
    int reply=0;

	//if(usart_test_hit(&AVR32_USART1))
	if (processCraftCmd == YES)
	{
		if(Get_User_Input((uint8*)&g_input_buffer[0]) > 0)
		{
        	reply = atoi((char *)g_input_buffer);
		}

		if(reply < Menu_Items)
		{
        	((void(*)())*(Menu_Functions + reply))();
		}

    	count = 0;
    	character = Menu_String[count];
    	while(character != 0)
    	{
    		print_dbg_char(character);
    		count++;
    		character = Menu_String[count];
    	}
		Command_Prompt();
		
		processCraftCmd = NO;
	}
}

//=================================================================================================
//	Function:	UsbDeviceManager
//=================================================================================================
enum USB_STATES {
	USB_INIT_DRIVER = 0,
	USB_NOT_CONNECTED = 1,
	USB_CONNECTED_AND_PROCESSING = 2
};
extern uint8 g_sampleProcessing;
#if 1 // Test - need access in main() to determine sleep level
uint8 usbMassStorageState = USB_INIT_DRIVER;
#endif
void UsbDeviceManager(void)
{
#if 0 // Test - need access in main() to determine sleep level
	static uint8 usbMassStorageState = USB_INIT_DRIVER;
#endif
	INPUT_MSG_STRUCT mn_msg;
	
	// Check if the USB and Mass Storage Driver have never been initialized
	if (usbMassStorageState == USB_INIT_DRIVER)
	{
		debug("Init USB Mass Storage Driver...\n");

		// Init the USB and Mass Storage driver			
		usb_task_init();
		device_mass_storage_task_init();

		// Set state to ready to process
		usbMassStorageState = USB_NOT_CONNECTED;	
	}

	// Check if USB Cable is plugged in and not monitoring and not handling a trigger
	if ((Is_usb_vbus_high()) && (g_sampleProcessing != ACTIVE_STATE) && (!getSystemEventState(TRIGGER_EVENT)) && 
			g_fileProcessActiveUsbLockout == OFF)
	{
		// Check if USB and Mass Storage driver init needs to occur
		if (usbMassStorageState == USB_NOT_CONNECTED)
		{
			overlayMessage("USB STATUS", "USB CABLE WAS CONNECTED", 1 * SOFT_SECS);

			// Recall the current active menu to repaint the display
			mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
			JUMP_TO_ACTIVE_MENU();
						
			debug("USB Mass Storage Driver Re-Init\n");
			// Init the USB and Mass Storage driver
			usb_task_init();
			device_mass_storage_task_init();

			// Set state to ready to process
			usbMassStorageState = USB_CONNECTED_AND_PROCESSING;	
		}

		// Call Usb and Device Storage drivers if connected to check and handle incoming actions
		usb_task();
		device_mass_storage_task();
	}
	else // USB Cable is not plugged in
	{
		// Check if the USB was plugged in prior
		if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
		{
			if (g_sampleProcessing == ACTIVE_STATE)
			{
				overlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR MONITORING", 1000 * SOFT_MSECS);
			}
			else if (g_fileProcessActiveUsbLockout == ON)
			{
				overlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR FILE OPERATION", 1000 * SOFT_MSECS);
			}
			else
			{
				overlayMessage("USB STATUS", "USB CABLE WAS DISCONNECTED", 1000 * SOFT_MSECS);

				// Recall the current active menu to repaint the display
				mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
				JUMP_TO_ACTIVE_MENU();
			}							

			debug("USB Cable Unplugged\n");
			Usb_disable();
		}
		
		usbMassStorageState = USB_NOT_CONNECTED;
	}	
}

//=================================================================================================
//	Function:	BootLoadManager
//=================================================================================================
#define APPLICATION    (((void *)AVR32_EBI_CS1_ADDRESS) + 0x00700000)
const char default_boot_name[] = {
	"Boot.s\0"
};

uint8 quickBootEntryJump = NO;
void BootLoadManager(void)
{
	int16 usart_status;
	int craft_char;
	char textBuffer[50];
	static void (*func)(void);
	func = (void(*)())APPLICATION;
	FL_FILE* file;
	uint32 i;

	usart_status = USART_RX_EMPTY;
	i = 0;

	usart_status = usart_read_char(DBG_USART, &craft_char);

#if 0
	if (craft_char == CTRL_B)
#else
	if ((craft_char == CTRL_B) || (quickBootEntryJump == YES))
#endif
	{
#if 1
		if (quickBootEntryJump == YES)
			overlayMessage("BOOTLOADER", "HIDDEN ENTRY...", 2 * SOFT_SECS);
		else
#endif
		overlayMessage("BOOTLOADER", "FOUND CTRL_B...", 2 * SOFT_SECS);

		sprintf(textBuffer,"C:\\System\\%s", default_boot_name);
		file = fl_fopen(textBuffer, "r");

		while (file == NULL)
		{
			//WriteLCD_smText( 0, 64, (unsigned char *)"Connect USB..", NORMAL_LCD);
			overlayMessage("BOOTLOADER", "BOOT FILE NOT FOUND. CONNECT THE USB CABLE", 0);

			usb_task_init();
			device_mass_storage_task_init();

			while(!Is_usb_vbus_high()) {}

			overlayMessage("BOOTLOADER", "WRITE BOOT.S TO SYSTEM DIR (REMOVE CABLE WHEN DONE)", 0);

			usb_task_init();
			device_mass_storage_task_init();

			while (Is_usb_vbus_high())
			{
				usb_task();
				device_mass_storage_task();
			}
			
			soft_usecWait(250 * SOFT_MSECS);
			
			file = fl_fopen(textBuffer, "r");
		}

		// Display initializing message
		debug("Starting Boot..\n");

		overlayMessage("BOOTLOADER", "STARTING BOOTLOADER...", 2 * SOFT_SECS);

		clearLcdDisplay();

		if (unpack_srec(file) == -1)
		{
			debugErr("SREC unpack unsuccessful!\n");
		}

		fl_fclose(file);

		Disable_global_interrupt();

		//Jump to boot application code
		func();
	}
	
	initCraftInterruptBuffers();
	Setup_8100_Usart_RS232_ISR();
}

//=================================================================================================
//	Function Break
//=================================================================================================
void SleepManager(void)
{
	// Check if no System Events and LCD is off and Modem is not transferring
	if ((g_systemEventFlags.wrd == 0x0000) && (getPowerControlState(LCD_POWER_ENABLE) == OFF) &&
	(g_modemStatus.xferState == NOP_CMD))
	{
		// Sleepy time
#if 0 // Normal
		SLEEP(AVR32_PM_SMODE_IDLE);
#else // Test
		// Check if USB is connected
		if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
		{
			// Can't operate the USB is the sleep mode is deeper than IDLE (due to HSB)
			SLEEP(AVR32_PM_SMODE_IDLE);
		}
		else // USB is not connected and a deeper sleep mode can be used
		{
			// Disable rs232 driver and receiver (Active low controls)
			gpio_set_gpio_pin(AVR32_PIN_PB08);
			gpio_set_gpio_pin(AVR32_PIN_PB09);

			SLEEP(AVR32_PM_SMODE_STANDBY);

			// Enable rs232 driver and receiver (Active low controls)
			gpio_clr_gpio_pin(AVR32_PIN_PB08);
			gpio_clr_gpio_pin(AVR32_PIN_PB09);
		}
#endif
	}
}

//=================================================================================================
//	Function Break
//=================================================================================================
void DisplayVersionToCraft(void)
{
	int majorVer, minorVer;
	char buildVer;
	sscanf(&g_buildVersion[0], "%d.%d.%c", &majorVer, &minorVer, &buildVer);

	debug("--- System Init complete (Version %d.%d.%c) ---\n", majorVer, minorVer, buildVer);
}

//=================================================================================================
//	Function:	Main
//	Purpose:	Application starting point
//=================================================================================================
int main(void)
{
	// Test code
	//testSnippetsBeforeInit();

    InitSystemHardware_NS8100();
	InitInterrupts_NS8100();
	InitSoftwareSettings_NS8100();
	BootLoadManager();
	
	DisplayVersionToCraft();
	
#if 0 // External sampling source
extern void startExternalRTCClock(uint16 sampleRate);
	g_updateCounter = 1;
	startExternalRTCClock(TEST_SAMPLE_RATE);

	while (1==1)
	{
		g_updateCounter++;
		
		if (g_updateCounter % TEST_SAMPLE_RATE == 0)
		{
			debug("Tick tock (%d, %d)\n", TEST_SAMPLE_RATE, g_updateCounter / TEST_SAMPLE_RATE);
		}
		
		while (!usart_tx_empty(DBG_USART)) {;}
		SLEEP(AVR32_PM_SMODE_IDLE);
	}
#endif

	// Test code
	testSnippetsAfterInit();

#if 1 // Normal
 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
		g_execCycles++;

		// Debug Test routines
		//craftTestMenuThruDebug();

		// Test code
		//testSnippetsExecLoop();

#if 1 // Normal operational cycle
		// Handle system events
	    SystemEventManager();

		// Handle menu events
	    MenuEventManager();

		// Handle craft processing
		CraftManager();

		// Handle messages to be processed
		MessageManager();

		// Handle USB device
		UsbDeviceManager();
		
		// Handle processing the factory setup
		FactorySetupManager();
#endif

		// Check if able to go to sleep
		SleepManager();
	}    
	// End of NS8100 Main
#endif

	// End of the world
	return (0);
}
