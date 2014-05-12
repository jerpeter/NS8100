///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: UserSelectMenus.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:10:03 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/UserSelectMenus.c,v $
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
#include "EventProcessing.h"
#include "InitDataBuffers.h"
#include "Old_Board.h"
#include "RemoteCommon.h"
#include "PowerManagement.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT alarmOneMenu[];
extern USER_MENU_STRUCT alarmTwoMenu[];
extern USER_MENU_STRUCT alarmOneTimeMenu[];
extern USER_MENU_STRUCT alarmTwoTimeMenu[];
extern USER_MENU_STRUCT alarmOneSeismicLevelMenu[];
extern USER_MENU_STRUCT alarmOneAirLevelMenu[];
extern USER_MENU_STRUCT alarmTwoSeismicLevelMenu[];
extern USER_MENU_STRUCT alarmTwoAirLevelMenu[];
extern USER_MENU_STRUCT barChannelMenu[];
extern USER_MENU_STRUCT barIntervalMenu[];
extern USER_MENU_STRUCT barScaleMenu[];
extern USER_MENU_STRUCT barResultMenu[];
extern USER_MENU_STRUCT baudRateMenu[];
extern USER_MENU_STRUCT companyMenu[];
extern USER_MENU_STRUCT copiesMenu[];
extern USER_MENU_STRUCT configMenu[];
extern USER_MENU_STRUCT displacementMenu[];
extern USER_MENU_STRUCT eraseEventsMenu[];
extern USER_MENU_STRUCT eraseSettingsMenu[];
extern USER_MENU_STRUCT flashWrappingMenu[];
extern USER_MENU_STRUCT freqPlotMenu[];
extern USER_MENU_STRUCT freqPlotStandardMenu[];
extern USER_MENU_STRUCT helpMenu[];
extern USER_MENU_STRUCT infoMenu[];
extern USER_MENU_STRUCT languageMenu[];
extern USER_MENU_STRUCT lcdImpulseTimeMenu[];
extern USER_MENU_STRUCT lcdTimeoutMenu[];
extern USER_MENU_STRUCT millibarMenu[];
extern USER_MENU_STRUCT modemSetupMenu[];
extern USER_MENU_STRUCT modemInitMenu[];
extern USER_MENU_STRUCT monitorLogMenu[];
extern USER_MENU_STRUCT operatorMenu[];
extern USER_MENU_STRUCT printerEnableMenu[];
extern USER_MENU_STRUCT printMonitorLogMenu[];
extern USER_MENU_STRUCT recordTimeMenu[];
extern USER_MENU_STRUCT saveRecordMenu[];
extern USER_MENU_STRUCT saveSetupMenu[];
extern USER_MENU_STRUCT sampleRateMenu[];
extern USER_MENU_STRUCT seismicTriggerMenu[];
extern USER_MENU_STRUCT sensitivityMenu[];
extern USER_MENU_STRUCT sensorTypeMenu[];
extern USER_MENU_STRUCT serialNumberMenu[];
extern USER_MENU_STRUCT summaryIntervalMenu[];
extern USER_MENU_STRUCT timerModeFreqMenu[];
extern USER_MENU_STRUCT timerModeMenu[];
extern USER_MENU_STRUCT unitsMenu[];
extern USER_MENU_STRUCT vectorSumMenu[];
extern USER_MENU_STRUCT waveformAutoCalMenu[];
extern USER_MENU_STRUCT zeroEventNumberMenu[];

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

//*****************************************************************************
//=============================================================================
// Air Scale Menu
//=============================================================================
//*****************************************************************************
#define AIR_SCALE_MENU_ENTRIES 4
USER_MENU_STRUCT airScaleMenu[AIR_SCALE_MENU_ENTRIES] = {
{NO_TAG, 0, AIR_CHANNEL_SCALE_TEXT, NO_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, AIR_SCALE_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, LINEAR_TEXT,		NO_TAG,	{1}},
{ITEM_2, 0, A_WEIGHTING_TEXT,	NO_TAG,	{2}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&airScaleMenuHandler}}
};

//-----------------------
// Air Scale Menu Handler
//-----------------------
void airScaleMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	//uint16 newItemIndex = *((uint16*)data);

	// Save Air Scale somewhere?

	if (keyPressed == ENTER_KEY)
	{
		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			// If alarm mode is off, the proceed to save setup
			if ((g_helpRecord.alarm_one_mode == ALARM_MODE_OFF) && (g_helpRecord.alarm_two_mode == ALARM_MODE_OFF))
			{
				ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
			}
			else // Goto Alarm setup menus
			{
				ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
			}
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_MSG(&barIntervalMenu, g_triggerRecord.bgrec.barInterval);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
				RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			ACTIVATE_USER_MENU_MSG(&barChannelMenu, 0);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Air Setup Menu
//=============================================================================
//*****************************************************************************
#define AIR_SETUP_MENU_ENTRIES 4
USER_MENU_STRUCT airSetupMenu[AIR_SETUP_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, A_WEIGHTING_OPTION_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, AIR_SETUP_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, 0, INCLUDED_TEXT,		NO_TAG, {ENABLED}},
{ITEM_2, 0, NOT_INCLUDED_TEXT,	NO_TAG, {DISABLED}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&airSetupMenuHandler}}
};

//-----------------------
// Air Setup Menu Handler
//-----------------------
void airSetupMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_factorySetupRecord.aweight_option = (uint8)airSetupMenu[newItemIndex].data;

		ACTIVATE_USER_MENU_MSG(&millibarMenu, g_printMillibars);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&sensorTypeMenu, g_factorySetupRecord.sensor_type);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm One Menu
//=============================================================================
//*****************************************************************************
#define ALARM_ONE_MENU_ENTRIES 6
USER_MENU_STRUCT alarmOneMenu[ALARM_ONE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_1_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ALARM_ONE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, OFF_TEXT,		NO_TAG, {ALARM_MODE_OFF}},
{ITEM_2, 0, SEISMIC_TEXT,	NO_TAG, {ALARM_MODE_SEISMIC}},
{ITEM_3, 0, AIR_TEXT,		NO_TAG, {ALARM_MODE_AIR}},
{ITEM_4, 0, BOTH_TEXT,		NO_TAG, {ALARM_MODE_BOTH}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOneMenuHandler}}
};

