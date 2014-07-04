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
extern USER_MENU_STRUCT bitAccuracyMenu[];
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
extern USER_MENU_STRUCT modemSetupMenu[];
extern USER_MENU_STRUCT modemInitMenu[];
extern USER_MENU_STRUCT monitorLogMenu[];
extern USER_MENU_STRUCT operatorMenu[];
#if 1 // Updated (Port lost change)
extern USER_MENU_STRUCT peakAccMenu[];
#endif
extern USER_MENU_STRUCT pretriggerSizeMenu[];
extern USER_MENU_STRUCT printerEnableMenu[];
extern USER_MENU_STRUCT printMonitorLogMenu[];
extern USER_MENU_STRUCT powerSavingsMenu[];
extern USER_MENU_STRUCT recalibrateMenu[];
extern USER_MENU_STRUCT recordTimeMenu[];
extern USER_MENU_STRUCT saveRecordMenu[];
extern USER_MENU_STRUCT saveSetupMenu[];
extern USER_MENU_STRUCT sampleRateMenu[];
extern USER_MENU_STRUCT sampleRateBargraphMenu[];
extern USER_MENU_STRUCT sampleRateComboMenu[];
extern USER_MENU_STRUCT seismicTriggerMenu[];
extern USER_MENU_STRUCT sensitivityMenu[];
extern USER_MENU_STRUCT sensorTypeMenu[];
extern USER_MENU_STRUCT serialNumberMenu[];
extern USER_MENU_STRUCT summaryIntervalMenu[];
extern USER_MENU_STRUCT timerModeFreqMenu[];
extern USER_MENU_STRUCT timerModeMenu[];
extern USER_MENU_STRUCT unitsOfMeasureMenu[];
extern USER_MENU_STRUCT unitsOfAirMenu[];
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AirScaleMenuHandler}}
};

//-----------------------
// Air Scale Menu Handler
//-----------------------
void AirScaleMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	//uint16 newItemIndex = *((uint16*)data);
#if 1 // fix_ns8100
	UNUSED(data);
#endif

	// Save Air Scale somewhere?
	
	if (keyPressed == ENTER_KEY)
	{
		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			// If alarm mode is off, the proceed to save setup
			if ((g_helpRecord.alarmOneMode == ALARM_MODE_OFF) && (g_helpRecord.alarmTwoMode == ALARM_MODE_OFF))
			{
				SETUP_USER_MENU_MSG(&saveSetupMenu, YES);
			}
			else // Goto Alarm setup menus
			{
				SETUP_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarmOneMode);
			}
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			SETUP_USER_MENU_MSG(&barIntervalMenu, g_triggerRecord.bgrec.barInterval);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_triggerRecord.op_mode == WAVEFORM_MODE)
		{
			SETUP_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
				RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			SETUP_USER_MENU_MSG(&barChannelMenu, 0);
		}
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AirSetupMenuHandler}}
};

//-----------------------
// Air Setup Menu Handler
//-----------------------
void AirSetupMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_factorySetupRecord.aweight_option = (uint8)airSetupMenu[newItemIndex].data;

#if 0 // ns7100
		SETUP_USER_MENU_MSG(&unitsOfAirMenu, g_helpRecord.unitsOfAir);
#else // ns8100
		SETUP_MENU_MSG(CAL_SETUP_MENU);
#endif
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&sensorTypeMenu, g_factorySetupRecord.sensor_type);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AlarmOneMenuHandler}}
};

//-----------------------
// Alarm One Menu Handler
//-----------------------
void AlarmOneMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.alarmOneMode = (uint8)alarmOneMenu[newItemIndex].data;

		switch (g_helpRecord.alarmOneMode)
		{
			case (ALARM_MODE_OFF):
				g_helpRecord.alarmOneSeismicLevel = NO_TRIGGER_CHAR;
				g_helpRecord.alarmOneAirLevel = NO_TRIGGER_CHAR;

				SETUP_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarmTwoMode);
			break;

			case (ALARM_MODE_SEISMIC):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR))
				{
					MessageBox(getLangText(WARNING_TEXT), "SEISMIC TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					SETUP_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarmOneMode);
				}
				else
				{
					g_helpRecord.alarmOneAirLevel = NO_TRIGGER_CHAR;

					// Setup Alarm One Seismic Level
					if (g_helpRecord.alarmOneSeismicLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmOneSeismicLevel = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmOneSeismicMinLevel = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarmOneSeismicLevel < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarmOneSeismicLevel = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarmOneSeismicMinLevel = ALARM_SEIS_MIN_VALUE;
					}

					// Call Alarm One Seismic Level
#if 1 // ns8100 - Down convert to current bit accuracy setting
					if (g_helpRecord.alarmOneSeismicLevel != NO_TRIGGER_CHAR)
					{
						g_tempTriggerLevelForMenuAdjsutment = g_helpRecord.alarmOneSeismicLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
					}		
#endif
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &g_tempTriggerLevelForMenuAdjsutment,
						(g_helpRecord.alarmOneSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						(g_helpRecord.alarmOneSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						g_bitAccuracyMidpoint);
				}
			break;

			case (ALARM_MODE_AIR):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR))
				{
					MessageBox(getLangText(WARNING_TEXT), "AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					SETUP_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarmOneMode);
				}
				else
				{
					g_helpRecord.alarmOneSeismicLevel = NO_TRIGGER_CHAR;

					// Setup Alarm One Air Level
					if (g_helpRecord.alarmOneAirLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmOneAirLevel = ALARM_ONE_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmOneAirMinLevel = (uint32)g_triggerRecord.trec.airTriggerLevel;

						if (g_helpRecord.alarmOneAirLevel < (uint32)g_triggerRecord.trec.airTriggerLevel)
						{
							g_helpRecord.alarmOneAirLevel = (uint32)g_triggerRecord.trec.airTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
#if 0 // Port lost change
						g_helpRecord.alarmOneAirMinLevel = ALARM_AIR_MIN_VALUE;
#else // Updated
						if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
						{
							g_helpRecord.alarmOneAirMinLevel = ALARM_AIR_MIN_VALUE;
						}
						else
						{
							g_helpRecord.alarmOneAirMinLevel = ALARM_AIR_MB_MIN_VALUE;
						}
#endif
					}

#if 0 // Port lost change
					// Call Alarm One Air Level
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &g_helpRecord.alarmOneAirLevel,
						g_helpRecord.alarmOneAirMinLevel, g_helpRecord.alarmOneAirMinLevel, ALARM_AIR_MAX_VALUE);
#else // Updated
					g_tempTriggerLevelForMenuAdjsutment = AirTriggerConvertToUnits(g_helpRecord.alarmOneAirLevel);

					if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
					{
						// Call Alarm One Air Level
						SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &g_tempTriggerLevelForMenuAdjsutment, g_helpRecord.alarmOneAirMinLevel,
														g_helpRecord.alarmOneAirMinLevel, ALARM_AIR_MAX_VALUE);
					}
					else // (g_helpRecord.unitsOfAir == MILLIBAR_TYPE)
					{
						// Call Alarm One Air Level
						SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmOneAirLevelMenu, &g_tempTriggerLevelForMenuAdjsutment, g_helpRecord.alarmOneAirMinLevel,
														g_helpRecord.alarmOneAirMinLevel, ALARM_AIR_MB_MAX_VALUE);
					}
#endif
				}
			break;

			case (ALARM_MODE_BOTH):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) ||
					(g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR)))
				{
					MessageBox(getLangText(WARNING_TEXT), "SEISMIC OR AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					SETUP_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarmOneMode);
				}
				else
				{
					// Setup Alarm One Seismic Level
					if (g_helpRecord.alarmOneSeismicLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmOneSeismicLevel = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmOneSeismicMinLevel = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarmOneSeismicLevel < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarmOneSeismicLevel = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarmOneSeismicMinLevel = ALARM_SEIS_MIN_VALUE;
					}

					// Setup Alarm One Air Level
					if (g_helpRecord.alarmOneAirLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmOneAirLevel = ALARM_ONE_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmOneAirMinLevel = (uint16)g_triggerRecord.trec.airTriggerLevel;

						if (g_helpRecord.alarmOneAirLevel < (uint32)g_triggerRecord.trec.airTriggerLevel)
						{
							g_helpRecord.alarmOneAirLevel = (uint32)g_triggerRecord.trec.airTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
#if 0 // Port lost change
						g_helpRecord.alarmOneAirMinLevel = ALARM_AIR_MIN_VALUE;
#else // Updated
						if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
						{
							g_helpRecord.alarmOneAirMinLevel = ALARM_AIR_MIN_VALUE;
						}
						else
						{
							g_helpRecord.alarmOneAirMinLevel = ALARM_AIR_MB_MIN_VALUE;
						}
#endif
					}

					// Call Alarm One Seismic Level
#if 1 // ns8100 - Down convert to current bit accuracy setting
					if (g_helpRecord.alarmOneSeismicLevel != NO_TRIGGER_CHAR)
					{
						g_tempTriggerLevelForMenuAdjsutment = g_helpRecord.alarmOneSeismicLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
					}		
#endif
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmOneSeismicLevelMenu, &g_tempTriggerLevelForMenuAdjsutment,
						(g_helpRecord.alarmOneSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						(g_helpRecord.alarmOneSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						g_bitAccuracyMidpoint);
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
				SETUP_USER_MENU_MSG(&airScaleMenu, 0);
			}
			else
			{
				SETUP_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
					RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
			}
		}
		else if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
		     SETUP_USER_MENU_MSG(&barResultMenu, g_helpRecord.vectorSum);
		}
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AlarmTwoMenuHandler}}
};

