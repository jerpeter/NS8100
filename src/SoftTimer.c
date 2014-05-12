///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: SoftTimer.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:00 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/SoftTimer.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
#include "RealTimeClock.h"
#include "MMC2114.h"
#include "Menu.h"
#include "SoftTimer.h"
#include "Display.h"
#include "PowerManagement.h"
#include "InitDataBuffers.h"
#include "SysEvents.h"
#include "Old_Board.h"
#include "Uart.h"
#include "Keypad.h"
#include "RemoteModem.h"
#include "ProcessBargraph.h"
#include "TextTypes.h"
#include "EventProcessing.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern SYS_EVENT_STRUCT SysEvents_flags;
extern MN_TIMER_STRUCT mn_timer;
extern uint8 g_doneTakingEvents;
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
extern uint16 manual_cal_flag;
extern uint16 g_autoRetries;
extern MODEM_SETUP_STRUCT modem_setup_rec;
extern uint8 g_autoDialoutState;
extern SOFT_TIMER_STRUCT g_rtcTimerBank[NUM_OF_SOFT_TIMERS];
extern uint32 g_rtcSoftTimerTickCount;
extern int8 g_lcdBacklightFlag;
extern int8 g_lcdPowerFlag;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern uint8 g_sampleProcessing;
extern REC_HELP_MN_STRUCT help_rec;
extern REC_EVENT_MN_STRUCT trig_rec;
extern MODEM_STATUS_STRUCT g_ModemStatus;
extern int32 active_menu;
extern uint8 modemResetStage;
extern uint8 autoCalDaysToWait;
extern uint16 g_CRLF;
extern uint8 enterMonitorModeAfterMidnightCal;
extern REC_HELP_MN_STRUCT help_rec;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------

/****************************************
*	Function:    assignSoftTimer
*	Purpose:
****************************************/
void assignSoftTimer(uint16 timerNum, uint32 timeout, void* callback)
{
	if (timerNum >= NUM_OF_SOFT_TIMERS)
	{
		debugErr("assignSoftTimer Error: Timer Number not valid: %d\n", timerNum);
		return;
	}

	if (timeout > 0)
	{
		g_rtcTimerBank[timerNum].state = TIMER_ASSIGNED;
		g_rtcTimerBank[timerNum].tickStart = g_rtcSoftTimerTickCount;
		g_rtcTimerBank[timerNum].timePeriod = g_rtcSoftTimerTickCount + timeout;
		g_rtcTimerBank[timerNum].callback = callback;
	}

	return;
}

/****************************************
*	Function:    clearSoftTimer
*	Purpose:
****************************************/
void clearSoftTimer(uint16 timerNum)
{
	if (timerNum >= NUM_OF_SOFT_TIMERS)
	{
		debugErr("clearSoftTimer Error: Timer Number not valid: %d\n", timerNum);
		return;
	}

	g_rtcTimerBank[timerNum].state = TIMER_UNASSIGNED;
	g_rtcTimerBank[timerNum].tickStart = 0;
	g_rtcTimerBank[timerNum].timePeriod = 0;
	g_rtcTimerBank[timerNum].callback = NULL;
}

/****************************************
*	Function:    checkSoftTimers
*	Purpose:
****************************************/
void checkSoftTimers(void)
{
	uint16 softTimerIndex;

	// Loop for our of our timers.
	for (softTimerIndex = 0; softTimerIndex < NUM_OF_SOFT_TIMERS; softTimerIndex++)
	{
		// Check if it is in use.
		if (TIMER_ASSIGNED == g_rtcTimerBank[softTimerIndex].state)
		{
			// Did our timer loop or wrap around the max?
			if (g_rtcTimerBank[softTimerIndex].tickStart > g_rtcTimerBank[softTimerIndex].timePeriod)
			{
				// We looped but did the tick count loop around the MAX?
				if (g_rtcSoftTimerTickCount < g_rtcTimerBank[softTimerIndex].tickStart)
				{
					// The counter looped so check the condition of the time period.
					if (g_rtcSoftTimerTickCount >= g_rtcTimerBank[softTimerIndex].timePeriod)
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
							debug("checkSoftTimers:Error function call is NULL\n");
						}
					}
				}
			}

			// The timer did not loop. But did our tick count loop?
			else
			{
				// Did the tick count loop around the MAX? Or did we go past the time?
				if ((g_rtcSoftTimerTickCount <  g_rtcTimerBank[softTimerIndex].tickStart	) ||
				   (g_rtcSoftTimerTickCount >= g_rtcTimerBank[softTimerIndex].timePeriod))
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
						debug("checkSoftTimers:Error function call is NULL\n");
					}

				}
			}
		}
	}

	return;
}