//-----------------------
// Alarm One Menu Handler
//-----------------------
void alarmOneMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.alarm_one_mode = (uint8)alarmOneMenu[newItemIndex].data;

		switch (g_helpRecord.alarm_one_mode)
		{
			case (ALARM_MODE_OFF):
				g_helpRecord.alarm_one_seismic_lvl = NO_TRIGGER_CHAR;
				g_helpRecord.alarm_one_air_lvl = NO_TRIGGER_CHAR;

				ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
			break;

			case (ALARM_MODE_SEISMIC):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR))
				{
					messageBox(getLangText(WARNING_TEXT), "SEISMIC TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
				}
				else
				{
					g_helpRecord.alarm_one_air_lvl = NO_TRIGGER_CHAR;

					// Setup Alarm One Seismic Level
					if (g_helpRecord.alarm_one_seismic_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_one_seismic_lvl = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_one_seismic_min_lvl = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarm_one_seismic_lvl < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarm_one_seismic_lvl = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_one_seismic_min_lvl = ALARM_SEIS_MIN_VALUE;
					}

					// Call Alarm One Seismic Level
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &g_helpRecord.alarm_one_seismic_lvl,
						g_helpRecord.alarm_one_seismic_min_lvl, g_helpRecord.alarm_one_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
				}
			break;

			case (ALARM_MODE_AIR):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.soundTriggerLevel == NO_TRIGGER_CHAR))
				{
					messageBox(getLangText(WARNING_TEXT), "AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
				}
				else
				{
					g_helpRecord.alarm_one_seismic_lvl = NO_TRIGGER_CHAR;

					// Setup Alarm One Air Level
					if (g_helpRecord.alarm_one_air_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_one_air_lvl = ALARM_ONE_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_one_air_min_lvl = (uint32)g_triggerRecord.trec.soundTriggerLevel;

						if (g_helpRecord.alarm_one_air_lvl < (uint32)g_triggerRecord.trec.soundTriggerLevel)
						{
							g_helpRecord.alarm_one_air_lvl = (uint32)g_triggerRecord.trec.soundTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_one_air_min_lvl = ALARM_AIR_MIN_VALUE;
					}

					// Call Alarm One Air Level
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &g_helpRecord.alarm_one_air_lvl,
						g_helpRecord.alarm_one_air_min_lvl, g_helpRecord.alarm_one_air_min_lvl, ALARM_AIR_MAX_VALUE);
				}
			break;

			case (ALARM_MODE_BOTH):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) ||
					(g_triggerRecord.trec.soundTriggerLevel == NO_TRIGGER_CHAR)))
				{
					messageBox(getLangText(WARNING_TEXT), "SEISMIC OR AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
				}
				else
				{
					// Setup Alarm One Seismic Level
					if (g_helpRecord.alarm_one_seismic_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_one_seismic_lvl = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_one_seismic_min_lvl = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarm_one_seismic_lvl < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarm_one_seismic_lvl = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_one_seismic_min_lvl = ALARM_SEIS_MIN_VALUE;
					}

					// Setup Alarm One Air Level
					if (g_helpRecord.alarm_one_air_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_one_air_lvl = ALARM_ONE_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_one_air_min_lvl = (uint16)g_triggerRecord.trec.soundTriggerLevel;

						if (g_helpRecord.alarm_one_air_lvl < (uint32)g_triggerRecord.trec.soundTriggerLevel)
						{
							g_helpRecord.alarm_one_air_lvl = (uint32)g_triggerRecord.trec.soundTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_one_air_min_lvl = ALARM_AIR_MIN_VALUE;
					}

					// Call Alarm One Seismic Level
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &g_helpRecord.alarm_one_seismic_lvl,
						g_helpRecord.alarm_one_seismic_min_lvl, g_helpRecord.alarm_one_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
				}
			break;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if ((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			if ((!g_factorySetupRecord.invalid) && (g_factorySetupRecord.aweight_option == ENABLED))
			{
				ACTIVATE_USER_MENU_MSG(&airScaleMenu, 0);
			}
			else
			{
				ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
					RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
			}
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
		     ACTIVATE_USER_MENU_MSG(&barResultMenu, g_helpRecord.vector_sum);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm Two Menu
//=============================================================================
//*****************************************************************************
#define ALARM_TWO_MENU_ENTRIES 6
USER_MENU_STRUCT alarmTwoMenu[ALARM_TWO_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_2_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ALARM_TWO_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, OFF_TEXT,		NO_TAG, {ALARM_MODE_OFF}},
{ITEM_2, 0, SEISMIC_TEXT,	NO_TAG, {ALARM_MODE_SEISMIC}},
{ITEM_3, 0, AIR_TEXT,		NO_TAG, {ALARM_MODE_AIR}},
{ITEM_4, 0, BOTH_TEXT,		NO_TAG, {ALARM_MODE_BOTH}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmTwoMenuHandler}}
};

//-----------------------
// Alarm Two Menu Handler
//-----------------------
void alarmTwoMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.alarm_two_mode = (uint8)alarmTwoMenu[newItemIndex].data;

		switch (g_helpRecord.alarm_two_mode)
		{
			case (ALARM_MODE_OFF):
				g_helpRecord.alarm_two_seismic_lvl = NO_TRIGGER_CHAR;
				g_helpRecord.alarm_two_air_lvl = NO_TRIGGER_CHAR;

				saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

				ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
			break;

			case (ALARM_MODE_SEISMIC):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR))
				{
					messageBox(getLangText(WARNING_TEXT), "SEISMIC TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
				}
				else
				{
					g_helpRecord.alarm_two_air_lvl = NO_TRIGGER_CHAR;

					// Setup Alarm Two Seismic Level
					if (g_helpRecord.alarm_two_seismic_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_two_seismic_lvl = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_two_seismic_min_lvl = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarm_two_seismic_lvl < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarm_two_seismic_lvl = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_two_seismic_min_lvl = ALARM_SEIS_MIN_VALUE;
					}

					// Call Alarm Two Seismic Level
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &g_helpRecord.alarm_two_seismic_lvl,
						g_helpRecord.alarm_two_seismic_min_lvl, g_helpRecord.alarm_two_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
				}
			break;

			case (ALARM_MODE_AIR):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.soundTriggerLevel == NO_TRIGGER_CHAR))
				{
					messageBox(getLangText(WARNING_TEXT), "AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
				}
				else
				{
					g_helpRecord.alarm_two_seismic_lvl = NO_TRIGGER_CHAR;

					// Setup Alarm Two Air Level
					if (g_helpRecord.alarm_two_air_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_two_air_lvl = ALARM_TWO_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_two_air_min_lvl = (uint16)g_triggerRecord.trec.soundTriggerLevel;

						if (g_helpRecord.alarm_two_air_lvl < (uint32)g_triggerRecord.trec.soundTriggerLevel)
						{
							g_helpRecord.alarm_two_air_lvl = (uint32)g_triggerRecord.trec.soundTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_two_air_min_lvl = ALARM_AIR_MIN_VALUE;
					}

					// Call Alarm One Air Level
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &g_helpRecord.alarm_two_air_lvl,
						g_helpRecord.alarm_two_air_min_lvl, g_helpRecord.alarm_two_air_min_lvl, ALARM_AIR_MAX_VALUE);
				}
			break;

			case (ALARM_MODE_BOTH):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) ||
					(g_triggerRecord.trec.soundTriggerLevel == NO_TRIGGER_CHAR)))
				{
					messageBox(getLangText(WARNING_TEXT), "SEISMIC OR AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
				}
				else
				{
					// Setup Alarm Two Seismic Level
					if (g_helpRecord.alarm_two_seismic_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_two_seismic_lvl = ALARM_TWO_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_two_seismic_min_lvl = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarm_two_seismic_lvl < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarm_two_seismic_lvl = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_two_seismic_min_lvl = ALARM_SEIS_MIN_VALUE;
					}

					// Setup Alarm Two Air Level
					if (g_helpRecord.alarm_two_air_lvl == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarm_two_air_lvl = ALARM_TWO_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarm_two_air_min_lvl = (uint16)g_triggerRecord.trec.soundTriggerLevel;

						if (g_helpRecord.alarm_two_air_lvl < (uint32)g_triggerRecord.trec.soundTriggerLevel)
						{
							g_helpRecord.alarm_two_air_lvl = (uint32)g_triggerRecord.trec.soundTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarm_two_air_min_lvl = ALARM_AIR_MIN_VALUE;
					}

					// Call Alarm Two Seismic Level
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &g_helpRecord.alarm_two_seismic_lvl,
						g_helpRecord.alarm_two_seismic_min_lvl, g_helpRecord.alarm_two_seismic_min_lvl, ALARM_SEIS_MAX_VALUE);
				}
			break;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_helpRecord.alarm_one_mode == ALARM_MODE_OFF)
		{
			ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
		}
		else
		{
			ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmOneTimeMenu, &g_helpRecord.alarm_one_time,
				ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT,
				ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Alarm Output Menu
//=============================================================================
//*****************************************************************************
#define ALARM_OUTPUT_MENU_ENTRIES 4
USER_MENU_STRUCT alarmOutputMenu[ALARM_OUTPUT_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ALARM_OUTPUT_MODE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ALARM_OUTPUT_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, DISABLED_TEXT,	NO_TAG, {DISABLED}},
{ITEM_2, 0, ENABLED_TEXT,	NO_TAG, {ENABLED}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&alarmOutputMenuHandler}}
};

//--------------------------
// Alarm Output Menu Handler
//--------------------------
void alarmOutputMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.alarm_one_mode = (uint8)alarmOutputMenu[newItemIndex].data;
		g_helpRecord.alarm_two_mode = (uint8)alarmOutputMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, ALARM_OUTPUT_MODE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Auto Cal Menu
//=============================================================================
//*****************************************************************************
#define AUTO_CAL_MENU_ENTRIES 6
USER_MENU_STRUCT autoCalMenu[AUTO_CAL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, AUTO_CALIBRATION_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, AUTO_CAL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_4)}},
{ITEM_1, 0, AFTER_EVERY_24_HRS_TEXT,	NO_TAG, {AUTO_24_HOUR_TIMEOUT}},
{ITEM_2, 0, AFTER_EVERY_48_HRS_TEXT,	NO_TAG, {AUTO_48_HOUR_TIMEOUT}},
{ITEM_3, 0, AFTER_EVERY_72_HRS_TEXT,	NO_TAG, {AUTO_72_HOUR_TIMEOUT}},
{ITEM_4, 0, NO_AUTO_CAL_TEXT,			NO_TAG, {AUTO_NO_CAL_TIMEOUT}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&autoCalMenuHandler}}
};

//----------------------
// Auto Cal Menu Handler
//----------------------
void autoCalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.auto_cal_mode = (uint8)autoCalMenu[newItemIndex].data;

		if (g_helpRecord.auto_cal_mode == AUTO_NO_CAL_TIMEOUT)
		{
			g_autoCalDaysToWait = 0;
		}
		else // Auto Cal enabled, set to process the first midnight
		{
			g_autoCalDaysToWait = 1;
		}

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, AUTO_CALIBRATION);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Auto Monitor Menu
//=============================================================================
//*****************************************************************************
#define AUTO_MONITOR_MENU_ENTRIES 6
USER_MENU_STRUCT autoMonitorMenu[AUTO_MONITOR_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, AUTO_MONITOR_AFTER_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, AUTO_MONITOR_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_4)}},
{ITEM_1, 2, MINUTES_TEXT,			NO_TAG, {AUTO_TWO_MIN_TIMEOUT}},
{ITEM_2, 3, MINUTES_TEXT,			NO_TAG, {AUTO_THREE_MIN_TIMEOUT}},
{ITEM_3, 4, MINUTES_TEXT,			NO_TAG, {AUTO_FOUR_MIN_TIMEOUT}},
{ITEM_4, 0, NO_AUTO_MONITOR_TEXT,	NO_TAG, {AUTO_NO_TIMEOUT}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&autoMonitorMenuHandler}}
};