//-----------------------
// Alarm Two Menu Handler
//-----------------------
void AlarmTwoMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.alarmTwoMode = (uint8)alarmTwoMenu[newItemIndex].data;

		switch (g_helpRecord.alarmTwoMode)
		{
			case (ALARM_MODE_OFF):
				g_helpRecord.alarmTwoSeismicLevel = NO_TRIGGER_CHAR;
				g_helpRecord.alarmTwoAirLevel = NO_TRIGGER_CHAR;

				SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

				SETUP_USER_MENU_MSG(&saveSetupMenu, YES);
			break;

			case (ALARM_MODE_SEISMIC):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR))
				{
					MessageBox(getLangText(WARNING_TEXT), "SEISMIC TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					SETUP_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarmTwoMode);
				}
				else
				{
					g_helpRecord.alarmTwoAirLevel = NO_TRIGGER_CHAR;

					// Setup Alarm Two Seismic Level
					if (g_helpRecord.alarmTwoSeismicLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmTwoSeismicLevel = ALARM_ONE_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmTwoSeismicMinLevel = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarmTwoSeismicLevel < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarmTwoSeismicLevel = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarmTwoSeismicMinLevel = ALARM_SEIS_MIN_VALUE;
					}

					// Call Alarm Two Seismic Level
#if 1 // ns8100 - Down convert to current bit accuracy setting
					if (g_helpRecord.alarmTwoSeismicLevel != NO_TRIGGER_CHAR)
					{
						g_tempTriggerLevelForMenuAdjsutment = g_helpRecord.alarmTwoSeismicLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
					}		
#endif
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &g_tempTriggerLevelForMenuAdjsutment,
						(g_helpRecord.alarmTwoSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						(g_helpRecord.alarmTwoSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						g_bitAccuracyMidpoint);
				}
			break;

			case (ALARM_MODE_AIR):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && (g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR))
				{
					MessageBox(getLangText(WARNING_TEXT), "AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					SETUP_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarmTwoMode);
				}
				else
				{
					g_helpRecord.alarmTwoSeismicLevel = NO_TRIGGER_CHAR;

					// Setup Alarm Two Air Level
					if (g_helpRecord.alarmTwoAirLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmTwoAirLevel = ALARM_TWO_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmTwoAirMinLevel = (uint16)g_triggerRecord.trec.airTriggerLevel;

						if (g_helpRecord.alarmTwoAirLevel < (uint32)g_triggerRecord.trec.airTriggerLevel)
						{
							g_helpRecord.alarmTwoAirLevel = (uint32)g_triggerRecord.trec.airTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
#if 0 // Port lost change
						g_helpRecord.alarmTwoAirMinLevel = ALARM_AIR_MIN_VALUE;
#else // Updated
						if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
						{
							g_helpRecord.alarmTwoAirMinLevel = ALARM_AIR_MIN_VALUE;
						}
						else
						{
							g_helpRecord.alarmTwoAirMinLevel = ALARM_AIR_MB_MIN_VALUE;
						}
#endif
					}

#if 0 // Port lost change
					// Call Alarm One Air Level
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &g_helpRecord.alarmTwoAirLevel,
						g_helpRecord.alarmTwoAirMinLevel, g_helpRecord.alarmTwoAirMinLevel, ALARM_AIR_MAX_VALUE);
#else // Updated
					g_tempTriggerLevelForMenuAdjsutment = AirTriggerConvertToUnits(g_helpRecord.alarmTwoAirLevel);

					if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
					{
						// Call Alarm One Air Level
						SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &g_tempTriggerLevelForMenuAdjsutment, g_helpRecord.alarmTwoAirMinLevel,
														g_helpRecord.alarmTwoAirMinLevel, ALARM_AIR_MAX_VALUE);
					}
					else // (g_helpRecord.unitsOfAir == MILLIBAR_TYPE)
					{
						// Call Alarm One Air Level
						SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoAirLevelMenu, &g_tempTriggerLevelForMenuAdjsutment, g_helpRecord.alarmTwoAirMinLevel,
														g_helpRecord.alarmTwoAirMinLevel, ALARM_AIR_MB_MAX_VALUE);
					}
#endif
				}
			break;

			case (ALARM_MODE_BOTH):
				if ((g_triggerRecord.op_mode == WAVEFORM_MODE) && ((g_triggerRecord.trec.seismicTriggerLevel == NO_TRIGGER_CHAR) ||
					(g_triggerRecord.trec.airTriggerLevel == NO_TRIGGER_CHAR)))
				{
					MessageBox(getLangText(WARNING_TEXT), "SEISMIC OR AIR TRIGGER SET TO NO TRIGGER. PLEASE CHANGE", MB_OK);

					SETUP_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarmTwoMode);
				}
				else
				{
					// Setup Alarm Two Seismic Level
					if (g_helpRecord.alarmTwoSeismicLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmTwoSeismicLevel = ALARM_TWO_SEIS_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmTwoSeismicMinLevel = g_triggerRecord.trec.seismicTriggerLevel;

						if (g_helpRecord.alarmTwoSeismicLevel < g_triggerRecord.trec.seismicTriggerLevel)
						{
							g_helpRecord.alarmTwoSeismicLevel = g_triggerRecord.trec.seismicTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
						g_helpRecord.alarmTwoSeismicMinLevel = ALARM_SEIS_MIN_VALUE;
					}

					// Setup Alarm Two Air Level
					if (g_helpRecord.alarmTwoAirLevel == NO_TRIGGER_CHAR)
					{
						g_helpRecord.alarmTwoAirLevel = ALARM_TWO_AIR_DEFAULT_TRIG_LVL;
					}

					if (g_triggerRecord.op_mode == WAVEFORM_MODE)
					{
						g_helpRecord.alarmTwoAirMinLevel = (uint16)g_triggerRecord.trec.airTriggerLevel;

						if (g_helpRecord.alarmTwoAirLevel < (uint32)g_triggerRecord.trec.airTriggerLevel)
						{
							g_helpRecord.alarmTwoAirLevel = (uint32)g_triggerRecord.trec.airTriggerLevel;
						}
					}
					else // g_triggerRecord.op_mode == BARGRAPH_MODE
					{
#if 0 // Port lost change
						g_helpRecord.alarmTwoAirMinLevel = ALARM_AIR_MIN_VALUE;
#else // Updated
						if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
						{
							g_helpRecord.alarmTwoAirMinLevel = ALARM_AIR_MIN_VALUE;
						}
						else
						{
							g_helpRecord.alarmTwoAirMinLevel = ALARM_AIR_MB_MIN_VALUE;
						}
#endif
					}

					// Call Alarm Two Seismic Level
#if 1 // ns8100 - Down convert to current bit accuracy setting
					if (g_helpRecord.alarmTwoSeismicLevel != NO_TRIGGER_CHAR)
					{
						g_tempTriggerLevelForMenuAdjsutment = g_helpRecord.alarmTwoSeismicLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint);
					}		
#endif
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&alarmTwoSeismicLevelMenu, &g_tempTriggerLevelForMenuAdjsutment,
						(g_helpRecord.alarmTwoSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						(g_helpRecord.alarmTwoSeismicMinLevel / (ALARM_SEIS_MAX_VALUE / g_bitAccuracyMidpoint)),
						g_bitAccuracyMidpoint);
				}
			break;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_helpRecord.alarmOneMode == ALARM_MODE_OFF)
		{
			SETUP_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarmOneMode);
		}
		else
		{
			SETUP_USER_MENU_FOR_FLOATS_MSG(&alarmOneTimeMenu, &g_helpRecord.alarmOneTime,
				ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT,
				ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
		}
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AlarmOutputMenuHandler}}
};

//--------------------------
// Alarm Output Menu Handler
//--------------------------
void AlarmOutputMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.alarmOneMode = (uint8)alarmOutputMenu[newItemIndex].data;
		g_helpRecord.alarmTwoMode = (uint8)alarmOutputMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, ALARM_OUTPUT_MODE);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AutoCalMenuHandler}}
};

//----------------------
// Auto Cal Menu Handler
//----------------------
void AutoCalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.autoCalMode = (uint8)autoCalMenu[newItemIndex].data;

		if (g_helpRecord.autoCalMode == AUTO_NO_CAL_TIMEOUT)
		{
			g_autoCalDaysToWait = 0;
		}
		else // Auto Cal enabled, set to process the first midnight
		{
			g_autoCalDaysToWait = 1;
		}

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, AUTO_CALIBRATION);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&AutoMonitorMenuHandler}}
};

//--------------------------
// Auto Monitor Menu Handler
//--------------------------
void AutoMonitorMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.autoMonitorMode = (uint8)autoMonitorMenu[newItemIndex].data;

		AssignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_helpRecord.autoMonitorMode * TICKS_PER_MIN), AutoMonitorTimerCallBack);

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, AUTO_MONITOR);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&BarChannelMenuHandler}}
};

//-------------------------
// Bar Channel Menu Handler
//-------------------------
void BarChannelMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
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
		else if (g_helpRecord.unitsOfMeasure == IMPERIAL)
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
		else // g_helpRecord.unitsOfMeasure == METRIC
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

		SETUP_USER_MENU_MSG(&barScaleMenu, g_triggerRecord.berec.barScale);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&sensitivityMenu, g_triggerRecord.srec.sensitivity);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&BarIntervalMenuHandler}}
};

