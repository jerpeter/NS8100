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
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
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

extern void InitKeypad(void);
extern void InitSystemHardware_NS8100(void);
extern void InitInterrupts_NS8100(void);
extern void InitSoftwareSettings_NS8100(void);
extern void TestSnippetsAfterInit(void);
extern void Setup_8100_Usart_RS232_ISR(void);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitInterrupts(void);
void InitSoftwareSettings(void);
void SystemEventManager(void);
void MenuEventManager(void);
void CraftManager(void);
void MessageManager(void);
void FactorySetupManager(void);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void SystemEventManager(void)
{
	//debug("System Event Manager called\n");

	if (getSystemEventState(TRIGGER_EVENT))
	{
		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			debug("Trigger Event (Wave)\n");
			MoveWaveformEventToFlash();
		}
		else if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			debug("Trigger Event (Combo)\n");
			MoveComboWaveformEventToFile();
		}
	}

	if (getSystemEventState(MANUEL_CAL_EVENT))
	{
		debug("Cal Pulse Event\n");
		MoveManuelCalToFlash();
	}

	if ((getSystemEventState(KEYPAD_EVENT)) ||
	(g_kpadCheckForKeyFlag && (g_kpadLookForKeyTickCount < g_keypadTimerTicks)))
	{
		if (getSystemEventState(KEYPAD_EVENT))
		{
			debug("Keypad Event\n");
			clearSystemEventFlag(KEYPAD_EVENT);

			KeypadProcessing(KEY_SOURCE_IRQ);
		}
		else
		{
			KeypadProcessing(KEY_SOURCE_TIMER);
		}

	}

	if (getSystemEventState(POWER_OFF_EVENT))
	{
		debug("Power Off Event\n");
		clearSystemEventFlag(POWER_OFF_EVENT);

		HandleUserPowerOffDuringTimerMode();
	}

	if (getSystemEventState(CYCLIC_EVENT))
	{
		debug("Cyclic Event\n");
		clearSystemEventFlag(CYCLIC_EVENT);

		#if 1 // Test (ISR/Exec Cycles)
		//sprintf((char*)&g_spareBuffer[0], "%d (%s) %d", (int)g_execCycles, ((g_channelSyncError == YES) ? "YES" : "NO"), (int)(g_sampleCount / 4));
		//OverlayMessage("EXEC CYCLES", (char*)&g_spareBuffer[0], 500 * SOFT_MSECS);
		debugRaw("ISR Ticks/sec: %d (E:%s), Exec: %d\n", (g_sampleCountHold / 4), ((g_channelSyncError == YES) ? "YES" : "NO"), g_execCycles);
		g_sampleCountHold = 0;
		g_execCycles = 0;
		g_channelSyncError = NO;
		#endif
		
		#if 0 // Test (Bargraph buffer)
		uint32 bgDataBufferSize = (g_bargraphDataEndPtr - g_bargraphDataStartPtr);
		float bgUsed;
		float bgLocation;
		
		if (((g_triggerRecord.op_mode == BARGRAPH_MODE) || (g_triggerRecord.op_mode == COMBO_MODE)) && (g_sampleProcessing == ACTIVE_STATE))
		{
			if (g_bargraphDataWritePtr >= g_bargraphDataReadPtr)
			{
				bgUsed = (((float)100 * (float)(g_bargraphDataWritePtr - g_bargraphDataReadPtr)) / (float)bgDataBufferSize);
			}
			else
			{
				bgUsed = (((float)100 * (float)(bgDataBufferSize + g_bargraphDataWritePtr - g_bargraphDataReadPtr)) / (float)bgDataBufferSize);
			}

			bgLocation = (((float)100 * (float)(g_bargraphDataWritePtr - g_bargraphDataStartPtr)) / (float)bgDataBufferSize);

			debugRaw("Bargraph Data Buffer Used: %3.2f%%, Free: %3.2f%%, Location: %3.2f%%\n", bgUsed, (float)(100 - bgUsed), bgLocation);
		}
		#endif

		ProcessTimerEvents();
	}

	if (getSystemEventState(MIDNIGHT_EVENT))
	{
		debug("Midnight Event\n");
		clearSystemEventFlag(MIDNIGHT_EVENT);

		HandleMidnightEvent();
	}

	if (getSystemEventState(UPDATE_TIME_EVENT))
	{
		clearSystemEventFlag(UPDATE_TIME_EVENT);
		UpdateCurrentTime();
	}

	if (getSystemEventState(BARGRAPH_EVENT))
	{
		clearSystemEventFlag(BARGRAPH_EVENT);

		if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			CalculateBargraphData();
		}
		else if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			CalculateComboData();
		}
	}

	#if 0 // Test (Attempt to process buffered waveform data - Throw away at some point)
	if (getSystemEventState())
	{
		clearSystemEventFlag();

		extern void processAndMoveWaveformData_ISR_Inline(void);
		processAndMoveWaveformData_ISR_Inline();
	}
	#endif

	if (getSystemEventState(WARNING1_EVENT))
	{
		debug("Warning Event 1\n");
		clearSystemEventFlag(WARNING1_EVENT);

		PowerControl(ALARM_1_ENABLE, ON);

		// Assign soft timer to turn the Alarm 1 signal off
		AssignSoftTimer(ALARM_ONE_OUTPUT_TIMER_NUM, (uint32)(g_helpRecord.alarmOneTime * 2), AlarmOneOutputTimerCallback);
	}

	if (getSystemEventState(WARNING2_EVENT))
	{
		debug("Warning Event 2\n");
		clearSystemEventFlag(WARNING2_EVENT);

		PowerControl(ALARM_2_ENABLE, ON);

		// Assign soft timer to turn the Alarm 2 signal off
		AssignSoftTimer(ALARM_TWO_OUTPUT_TIMER_NUM, (uint32)(g_helpRecord.alarmTwoTime * 2), AlarmTwoOutputTimerCallback);
	}

	if (getSystemEventState(UPDATE_OFFSET_EVENT))
	{
		UpdateChannelOffsetsForTempChange();
	}

	if (getSystemEventState(AUTO_DIALOUT_EVENT))
	{
		clearSystemEventFlag(AUTO_DIALOUT_EVENT);

		StartAutoDialoutProcess();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MenuEventManager(void)
{
	//debug("Menu Event Manager called\n");

	INPUT_MSG_STRUCT mn_msg;

	if (getMenuEventState(RESULTS_MENU_EVENT))
	{
		clearMenuEventFlag(RESULTS_MENU_EVENT);

		if (g_triggerRecord.op_mode == MANUAL_CAL_MODE)
		{
			SETUP_RESULTS_MENU_MANUEL_CAL_MSG(RESULTS_MENU);
		}
		else
		{
			SETUP_RESULTS_MENU_MONITORING_MSG(RESULTS_MENU);
		}
		JUMP_TO_ACTIVE_MENU();
	}

	if (getTimerEventState(SOFT_TIMER_CHECK_EVENT))
	{
		clearTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

		// Handle and process software timers
		CheckSoftTimers();
	}

	if (getTimerEventState(TIMER_MODE_TIMER_EVENT))
	{
		clearTimerEventFlag(TIMER_MODE_TIMER_EVENT);

		debug("Timer mode: Start monitoring...\n");
		SETUP_MENU_WITH_DATA_MSG(MONITOR_MENU, g_triggerRecord.op_mode);
		JUMP_TO_ACTIVE_MENU();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CraftManager(void)
{
	//debug("Craft Manager called\n");

	// Check if there is an modem present and if the connection state is connected
	if ((MODEM_CONNECTED == READ_DSR) && (CONNECTED == g_modemStatus.connectionState))
	{
		// Check if the connection has been lost
		if (NO_CONNECTION == READ_DCD)
		{
			// Check if a ring indicator has been received
			if ((g_modemStatus.ringIndicator != READ_RI) && (READ_RI == RING))
			{
				// Check if nine rings have been received
				if (g_modemStatus.numberOfRings++ > 9)
				{
					// Reset flag
					g_modemStatus.numberOfRings = 0;

					// Assign timer to process modem init
					AssignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_QUICK_DELAY, ModemDelayTimerCallback);
				}
			}

			// Assign ring indicator to the status of the ring indicator line
			g_modemStatus.ringIndicator = (uint8)READ_RI;

			// Relock the system
			g_modemStatus.systemIsLockedFlag = YES;

			if (CONNECTED == g_modemStatus.firstConnection)
			{
				g_modemStatus.firstConnection = NOP_CMD;
				AssignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_QUICK_DELAY, ModemDelayTimerCallback);
			}
		}
		else
		{
			g_modemStatus.firstConnection = CONNECTED;
		}
	}
	else
	{
		if ((YES == g_modemSetupRecord.modemStatus) && (CONNECTED == g_modemStatus.connectionState))
		{
			g_modemStatus.connectionState = NOP_CMD;
			g_modemStatus.systemIsLockedFlag = YES;
			AssignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_DELAY, ModemDelayTimerCallback);
		}
	}

	// Check if the Auto Dialout state is not idle
	if (g_autoDialoutState != AUTO_DIAL_IDLE)
	{
		// Run the Auto Dialout State machine
		AutoDialoutStateMachine();
	}

	// Are we transfering data, if so process the correct command.
	if (NOP_CMD != g_modemStatus.xferState)
	{
		if (DEMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDEMData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.autoPrint = g_modemStatus.xferPrintState;
			}
		}
		else if (DSMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDSMData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.autoPrint = g_modemStatus.xferPrintState;
			}
		}
		else if (DQMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDQMData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.autoPrint = g_modemStatus.xferPrintState;
			}
		}
		else if (VMLx_CMD == g_modemStatus.xferState)
		{
			sendVMLData();

			if (NOP_CMD == g_modemStatus.xferState)
			{
				g_helpRecord.autoPrint = g_modemStatus.xferPrintState;
			}
		}
	}

	// Data from the craft port is independent of the modem.
	if (YES == g_modemStatus.craftPortRcvFlag)
	{
		g_modemStatus.craftPortRcvFlag = NO;
		ProcessCraftData();
	}

	// Did we raise the craft data port flag, if so process the incomming data.
	if (getSystemEventState(CRAFT_PORT_EVENT))
	{
		clearSystemEventFlag(CRAFT_PORT_EVENT);
		RemoteCmdMessageProcessing();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MessageManager(void)
{
	INPUT_MSG_STRUCT input_msg;

	// Check if there is a message in the queue
	if (CheckInputMsg(&input_msg) != INPUT_BUFFER_EMPTY)
	{
		debug("Processing message...\n");

		// Process message
		ProcessInputMsg(input_msg);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void FactorySetupManager(void)
{
	//debug("Factory Setup Manager called\n");

	INPUT_MSG_STRUCT mn_msg;

	if (g_factorySetupSequence == ENTER_FACTORY_SETUP)
	{
		g_factorySetupSequence = PROCESS_FACTORY_SETUP;

		KeypressEventMgr();
		MessageBox(getLangText(STATUS_TEXT), getLangText(YOU_HAVE_ENTERED_THE_FACTORY_SETUP_TEXT), MB_OK);

		SETUP_MENU_MSG(DATE_TIME_MENU);
		JUMP_TO_ACTIVE_MENU();
	}
	#if 0 // Added to work around On Key IRQ problem
	else if (g_factorySetupRecord.invalid && g_factorySetupSequence != PROCESS_FACTORY_SETUP)
	{
		g_factorySetupSequence = PROCESS_FACTORY_SETUP;

		KeypressEventMgr();
		MessageBox(getLangText(STATUS_TEXT), getLangText(YOU_HAVE_ENTERED_THE_FACTORY_SETUP_TEXT), MB_OK);

		SETUP_MENU_MSG(DATE_TIME_MENU);
		JUMP_TO_ACTIVE_MENU();
	}
	#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void HandleSystemEvents(void)
{
	if (g_systemEventFlags.wrd)
	SystemEventManager();

	if (g_menuEventFlags.wrd)
	MenuEventManager();

	MessageManager();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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
			OverlayMessage("USB STATUS", "USB CABLE WAS CONNECTED", 1 * SOFT_SECS);

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
				OverlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR MONITORING", 1000 * SOFT_MSECS);
			}
			else if (g_fileProcessActiveUsbLockout == ON)
			{
				OverlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR FILE OPERATION", 1000 * SOFT_MSECS);
			}
			else
			{
				OverlayMessage("USB STATUS", "USB CABLE WAS DISCONNECTED", 1000 * SOFT_MSECS);

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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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
			OverlayMessage("BOOTLOADER", "HIDDEN ENTRY...", 2 * SOFT_SECS);
		else
#endif
		OverlayMessage("BOOTLOADER", "FOUND CTRL_B...", 2 * SOFT_SECS);

		sprintf(textBuffer,"C:\\System\\%s", default_boot_name);
		file = fl_fopen(textBuffer, "r");

		while (file == NULL)
		{
			//WriteLCD_smText( 0, 64, (unsigned char *)"Connect USB..", NORMAL_LCD);
			OverlayMessage("BOOTLOADER", "BOOT FILE NOT FOUND. CONNECT THE USB CABLE", 0);

			usb_task_init();
			device_mass_storage_task_init();

			while(!Is_usb_vbus_high()) {}

			OverlayMessage("BOOTLOADER", "WRITE BOOT.S TO SYSTEM DIR (REMOVE CABLE WHEN DONE)", 0);

			usb_task_init();
			device_mass_storage_task_init();

			while (Is_usb_vbus_high())
			{
				usb_task();
				device_mass_storage_task();
			}
			
			SoftUsecWait(250 * SOFT_MSECS);
			
			file = fl_fopen(textBuffer, "r");
		}

		// Display initializing message
		debug("Starting Boot..\n");

		OverlayMessage("BOOTLOADER", "STARTING BOOTLOADER...", 2 * SOFT_SECS);

		ClearLcdDisplay();

		if (Unpack_srec(file) == -1)
		{
			debugErr("SREC unpack unsuccessful!\n");
		}

		fl_fclose(file);

		Disable_global_interrupt();

		//Jump to boot application code
		func();
	}
	
	InitCraftInterruptBuffers();
	Setup_8100_Usart_RS232_ISR();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
inline void SetupPowerSavingsBeforeSleeping(void)
{
#if 1
	// Only disable for Min and Most since None and Max are either permanently on or off
	if ((g_helpRecord.powerSavingsLevel == POWER_SAVINGS_MINIMUM) || (g_helpRecord.powerSavingsLevel == POWER_SAVINGS_MOST))
	{
		// Disable rs232 driver and receiver (Active low control)
		PowerControl(SERIAL_232_DRIVER_ENABLE, OFF);
		PowerControl(SERIAL_232_RECEIVER_ENABLE, OFF);
	}
#endif

#if 1
	// Enable pull ups on the data lines
	AVR32_GPIO.port[2].puers = 0xFC00; // 1111 1100 0000 0000
	AVR32_GPIO.port[3].puers = 0x03FF; // 0000 0011 1111 1111
#endif

	g_powerSavingsForSleepEnabled = YES;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
inline void RevertPowerSavingsAfterSleeping(void)
{
#if 1
	// Disable pull ups on the data lines
	AVR32_GPIO.port[2].puerc = 0xFC00; // 1111 1100 0000 0000
	AVR32_GPIO.port[3].puerc = 0x03FF; // 0000 0011 1111 1111
#endif

#if 1
	// Only enable for Min and Most since None and Max are either permanently on or off
	if ((g_helpRecord.powerSavingsLevel == POWER_SAVINGS_MINIMUM) || (g_helpRecord.powerSavingsLevel == POWER_SAVINGS_MOST))
	{
		// Enable rs232 driver and receiver (Active low control)
		PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
		PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
	}
#endif

	g_powerSavingsForSleepEnabled = NO;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void SleepManager(void)
{
	// Check if no System Events and LCD is off and Modem is not transferring
	if ((g_systemEventFlags.wrd == 0x0000) && (GetPowerControlState(LCD_POWER_ENABLE) == OFF) &&
	(g_modemStatus.xferState == NOP_CMD))
	{
#if 0 // Test
		debug("Going to lunch and I'll be gone forever...\n");
		StopExternalRtcClock();
extern void rtc_disable_interrupt(volatile avr32_rtc_t *rtc);
extern void rtc_clear_interrupt(volatile avr32_rtc_t *rtc);
		rtc_disable_interrupt(&AVR32_RTC);
		rtc_clear_interrupt(&AVR32_RTC);
		//while (1) {	; }
#endif

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
			SetupPowerSavingsBeforeSleeping();

#if 0 // Normal
			if (g_sleepModeState == AVR32_PM_SMODE_STANDBY) { SLEEP(AVR32_PM_SMODE_STANDBY); }
			else if (g_sleepModeState == AVR32_PM_SMODE_FROZEN) { SLEEP(AVR32_PM_SMODE_FROZEN); }
			else if (g_sleepModeState == AVR32_PM_SMODE_IDLE) { SLEEP(AVR32_PM_SMODE_IDLE); }
#else // Test
			SLEEP(AVR32_PM_SMODE_STOP);
#endif

			// Check if needing to revert the power savings (if monitoring then the ISR will handle this operation)
			if (g_powerSavingsForSleepEnabled == YES)
			{
				RevertPowerSavingsAfterSleeping();
			}
		}
#endif
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayVersionToCraft(void)
{
	int majorVer, minorVer;
	char buildVer;
	sscanf(&g_buildVersion[0], "%d.%d.%c", &majorVer, &minorVer, &buildVer);

	debug("--- System Init complete (Version %d.%d.%c) ---\n", majorVer, minorVer, buildVer);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TestExternalSamplingSource(void)
{
extern void StartExternalRtcClock(uint16 sampleRate);
	g_updateCounter = 1;
	uint16 sampleRate = 1024;

	StartExternalRtcClock(sampleRate);

	while (1==1)
	{
		g_updateCounter++;
	
		if (g_updateCounter % sampleRate == 0)
		{
			debug("Tick tock (%d, %d)\n", sampleRate, g_updateCounter / sampleRate);
		}
	
		while (!usart_tx_empty(DBG_USART)) {;}
		SLEEP(AVR32_PM_SMODE_IDLE);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int main(void)
{
#if 0 // Test
	gpio_enable_pin_pull_up(AVR32_PIN_PX00);
	gpio_enable_pin_pull_up(AVR32_PIN_PX01);
	gpio_enable_pin_pull_up(AVR32_PIN_PX02);
	gpio_enable_pin_pull_up(AVR32_PIN_PX03);
	gpio_enable_pin_pull_up(AVR32_PIN_PX04);
	gpio_enable_pin_pull_up(AVR32_PIN_PX05);
	gpio_enable_pin_pull_up(AVR32_PIN_PX06);
	gpio_enable_pin_pull_up(AVR32_PIN_PX07);
	gpio_enable_pin_pull_up(AVR32_PIN_PX08);
	gpio_enable_pin_pull_up(AVR32_PIN_PX09);
	gpio_enable_pin_pull_up(AVR32_PIN_PX10);
	gpio_enable_pin_pull_up(AVR32_PIN_PX11);
	gpio_enable_pin_pull_up(AVR32_PIN_PX12);
	gpio_enable_pin_pull_up(AVR32_PIN_PX13);
	gpio_enable_pin_pull_up(AVR32_PIN_PX14);
	gpio_enable_pin_pull_up(AVR32_PIN_PX15);
	gpio_enable_pin_pull_up(AVR32_PIN_PX16);
	gpio_enable_pin_pull_up(AVR32_PIN_PX17);
	gpio_enable_pin_pull_up(AVR32_PIN_PX18);
	gpio_enable_pin_pull_up(AVR32_PIN_PX19);
	gpio_enable_pin_pull_up(AVR32_PIN_PX20);
	gpio_enable_pin_pull_up(AVR32_PIN_PX21);
	gpio_enable_pin_pull_up(AVR32_PIN_PX22);
	gpio_enable_pin_pull_up(AVR32_PIN_PX23);
	gpio_enable_pin_pull_up(AVR32_PIN_PX24);
	gpio_enable_pin_pull_up(AVR32_PIN_PX25);
	gpio_enable_pin_pull_up(AVR32_PIN_PX26);
	gpio_enable_pin_pull_up(AVR32_PIN_PX27);
	gpio_enable_pin_pull_up(AVR32_PIN_PX28);
	gpio_enable_pin_pull_up(AVR32_PIN_PX29);
	gpio_enable_pin_pull_up(AVR32_PIN_PX30);
	gpio_enable_pin_pull_up(AVR32_PIN_PX31);
	gpio_enable_pin_pull_up(AVR32_PIN_PX32);
	gpio_enable_pin_pull_up(AVR32_PIN_PX33);
	gpio_enable_pin_pull_up(AVR32_PIN_PX34);
	gpio_enable_pin_pull_up(AVR32_PIN_PX35);
	gpio_enable_pin_pull_up(AVR32_PIN_PX36);
	gpio_enable_pin_pull_up(AVR32_PIN_PX37);
	gpio_enable_pin_pull_up(AVR32_PIN_PX38);
	gpio_enable_pin_pull_up(AVR32_PIN_PX39);

	gpio_enable_pin_pull_up(AVR32_PIN_PA00);
	gpio_enable_pin_pull_up(AVR32_PIN_PA01);
	gpio_enable_pin_pull_up(AVR32_PIN_PA02);
	gpio_enable_pin_pull_up(AVR32_PIN_PA03);
	gpio_enable_pin_pull_up(AVR32_PIN_PA04);
	gpio_enable_pin_pull_up(AVR32_PIN_PA05);
	gpio_enable_pin_pull_up(AVR32_PIN_PA06);
	gpio_enable_pin_pull_up(AVR32_PIN_PA07);
	gpio_enable_pin_pull_up(AVR32_PIN_PA08);
	gpio_enable_pin_pull_up(AVR32_PIN_PA09);
	gpio_enable_pin_pull_up(AVR32_PIN_PA10);
	gpio_enable_pin_pull_up(AVR32_PIN_PA11);
	gpio_enable_pin_pull_up(AVR32_PIN_PA12);
	gpio_enable_pin_pull_up(AVR32_PIN_PA13);
	gpio_enable_pin_pull_up(AVR32_PIN_PA14);
	gpio_enable_pin_pull_up(AVR32_PIN_PA15);
	gpio_enable_pin_pull_up(AVR32_PIN_PA16);
	gpio_enable_pin_pull_up(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PA18);
	gpio_enable_pin_pull_up(AVR32_PIN_PA19);
	gpio_enable_pin_pull_up(AVR32_PIN_PA20);
	gpio_enable_pin_pull_up(AVR32_PIN_PA21);
	gpio_enable_pin_pull_up(AVR32_PIN_PA22);
	gpio_enable_pin_pull_up(AVR32_PIN_PA23);
	gpio_enable_pin_pull_up(AVR32_PIN_PA24);
	gpio_enable_pin_pull_up(AVR32_PIN_PA25);
	gpio_enable_pin_pull_up(AVR32_PIN_PA26);
	gpio_enable_pin_pull_up(AVR32_PIN_PA27);
	gpio_enable_pin_pull_up(AVR32_PIN_PA28);
	gpio_enable_pin_pull_up(AVR32_PIN_PA29);
	gpio_enable_pin_pull_up(AVR32_PIN_PA30);

	gpio_enable_pin_pull_up(AVR32_PIN_PB00);
	gpio_enable_pin_pull_up(AVR32_PIN_PB01);
	gpio_enable_pin_pull_up(AVR32_PIN_PB02);
	gpio_enable_pin_pull_up(AVR32_PIN_PB03);
	gpio_enable_pin_pull_up(AVR32_PIN_PB04);
	gpio_enable_pin_pull_up(AVR32_PIN_PB05);
	gpio_enable_pin_pull_up(AVR32_PIN_PB06);
	gpio_enable_pin_pull_up(AVR32_PIN_PB07);
	gpio_enable_pin_pull_up(AVR32_PIN_PB08);
	gpio_enable_pin_pull_up(AVR32_PIN_PB09);
	gpio_enable_pin_pull_up(AVR32_PIN_PB10);
	gpio_enable_pin_pull_up(AVR32_PIN_PB11);
	gpio_enable_pin_pull_up(AVR32_PIN_PB12);
	gpio_enable_pin_pull_up(AVR32_PIN_PB13);
	gpio_enable_pin_pull_up(AVR32_PIN_PB14);
	gpio_enable_pin_pull_up(AVR32_PIN_PB15);
	gpio_enable_pin_pull_up(AVR32_PIN_PB16);
	gpio_enable_pin_pull_up(AVR32_PIN_PB17);
	gpio_enable_pin_pull_up(AVR32_PIN_PB18);
	gpio_enable_pin_pull_up(AVR32_PIN_PB19);
	gpio_enable_pin_pull_up(AVR32_PIN_PB20);
	gpio_enable_pin_pull_up(AVR32_PIN_PB21);
	gpio_enable_pin_pull_up(AVR32_PIN_PB22);
	gpio_enable_pin_pull_up(AVR32_PIN_PB23);
	gpio_enable_pin_pull_up(AVR32_PIN_PB24);
	gpio_enable_pin_pull_up(AVR32_PIN_PB25);
	gpio_enable_pin_pull_up(AVR32_PIN_PB26);
	gpio_enable_pin_pull_up(AVR32_PIN_PB27);
	gpio_enable_pin_pull_up(AVR32_PIN_PB28);
	gpio_enable_pin_pull_up(AVR32_PIN_PB29);
	gpio_enable_pin_pull_up(AVR32_PIN_PB30);
	gpio_enable_pin_pull_up(AVR32_PIN_PB31);

	InitSystemHardware_NS8100();

	while (1) {;}
#endif

#if 1
    // Initialize the system
	InitSystemHardware_NS8100();
	InitInterrupts_NS8100();
	InitSoftwareSettings_NS8100();

	BootLoadManager();
	DisplayVersionToCraft();

#if 0 // Test
	// Disable rs232 driver and receiver (Active low control)
	PowerControl(SERIAL_232_DRIVER_ENABLE, OFF);
	PowerControl(SERIAL_232_RECEIVER_ENABLE, OFF);
#endif

 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
		// Count Exec cycles
		g_execCycles++;

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

		// Check if able to go to sleep
		SleepManager();
	}    
	// End of NS8100 Main

	// End of the world
	return (0);
#endif
}