//--------------------------
// Auto Monitor Menu Handler
//--------------------------
void autoMonitorMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.auto_monitor_mode = (uint8)autoMonitorMenu[newItemIndex].data;

		assignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_helpRecord.auto_monitor_mode * TICKS_PER_MIN), autoMonitorTimerCallBack);

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, AUTO_MONITOR);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Bar Channel Menu
//=============================================================================
//*****************************************************************************
#define BAR_CHANNEL_MENU_ENTRIES 5
USER_MENU_STRUCT barChannelMenu[BAR_CHANNEL_MENU_ENTRIES] = {
{NO_TAG, 0, MONITOR_BARGRAPH_TEXT, NO_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BAR_CHANNEL_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, BOTH_TEXT,		NO_TAG, {BAR_BOTH_CHANNELS}},
{ITEM_2, 0, SEISMIC_TEXT,	NO_TAG, {BAR_SEISMIC_CHANNEL}},
{ITEM_3, 0, AIR_TEXT,		NO_TAG, {BAR_AIR_CHANNEL}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&barChannelMenuHandler}}
};

//-------------------------
// Bar Channel Menu Handler
//-------------------------
void barChannelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);
	uint16 gainFactor = (uint16)((g_triggerRecord.srec.sensitivity == LOW) ? 200 : 400);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.berec.barChannel = (uint8)barChannelMenu[newItemIndex].data;

		if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
		{
			sprintf((char*)&g_menuTags[BAR_SCALE_FULL_TAG].text, "100%% (%.0f mg)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_FULL));
			sprintf((char*)&g_menuTags[BAR_SCALE_HALF_TAG].text, " 50%% (%.0f mg)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_HALF));
			sprintf((char*)&g_menuTags[BAR_SCALE_QUARTER_TAG].text, " 25%% (%.0f mg)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_QUARTER));
			sprintf((char*)&g_menuTags[BAR_SCALE_EIGHTH_TAG].text, " 12%% (%.0f mg)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_EIGHTH));
		}
		else if (g_helpRecord.units_of_measure == IMPERIAL)
		{
			sprintf((char*)&g_menuTags[BAR_SCALE_FULL_TAG].text, "100%% (%.2f in/s)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_FULL));
			sprintf((char*)&g_menuTags[BAR_SCALE_HALF_TAG].text, " 50%% (%.2f in/s)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_HALF));
			sprintf((char*)&g_menuTags[BAR_SCALE_QUARTER_TAG].text, " 25%% (%.2f in/s)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_QUARTER));
			sprintf((char*)&g_menuTags[BAR_SCALE_EIGHTH_TAG].text, " 12%% (%.2f in/s)",
					(float)g_factorySetupRecord.sensor_type / (float)(gainFactor * BAR_SCALE_EIGHTH));
		}
		else // g_helpRecord.units_of_measure == METRIC
		{
			sprintf((char*)&g_menuTags[BAR_SCALE_FULL_TAG].text, "100%% (%lu mm/s)",
					(uint32)((float)g_factorySetupRecord.sensor_type * (float)25.4 /
					(float)(gainFactor * BAR_SCALE_FULL)));
			sprintf((char*)&g_menuTags[BAR_SCALE_HALF_TAG].text, " 50%% (%lu mm/s)",
					(uint32)((float)g_factorySetupRecord.sensor_type * (float)25.4 /
					(float)(gainFactor * BAR_SCALE_HALF)));
			sprintf((char*)&g_menuTags[BAR_SCALE_QUARTER_TAG].text, " 25%% (%lu mm/s)",
					(uint32)((float)g_factorySetupRecord.sensor_type * (float)25.4 /
					(float)(gainFactor * BAR_SCALE_QUARTER)));
			sprintf((char*)&g_menuTags[BAR_SCALE_EIGHTH_TAG].text, " 12%% (%lu mm/s)",
					(uint32)((float)g_factorySetupRecord.sensor_type * (float)25.4 /
					(float)(gainFactor * BAR_SCALE_EIGHTH)));
		}

		ACTIVATE_USER_MENU_MSG(&barScaleMenu, g_triggerRecord.berec.barScale);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&sensitivityMenu, g_triggerRecord.srec.sensitivity);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Bar Interval Menu
//=============================================================================
//*****************************************************************************
#define BAR_INTERVAL_MENU_ENTRIES 9
USER_MENU_STRUCT barIntervalMenu[BAR_INTERVAL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, BAR_INTERVAL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BAR_INTERVAL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_7)}},
{ITEM_1, 1,  SECOND_TEXT,	NO_TAG, {ONE_SEC_PRD}},
{ITEM_2, 10, SECONDS_TEXT,	NO_TAG, {TEN_SEC_PRD}},
{ITEM_3, 20, SECONDS_TEXT,	NO_TAG, {TWENTY_SEC_PRD}},
{ITEM_4, 30, SECONDS_TEXT,	NO_TAG, {THIRTY_SEC_PRD}},
{ITEM_5, 40, SECONDS_TEXT,	NO_TAG, {FOURTY_SEC_PRD}},
{ITEM_6, 50, SECONDS_TEXT,	NO_TAG, {FIFTY_SEC_PRD}},
{ITEM_7, 60, SECONDS_TEXT,	NO_TAG, {SIXTY_SEC_PRD}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&barIntervalMenuHandler}}
};

//--------------------------
// Bar Interval Menu Handler
//--------------------------
void barIntervalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.bgrec.barInterval = barIntervalMenu[newItemIndex].data;

		ACTIVATE_USER_MENU_MSG(&summaryIntervalMenu, g_triggerRecord.bgrec.summaryInterval);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&barScaleMenu, g_triggerRecord.berec.barScale);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Bar Scale Menu
//=============================================================================
//*****************************************************************************
#define BAR_SCALE_MENU_ENTRIES 6
USER_MENU_STRUCT barScaleMenu[BAR_SCALE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, BAR_SCALE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BAR_SCALE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, NULL_TEXT,	BAR_SCALE_FULL_TAG,		{BAR_SCALE_FULL}},
{ITEM_2, 0, NULL_TEXT,	BAR_SCALE_HALF_TAG,		{BAR_SCALE_HALF}},
{ITEM_3, 0, NULL_TEXT,	BAR_SCALE_QUARTER_TAG,	{BAR_SCALE_QUARTER}},
{ITEM_4, 0, NULL_TEXT,	BAR_SCALE_EIGHTH_TAG,	{BAR_SCALE_EIGHTH}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&barScaleMenuHandler}}
};

//--------------------------
// Bar Scale Menu Handler
//--------------------------
void barScaleMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.berec.barScale = (uint8)(barScaleMenu[newItemIndex].data);

		ACTIVATE_USER_MENU_MSG(&barIntervalMenu, g_triggerRecord.bgrec.barInterval);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&barChannelMenu, g_triggerRecord.berec.barChannel);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Summary Interval Menu
//=============================================================================
//*****************************************************************************
#define SUMMARY_INTERVAL_MENU_ENTRIES 10
USER_MENU_STRUCT summaryIntervalMenu[SUMMARY_INTERVAL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SUMMARY_INTERVAL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SUMMARY_INTERVAL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_4)}},
{ITEM_1, 5,  MINUTES_TEXT,	NO_TAG, {FIVE_MINUTE_INTVL}},
{ITEM_2, 15, MINUTES_TEXT,	NO_TAG, {FIFTEEN_MINUTE_INTVL}},
{ITEM_3, 30, MINUTES_TEXT,	NO_TAG, {THIRTY_MINUTE_INTVL}},
{ITEM_4, 1,  HOUR_TEXT,		NO_TAG, {ONE_HOUR_INTVL}},
{ITEM_5, 2,  HOURS_TEXT,	NO_TAG, {TWO_HOUR_INTVL}},
{ITEM_6, 4,  HOURS_TEXT,	NO_TAG, {FOUR_HOUR_INTVL}},
{ITEM_7, 8,  HOURS_TEXT,	NO_TAG, {EIGHT_HOUR_INTVL}},
{ITEM_8, 12, HOURS_TEXT,	NO_TAG, {TWELVE_HOUR_INTVL}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&summaryIntervalMenuHandler}}
};

//------------------------------
// Summary Interval Menu Handler
//------------------------------
void summaryIntervalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.bgrec.summaryInterval = summaryIntervalMenu[newItemIndex].data;

		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&lcdImpulseTimeMenu, &g_triggerRecord.berec.impulseMenuUpdateSecs,
			LCD_IMPULSE_TIME_DEFAULT_VALUE, LCD_IMPULSE_TIME_MIN_VALUE, LCD_IMPULSE_TIME_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&barIntervalMenu, g_triggerRecord.bgrec.barInterval);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Bar Result Menu