/****************************************
*	Function:    displayTimerCallBack
*	Purpose:
****************************************/
void displayTimerCallBack(void)
{
	debug("LCD Backlight Timer callback: activated.\n");

	g_lcdBacklightFlag = DISABLED;
	setLcdBacklightState(BACKLIGHT_OFF);
}

/****************************************
*	Function:    alarmOneOutputTimerCallback
*	Purpose:
****************************************/
void alarmOneOutputTimerCallback(void)
{
#if 0 // fix_ns8100
	// Deactivate alarm 1 signal
	reg_TIM2PORT.reg &= 0x0B;
#endif
}

/****************************************
*	Function:    alarmTwoOutputTimerCallback
*	Purpose:
****************************************/
void alarmTwoOutputTimerCallback(void)
{
#if 0 // fix_ns8100
	// Deactivate alarm 2 signal
	reg_TIM2PORT.reg &= 0x07;
#endif
}

/****************************************
*	Function:    lcdPwTimerCallBack
*	Purpose:
****************************************/
void lcdPwTimerCallBack(void)
{
	debug("LCD Power Timer callback: activated.\n");

	g_lcdPowerFlag = DISABLED;

	powerControl(LCD_CONTRAST_ENABLE, OFF);
	clearLcdDisplay();
	clearControlLinesLcdDisplay();
	powerControl(LCD_POWER_ENABLE, OFF);

	if (g_sampleProcessing == SAMPLING_STATE)
	{
		debug("LCD Power Timer callback: disabling Monitor Update Timer.\n");
		clearSoftTimer(MENU_UPDATE_TIMER_NUM);
	}
}

/****************************************
*	Function:    monitorUpdateTimerCallback
*	Purpose:	 update the LCD to show updated time and battery voltage
****************************************/
void menuUpdateTimerCallBack(void)
{
	INPUT_MSG_STRUCT mn_msg;

	// Clear out the message parameters
	mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;

	// Recall the current active menu to repaint the display
	(*menufunc_ptrs[active_menu]) (mn_msg);

	assignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, menuUpdateTimerCallBack);
}

/****************************************
*	Function:    powerOffTimerCallBack
*	Purpose:
****************************************/
void powerOffTimerCallback(void)
{
	debug("Power Off Timer callback: activated.\n");

	// Handle stopping 430 communication and finishing any processing
	stopMonitoring(trig_rec.op_mode, FINISH_PROCESSING);

	overlayMessage(getLangText(TIMER_MODE_TEXT), getLangText(POWERING_UNIT_OFF_NOW_TEXT), 2 * SOFT_SECS);

	// Re-Enable the Power Off key
	powerControl(POWER_SHUTDOWN_ENABLE, ON);

	// Power the unit off
	debug("Timer mode: Finished for the day, sleep time.\n");

	PowerUnitOff(SHUTDOWN_UNIT);
}

/****************************************
*	Function:    modemDelayTimerCallback
*	Purpose:
****************************************/
void modemDelayTimerCallback(void)
{
#if 0 // fix_ns8100
	if (YES == g_ModemStatus.modemAvailable)
	{
		if ((READ_DSR == MODEM_CONNECTED) && (READ_DCD == NO_CONNECTION))
		{
			modemInitProcess();
		}
		else
		{
			assignSoftTimer(MODEM_DELAY_TIMER_NUM, MODEM_ATZ_DELAY, modemDelayTimerCallback);
		}
	}
#endif
}

