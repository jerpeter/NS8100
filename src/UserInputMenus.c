///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: UserInputMenus.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:09 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/UserInputMenus.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "Menu.h"
#include "Rec.h"
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
extern int32 active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern REC_EVENT_MN_STRUCT trig_rec;
extern REC_HELP_MN_STRUCT help_rec;
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern MODEM_SETUP_STRUCT modem_setup_rec;
extern USER_MENU_CACHE_DATA userMenuCacheData;
extern USER_MENU_TAGS_STRUCT menuTags[];
extern uint8 summaryListMenuActive;
extern SUMMARY_DATA *results_summtable_ptr;
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
///	Globals
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(DB_TYPE, DBA_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&airTriggerMenuHandler}}
};

//-------------------------
// Air Trigger Menu Handler
//-------------------------
void airTriggerMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		trig_rec.trec.soundTriggerLevel = *((uint32*)data);

		if (trig_rec.trec.soundTriggerLevel == NO_TRIGGER_CHAR)
		{
			debug("Air Trigger: No Trigger\n");
		}
		else
		{
			debug("Air Trigger: %d\n", trig_rec.trec.soundTriggerLevel);
		}

		if ((trig_rec.trec.soundTriggerLevel == NO_TRIGGER_CHAR) && trig_rec.trec.seismicTriggerLevel == NO_TRIGGER_CHAR)
		{
			messageBox(getLangText(WARNING_TEXT), "BOTH SEISMIC AND AIR SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);
			
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &trig_rec.trec.seismicTriggerLevel,
				SEISMIC_TRIGGER_DEFAULT_VALUE, SEISMIC_TRIGGER_MIN_VALUE, SEISMIC_TRIGGER_MAX_VALUE);
		}
		else
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &trig_rec.trec.record_time,
				RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, trig_rec.trec.record_time_max);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &trig_rec.trec.seismicTriggerLevel,
			SEISMIC_TRIGGER_DEFAULT_VALUE, SEISMIC_TRIGGER_MIN_VALUE, SEISMIC_TRIGGER_MAX_VALUE);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(IN_TYPE, MM_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneSeismicLevelMenuHandler}}
};

//-------------------------------------
// Alarm One Seismic Level Menu Handler
//-------------------------------------
void alarmOneSeismicLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.alarm_one_seismic_lvl = *((uint32*)data);
		
		debug("Alarm 1 Seismic Level Count: %d\n", help_rec.alarm_one_seismic_lvl);

		if (help_rec.alarm_one_mode == ALARM_MODE_BOTH)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &help_rec.alarm_one_air_lvl,
				help_rec.alarm_one_air_min_lvl, help_rec.alarm_one_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else // help_rec.alarm_one_mode == ALARM_MODE_SEISMIC
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmOneTimeMenu, &help_rec.alarm_one_time,
				ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
				ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&alarmOneMenu, help_rec.alarm_one_mode);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(DB_TYPE, DBA_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneAirLevelMenuHandler}}
};

//---------------------------------
// Alarm One Air Level Menu Handler
//---------------------------------
void alarmOneAirLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.alarm_one_air_lvl = *((uint32*)data);
		
		debug("Alarm 1 Air Level: %f\n", help_rec.alarm_one_air_lvl);

		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmOneTimeMenu, &help_rec.alarm_one_time,
			ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
			ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (help_rec.alarm_one_mode == ALARM_MODE_BOTH)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &help_rec.alarm_one_seismic_lvl,
				help_rec.alarm_one_seismic_min_lvl, help_rec.alarm_one_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&alarmOneMenu, &help_rec.alarm_one_mode);
		}
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneTimeMenuHandler}}
};

//----------------------------
// Alarm One Time Menu Handler
//----------------------------
void alarmOneTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.alarm_one_time = *((float*)data);
		
		debug("Alarm 1 Time: %f\n", help_rec.alarm_one_time);

		ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, help_rec.alarm_two_mode);
	}
	else if (keyPressed == ESC_KEY)
	{
		if ((help_rec.alarm_one_mode == ALARM_MODE_BOTH) || (help_rec.alarm_one_mode == ALARM_MODE_AIR))
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &help_rec.alarm_one_air_lvl,
				help_rec.alarm_one_air_min_lvl, help_rec.alarm_one_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else if (help_rec.alarm_one_mode == ALARM_MODE_SEISMIC)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &help_rec.alarm_one_seismic_lvl,
				help_rec.alarm_one_seismic_min_lvl, help_rec.alarm_one_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
		}
		else // help_rec.alarm_one_mode == ALARM_MODE_OFF
		{
			ACTIVATE_USER_MENU_MSG(&alarmOneMenu, &help_rec.alarm_one_mode);
		}
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(IN_TYPE, MM_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoSeismicLevelMenuHandler}}
};

