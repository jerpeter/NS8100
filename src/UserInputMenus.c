///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: UserInputMenus.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:02 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/UserInputMenus.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Menu.h"
#include "Record.h"
#include "Display.h"
#include "Typedefs.h"
#include "Uart.h"
#include "Keypad.h"
#include "SysEvents.h"
#include "Flash.h"
#include "RemoteCommon.h"
#include "RemoteHandler.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT alarmTwoMenu[];
extern USER_MENU_STRUCT alarmOneSeismicLevelMenu[];
extern USER_MENU_STRUCT alarmOneAirLevelMenu[];
extern USER_MENU_STRUCT alarmTwoSeismicLevelMenu[];
extern USER_MENU_STRUCT alarmTwoAirLevelMenu[];
extern USER_MENU_STRUCT alarmOneTimeMenu[];
extern USER_MENU_STRUCT alarmTwoTimeMenu[];
extern USER_MENU_STRUCT alarmOneMenu[];
extern USER_MENU_STRUCT alarmTwoMenu[];
extern USER_MENU_STRUCT airScaleMenu[];
extern USER_MENU_STRUCT barChannelMenu[];
extern USER_MENU_STRUCT barResultMenu[];
extern USER_MENU_STRUCT bitAccuracyMenu[];
extern USER_MENU_STRUCT configMenu[];
extern USER_MENU_STRUCT distanceToSourceMenu[];
extern USER_MENU_STRUCT modemSetupMenu[];
extern USER_MENU_STRUCT modemDialMenu[];
extern USER_MENU_STRUCT modemInitMenu[];
extern USER_MENU_STRUCT modemResetMenu[];
extern USER_MENU_STRUCT modemRetryMenu[];
extern USER_MENU_STRUCT modemRetryTimeMenu[];
extern USER_MENU_STRUCT modeMenu[];
extern USER_MENU_STRUCT notesMenu[];
extern USER_MENU_STRUCT operatorMenu[];
extern USER_MENU_STRUCT recordTimeMenu[];
extern USER_MENU_STRUCT sampleRateMenu[];
extern USER_MENU_STRUCT saveSetupMenu[];
extern USER_MENU_STRUCT seismicTriggerMenu[];
extern USER_MENU_STRUCT seismicLocationMenu[];
extern USER_MENU_STRUCT sensitivityMenu[];
extern USER_MENU_STRUCT sensorTypeMenu[];
extern USER_MENU_STRUCT summaryIntervalMenu[];
extern USER_MENU_STRUCT unlockCodeMenu[];
extern USER_MENU_STRUCT weightPerDelayMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