//--------------------------
// Bar Interval Menu Handler
//--------------------------
void BarIntervalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.bgrec.barInterval = barIntervalMenu[newItemIndex].data;

		SETUP_USER_MENU_MSG(&summaryIntervalMenu, g_triggerRecord.bgrec.summaryInterval);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&barScaleMenu, g_triggerRecord.berec.barScale);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&BarScaleMenuHandler}}
};

//--------------------------
// Bar Scale Menu Handler
//--------------------------
void BarScaleMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.berec.barScale = (uint8)(barScaleMenu[newItemIndex].data);

		SETUP_USER_MENU_MSG(&barIntervalMenu, g_triggerRecord.bgrec.barInterval);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&barChannelMenu, g_triggerRecord.berec.barChannel);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SummaryIntervalMenuHandler}}
};

//------------------------------
// Summary Interval Menu Handler
//------------------------------
void SummaryIntervalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.bgrec.summaryInterval = summaryIntervalMenu[newItemIndex].data;

		SETUP_USER_MENU_FOR_INTEGERS_MSG(&lcdImpulseTimeMenu, &g_triggerRecord.berec.impulseMenuUpdateSecs,
			LCD_IMPULSE_TIME_DEFAULT_VALUE, LCD_IMPULSE_TIME_MIN_VALUE, LCD_IMPULSE_TIME_MAX_VALUE);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&barIntervalMenu, g_triggerRecord.bgrec.barInterval);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&BarResultMenuHandler}}
};

//------------------------
// Bar Result Menu Handler
//------------------------
void BarResultMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.vectorSum = (uint8)barResultMenu[newItemIndex].data;
		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

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

#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
			{
				g_tempTriggerLevelForMenuAdjsutment = g_triggerRecord.trec.seismicTriggerLevel / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			SETUP_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &g_tempTriggerLevelForMenuAdjsutment,
				(SEISMIC_TRIGGER_DEFAULT_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
				(SEISMIC_TRIGGER_MIN_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else if ((g_helpRecord.alarmOneMode == ALARM_MODE_OFF) && (g_helpRecord.alarmTwoMode == ALARM_MODE_OFF))
		{
			SETUP_USER_MENU_MSG(&saveSetupMenu, YES);
		}
		else
		{
			SETUP_USER_MENU_MSG(&alarmOneMenu, g_helpRecord.alarmOneMode);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_FOR_INTEGERS_MSG(&lcdImpulseTimeMenu, &g_triggerRecord.berec.impulseMenuUpdateSecs,
			LCD_IMPULSE_TIME_DEFAULT_VALUE, LCD_IMPULSE_TIME_MIN_VALUE, LCD_IMPULSE_TIME_MAX_VALUE);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Baud Rate Menu
//=============================================================================
//*****************************************************************************
#define BAUD_RATE_MENU_ENTRIES 7
USER_MENU_STRUCT baudRateMenu[BAUD_RATE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, BAUD_RATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BAUD_RATE_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, BAUD_RATE_115200_TEXT,	NO_TAG, {BAUD_RATE_115200}},
{ITEM_2, 57600, BAUD_RATE_TEXT,	NO_TAG, {BAUD_RATE_57600}},
{ITEM_3, 38400, BAUD_RATE_TEXT,	NO_TAG, {BAUD_RATE_38400}},
{ITEM_4, 19200, BAUD_RATE_TEXT,	NO_TAG, {BAUD_RATE_19200}},
{ITEM_5, 9600, BAUD_RATE_TEXT,	NO_TAG, {BAUD_RATE_9600}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&BaudRateMenuHandler}}
};

//-----------------------
// Baud Rate Menu Handler
//-----------------------
#include "usart.h"
void BaudRateMenuHandler(uint8 keyPressed, void* data)
{
	uint32 usartRetries = USART_DEFAULT_TIMEOUT;
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);
	usart_options_t usart_1_rs232_options =
	{
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};

	if (keyPressed == ENTER_KEY)
	{
		if (g_helpRecord.baudRate != baudRateMenu[newItemIndex].data)
		{
			g_helpRecord.baudRate = (uint8)baudRateMenu[newItemIndex].data;

			switch (baudRateMenu[newItemIndex].data)
			{
				case BAUD_RATE_115200: usart_1_rs232_options.baudrate = 115200; break;
				case BAUD_RATE_57600: usart_1_rs232_options.baudrate = 57600; break;
				case BAUD_RATE_38400: usart_1_rs232_options.baudrate = 38400; break;
				case BAUD_RATE_19200: usart_1_rs232_options.baudrate = 19200; break;
				case BAUD_RATE_9600: usart_1_rs232_options.baudrate = 9600; break;
			}

			// Check that receive is ready/idle
			while (((AVR32_USART1.csr & AVR32_USART_CSR_RXRDY_MASK) == 0) && usartRetries)
			{
				usartRetries--;
			}

			// Check that transmit is ready/idle
			usartRetries = USART_DEFAULT_TIMEOUT;
			while (((AVR32_USART1.csr & AVR32_USART_CSR_TXRDY_MASK) == 0) && usartRetries)
			{
				usartRetries--;
			}

#if 1 // ns8100 (Added to help Dave's Supergraphics handle Baud change)
			//-------------------------------------------------------------------------
			// Signal remote end that RS232 Comm is unavailable
			//-------------------------------------------------------------------------
			CLEAR_RTS; CLEAR_DTR;
#endif

			// Reset the RS232
			usart_reset(&AVR32_USART1);

			// Re-Initialize the RS232 with the new baud rate
			usart_init_rs232(&AVR32_USART1, &usart_1_rs232_options, FOSC0);

#if 1 // ns8100 (Added to help Dave's Supergraphics handle Baud change)
			//-------------------------------------------------------------------------
			// Signal remote end that RS232 Comm is available
			//-------------------------------------------------------------------------
			SET_RTS; SET_DTR;
#endif

			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
		}

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, BAUD_RATE);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Bit Accuracy Menu
//=============================================================================
//*****************************************************************************
#define BIT_ACCURACY_MENU_ENTRIES 6
USER_MENU_STRUCT bitAccuracyMenu[BIT_ACCURACY_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, BIT_ACCURACY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, BIT_ACCURACY_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, ACCURACY_16_BIT, BIT_TEXT, NO_TAG, {ACCURACY_16_BIT}},
{ITEM_2, ACCURACY_14_BIT, BIT_TEXT, NO_TAG, {ACCURACY_14_BIT}},
{ITEM_3, ACCURACY_12_BIT, BIT_TEXT, NO_TAG, {ACCURACY_12_BIT}},
{ITEM_4, ACCURACY_10_BIT, BIT_TEXT, NO_TAG, {ACCURACY_10_BIT}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&BitAccuracyMenuHandler}}
};