//=============================================================================
//*****************************************************************************
#define BAR_RESULT_MENU_ENTRIES 4
USER_MENU_STRUCT barResultMenu[BAR_RESULT_MENU_ENTRIES] = {
{NO_TAG, 0, BAR_PRINTOUT_RESULTS_TEXT, NO_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BAR_RESULT_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, PEAK_TEXT,			NO_TAG, {BAR_RESULT_PEAK}},
{ITEM_2, 0, VECTOR_SUM_TEXT,	NO_TAG, {BAR_RESULT_VECTOR_SUM}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&barResultMenuHandler}}
};

//------------------------
// Bar Result Menu Handler
//------------------------
void barResultMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.vector_sum = (uint8)barResultMenu[newItemIndex].data;
		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		// If Combo mode, jump back over to waveform specific settings
		if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
			{
				USER_MENU_DEFAULT_TYPE(seismicTriggerMenu) = MG_TYPE;
				USER_MENU_ALT_TYPE(seismicTriggerMenu) = MG_TYPE;
			}
			else
			{
				USER_MENU_DEFAULT_TYPE(seismicTriggerMenu) = IN_TYPE;
				USER_MENU_ALT_TYPE(seismicTriggerMenu) = MM_TYPE;
			}

			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &g_triggerRecord.trec.seismicTriggerLevel,
				SEISMIC_TRIGGER_DEFAULT_VALUE, SEISMIC_TRIGGER_MIN_VALUE, SEISMIC_TRIGGER_MAX_VALUE);
		}
		else if ((g_helpRecord.alarm_one_mode == ALARM_MODE_OFF) && (g_helpRecord.alarm_two_mode == ALARM_MODE_OFF))
		{
			ACTIVATE_USER_MENU_MSG(&saveSetupMenu, YES);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarm_one_mode);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&lcdImpulseTimeMenu, &g_triggerRecord.berec.impulseMenuUpdateSecs,
			LCD_IMPULSE_TIME_DEFAULT_VALUE, LCD_IMPULSE_TIME_MIN_VALUE, LCD_IMPULSE_TIME_MAX_VALUE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Baud Rate Menu
//=============================================================================
//*****************************************************************************
#define BAUD_RATE_MENU_ENTRIES 5
USER_MENU_STRUCT baudRateMenu[BAUD_RATE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, BAUD_RATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BAUD_RATE_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_1)}},
{ITEM_1, 38400, NULL_TEXT,	NO_TAG, {BAUD_RATE_38400}},
{ITEM_2, 19200, NULL_TEXT,	NO_TAG, {BAUD_RATE_19200}},
{ITEM_3, 9600, NULL_TEXT,	NO_TAG, {BAUD_RATE_9600}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&baudRateMenuHandler}}
};

//-----------------------
// Baud Rate Menu Handler
//-----------------------
void baudRateMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (g_helpRecord.baud_rate != baudRateMenu[newItemIndex].data)
		{
			g_helpRecord.baud_rate = (uint8)baudRateMenu[newItemIndex].data;

			switch (baudRateMenu[newItemIndex].data)
			{
#if 0 // fix_ns8100
				case BAUD_RATE_38400:
					uart_init(38400, CRAFT_COM_PORT);
					break;

				case BAUD_RATE_19200:
					uart_init(19200, CRAFT_COM_PORT);
					break;

				case BAUD_RATE_9600:
					uart_init(9600, CRAFT_COM_PORT);
					break;
#endif
			}

			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
		}

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, BAUD_RATE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Config Menu
//=============================================================================
//*****************************************************************************
#define CONFIG_MENU_ENTRIES 30
USER_MENU_STRUCT configMenu[CONFIG_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, CONFIG_OPTIONS_MENU_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, CONFIG_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{NO_TAG, 0, ALARM_OUTPUT_MODE_TEXT,		NO_TAG, {ALARM_OUTPUT_MODE}},
{NO_TAG, 0, AUTO_CALIBRATION_TEXT,		NO_TAG, {AUTO_CALIBRATION}},
{NO_TAG, 0, AUTO_DIAL_INFO_TEXT,		NO_TAG, {AUTO_DIAL_INFO}},
{NO_TAG, 0, AUTO_MONITOR_TEXT,			NO_TAG, {AUTO_MONITOR}},
{NO_TAG, 0, BATTERY_TEXT,				NO_TAG, {BATTERY}},
{NO_TAG, 0, BAUD_RATE_TEXT,				NO_TAG, {BAUD_RATE}},
{NO_TAG, 0, CALIBRATION_DATE_TEXT,		NO_TAG, {CALIBRATION_DATE}},
{NO_TAG, 0, COPIES_TEXT,				NO_TAG, {COPIES}},
{NO_TAG, 0, DATE_TIME_TEXT,				NO_TAG, {DATE_TIME}},
{NO_TAG, 0, ERASE_MEMORY_TEXT,			NO_TAG, {ERASE_FLASH}},
{NO_TAG, 0, FLASH_WRAPPING_TEXT,		NO_TAG, {FLASH_WRAPPING}},
{NO_TAG, 0, FLASH_STATS_TEXT,			NO_TAG, {FLASH_STATS}},
{NO_TAG, 0, FREQUENCY_PLOT_TEXT,		NO_TAG, {FREQUENCY_PLOT}},
{NO_TAG, 0, LANGUAGE_TEXT,				NO_TAG, {LANGUAGE}},
{NO_TAG, 0, LCD_CONTRAST_TEXT,			NO_TAG, {LCD_CONTRAST}},
{NO_TAG, 0, LCD_TIMEOUT_TEXT,			NO_TAG, {LCD_TIMEOUT}},
{NO_TAG, 0, MODEM_SETUP_TEXT,			NO_TAG, {MODEM_SETUP}},
{NO_TAG, 0, MONITOR_LOG_TEXT,			NO_TAG, {MONITOR_LOG}},
{NO_TAG, 0, PRINTER_TEXT,				NO_TAG, {PRINTER}},
{NO_TAG, 0, PRINT_MONITOR_LOG_TEXT,		NO_TAG, {PRINT_MONITOR_LOG}},
{NO_TAG, 0, REPORT_DISPLACEMENT_TEXT,	NO_TAG, {REPORT_DISPLACEMENT}},
{NO_TAG, 0, SENSOR_GAIN_TYPE_TEXT,		NO_TAG, {SENSOR_GAIN_TYPE}},
{NO_TAG, 0, SERIAL_NUMBER_TEXT,			NO_TAG, {SERIAL_NUMBER}},
{NO_TAG, 0, SUMMARIES_EVENTS_TEXT,		NO_TAG, {SUMMARIES_EVENTS}},
{NO_TAG, 0, TIMER_MODE_TEXT,			NO_TAG, {TIMER_MODE}},
{NO_TAG, 0, UNITS_OF_MEASURE_TEXT,		NO_TAG, {UNITS_OF_MEASURE}},
{NO_TAG, 0, VECTOR_SUM_TEXT,			NO_TAG, {VECTOR_SUM}},
{NO_TAG, 0, WAVEFORM_AUTO_CAL_TEXT,		NO_TAG, {WAVEFORM_AUTO_CAL}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&configMenuHandler}}
};