//-------------------------------------
// Alarm Two Seismic Level Menu Handler
//-------------------------------------
void alarmTwoSeismicLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.alarm_two_seismic_lvl = *((uint32*)data);
		
		debug("Alarm 2 Seismic Level Count: %d\n", help_rec.alarm_two_seismic_lvl);

		if (help_rec.alarm_two_mode == ALARM_MODE_BOTH)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &help_rec.alarm_two_air_lvl,
				help_rec.alarm_two_air_min_lvl, help_rec.alarm_two_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else // help_rec.alarm_two_mode == ALARM_MODE_SEISMIC
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &help_rec.alarm_two_time,
				ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
				ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, help_rec.alarm_two_mode);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(DB_TYPE, DBA_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoAirLevelMenuHandler}}
};

//---------------------------------
// Alarm Two Air Level Menu Handler
//---------------------------------
void alarmTwoAirLevelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.alarm_two_air_lvl = *((uint32*)data);
		
		debug("Alarm 2 Air Level: %f\n", help_rec.alarm_two_air_lvl);

		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &help_rec.alarm_two_time,
			ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT, 
			ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (help_rec.alarm_two_mode == ALARM_MODE_BOTH)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &help_rec.alarm_two_seismic_lvl,
				help_rec.alarm_two_seismic_min_lvl, help_rec.alarm_two_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, &help_rec.alarm_two_mode);
		}
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoTimeMenuHandler}}
};

//----------------------------
// Alarm Two Time Menu Handler
//----------------------------
void alarmTwoTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.alarm_two_time = *((float*)data);
		
		debug("Alarm 2 Time: %f\n", help_rec.alarm_two_time);

		saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
	}
	else if (keyPressed == ESC_KEY)
	{
		if ((help_rec.alarm_two_mode == ALARM_MODE_BOTH) || (help_rec.alarm_two_mode == ALARM_MODE_AIR))
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &help_rec.alarm_two_air_lvl,
				help_rec.alarm_two_air_min_lvl, help_rec.alarm_two_air_min_lvl, ALARM_AIR_MAX_VALUE);
		}
		else if (help_rec.alarm_two_mode == ALARM_MODE_SEISMIC)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &help_rec.alarm_two_seismic_lvl,
				help_rec.alarm_two_seismic_min_lvl, help_rec.alarm_two_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
		}
		else // help_rec.alarm_two_mode == ALARM_MODE_OFF
		{
			ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, &help_rec.alarm_two_mode);
		}
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_COMPANY_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&companyMenuHandler}}
};

//---------------------
// Company Menu Handler
//---------------------
void companyMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&trig_rec.trec.client), (char*)data);
		debug("Company: <%s>, Length: %d\n", trig_rec.trec.client, strlen((char*)trig_rec.trec.client));
		
		ACTIVATE_USER_MENU_MSG(&seismicLocationMenu, &trig_rec.trec.loc);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (trig_rec.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&sampleRateMenu, &trig_rec.trec.sample_rate);
		}
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(NO_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&copiesMenuHandler}}
};

//--------------------
// Copies Menu Handler
//--------------------
void copiesMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		// Check if the user is printing an event from the summary list
		if (summaryListMenuActive == YES)
		{
			active_menu = SUMMARY_MENU;
			ACTIVATE_MENU_MSG(); mn_msg.data[0] = ESC_KEY;
		}
		else // summaryListMenuActive == NO
		{
			help_rec.copies = *((uint8*)data);
			debug("Copies: %d\n", help_rec.copies);

			saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, COPIES);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(FT_TYPE, M_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&distanceToSourceMenuHandler}}
};

//--------------------------------
// Distance to Source Menu Handler
//--------------------------------
void distanceToSourceMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		trig_rec.trec.dist_to_source = *((float*)data);

		if (help_rec.units_of_measure == METRIC_TYPE)
			trig_rec.trec.dist_to_source *= FT_PER_METER;

		debug("Distance to Source: %f ft\n", trig_rec.trec.dist_to_source);

		if (trig_rec.op_mode == WAVEFORM_MODE)
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&weightPerDelayMenu, &trig_rec.trec.weight_per_delay,
				WEIGHT_PER_DELAY_DEFAULT_VALUE, WEIGHT_PER_DELAY_INCREMENT_VALUE,
				WEIGHT_PER_DELAY_MIN_VALUE, WEIGHT_PER_DELAY_MAX_VALUE);
		}
		else if (trig_rec.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_MSG(&operatorMenu, &trig_rec.trec.oper);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&notesMenu, &trig_rec.trec.comments);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&lcdImpulseTimeMenuHandler}}
};