//-------------------------
// Bit Accuracy Menu Handler
//-------------------------
void BitAccuracyMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.trec.bitAccuracy = bitAccuracyMenu[newItemIndex].data;

		switch (g_triggerRecord.trec.bitAccuracy)
		{
			case ACCURACY_10_BIT: { g_bitAccuracyMidpoint = ACCURACY_10_BIT_MIDPOINT; } break;
			case ACCURACY_12_BIT: { g_bitAccuracyMidpoint = ACCURACY_12_BIT_MIDPOINT; } break;
			case ACCURACY_14_BIT: { g_bitAccuracyMidpoint = ACCURACY_14_BIT_MIDPOINT; } break;
			default: { g_bitAccuracyMidpoint = ACCURACY_16_BIT_MIDPOINT; } break;
		}

		// Check if sample rate is 16K which can not process temp readings
		if (g_triggerRecord.trec.sample_rate == SAMPLE_RATE_16K)
		{
			SETUP_USER_MENU_MSG(&companyMenu, &g_triggerRecord.trec.client);
		}
		else // All other sample rates
		{
			SETUP_USER_MENU_MSG(&recalibrateMenu, g_triggerRecord.trec.adjustForTempDrift);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_triggerRecord.op_mode == BARGRAPH_MODE)
		{
			SETUP_USER_MENU_MSG(&sampleRateBargraphMenu, g_triggerRecord.trec.sample_rate);
		}		
		else if (g_triggerRecord.op_mode == COMBO_MODE)
		{
			SETUP_USER_MENU_MSG(&sampleRateComboMenu, g_triggerRecord.trec.sample_rate);
		}			
		else
		{
			SETUP_USER_MENU_MSG(&sampleRateMenu, g_triggerRecord.trec.sample_rate);
		}			
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Config Menu
//=============================================================================
//*****************************************************************************
#if 0 // ns7100
#define CONFIG_MENU_ENTRIES 31
#else // ns8100
#if 0 // Port lost change
#define CONFIG_MENU_ENTRIES 27
#else // Updated
#define CONFIG_MENU_ENTRIES 30
#endif
#endif
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
#if 0 // ns7100
{NO_TAG, 0, COPIES_TEXT,				NO_TAG, {COPIES}},
#endif
{NO_TAG, 0, DATE_TIME_TEXT,				NO_TAG, {DATE_TIME}},
{NO_TAG, 0, ERASE_MEMORY_TEXT,			NO_TAG, {ERASE_FLASH}},
{NO_TAG, 0, FLASH_WRAPPING_TEXT,		NO_TAG, {FLASH_WRAPPING}},
{NO_TAG, 0, FLASH_STATS_TEXT,			NO_TAG, {FLASH_STATS}},
#if 0 // ns7100
{NO_TAG, 0, FREQUENCY_PLOT_TEXT,		NO_TAG, {FREQUENCY_PLOT}},
#endif
{NO_TAG, 0, LANGUAGE_TEXT,				NO_TAG, {LANGUAGE}},
{NO_TAG, 0, LCD_CONTRAST_TEXT,			NO_TAG, {LCD_CONTRAST}},
{NO_TAG, 0, LCD_TIMEOUT_TEXT,			NO_TAG, {LCD_TIMEOUT}},
{NO_TAG, 0, MODEM_SETUP_TEXT,			NO_TAG, {MODEM_SETUP}},
{NO_TAG, 0, MONITOR_LOG_TEXT,			NO_TAG, {MONITOR_LOG}},
{NO_TAG, 0, PRETRIGGER_SIZE_TEXT,		NO_TAG, {PRETRIGGER_SIZE}},
{NO_TAG, 0, POWER_SAVINGS_TEXT,			NO_TAG, {POWER_SAVINGS}},
#if 0 // ns7100
{NO_TAG, 0, PRINTER_TEXT,				NO_TAG, {PRINTER}},
{NO_TAG, 0, PRINT_MONITOR_LOG_TEXT,		NO_TAG, {PRINT_MONITOR_LOG}},
#endif
{NO_TAG, 0, REPORT_DISPLACEMENT_TEXT,	NO_TAG, {REPORT_DISPLACEMENT}},
#if 1 // Updated (Port lost change)
{NO_TAG, 0, REPORT_PEAK_ACC_TEXT,		NO_TAG, {REPORT_PEAK_ACC}},
#endif
{NO_TAG, 0, SENSOR_GAIN_TYPE_TEXT,		NO_TAG, {SENSOR_GAIN_TYPE}},
{NO_TAG, 0, SERIAL_NUMBER_TEXT,			NO_TAG, {SERIAL_NUMBER}},
{NO_TAG, 0, SUMMARIES_EVENTS_TEXT,		NO_TAG, {SUMMARIES_EVENTS}},
{NO_TAG, 0, TIMER_MODE_TEXT,			NO_TAG, {TIMER_MODE}},
{NO_TAG, 0, UNITS_OF_MEASURE_TEXT,		NO_TAG, {UNITS_OF_MEASURE}},
{NO_TAG, 0, UNITS_OF_AIR_TEXT,			NO_TAG, {UNITS_OF_AIR}},
{NO_TAG, 0, VECTOR_SUM_TEXT,			NO_TAG, {VECTOR_SUM}},
{NO_TAG, 0, WAVEFORM_AUTO_CAL_TEXT,		NO_TAG, {WAVEFORM_AUTO_CAL}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&ConfigMenuHandler}}
};

//--------------------
// Config Menu Handler
//--------------------
void ConfigMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		switch (configMenu[newItemIndex].data)
		{
			case (ALARM_OUTPUT_MODE):
				SETUP_USER_MENU_MSG(&alarmOutputMenu, (g_helpRecord.alarmOneMode | g_helpRecord.alarmTwoMode));
			break;

			case (AUTO_CALIBRATION):
				SETUP_USER_MENU_MSG(&autoCalMenu, g_helpRecord.autoCalMode);
			break;

			case (AUTO_DIAL_INFO):
				DisplayAutoDialInfo();
			break;

			case (AUTO_MONITOR):
				SETUP_USER_MENU_MSG(&autoMonitorMenu, g_helpRecord.autoMonitorMode);
			break;

			case (BATTERY):
				SETUP_MENU_MSG(BATTERY_MENU);
			break;

			case (BAUD_RATE):
				SETUP_USER_MENU_MSG(&baudRateMenu, g_helpRecord.baudRate);
			break;

			case (CALIBRATION_DATE):
				DisplayCalDate();
			break;

			case (COPIES):
				if (SUPERGRAPH_UNIT)
				{
					SETUP_USER_MENU_FOR_INTEGERS_MSG(&copiesMenu, &g_helpRecord.copies,
						COPIES_DEFAULT_VALUE, COPIES_MIN_VALUE, COPIES_MAX_VALUE);
				}
				else // MINIGRAPH_UNIT
				{
					MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (DATE_TIME):
				SETUP_MENU_MSG(DATE_TIME_MENU);
			break;

			case (ERASE_FLASH):
				SETUP_USER_MENU_MSG(&eraseEventsMenu, YES);
			break;

			case (FLASH_WRAPPING):
				SETUP_USER_MENU_MSG(&flashWrappingMenu, g_helpRecord.flashWrapping);
			break;

			case (FLASH_STATS):
				DisplayFlashUsageStats();
			break;

			case (FREQUENCY_PLOT):
				if (SUPERGRAPH_UNIT)
				{
					SETUP_USER_MENU_MSG(&freqPlotMenu, g_helpRecord.freqPlotMode);
				}
				else // MINIGRAPH_UNIT
				{
					MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (LANGUAGE):
				SETUP_USER_MENU_MSG(&languageMenu, g_helpRecord.languageMode);
			break;

			case (LCD_CONTRAST):
				SETUP_MENU_MSG(LCD_CONTRAST_MENU);
			break;

			case (LCD_TIMEOUT):
				SETUP_USER_MENU_FOR_INTEGERS_MSG(&lcdTimeoutMenu, &g_helpRecord.lcdTimeout,
					LCD_TIMEOUT_DEFAULT_VALUE, LCD_TIMEOUT_MIN_VALUE, LCD_TIMEOUT_MAX_VALUE);
			break;

			case (MODEM_SETUP):
				SETUP_USER_MENU_MSG(&modemSetupMenu, g_modemSetupRecord.modemStatus);
			break;

			case (MONITOR_LOG):
				SETUP_USER_MENU_MSG(&monitorLogMenu, DEFAULT_ITEM_1);
			break;

			case (PRETRIGGER_SIZE):
				SETUP_USER_MENU_MSG(&pretriggerSizeMenu, g_helpRecord.pretrigBufferDivider);
			break;

			case (PRINTER):
				if (SUPERGRAPH_UNIT)
				{
					SETUP_USER_MENU_MSG(&printerEnableMenu, g_helpRecord.autoPrint);
				}
				else // MINIGRAPH_UNIT
				{
					MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (PRINT_MONITOR_LOG):
				if (SUPERGRAPH_UNIT)
				{
					SETUP_USER_MENU_MSG(&printMonitorLogMenu, g_helpRecord.printMonitorLog);
				}
				else
				{
					MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
				}
			break;

			case (POWER_SAVINGS):
				SETUP_USER_MENU_MSG(&powerSavingsMenu, g_helpRecord.powerSavingsLevel);
			break;

			case (REPORT_DISPLACEMENT):
				SETUP_USER_MENU_MSG(&displacementMenu, g_helpRecord.reportDisplacement);
			break;

#if 1 // Updated (Port lost change)
			case (REPORT_PEAK_ACC):
				SETUP_USER_MENU_MSG(&peakAccMenu, g_helpRecord.reportPeakAcceleration);
			break;
#endif

			case (SENSOR_GAIN_TYPE):
				DisplaySensorType();
			break;

			case (SERIAL_NUMBER):
				DisplaySerialNumber();
			break;

			case (SUMMARIES_EVENTS):
				SETUP_MENU_MSG(SUMMARY_MENU); mn_msg.data[0] = START_FROM_TOP;
			break;

			case (TIMER_MODE):
				if (g_helpRecord.timerMode == ENABLED)
				{
					// Show current Timer Mode settings
					DisplayTimerModeSettings();

					// Check if the user wants to cancel the current timer mode
					if (MessageBox(getLangText(STATUS_TEXT), getLangText(CANCEL_TIMER_MODE_Q_TEXT), MB_YESNO) == MB_FIRST_CHOICE)
					{
						g_helpRecord.timerMode = DISABLED;

						// Disable power off protection
						PowerControl(POWER_OFF_PROTECTION_ENABLE, OFF);

						// Disable the Power Off timer if it's set
						ClearSoftTimer(POWER_OFF_TIMER_NUM);

						SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

						SETUP_USER_MENU_MSG(&timerModeMenu, g_helpRecord.timerMode);
					}
					else // User did not cancel timer mode
					{
						SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
					}
				}
				else // Timer mode is disabled
				{
					SETUP_USER_MENU_MSG(&timerModeMenu, g_helpRecord.timerMode);
				}
			break;

			case (UNITS_OF_MEASURE):
				SETUP_USER_MENU_MSG(&unitsOfMeasureMenu, g_helpRecord.unitsOfMeasure);
			break;

			case (UNITS_OF_AIR):
				SETUP_USER_MENU_MSG(&unitsOfAirMenu, g_helpRecord.unitsOfAir);
			break;

			case (VECTOR_SUM):
				SETUP_USER_MENU_MSG(&vectorSumMenu, g_helpRecord.vectorSum);
			break;

			case (WAVEFORM_AUTO_CAL):
				SETUP_USER_MENU_MSG(&waveformAutoCalMenu, g_helpRecord.autoCalForWaveform);
			break;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&helpMenu, CONFIG);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&DisplacementMenuHandler}}
};

//------------------------------
// Displacement Menu Handler
//------------------------------
void DisplacementMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.reportDisplacement = (uint8)displacementMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, REPORT_DISPLACEMENT);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&EraseEventsMenuHandlerz}}
};