//--------------------
// Config Menu Handler
//--------------------
void configMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		switch (configMenu[newItemIndex].data)
		{
			case (ALARM_OUTPUT_MODE):
				ACTIVATE_USER_MENU_MSG(&alarmOutputMenu, (g_helpRecord.alarm_one_mode | g_helpRecord.alarm_two_mode));
			break;

			case (AUTO_CALIBRATION):
				ACTIVATE_USER_MENU_MSG(&autoCalMenu, g_helpRecord.auto_cal_mode);
			break;

			case (AUTO_DIAL_INFO):
				displayAutoDialInfo();
			break;

			case (AUTO_MONITOR):
				ACTIVATE_USER_MENU_MSG(&autoMonitorMenu, g_helpRecord.auto_monitor_mode);
			break;

			case (BATTERY):
				g_activeMenu = BATTERY_MENU; ACTIVATE_MENU_MSG();
			break;

			case (BAUD_RATE):
				ACTIVATE_USER_MENU_MSG(&baudRateMenu, g_helpRecord.baud_rate);
			break;

			case (CALIBRATION_DATE):
				displayCalDate();
			break;

			case (COPIES):
				if (SUPERGRAPH_UNIT)
				{
					ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&copiesMenu, &g_helpRecord.copies,
						COPIES_DEFAULT_VALUE, COPIES_MIN_VALUE, COPIES_MAX_VALUE);
				}
				else // MINIGRAPH_UNIT
				{
					messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (DATE_TIME):
				g_activeMenu = DATE_TIME_MENU; ACTIVATE_MENU_MSG();
			break;

			case (ERASE_FLASH):
				ACTIVATE_USER_MENU_MSG(&eraseEventsMenu, YES);
			break;

			case (FLASH_WRAPPING):
				ACTIVATE_USER_MENU_MSG(&flashWrappingMenu, g_helpRecord.flash_wrapping);
			break;

			case (FLASH_STATS):
				displayFlashUsageStats();
			break;

			case (FREQUENCY_PLOT):
				if (SUPERGRAPH_UNIT)
				{
					ACTIVATE_USER_MENU_MSG(&freqPlotMenu, g_helpRecord.freq_plot_mode);
				}
				else // MINIGRAPH_UNIT
				{
					messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (LANGUAGE):
				ACTIVATE_USER_MENU_MSG(&languageMenu, g_helpRecord.lang_mode);
			break;

			case (LCD_CONTRAST):
				g_activeMenu = LCD_CONTRAST_MENU; ACTIVATE_MENU_MSG();
			break;

			case (LCD_TIMEOUT):
				ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&lcdTimeoutMenu, &g_helpRecord.lcd_timeout,
					LCD_TIMEOUT_DEFAULT_VALUE, LCD_TIMEOUT_MIN_VALUE, LCD_TIMEOUT_MAX_VALUE);
			break;

			case (MODEM_SETUP):
				ACTIVATE_USER_MENU_MSG(&modemSetupMenu, g_modemSetupRecord.modemStatus);
			break;

			case (MONITOR_LOG):
				ACTIVATE_USER_MENU_MSG(&monitorLogMenu, DEFAULT_ITEM_1);
			break;

			case (PRINTER):
				if (SUPERGRAPH_UNIT)
				{
					ACTIVATE_USER_MENU_MSG(&printerEnableMenu, g_helpRecord.auto_print);
				}
				else // MINIGRAPH_UNIT
				{
					messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (PRINT_MONITOR_LOG):
				if (SUPERGRAPH_UNIT)
				{
					ACTIVATE_USER_MENU_MSG(&printMonitorLogMenu, g_helpRecord.print_monitor_log);
				}
				else
				{
					messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (REPORT_DISPLACEMENT):
				ACTIVATE_USER_MENU_MSG(&displacementMenu, g_helpRecord.report_displacement);
			break;

			case (SENSOR_GAIN_TYPE):
				displaySensorType();
			break;

			case (SERIAL_NUMBER):
				displaySerialNumber();
			break;

			case (SUMMARIES_EVENTS):
				g_activeMenu = SUMMARY_MENU; ACTIVATE_MENU_MSG(); mn_msg.data[0] = START_FROM_TOP;
			break;

			case (TIMER_MODE):
				if (g_helpRecord.timer_mode == ENABLED)
				{
					// Show current Timer Mode settings
					displayTimerModeSettings();

					// Check if the user wants to cancel the current timer mode
					if (messageBox(getLangText(STATUS_TEXT), getLangText(CANCEL_TIMER_MODE_Q_TEXT), MB_YESNO) == MB_FIRST_CHOICE)
					{
						g_helpRecord.timer_mode = DISABLED;

						// Enable the Power Off key
						powerControl(POWER_SHUTDOWN_ENABLE, ON);

						// Disable the Power Off timer if it's set
						clearSoftTimer(POWER_OFF_TIMER_NUM);

						saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

						ACTIVATE_USER_MENU_MSG(&timerModeMenu, g_helpRecord.timer_mode);
					}
					else // User did not cancel timer mode
					{
						ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
					}
				}
				else // Timer mode is disabled
				{
					ACTIVATE_USER_MENU_MSG(&timerModeMenu, g_helpRecord.timer_mode);
				}
			break;

			case (UNITS_OF_MEASURE):
				ACTIVATE_USER_MENU_MSG(&unitsMenu, g_helpRecord.units_of_measure);
			break;

			case (VECTOR_SUM):
				ACTIVATE_USER_MENU_MSG(&vectorSumMenu, g_helpRecord.vector_sum);
			break;

			case (WAVEFORM_AUTO_CAL):
				ACTIVATE_USER_MENU_MSG(&waveformAutoCalMenu, g_helpRecord.auto_cal_in_waveform);
			break;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Displacement Menu
//=============================================================================
//*****************************************************************************
#define DISPLACEMENT_MENU_ENTRIES 4
USER_MENU_STRUCT displacementMenu[DISPLACEMENT_MENU_ENTRIES] = {
{NO_TAG, 0, REPORT_DISPLACEMENT_TEXT, NO_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, DISPLACEMENT_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_2)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&displacementMenuHandler}}
};

//------------------------------
// Displacement Menu Handler
//------------------------------
void displacementMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.report_displacement = (uint8)displacementMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, REPORT_DISPLACEMENT);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Erase Events Menu
//=============================================================================
//*****************************************************************************
#define ERASE_EVENTS_MENU_ENTRIES 4
USER_MENU_STRUCT eraseEventsMenu[ERASE_EVENTS_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ERASE_EVENTS_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ERASE_EVENTS_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&eraseEventsMenuHandler}}
};

//--------------------------
// Erase Events Menu Handler
//--------------------------
void eraseEventsMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);
	char stringBuff[75];
	uint8 choice = MB_SECOND_CHOICE;

	if (keyPressed == ENTER_KEY)
	{
		if (eraseEventsMenu[newItemIndex].data == YES)
		{
			// Check if the user really wants to erase all the events
			choice = messageBox(getLangText(CONFIRM_TEXT), getLangText(REALLY_ERASE_ALL_EVENTS_Q_TEXT), MB_YESNO);

			// User selected Yes
			if (choice == MB_FIRST_CHOICE)
			{
				// Warn the user that turning off the power at this point is bad
				messageBox(getLangText(WARNING_TEXT), getLangText(DO_NOT_TURN_THE_UNIT_OFF_UNTIL_THE_OPERATION_IS_COMPLETE_TEXT), MB_OK);

				// Display a message alerting the user that the erase operation is in progress
				byteSet(&stringBuff[0], 0, sizeof(stringBuff));
				sprintf(stringBuff, "%s %s", getLangText(ERASE_OPERATION_IN_PROGRESS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT));
				overlayMessage(getLangText(STATUS_TEXT), stringBuff, 0);

#if 0 // ns7100
				// Erase all the data sectors (leave the boots sectors alone)
				sectorErase((uint16*)(FLASH_BASE_ADDR + FLASH_SECTOR_SIZE_x8),
							TOTAL_FLASH_DATA_SECTORS);
#else // ns8100
				deleteEventFileRecords();
#endif

				// Re-Init the ram summary table and the flash buffers
				initRamSummaryTbl();
				InitFlashBuffs();

				// Display a message that the operation is complete
				overlayMessage(getLangText(SUCCESS_TEXT), getLangText(ERASE_COMPLETE_TEXT), (2 * SOFT_SECS));

				// Call routine to reset the LCD display timers
				keypressEventMgr();

				ACTIVATE_USER_MENU_MSG(&zeroEventNumberMenu, YES);
			}
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&eraseSettingsMenu, YES);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, ERASE_FLASH);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Erase Settings Menu
//=============================================================================
//*****************************************************************************
#define ERASE_SETTINGS_MENU_ENTRIES 4
USER_MENU_STRUCT eraseSettingsMenu[ERASE_SETTINGS_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ERASE_SETTINGS_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ERASE_SETTINGS_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&eraseSettingsMenuHandler}}
};

//----------------------------
// Erase Settings Menu Handler
//----------------------------
void eraseSettingsMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (eraseEventsMenu[newItemIndex].data == YES)
		{
			messageBox(getLangText(WARNING_TEXT), getLangText(DO_NOT_TURN_THE_UNIT_OFF_UNTIL_THE_OPERATION_IS_COMPLETE_TEXT), MB_OK);

			overlayMessage(getLangText(STATUS_TEXT), getLangText(ERASE_OPERATION_IN_PROGRESS_TEXT), 0);

			//sectorErase((uint16*)FLASH_BASE_ADDR, 1);
			EraseParameterMemory(0, ((sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1)) +
									sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT)));

			// Reprogram Factroy Setup record
			saveRecData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);
			// Just in case, reinit Sensor Parameters
			initSensorParameters(g_factorySetupRecord.sensor_type, (uint8)g_triggerRecord.srec.sensitivity);

			// Load Defaults for Waveform
			loadTrigRecordDefaults((REC_EVENT_MN_STRUCT*)&g_triggerRecord, WAVEFORM_MODE);

			// Load Help rec defaults
			loadHelpRecordDefaults((REC_HELP_MN_STRUCT*)&g_helpRecord);
			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			// Clear out modem setup and save
			byteSet(&g_modemSetupRecord, 0, sizeof(MODEM_SETUP_STRUCT));
			g_modemSetupRecord.modemStatus = NO;
			saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);

			overlayMessage(getLangText(SUCCESS_TEXT), getLangText(ERASE_COMPLETE_TEXT), (2 * SOFT_SECS));
		}

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&eraseEventsMenu, YES);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Flash Wrapping Menu
//=============================================================================
//*****************************************************************************
#define FLASH_WRAPPING_MENU_ENTRIES 4
USER_MENU_STRUCT flashWrappingMenu[FLASH_WRAPPING_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, FLASH_WRAPPING_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, FLASH_WRAPPING_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&flashWrappingMenuHandler}}
};