//*****************************************************************************
//=============================================================================
// Air Trigger Menu
//=============================================================================
//*****************************************************************************
#define AIR_TRIGGER_MENU_ENTRIES 4
USER_MENU_STRUCT airTriggerMenu[AIR_TRIGGER_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, AIR_TRIGGER_TEXT, TITLE_POST_TAG, 
	{INSERT_USER_MENU_INFO(INTEGER_SPECIAL_TYPE, AIR_TRIGGER_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(DB_TYPE, DBA_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&airTriggerMenuHandler}}
};

//-------------------------
// Air Trigger Menu Handler
//-------------------------
void airTriggerMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_triggerRecord.trec.airTriggerLevel = *((uint32*)data);

		if (g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR)
		{
			debug("Air Trigger: No Trigger\n");
		}
		else
		{
			debug("Air Trigger: %d\n", g_triggerRecord.trec.airTriggerLevel);
		}

		if ((g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR) && g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR)
		{
			messageBox(getLangText(WARNING_TEXT), "BOTH SEISMIC AND AIR SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);
			
#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
			{
				g_triggerRecord.trec.seismicTriggerLevel /= (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &g_triggerRecord.trec.seismicTriggerLevel,
				(SEISMIC_TRIGGER_DEFAULT_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
				(SEISMIC_TRIGGER_MIN_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
				RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
#if 1 // ns8100 - Down convert to current bit accuracy setting
		if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
		{
			g_triggerRecord.trec.seismicTriggerLevel /= (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint);
		}		
#endif
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &g_triggerRecord.trec.seismicTriggerLevel,
			(SEISMIC_TRIGGER_DEFAULT_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
			(SEISMIC_TRIGGER_MIN_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
			g_bitAccuracyMidpoint);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm One Seismic Level Menu
//=============================================================================
//*****************************************************************************
#define ALARM_ONE_SEISMIC_LEVEL_MENU_ENTRIES 5
USER_MENU_STRUCT alarmOneSeismicLevelMenu[ALARM_ONE_SEISMIC_LEVEL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_1_SEISMIC_LVL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_COUNT_TYPE, ALARM_ONE_SEISMIC_LEVEL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_3)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(IN_TYPE, MM_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneSeismicLevelMenuHandler}}
};

//-------------------------------------
// Alarm One Seismic Level Menu Handler
//-------------------------------------
void alarmOneSeismicLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.alarm_one_seismic_lvl = *((uint32*)data);
		
		if (g_helpRecord.alarm_one_seismic_lvl == NO_TRIGGER_CHAR)
		{
			debug("Alarm 1 Seismic Trigger: No Trigger\n");
		}
		else
		{
#if 1 // ns8100 - Up convert to 16-bit
			g_helpRecord.alarm_one_seismic_lvl *= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
#endif
			debug("Alarm 1 Seismic Level Count: %d\n", g_helpRecord.alarm_one_seismic_lvl);
		}

		if (g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &g_helpRecord.alarm_one_air_lvl,
				g_helpRecord.alarm_one_air_min_lvl, g_helpRecord.alarm_one_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else // g_helpRecord.alarm_one_mode == ALARM_MODE_SEISMIC
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmOneTimeMenu, &g_helpRecord.alarm_one_time,
				ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
				ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
#if 1 // ns8100 - Up convert to 16-bit
		if (g_helpRecord.alarm_one_seismic_lvl != NO_TRIGGER_CHAR)
		{
			g_helpRecord.alarm_one_seismic_lvl *= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
		}		
#endif

		ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm One Air Level Menu
//=============================================================================
//*****************************************************************************
#define ALARM_ONE_AIR_LEVEL_MENU_ENTRIES 4
USER_MENU_STRUCT alarmOneAirLevelMenu[ALARM_ONE_AIR_LEVEL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_1_AIR_LEVEL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_SPECIAL_TYPE, ALARM_ONE_AIR_LEVEL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(DB_TYPE, DBA_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneAirLevelMenuHandler}}
};

//---------------------------------
// Alarm One Air Level Menu Handler
//---------------------------------
void alarmOneAirLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.alarm_one_air_lvl = *((uint32*)data);
		
		debug("Alarm 1 Air Level: %d\n", g_helpRecord.alarm_one_air_lvl);

		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmOneTimeMenu, &g_helpRecord.alarm_one_time,
			ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
			ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH)
		{
#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_helpRecord.alarm_one_seismic_lvl != NO_TRIGGER_CHAR)
			{
				g_helpRecord.alarm_one_seismic_lvl /= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &g_helpRecord.alarm_one_seismic_lvl,
				(g_helpRecord.alarm_one_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				(g_helpRecord.alarm_one_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&alarmOneMenu, &g_helpRecord.alarm_one_mode);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm One Time Menu
//=============================================================================
//*****************************************************************************
#define ALARM_ONE_TIME_MENU_ENTRIES 4
USER_MENU_STRUCT alarmOneTimeMenu[ALARM_ONE_TIME_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_1_TIME_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(FLOAT_TYPE, ALARM_ONE_TIME_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneTimeMenuHandler}}
};

//----------------------------
// Alarm One Time Menu Handler
//----------------------------
void alarmOneTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.alarm_one_time = *((float*)data);
		
		debug("Alarm 1 Time: %f\n", g_helpRecord.alarm_one_time);

		ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
	}
	else if (keyPressed == ESC_KEY)
	{
		if ((g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH) || (g_helpRecord.alarm_one_mode == ALARM_MODE_AIR))
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &g_helpRecord.alarm_one_air_lvl,
				g_helpRecord.alarm_one_air_min_lvl, g_helpRecord.alarm_one_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else if (g_helpRecord.alarm_one_mode == ALARM_MODE_SEISMIC)
		{
#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_helpRecord.alarm_one_seismic_lvl != NO_TRIGGER_CHAR)
			{
				g_helpRecord.alarm_one_seismic_lvl /= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &g_helpRecord.alarm_one_seismic_lvl,
				(g_helpRecord.alarm_one_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				(g_helpRecord.alarm_one_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else // g_helpRecord.alarm_one_mode == ALARM_MODE_OFF
		{
			ACTIVATE_USER_MENU_MSG(&alarmOneMenu, &g_helpRecord.alarm_one_mode);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm Two Seismic Level Menu
//=============================================================================
//*****************************************************************************
#define ALARM_TWO_SEISMIC_LEVEL_MENU_ENTRIES 5
USER_MENU_STRUCT alarmTwoSeismicLevelMenu[ALARM_TWO_SEISMIC_LEVEL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_2_SEISMIC_LVL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_COUNT_TYPE, ALARM_TWO_SEISMIC_LEVEL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_3)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(IN_TYPE, MM_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoSeismicLevelMenuHandler}}
};

//-------------------------------------
// Alarm Two Seismic Level Menu Handler
//-------------------------------------
void alarmTwoSeismicLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.alarm_two_seismic_lvl = *((uint32*)data);
		
		if (g_helpRecord.alarm_two_seismic_lvl == NO_TRIGGER_CHAR)
		{
			debug("Alarm 2 Seismic Trigger: No Trigger\n");
		}
		else
		{
#if 1 // ns8100 - Up convert to 16-bit
			g_helpRecord.alarm_two_seismic_lvl *= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
#endif
			debug("Alarm 2 Seismic Level Count: %d\n", g_helpRecord.alarm_two_seismic_lvl);
		}

		if (g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &g_helpRecord.alarm_two_air_lvl,
				g_helpRecord.alarm_two_air_min_lvl, g_helpRecord.alarm_two_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else // g_helpRecord.alarm_two_mode == ALARM_MODE_SEISMIC
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &g_helpRecord.alarm_two_time,
				ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
				ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
#if 1 // ns8100 - Up convert to 16-bit
		if (g_helpRecord.alarm_two_seismic_lvl != NO_TRIGGER_CHAR)
		{
			g_helpRecord.alarm_two_seismic_lvl *= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
		}		
#endif

		ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm Two Air Level Menu
//=============================================================================
//*****************************************************************************
#define ALARM_TWO_AIR_LEVEL_MENU_ENTRIES 4
USER_MENU_STRUCT alarmTwoAirLevelMenu[ALARM_TWO_AIR_LEVEL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_2_AIR_LEVEL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_SPECIAL_TYPE, ALARM_TWO_AIR_LEVEL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(DB_TYPE, DBA_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoAirLevelMenuHandler}}
};

//---------------------------------
// Alarm Two Air Level Menu Handler
//---------------------------------
void alarmTwoAirLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.alarm_two_air_lvl = *((uint32*)data);
		
		debug("Alarm 2 Air Level: %d\n", g_helpRecord.alarm_two_air_lvl);

		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &g_helpRecord.alarm_two_time,
			ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
			ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH)
		{
#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_helpRecord.alarm_two_seismic_lvl != NO_TRIGGER_CHAR)
			{
				g_helpRecord.alarm_two_seismic_lvl /= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &g_helpRecord.alarm_two_seismic_lvl,
				(g_helpRecord.alarm_two_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				(g_helpRecord.alarm_two_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, &g_helpRecord.alarm_two_mode);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm Two Time Menu
//=============================================================================
//*****************************************************************************
#define ALARM_TWO_TIME_MENU_ENTRIES 4
USER_MENU_STRUCT alarmTwoTimeMenu[ALARM_TWO_TIME_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_2_TIME_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(FLOAT_TYPE, ALARM_TWO_TIME_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoTimeMenuHandler}}
};

//----------------------------
// Alarm Two Time Menu Handler
//----------------------------
void alarmTwoTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.alarm_two_time = *((float*)data);
		
		debug("Alarm 2 Time: %f\n", g_helpRecord.alarm_two_time);

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
	}
	else if (keyPressed == ESC_KEY)
	{
		if ((g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH) || (g_helpRecord.alarm_two_mode == ALARM_MODE_AIR))
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &g_helpRecord.alarm_two_air_lvl,
				g_helpRecord.alarm_two_air_min_lvl, g_helpRecord.alarm_two_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else if (g_helpRecord.alarm_two_mode == ALARM_MODE_SEISMIC)
		{
#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_helpRecord.alarm_two_seismic_lvl != NO_TRIGGER_CHAR)
			{
				g_helpRecord.alarm_two_seismic_lvl /= (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &g_helpRecord.alarm_two_seismic_lvl,
				(g_helpRecord.alarm_two_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				(g_helpRecord.alarm_two_seismic_min_lvl / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else // g_helpRecord.alarm_two_mode == ALARM_MODE_OFF
		{
			ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, &g_helpRecord.alarm_two_mode);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Company Menu
//=============================================================================
//*****************************************************************************
#define COMPANY_MENU_ENTRIES 5
#define MAX_COMPANY_CHARS 30
USER_MENU_STRUCT companyMenu[COMPANY_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, COMPANY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, COMPANY_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_COMPANY_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&companyMenuHandler}}
};

//---------------------
// Company Menu Handler
//---------------------
void companyMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_triggerRecord.trec.client), (char*)data);
		debug("Company: <%s>, Length: %d\n", g_triggerRecord.trec.client, strlen((char*)g_triggerRecord.trec.client));
		
		ACTIVATE_USER_MENU_MSG(&seismicLocationMenu, &g_triggerRecord.trec.loc);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&bitAccuracyMenu, g_triggerRecord.trec.bitAccuracy);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Copies Menu
//=============================================================================
//*****************************************************************************
#define COPIES_MENU_ENTRIES 4
USER_MENU_STRUCT copiesMenu[COPIES_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, COPIES_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_BYTE_TYPE, COPIES_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(NO_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&copiesMenuHandler}}
};

//--------------------
// Copies Menu Handler
//--------------------
void copiesMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		// Check if the user is printing an event from the summary list
		if (g_summaryListMenuActive == YES)
		{
			g_activeMenu = SUMMARY_MENU;
			ACTIVATE_MENU_MSG(); mn_msg.data[0] = ESC_KEY;
		}
		else // g_summaryListMenuActive == NO
		{
			g_helpRecord.copies = *((uint8*)data);
			debug("Copies: %d\n", g_helpRecord.copies);

			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, COPIES);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Distance to Source Menu
//=============================================================================
//*****************************************************************************
#define DISTANCE_TO_SOURCE_MENU_ENTRIES 4
USER_MENU_STRUCT distanceToSourceMenu[DISTANCE_TO_SOURCE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, DISTANCE_TO_SOURCE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(FLOAT_WITH_N_TYPE, DISTANCE_TO_SOURCE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(FT_TYPE, M_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&distanceToSourceMenuHandler}}
};

//--------------------------------
// Distance to Source Menu Handler
//--------------------------------
void distanceToSourceMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_triggerRecord.trec.dist_to_source = *((float*)data);

		if (g_helpRecord.units_of_measure == METRIC_TYPE)
			g_triggerRecord.trec.dist_to_source *= FT_PER_METER;

		debug("Distance to Source: %f ft\n", g_triggerRecord.trec.dist_to_source);

		if ((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&weightPerDelayMenu, &g_triggerRecord.trec.weight_per_delay,
				WEIGHT_PER_DELAY_DEFAULT_VALUE, WEIGHT_PER_DELAY_INCREMENT_VALUE,
				WEIGHT_PER_DELAY_MIN_VALUE, WEIGHT_PER_DELAY_MAX_VALUE);
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_MSG(&operatorMenu, &g_triggerRecord.trec.oper);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&notesMenu, &g_triggerRecord.trec.comments);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Lcd Impulse Time Menu
//=============================================================================
//*****************************************************************************
#define LCD_IMPULSE_TIME_MENU_ENTRIES 4
USER_MENU_STRUCT lcdImpulseTimeMenu[LCD_IMPULSE_TIME_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, LCD_IMPULSE_TIME_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_BYTE_TYPE, LCD_IMPULSE_TIME_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&lcdImpulseTimeMenuHandler}}
};

//------------------------------
// Lcd Impulse Time Menu Handler
//------------------------------
void lcdImpulseTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_triggerRecord.berec.impulseMenuUpdateSecs = *((uint8*)data);

		debug("LCD Impulse Menu Update Seconds: %d\n", g_triggerRecord.berec.impulseMenuUpdateSecs);

		saveRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&barResultMenu, g_helpRecord.vector_sum);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&summaryIntervalMenu, g_triggerRecord.bgrec.summaryInterval);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Lcd Timeout Menu
//=============================================================================
//*****************************************************************************
#define LCD_TIMEOUT_MENU_ENTRIES 4
USER_MENU_STRUCT lcdTimeoutMenu[LCD_TIMEOUT_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, LCD_TIMEOUT_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_BYTE_TYPE, LCD_TIMEOUT_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(MINS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&lcdTimeoutMenuHandler}}
};

//-------------------------
// Lcd Timeout Menu Handler
//-------------------------
void lcdTimeoutMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_helpRecord.lcd_timeout = *((uint8*)data);

		debug("LCD Timeout: %d\n", g_helpRecord.lcd_timeout);

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, LCD_TIMEOUT);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Modem Dial Menu
//=============================================================================
//*****************************************************************************
#define MODEM_DIAL_MENU_ENTRIES 5
#define MAX_MODEM_DIAL_CHARS 30
USER_MENU_STRUCT modemDialMenu[MODEM_DIAL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MODEM_DIAL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, MODEM_DIAL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_MODEM_DIAL_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemDialMenuHandler}}
};

//------------------------
// Modem Dial Menu Handler
//------------------------
void modemDialMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_modemSetupRecord.dial), (char*)data);
		debug("Modem Dial: <%s>, Length: %d\n", g_modemSetupRecord.dial, strlen((char*)g_modemSetupRecord.dial));

		ACTIVATE_USER_MENU_MSG(&modemResetMenu, &g_modemSetupRecord.reset);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemInitMenu, &g_modemSetupRecord.init);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Modem Init Menu