//--------------------------
// Erase Events Menu Handler
//--------------------------
void EraseEventsMenuHandlerz(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);
	char stringBuff[75];
	uint8 choice = MB_SECOND_CHOICE;

	if (keyPressed == ENTER_KEY)
	{
		if (eraseEventsMenu[newItemIndex].data == YES)
		{
			// Check if the user really wants to erase all the events
			choice = MessageBox(getLangText(CONFIRM_TEXT), getLangText(REALLY_ERASE_ALL_EVENTS_Q_TEXT), MB_YESNO);

			// User selected Yes
			if (choice == MB_FIRST_CHOICE)
			{
				// Warn the user that turning off the power at this point is bad
				MessageBox(getLangText(WARNING_TEXT), getLangText(DO_NOT_TURN_THE_UNIT_OFF_UNTIL_THE_OPERATION_IS_COMPLETE_TEXT), MB_OK);

				// Display a message alerting the user that the erase operation is in progress
				ByteSet(&stringBuff[0], 0, sizeof(stringBuff));
				sprintf(stringBuff, "%s %s", getLangText(ERASE_OPERATION_IN_PROGRESS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT));
				OverlayMessage(getLangText(STATUS_TEXT), stringBuff, 0);

#if 0 // ns7100
				// Erase all the data sectors (leave the boots sectors alone)
				SectorErase((uint16*)(FLASH_BASE_ADDR + FLASH_SECTOR_SIZE_x8),
							TOTAL_FLASH_DATA_SECTORS);
#else // ns8100
				DeleteEventFileRecords();
#endif

				// Re-Init the ram summary table and the flash buffers
				InitRamSummaryTbl();
				InitFlashBuffs();

				// Display a message that the operation is complete
				OverlayMessage(getLangText(SUCCESS_TEXT), getLangText(ERASE_COMPLETE_TEXT), (2 * SOFT_SECS));

				// Call routine to reset the LCD display timers
				KeypressEventMgr();

				SETUP_USER_MENU_MSG(&zeroEventNumberMenu, YES);
			}
		}
		else
		{
#if 0 // Port lost change
			SETUP_USER_MENU_MSG(&eraseSettingsMenu, YES);
#else // Updated
			SETUP_USER_MENU_MSG(&eraseSettingsMenu, NO);
#endif
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, ERASE_FLASH);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Erase Settings Menu
//=============================================================================
//*****************************************************************************
#define ERASE_SETTINGS_MENU_ENTRIES 4
USER_MENU_STRUCT eraseSettingsMenu[ERASE_SETTINGS_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, ERASE_SETTINGS_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, ERASE_SETTINGS_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, 0, YES_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,	NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&EraseSettingsMenuHandler}}
};

//----------------------------
// Erase Settings Menu Handler
//----------------------------
void EraseSettingsMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (eraseEventsMenu[newItemIndex].data == YES)
		{
			MessageBox(getLangText(WARNING_TEXT), getLangText(DO_NOT_TURN_THE_UNIT_OFF_UNTIL_THE_OPERATION_IS_COMPLETE_TEXT), MB_OK);

			OverlayMessage(getLangText(STATUS_TEXT), getLangText(ERASE_OPERATION_IN_PROGRESS_TEXT), 0);

			//SectorErase((uint16*)FLASH_BASE_ADDR, 1);
			EraseParameterMemory(0, ((sizeof(REC_EVENT_MN_STRUCT) * (MAX_NUM_OF_SAVED_SETUPS + 1)) +
									sizeof(REC_HELP_MN_STRUCT) + sizeof(MODEM_SETUP_STRUCT)));

			// Reprogram Factroy Setup record
			SaveRecordData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);
			// Just in case, reinit Sensor Parameters
			InitSensorParameters(g_factorySetupRecord.sensor_type, (uint8)g_triggerRecord.srec.sensitivity);

			// Load Defaults for Waveform
			LoadTrigRecordDefaults((REC_EVENT_MN_STRUCT*)&g_triggerRecord, WAVEFORM_MODE);

			// Load Help rec defaults
			LoadHelpRecordDefaults((REC_HELP_MN_STRUCT*)&g_helpRecord);
			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			// Clear out modem setup and save
			ByteSet(&g_modemSetupRecord, 0, sizeof(MODEM_SETUP_STRUCT));
			g_modemSetupRecord.modemStatus = NO;
			SaveRecordData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);

			OverlayMessage(getLangText(SUCCESS_TEXT), getLangText(ERASE_COMPLETE_TEXT), (2 * SOFT_SECS));
		}

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&eraseEventsMenu, YES);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&FlashWrappingMenuHandler}}
};

//----------------------------
// Flash Wrapping Menu Handler
//----------------------------
void FlashWrappingMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.flashWrapping = (uint8)(flashWrappingMenu[newItemIndex].data);

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, FLASH_WRAPPING);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&FreqPlotMenuHandler}}
};

//-----------------------
// Freq Plot Menu Handler
//-----------------------
void FreqPlotMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.freqPlotMode = (uint8)freqPlotMenu[newItemIndex].data;

		if (g_helpRecord.freqPlotMode == YES)
		{
			SETUP_USER_MENU_MSG(&freqPlotStandardMenu, g_helpRecord.freqPlotType);
		}
		else
		{
			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, FREQUENCY_PLOT);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&FreqPlotStandardMenuHandler}}
};


//--------------------------------
// Freq Plot Standard Menu Handler
//--------------------------------
void FreqPlotStandardMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.freqPlotType = (uint8)freqPlotStandardMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&freqPlotMenu, g_helpRecord.freqPlotMode);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Help Menu
//=============================================================================
//*****************************************************************************
#if 1 // Normal
#define HELP_MENU_ENTRIES 4
USER_MENU_STRUCT helpMenu[HELP_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, HELP_MENU_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, HELP_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, CONFIG_AND_OPTIONS_TEXT,	NO_TAG, {CONFIG}},
{ITEM_2, 0, HELP_INFORMATION_TEXT,		NO_TAG, {INFORMATION}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&HelpMenuHandler}}
};
#else // Test
#define HELP_MENU_ENTRIES 5
USER_MENU_STRUCT helpMenu[HELP_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, HELP_MENU_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, HELP_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, CONFIG_AND_OPTIONS_TEXT,	NO_TAG, {CONFIG}},
{ITEM_2, 0, HELP_INFORMATION_TEXT,		NO_TAG, {INFORMATION}},
{ITEM_3, 0, TESTING_TEXT,				NO_TAG, {TEST_OPTION}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&HelpMenuHandler}}
};
#endif

//------------------
// Help Menu Handler
//------------------
void HelpMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);
	char buildString[50];

	if (keyPressed == ENTER_KEY)
	{
		if (helpMenu[newItemIndex].data == CONFIG)
		{
			SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
#if 0 // Test
		else if (helpMenu[newItemIndex].data == TEST_OPTION)
		{
extern void PowerDownAndHalt(void);
			PowerDownAndHalt();
		}
#endif
		else
		{
#if 0 // ns7100
			MessageBox(getLangText(STATUS_TEXT), getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT), MB_OK);
#else
			sprintf(buildString, "%s %s %s", getLangText(SOFTWARE_VER_TEXT), (char*)g_buildVersion, (char*)g_buildDate);
			MessageBox(getLangText(STATUS_TEXT), buildString, MB_OK);
#endif

			// Don't call menu for now, since it's empty
			//SETUP_USER_MENU_MSG(&infoMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_MENU_MSG(MAIN_MENU);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&InfoMenuHandler}}
};

//------------------
// Info Menu Handler
//------------------
void InfoMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	//uint16 newItemIndex = *((uint16*)data);
	UNUSED(data);	

	if (keyPressed == ENTER_KEY)
	{
		SETUP_USER_MENU_MSG(&helpMenu, CONFIG);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&helpMenu, INFORMATION);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&LanguageMenuHandler}}
};

//----------------------
// Language Menu Handler
//----------------------
void LanguageMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		switch (languageMenu[newItemIndex].data)
		{
			case ENGLISH_LANG:
			case FRENCH_LANG:
			case ITALIAN_LANG:
			case GERMAN_LANG:
			case SPANISH_LANG:
					g_helpRecord.languageMode = (uint8)languageMenu[newItemIndex].data;
					BuildLanguageLinkTable(g_helpRecord.languageMode);
					SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
					break;

			default:
					MessageBox(getLangText(STATUS_TEXT), getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT), MB_OK);
					break;
		}

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, LANGUAGE);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&ModeMenuHandler}}
};

//------------------
// Mode Menu Handler
//------------------
void ModeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (modeMenu[newItemIndex].data == MONITOR)
		{
			// Call the Monitor menu and start monitoring
			SETUP_MENU_WITH_DATA_MSG(MONITOR_MENU, (uint32)g_triggerRecord.op_mode);
		}
		else // Mode is EDIT
		{
			if (g_triggerRecord.op_mode == BARGRAPH_MODE)
			{
				SETUP_USER_MENU_MSG(&sampleRateBargraphMenu, g_triggerRecord.trec.sample_rate);
			}		
			else if (g_triggerRecord.op_mode == COMBO_MODE)
			{
				SETUP_USER_MENU_MSG(&sampleRateComboMenu, g_triggerRecord.trec.sample_rate);
			}			
			else
			{
				SETUP_USER_MENU_MSG(&sampleRateMenu, g_triggerRecord.trec.sample_rate);
			}			
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_MENU_MSG(MAIN_MENU); mn_msg.data[0] = ESC_KEY;
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&ModemSetupMenuHandler}}
};