//----------------------------
// Flash Wrapping Menu Handler
//----------------------------
void flashWrappingMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.flash_wrapping = (uint8)(flashWrappingMenu[newItemIndex].data);

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, FLASH_WRAPPING);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Freq Plot Menu
//=============================================================================
//*****************************************************************************
#define FREQ_PLOT_MENU_ENTRIES 4
USER_MENU_STRUCT freqPlotMenu[FREQ_PLOT_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, FREQUENCY_PLOT_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, FREQ_PLOT_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&freqPlotMenuHandler}}
};

//-----------------------
// Freq Plot Menu Handler
//-----------------------
void freqPlotMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.freq_plot_mode = (uint8)freqPlotMenu[newItemIndex].data;

		if (g_helpRecord.freq_plot_mode == YES)
		{
			ACTIVATE_USER_MENU_MSG(&freqPlotStandardMenu, g_helpRecord.freq_plot_type);
		}
		else
		{
			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, FREQUENCY_PLOT);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Freq Plot Standard Menu
//=============================================================================
//*****************************************************************************
#define FREQ_PLOT_STANDARD_MENU_ENTRIES 7
USER_MENU_STRUCT freqPlotStandardMenu[FREQ_PLOT_STANDARD_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, PLOT_STANDARD_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, FREQ_PLOT_STANDARD_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, US_BOM_TEXT,	NO_TAG, {FREQ_PLOT_US_BOM_STANDARD}},
{ITEM_2, 0, FRENCH_TEXT,	NO_TAG, {FREQ_PLOT_FRENCH_STANDARD}},
{ITEM_3, 0, DIN_4150_TEXT,	NO_TAG, {FREQ_PLOT_DIN_4150_STANDARD}},
{ITEM_4, 0, BRITISH_TEXT,	NO_TAG, {FREQ_PLOT_BRITISH_7385_STANDARD}},
{ITEM_5, 0, SPANISH_TEXT,	NO_TAG, {FREQ_PLOT_SPANISH_STANDARD}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&freqPlotStandardMenuHandler}}
};


//--------------------------------
// Freq Plot Standard Menu Handler
//--------------------------------
void freqPlotStandardMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.freq_plot_type = (uint8)freqPlotStandardMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&freqPlotMenu, g_helpRecord.freq_plot_mode);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Help Menu
//=============================================================================
//*****************************************************************************
#define HELP_MENU_ENTRIES 4
USER_MENU_STRUCT helpMenu[HELP_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, HELP_MENU_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, HELP_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, CONFIG_AND_OPTIONS_TEXT,	NO_TAG, {CONFIG}},
{ITEM_2, 0, HELP_INFORMATION_TEXT,		NO_TAG, {INFORMATION}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&helpMenuHandler}}
};

//------------------
// Help Menu Handler
//------------------
void helpMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (helpMenu[newItemIndex].data == CONFIG)
		{
			ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
		else
		{
			messageBox(getLangText(STATUS_TEXT), getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT), MB_OK);

			// Don't call menu for now, since it's empty
			//ACTIVATE_USER_MENU_MSG(&infoMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		g_activeMenu = MAIN_MENU;
		ACTIVATE_MENU_MSG();
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Info Menu
//=============================================================================
//*****************************************************************************
#define INFO_MENU_ENTRIES 4
USER_MENU_STRUCT infoMenu[INFO_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, HELP_INFO_MENU_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, INFO_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{NO_TAG, 0, NULL_TEXT,	NO_TAG, {DEFAULT_ITEM_1}},
{NO_TAG, 0, NOT_INCLUDED_TEXT,	NO_TAG, {DEFAULT_ITEM_2}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&infoMenuHandler}}
};

//------------------
// Info Menu Handler
//------------------
void infoMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	//uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&helpMenu, INFORMATION);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Language Menu
//=============================================================================
//*****************************************************************************
#define LANGUAGE_MENU_ENTRIES 7
USER_MENU_STRUCT languageMenu[LANGUAGE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, LANGUAGE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, LANGUAGE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, ENGLISH_TEXT,	NO_TAG, {ENGLISH_LANG}},
{ITEM_2, 0, FRENCH_TEXT,	NO_TAG, {FRENCH_LANG}},
{ITEM_3, 0, SPANISH_TEXT,	NO_TAG, {SPANISH_LANG}},
{ITEM_4, 0, ITALIAN_TEXT,	NO_TAG, {ITALIAN_LANG}},
{ITEM_5, 0, GERMAN_TEXT,	NO_TAG, {GERMAN_LANG}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&languageMenuHandler}}
};

//----------------------
// Language Menu Handler
//----------------------
void languageMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		switch (languageMenu[newItemIndex].data)
		{
			case ENGLISH_LANG:
			case FRENCH_LANG:
			case ITALIAN_LANG:
			case GERMAN_LANG:
					g_helpRecord.lang_mode = (uint8)languageMenu[newItemIndex].data;
					buildLanguageLinkTable(g_helpRecord.lang_mode);
					saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
					break;

			default:
					messageBox(getLangText(STATUS_TEXT), getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT), MB_OK);
					break;
		}

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, LANGUAGE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Millibar Menu
//=============================================================================
//*****************************************************************************
#define MILLIBAR_MENU_ENTRIES 4
USER_MENU_STRUCT millibarMenu[MILLIBAR_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, REPORT_MILLIBARS_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, MILLIBAR_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&millibarMenuHandler}}
};

//-----------------------
// Millibar Menu Handler
//-----------------------
void millibarMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_printMillibars = (uint8)millibarMenu[newItemIndex].data;

		g_activeMenu = CAL_SETUP_MENU;
		ACTIVATE_MENU_MSG();
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&airSetupMenu, g_factorySetupRecord.aweight_option);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Mode Menu
//=============================================================================
//*****************************************************************************
#define MODE_MENU_ENTRIES 4
USER_MENU_STRUCT modeMenu[MODE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, NULL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, MODE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, MONITOR_TEXT,	NO_TAG, {MONITOR}},
{ITEM_2, 0, EDIT_TEXT,		NO_TAG, {EDIT}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modeMenuHandler}}
};

//------------------
// Mode Menu Handler
//------------------
void modeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (modeMenu[newItemIndex].data == MONITOR)
		{
			// Call the Monitor menu and start monitoring
			g_activeMenu = MONITOR_MENU;
			ACTIVATE_MENU_WITH_DATA_MSG((uint32)g_triggerRecord.op_mode);
		}
		else // Mode is EDIT
		{
			if (g_triggerRecord.op_mode == BARGRAPH_MODE)
			{
				ACTIVATE_USER_MENU_MSG(&companyMenu, &g_triggerRecord.trec.client);
			}
			else // WAVEFORM_MODE or COMBO_MODE
			{
				ACTIVATE_USER_MENU_MSG(&sampleRateMenu, g_triggerRecord.trec.sample_rate);
			}
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		g_activeMenu = MAIN_MENU;
		ACTIVATE_MENU_MSG(); mn_msg.data[0] = ESC_KEY;
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Modem Setup Menu
//=============================================================================
//*****************************************************************************
#define MODEM_SETUP_MENU_ENTRIES 4
USER_MENU_STRUCT modemSetupMenu[MODEM_SETUP_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MODEM_SETUP_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, MODEM_SETUP_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&modemSetupMenuHandler}}
};

//-------------------------
// Modem Setup Menu Handler
//-------------------------
void modemSetupMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_modemSetupRecord.modemStatus = (uint16)(modemSetupMenu[newItemIndex].data);

		if (g_modemSetupRecord.modemStatus == YES)
		{
			ACTIVATE_USER_MENU_MSG(&modemInitMenu, &g_modemSetupRecord.init);

		}
		else // Modem Setup is NO
		{
			g_modemStatus.connectionState = NOP_CMD;
			g_modemStatus.modemAvailable = NO;

			g_modemStatus.craftPortRcvFlag = NO;		// Flag to indicate that incomming data has been received.
			g_modemStatus.xferState = NOP_CMD;		// Flag for xmitting data to the craft.
			g_modemStatus.xferMutex = NO;				// Flag to stop other message command from executing.
			g_modemStatus.systemIsLockedFlag = YES;

			g_modemStatus.ringIndicator = 0;

			saveRecData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
			ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, MODEM_SETUP);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Monitor Log Menu
//=============================================================================
//*****************************************************************************
#define MONITOR_LOG_MENU_ENTRIES 5
USER_MENU_STRUCT monitorLogMenu[MONITOR_LOG_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, MONITOR_LOG_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, MONITOR_LOG_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, VIEW_MONITOR_LOG_TEXT,	NO_TAG, {VIEW_LOG}},
{ITEM_2, 0, PRINT_MONITOR_LOG_TEXT,	NO_TAG, {PRINT_LOG}},
{ITEM_3, 0, LOG_RESULTS_TEXT,		NO_TAG, {LOG_RESULTS}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&monitorLogMenuHandler}}
};