//=============================================================================
//*****************************************************************************
#define MODEM_INIT_MENU_ENTRIES 6
#define MAX_MODEM_INIT_CHARS 60
USER_MENU_STRUCT modemInitMenu[MODEM_INIT_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MODEM_INIT_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, MODEM_INIT_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_MODEM_INIT_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemInitMenuHandler}}
};

//------------------------
// Modem Init Menu Handler
//------------------------
void modemInitMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_modemSetupRecord.init), (char*)data);
		debug("Modem Init: <%s>, Length: %d\n", g_modemSetupRecord.init, strlen((char*)g_modemSetupRecord.init));

		ACTIVATE_USER_MENU_MSG(&modemDialMenu, &g_modemSetupRecord.dial);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemSetupMenu, g_modemSetupRecord.modemStatus);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Modem Reset Menu
//=============================================================================
//*****************************************************************************
#define MODEM_RESET_MENU_ENTRIES 4
#define MAX_MODEM_RESET_CHARS 15
USER_MENU_STRUCT modemResetMenu[MODEM_RESET_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MODEM_RESET_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, MODEM_RESET_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_MODEM_RESET_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemResetMenuHandler}}
};

//-------------------------
// Modem Reset Menu Handler
//-------------------------
void modemResetMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_modemSetupRecord.reset), (char*)data);
		debug("Modem Reset: <%s>, Length: %d\n", g_modemSetupRecord.reset, strlen((char*)g_modemSetupRecord.reset));

		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryMenu, &g_modemSetupRecord.retries,
			MODEM_RETRY_DEFAULT_VALUE, MODEM_RETRY_MIN_VALUE, MODEM_RETRY_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemDialMenu, &g_modemSetupRecord.dial);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Modem Retry Menu