//-------------------------
// Modem Setup Menu Handler
//-------------------------
void ModemSetupMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_modemSetupRecord.modemStatus = (uint16)(modemSetupMenu[newItemIndex].data);

		if (g_modemSetupRecord.modemStatus == YES)
		{
			SETUP_USER_MENU_MSG(&modemInitMenu, &g_modemSetupRecord.init);

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

			SaveRecordData(&g_modemSetupRecord, DEFAULT_RECORD, REC_MODEM_SETUP_TYPE);
			SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, MODEM_SETUP);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&MonitorLogMenuHandler}}
};

//-------------------------
// Monitor Log Menu Handler
//-------------------------
void MonitorLogMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if(keyPressed == ENTER_KEY)
	{
		if(newItemIndex == VIEW_LOG)
		{
			SETUP_MENU_MSG(VIEW_MONITOR_LOG_MENU);
		}
		else if(newItemIndex == PRINT_LOG)
		{
			MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
		}
		else if(newItemIndex == LOG_RESULTS)
		{
			MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
		}
	}
	else if(keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, MONITOR_LOG);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Peak Acceleration Menu
//=============================================================================
//*****************************************************************************
#define PEAK_ACC_MENU_ENTRIES 4
USER_MENU_STRUCT peakAccMenu[PEAK_ACC_MENU_ENTRIES] = {
{NO_TAG, 0, REPORT_PEAK_ACC_TEXT, NO_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, PEAK_ACC_MENU_ENTRIES, TITLE_LEFT_JUSTIFIED, DEFAULT_ITEM_2)}},
{ITEM_1, 0, YES_TEXT,   NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT, NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&PeakAccMenuHandler}}
};

//-------------------------------
// Peak Acceleration Menu Handler
//-------------------------------
void PeakAccMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if(keyPressed == ENTER_KEY)
	{
		g_helpRecord.reportPeakAcceleration = (uint8)peakAccMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if(keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, REPORT_PEAK_ACC);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Pretrigger Size Menu
//=============================================================================
//*****************************************************************************
#define PRETRIGGER_SIZE_MENU_ENTRIES 5
USER_MENU_STRUCT pretriggerSizeMenu[PRETRIGGER_SIZE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, PRETRIGGER_SIZE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, PRETRIGGER_SIZE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, QUARTER_SECOND_TEXT,	NO_TAG, {4}},
{ITEM_2, 0, HALF_SECOND_TEXT,	NO_TAG, {2}},
{ITEM_3, 0, FULL_SECOND_TEXT,	NO_TAG, {1}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&PretriggerSizeMenuHandler}}
};

//-----------------------------
// Pretrigger Size Menu Handler
//-----------------------------
void PretriggerSizeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.pretrigBufferDivider = (uint8)pretriggerSizeMenu[newItemIndex].data;

		debug("New Pretrigger divider: %d\n", g_helpRecord.pretrigBufferDivider);

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, PRETRIGGER_SIZE);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&PrinterEnableMenuHandler}}
};

//----------------------------
// Printer Enable Menu Handler
//----------------------------
void PrinterEnableMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.autoPrint = (uint8)printerEnableMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, PRINTER);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&PrintOutMenuHandler}}
};

//-----------------------
// Print Out Menu Handler
//-----------------------
void PrintOutMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);
	uint8 tempCopies = COPIES_DEFAULT_VALUE;

	if (keyPressed == ENTER_KEY)
	{
		if (printOutMenu[newItemIndex].data == YES)
		{
			// Using tempCopies just to seed with a value of 1 with the understanding that this value will
			//  be referenced before this routine exits (User Menu Handler) and not at all afterwards
			SETUP_USER_MENU_FOR_INTEGERS_MSG(&copiesMenu, &tempCopies,
				COPIES_DEFAULT_VALUE, COPIES_MIN_VALUE, COPIES_MAX_VALUE);
		}
		else
		{
			SETUP_MENU_MSG(SUMMARY_MENU); mn_msg.data[0] = ESC_KEY;
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_MENU_MSG(SUMMARY_MENU); mn_msg.data[0] = ESC_KEY;
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&PrintMonitorLogMenuHandler}}
};

//---------------------------------------
// Print Monitor Log Menu Handler
//---------------------------------------
void PrintMonitorLogMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.printMonitorLog = (uint8)printMonitorLogMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, PRINT_MONITOR_LOG);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Power Savings Menu
//=============================================================================
//*****************************************************************************
#define POWER_SAVINGS_MENU_ENTRIES 6
USER_MENU_STRUCT powerSavingsMenu[POWER_SAVINGS_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, POWER_SAVINGS_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, POWER_SAVINGS_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, NONE_TEXT,	NO_TAG, {POWER_SAVINGS_NONE}},
{ITEM_2, 0, MINIMUM_TEXT,	NO_TAG, {POWER_SAVINGS_MINIMUM}},
{ITEM_3, 0, MOST_TEXT,	NO_TAG, {POWER_SAVINGS_MOST}},
{ITEM_4, 0, MAX_TEXT,	NO_TAG, {POWER_SAVINGS_MAX}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&PowerSavingsMenuHandler}}
};

//---------------------------------------
// Power Savings Menu Handler
//---------------------------------------
void PowerSavingsMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.powerSavingsLevel = (uint8)powerSavingsMenu[newItemIndex].data;

		AdjustPowerSavings();

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, POWER_SAVINGS);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Recalibrate Menu
//=============================================================================
//*****************************************************************************
#define RECALIBRATE_MENU_ENTRIES 4
USER_MENU_STRUCT recalibrateMenu[RECALIBRATE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, RECALIBRATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, RECALIBRATE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, ON_TEMP_CHANGE_TEXT,	NO_TAG, {YES}},
{ITEM_2, 0, NO_TEXT,					NO_TAG, {NO}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&RecalibrateMenuHandler}}
};

//--------------------------------
// Recalibrate Menu Handler
//--------------------------------
void RecalibrateMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_triggerRecord.trec.adjustForTempDrift = (uint8)recalibrateMenu[newItemIndex].data;

		// Save the current trig record into the default location
		SaveRecordData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&companyMenu, &g_triggerRecord.trec.client);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&bitAccuracyMenu, g_triggerRecord.trec.bitAccuracy);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Sample Rate Menu
//=============================================================================
//*****************************************************************************
#define SAMPLE_RATE_MENU_ENTRIES 8
USER_MENU_STRUCT sampleRateMenu[SAMPLE_RATE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SAMPLE_RATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SAMPLE_RATE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, SAMPLE_RATE_512, NULL_TEXT, NO_TAG, {MIN_SAMPLE_RATE}}, // 512
{ITEM_2, SAMPLE_RATE_1K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_1K}},
{ITEM_3, SAMPLE_RATE_2K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_2K}},
{ITEM_4, SAMPLE_RATE_4K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_4K}},
{ITEM_5, SAMPLE_RATE_8K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_8K}},
{ITEM_6, SAMPLE_RATE_16K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_16K}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SampleRateMenuHandler}}
};

#define SAMPLE_RATE_BARGRAPH_MENU_ENTRIES 7
USER_MENU_STRUCT sampleRateBargraphMenu[SAMPLE_RATE_BARGRAPH_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SAMPLE_RATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SAMPLE_RATE_BARGRAPH_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, SAMPLE_RATE_512, NULL_TEXT, NO_TAG, {MIN_SAMPLE_RATE}}, // 512
{ITEM_2, SAMPLE_RATE_1K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_1K}},
{ITEM_3, SAMPLE_RATE_2K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_2K}},
{ITEM_4, SAMPLE_RATE_4K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_4K}},
{ITEM_5, SAMPLE_RATE_8K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_8K}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SampleRateMenuHandler}}
};

#define SAMPLE_RATE_COMBO_MENU_ENTRIES 6
USER_MENU_STRUCT sampleRateComboMenu[SAMPLE_RATE_COMBO_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, SAMPLE_RATE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, SAMPLE_RATE_COMBO_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_2)}},
{ITEM_1, SAMPLE_RATE_512, NULL_TEXT, NO_TAG, {MIN_SAMPLE_RATE}}, // 512
{ITEM_2, SAMPLE_RATE_1K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_1K}},
{ITEM_3, SAMPLE_RATE_2K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_2K}},
{ITEM_4, SAMPLE_RATE_4K, NULL_TEXT, NO_TAG, {SAMPLE_RATE_4K}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SampleRateMenuHandler}}
};