//-------------------------
// Monitor Log Menu Handler
//-------------------------
void monitorLogMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if(keyPressed == ENTER_KEY)
	{
		if(newItemIndex == VIEW_LOG)
		{
			g_activeMenu = VIEW_MONITOR_LOG_MENU; ACTIVATE_MENU_MSG();
		}
		else if(newItemIndex == PRINT_LOG)
		{
			messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
		}
		else if(newItemIndex == LOG_RESULTS)
		{
			messageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
		}
	}
	else if(keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, MONITOR_LOG);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Printer Enable Menu
//=============================================================================
//*****************************************************************************
#define PRINTER_ENABLE_MENU_ENTRIES 4
USER_MENU_STRUCT printerEnableMenu[PRINTER_ENABLE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, PRINTER_ON_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, PRINTER_ENABLE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&printerEnableMenuHandler}}
};

//----------------------------
// Printer Enable Menu Handler
//----------------------------
void printerEnableMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.auto_print = (uint8)printerEnableMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, PRINTER);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Print Out Menu
//=============================================================================
//*****************************************************************************
#define PRINT_OUT_MENU_ENTRIES 4
USER_MENU_STRUCT printOutMenu[PRINT_OUT_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, PRINT_GRAPH_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, PRINT_OUT_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&printOutMenuHandler}}
};

//-----------------------
// Print Out Menu Handler
//-----------------------
void printOutMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);
	uint8 tempCopies = COPIES_DEFAULT_VALUE;

	if (keyPressed == ENTER_KEY)
	{
		if (printOutMenu[newItemIndex].data == YES)
		{
			// Using tempCopies just to seed with a value of 1 with the understanding that this value will
			//  be referenced before this routine exits (User Menu Handler) and not at all afterwards
			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&copiesMenu, &tempCopies,
				COPIES_DEFAULT_VALUE, COPIES_MIN_VALUE, COPIES_MAX_VALUE);
		}
		else
		{
			g_activeMenu = SUMMARY_MENU;
			ACTIVATE_MENU_MSG(); mn_msg.data[0] = ESC_KEY;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		g_activeMenu = SUMMARY_MENU;
		ACTIVATE_MENU_MSG(); mn_msg.data[0] = ESC_KEY;
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Print Monitor Log Menu
//=============================================================================
//*****************************************************************************
#define PRINT_MONITOR_LOG_RESULTS_MENU_ENTRIES 4
USER_MENU_STRUCT printMonitorLogMenu[PRINT_MONITOR_LOG_RESULTS_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, PRINT_MONITOR_LOG_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, PRINT_MONITOR_LOG_RESULTS_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&printMonitorLogMenuHandler}}
};

//---------------------------------------
// Print Monitor Log Menu Handler
//---------------------------------------
void printMonitorLogMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.print_monitor_log = (uint8)printMonitorLogMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, PRINT_MONITOR_LOG);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Sample Rate Menu
//=============================================================================
//*****************************************************************************
#define SAMPLE_RATE_MENU_ENTRIES 7
USER_MENU_STRUCT sampleRateMenu[SAMPLE_RATE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SAMPLE_RATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SAMPLE_RATE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, 512, NULL_TEXT, NO_TAG, {MIN_SAMPLE_RATE}}, // 512
{ITEM_2, 1024, NULL_TEXT, NO_TAG, {1024}},
{ITEM_3, 2048, NULL_TEXT, NO_TAG, {2048}},
{ITEM_4, 4096, NULL_TEXT, NO_TAG, {4096}},
{ITEM_5, 8192, NULL_TEXT, NO_TAG, {MAX_SAMPLE_RATE}}, // 8192
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&sampleRateMenuHandler}}
};

//-------------------------
// Sample Rate Menu Handler
//-------------------------
void sampleRateMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);
	char message[70];

	if (keyPressed == ENTER_KEY)
	{
		if ((sampleRateMenu[newItemIndex].data < sampleRateMenu[ITEM_1].data) || 
			(sampleRateMenu[newItemIndex].data > sampleRateMenu[ITEM_5].data))
		{
			byteSet(&message[0], 0, sizeof(message));
			sprintf((char*)message, "%lu %s", sampleRateMenu[newItemIndex].data,
					getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT));
			messageBox(getLangText(STATUS_TEXT), (char*)message, MB_OK);

			debug("Sample Rate: %d is not supported for this mode.\n", g_triggerRecord.trec.record_time_max);

			ACTIVATE_USER_MENU_MSG(&sampleRateMenu, sampleRateMenu[DEFAULT_ITEM_2].data);
		}
		else
		{
			g_triggerRecord.trec.sample_rate = sampleRateMenu[newItemIndex].data;

			// fix_ns8100 - Add in Combo mode calc that accounts for Bargraph buffers
			g_triggerRecord.trec.record_time_max = (uint16)(((uint32)((EVENT_BUFF_SIZE_IN_WORDS -
				((g_triggerRecord.trec.sample_rate / 4) * g_sensorInfoPtr->numOfChannels) -
					((g_triggerRecord.trec.sample_rate / MIN_SAMPLE_RATE) * MAX_CAL_SAMPLES * g_sensorInfoPtr->numOfChannels)) /
						(g_triggerRecord.trec.sample_rate * g_sensorInfoPtr->numOfChannels))));

			debug("New Max Record Time: %d\n", g_triggerRecord.trec.record_time_max);

			ACTIVATE_USER_MENU_MSG(&companyMenu, &g_triggerRecord.trec.client);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		updateModeMenuTitle(g_triggerRecord.op_mode);
		ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Save Setup Menu
//=============================================================================
//*****************************************************************************
#define SAVE_SETUP_MENU_ENTRIES 4
USER_MENU_STRUCT saveSetupMenu[SAVE_SETUP_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SAVE_SETUP_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SAVE_SETUP_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&saveSetupMenuHandler}}
};

//------------------------
// Save Setup Menu Handler
//------------------------
void saveSetupMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (saveSetupMenu[newItemIndex].data == YES)
		{
			ACTIVATE_USER_MENU_MSG(&saveRecordMenu, (uint8)0);
		}
		else // User selected NO
		{
			// Save the current trig record into the default location
			saveRecData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

			updateModeMenuTitle(g_triggerRecord.op_mode);
			ACTIVATE_USER_MENU_MSG(&modeMenu, MONITOR);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		switch (g_triggerRecord.op_mode)
		{
			case WAVEFORM_MODE:
			case COMBO_MODE:
				if ((g_helpRecord.alarm_one_mode == ALARM_MODE_OFF) &&
					(g_helpRecord.alarm_two_mode == ALARM_MODE_OFF))
				{
					if ((!g_factorySetupRecord.invalid) && (g_factorySetupRecord.aweight_option == ENABLED))
					{
						ACTIVATE_USER_MENU_MSG(&airScaleMenu, 0);
					}
					else
					{
						ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
							RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
					}
				}
				else
				{
					if (g_helpRecord.alarm_two_mode == ALARM_MODE_OFF)
					{
						ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
					}
					else
					{
						ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &g_helpRecord.alarm_two_time,
							ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT,
							ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
					}
				}
			break;

			case BARGRAPH_MODE:
				if ((g_helpRecord.alarm_one_mode == ALARM_MODE_OFF) &&
					(g_helpRecord.alarm_two_mode == ALARM_MODE_OFF))
				{
					ACTIVATE_USER_MENU_MSG(&barResultMenu, g_helpRecord.vector_sum);
				}
				else
				{
					if (g_helpRecord.alarm_two_mode == ALARM_MODE_OFF)
					{
						ACTIVATE_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarm_two_mode);
					}
					else
					{
						ACTIVATE_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &g_helpRecord.alarm_two_time,
							ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT,
							ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
					}
				}
			break;
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}
//*****************************************************************************
//=============================================================================
// Sensitivity Menu
//=============================================================================
//*****************************************************************************
#define SENSITIVITY_MENU_ENTRIES 4
USER_MENU_STRUCT sensitivityMenu[SENSITIVITY_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SENSITIVITY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SENSITIVITY_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, LOW_TEXT,	LOW_SENSITIVITY_MAX_TAG, {LOW}},
{ITEM_2, 0, HIGH_TEXT,	HIGH_SENSITIVITY_MAX_TAG, {HIGH}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&sensitivityMenuHandler}}
};