//------------------------------
// Lcd Impulse Time Menu Handler
//------------------------------
void lcdImpulseTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		trig_rec.berec.impulseMenuUpdateSecs = *((uint8*)data);

		debug("LCD Impulse Menu Update Seconds: %d\n", trig_rec.berec.impulseMenuUpdateSecs);

		saveRecData(&trig_rec, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&barResultMenu, help_rec.vector_sum);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&summaryIntervalMenu, trig_rec.bgrec.summaryInterval);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(MINS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&lcdTimeoutMenuHandler}}
};

//-------------------------
// Lcd Timeout Menu Handler
//-------------------------
void lcdTimeoutMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		help_rec.lcd_timeout = *((uint8*)data);

		debug("LCD Timeout: %d\n", help_rec.lcd_timeout);

		saveRecData(&help_rec, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, LCD_TIMEOUT);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_MODEM_DIAL_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemDialMenuHandler}}
};

//------------------------
// Modem Dial Menu Handler
//------------------------
void modemDialMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&modem_setup_rec.dial), (char*)data);
		debug("Modem Dial: <%s>, Length: %d\n", modem_setup_rec.dial, strlen((char*)modem_setup_rec.dial));

		ACTIVATE_USER_MENU_MSG(&modemResetMenu, &modem_setup_rec.reset);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemInitMenu, &modem_setup_rec.init);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_MODEM_INIT_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemInitMenuHandler}}
};

//------------------------
// Modem Init Menu Handler
//------------------------
void modemInitMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&modem_setup_rec.init), (char*)data);
		debug("Modem Init: <%s>, Length: %d\n", modem_setup_rec.init, strlen((char*)modem_setup_rec.init));

		ACTIVATE_USER_MENU_MSG(&modemDialMenu, &modem_setup_rec.dial);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemSetupMenu, modem_setup_rec.modemStatus);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_MODEM_RESET_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemResetMenuHandler}}
};

//-------------------------
// Modem Reset Menu Handler
//-------------------------
void modemResetMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&modem_setup_rec.reset), (char*)data);
		debug("Modem Reset: <%s>, Length: %d\n", modem_setup_rec.reset, strlen((char*)modem_setup_rec.reset));

		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryMenu, &modem_setup_rec.retries,
			MODEM_RETRY_DEFAULT_VALUE, MODEM_RETRY_MIN_VALUE, MODEM_RETRY_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemDialMenu, &modem_setup_rec.dial);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(NO_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemRetryMenuHandler}}
};

//-------------------------
// Modem Retry Menu Handler
//-------------------------
void modemRetryMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		modem_setup_rec.retries = *((uint8*)data);
		debug("Modem Retries: %d\n", modem_setup_rec.retries);
		
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryTimeMenu, &modem_setup_rec.retryTime,
			MODEM_RETRY_TIME_DEFAULT_VALUE, MODEM_RETRY_TIME_MIN_VALUE, MODEM_RETRY_TIME_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&modemResetMenu, &modem_setup_rec.reset);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(MINS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemRetryTimeMenuHandler}}
};

//------------------------------
// Modem Retry Time Menu Handler
//------------------------------
void modemRetryTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		modem_setup_rec.retryTime = *((uint8*)data);
		debug("Modem Retry Time: %d\n", modem_setup_rec.retryTime);
		
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&unlockCodeMenu, &modem_setup_rec.unlockCode,
			UNLOCK_CODE_DEFAULT_VALUE, UNLOCK_CODE_MIN_VALUE, UNLOCK_CODE_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryMenu, &modem_setup_rec.retries,
			MODEM_RETRY_DEFAULT_VALUE, MODEM_RETRY_MIN_VALUE, MODEM_RETRY_MAX_VALUE);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_NOTES_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&notesMenuHandler}}
};

//-------------------
// Notes Menu Handler
//-------------------
void notesMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&trig_rec.trec.comments), (char*)data);
		debug("Notes: <%s>, Length: %d\n", trig_rec.trec.comments, strlen((char*)trig_rec.trec.comments));

		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&distanceToSourceMenu, &trig_rec.trec.dist_to_source,
			DISTANCE_TO_SOURCE_DEFAULT_VALUE, DISTANCE_TO_SOURCE_INCREMENT_VALUE,
			DISTANCE_TO_SOURCE_MIN_VALUE, DISTANCE_TO_SOURCE_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&seismicLocationMenu, &trig_rec.trec.loc);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_OPERATOR_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&operatorMenuHandler}}
};