//=============================================================================
//*****************************************************************************
#define MODEM_RETRY_MENU_ENTRIES 4
USER_MENU_STRUCT modemRetryMenu[MODEM_RETRY_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MODEM_RETRY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_BYTE_TYPE, MODEM_RETRY_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(NO_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemRetryMenuHandler}}
};

//-------------------------
// Modem Retry Menu Handler
//-------------------------
void modemRetryMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_modemSetupRecord.retries = *((uint8*)data);
		debug("Modem Retries: %d\n", g_modemSetupRecord.retries);
		
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryTimeMenu, &g_modemSetupRecord.retryTime,
			MODEM_RETRY_TIME_DEFAULT_VALUE, MODEM_RETRY_TIME_MIN_VALUE, MODEM_RETRY_TIME_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemResetMenu, &g_modemSetupRecord.reset);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Modem Retry Time Menu
//=============================================================================
//*****************************************************************************
#define MODEM_RETRY_TIME_MENU_ENTRIES 4
USER_MENU_STRUCT modemRetryTimeMenu[MODEM_RETRY_TIME_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MODEM_RETRY_TIME_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_BYTE_TYPE, MODEM_RETRY_TIME_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(MINS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemRetryTimeMenuHandler}}
};

//------------------------------
// Modem Retry Time Menu Handler
//------------------------------
void modemRetryTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_modemSetupRecord.retryTime = *((uint8*)data);
		debug("Modem Retry Time: %d\n", g_modemSetupRecord.retryTime);
		
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&unlockCodeMenu, &g_modemSetupRecord.unlockCode,
			UNLOCK_CODE_DEFAULT_VALUE, UNLOCK_CODE_MIN_VALUE, UNLOCK_CODE_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryMenu, &g_modemSetupRecord.retries,
			MODEM_RETRY_DEFAULT_VALUE, MODEM_RETRY_MIN_VALUE, MODEM_RETRY_MAX_VALUE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Notes Menu