/****************************************
*	Function:    autoCalInWaveformTimerCallback
*	Purpose:
****************************************/
void autoCalInWaveformTimerCallback(void)
{
	handleManualCalibration();
}

/****************************************
*	Function:    modemResetTimerCallback
*	Purpose:
****************************************/
void modemResetTimerCallback(void)
{
	if (modemResetStage == 0)
	{
		// If for some reason this executes, make sure the timer is disabled
		clearSoftTimer(MODEM_RESET_TIMER_NUM);
	}
	else if (modemResetStage == 1)
	{
#if 0 // fix_ns8100
		SET_DTR;
#endif
		assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), modemResetTimerCallback);
		modemResetStage = 2;
	}
	else if (modemResetStage == 2)
	{
		uart_puts((char*)(CMDMODE_CMD_STRING), CRAFT_COM_PORT);
		assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), modemResetTimerCallback);
		modemResetStage = 3;
	}
	else if (modemResetStage == 3)
	{
		uart_puts((char*)(CMDMODE_CMD_STRING), CRAFT_COM_PORT);
		assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), modemResetTimerCallback);
		modemResetStage = 4;
	}
	else if (modemResetStage == 4)
	{
		uart_puts((char*)(ATH_CMD_STRING), CRAFT_COM_PORT);
		uart_puts((char*)&g_CRLF, CRAFT_COM_PORT);
		assignSoftTimer(MODEM_RESET_TIMER_NUM, (uint32)(3 * TICKS_PER_SEC), modemResetTimerCallback);
		modemResetStage = 5;
	}
	else if (modemResetStage == 5)
	{
		modemInitProcess();
		modemResetStage = 0;
	}
}

/****************************************
*	Function:    autoMonitorTimerCallBack
*	Purpose:
****************************************/
void autoMonitorTimerCallBack(void)
{
	INPUT_MSG_STRUCT mn_msg;

	debug("Auto Monitor Timer callback: activated.\n");

	// Make sure the Auto Monitor timer is disabled
	clearSoftTimer(AUTO_MONITOR_TIMER_NUM);

	// Check if the unit is not alread monitoring
	if (g_sampleProcessing != SAMPLING_STATE)
	{
		// Enter monitor mode with the current mode
		active_menu = MONITOR_MENU;
		ACTIVATE_MENU_WITH_DATA_MSG(trig_rec.op_mode);
		(*menufunc_ptrs[active_menu]) (mn_msg);
	}
}

/****************************************
*	Function:    procTimerEvents(void)
*	Purpose:
****************************************/
void procTimerEvents(void)
{
	DATE_TIME_STRUCT currentTime = getCurrentTime();
	static uint8 processingMidnight = NO;

	// Check for Midnight and make sure that the current Midnight isn't already been processed
	if ((currentTime.hour == 0) && (currentTime.min == 0) && (processingMidnight == NO))
	{
		processingMidnight = YES;
		raiseSystemEventFlag(MIDNIGHT_EVENT);
	}
	else if (processingMidnight == YES)
	{
		if ((currentTime.hour != 0) && (currentTime.min != 0))
		{
			processingMidnight = NO;
		}
	}

#if 0
	INPUT_MSG_STRUCT mn_msg;
	char msgBuffer[25];

	// fix_ns8100 - Battery voltage not reading correctly
	// Check if the unit is in monitor mode and the battery voltage has dropped below 5 volts
	if ((g_sampleProcessing == SAMPLING_STATE) && (convertedBatteryLevel(BATTERY_VOLTAGE) < 5.0))
	{
		// Disable the monitor menu update timer
		clearSoftTimer(MENU_UPDATE_TIMER_NUM);

		// Need to stop print jobs

		// Voltage is low, turn printing off
		help_rec.auto_print = OFF;

		// Handle stopping 430 communication
		stopMonitoring(trig_rec.op_mode, FINISH_PROCESSING);

		// Clear and setup the message buffer
		byteSet(&(msgBuffer[0]), 0, sizeof(msgBuffer));
		sprintf(msgBuffer, "%s %s", getLangText(BATTERY_VOLTAGE_TEXT), getLangText(LOW_TEXT));

		// Overlay a "Battery Voltage Low" message
		overlayMessage(getLangText(WARNING_TEXT), msgBuffer, (2 * SOFT_SECS));

		// Jump to the main menu
		active_menu = MAIN_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[active_menu]) (mn_msg);
	}
