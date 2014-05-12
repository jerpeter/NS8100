///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: SysEvents.h,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:01 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/SysEvents.h,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#ifndef _SYS_EVENT_H_
#define _SYS_EVENT_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
typedef struct
{
	uint16 wrd;
} SYS_EVENT_STRUCT;

typedef struct
{
	uint16 wrd;
} MN_TIMER_STRUCT;

typedef struct
{
	uint16 wrd;
} MN_EVENT_STRUCT;

// System Events
#define TRIGGER_EVENT		0x0001
#define BARGRAPH_EVENT		0x0002
#define COMBO_EVENT			0x0004
#define POWER_OFF_EVENT		0x0008
#define KEYPAD_EVENT		0x0010
#define CYCLIC_EVENT		0x0020
#define MIDNIGHT_EVENT		0x0040
#define CAL_EVENT			0x0080
#define AUTO_DIALOUT_EVENT	0x0100
#define WARNING1_EVENT 		0x0200
#define WARNING2_EVENT 		0x0400
#define MANUEL_CAL_EVENT 	0x0800
#define UPDATE_TIME_EVENT	0x1000
#define UPDATE_MENU_EVENT	0x2000
#define CRAFT_PORT_EVENT	0x4000
#define ANY_SYSTEM_EVENT	0xFFFF

// Timer Events
#define TIMER_MODE_TIMER_EVENT	0x0080
#define SOFT_TIMER_CHECK_EVENT	0x0100
#define ANY_TIMER_EVENT			0xFFFF

// Menu Events
#define RESULTS_MENU_EVENT	0x0020
#define ANY_MENU_EVENT		0xFFFF

// System Event Macros
#define raiseSystemEventFlag(x) (SysEvents_flags.wrd |= x)
#define clearSystemEventFlag(x) (SysEvents_flags.wrd &= ~x)
#define getSystemEventState(x)	(SysEvents_flags.wrd & x)

// Timer Event Macros
#define raiseTimerEventFlag(x)	(mn_timer.wrd |= x)
#define clearTimerEventFlag(x)	(mn_timer.wrd &= ~x)
#define getTimerEventState(x)	(mn_timer.wrd & x)

// Menu Event Macros
#define raiseMenuEventFlag(x)	(mn_event_flags.wrd |= x)
#define clearMenuEventFlag(x)	(mn_event_flags.wrd &= ~x)
#define getMenuEventState(x)	(mn_event_flags.wrd & x)

// Processing states
enum {
	IDLE_STATE     = 0x10,
	SAMPLING_STATE = 0x20
};

#define START_TRIGGER_CMD 20
#define STOP_TRIGGER_CMD 30
#define ZERO_SENSORS_CMD 40
#define MANUEL_TRIGGER_CMD 50

#define SAMPLING_CMD_ID 90
#define MANUAL_CAL_PULSE_CMD 92

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------

#endif // _SYS_EVENT_H_