//=============================================================================
//*****************************************************************************
#define NOTES_MENU_ENTRIES 8
#define MAX_NOTES_CHARS 100
USER_MENU_STRUCT notesMenu[NOTES_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, NOTES_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, NOTES_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_NOTES_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&notesMenuHandler}}
};

//-------------------
// Notes Menu Handler
//-------------------
void notesMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_triggerRecord.trec.comments), (char*)data);
		debug("Notes: <%s>, Length: %d\n", g_triggerRecord.trec.comments, strlen((char*)g_triggerRecord.trec.comments));

		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&distanceToSourceMenu, &g_triggerRecord.trec.dist_to_source,
			DISTANCE_TO_SOURCE_DEFAULT_VALUE, DISTANCE_TO_SOURCE_INCREMENT_VALUE,
			DISTANCE_TO_SOURCE_MIN_VALUE, DISTANCE_TO_SOURCE_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&seismicLocationMenu, &g_triggerRecord.trec.loc);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Operator Menu
//=============================================================================
//*****************************************************************************
#define OPERATOR_MENU_ENTRIES 5
#define MAX_OPERATOR_CHARS 30
USER_MENU_STRUCT operatorMenu[OPERATOR_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, OPERATOR_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, OPERATOR_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_OPERATOR_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&operatorMenuHandler}}
};