#endif
}

/****************************************
*	Function:   handleMidnightEvent
*	Purpose:	Handle the operations that need to occur at or after midnight
****************************************/
void handleMidnightEvent(void)
{
	INPUT_MSG_STRUCT mn_msg;
	char message[50];
	FLASH_USAGE_STRUCT flashStats;

	if ((trig_rec.op_mode == BARGRAPH_MODE) && (g_sampleProcessing == SAMPLING_STATE))
	{
		// Do not handle midnight calibration since a manual cal is forced at the beginning of Bargraph

		// Overlay a message that the current bargraph has ended
		byteSet(&message[0], 0, sizeof(message));
		sprintf(message, "%s %s", getLangText(MONITOR_BARGRAPH_TEXT), getLangText(END_TEXT));
		overlayMessage(getLangText(STATUS_TEXT), message, 0);

		// Handle stopping the current bargraph
		stopMonitoring(trig_rec.op_mode, FINISH_PROCESSING);

		// Start up a new Bargraph
		active_menu = MONITOR_MENU;
		ACTIVATE_MENU_WITH_DATA_MSG(trig_rec.op_mode);
		(*menufunc_ptrs[active_menu]) (mn_msg);
	}
	// Check if Auto Cal is active
	else if (help_rec.auto_cal_mode != AUTO_NO_CAL_TIMEOUT)
	{
		// Decrement days to wait
		if (autoCalDaysToWait > 0) autoCalDaysToWait--;

		// Check if time to do Auto Calibration
		if (autoCalDaysToWait == 0)
		{
			// Perform Auto Cal logic

			// Reset the days to wait count
			switch (help_rec.auto_cal_mode)
			{
				case AUTO_24_HOUR_TIMEOUT: autoCalDaysToWait = 1; break;
				case AUTO_48_HOUR_TIMEOUT: autoCalDaysToWait = 2; break;
				case AUTO_72_HOUR_TIMEOUT: autoCalDaysToWait = 3; break;
			}

			// Set flag to alert the Results menu which action to take after a cal pulse result is received
			if (g_sampleProcessing == SAMPLING_STATE)
				enterMonitorModeAfterMidnightCal = YES;

			// Handle stopping 430 communication and finishing any processing
			stopMonitoring(trig_rec.op_mode, FINISH_PROCESSING);

			getFlashUsageStats(&flashStats);

			if ((help_rec.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
			{
				enterMonitorModeAfterMidnightCal = NO;

				// Unable to store any more data in the selected mode
				active_menu = MAIN_MENU;
				ACTIVATE_MENU_MSG();
				(*menufunc_ptrs[active_menu]) (mn_msg);

				overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
			}
			else
			{
				// Overlay a message that calibration is taking place
				overlayMessage(getLangText(STATUS_TEXT), getLangText(CALIBRATING_TEXT), (2 * SOFT_SECS));

				// Issue a Cal Pulse message
	            startMonitoring(trig_rec.trec, MANUAL_CAL_PULSE_CMD, MANUAL_CAL_MODE);

				// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
				soft_usecWait(250 * SOFT_MSECS);

				// Stop data transfer
	            stopDataCollection();

				// Done processing Auto Cal logic at this point, just return
				// Results menu will pick up the Cal event and re-enter monitor mode
			}
		}
	}

	// Check if Aut Dialout processing is not active
	if (g_autoDialoutState == AUTO_DIAL_IDLE)
	{
		// At midnight reset the modem (to better handle problems with USR modems)
		modemResetProcess();
	}
}