//-------------------------
// Sample Rate Menu Handler
//-------------------------
void SampleRateMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);
	char message[70];

	if (keyPressed == ENTER_KEY)
	{
		if ((sampleRateMenu[newItemIndex].data < sampleRateMenu[ITEM_1].data) || 
			(sampleRateMenu[newItemIndex].data > sampleRateMenu[ITEM_6].data) ||
			((sampleRateMenu[newItemIndex].data > sampleRateMenu[ITEM_5].data) && (g_triggerRecord.op_mode == BARGRAPH_MODE)) ||
			((sampleRateMenu[newItemIndex].data > sampleRateMenu[ITEM_4].data) && (g_triggerRecord.op_mode == COMBO_MODE)))
		{
			ByteSet(&message[0], 0, sizeof(message));
			sprintf((char*)message, "%lu %s", sampleRateMenu[newItemIndex].data,
					getLangText(CURRENTLY_NOT_IMPLEMENTED_TEXT));
			MessageBox(getLangText(STATUS_TEXT), (char*)message, MB_OK);

			debug("Sample Rate: %d is not supported for this mode.\n", g_triggerRecord.trec.record_time_max);

			SETUP_USER_MENU_MSG(&sampleRateMenu, sampleRateMenu[DEFAULT_ITEM_2].data);
		}
		else
		{
			g_triggerRecord.trec.sample_rate = sampleRateMenu[newItemIndex].data;

			// fix_ns8100 - Add in Combo mode calc that accounts for Bargraph buffers
			g_triggerRecord.trec.record_time_max = (uint16)(((uint32)((EVENT_BUFF_SIZE_IN_WORDS -
#if 0 // Fixed Pretrigger size
				((g_triggerRecord.trec.sample_rate / 4) * g_sensorInfoPtr->numOfChannels) -
#else // Variable Pretrigger size
				((g_triggerRecord.trec.sample_rate / g_helpRecord.pretrigBufferDivider) * g_sensorInfoPtr->numOfChannels) -
#endif
				((g_triggerRecord.trec.sample_rate / MIN_SAMPLE_RATE) * MAX_CAL_SAMPLES * g_sensorInfoPtr->numOfChannels)) /
				(g_triggerRecord.trec.sample_rate * g_sensorInfoPtr->numOfChannels))));

			debug("New Max Record Time: %d\n", g_triggerRecord.trec.record_time_max);

			SETUP_USER_MENU_MSG(&bitAccuracyMenu, g_triggerRecord.trec.bitAccuracy);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		UpdateModeMenuTitle(g_triggerRecord.op_mode);
		SETUP_USER_MENU_MSG(&modeMenu, MONITOR);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SaveSetupMenuHandler}}
};

//------------------------
// Save Setup Menu Handler
//------------------------
void SaveSetupMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		if (saveSetupMenu[newItemIndex].data == YES)
		{
			SETUP_USER_MENU_MSG(&saveRecordMenu, (uint8)0);
		}
		else // User selected NO
		{
			// Save the current trig record into the default location
			SaveRecordData(&g_triggerRecord, DEFAULT_RECORD, REC_TRIGGER_USER_MENU_TYPE);

			UpdateModeMenuTitle(g_triggerRecord.op_mode);
			SETUP_USER_MENU_MSG(&modeMenu, MONITOR);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		switch (g_triggerRecord.op_mode)
		{
			case WAVEFORM_MODE:
			case COMBO_MODE:
				if ((g_helpRecord.alarmOneMode == ALARM_MODE_OFF) &&
					(g_helpRecord.alarmTwoMode == ALARM_MODE_OFF))
				{
					if ((!g_factorySetupRecord.invalid) && (g_factorySetupRecord.aweight_option == ENABLED))
					{
						SETUP_USER_MENU_MSG(&airScaleMenu, 0);
					}
					else
					{
						SETUP_USER_MENU_FOR_INTEGERS_MSG(&recordTimeMenu, &g_triggerRecord.trec.record_time,
							RECORD_TIME_DEFAULT_VALUE, RECORD_TIME_MIN_VALUE, g_triggerRecord.trec.record_time_max);
					}
				}
				else
				{
					if (g_helpRecord.alarmTwoMode == ALARM_MODE_OFF)
					{
						SETUP_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarmTwoMode);
					}
					else
					{
						SETUP_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &g_helpRecord.alarmTwoTime,
							ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT,
							ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
					}
				}
			break;

			case BARGRAPH_MODE:
				if ((g_helpRecord.alarmOneMode == ALARM_MODE_OFF) &&
					(g_helpRecord.alarmTwoMode == ALARM_MODE_OFF))
				{
					SETUP_USER_MENU_MSG(&barResultMenu, g_helpRecord.vectorSum);
				}
				else
				{
					if (g_helpRecord.alarmTwoMode == ALARM_MODE_OFF)
					{
						SETUP_USER_MENU_MSG(&alarmTwoMenu, g_helpRecord.alarmTwoMode);
					}
					else
					{
						SETUP_USER_MENU_FOR_FLOATS_MSG(&alarmTwoTimeMenu, &g_helpRecord.alarmTwoTime,
							ALARM_OUTPUT_TIME_DEFAULT, ALARM_OUTPUT_TIME_INCREMENT,
							ALARM_OUTPUT_TIME_MIN, ALARM_OUTPUT_TIME_MAX);
					}
				}
			break;
		}
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SensitivityMenuHandler}}
};

//-------------------------
// Sensitivity Menu Handler
//-------------------------
void SensitivityMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
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

#if 1 // ns8100 - Down convert to current bit accuracy setting
			if (g_triggerRecord.trec.seismicTriggerLevel != NO_TRIGGER_CHAR)
			{
				g_tempTriggerLevelForMenuAdjsutment = g_triggerRecord.trec.seismicTriggerLevel / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint);
			}		
#endif
			SETUP_USER_MENU_FOR_INTEGERS_MSG(&seismicTriggerMenu, &g_tempTriggerLevelForMenuAdjsutment,
				(SEISMIC_TRIGGER_DEFAULT_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
				(SEISMIC_TRIGGER_MIN_VALUE / (SEISMIC_TRIGGER_MAX_VALUE / g_bitAccuracyMidpoint)),
				g_bitAccuracyMidpoint);
		}
		else if ((g_triggerRecord.op_mode == BARGRAPH_MODE) || (g_triggerRecord.op_mode == COMBO_MODE))
		{
			SETUP_USER_MENU_MSG(&barChannelMenu, g_triggerRecord.berec.barChannel);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&operatorMenu, &g_triggerRecord.trec.oper);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&SensorTypeMenuHandler}}
};

//-------------------------
// Sensor Type Menu Handler
//-------------------------
void SensorTypeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_factorySetupRecord.sensor_type = (uint16)sensorTypeMenu[newItemIndex].data;

		if (g_factorySetupRecord.sensor_type == SENSOR_ACC) { debug("Factory Setup: New Seismic sensor type: Accelerometer\n"); }
		else { debug("Factory Setup: New Seismic sensor type: %3.1f in\n", (float)g_factorySetupRecord.sensor_type / (float)204.8); }

		if (g_factorySetupSequence == PROCESS_FACTORY_SETUP)
		{
			SETUP_USER_MENU_MSG(&airSetupMenu, g_factorySetupRecord.aweight_option);
		}
		else
		{
			SETUP_USER_MENU_MSG(&helpMenu, CONFIG);
		}
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_factorySetupSequence == PROCESS_FACTORY_SETUP)
		{
			SETUP_USER_MENU_MSG(&serialNumberMenu, &g_factorySetupRecord.serial_num);
		}
		else
		{
			SETUP_USER_MENU_MSG(&helpMenu, CONFIG);
		}
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&TimerModeMenuHandler}}
};

//------------------------
// Timer Mode Menu Handler
//------------------------
void TimerModeMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.timerMode = (uint8)timerModeMenu[newItemIndex].data;

		if (g_helpRecord.timerMode == ENABLED)
		{
			SETUP_USER_MENU_MSG(&timerModeFreqMenu, g_helpRecord.timerModeFrequency);
		}
		else
		{
			SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
		}

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
	}
	else if (keyPressed == ESC_KEY)
	{
		if (g_helpRecord.timerMode == ENABLED)
		{
			g_helpRecord.timerMode = DISABLED;

			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

			// Disable the Power Off timer if it's set
			ClearSoftTimer(POWER_OFF_TIMER_NUM);

			MessageBox(getLangText(STATUS_TEXT), getLangText(TIMER_MODE_SETUP_NOT_COMPLETED_TEXT), MB_OK);
			MessageBox(getLangText(STATUS_TEXT), getLangText(DISABLING_TIMER_MODE_TEXT), MB_OK);
		}

		SETUP_USER_MENU_MSG(&configMenu, TIMER_MODE);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Timer Mode Freq Menu
//=============================================================================
//*****************************************************************************
#define TIMER_MODE_FREQ_MENU_ENTRIES 8
USER_MENU_STRUCT timerModeFreqMenu[TIMER_MODE_FREQ_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, TIMER_FREQUENCY_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, TIMER_MODE_FREQ_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, ONE_TIME_TEXT,			NO_TAG, {TIMER_MODE_ONE_TIME}},
{ITEM_2, 0, HOURLY_TEXT,			NO_TAG, {TIMER_MODE_HOURLY}},
{ITEM_3, 0, DAILY_EVERY_DAY_TEXT,	NO_TAG, {TIMER_MODE_DAILY}},
{ITEM_4, 0, DAILY_WEEKDAYS_TEXT,	NO_TAG, {TIMER_MODE_WEEKDAYS}},
{ITEM_5, 0, WEEKLY_TEXT,			NO_TAG, {TIMER_MODE_WEEKLY}},
{ITEM_6, 0, MONTHLY_TEXT,			NO_TAG, {TIMER_MODE_MONTHLY}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&TimerModeFreqMenuHandler}}
};