//----------------------
// Operator Menu Handler
//----------------------
void operatorMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_triggerRecord.trec.oper), (char*)data);
		debug("Operator: <%s>, Length: %d\n", g_triggerRecord.trec.oper, strlen((char*)g_triggerRecord.trec.oper));

		if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
		{
			sprintf((char*)&g_menuTags[LOW_SENSITIVITY_MAX_TAG].text, " (%.0fmg)", 
					(float)g_factorySetupRecord.sensor_type / (float)200);
			sprintf((char*)&g_menuTags[HIGH_SENSITIVITY_MAX_TAG].text, " (%.0fmg)", 
					(float)g_factorySetupRecord.sensor_type / (float)400);
		}
		else if (g_helpRecord.units_of_measure == IMPERIAL)
		{
			sprintf((char*)&g_menuTags[LOW_SENSITIVITY_MAX_TAG].text, " (%.2fin)", 
					(float)g_factorySetupRecord.sensor_type / (float)200);
			sprintf((char*)&g_menuTags[HIGH_SENSITIVITY_MAX_TAG].text, " (%.2fin)", 
					(float)g_factorySetupRecord.sensor_type / (float)400);
		}
		else // g_helpRecord.units_of_measure == METRIC
		{
			sprintf((char*)&g_menuTags[LOW_SENSITIVITY_MAX_TAG].text, " (%.2fmm)", 
					(float)g_factorySetupRecord.sensor_type * (float)25.4 / (float)200);
			sprintf((char*)&g_menuTags[HIGH_SENSITIVITY_MAX_TAG].text, " (%.2fmm)", 
					(float)g_factorySetupRecord.sensor_type * (float)25.4 / (float)400);
		}

		ACTIVATE_USER_MENU_MSG(&sensitivityMenu, g_triggerRecord.srec.sensitivity);
	}
	else if (keyPressed == ESC_KEY)
	{
		if ((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&weightPerDelayMenu, &g_triggerRecord.trec.weight_per_delay,
				WEIGHT_PER_DELAY_DEFAULT_VALUE, WEIGHT_PER_DELAY_INCREMENT_VALUE,
				WEIGHT_PER_DELAY_MIN_VALUE, WEIGHT_PER_DELAY_MAX_VALUE);
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&distanceToSourceMenu, &g_triggerRecord.trec.dist_to_source,
				DISTANCE_TO_SOURCE_DEFAULT_VALUE, DISTANCE_TO_SOURCE_INCREMENT_VALUE,
				DISTANCE_TO_SOURCE_MIN_VALUE, DISTANCE_TO_SOURCE_MAX_VALUE);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Record Time Menu
//=============================================================================
//*****************************************************************************
#define RECORD_TIME_MENU_ENTRIES 4
USER_MENU_STRUCT recordTimeMenu[RECORD_TIME_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, RECORD_TIME_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_LONG_TYPE, RECORD_TIME_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&recordTimeMenuHandler}}
};

//-------------------------
// Record Time Menu Handler
//-------------------------
void recordTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_triggerRecord.trec.record_time = *((uint32*)data);
		debug("Record Time: %d\n", g_triggerRecord.trec.record_time);

		if ((!g_factorySetupRecord.invalid) && (g_factorySetupRecord.aweight_option == ENABLED))
		{
			ACTIVATE_USER_MENU_MSG(&airScaleMenu, 0);
		}
		else
		{
			// If alarm mode is off, then proceed to save setup
			if ((g_helpRecord.alarm_one_mode == ALARM_MODE_OFF) && (g_helpRecord.alarm_two_mode == ALARM_MODE_OFF))
			{
				ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
			}
			else // Goto Alarm setup menus
			{
				ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
			}
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&airTriggerMenu, &g_triggerRecord.trec.airTriggerLevel,
			AIR_TRIGGER_DEFAULT_VALUE, AIR_TRIGGER_MIN_VALUE, AIR_TRIGGER_MAX_VALUE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Save Record Menu
//=============================================================================
//*****************************************************************************
#define SAVE_RECORD_MENU_ENTRIES 4
#define MAX_SAVE_RECORD_CHARS 8
USER_MENU_STRUCT saveRecordMenu[SAVE_RECORD_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, NAME_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, SAVE_RECORD_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_SAVE_RECORD_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&saveRecordMenuHandler}}
};

