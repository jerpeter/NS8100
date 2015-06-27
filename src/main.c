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
#include "adc.h"
#include "usb_task.h"
#include "device_mass_storage_task.h"
#include "host_mass_storage_task.h"
#include "usb_drv.h"
#include "srec.h"
#include "flashc.h"
#include "ushell_task.h"
#include "rtc.h"
#include "tc.h"
#include "fsaccess.h"

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
void CheckInternalMemoryForCorruption(void);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void SystemEventManager(void)
{
	//___________________________________________________________________________________________
	if (getSystemEventState(TRIGGER_EVENT))
	{
		if ((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			debug("Trigger Event (%s)\r\n", (g_triggerRecord.op_mode == WAVEFORM_MODE) ? "Wave" : "Combo");
			MoveWaveformEventToFile();
		}
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(BARGRAPH_EVENT))
	{
		clearSystemEventFlag(BARGRAPH_EVENT);

		if ((g_triggerRecord.op_mode == BARGRAPH_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			CalculateBargraphData();
		}
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(MANUAL_CAL_EVENT))
	{
		debug("Manual Cal Pulse Event\r\n");
		MoveManualCalToFile();
	}

	//___________________________________________________________________________________________
	if ((getSystemEventState(KEYPAD_EVENT)) || (g_kpadCheckForKeyFlag && (g_kpadLookForKeyTickCount < g_keypadTimerTicks)))
	{
		if (getSystemEventState(KEYPAD_EVENT))
		{
			debug("Keypad Event\r\n");
			clearSystemEventFlag(KEYPAD_EVENT);

			KeypadProcessing(KEY_SOURCE_IRQ);
		}
		else
		{
			KeypadProcessing(KEY_SOURCE_TIMER);
		}

	}

	//___________________________________________________________________________________________
#if 0 // Unused
	if (getSystemEventState())
	{
		debug("Power Off Event\r\n");
		clearSystemEventFlag();

		HandleUserPowerOffDuringTimerMode();
	}
#endif

	//___________________________________________________________________________________________
	if (getSystemEventState(LOW_BATTERY_WARNING_EVENT))
	{
		clearSystemEventFlag(LOW_BATTERY_WARNING_EVENT);
		debugWarn("Low Battery Event\r\n");

		// Check if actively monitoring
		if (g_sampleProcessing == ACTIVE_STATE)
		{
			// Stop monitoring
			StopMonitoringForLowPowerState();
		}

		sprintf((char*)g_spareBuffer, "%s %s (%3.2f). PLEASE CHARGE BATTERY.", getLangText(BATTERY_VOLTAGE_TEXT), getLangText(LOW_TEXT), (GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE)));
		OverlayMessage(getLangText(WARNING_TEXT), (char*)g_spareBuffer, (1 * SOFT_SECS));

		g_lowBatteryState = YES;
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(CYCLIC_EVENT))
	{
		clearSystemEventFlag(CYCLIC_EVENT);

#if 1 // Test (ISR/Exec Cycles)
		// Check if USB is in device mode otherwise in Host and conflicts with uShell
		if (Is_usb_id_device())
		{
			if ((g_execCycles / 4) > 10000) { strcpy((char*)g_spareBuffer, ">10K"); }
			else { sprintf((char*)g_spareBuffer, "%d", (uint16)(g_execCycles / 4)); }

			debug("(Cyclic Event) ISR Ticks/sec: %d (E:%s), Exec/sec: %s\r\n", (g_sampleCountHold / 4), ((g_channelSyncError == YES) ? "YES" : "NO"), (char*)g_spareBuffer);
		}
		g_sampleCountHold = 0;
		g_execCycles = 0;
		g_channelSyncError = NO;
#else
		debug("Cyclic Event\r\n");
#endif

		if (g_lowBatteryState == YES)
		{
			//if (gpio_get_pin_value(AVR32_PIN_PA21) == HIGH)
			if (GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE) > LOW_VOLTAGE_THRESHOLD)
			{
				debug("Recovered from Low Battery Warning Event\r\n");

				g_lowBatteryState = NO;
			}
#if 0 // Don't annoy the user for now
			else
			{
				sprintf((char*)g_spareBuffer, "%s %s (%3.2f). PLEASE CHARGE BATTERY.", getLangText(BATTERY_VOLTAGE_TEXT), getLangText(LOW_TEXT), (GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE)));
				OverlayMessage(getLangText(WARNING_TEXT), (char*)g_spareBuffer, (1 * SOFT_SECS));
			}
#endif
		}
		// Check if the battery voltage is below the safe threshold for operation
		else if (GetExternalVoltageLevelAveraged(BATTERY_VOLTAGE) < LOW_VOLTAGE_THRESHOLD)
		{
			// Change state to signal a low battery
			raiseSystemEventFlag(LOW_BATTERY_WARNING_EVENT);
		}

#if 0 // Removed debug log file due to inducing system problems
		if ((g_debugBufferCount > GLOBAL_DEBUG_BUFFER_THRESHOLD) && (g_fileAccessLock == AVAILABLE))
		{
			debug("Dumping debug output to debug log file\r\n");
			WriteDebugBufferToFile();
		}
#endif

		CheckForMidnight();

		CheckInternalMemoryForCorruption();
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(MIDNIGHT_EVENT))
	{
		debug("Midnight Event\r\n");
		clearSystemEventFlag(MIDNIGHT_EVENT);

		g_testTimeSinceLastMidnight = g_rtcSoftTimerTickCount;

		HandleMidnightEvent();
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(UPDATE_TIME_EVENT))
	{
		if (UpdateCurrentTime() == PASSED)
		{
			clearSystemEventFlag(UPDATE_TIME_EVENT);
		}
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(WARNING1_EVENT))
	{
		clearSystemEventFlag(WARNING1_EVENT);

		if (IsSoftTimerActive(ALARM_ONE_OUTPUT_TIMER_NUM) == NO)
		{
			PowerControl(ALARM_1_ENABLE, ON);
			debug("Warning Event 1 Alarm started\r\n");
		}

		// Assign (or Re-assign) soft timer to turn the Alarm 1 signal off
		AssignSoftTimer(ALARM_ONE_OUTPUT_TIMER_NUM, (uint32)(g_unitConfig.alarmOneTime * 2), AlarmOneOutputTimerCallback);
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(WARNING2_EVENT))
	{
		clearSystemEventFlag(WARNING2_EVENT);

		if (IsSoftTimerActive(ALARM_TWO_OUTPUT_TIMER_NUM) == NO)
		{
			PowerControl(ALARM_2_ENABLE, ON);
			debug("Warning Event 2 Alarm started\r\n");
		}

		// Assign (or Re-assign) soft timer to turn the Alarm 2 signal off
		AssignSoftTimer(ALARM_TWO_OUTPUT_TIMER_NUM, (uint32)(g_unitConfig.alarmTwoTime * 2), AlarmTwoOutputTimerCallback);
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(AUTO_DIALOUT_EVENT))
	{
		clearSystemEventFlag(AUTO_DIALOUT_EVENT);

		StartAutoDialoutProcess();
	}

	//___________________________________________________________________________________________
	if (getSystemEventState(UPDATE_OFFSET_EVENT))
	{
		UpdateChannelOffsetsForTempChange();
	}

	//___________________________________________________________________________________________
	if (g_powerOffActivated == YES)
	{
		// Check if idle
		if (g_sampleProcessing == IDLE_STATE)
		{
			// Check if not in Timer mode
			if (g_unitConfig.timerMode == DISABLED)
			{
				if (!(getSystemEventState(TRIGGER_EVENT)) && !(getSystemEventState(BARGRAPH_EVENT)))
				{
					PowerUnitOff(SHUTDOWN_UNIT);
				}
			}
			else // In Timer mode
			{
				// Reset the flag
				g_powerOffActivated = NO;

				// Prompt the user that power off is unavailable
				OverlayMessage(getLangText(STATUS_TEXT), getLangText(UNIT_IS_IN_TIMER_MODE_TEXT), (2 * SOFT_SECS));
			}
		}
		// Else Monitoring
		else { g_powerOffActivated = NO; }
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MenuEventManager(void)
{
	INPUT_MSG_STRUCT mn_msg;

	//debug("Menu Event Manager called\r\n");

	//___________________________________________________________________________________________
	if (getMenuEventState(RESULTS_MENU_EVENT))
	{
		clearMenuEventFlag(RESULTS_MENU_EVENT);

		if (g_triggerRecord.op_mode == MANUAL_CAL_MODE)
		{
			SETUP_RESULTS_MENU_MANUAL_CAL_MSG(RESULTS_MENU);
		}
		else
		{
			SETUP_RESULTS_MENU_MONITORING_MSG(RESULTS_MENU);
		}
		JUMP_TO_ACTIVE_MENU();
	}

	//___________________________________________________________________________________________
	if (getTimerEventState(SOFT_TIMER_CHECK_EVENT))
	{
		clearTimerEventFlag(SOFT_TIMER_CHECK_EVENT);

		// Handle and process software timers
		CheckSoftTimers();
	}

	//___________________________________________________________________________________________
	if (getTimerEventState(TIMER_MODE_TIMER_EVENT))
	{
		clearTimerEventFlag(TIMER_MODE_TIMER_EVENT);

		debug("Timer mode: Start monitoring...\r\n");
		SETUP_MENU_WITH_DATA_MSG(MONITOR_MENU, g_triggerRecord.op_mode);
		JUMP_TO_ACTIVE_MENU();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CraftManager(void)
{
	//debug("Craft Manager called\r\n");

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
		}
		else if (DSMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDSMData();
		}
		else if (DQMx_CMD == g_modemStatus.xferState)
		{
			g_modemStatus.xferState = sendDQMData();
		}
		else if (VMLx_CMD == g_modemStatus.xferState)
		{
			sendVMLData();
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
		debug("Processing message...\r\n");

		// Process message
		ProcessInputMsg(input_msg);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void FactorySetupManager(void)
{
	//debug("Factory Setup Manager called\r\n");

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
void init_hmatrix(void)
{
	union
	{
		unsigned long                 scfg;
		avr32_hmatrix_scfg_t          SCFG;
	} u_avr32_hmatrix_scfg;

	// For the internal-flash HMATRIX slave, use last master as default.
	u_avr32_hmatrix_scfg.scfg = AVR32_HMATRIX.scfg[AVR32_HMATRIX_SLAVE_FLASH];
	u_avr32_hmatrix_scfg.SCFG.defmstr_type = AVR32_HMATRIX_DEFMSTR_TYPE_LAST_DEFAULT;
	AVR32_HMATRIX.scfg[AVR32_HMATRIX_SLAVE_FLASH] = u_avr32_hmatrix_scfg.scfg;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
extern uint8 g_sampleProcessing;
extern volatile U8 device_state;
extern Bool wrong_class_connected;
extern Bool ms_process_first_connect_disconnect;
extern Bool ms_usb_prevent_sleep;
extern Bool ushell_cmd_syncevents(uint16_t*, uint16_t*);
#include "ushell_task.h"

void UsbDeviceManager(void)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 totalEventsCopied;
	uint16 totalEventsSkipped;

	//___________________________________________________________________________________________
	if (usbMassStorageState == USB_INIT_DRIVER)
	{
		// Check if the USB and Mass Storage Driver have never been initialized
		debug("Init USB Mass Storage Driver...\r\n");

#if 1 // (NS8100_ALPHA_PROTOTYPE || NS8100_BETA_PROTOTYPE)
		if (Is_usb_id_device())
		{
			// Disable VBUS power
			Usb_set_vbof_active_low();
		}
		else
		{
			// Enable VBUS power
			Usb_set_vbof_active_high();
		}

		// Wait for line to settle
		SoftUsecWait(25 * SOFT_MSECS);
#endif
		UNUSED(mn_msg);

		// Brought forward from USB OTG MSC Host/Device example
		//init_hmatrix();

		// Init the USB and Mass Storage driver			
		usb_task_init();
		device_mass_storage_task_init();
		host_mass_storage_task_init();
		ushell_task_init(FOSC0);

		// Set state to ready to process
		//usbMassStorageState = USB_NOT_CONNECTED;
		if (Is_usb_id_device())
		{
			//usbMode = USB_DEVICE_MODE_SELECTED;
			//usbMassStorageState = USB_DEVICE_MODE_SELECTED;
			debug("USB Device Mode enabled\r\n");
		}
		else
		{
			//usbMode = USB_HOST_MODE_SELECTED;
			//usbMassStorageState = USB_HOST_MODE_SELECTED;
			debug("USB OTG Host Mode enabled\r\n");
		}

		usbMassStorageState = USB_READY;
		debug("USB State changed to: Ready\r\n");
	}
	//___________________________________________________________________________________________
	else if ((usbMassStorageState == USB_READY) || (usbMassStorageState == USB_CONNECTED_AND_PROCESSING))
	{
		//___________________________________________________________________________________________
		// Check if processing is needed elsewhere
		if ((g_sampleProcessing == ACTIVE_STATE) || (getSystemEventState(TRIGGER_EVENT)) || (g_fileProcessActiveUsbLockout == ON) ||
			(g_activeMenu == CAL_SETUP_MENU) || (g_unitConfig.powerSavingsLevel > POWER_SAVINGS_NORMAL) || (g_activeMenu == MONITOR_MENU))
		{
			// Need to disable USB for other processing
			debug("USB disabled for other processing\r\n");

			if (Is_usb_enabled()) { Usb_disable(); }

			usbMassStorageState = USB_DISABLED_FOR_OTHER_PROCESSING;
			ms_usb_prevent_sleep = NO;
			debug("USB State changed to: Disabled for other processing\r\n");
		}
		//___________________________________________________________________________________________
		// Ready to process USB in either Device or OTG Host mode
		else
		{
			usb_task();
			device_mass_storage_task();
			host_mass_storage_task();
			ushell_task();

			//___________________________________________________________________________________________
			// Check if USB is in Device mode
			if (Is_usb_id_device())
			{
				// Check if VBUS is high meaning a remote PC is powering
				if (Is_usb_vbus_high())
				{
					if (usbMassStorageState == USB_READY)
					{
						usbMassStorageState = USB_CONNECTED_AND_PROCESSING;
						ms_usb_prevent_sleep = YES;
						debug("USB State changed to: Connected and processing\r\n");
					}
				}
				// Check if state was connected and VBUS power has been removed
				else if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
				{
					// Check if USB was in OTG Host mode and cable + USB Thumb drive were removed at the same time (now in Device mode)
					if (usbThumbDriveWasConnected == YES)
					{
						usbThumbDriveWasConnected = FALSE;
						ms_process_first_connect_disconnect = FALSE;
						OverlayMessage(getLangText(STATUS_TEXT), "USB THUMB DRIVE DISCONNECTED", (2 * SOFT_SECS));
					}
#if 0 // Not working properly
					else // Device mode was connected and processing
					{
						debug("USB Device cable was removed\r\n");
						OverlayMessage(getLangText(STATUS_TEXT), "USB DEVICE CABLE WAS REMOVED", (2 * SOFT_SECS));
					}
#endif
					usbMassStorageState = USB_READY;
					ms_usb_prevent_sleep = NO;
					debug("USB State changed to: Ready\r\n");
				}
#if 0 // Not working properly
				else if (promptForUsbOtgHostCableRemoval == YES)
				{
					promptForUsbOtgHostCableRemoval = NO;

					debug("USB OTG Host cable was removed\r\n");
					OverlayMessage(getLangText(STATUS_TEXT), "USB HOST OTG CABLE WAS REMOVED", (2 * SOFT_SECS));
				}
#endif
			}
			//___________________________________________________________________________________________
			else // USB is in OTG Host mode
			{
				//___________________________________________________________________________________________
				// Check if the wrong class was connected
				if (wrong_class_connected == TRUE) //|| (ms_device_not_supported == TRUE))
				{
					Usb_disable();
					debug("USB disabled due to unsupported device\r\n");

					MessageBox(getLangText(ERROR_TEXT), "UNSUPPORTED DEVICE. PLEASE REMOVE IMMEDIATELY BEFORE SELECTING OK", MB_OK);

					wrong_class_connected = FALSE;
					//ms_device_not_supported = FALSE;

					Usb_enable();
					debug("USB re-enabled for processing again\r\n");

					usb_task_init();
					device_mass_storage_task_init();
					host_mass_storage_task_init();
					ushell_task_init(FOSC0);

					usbMassStorageState = USB_READY;
					ms_usb_prevent_sleep = NO;
					debug("USB State changed to: Ready\r\n");
				}
				//___________________________________________________________________________________________
				// Check if USB Thumb drive has just been connected
				else if ((ms_connected == TRUE) && (ms_process_first_connect_disconnect == TRUE))
				{
					// USB is active
					usbMassStorageState = USB_CONNECTED_AND_PROCESSING;
					ms_usb_prevent_sleep = YES;
					debug("USB State changed to: Connected and processing\r\n");

					// Don't process this connection again until it's disconnected
					ms_process_first_connect_disconnect = FALSE;

					// State processing to help with differentiating between USB Thumb drive disconnect and cable disconnect
					usbThumbDriveWasConnected = YES;

					// Check if the LCD Power was turned off
					if (g_lcdPowerFlag == DISABLED)
					{
						g_lcdPowerFlag = ENABLED;
						SetLcdContrast(g_contrast_value);
						PowerControl(LCD_POWER_ENABLE, ON);
						SoftUsecWait(LCD_ACCESS_DELAY);
						InitLcdDisplay();					// Setup LCD segments and clear display buffer
						AssignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_unitConfig.lcdTimeout * TICKS_PER_MIN), LcdPwTimerCallBack);

						g_lcdBacklightFlag = ENABLED;
						SetLcdBacklightState(BACKLIGHT_BRIGHT);
						AssignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, DisplayTimerCallBack);
					}

					OverlayMessage(getLangText(STATUS_TEXT), "USB THUMB DRIVE DISCOVERED", (2 * SOFT_SECS));

					// Check if the user wants to sync the unit events to the USB Thumb drive
					if (MessageBox("USB DOWNLOAD", "SYNC EVENTS TO USB THUMB DRIVE?", MB_YESNO) == MB_FIRST_CHOICE)
					{
						OverlayMessage(getLangText(STATUS_TEXT), "SYNC IN PROGRESS", 0);

						if (ushell_cmd_syncevents(&totalEventsCopied, &totalEventsSkipped) == TRUE)
						{
							sprintf((char*)g_spareBuffer, "SYNC SUCCESSFUL. TOTAL EVENTS: %d NEW: %d (EXISTING: %d)", (totalEventsCopied + totalEventsSkipped), totalEventsCopied, totalEventsSkipped);
							MessageBox("USB DOWNLOAD", (char*)g_spareBuffer, MB_OK);
						}
						else // Error
						{
							MessageBox("USB DOWNLOAD", "SYNC ENCOUNTERED AN ERROR", MB_OK);
						}
					}
					else // User does not want to sync events
					{

					}

					if (GetExternalVoltageLevelAveraged(EXT_CHARGE_VOLTAGE) < EXTERNAL_VOLTAGE_PRESENT)
					{
						OverlayMessage(getLangText(STATUS_TEXT), "PLEASE REMOVE THUMB DRIVE TO CONSERVE POWER", (2 * SOFT_SECS));
					}
				}
				//___________________________________________________________________________________________
				// Check if USB Thumb drive has just been removed
				else if ((ms_connected == FALSE) && (ms_process_first_connect_disconnect == TRUE))
				{
					if (usbThumbDriveWasConnected == YES)
					{
						usbThumbDriveWasConnected = NO;
						OverlayMessage(getLangText(STATUS_TEXT), "USB THUMB DRIVE DISCONNECTED", (2 * SOFT_SECS));
					}

					// Don't process this disconnection again until it's connected
					ms_process_first_connect_disconnect = FALSE;

					// USB is ready for a device to be connected
					usbMassStorageState = USB_READY;
					ms_usb_prevent_sleep = NO;
					//promptForUsbOtgHostCableRemoval = YES;
					debug("USB State changed to: Ready\r\n");
				}
			}
		}
	}
	//___________________________________________________________________________________________
	else if (usbMassStorageState == USB_DISABLED_FOR_OTHER_PROCESSING)
	{
		if ((g_sampleProcessing != ACTIVE_STATE) && (!getSystemEventState(TRIGGER_EVENT)) && (g_fileProcessActiveUsbLockout == OFF) &&
			(g_activeMenu != CAL_SETUP_MENU) && (g_unitConfig.powerSavingsLevel < POWER_SAVINGS_MOST))
		{
			debug("USB enabled for processing again\r\n");
			Usb_enable();

			usb_task_init();
			device_mass_storage_task_init();
			host_mass_storage_task_init();
			ushell_task_init(FOSC0);

			usbMassStorageState = USB_READY;
			ms_usb_prevent_sleep = NO;
			debug("USB State changed to: Ready\r\n");
		}
	}

#if 0
	if (Is_usb_id_device() && (usbMode == USB_HOST_MODE_SELECTED))
	{
		usbMode = USB_DEVICE_MODE_SELECTED;

		// Disable VBUS power
		Usb_set_vbof_active_low();

		// Wait for line to settle
		SoftUsecWait(25 * SOFT_MSECS);
	}
	else if ((!Is_usb_id_device()) && (usbMode == USB_DEVICE_MODE_SELECTED))
	{
		usbMode = USB_HOST_MODE_SELECTED;

		// Enable VBUS power
		Usb_set_vbof_active_high();

		// Wait for line to settle
		SoftUsecWait(25 * SOFT_MSECS);
	}
#endif

#if 0
	if ((usbMassStorageState == USB_DEVICE_MODE_SELECTED) && (Is_usb_id_device()))
	{
		usb_task();
		device_mass_storage_task();
	}
	else if ((usbMassStorageState == USB_HOST_MODE_SELECTED) && (!Is_usb_id_device()))
	{
		usb_task();
		host_mass_storage_task();
		ushell_task();
	}
	else
	{
		if (Is_usb_id_device())
		{
			debug("Changing USB to Device Mode\r\n");
			usbMassStorageState = USB_DEVICE_MODE_SELECTED;
			usb_task_init();
			device_mass_storage_task_init();
		}
		else
		{
			debug("Changing USB to OTG Host Mode\r\n");
			usbMassStorageState = USB_HOST_MODE_SELECTED;
			usb_task_init();
			host_mass_storage_task_init();
			ushell_task_init(FOSC0);
		}
	}
#endif

#if 0 // Normal
	//___________________________________________________________________________________________
	else if ((usbMassStorageState == USB_NOT_CONNECTED) || (usbMassStorageState == USB_HOST_MODE_WAITING_FOR_DEVICE))
	{
		// Check if ready for USB (not monitoring and not handling a trigger and not processing an SD card file)
		if ((g_sampleProcessing != ACTIVE_STATE) && (!getSystemEventState(TRIGGER_EVENT)) && (g_fileProcessActiveUsbLockout == OFF))
		{
			// Check if USB ID is set for Device and VBUS is High (plugged into PC)
			if ((Is_usb_id_device()) && (Is_usb_vbus_high()))
			{
				OverlayMessage("USB DEVICE STATUS", "USB TO PC CABLE WAS CONNECTED", 1 * SOFT_SECS);
				debug("USB Device mode: USB to PC connection established\r\n");

				// Recall the current active menu to repaint the display
				mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
				JUMP_TO_ACTIVE_MENU();

				// Re-Init the USB and Mass Storage driver
				debug("USB Device Mass Storage Driver Re-Init\r\n");
				usb_task_init();
				device_mass_storage_task_init();

				// Set state to ready to process
				usbMassStorageState = USB_CONNECTED_AND_PROCESSING;
			}
			// Check if USB ID is set for Host
			else if (!Is_usb_id_device())
			{
				if ((Is_host_device_connection()) || (device_state == DEVICE_POWERED))
				{
					OverlayMessage("USB HOST STATUS", "USB DEVICE WAS CONNECTED", 1 * SOFT_SECS);
					debug("USB OTG Host mode: OTG Host cable was connected\r\n");

					// Recall the current active menu to repaint the display
					mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
					JUMP_TO_ACTIVE_MENU();

					// Re-Init the USB and Mass Storage driver
					debug("USB OTG Host Mass Storage Driver Re-Init\r\n");
					usb_task_init();
					host_mass_storage_task_init();
					ushell_task_init(FOSC0);

					// Set state to host mode looking for a device
					usbMassStorageState = USB_CONNECTED_AND_PROCESSING;
				}
				else if (usbMassStorageState == USB_NOT_CONNECTED)
				{
					OverlayMessage("USB HOST STATUS", "USB OTG HOST CABLE WAS CONNECTED", 1 * SOFT_SECS);
					debug("USB OTG Host mode: OTG Host cable was connected\r\n");

					// Recall the current active menu to repaint the display
					mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
					JUMP_TO_ACTIVE_MENU();

					// Re-Init the USB and Mass Storage driver
					debug("USB OTG Host Mass Storage Driver Re-Init\r\n");
					usb_task_init();
					host_mass_storage_task_init();
					ushell_task_init(FOSC0);

					// Set state to host mode looking for a device
					usbMassStorageState = USB_HOST_MODE_WAITING_FOR_DEVICE;
				}
			}
		}
	}
	//___________________________________________________________________________________________
	else if (usbMassStorageState == USB_HOST_MODE_WAITING_FOR_DEVICE)
	{
		if ((Is_host_device_connection()) || (device_state == DEVICE_POWERED))
		{
			OverlayMessage("USB HOST STATUS", "USB DEVICE WAS CONNECTED", 1 * SOFT_SECS);
			debug("USB OTG Host mode: OTG Host cable was connected\r\n");

			// Set state to host mode looking for a device
			usbMassStorageState = USB_CONNECTED_AND_PROCESSING;
		}
	}
	//___________________________________________________________________________________________
	else if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
	{
		if (((Is_usb_id_device() && Is_usb_vbus_high()) || ((!Is_usb_id_device()) && (!Is_host_device_disconnection())))
			&& (g_sampleProcessing != ACTIVE_STATE) && (!getSystemEventState(TRIGGER_EVENT)) && g_fileProcessActiveUsbLockout == OFF)
		{
			// Call Usb and Device Storage drivers if connected to check and handle incoming actions
			usb_task();
			device_mass_storage_task();
			host_mass_storage_task();
			ushell_task();
		}
		else // USB connection lost/gone or needs to be disabled
		{
			if (g_sampleProcessing == ACTIVE_STATE) // A Trigger event is an extension of Active state so don't need to check for that specifically
			{
				OverlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR MONITORING", 1000 * SOFT_MSECS);
				debug("USB disabled for monitoring\r\n");
				Usb_disable();
				usbMassStorageState = USB_DISABLED_FOR_OTHER_PROCESSING;
			}
			else if (g_fileProcessActiveUsbLockout == ON)
			{
				OverlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR FILE OPERATION", 1000 * SOFT_MSECS);
				debug("USB disabled for file operation\r\n");
				Usb_disable();
				usbMassStorageState = USB_DISABLED_FOR_OTHER_PROCESSING;
			}
			else
			{
				// Check if USB ID is set for Device mode (PC connection)
				if (Is_usb_id_device())
				{
					OverlayMessage("USB DEVICE STATUS", "USB TO PC CABLE WAS DISCONNECTED", 1 * SOFT_SECS);
					debug("USB Device mode: USB to PC connection removed\r\n");
				}
				else
				{
					OverlayMessage("USB HOST STATUS", "USB DEVICE WAS REMOVED", 1 * SOFT_SECS);
					debug("USB OTG Host mode: Device disconnected\r\n");
				}

				// Recall the current active menu to repaint the display
				mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
				JUMP_TO_ACTIVE_MENU();

				usbMassStorageState = USB_NOT_CONNECTED;
			}
		}
	}
	//___________________________________________________________________________________________
	else if (usbMassStorageState == USB_DISABLED_FOR_OTHER_PROCESSING)
	{
		// Check if system is ready for USB processing again
		if ((g_sampleProcessing != ACTIVE_STATE) && (!getSystemEventState(TRIGGER_EVENT)) && (g_fileProcessActiveUsbLockout == OFF))
		{
			// Reenable the USB
			debug("USB re-enabled\r\n");
			Usb_enable();
			usbMassStorageState = USB_NOT_CONNECTED;
		}
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#define APPLICATION    (((void *)AVR32_EBI_CS1_ADDRESS) + 0x00700000)
const char default_boot_name[] = {
	"Boot.s\0"
};

void BootLoadManager(void)
{
	int16 usart_status;
	int craft_char;
	char textBuffer[50];
	static void (*func)(void);
	func = (void(*)())APPLICATION;
	uint32 i;
	int file = -1;

	usart_status = USART_RX_EMPTY;
	i = 0;

	usart_status = usart_read_char(DBG_USART, &craft_char);

	if ((craft_char == CTRL_B) || (g_quickBootEntryJump == YES))
	{
#if 1
		if (g_quickBootEntryJump == YES)
		{
			if (g_sampleProcessing == ACTIVE_STATE)
			{
				// Don't allow hidden jumping to boot during Monitoring
				g_quickBootEntryJump = NO;
				return;
			}
			// else (g_sampleProcessing == IDLE_STATE)

			if (g_lcdPowerFlag == DISABLED)
			{
				SetLcdContrast(g_contrast_value);
				PowerControl(LCD_POWER_ENABLE, ON);
				SoftUsecWait(LCD_ACCESS_DELAY);
				InitLcdDisplay();
				SetLcdBacklightState(BACKLIGHT_BRIGHT);
			}
			else
			{
				SetLcdBacklightState(BACKLIGHT_BRIGHT);
			}

			OverlayMessage("BOOTLOADER", "HIDDEN ENTRY...", 2 * SOFT_SECS);
		}
		else
#endif
			OverlayMessage("BOOTLOADER", "FOUND CTRL_B...", 2 * SOFT_SECS);

		if (g_fileAccessLock != AVAILABLE)
		{
			ReportFileSystemAccessProblem("Bootloader access");
		}

		nav_select(FS_NAV_ID_DEFAULT);
		sprintf(textBuffer, "A:\\System\\%s", default_boot_name);
		file = open(textBuffer, O_RDONLY);

		while (file == -1)
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
			
			file = open(textBuffer, O_RDONLY);
		}

		// Display initializing message
		debug("Starting Boot..\r\n");

		OverlayMessage("BOOTLOADER", "STARTING BOOTLOADER...", 2 * SOFT_SECS);

		ClearLcdDisplay();

		if (Unpack_srec(file) == -1)
		{
			debugErr("SREC unpack unsuccessful\r\n");
		}

		g_testTimeSinceLastFSWrite = g_rtcSoftTimerTickCount;
		close(file);

#if 0 // Removed debug log file due to inducing system problems
		debug("Dumping debug output to debug log file\r\n");
		debug("Adding On/Off Log timestamp before jumping to boot\r\n");
		WriteDebugBufferToFile();
#endif
		AddOnOffLogTimestamp(JUMP_TO_BOOT);

		// Enable half second tick
extern void rtc_disable_interrupt(volatile avr32_rtc_t *rtc);
		rtc_disable_interrupt(&AVR32_RTC);
		tc_stop(&AVR32_TC, TC_SAMPLE_TIMER_CHANNEL);
		tc_stop(&AVR32_TC, TC_CALIBRATION_TIMER_CHANNEL);
		tc_stop(&AVR32_TC, TC_TYPEMATIC_TIMER_CHANNEL);
		AVR32_EIC.IER.int0 = 0;
		AVR32_EIC.IER.int1 = 0;
		AVR32_EIC.IER.int2 = 0;
		AVR32_EIC.IER.int3 = 0;
		AVR32_EIC.IER.int4 = 0;
		AVR32_EIC.IER.int5 = 0;
		AVR32_EIC.IER.int6 = 0;
		AVR32_EIC.IER.int7 = 0;
		StopExternalRtcClock();
		Usb_disable();
		Usb_disable_id_interrupt();
		Usb_disable_vbus_interrupt();
		twi_disable_interrupt(&AVR32_TWI);
		usart_reset(&AVR32_USART0);
		AVR32_USART1.idr = 0xFFFFFFFF;
		usart_reset(&AVR32_USART1);
		usart_reset(&AVR32_USART2);
		usart_reset(&AVR32_USART3);

		PowerControl(POWER_OFF_PROTECTION_ENABLE, OFF);

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
static uint8 rs232PutToSleepState = NO;
inline void SetupPowerSavingsBeforeSleeping(void)
{
	g_testApplyPS = Get_system_register(AVR32_COUNT);

	// Only disable for Min and Most since None and Max are either permanently on or off
	if ((g_unitConfig.powerSavingsLevel >= POWER_SAVINGS_MINIMUM) || (g_unitConfig.powerSavingsLevel <= POWER_SAVINGS_MOST))
	{
		if (READ_DCD == NO_CONNECTION)
		{
			rs232PutToSleepState = YES;

			// Disable rs232 driver and receiver (Active low control)
			PowerControl(SERIAL_232_DRIVER_ENABLE, OFF);
			PowerControl(SERIAL_232_RECEIVER_ENABLE, OFF);
		}
	}

	// Enable pull ups on the data lines
	AVR32_GPIO.port[2].puers = 0xFC00; // 1111 1100 0000 0000
	AVR32_GPIO.port[3].puers = 0x03FF; // 0000 0011 1111 1111

	g_powerSavingsForSleepEnabled = YES;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
inline void RevertPowerSavingsAfterSleeping(void)
{
	g_testRevertPS = Get_system_register(AVR32_COUNT);

#if 1 // Normal
	// Disable pull ups on the data lines
	AVR32_GPIO.port[2].puerc = 0xFC00; // 1111 1100 0000 0000
	AVR32_GPIO.port[3].puerc = 0x03FF; // 0000 0011 1111 1111
#endif

#if 1
	// Only enable for Min and Most since None and Max are either permanently on or off
	if ((g_unitConfig.powerSavingsLevel >= POWER_SAVINGS_MINIMUM) || (g_unitConfig.powerSavingsLevel <= POWER_SAVINGS_MOST))
	{
		if (rs232PutToSleepState == YES)
		{
			rs232PutToSleepState = NO;

#if 1 // Normal
			// Enable rs232 driver and receiver (Active low control)
			PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
			PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
#else
			SoftUsecWait(5);
			SoftUsecWait(5);
#endif
		}
	}
#endif

	g_powerSavingsForSleepEnabled = NO;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerManager(void)
{
	uint8 sleepStateNeeded;

#if 0 // No current signal to show a serial connection has been re-established
	/*
	static uint8 rs232State = ON;

	// Check if not set to the Max power savings
	if (g_unitConfig.powerSavingsLevel != POWER_SAVINGS_MAX)
	{
		// Check if rs232 is already on and DSR shows no connection
		if ((rs232State == ON) && (gpio_get_pin_value(AVR32_PIN_PB24) == 1))
		{
			// Disable the serial rs232 drivers
			PowerControl(SERIAL_232_DRIVER_ENABLE, OFF);
			PowerControl(SERIAL_232_RECEIVER_ENABLE, OFF);
			rs232State = OFF;
		}
		// Check if rs232 is off and DSR shows a new connection
		else if ((rs232State == OFF) && (gpio_get_pin_value(AVR32_PIN_PB24) == 0))
		{
			// Enable the serial rs232 drivers
			PowerControl(SERIAL_232_DRIVER_ENABLE, ON);
			PowerControl(SERIAL_232_RECEIVER_ENABLE, ON);
			rs232State = ON;
			debug("USART1 RS232 Rx/Tx Drivers enabled\r\n");
		}
	}
	*/
#endif

	// Check if no System Events (or just update offset) and LCD is off and Modem is not transferring and USB is not connected
	if (((g_systemEventFlags.wrd == NO_SYSTEM_EVENT_ACTIVE) || (g_systemEventFlags.wrd == UPDATE_OFFSET_EVENT)) && (GetPowerControlState(LCD_POWER_ENABLE) == OFF) &&
		(g_modemStatus.xferState == NOP_CMD) && (ms_usb_prevent_sleep == NO)) //(usbMassStorageState != USB_CONNECTED_AND_PROCESSING))
	{
		SetupPowerSavingsBeforeSleeping();

		g_sleepModeEngaged = YES;

		// Check if actively monitoring
		if (g_sampleProcessing == ACTIVE_STATE)
		{
			// Check if a higher sample rate that can't use Stop sleep mode (Wave 16K and Bar 8K)
			if ((g_triggerRecord.trec.sample_rate == SAMPLE_RATE_16K) ||
				((g_triggerRecord.op_mode == BARGRAPH_MODE) && (g_triggerRecord.trec.sample_rate == SAMPLE_RATE_8K)))
			{
				sleepStateNeeded = AVR32_PM_SMODE_IDLE;
			}
			// Check if Wave 8K
			else if (g_triggerRecord.trec.sample_rate == SAMPLE_RATE_8K)
			{
				sleepStateNeeded = AVR32_PM_SMODE_FROZEN;
			}
			// Check if Wave, Bar and Combo 4K
			else if (g_triggerRecord.trec.sample_rate == SAMPLE_RATE_4K)
			{
				sleepStateNeeded = AVR32_PM_SMODE_STANDBY;
			}
			else // Lower sample rates can use Stop mode (Wave, Bar and Combo for 0.5K, 1K and 2K)
			{
				sleepStateNeeded = AVR32_PM_SMODE_STOP;
			}
		}
		else // Not monitoring
		{
			sleepStateNeeded = AVR32_PM_SMODE_STOP;
		}

		// Check if not already set for Idle sleep and not max power savings and a remote/craft is connected (DSR and DCD)
		if ((sleepStateNeeded != AVR32_PM_SMODE_IDLE) && (g_unitConfig.powerSavingsLevel != POWER_SAVINGS_MAX) &&
			((READ_DSR == MODEM_CONNECTED) && (READ_DCD == CONNECTION_ESTABLISHED)))
		{
			sleepStateNeeded = AVR32_PM_SMODE_FROZEN;
		}

		// Check if sleep mode changed
		if (g_sleepModeState != sleepStateNeeded)
		{
			// Track the new state
			g_sleepModeState = sleepStateNeeded;

			if (g_sleepModeState == AVR32_PM_SMODE_IDLE) { debug("Changing Sleep mode to Idle\r\n"); }
			else if (g_sleepModeState == AVR32_PM_SMODE_FROZEN) { debug("Changing Sleep mode to Frozen\r\n"); }
			else if (g_sleepModeState == AVR32_PM_SMODE_STANDBY) { debug("Changing Sleep mode to Standby\r\n"); }
			else if (g_sleepModeState == AVR32_PM_SMODE_STOP) { debug("Changing Sleep mode to Stop\r\n"); }
		}

#if 1 // Test reads to clear the bus
		PB_READ_TO_CLEAR_BUS_BEFORE_SLEEP;
#endif

#if 0 // Normal
		if (g_sleepModeState == AVR32_PM_SMODE_IDLE) { SLEEP(AVR32_PM_SMODE_IDLE); }
		else if (g_sleepModeState == AVR32_PM_SMODE_FROZEN) { SLEEP(AVR32_PM_SMODE_FROZEN); }
		else if (g_sleepModeState == AVR32_PM_SMODE_STANDBY) { SLEEP(AVR32_PM_SMODE_STANDBY); }
		else if (g_sleepModeState == AVR32_PM_SMODE_STOP) { SLEEP(AVR32_PM_SMODE_STOP); }
#else // Fix to prevent STOP mode which causes random exceptions
		if (g_sleepModeState == AVR32_PM_SMODE_IDLE) { SLEEP(AVR32_PM_SMODE_IDLE); }
		else if (g_sleepModeState == AVR32_PM_SMODE_FROZEN) { SLEEP(AVR32_PM_SMODE_FROZEN); }
		else { SLEEP(AVR32_PM_SMODE_STANDBY); }

		g_testAfterSleep = Get_system_register(AVR32_COUNT);
#endif
		// Check if needing to revert the power savings (if monitoring then the ISR will handle this operation)
		if (g_powerSavingsForSleepEnabled == YES)
		{
			RevertPowerSavingsAfterSleeping();
		}
	}
	else // To busy to sleep
	{
		g_sleepModeEngaged = NO;
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

	debug("--- System Init complete (Version %d.%d.%c) ---\r\n", majorVer, minorVer, buildVer);
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
			debug("Tick tock (%d, %d)\r\n", sampleRate, g_updateCounter / sampleRate);
		}
	
		while (!usart_tx_empty(DBG_USART)) {;}
		SLEEP(AVR32_PM_SMODE_IDLE);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
#include "nlao_exceptions.h"
#define MAX_STATES	2
#define LAST_STATE	(MAX_STATES - 1)
__attribute__((__interrupt__))
void exception(uint32_t r12, uint32_t r11, uint32_t r10, uint32_t r9, uint32_t exception_number, uint32_t lr, uint32_t r7, uint32_t r6, uint32_t r5, uint32_t r4,
                uint32_t r3, uint32_t r2, uint32_t r1, uint32_t r0, uint32_t sp, uint32_t sr, uint32_t pc, uint32_t stack0, uint32_t stack1, uint32_t stack2)
{
	char exceptionText[30];
	char exceptionMessage[75];
	int LFST = -1;
	int LTT = -1;
	int LMT = -1;
	int LCPT = -1;
	uint16 testCounter = 0;
	uint16* intMem = (uint16*)0x3000;
	uint32 testKPGetExtVoltage;
	uint32 testKPReadMCP23018;
	uint32 testKPWriteMCP23018;
	uint32 testAfterSleepISR;
	uint32 testAfterSleep;
	uint32 testRevertPS;
	uint32 testApplyPS;
	uint32 testCycleCount;

	//uint8 state = 0;
	//uint8 key = KEY_NONE;

    /* Adjust SP to pre-exception value.
        * PC and SR are 4 bytes each.
        */
    sp += 8;

	testCycleCount = Get_system_register(AVR32_COUNT);

	UNUSED(r12);
	UNUSED(r11);
	UNUSED(r10);
	UNUSED(r9);
	UNUSED(lr);
	UNUSED(r7);
	UNUSED(r6);
	UNUSED(r5);
	UNUSED(r4);
	UNUSED(r3);
	UNUSED(r2);
	UNUSED(r1);
	UNUSED(r0);
	UNUSED(sp);
	UNUSED(sr);
	UNUSED(stack0);
	UNUSED(stack1);
	UNUSED(stack2);

	//debugRaw("\r\n\nERROR - Exception: ");

	switch (((exception_number * 4) - 4))
	{
		case EVBA_UNRECOVERABLE: strcpy((char*)&exceptionText, "Unrecoverable"); debugRaw("Unrecoverable"); break;
		case EVBA_TLB_MULTIPLE:	strcpy((char*)&exceptionText, "TLB Multiple Hit"); debugRaw("TLB Multiple Hit"); break;
		case EVBA_BUS_ERROR_DATA: strcpy((char*)&exceptionText, "Bus Error Data Fetch"); debugRaw("Bus Error Data Fetch"); break;
		case EVBA_BUS_ERROR_INSTR: strcpy((char*)&exceptionText, "Bus Error instruction Fetch"); debugRaw("Bus Error instruction Fetch"); break;
		case EVBA_NMI: strcpy((char*)&exceptionText, "NMI"); debugRaw("NMI"); break;
		case EVBA_INSTR_ADDR: strcpy((char*)&exceptionText, "Instruction Address"); debugRaw("Instruction Address"); break;
		case EVBA_ITLB_PROT: strcpy((char*)&exceptionText, "ITLB Protection"); debugRaw("ITLB Protection"); break;
		case EVBA_BREAKPOINT: strcpy((char*)&exceptionText, "Breakpoint"); debugRaw("Breakpoint"); break;
		case EVBA_ILLEGAL_OPCODE: strcpy((char*)&exceptionText, "Illegal Opcode"); debugRaw("Illegal Opcode"); break;
		case EVBA_UNIMPLEMENTED: strcpy((char*)&exceptionText, "Unimplemented Instruction"); debugRaw("Unimplemented Instruction"); break;
		case EVBA_PRIVILEGE_VIOL: strcpy((char*)&exceptionText, "Privilege Violation"); debugRaw("Privilege Violation"); break;
		case EVBA_FLOATING_POINT: strcpy((char*)&exceptionText, "Floating Point"); debugRaw("Floating Point"); break;
		case EVBA_COP_ABSENT: strcpy((char*)&exceptionText, "Coprocessor Absent"); debugRaw("Coprocessor Absent"); break;
		case EVBA_DATA_ADDR_R: strcpy((char*)&exceptionText, "Data Address (Read)"); debugRaw("Data Address (Read)"); break;
		case EVBA_DATA_ADDR_W: strcpy((char*)&exceptionText, "Data Address (Write)"); debugRaw("Data Address (Write)"); break;
		case EVBA_DTLB_PROT_R: strcpy((char*)&exceptionText, "DLTB Protection (Read)"); debugRaw("DLTB Protection (Read)"); break;
		case EVBA_DTLB_PROT_W: strcpy((char*)&exceptionText, "DTLB Protection (Write)"); debugRaw("DTLB Protection (Write)"); break;
		case EVBA_DTLB_MODIFIED: strcpy((char*)&exceptionText, "DTLB Modified"); debugRaw("DTLB Modified"); break;
		case EVBA_ITLB_MISS: strcpy((char*)&exceptionText, "ITLB Miss"); debugRaw("ITLB Miss"); break;
		case EVBA_DTLB_MISS_R: strcpy((char*)&exceptionText, "DTLB Miss (Read)"); debugRaw("DTLB Miss (Read)"); break;
		case EVBA_DTLB_MISS_W: strcpy((char*)&exceptionText, "DTLB Miss (Write)"); debugRaw("DTLB Miss (Write)"); break;
		case EVBA_SCALL: strcpy((char*)&exceptionText, "Scall"); debugRaw("Scall"); break;
		default: strcpy((char*)&exceptionText, "Unknown EVBA offset"); debugErr("Unknown EVBA offset"); break;
	}

	//debugRaw(" at PC = 0x%x\r\n\n", pc);

	// Check if the LCD Power was turned off
	if (g_lcdPowerFlag == DISABLED)
	{
		g_lcdPowerFlag = ENABLED;
		SetLcdContrast(g_contrast_value);
		PowerControl(LCD_POWER_ENABLE, ON);
		SoftUsecWait(LCD_ACCESS_DELAY);
		InitLcdDisplay();
	}

	// Check if the LCD Backlight was turned off
	if (g_lcdBacklightFlag == DISABLED)
	{
		g_lcdBacklightFlag = ENABLED;
		SetLcdBacklightState(BACKLIGHT_BRIGHT);
	}

#if 1
	if (g_rtcSoftTimerTickCount >= g_testTimeSinceLastFSWrite) { LFST = (g_rtcSoftTimerTickCount - g_testTimeSinceLastFSWrite); }
	if (g_rtcSoftTimerTickCount >= g_testTimeSinceLastTrigger) { LTT = (g_rtcSoftTimerTickCount - g_testTimeSinceLastTrigger); }
	if (g_rtcSoftTimerTickCount >= g_testTimeSinceLastMidnight) { LMT = (g_rtcSoftTimerTickCount - g_testTimeSinceLastMidnight); }
	if (g_rtcSoftTimerTickCount >= g_testTimeSinceLastCalPulse) { LCPT = (g_rtcSoftTimerTickCount - g_testTimeSinceLastCalPulse); }
#endif

#if 1
	while (intMem < (uint16*)0x4000)
	{
		// Check internal memory locations below the stack
		if (*intMem == 0xD673)
		{
			testCounter += 2;
			intMem++;
		}
		else break;
	}
#endif

#if 1
	if (testCycleCount > g_testKPGetExtVoltage) { testKPGetExtVoltage = testCycleCount - g_testKPGetExtVoltage; }
	else { testKPGetExtVoltage = 0xFFFFFFFF - g_testKPGetExtVoltage + testCycleCount; }

	if (testCycleCount > g_testKPReadMCP23018) { testKPReadMCP23018 = testCycleCount - g_testKPReadMCP23018; }
	else { testKPReadMCP23018 = 0xFFFFFFFF - g_testKPReadMCP23018 + testCycleCount; }

	if (testCycleCount > g_testKPWriteMCP23018) { testKPWriteMCP23018 = testCycleCount - g_testKPWriteMCP23018; }
	else { testKPWriteMCP23018 = 0xFFFFFFFF - g_testKPWriteMCP23018 + testCycleCount; }

	if (testCycleCount > g_testAfterSleepISR) { testAfterSleepISR = testCycleCount - g_testAfterSleepISR; }
	else { testAfterSleepISR = 0xFFFFFFFF - g_testAfterSleepISR + testCycleCount; }

	if (testCycleCount > g_testAfterSleep) { testAfterSleep = testCycleCount - g_testAfterSleep; }
	else { testAfterSleep = 0xFFFFFFFF - g_testAfterSleep + testCycleCount; }

	if (testCycleCount > g_testRevertPS) { testRevertPS = testCycleCount - g_testRevertPS; }
	else { testRevertPS = 0xFFFFFFFF - g_testRevertPS + testCycleCount; }

	if (testCycleCount > g_testApplyPS) { testApplyPS = testCycleCount - g_testApplyPS; }
	else { testApplyPS = 0xFFFFFFFF - g_testApplyPS + testCycleCount; }
#endif

	while (1)
	{
		if (((exception_number * 4) - 4) == EVBA_BREAKPOINT)
		{
			switch (g_breakpointCause)
			{
				case BP_INT_MEM_CORRUPTED: sprintf((char*)&exceptionMessage, "BREAKPOINT: INTERNAL MEM TAMPERED"); break;
				case BP_SOFT_LOOP: sprintf((char*)&exceptionMessage, "BREAKPOINT: NON ISR FOREVER LOOP DETECTED"); break;
				case BP_MB_LOOP: sprintf((char*)&exceptionMessage, "BREAKPOINT: MESSAGE BOX FOREVER LOOP DETECTED"); break;
				case BP_UNHANDLED_INT: sprintf((char*)&exceptionMessage, "BREAKPOINT: UNHANDLED INTERRUPT DETECTED"); break;
				case BP_AD_CHAN_SYNC_ERR: sprintf((char*)&exceptionMessage, "BREAKPOINT: A/D CHAN SYNC ERROR"); break;
				default: sprintf((char*)&exceptionMessage, "BREAKPOINT: CAUSE UNKNOWN");
			}
			OverlayMessage("EXC SCREEN 1", (char*)&exceptionMessage, (8 * SOFT_SECS));

			if (g_breakpointCause == BP_MB_LOOP)
			{
				sprintf((char*)&exceptionMessage, "MSG TEXT: %s", (char*)g_debugBuffer);
				//OverlayMessage("EXC SCREEN 1a", (char*)&exceptionMessage, (8 * SOFT_SECS));
				OverlayMessage((char*)&g_buildVersion, (char*)&exceptionMessage, (8 * SOFT_SECS));
			}
			else if (g_breakpointCause == BP_INT_MEM_CORRUPTED)
			{
				testCounter = 0;
				intMem = (uint16*)0x0004;
				while (intMem < (uint16*)0x3000)
				{
					if (*intMem != 0xD673)
					{
						testCounter++;
					}

					intMem++;
				}

				intMem = (uint16*)0x4000;
				while (intMem < (uint16*)0x10000)
				{
					if (*intMem != 0xD673)
					{
						testCounter++;
					}

					intMem++;
				}

				intMem = (uint16*)0x0004;
				while (intMem < (uint16*)0x3000)
				{
					if (*intMem != 0xD673)
					{
						break;
					}

					intMem++;
				}

				if (intMem >= (uint16*)0x3000)
				{
					intMem = (uint16*)0x4000;
					while (intMem < (uint16*)0x10000)
					{
						if (*intMem != 0xD673)
						{
							break;
						}

						intMem++;
					}
				}

				sprintf((char*)&exceptionMessage, "FIRST ADDR:%p COUNT:%d", intMem, testCounter);
				//OverlayMessage("EXC SCREEN 1a", (char*)&exceptionMessage, (8 * SOFT_SECS));
				OverlayMessage((char*)&g_buildVersion, (char*)&exceptionMessage, (8 * SOFT_SECS));
			}
			else
			{
				sprintf((char*)&exceptionMessage, "MAX SD:%d bytes (TOP ADDR:%p)", (int)(0x1000 - testCounter), intMem);
				//OverlayMessage("EXC SCREEN 1a", (char*)&exceptionMessage, (8 * SOFT_SECS));
				OverlayMessage((char*)&g_buildVersion, (char*)&exceptionMessage, (8 * SOFT_SECS));
			}
		}
		else
		{
			sprintf((char*)&exceptionMessage, "EXC: %s at PC:0x%lx", (char*)exceptionText, pc);
			OverlayMessage("EXC SCREEN 1", (char*)&exceptionMessage, (8 * SOFT_SECS));

			sprintf((char*)&exceptionMessage, "MAX SD:%d bytes (TOP ADDR:%p)", (int)(0x1000 - testCounter), intMem);
			//OverlayMessage("EXC SCREEN 1a", (char*)&exceptionMessage, (8 * SOFT_SECS));
			OverlayMessage((char*)&g_buildVersion, (char*)&exceptionMessage, (8 * SOFT_SECS));
		}

		sprintf((char*)&exceptionMessage, "SP:0x%lx S0:0x%lx S1:0x%lx S2:0x%lx", sp, stack0, stack1, stack2);
		OverlayMessage("EXC SCREEN 2", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "SR:0x%lx LR:0x%lx EXC:0x%lx", sr, lr, exception_number);
		OverlayMessage("EXC SCREEN 3", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "R0:0x%lx R1:0x%lx R2:0x%lx R3:0x%lx", r0, r1, r2, r3);
		OverlayMessage("EXC SCREEN 4", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "R4:0x%lx R5:0x%lx R6:0x%lx R7:0x%lx", r4, r5, r6, r7);
		OverlayMessage("EXC SCREEN 5", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "R9:0x%lx R10:0x%lx R11:0x%lx R12:0x%lx", r9, r10, r11, r12);
		OverlayMessage("EXC SCREEN 6", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "SR:%lu FS:%d T:%d CP:%d M:%d", g_rtcSoftTimerTickCount, LFST, LTT, LCPT, LMT);
		OverlayMessage("EXC SCREEN 7", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "GEV:%lu RMCP:%lu WMCP:%lu", testKPGetExtVoltage, testKPReadMCP23018, testKPWriteMCP23018);
		OverlayMessage("EXC SCREEN 8", (char*)&exceptionMessage, (8 * SOFT_SECS));

		sprintf((char*)&exceptionMessage, "ASI: %lu AS:%lu APS:%lu RPS:%lu", testAfterSleepISR, testAfterSleep, testApplyPS, testRevertPS);
		OverlayMessage("EXC SCREEN 9", (char*)&exceptionMessage, (8 * SOFT_SECS));
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 intMemProblem = NO;
uint8 spareBufferTitle[100];
uint8 spareBufferText[100];
uint32 intMemCount = 0;
void TestIntMem(char* titleString)
{
#if 1 // ET Test
	uint16* intMem = (uint16*)0x0004;
	while (intMem < (uint16*)0x3000)
	{
		// Check internal memory locations below the stack
		if (*intMem != 0xD673)
		{
			intMemCount = 0;
			sprintf((char*)spareBufferText, "UNUSED INT MEM CHANGED: ADDR %p = 0x%x", intMem, *intMem);
			//OverlayMessage(titleString, (char*)g_spareBuffer, (3 * SOFT_SECS));
			intMemProblem = YES;
			strcpy((char*)spareBufferTitle, titleString);

			while (intMem < (uint16*)0x3000)
			{
				if (*intMem != 0xD673) { intMemCount++; }
				*intMem++ = 0xD673;
			}
			break;
		}

		intMem++;
	}

	intMem = (uint16*)0x4000;
	while (intMem < (uint16*)0x10000)
	{
		// Check internal memory locations below the stack
		if (*intMem != 0xD673)
		{
			intMemCount = 0;
			sprintf((char*)spareBufferText, "UNUSED INT MEM CHANGED: ADDR %p = 0x%x", intMem, *intMem);
			//OverlayMessage(titleString, (char*)g_spareBuffer, (3 * SOFT_SECS));
			intMemProblem = YES;
			strcpy((char*)spareBufferTitle, titleString);

			while (intMem < (uint16*)0x10000)
			{
				if (*intMem != 0xD673) { intMemCount++; }
				*intMem++ = 0xD673;
			}
			break;
		}

		intMem++;
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CheckInternalMemoryForCorruption(void)
{
#if 1 // ET Test
	uint16* intMem = (uint16*)0x0004;

	while (intMem < (uint16*)0x3000)
	{
		// Check internal memory locations below the stack
		if (*intMem != 0xD673)
		{
#if 1
			g_breakpointCause = BP_INT_MEM_CORRUPTED;
			__asm__ __volatile__ ("breakpoint");
#endif
			sprintf((char*)g_spareBuffer, "UNUSED INT MEM CHANGED: ADDR %p = 0x%x", intMem, *intMem);
			OverlayMessage(getLangText(ERROR_TEXT), (char*)g_spareBuffer, (3 * SOFT_SECS));

			while (intMem < (uint16*)0x3000)
			{
				*intMem++ = 0xD673;
			}
			break;
		}

		intMem++;
	}

	intMem = (uint16*)0x4000;
	while (intMem < (uint16*)0x10000)
	{
		// Check internal memory locations below the stack
		if (*intMem != 0xD673)
		{
#if 1
			g_breakpointCause = BP_INT_MEM_CORRUPTED;
			__asm__ __volatile__ ("breakpoint");
#endif
			sprintf((char*)g_spareBuffer, "UNUSED INT MEM CHANGED: ADDR %p = 0x%x", intMem, *intMem);
			OverlayMessage(getLangText(ERROR_TEXT), (char*)g_spareBuffer, (3 * SOFT_SECS));

			while (intMem < (uint16*)0x10000)
			{
				*intMem++ = 0xD673;
			}
			break;
		}

		intMem++;
	}
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InternalMemoryFill(void)
{
	uint16* intMem = (uint16*)0x0004;
	while (intMem < (uint16*)0x10000)
	{
		// Ignore the top of the stack
		if ((uint32)intMem < 0x3F00 || (uint32)intMem >= 0x4000)
		{
			*intMem = 0xD673;
		}

		intMem++;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void EnableGlobalException(void)
{
	// Import the Exception Vector Base Address.
	extern void _evba;

	Set_system_register(AVR32_EVBA, (int)&_evba);

	// Enable exceptions.
	Enable_global_exception();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
int main(void)
{
    // Initialize the system
	InternalMemoryFill();
	InitSystemHardware_NS8100();
	InitInterrupts_NS8100();
	InitSoftwareSettings_NS8100();

	BootLoadManager();
	DisplayVersionToCraft();
	EnableGlobalException();

 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
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
		PowerManager();

		// Count Exec cycles
		g_execCycles++;
	}    
	// End of NS8100 Main

	// End of the world
	return (0);
}