//-----------------------------
// Timer Mode Freq Menu Handler
//-----------------------------
void TimerModeFreqMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.timerModeFrequency = (uint8)timerModeFreqMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		if (g_helpRecord.timerModeFrequency == TIMER_MODE_HOURLY)
		{
			MessageBox("TIMER MODE", "FOR HOURLY MODE THE HOURS AND MINUTES FIELDS ARE INDEPENDENT", MB_OK);
			MessageBox("TIMER MODE HOURLY", "HOURS = ACTIVE HOURS, MINS = ACTIVE MIN RANGE EACH HOUR", MB_OK);
		}
		
		SETUP_MENU_MSG(TIMER_MODE_TIME_MENU);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&timerModeMenu, g_helpRecord.timerMode);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Units Of Measure Menu
//=============================================================================
//*****************************************************************************
#define UNITS_OF_MEASURE_MENU_ENTRIES 4
USER_MENU_STRUCT unitsOfMeasureMenu[UNITS_OF_MEASURE_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, UNITS_OF_MEASURE_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, UNITS_OF_MEASURE_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, IMPERIAL_TEXT,	NO_TAG, {IMPERIAL_TYPE}},
{ITEM_2, 0, METRIC_TEXT,	NO_TAG, {METRIC_TYPE}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&UnitsOfMeasureMenuHandler}}
};

//------------------------------
// Units Of Measure Menu Handler
//------------------------------
void UnitsOfMeasureMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.unitsOfMeasure = (uint8)unitsOfMeasureMenu[newItemIndex].data;

		if (g_helpRecord.unitsOfMeasure == IMPERIAL_TYPE)
		{
			g_sensorInfoPtr->unitsFlag = IMPERIAL_TYPE;
			g_sensorInfoPtr->measurementRatio = (float)IMPERIAL;
		}
		else // Metric
		{
			g_sensorInfoPtr->unitsFlag = METRIC_TYPE;
			g_sensorInfoPtr->measurementRatio = (float)METRIC;
		}

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, UNITS_OF_MEASURE);
	}

	JUMP_TO_ACTIVE_MENU();
}

//*****************************************************************************
//=============================================================================
// Units Of Air Menu
//=============================================================================
//*****************************************************************************
#define UNITS_OF_AIR_MENU_ENTRIES 4
USER_MENU_STRUCT unitsOfAirMenu[UNITS_OF_AIR_MENU_ENTRIES] = {
{TITLE_PRE_TAG, 0, UNITS_OF_AIR_TEXT, TITLE_POST_TAG,
	{INSERT_USER_MENU_INFO(SELECT_TYPE, UNITS_OF_AIR_MENU_ENTRIES, TITLE_CENTERED, DEFAULT_ITEM_1)}},
{ITEM_1, 0, DECIBEL_TEXT,	NO_TAG, {DECIBEL_TYPE}},
{ITEM_2, 0, MILLIBAR_TEXT,	NO_TAG, {MILLIBAR_TYPE}},
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&UnitsOfAirMenuHandler}}
};

//--------------------------
// Units Of Air Menu Handler
//--------------------------
void UnitsOfAirMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

#if 1 // Port lost change
	if (keyPressed == ENTER_KEY)
	{
		if (g_helpRecord.unitsOfAir != (uint8)unitsOfAirMenu[newItemIndex].data)
		{
			g_helpRecord.unitsOfAir = (uint8)unitsOfAirMenu[newItemIndex].data;

			if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
			{
				g_sensorInfoPtr->airUnitsFlag = DB_TYPE;
				g_sensorInfoPtr->ameasurementRatio = (float)DECIBEL;
			}
			else // (g_helpRecord.unitsOfAir == MILLIBAR_TYPE)
			{
				g_sensorInfoPtr->airUnitsFlag = MB_TYPE;
				g_sensorInfoPtr->ameasurementRatio = (float)MILLIBAR;
			}

			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
		}
#else // Air in units value
	if (keyPressed == ENTER_KEY)
	{
		if (g_helpRecord.unitsOfAir != (uint8)unitsOfAirMenu[newItemIndex].data)
		{
			g_helpRecord.unitsOfAir = (uint8)unitsOfAirMenu[newItemIndex].data;

			// Check if the units of air was changed to dB, which means the prior mb values need to be converted
			if (g_helpRecord.unitsOfAir == DECIBEL_TYPE)
			{
				g_sensorInfoPtr->airUnitsFlag = DB_TYPE;
				g_sensorInfoPtr->ameasurementRatio = (float)DECIBEL;

				debug("Units of Air: Convert from MB value %f\n", (float)((float)g_triggerRecord.trec.airTriggerLevel / (float)10000));

				g_triggerRecord.trec.airTriggerLevel = ConvertMBtoDB(g_triggerRecord.trec.airTriggerLevel);
				g_helpRecord.alarmOneAirMinLevel = ConvertMBtoDB(g_helpRecord.alarmOneAirMinLevel);
				g_helpRecord.alarmTwoAirMinLevel = ConvertMBtoDB(g_helpRecord.alarmTwoAirMinLevel);
				g_helpRecord.alarmOneAirLevel = ConvertMBtoDB(g_helpRecord.alarmOneAirLevel);
				g_helpRecord.alarmTwoAirLevel = ConvertMBtoDB(g_helpRecord.alarmTwoAirLevel);

				debug("Units of Air: Convert to DB value %d\n", g_triggerRecord.trec.airTriggerLevel);
			}
			else // g_helpRecord.unitsOfAir == MILLIBAR_TYPE, units of air now mb, change from prior dB
			{
				g_sensorInfoPtr->airUnitsFlag = MB_TYPE;
				g_sensorInfoPtr->ameasurementRatio = (float)MILLIBAR;

				debug("Units of Air: Convert from DB value %d\n", g_triggerRecord.trec.airTriggerLevel);

				g_triggerRecord.trec.airTriggerLevel = ConvertDBtoMB(g_triggerRecord.trec.airTriggerLevel);
				g_helpRecord.alarmOneAirMinLevel = ConvertDBtoMB(g_helpRecord.alarmOneAirMinLevel);
				g_helpRecord.alarmTwoAirMinLevel = ConvertDBtoMB(g_helpRecord.alarmTwoAirMinLevel);
				g_helpRecord.alarmOneAirLevel = ConvertDBtoMB(g_helpRecord.alarmOneAirLevel);
				g_helpRecord.alarmTwoAirLevel = ConvertDBtoMB(g_helpRecord.alarmTwoAirLevel);

				debug("Units of Air: Convert to MB value %f\n", (float)((float)g_triggerRecord.trec.airTriggerLevel / (float)10000));
			}

			SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
		}
#endif

#if 0 // ns7100
		SETUP_MENU_MSG(CAL_SETUP_MENU);
#else // ns8100
		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
#endif
	}
	else if (keyPressed == ESC_KEY)
	{
#if 0 // ns7100
		SETUP_USER_MENU_MSG(&airSetupMenu, g_factorySetupRecord.aweight_option);
#else // ns8100
		SETUP_USER_MENU_MSG(&configMenu, UNITS_OF_AIR);
#endif
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&VectorSumMenuHandler}}
};

//------------------------
// Vector Sum Menu Handler
//------------------------
void VectorSumMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if (keyPressed == ENTER_KEY)
	{
		g_helpRecord.vectorSum = (uint8)vectorSumMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, VECTOR_SUM);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&WaveformAutoCalMenuHandler}}
};

//-------------------------------
// Waveform Auto Cal Menu Handler
//-------------------------------
void WaveformAutoCalMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);

	if(keyPressed == ENTER_KEY)
	{
		g_helpRecord.autoCalForWaveform = (uint8)waveformAutoCalMenu[newItemIndex].data;

		SaveRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

		SETUP_USER_MENU_MSG(&configMenu, DEFAULT_ITEM_1);
	}
	else if(keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&configMenu, WAVEFORM_AUTO_CAL);
	}

	JUMP_TO_ACTIVE_MENU();
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
{END_OF_MENU, (uint8)0, (uint8)0, (uint8)0, {(uint32)&ZeroEventNumberMenuHandler}}
};

//-------------------------------
// Zero Event Number Menu Handler
//-------------------------------
void ZeroEventNumberMenuHandler(uint8 keyPressed, void* data)
{
	INPUT_MSG_STRUCT mn_msg = {0, 0, {}};
	uint16 newItemIndex = *((uint16*)data);
	CURRENT_EVENT_NUMBER_STRUCT eventNumberRecord;

	if (keyPressed == ENTER_KEY)
	{
		if (zeroEventNumberMenu[newItemIndex].data == YES)
		{
#if 0 // ns7100
			SectorErase((uint16*)(FLASH_BASE_ADDR + FLASH_BOOT_SECTOR_SIZE_x8), 1);
#else // ns8100
			eventNumberRecord.currentEventNumber = 0;
			SaveRecordData(&eventNumberRecord, DEFAULT_RECORD, REC_UNIQUE_EVENT_ID_TYPE);
#endif
			InitCurrentEventNumber();

			__autoDialoutTbl.lastDownloadedEvent = 0;

			OverlayMessage(getLangText(SUCCESS_TEXT), getLangText(EVENT_NUMBER_ZEROING_COMPLETE_TEXT), (2 * SOFT_SECS));
		}

#if 0 // Port lost change
		SETUP_USER_MENU_MSG(&eraseSettingsMenu, YES);
#else // Updated
		SETUP_USER_MENU_MSG(&eraseSettingsMenu, NO);
#endif
	}
	else if (keyPressed == ESC_KEY)
	{
		SETUP_USER_MENU_MSG(&eraseEventsMenu, YES);
	}

	JUMP_TO_ACTIVE_MENU();
}