//----------------------
// Operator Menu Handler
//----------------------
void operatorMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&trig_rec.trec.oper), (char*)data);
		debug("Operator: <%s>, Length: %d\n", trig_rec.trec.oper, strlen((char*)trig_rec.trec.oper));

		if (factory_setup_rec.sensor_type == SENSOR_ACC)
		{
			sprintf((char*)&menuTags[LOW_SENSITIVITY_MAX_TAG].text, " (%.0fmg)", 
					(float)factory_setup_rec.sensor_type / (float)200);
			sprintf((char*)&menuTags[HIGH_SENSITIVITY_MAX_TAG].text, " (%.0fmg)", 
					(float)factory_setup_rec.sensor_type / (float)400);
		}
		else if (help_rec.units_of_measure == IMPERIAL)
		{
			sprintf((char*)&menuTags[LOW_SENSITIVITY_MAX_TAG].text, " (%.2fin)", 
					(float)factory_setup_rec.sensor_type / (float)200);
			sprintf((char*)&menuTags[HIGH_SENSITIVITY_MAX_TAG].text, " (%.2fin)", 
					(float)factory_setup_rec.sensor_type / (float)400);
		}
		else // help_rec.units_of_measure == METRIC
		{
			sprintf((char*)&menuTags[LOW_SENSITIVITY_MAX_TAG].text, " (%.2fmm)", 
					(float)factory_setup_rec.sensor_type * (float)25.4 / (float)200);
			sprintf((char*)&menuTags[HIGH_SENSITIVITY_MAX_TAG].text, " (%.2fmm)", 
					(float)factory_setup_rec.sensor_type * (float)25.4 / (float)400);
		}

		ACTIVATE_USER_MENU_MSG(&sensitivityMenu, trig_rec.srec.sensitivity);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (trig_rec.op_mode == WAVEFORM_MODE)
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&weightPerDelayMenu, &trig_rec.trec.weight_per_delay,
				WEIGHT_PER_DELAY_DEFAULT_VALUE, WEIGHT_PER_DELAY_INCREMENT_VALUE,
				WEIGHT_PER_DELAY_MIN_VALUE, WEIGHT_PER_DELAY_MAX_VALUE);
		}
		else if (trig_rec.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&distanceToSourceMenu, &trig_rec.trec.dist_to_source,
				DISTANCE_TO_SOURCE_DEFAULT_VALUE, DISTANCE_TO_SOURCE_INCREMENT_VALUE,
				DISTANCE_TO_SOURCE_MIN_VALUE, DISTANCE_TO_SOURCE_MAX_VALUE);
		}
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(SECS_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&recordTimeMenuHandler}}
};

//-------------------------
// Record Time Menu Handler
//-------------------------
void recordTimeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		trig_rec.trec.record_time = *((uint32*)data);
		debug("Record Time: %d\n", trig_rec.trec.record_time);

		if ((!factory_setup_rec.invalid) && (factory_setup_rec.aweight_option == ENABLED))
		{
			ACTIVATE_USER_MENU_MSG(&airScaleMenu, 0);
		}
		else
		{
			// If alarm mode is off, the proceed to save setup
			if ((help_rec.alarm_one_mode == ALARM_MODE_OFF) && (help_rec.alarm_two_mode == ALARM_MODE_OFF))
			{
				ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
			}
			else // Goto Alarm setup menus
			{
				ACTIVATE_USER_MENU_MSG(&alarmOneMenu, help_rec.alarm_one_mode);
			}
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&airTriggerMenu, &trig_rec.trec.soundTriggerLevel,
			AIR_TRIGGER_DEFAULT_VALUE, AIR_TRIGGER_MIN_VALUE, AIR_TRIGGER_MAX_VALUE);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_SAVE_RECORD_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&saveRecordMenuHandler}}
};

//-------------------------
// Save Record Menu Handler
//-------------------------
void saveRecordMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
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
					strcpy((char*)trig_rec.name, (char*)data);

					// Save the trigger record in the available location
					saveRecData(&trig_rec, availableLocation, REC_TRIGGER_USER_MENU_TYPE);

					// Also save the trigger record in the default record for future use
					saveRecData(&trig_rec, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

					// Update the title based on the current mode
					updateModeMenuTitle(trig_rec.op_mode);
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
				strcpy((char*)trig_rec.name, (char*)data);

				// Out of empty record slots, goto the Overwrite menu
				active_menu = OVERWRITE_MENU;
				ACTIVATE_MENU_WITH_DATA_MSG(trig_rec.op_mode);
			}
			else // Found an empty record slot
			{
				// Copy over the record name
				strcpy((char*)trig_rec.name, (char*)data);

				// Save the trigger record in the available location
				saveRecData(&trig_rec, availableLocation, REC_TRIGGER_USER_MENU_TYPE);

				// Also save the trigger record in the default record for future use
				saveRecData(&trig_rec, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

				// Update the title based on the current mode
				updateModeMenuTitle(trig_rec.op_mode);
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

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_SEISMIC_LOCATION_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&seismicLocationMenuHandler}}
};

