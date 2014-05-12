///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: SoftTimer.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:00 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/SoftTimer.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _SOFT_TIMER_H_
#define _SOFT_TIMER_H_ 

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define TIMER_UNASSIGNED 0
#define TIMER_ASSIGNED 1 

enum{
	DISPLAY_ON_OFF_TIMER_NUM = 0,
	LCD_PW_ON_OFF_TIMER_NUM,
	AUTO_MONITOR_TIMER_NUM,
	MENU_UPDATE_TIMER_NUM,
	ALARM_ONE_OUTPUT_TIMER_NUM,
	ALARM_TWO_OUTPUT_TIMER_NUM,
	POWER_OFF_TIMER_NUM,
	MODEM_DELAY_TIMER_NUM,
	MODEM_RESET_TIMER_NUM,
	// Add new timers here
	NUM_OF_SOFT_TIMERS
};

#define DEACTIVATED 0
#define ACTIVATED 1

#define TICKS_PER_SEC 	2 // Number of ticks per second
#define TICKS_PER_MIN	(TICKS_PER_SEC * 60) // Number of ticks per minute
#define TICKS_PER_HOUR	(TICKS_PER_MIN * 60) // Number of ticks per hour

#define ONE_SECOND_TIMEOUT		1 * TICKS_PER_SEC // secs * ticks

#define TIMEOUT_DISABLED		0 // secs
#define LCD_BACKLIGHT_TIMEOUT	60 * TICKS_PER_SEC  // secs * ticks
#define LCD_POWER_TIMEOUT		120 * TICKS_PER_SEC // secs * ticks
#define MODEM_ATZ_DELAY			15 * TICKS_PER_SEC
#define MODEM_ATZ_QUICK_DELAY	2 * TICKS_PER_SEC

typedef struct
{
	uint32 	state;		// The timer is in use or not.
	uint32  tickStart;	// Tick value when turned on.		
	uint32 	timePeriod;	// The time period to wait, number of ticks.
	void (*callback)(void);

} SOFT_TIMER_STRUCT;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void assignSoftTimer(uint16, uint32, void*);
void clearSoftTimer(uint16);
void checkSoftTimers(void);
void procTimerEvents(void);
void processTimerMode(void);
void handleMidnightEvent(void);
BOOLEAN timerModeActiveCheck(void);
void resetTimeOfDayAlarm(void);
void handleUserPowerOffDuringTimerMode(void);
void displayTimerCallBack(void);
void lcdPwTimerCallBack(void);
void autoMonitorTimerCallBack(void);
void menuUpdateTimerCallBack(void);
void alarmOneOutputTimerCallback(void);
void alarmTwoOutputTimerCallback(void);
void powerOffTimerCallback(void);
void modemDelayTimerCallback(void);
void modemResetTimerCallback(void);
void autoCalInWaveformTimerCallback(void);

#endif // _SOFT_TIMER_H_