//-------------------------
// Save Record Menu Handler
//-------------------------
void saveRecordMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	uint8 availableLocation = 0;
	uint8 choice;
	uint8 match = NO;
	char message[50];
	
	if (keyPressed == ENTER_KEY)
	{	
		if (strlen((char*)data) != 0)
		{
			debug("Save Record Name: <%s>\n", (char*)data);
			availableLocation = checkForAvailableTriggerRecordEntry((char*)data, &match);

			if (match == YES)
			{
				// Found another saved record with the same name

				// Prompt to alert the user the name has been used already
				overlayMessage(getLangText(WARNING_TEXT), getLangText(NAME_ALREADY_USED_TEXT), 2 * SOFT_SECS);
				
				// Ask if they want to overwrite the saved setup
				choice = messageBox(getLangText(WARNING_TEXT), getLangText(OVERWRITE_SETTINGS_TEXT), MB_YESNO);
				
				// Check if user selected YES
				if (choice == MB_FIRST_CHOICE)
				{
					// Copy over the record name
					strcpy((char*)g_triggerRecord.name, (char*)data);

					// Save the trigger record in the available location
					saveRecData(&g_triggerRecord, availableLocation, REC_TRIGGER_USER_MENU_TYPE);

					// Also save the trigger record in the default record for future use
					saveRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

					// Update the title based on the current mode
					updateModeMenuTitle(g_triggerRecord.op_mode);
					ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
				}
				else
				{
					// Let the code recall the current menu
					// If that doesn't work, then uncomment the following line to call the menu from start
					//ACTIVATE_USER_MENU_MSG(&saveRecordMenu, (uint8)NULL);
				}
			}
			else if (availableLocation == 0)
			{
				// Prompt to alert the user that the saved settings are full
				byteSet(&message[0], 0, sizeof(message));
				sprintf(message, "%s %s", getLangText(SAVED_SETTINGS_TEXT), getLangText(FULL_TEXT));
				overlayMessage(getLangText(WARNING_TEXT), message, 2 * SOFT_SECS);

				// Copy over the record name
				strcpy((char*)g_triggerRecord.name, (char*)data);

				// Out of empty record slots, goto the Overwrite menu
				g_activeMenu = OVERWRITE_MENU;
				ACTIVATE_MENU_WITH_DATA_MSG(g_triggerRecord.op_mode);
			}
			else // Found an empty record slot
			{
				// Copy over the record name
				strcpy((char*)g_triggerRecord.name, (char*)data);

				// Save the trigger record in the available location
				saveRecData(&g_triggerRecord, availableLocation, REC_TRIGGER_USER_MENU_TYPE);

				// Also save the trigger record in the default record for future use
				saveRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

				// Update the title based on the current mode
				updateModeMenuTitle(g_triggerRecord.op_mode);
				ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
			}
		}
		else // Name was empty, don't allow it to be saved
		{
			messageBox(getLangText(WARNING_TEXT), getLangText(NAME_MUST_HAVE_AT_LEAST_ONE_CHARACTER_TEXT), MB_OK);
			
			// Let the code recall the current menu
			// If that doesn't work, then uncomment the following line to call the menu from start
			//ACTIVATE_USER_MENU_MSG(&saveRecordMenu, (uint8)NULL);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Seismic Location Menu
//=============================================================================
//*****************************************************************************
#define SEISMIC_LOCATION_MENU_ENTRIES 5
#define MAX_SEISMIC_LOCATION_CHARS 30
USER_MENU_STRUCT seismicLocationMenu[SEISMIC_LOCATION_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SEIS_LOCATION_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, SEISMIC_LOCATION_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_SEISMIC_LOCATION_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&seismicLocationMenuHandler}}
};

//------------------------------
// Seismic Location Menu Handler
//------------------------------
void seismicLocationMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&g_triggerRecord.trec.loc), (char*)data);
		debug("Seismic Location: <%s>, Length: %d\n", g_triggerRecord.trec.loc, strlen((char*)g_triggerRecord.trec.loc));

		ACTIVATE_USER_MENU_MSG(&notesMenu, &g_triggerRecord.trec.comments);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&companyMenu, &g_triggerRecord.trec.client);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Seismic Trigger Level Menu
//=============================================================================
//*****************************************************************************
#define SEISMIC_TRIGGER_MENU_ENTRIES 5
USER_MENU_STRUCT seismicTriggerMenu[SEISMIC_TRIGGER_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SEISMIC_TRIGGER_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_COUNT_TYPE, SEISMIC_TRIGGER_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_3)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(IN_TYPE, MM_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&seismicTriggerMenuHandler}}
};