//-------------------------
// Sensitivity Menu Handler
//-------------------------
void sensitivityMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.srec.sensitivity = (uint8)sensitivityMenu[newItemIndex].data;

		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
			{
				USER_MENU_DEFAULT_TYPE(seismicTriggerMenu) = MG_TYPE;
				USER_MENU_ALT_TYPE(seismicTriggerMenu) = MG_TYPE;
			}
			else
			{
				USER_MENU_DEFAULT_TYPE(seismicTriggerMenu) = IN_TYPE;
				USER_MENU_ALT_TYPE(seismicTriggerMenu) = MM_TYPE;
			}

			ACTIVATE_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &g_triggerRecord.trec.seismicTriggerLevel,
				SEISMIC_TRIGGER_DEFAULT_VALUE, SEISMIC_TRIGGER_MIN_VALUE, SEISMIC_TRIGGER_MAX_VALUE);
		}
		else if ((g_triggerRecord.op_mode == BARGRAPH_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			ACTIVATE_USER_MENU_MSG(&barChannelMenu, g_triggerRecord.berec.barChannel);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&operatorMenu, &g_triggerRecord.trec.oper);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Sensor Type Menu
//=============================================================================
//*****************************************************************************
#define SENSOR_TYPE_MENU_ENTRIES 7
USER_MENU_STRUCT sensorTypeMenu[SENSOR_TYPE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SENSOR_GAIN_TYPE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SENSOR_TYPE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, 0, X1_20_IPS_TEXT,		NO_TAG, {SENSOR_20_IN}},
{ITEM_2, 0, X2_10_IPS_TEXT,		NO_TAG, {SENSOR_10_IN}},
{ITEM_3, 0, X4_5_IPS_TEXT,		NO_TAG, {SENSOR_5_IN}},
{ITEM_4, 0, X8_2_5_IPS_TEXT,	NO_TAG, {SENSOR_2_5_IN}},
{ITEM_5, 0, ACC_793L_TEXT,		NO_TAG, {SENSOR_ACC}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&sensorTypeMenuHandler}}
};

//-------------------------
// Sensor Type Menu Handler
//-------------------------
void sensorTypeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_factorySetupRecord.sensor_type = (uint16)sensorTypeMenu[newItemIndex].data;

		if (g_factorySetupSequence == PROCESS_FACTORY_SETUP)
		{
			ACTIVATE_USER_MENU_MSG(&airSetupMenu, g_factorySetupRecord.aweight_option);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_factorySetupSequence == PROCESS_FACTORY_SETUP)
		{
			ACTIVATE_USER_MENU_MSG(&serialNumberMenu, &g_factorySetupRecord.serial_num);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
		}
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Timer Mode Menu
//=============================================================================
//*****************************************************************************
#define TIMER_MODE_MENU_ENTRIES 4
USER_MENU_STRUCT timerModeMenu[TIMER_MODE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, TIMER_MODE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, TIMER_MODE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, DISABLED_TEXT,	NO_TAG, {DISABLED}},
{ITEM_2, 0, ENABLED_TEXT,	NO_TAG, {ENABLED}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&timerModeMenuHandler}}
};

//------------------------
// Timer Mode Menu Handler
//------------------------
void timerModeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.timer_mode = (uint8)timerModeMenu[newItemIndex].data;

		if (g_helpRecord.timer_mode == ENABLED)
		{
			ACTIVATE_USER_MENU_MSG(&timerModeFreqMenu, g_helpRecord.timer_mode_freq);
		}
		else
		{
			ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_helpRecord.timer_mode == ENABLED)
		{
			g_helpRecord.timer_mode = DISABLED;

			saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			// Disable the Power Off timer if it's set
			clearSoftTimer(POWER_OFF_TIMER_NUM);

			messageBox(getLangText(STATUS_TEXT), getLangText(TIMER_MODE_SETUP_NOT_COMPLETED_TEXT), MB_OK);
			messageBox(getLangText(STATUS_TEXT), getLangText(DISABLING_TIMER_MODE_TEXT), MB_OK);
		}

		ACTIVATE_USER_MENU_MSG(&configMenu, TIMER_MODE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Timer Mode Freq Menu
//=============================================================================
//*****************************************************************************
#define TIMER_MODE_FREQ_MENU_ENTRIES 7
USER_MENU_STRUCT timerModeFreqMenu[TIMER_MODE_FREQ_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, TIMER_FREQUENCY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, TIMER_MODE_FREQ_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, ONE_TIME_TEXT,			NO_TAG, {TIMER_MODE_ONE_TIME}},
{ITEM_2, 0, DAILY_EVERY_DAY_TEXT,	NO_TAG, {TIMER_MODE_DAILY}},
{ITEM_3, 0, DAILY_WEEKDAYS_TEXT,	NO_TAG, {TIMER_MODE_WEEKDAYS}},
{ITEM_4, 0, WEEKLY_TEXT,			NO_TAG, {TIMER_MODE_WEEKLY}},
{ITEM_5, 0, MONTHLY_TEXT,			NO_TAG, {TIMER_MODE_MONTHLY}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&timerModeFreqMenuHandler}}
};

//-----------------------------
// Timer Mode Freq Menu Handler
//-----------------------------
void timerModeFreqMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.timer_mode_freq = (uint8)timerModeFreqMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		g_activeMenu = TIMER_MODE_TIME_MENU;
		ACTIVATE_MENU_MSG();
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&timerModeMenu, g_helpRecord.timer_mode);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Units Menu
//=============================================================================
//*****************************************************************************
#define UNITS_MENU_ENTRIES 4
USER_MENU_STRUCT unitsMenu[UNITS_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, UNITS_OF_MEASURE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, UNITS_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, IMPERIAL_TEXT,	NO_TAG, {IMPERIAL_TYPE}},
{ITEM_2, 0, METRIC_TEXT,	NO_TAG, {METRIC_TYPE}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&unitsMenuHandler}}
};

//-------------------
// Units Menu Handler
//-------------------
void unitsMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.units_of_measure = (uint8)unitsMenu[newItemIndex].data;

		if (g_helpRecord.units_of_measure == IMPERIAL_TYPE)
		{
			g_sensorInfoPtr->unitsFlag = IMPERIAL_TYPE;
			g_sensorInfoPtr->measurementRatio = (float)IMPERIAL;
		}
		else // Metric
		{
			g_sensorInfoPtr->unitsFlag = METRIC_TYPE;
			g_sensorInfoPtr->measurementRatio = (float)METRIC;
		}

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, UNITS_OF_MEASURE);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Vector Sum Menu
//=============================================================================
//*****************************************************************************
#define VECTOR_SUM_MENU_ENTRIES 4
USER_MENU_STRUCT vectorSumMenu[VECTOR_SUM_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, VECTOR_SUM_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, VECTOR_SUM_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, OFF_TEXT,	NO_TAG, {DISABLED}},
{ITEM_2, 0, ON_TEXT,	NO_TAG, {ENABLED}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&vectorSumMenuHandler}}
};

//------------------------
// Vector Sum Menu Handler
//------------------------
void vectorSumMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.vector_sum = (uint8)vectorSumMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, VECTOR_SUM);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Waveform Auto Cal Menu
//=============================================================================
//*****************************************************************************
#define WAVEFORM_AUTO_CAL_MENU_ENTRIES 4
USER_MENU_STRUCT waveformAutoCalMenu[WAVEFORM_AUTO_CAL_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, WAVEFORM_AUTO_CAL_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, WAVEFORM_AUTO_CAL_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, DISABLED_TEXT,				NO_TAG, {DISABLED}},
{ITEM_2, 0, START_OF_WAVEFORM_TEXT,	NO_TAG, {ENABLED}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&waveformAutoCalMenuHandler}}
};

//-------------------------------
// Waveform Auto Cal Menu Handler
//-------------------------------
void waveformAutoCalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if(keyPressed == ENTER_KEY)
	{
		g_helpRecord.auto_cal_in_waveform = (uint8)waveformAutoCalMenu[newItemIndex].data;

		saveRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		ACTIVATE_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if(keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&configMenu, WAVEFORM_AUTO_CAL);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//*****************************************************************************
//=============================================================================
// Zero Event Number Menu
//=============================================================================
//*****************************************************************************
#define ZERO_EVENT_NUMBER_MENU_ENTRIES 4
USER_MENU_STRUCT zeroEventNumberMenu[ZERO_EVENT_NUMBER_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ZERO_EVENT_NUMBER_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ZERO_EVENT_NUMBER_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&zeroEventNumberMenuHandler}}
};

//-------------------------------
// Zero Event Number Menu Handler
//-------------------------------
void zeroEventNumberMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg;
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (zeroEventNumberMenu[newItemIndex].data == YES)
		{
			//sectorErase((uint16*)(FLASH_BASE_ADDR + FLASH_BOOT_SECTOR_SIZE_x8), 1);
			EraseParameterMemory(FLASH_BOOT_SECTOR_SIZE_x8, 4);
			initCurrentEventNumber();

			__autoDialoutTbl.lastDownloadedEvent = 0;

			overlayMessage(getLangText(SUCCESS_TEXT), getLangText(EVENT_NUMBER_ZEROING_COMPLETE_TEXT), (2 * SOFT_SECS));
		}

		ACTIVATE_USER_MENU_MSG(&eraseSettingsMenu, YES);
	}
	else if (keyPressed == ESC_KEY)
	{
		ACTIVATE_USER_MENU_MSG(&eraseEventsMenu, YES);
	}

	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}