//------------------------------
// Seismic Location Menu Handler
//------------------------------
void seismicLocationMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		strcpy((char*)(&trig_rec.trec.loc), (char*)data);
		debug("Seismic Location: <%s>, Length: %d\n", trig_rec.trec.loc, strlen((char*)trig_rec.trec.loc));

		ACTIVATE_USER_MENU_MSG(&notesMenu, &trig_rec.trec.comments);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&companyMenu, &trig_rec.trec.client);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(IN_TYPE, MM_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&seismicTriggerMenuHandler}}
};

//-----------------------------
// Seismic Trigger Menu Handler
//-----------------------------
void seismicTriggerMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		trig_rec.trec.seismicTriggerLevel = *((uint32*)data);

		if (trig_rec.trec.seismicTriggerLevel == NO_TRIGGER_CHAR)
		{
			debug("Seismic Trigger: No Trigger\n");
		}
		else
		{
			debug("Seismic Trigger: %d counts\n", trig_rec.trec.seismicTriggerLevel);
		}

		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&airTriggerMenu, &trig_rec.trec.soundTriggerLevel,
			AIR_TRIGGER_DEFAULT_VALUE, AIR_TRIGGER_MIN_VALUE, AIR_TRIGGER_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&sensitivityMenu, trig_rec.srec.sensitivity);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {MAX_SERIAL_NUMBER_CHARS}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&serialNumberMenuHandler}}
};

//---------------------------
// Serial Number Menu Handler
//---------------------------
void serialNumberMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		debug("Serial #: <%s>, Length: %d\n", (char*)data, strlen((char*)data));
		strcpy((char*)factory_setup_rec.serial_num, (char*)data);

		ACTIVATE_USER_MENU_MSG(&sensorTypeMenu, factory_setup_rec.sensor_type);
	}
	else if (keyPressed == ESC_KEY)
	{
		active_menu = DATE_TIME_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[active_menu]) (mn_msg);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(LBS_TYPE, KG_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&weightPerDelayMenuHandler}}
};

//------------------------------
// Weight per Delay Menu Handler
//------------------------------
void weightPerDelayMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		trig_rec.trec.weight_per_delay = *((float*)data);

		if (help_rec.units_of_measure == METRIC_TYPE)
			trig_rec.trec.weight_per_delay *= LBS_PER_KG;

		debug("Weight per Delay: %.1f lbs\n", trig_rec.trec.weight_per_delay);

		ACTIVATE_USER_MENU_MSG(&operatorMenu, &trig_rec.trec.oper);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&distanceToSourceMenu, &trig_rec.trec.dist_to_source,
			DISTANCE_TO_SOURCE_DEFAULT_VALUE, DISTANCE_TO_SOURCE_INCREMENT_VALUE,
			DISTANCE_TO_SOURCE_MIN_VALUE, DISTANCE_TO_SOURCE_MAX_VALUE);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
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
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG, {INSERT_USER_MENU_WORD_DATA(NO_TYPE, NO_ALT_TYPE)}},
{NO_TAG, 0, (uint8)NULL_TEXT, NO_TAG},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&unlockCodeMenuHandler}}
};

//-------------------------
// Unlock Code Menu Handler
//-------------------------
void unlockCodeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	
	if (keyPressed == ENTER_KEY)
	{	
		modem_setup_rec.unlockCode = *((uint16*)data);

		debug("Modem Unlock Code: %4d\n", modem_setup_rec.unlockCode);

		saveRecData(&modem_setup_rec, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);

#if 0 // fix_ns8100
		if (READ_DCD == NO_CONNECTION)
		{
			craftInitStatusFlags();
		}
#endif
		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&modemRetryTimeMenu, &modem_setup_rec.retryTime,
			MODEM_RETRY_TIME_DEFAULT_VALUE, MODEM_RETRY_TIME_MIN_VALUE, MODEM_RETRY_TIME_MAX_VALUE);
	}

	(*menufunc_ptrs[active_menu]) (mn_msg);
}