//-----------------------------
// Seismic Trigger Menu Handler
//-----------------------------
void seismicTriggerMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_triggerRecord.trec.seismicTriggerLevel = *((uint32*)data);

		if (g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR)
		{
			debug("Seismic Trigger: No Trigger\n");
		}
		else
		{
#if 1 // ns8100 - Up convert to 16-bit
			g_triggerRecord.trec.seismicTriggerLevel *= (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint);
#endif
			debug("Seismic Trigger: %d counts\n", g_triggerRecord.trec.seismicTriggerLevel);
		}

		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&airTriggerMenu, &g_triggerRecord.trec.airTriggerLevel,
			AIR_TRIGGER_DEFAULT_VALUE, AIR_TRIGGER_MIN_VALUE, AIR_TRIGGER_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
#if 1 // ns8100 - Up convert to 16-bit
		if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
		{
			g_triggerRecord.trec.seismicTriggerLevel *= (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint);
		}		
#endif

		if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			ACTIVATE_USER_MENU_MSG(&barResultMenu, g_helpRecord.vector_sum);
		}		
		else // WAVEFORM_MODE
		{
			ACTIVATE_USER_MENU_MSG(&sensitivityMenu, g_triggerRecord.srec.sensitivity);	
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Serial Number Menu
//=============================================================================
//*****************************************************************************
#define SERIAL_NUMBER_MENU_ENTRIES 4
#define MAX_SERIAL_NUMBER_CHARS 15
USER_MENU_STRUCT serialNumberMenu[SERIAL_NUMBER_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SERIAL_NUMBER_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(STRING_TYPE, SERIAL_NUMBER_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {MAX_SERIAL_NUMBER_CHARS}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&serialNumberMenuHandler}}
};

//---------------------------
// Serial Number Menu Handler
//---------------------------
void serialNumberMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		debug("Serial #: <%s>, Length: %d\n", (char*)data, strlen((char*)data));
		strcpy((char*)g_factorySetupRecord.serial_num, (char*)data);

		ACTIVATE_USER_MENU_MSG(&sensorTypeMenu, g_factorySetupRecord.sensor_type);
	}
	else if (keyPressed == ESC_KEY)
	{
		g_activeMenu = DATE_TIME_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Weight per Delay Menu
//=============================================================================
//*****************************************************************************
#define WEIGHT_PER_DELAY_MENU_ENTRIES 4
USER_MENU_STRUCT weightPerDelayMenu[WEIGHT_PER_DELAY_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, WEIGHT_PER_DELAY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(FLOAT_WITH_N_TYPE, WEIGHT_PER_DELAY_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(LBS_TYPE, KG_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&weightPerDelayMenuHandler}}
};

//------------------------------
// Weight per Delay Menu Handler
//------------------------------
void weightPerDelayMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_triggerRecord.trec.weight_per_delay = *((float*)data);

		if (g_helpRecord.units_of_measure == METRIC_TYPE)
			g_triggerRecord.trec.weight_per_delay *= LBS_PER_KG;

		debug("Weight per Delay: %.1f lbs\n", g_triggerRecord.trec.weight_per_delay);

		ACTIVATE_USER_MENU_MSG(&operatorMenu, &g_triggerRecord.trec.oper);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&distanceToSourceMenu, &g_triggerRecord.trec.dist_to_source,
			DISTANCE_TO_SOURCE_DEFAULT_VALUE, DISTANCE_TO_SOURCE_INCREMENT_VALUE,
			DISTANCE_TO_SOURCE_MIN_VALUE, DISTANCE_TO_SOURCE_MAX_VALUE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Unlock Code Menu
//=============================================================================
//*****************************************************************************
#define UNLOCK_CODE_MENU_ENTRIES 4
USER_MENU_STRUCT unlockCodeMenu[UNLOCK_CODE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, UNLOCK_CODE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(INTEGER_WORD_FIXED_TYPE, UNLOCK_CODE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ROW_2)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(NO_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&unlockCodeMenuHandler}}
};

//-------------------------
// Unlock Code Menu Handler
//-------------------------
void unlockCodeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0};
	
	if (keyPressed == ENTER_KEY)
	{	
		g_modemSetupRecord.unlockCode = *((uint16*)data);

		debug("Modem Unlock Code: %4d\n", g_modemSetupRecord.unlockCode);

		saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);

		if (READ_DCD == NO_CONNECTION)
		{
			craftInitStatusFlags();
		}

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryTimeMenu, &g_modemSetupRecord.retryTime,
			MODEM_RETRY_TIME_DEFAULT_VALUE, MODEM_RETRY_TIME_MIN_VALUE, MODEM_RETRY_TIME_MAX_VALUE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}
