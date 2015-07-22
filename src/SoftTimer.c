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
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
#include "RealTimeClock.h"
#include "Menu.h"
#include "SoftTimer.h"
#include "Display.h"
#include "PowerManagement.h"
#include "InitDataBuffers.h"
#include "SysEvents.h"
#include "Uart.h"
#include "Keypad.h"
#include "RemoteOperation.h"
#include "ProcessBargraph.h"
#include "TextTypes.h"
#include "EventProcessing.h"
#include "M23018.h"

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
///	Function Break
///----------------------------------------------------------------------------
uint8 IsSoftTimerActive(uint16 timerNum)
{
	if ((timerNum < NUM_OF_SOFT_TIMERS) && (g_rtcTimerBank[timerNum].state == TIMER_ASSIGNED))
	{
		return YES;
	}

	return NO;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AssignSoftTimer(uint16 timerNum, uint32 timeout, void* callback)
{
	if (timerNum >= NUM_OF_SOFT_TIMERS)
	{
		debugErr("AssignSoftTimer Error: Timer Number not valid: %d\r\n", timerNum);
		return;
	}

	// Check that the timeout condition is set for some time in the future
	if (timeout > 0)
	{
		g_rtcTimerBank[timerNum].state = TIMER_ASSIGNED;
		g_rtcTimerBank[timerNum].tickStart = g_lifetimeHalfSecondTickCount;
		g_rtcTimerBank[timerNum].timePeriod = g_lifetimeHalfSecondTickCount + timeout;
		g_rtcTimerBank[timerNum].timeoutValue = timeout;
		g_rtcTimerBank[timerNum].callback = callback;
	}
	else // Timeout is zero, go ahead and clear the timer just in case it's already set/active
	{
		ClearSoftTimer(timerNum);
	}

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ResetSoftTimer(uint16 timerNum)
{
	if (timerNum >= NUM_OF_SOFT_TIMERS)
	{
		debugErr("AssignSoftTimer Error: Timer Number not valid: %d\r\n", timerNum);
		return;
	}

	if (g_rtcTimerBank[timerNum].state == TIMER_ASSIGNED)
	{
		g_rtcTimerBank[timerNum].tickStart = g_lifetimeHalfSecondTickCount;
		g_rtcTimerBank[timerNum].timePeriod = g_lifetimeHalfSecondTickCount + g_rtcTimerBank[timerNum].timeoutValue;
	}

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ClearSoftTimer(uint16 timerNum)
{
	if (timerNum >= NUM_OF_SOFT_TIMERS)
	{
		debugErr("ClearSoftTimer Error: Timer Number not valid: %d\r\n", timerNum);
		return;
	}

	g_rtcTimerBank[timerNum].state = TIMER_UNASSIGNED;
	g_rtcTimerBank[timerNum].tickStart = 0;
	g_rtcTimerBank[timerNum].timePeriod = 0;
	g_rtcTimerBank[timerNum].timeoutValue = 0;
	g_rtcTimerBank[timerNum].callback = NULL;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CheckSoftTimers(void)
{
	uint16 softTimerIndex;

	// Loop for our of our timers.
	for (softTimerIndex = 0; softTimerIndex < NUM_OF_SOFT_TIMERS; softTimerIndex++)
	{
		// Check if it is in use.
		if (g_rtcTimerBank[softTimerIndex].state == TIMER_ASSIGNED)
		{
			// Did our timer loop or wrap around the max?
			if (g_rtcTimerBank[softTimerIndex].tickStart > g_rtcTimerBank[softTimerIndex].timePeriod)
			{
				// We looped but did the tick count loop around the MAX?
				if (g_lifetimeHalfSecondTickCount < g_rtcTimerBank[softTimerIndex].tickStart)
				{
					// The counter looped so check the condition of the time period.
					if (g_lifetimeHalfSecondTickCount >= g_rtcTimerBank[softTimerIndex].timePeriod)
					{
						// Once the timer has activated, clear the timer state and other values
						g_rtcTimerBank[softTimerIndex].state = TIMER_UNASSIGNED;
						g_rtcTimerBank[softTimerIndex].tickStart = 0;
						g_rtcTimerBank[softTimerIndex].timePeriod = 0;

						// Process the callback, or func is null and an error.
						if (NULL != g_rtcTimerBank[softTimerIndex].callback)
						{
							(*(g_rtcTimerBank[softTimerIndex].callback)) ();
						}
						else
						{
							debug("CheckSoftTimers:Error function call is NULL\r\n");
						}
					}
				}
			}
			// The timer did not loop. But did our tick count loop?
			else
			{
				// Did the tick count loop around the MAX? Or did we go past the time?
				if ((g_lifetimeHalfSecondTickCount < g_rtcTimerBank[softTimerIndex].tickStart) ||
					(g_lifetimeHalfSecondTickCount >= g_rtcTimerBank[softTimerIndex].timePeriod))
				{
					// Once the timer has activated, clear the timer state and other values
					g_rtcTimerBank[softTimerIndex].state = TIMER_UNASSIGNED;
					g_rtcTimerBank[softTimerIndex].tickStart = 0;
					g_rtcTimerBank[softTimerIndex].timePeriod = 0;

					// Process the callback, or func is null and an error.
					if (NULL != g_rtcTimerBank[softTimerIndex].callback)
					{
						(*(g_rtcTimerBank[softTimerIndex].callback)) ();
					}
					else
					{
						debug("CheckSoftTimers:Error function call is NULL\r\n");
					}
				}
			}
		}
	}

	return;
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void DisplayTimerCallBack(void)
{
	debug("LCD Backlight Timer callback: activated.\r\n");

	g_lcdBacklightFlag = DISABLED;
	SetLcdBacklightState(BACKLIGHT_OFF);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AlarmOneOutputTimerCallback(void)
{
	// Deactivate alarm 1 signal
	PowerControl(ALARM_1_ENABLE, OFF);

	debug("Warning Event 1 Alarm finished\r\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AlarmTwoOutputTimerCallback(void)
{
	// Deactivate alarm 2 signal
	PowerControl(ALARM_2_ENABLE, OFF);

	debug("Warning Event 2 Alarm finished\r\n");
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void LcdPwTimerCallBack(void)
{
	debug("LCD Power Timer callback: activated.\r\n");

	g_lcdPowerFlag = DISABLED;

	PowerControl(LCD_CONTRAST_ENABLE, OFF);
	ClearLcdDisplay();
	ClearControlLinesLcdDisplay();
	LcdClearPortReg();
	PowerControl(LCD_POWER_ENABLE, OFF);

	if (g_sampleProcessing == ACTIVE_STATE)
	{
		debug("LCD Power Timer callback: disabling Monitor Update Timer.\r\n");
		ClearSoftTimer(MENU_UPDATE_TIMER_NUM);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MenuUpdateTimerCallBack(void)
{
	INPUT_MSG_STRUCT mn_msg;

	// Clear out the message parameters
	mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;

	// Recall the current active menu to repaint the display
	JUMP_TO_ACTIVE_MENU();

	AssignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, MenuUpdateTimerCallBack);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void KeypadLedUpdateTimerCallBack(void)
{
	static uint8 ledState = KEYPAD_LED_STATE_UNKNOWN;
	uint8 lastLedState;
	uint8 config; // = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
	BOOLEAN externalChargePresent = CheckExternalChargeVoltagePresent();

	// States
	// 1) Init complete, not monitoring, not charging --> Static Green
	// 2) Init complete, not monitoring, charging --> Static Red
	// 3) Init complete, monitoring, not charging --> Flashing Green (state transition)
	// 4) Init complete, monitoring, charging --> Alternate Flashing Green/Red (state transition)

	// Hold the last state
	lastLedState = ledState;

	if ((g_sampleProcessing == IDLE_STATE) && (externalChargePresent == FALSE))
	{
		//debug("Keypad LED: State 1\r\n");
		
		ledState = KEYPAD_LED_STATE_IDLE_GREEN_ON;
	}
	else if ((g_sampleProcessing == IDLE_STATE) && (externalChargePresent == TRUE))
	{
		//debug("Keypad LED: State 2\r\n");

		ledState = KEYPAD_LED_STATE_CHARGE_RED_ON;
	}
	else if ((g_sampleProcessing == ACTIVE_STATE) && (externalChargePresent == FALSE))
	{
		//debug("Keypad LED: State 3\r\n");

		if (ledState == KEYPAD_LED_STATE_ACTIVE_GREEN_ON)
		{
			ledState = KEYPAD_LED_STATE_ACTIVE_GREEN_OFF;
		}
		else
		{
			ledState = KEYPAD_LED_STATE_ACTIVE_GREEN_ON;
		}
	}		
	else // ((g_sampleProcessing == ACTIVE_STATE) && (externalChargePresent == TRUE))
	{
		//debug("Keypad LED: State 4\r\n");

		if (ledState == KEYPAD_LED_STATE_ACTIVE_CHARGE_GREEN_ON)
		{
			ledState = KEYPAD_LED_STATE_ACTIVE_CHARGE_RED_ON;
		}
		else
		{
			ledState = KEYPAD_LED_STATE_ACTIVE_CHARGE_GREEN_ON;
		}
	}
	
	// Check if the state changed
	if (ledState != lastLedState)
	{
		config = ReadMcp23018(IO_ADDRESS_KPD, GPIOA);
		
		switch (ledState)
		{
			case KEYPAD_LED_STATE_BOTH_OFF:
			case KEYPAD_LED_STATE_ACTIVE_GREEN_OFF:
				config &= ~RED_LED_PIN;
				config &= ~GREEN_LED_PIN;
				break;
				
			case KEYPAD_LED_STATE_IDLE_GREEN_ON:
			case KEYPAD_LED_STATE_ACTIVE_GREEN_ON:
			case KEYPAD_LED_STATE_ACTIVE_CHARGE_GREEN_ON:
				config &= ~RED_LED_PIN;
				config |= GREEN_LED_PIN;
				break;
				
			case KEYPAD_LED_STATE_CHARGE_RED_ON:
			case KEYPAD_LED_STATE_ACTIVE_CHARGE_RED_ON:
				config &= ~GREEN_LED_PIN;
				config |= RED_LED_PIN;
				break;
		}

		WriteMcp23018(IO_ADDRESS_KPD, GPIOA, config);
	}

	AssignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, KeypadLedUpdateTimerCallBack);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerOffTimerCallback(void)
{
	debug("Power Off Timer callback: activated.\r\n");

	// Handle and finish any processing
	StopMonitoring(g_triggerRecord.opMode, FINISH_PROCESSING);

	if (g_timerModeLastRun == YES)
	{
		debug("Timer Mode: Ending last session, now disabling...\r\n");
		g_unitConfig.timerMode = DISABLED;

		// Save Unit Config (also covers LCD contrast change case)
		SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
	}
	else if (g_lcdContrastChanged == YES)
	{
		// Save Unit Config here to prevent constant saving on LCD contrast adjustment
		SaveRecordData(&g_unitConfig, DEFAULT_RECORD, REC_UNIT_CONFIG_TYPE);
	}
		
	OverlayMessage(getLangText(TIMER_MODE_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), 3 * SOFT_SECS);

	// Power the unit off
	debug("Timer mode: Finished for the day, sleep time.\r\n");

	PowerUnitOff(SHUTDOWN_UNIT);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ModemDelayTimerCallback(void)
{
	if (YES == g_modemStatus.modemAvailable)
	{
		if ((READ_DSR == MODEM_CONNECTED) && (READ_DCD == NO_CONNECTION))
		{
			ModemInitProcess();
		}
		else
		{
			AssignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_DELAY, ModemDelayTimerCallback);
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AutoCalInWaveformTimerCallback(void)
{
	HandleManualCalibration();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ModemResetTimerCallback(void)
{
	if (g_modemResetStage == 0)
	{
		// If for some reason this executes, make sure the timer is disabled
		ClearSoftTimer(MODEM_RESET_TIMER_NUM);
	}
	else if (g_modemResetStage == 1)
	{
		SET_DTR;

		AssignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), ModemResetTimerCallback);
		g_modemResetStage = 2;
	}
	else if (g_modemResetStage == 2)
	{
		UartPuts((char*)(CMDMODE_CMD_STRING), CRAFT_COM_PORT);
		AssignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), ModemResetTimerCallback);
		g_modemResetStage = 3;
	}
	else if (g_modemResetStage == 3)
	{
		UartPuts((char*)(CMDMODE_CMD_STRING), CRAFT_COM_PORT);
		AssignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), ModemResetTimerCallback);
		g_modemResetStage = 4;
	}
	else if (g_modemResetStage == 4)
	{
		UartPuts((char*)(ATH_CMD_STRING), CRAFT_COM_PORT);
		UartPuts((char*)&g_CRLF, CRAFT_COM_PORT);
		AssignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), ModemResetTimerCallback);
		g_modemResetStage = 5;
	}
	else if (g_modemResetStage == 5)
	{
		ModemInitProcess();
		g_modemResetStage = 0;
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void AutoMonitorTimerCallBack(void)
{
	INPUT_MSG_STRUCT mn_msg;

	debug("Auto Monitor Timer callback: activated.\r\n");

	// Check if the USB is currently handling an active connection
	if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
	{
		AssignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_unitConfig.autoMonitorMode * TICKS_PER_MIN), AutoMonitorTimerCallBack);
	}
	else
	{
		// Make sure the Auto Monitor timer is disabled
		ClearSoftTimer(AUTO_MONITOR_TIMER_NUM);

		// Check if the unit is not already monitoring
		if (g_sampleProcessing != ACTIVE_STATE)
		{
			if ((g_factorySetupRecord.invalid) || (g_lowBatteryState == YES)) { PromptUserUnableToEnterMonitoring(); }
			else // Safe to enter monitor mode
			{
				// Enter monitor mode with the current mode
				SETUP_MENU_WITH_DATA_MSG(MONITOR_MENU, g_triggerRecord.opMode);
				JUMP_TO_ACTIVE_MENU();
			}
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CheckForMidnight(void)
{
	static uint8 processingMidnight = NO;
	DATE_TIME_STRUCT currentTime = GetCurrentTime();

	// Check for Midnight and make sure that the current Midnight isn't already been processed
	if ((currentTime.hour == 0) && (currentTime.min == 0) && (processingMidnight == NO))
	{
		// Check that the unit has been on for 1 minute before processing a midnight event (especially in the case of powering up at midnight)
		if (g_lifetimeHalfSecondTickCount > (1 * TICKS_PER_MIN))
		{
			processingMidnight = YES;
			raiseSystemEventFlag(MIDNIGHT_EVENT);
		}
	}
	else if (processingMidnight == YES)
	{
		if ((currentTime.hour != 0) && (currentTime.min != 0))
		{
			processingMidnight = NO;
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
extern void StartDataCollection(uint32 sampleRate);
extern void GetManualCalibration(void);
void HandleMidnightEvent(void)
{
	INPUT_MSG_STRUCT mn_msg;
	char message[50];

	// Check if actively monitoring
	if (g_sampleProcessing == ACTIVE_STATE)
	{
		// Check if Bargraph or Combo and actively monitoring
		if ((g_triggerRecord.opMode == BARGRAPH_MODE) || (g_triggerRecord.opMode == COMBO_MODE))
		{
			// Do not handle midnight calibration since a manual cal is forced at the beginning of Bargraph and Combo

			// Overlay a message that the current Bargraph or Combo has ended
			memset(&message[0], 0, sizeof(message));
			sprintf(message, "%s %s", (g_triggerRecord.opMode == BARGRAPH_MODE) ? (getLangText(BARGRAPH_MODE_TEXT)) : (getLangText(COMBO_MODE_TEXT)), getLangText(END_TEXT));
			OverlayMessage(getLangText(STATUS_TEXT), message, 0);

			// Handle stopping the current Bargraph or Combo
			StopMonitoring(g_triggerRecord.opMode, FINISH_PROCESSING);

			// Start up a new Bargraph or Combo
			SETUP_MENU_WITH_DATA_MSG(MONITOR_MENU, g_triggerRecord.opMode);
			JUMP_TO_ACTIVE_MENU();
		}
		else if (g_triggerRecord.opMode == WAVEFORM_MODE)
		{
			// Check if not busy processing an event, otherwise handling a manual cal or event which would be accompanied by a cal pulse so skip midnight cal
			if (g_busyProcessingEvent == NO)
			{
				// Make sure there is room to store a Manual calibration event
				if ((g_unitConfig.flashWrapping == NO) && (g_sdCardUsageStats.manualCalsLeft == 0))
				{
					sprintf((char*)g_spareBuffer, "%s (%s %s) %s %s", getLangText(FLASH_MEMORY_IS_FULL_TEXT), getLangText(WRAPPING_TEXT), getLangText(DISABLED_TEXT),
							getLangText(CALIBRATION_TEXT), getLangText(UNAVAILABLE_TEXT));
					OverlayMessage(getLangText(WARNING_TEXT), (char*)g_spareBuffer, (5 * SOFT_SECS));
				}
				else // Handle midnight cal while in Waveform mode
				{
					// Stop data transfer
					StopDataClock();

					// Perform Cal while in monitor mode
					OverlayMessage(getLangText(STATUS_TEXT), getLangText(PERFORMING_CALIBRATION_TEXT), 0);

					GetManualCalibration();

					InitDataBuffs(g_triggerRecord.opMode);

					// Make sure to reset the gain after the Cal pulse (which is generated at low sensitivity)
					if (g_triggerRecord.srec.sensitivity == HIGH)
					{
						SetSeismicGainSelect(SEISMIC_GAIN_HIGH);
					}

					StartDataCollection(g_triggerRecord.trec.sample_rate);
				}
			}
		}
	}
	// Check if Auto Calibration at midnight is active (any value but zero)
	else if (g_unitConfig.autoCalMode) // != AUTO_NO_CAL_TIMEOUT
	{
		// Decrement days to wait
		if (g_autoCalDaysToWait > 0) g_autoCalDaysToWait--;

		// Check if time to do Auto Calibration
		if (g_autoCalDaysToWait == 0)
		{
			// Perform Auto Cal logic

			// Reset the days to wait count
			switch (g_unitConfig.autoCalMode)
			{
				case AUTO_24_HOUR_TIMEOUT: g_autoCalDaysToWait = 1; break;
				case AUTO_48_HOUR_TIMEOUT: g_autoCalDaysToWait = 2; break;
				case AUTO_72_HOUR_TIMEOUT: g_autoCalDaysToWait = 3; break;
			}

			// If the user was in the Summary list menu, reset the global
			g_summaryListMenuActive = NO;

			if ((g_unitConfig.flashWrapping == NO) && (g_sdCardUsageStats.manualCalsLeft == 0))
			{
				g_enterMonitorModeAfterMidnightCal = NO;

				// Unable to store any more data in the selected mode
				SETUP_MENU_MSG(MAIN_MENU);
				JUMP_TO_ACTIVE_MENU();

				sprintf((char*)g_spareBuffer, "%s (%s %s) %s %s", getLangText(FLASH_MEMORY_IS_FULL_TEXT), getLangText(WRAPPING_TEXT), getLangText(DISABLED_TEXT),
						getLangText(CALIBRATION_TEXT), getLangText(UNAVAILABLE_TEXT));
				OverlayMessage(getLangText(WARNING_TEXT), (char*)g_spareBuffer, (5 * SOFT_SECS));
			}
			else
			{
				// Perform Midnight calibration (outside of active monitoring)
				OverlayMessage(getLangText(STATUS_TEXT), getLangText(PERFORMING_CALIBRATION_TEXT), 0);

				GetManualCalibration();
			}
		}
	}

	// Check if Auto Dialout processing is not active
	if (g_autoDialoutState == AUTO_DIAL_IDLE)
	{
		// At midnight reset the modem (to better handle problems with USR modems)
		g_autoRetries = 0;
		ModemResetProcess();
	}
}

