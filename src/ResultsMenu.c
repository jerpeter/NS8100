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
#include <math.h>
#include "Typedefs.h"
#include "Menu.h"
#include "Display.h"
#include "Common.h"
#include "InitDataBuffers.h"
#include "Summary.h"
#include "SysEvents.h"
#include "EventProcessing.h"
#include "Uart.h"
#include "Keypad.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define RESULTS_MN_TABLE_SIZE DEFAULT_MN_SIZE
#define RESULTS_WND_STARTING_COL DEFAULT_COL_THREE
#define RESULTS_WND_END_COL DEFAULT_END_COL
#define RESULTS_WND_STARTING_ROW DEFAULT_MENU_ROW_TWO
#define RESULTS_WND_END_ROW (DEFAULT_MENU_ROW_SEVEN - 1)
#define RESULTS_MN_TBL_START_LINE 0
#define TOTAL_DOTS 4

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static uint16 s_monitorSessionFirstEvent = 0;
static uint16 s_monitorSessionLastEvent = 0;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void ResultsMenuDisplay(WND_LAYOUT_STRUCT*);
void ResultsMenuProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ResultsMenu(INPUT_MSG_STRUCT msg)
{
	static WND_LAYOUT_STRUCT wnd_layout;
	static MN_LAYOUT_STRUCT mn_layout;

	ResultsMenuProc(msg, &wnd_layout, &mn_layout);

	if (g_activeMenu == RESULTS_MENU)
	{
		ResultsMenuDisplay(&wnd_layout);
		WriteMapToLcd(g_mmap);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ResultsMenuProc(INPUT_MSG_STRUCT msg, WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	uint32 delay = 3 * TICKS_PER_SEC;

	if (msg.cmd == ACTIVATE_MENU_CMD)
	{
		wnd_layout_ptr->start_col =	RESULTS_WND_STARTING_COL;
		wnd_layout_ptr->end_col =	RESULTS_WND_END_COL;
		wnd_layout_ptr->start_row =	RESULTS_WND_STARTING_ROW;
		wnd_layout_ptr->end_row =	RESULTS_WND_END_ROW;
		mn_layout_ptr->curr_ln =	RESULTS_MN_TBL_START_LINE;
		mn_layout_ptr->top_ln =		RESULTS_MN_TBL_START_LINE;
	}
	else if (msg.cmd == ACTIVATE_MENU_WITH_DATA_CMD)
	{
		wnd_layout_ptr->start_col =	RESULTS_WND_STARTING_COL;
		wnd_layout_ptr->end_col =	RESULTS_WND_END_COL;
		wnd_layout_ptr->start_row =	RESULTS_WND_STARTING_ROW;
		wnd_layout_ptr->end_row =	RESULTS_WND_END_ROW;
		mn_layout_ptr->curr_ln =	RESULTS_MN_TBL_START_LINE;
		mn_layout_ptr->top_ln =		RESULTS_MN_TBL_START_LINE;

		g_resultsRamSummaryPtr = g_lastCompletedRamSummaryIndex;
		g_updateResultsEventRecord = YES;
		
		s_monitorSessionFirstEvent = GetStartingEventNumberForCurrentMonitorLog();
		s_monitorSessionLastEvent = GetUniqueEventNumber(g_resultsRamSummaryPtr);

		// Check if we have recorded enough events to wrap the ram summary table
		if ((s_monitorSessionLastEvent - s_monitorSessionFirstEvent) >= TOTAL_RAM_SUMMARIES)
		{
			// Set the first event number to the oldest ram summary entry reference event number
			s_monitorSessionFirstEvent = (uint16)(s_monitorSessionLastEvent - TOTAL_RAM_SUMMARIES + 1);
		}

		debug("g_lastCompletedRamSummaryIndex Event Number = %d\r\n", g_lastCompletedRamSummaryIndex->fileEventNum);

		// Check if data corresponds to a Calibration Pulse
		if (msg.data[0] == 13)
		{
			// If the unit was monitoring prior to the auto-cal, call soft timer to
			// reenter monitor mode in the near future
			if (g_enterMonitorModeAfterMidnightCal == YES)
			{
				// Reset flag
				g_enterMonitorModeAfterMidnightCal = NO;

				// Check if Auto Cal is enabled
				if (g_unitConfig.autoCalForWaveform == YES)
				{
					// Set flag to skip auto calibration at start of waveform
					g_skipAutoCalInWaveformAfterMidnightCal = YES;
				}

				// Assign a timer to re-enter monitor mode
				AssignSoftTimer(AUTO_MONITOR_TIMER_NUM, delay, AutoMonitorTimerCallBack);
			}
		}
	}
	else if (msg.cmd == KEYPRESS_MENU_CMD)
	{
		if (g_waitForUser == TRUE)
		{
			switch (msg.data[0])
			{
				case (ENTER_KEY):
				case (ESC_KEY):
					g_waitForUser = FALSE;
					break;
				default :
					break;
			}
		}
		else if ((g_promtForCancelingPrintJobs == TRUE) || (g_promtForLeavingMonitorMode == TRUE))
		{
			switch (msg.data[0])
			{
				case (ENTER_KEY):
					if (g_promtForCancelingPrintJobs == TRUE)
					{
						if (g_monitorModeActiveChoice == MB_FIRST_CHOICE)
						{
						}

						// Done handling cancel print jobs, now handle leaving monitor mode
						g_promtForCancelingPrintJobs = FALSE;
						g_promtForLeavingMonitorMode = TRUE;
					}
					else if (g_promtForLeavingMonitorMode == TRUE)
					{
						if (g_monitorModeActiveChoice == MB_FIRST_CHOICE)
						{
							StopMonitoring(g_triggerRecord.opMode, EVENT_PROCESSING);

							SETUP_MENU_MSG(MAIN_MENU);
							JUMP_TO_ACTIVE_MENU();
						}

						g_promtForLeavingMonitorMode = FALSE;
					}

					g_monitorModeActiveChoice = MB_FIRST_CHOICE;
				break;

				case (ESC_KEY):
					// Do not process the escape key
				break;

				case UP_ARROW_KEY:
					if (g_monitorModeActiveChoice == MB_SECOND_CHOICE)
						g_monitorModeActiveChoice = MB_FIRST_CHOICE;

				break;

				case DOWN_ARROW_KEY:
					if (g_monitorModeActiveChoice == MB_FIRST_CHOICE)
						g_monitorModeActiveChoice = MB_SECOND_CHOICE;

				break;
			}
		}
		else // (g_waitForUser == FLASE) && (g_promtForLeavingMonitorMode == FALSE)
		{
			switch (msg.data[0])
			{
				case (ENTER_KEY):
					if (g_sampleProcessing != ACTIVE_STATE)
					{
						MessageBox(getLangText(STATUS_TEXT), getLangText(NOT_INCLUDED_TEXT), MB_OK);
					}
				break;

				case (ESC_KEY):
					if (g_sampleProcessing == ACTIVE_STATE)
					{
						g_monitorEscapeCheck = YES;
						g_promtForLeavingMonitorMode = TRUE;
					}
					else // g_sampleProcessing != ACTIVE_STATE
					{
 						if (g_summaryListMenuActive == YES)
 						{
							SETUP_MENU_MSG(SUMMARY_MENU); msg.data[0] = ESC_KEY;
						}
						else
						{
							SETUP_MENU_MSG(MAIN_MENU);
						}

						JUMP_TO_ACTIVE_MENU();
					}
				break;

				case (PLUS_KEY):
					switch (g_displayAlternateResultState)
					{
						case DEFAULT_RESULTS: g_displayAlternateResultState = DEFAULT_ALTERNATE_RESULTS; break;
						case DEFAULT_ALTERNATE_RESULTS: g_displayAlternateResultState = VECTOR_SUM_RESULTS; break;
						case VECTOR_SUM_RESULTS: g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS; break;
						case PEAK_DISPLACEMENT_RESULTS: g_displayAlternateResultState = PEAK_ACCELERATION_RESULTS; break;
						case PEAK_ACCELERATION_RESULTS: g_displayAlternateResultState = DEFAULT_RESULTS; break;
					}
				break;

				case (MINUS_KEY):
					switch (g_displayAlternateResultState)
					{
						case DEFAULT_RESULTS: g_displayAlternateResultState = PEAK_ACCELERATION_RESULTS; break;
						case DEFAULT_ALTERNATE_RESULTS: g_displayAlternateResultState = DEFAULT_RESULTS; break;
						case VECTOR_SUM_RESULTS: g_displayAlternateResultState = DEFAULT_ALTERNATE_RESULTS; break;
						case PEAK_DISPLACEMENT_RESULTS: g_displayAlternateResultState = VECTOR_SUM_RESULTS; break;
						case PEAK_ACCELERATION_RESULTS: g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS; break;
					}
				break;

				case (DOWN_ARROW_KEY):
				case (UP_ARROW_KEY):
					if (g_sampleProcessing == ACTIVE_STATE)
					{
						if (msg.data[0] == DOWN_ARROW_KEY)
						{
							if (GetUniqueEventNumber(g_resultsRamSummaryPtr) < s_monitorSessionLastEvent)
							{
								g_resultsRamSummaryPtr++;
								g_updateResultsEventRecord = YES;

								if (g_resultsRamSummaryPtr > &__ramFlashSummaryTbl[LAST_RAM_SUMMARY_INDEX])
								{
									g_resultsRamSummaryPtr = &__ramFlashSummaryTbl[0];
								}
							}
						}
						else // msg.data[0] == UP_ARROW_KEY
						{
							if (GetUniqueEventNumber(g_resultsRamSummaryPtr) > s_monitorSessionFirstEvent)
							{
								g_resultsRamSummaryPtr--;
								g_updateResultsEventRecord = YES;

								if (g_resultsRamSummaryPtr < &__ramFlashSummaryTbl[0])
								{
									g_resultsRamSummaryPtr = &__ramFlashSummaryTbl[LAST_RAM_SUMMARY_INDEX];
								}
							}
						}
					}
					else // (g_sampleProcessing == IDLE_STATE)
					{
						SETUP_MENU_MSG(SUMMARY_MENU); mn_msg.data[0] = ESC_KEY;
						JUMP_TO_ACTIVE_MENU();

						mn_msg.length = 1;
						mn_msg.cmd = KEYPRESS_MENU_CMD;
						mn_msg.data[0] = msg.data[0];
						SendInputMsg(&mn_msg);

						mn_msg.length = 1;
						mn_msg.cmd = KEYPRESS_MENU_CMD;
						mn_msg.data[0] = ENTER_KEY;
						SendInputMsg(&mn_msg);
					}
				break;

				default:
					break;
			}
		}
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void ResultsMenuDisplay(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
	static uint8 dotState = 0;
	uint8 adjust;
	uint8 length = 0, i = 0;
	uint8 dotBuff[TOTAL_DOTS];
	uint8 gainFactor;
	uint8 arrowChar;
	float div;
	float normalize_max_peak;
	float tempVS;
	float tempPeakDisp = 0;
	float tempPeakAcc;
	char buff[50];
	char srBuff[6];
	char displayFormat[10];
	DATE_TIME_STRUCT time;
	uint16 bitAccuracyScale;
	uint8 calResults = PASSED;

	if ((g_updateResultsEventRecord == YES) || (g_forcedCalibration == YES))
	{
		if (g_summaryListMenuActive == YES)
		{
			// Results cached in g_summaryList.cachedEntry
		}			
		else
		{
			GetSummaryFromSummaryList(g_resultsRamSummaryPtr->fileEventNum);
		}		

		g_updateResultsEventRecord = NO;

		debugRaw("\n\tResults Evt: %04d, Mode: %d\r\n", g_summaryList.cachedEntry.eventNumber, g_summaryList.cachedEntry.mode);
		debugRaw("\tStored peaks: a:%x r:%x v:%x t:%x\r\n", g_summaryList.cachedEntry.channelSummary.a.peak, g_summaryList.cachedEntry.channelSummary.r.peak,
					g_summaryList.cachedEntry.channelSummary.v.peak, g_summaryList.cachedEntry.channelSummary.t.peak);
	}

	memset(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	//-------------------------------------------------------------
	// Event specific scaling factors
	//-------------------------------------------------------------
	
	// Set the gain factor that was used to record the event (sensitivity)
	if ((g_summaryList.cachedEntry.gainSelect & 0x01) == GAIN_SELECT_x2) { gainFactor = 2; }
	else { gainFactor = 4; }

	// Set the scale based on the stored bit accuracy the event was recorded with
	switch (g_summaryList.cachedEntry.bitAccuracy)
	{
		case ACCURACY_10_BIT: { bitAccuracyScale = ACCURACY_10_BIT_MIDPOINT; } break;
		case ACCURACY_12_BIT: {	bitAccuracyScale = ACCURACY_12_BIT_MIDPOINT; } break;
		case ACCURACY_14_BIT: { bitAccuracyScale = ACCURACY_14_BIT_MIDPOINT; } break;
		default: // ACCURACY_16_BIT
			bitAccuracyScale = ACCURACY_16_BIT_MIDPOINT;
			break;
	}

	// Calculate the divider used for converting stored A/D peak counts to units of measure
	div = (float)(bitAccuracyScale * SENSOR_ACCURACY_100X_SHIFT * gainFactor) / (float)(g_summaryList.cachedEntry.seismicSensorType);

	//-----------------------------------------------------------------------
	// PRINT MONITORING
	//-----------------------------------------------------------------------
	wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;

	if (g_sampleProcessing == ACTIVE_STATE)
	{
		memset(&buff[0], 0, sizeof(buff));
		memset(&dotBuff[0], 0, sizeof(dotBuff));

		for (i = 0; i < dotState; i++)		dotBuff[i] = '.';
		for (; i < (TOTAL_DOTS-1); i++)		dotBuff[i] = ' ';
		if (++dotState >= TOTAL_DOTS)		dotState = 0;

		if (g_triggerRecord.trec.sample_rate == 512)
			sprintf((char*)srBuff, ".5K");
		else
			sprintf((char*)srBuff, "%dK", (int)(g_triggerRecord.trec.sample_rate / SAMPLE_RATE_1K));

		if (g_monitorOperationMode == WAVEFORM_MODE)
		{
			if (g_busyProcessingEvent == YES)
			{
				length = (uint8)sprintf((char*)buff, "%s%s(W-%s)", getLangText(PROCESSING_TEXT), dotBuff, srBuff);
			}
			else
			{
				length = (uint8)sprintf((char*)buff, "%s%s(W-%s)", getLangText(MONITORING_TEXT), dotBuff, srBuff);
			}
		}
		else if (g_monitorOperationMode == BARGRAPH_MODE)
		{
			length = (uint8)sprintf((char*)buff, "%s%s(B-%s)", getLangText(MONITORING_TEXT), dotBuff, srBuff);
		}
		else if (g_monitorOperationMode == COMBO_MODE)
		{
			length = (uint8)sprintf((char*)buff, "%s%s(C-%s)", getLangText(MONITORING_TEXT), dotBuff, srBuff);
		}
	}
	else
	{
		switch (g_summaryList.cachedEntry.mode)
		{
			case MANUAL_CAL_MODE:
				length = sprintf(buff, "%s", getLangText(CAL_SUMMARY_TEXT));
				break;

			case WAVEFORM_MODE:
				length = sprintf(buff, "%s", getLangText(EVENT_SUMMARY_TEXT));
				break;

			case BARGRAPH_MODE:
				length = sprintf(buff, "%s", "BARGRAPH SUMMARY");
				break;

			case COMBO_MODE:
				if (g_summaryList.cachedEntry.subMode == WAVEFORM_MODE) { length = sprintf(buff, "%s", "COMBO - EVENT SUM"); }
				else if (g_summaryList.cachedEntry.subMode == BARGRAPH_MODE) { length = sprintf(buff, "%s", "COMBO - BARGRAPH"); }
				break;

			default:
				break;
		}
	}

	// Setup current column to center text
	wnd_layout_ptr->curr_col =
		(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

	// Write string to screen
	WndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr,SIX_BY_EIGHT_FONT,REG_LN);

	// Next line after title
	// Check if monitoring
	if (g_sampleProcessing == ACTIVE_STATE)
	{
		if (g_monitorOperationMode == WAVEFORM_MODE)
		{
			if (g_summaryList.cachedEntry.eventNumber == s_monitorSessionFirstEvent)
				arrowChar = DOWN_ARROW_CHAR;
			else if (g_summaryList.cachedEntry.eventNumber == s_monitorSessionLastEvent)
				arrowChar = UP_ARROW_CHAR;
			else
				arrowChar = BOTH_ARROWS_CHAR;

			if (s_monitorSessionFirstEvent != s_monitorSessionLastEvent)
			{
				sprintf(buff, "%c", arrowChar);
				wnd_layout_ptr->curr_col = 120;
				WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			}
		}
	}
	else // Not monitoring
	{
		if (g_summaryListMenuActive == YES)
		{
			sprintf(buff, "%c", g_summaryListArrowChar);
			wnd_layout_ptr->curr_col = 120;
			WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
	}

	// Advance to next row
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	if ((g_sampleProcessing == ACTIVE_STATE) && (g_summaryList.cachedEntry.mode != MANUAL_CAL_MODE))
	{
		// Date & Time
		time = GetCurrentTime();
		memset(&buff[0], 0, sizeof(buff));
		length = (uint8)sprintf(buff,"(%02d:%02d:%02d)", time.hour, time.min, time.sec);

		// Setup current column, Write string to screen, Advance to next row
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		WndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr,SIX_BY_EIGHT_FONT,REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
	}

	//-----------------------------------------------------------------------
	// Results
	//-----------------------------------------------------------------------

	wnd_layout_ptr->curr_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col = wnd_layout_ptr->start_col;

	//-------------------------------------------------------------
	// Date Time Info
	memset(&buff[0], 0, sizeof(buff));
	ConvertTimeStampToString(buff, &g_summaryList.cachedEntry.eventTime, REC_DATE_TIME_DISPLAY);

	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	//-------------------------------------------------------------
	// Event number
	memset(&buff[0], 0, sizeof(buff));

	// Remove commented code assuming display works
	sprintf(buff, "%s %04d", getLangText(EVENT_TEXT), g_summaryList.cachedEntry.eventNumber);

	WndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr,SIX_BY_EIGHT_FONT,REG_LN);

	//-------------------------------------------------------------
	// Units inches or millimeters LABEL
	memset(&buff[0], 0, sizeof(buff));
	if (g_summaryList.cachedEntry.seismicSensorType == SENSOR_ACC)
	{
		sprintf(buff, "mg/s");
	}
	else if (g_sensorInfo.unitsFlag == IMPERIAL_TYPE)
	{
		sprintf(buff, "in/s");
	}
	else
	{
		sprintf(buff, "mm/s");
	}

	// Setup current column, Write string to screen, Advance to next row
	wnd_layout_ptr->curr_col = (uint16)(wnd_layout_ptr->next_col + 12);
	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	//-------------------------------------------------------------
	// R LABEL, T LABEL, V LABEL
	memset(&buff[0], 0, sizeof(buff));
	sprintf(buff,"   R      T      V");

	// Setup current column, Write string to screen,
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	//-------------------------------------------------------------
	// R DATA
	// Using the Sensor times 100 definition.
	normalize_max_peak = (float)g_summaryList.cachedEntry.channelSummary.r.peak / (float)div;

	if (g_summaryList.cachedEntry.mode == MANUAL_CAL_MODE)
	{
		if ((normalize_max_peak < 0.375) || (normalize_max_peak > 0.625))
		calResults = FAILED;
	}

	if ((g_summaryList.cachedEntry.seismicSensorType != SENSOR_ACC) && (g_unitConfig.unitsOfMeasure == METRIC_TYPE))
	{
		normalize_max_peak *= (float)METRIC;
	}

	memset(&buff[0], 0, sizeof(buff));
	if (normalize_max_peak >= 100)
		sprintf(buff, "%05.2f", normalize_max_peak);
	else if (normalize_max_peak >= 10)
		sprintf(buff, "%05.3f", normalize_max_peak);
	else
		sprintf(buff, "%05.4f", normalize_max_peak);

	// Setup current column, Write string to screen,
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	//-------------------------------------------------------------
	// T DATA
	normalize_max_peak = (float)g_summaryList.cachedEntry.channelSummary.t.peak / (float)div;

	if (g_summaryList.cachedEntry.mode == MANUAL_CAL_MODE)
	{
		if ((normalize_max_peak < 0.375) || (normalize_max_peak > 0.625))
		calResults = FAILED;
	}

	if ((g_summaryList.cachedEntry.seismicSensorType != SENSOR_ACC) && (g_unitConfig.unitsOfMeasure == METRIC_TYPE))
	{
		normalize_max_peak *= (float)METRIC;
	}

	memset(&buff[0], 0, sizeof(buff));
	if (normalize_max_peak >= 100)
		sprintf(buff, "%05.2f", normalize_max_peak);
	else if (normalize_max_peak >= 10)
		sprintf(buff, "%05.3f", normalize_max_peak);
	else
		sprintf(buff, "%05.4f", normalize_max_peak);

	// Setup current column, Write string to screen,
	wnd_layout_ptr->curr_col = (uint16)(wnd_layout_ptr->next_col + 6);
	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	//-------------------------------------------------------------
	// V DATA
	normalize_max_peak = (float)g_summaryList.cachedEntry.channelSummary.v.peak / (float)div;

	if (g_summaryList.cachedEntry.mode == MANUAL_CAL_MODE)
	{
		if ((normalize_max_peak < 0.375) || (normalize_max_peak > 0.625))
		calResults = FAILED;
	}

	if ((g_summaryList.cachedEntry.seismicSensorType != SENSOR_ACC) && (g_unitConfig.unitsOfMeasure == METRIC_TYPE))
	{
		normalize_max_peak *= (float)METRIC;
	}

	memset(&buff[0], 0, sizeof(buff));
	if (normalize_max_peak >= 100)
		sprintf(buff, "%05.2f", normalize_max_peak);
	else if (normalize_max_peak >= 10)
		sprintf(buff, "%05.3f", normalize_max_peak);
	else
		sprintf(buff, "%05.4f", normalize_max_peak);

	// Setup current column, Write string to screen, Advance to next row
	wnd_layout_ptr->curr_col = (uint16)(wnd_layout_ptr->next_col + 6);
	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	//-------------------------------------------------------------
	// R FREQ, T FREQ, V FREQ
	memset(&buff[0], 0, sizeof(buff));
	if ((g_summaryList.cachedEntry.mode == BARGRAPH_MODE) ||
	((g_summaryList.cachedEntry.mode == COMBO_MODE) && (g_summaryList.cachedEntry.subMode == BARGRAPH_MODE)))
	{
		sprintf(buff,"%5.2f %5.2f %5.2f Hz",
		((float)g_summaryList.cachedEntry.sampleRate / (float)((g_summaryList.cachedEntry.channelSummary.r.frequency * 2) - 1)),
		((float)g_summaryList.cachedEntry.sampleRate / (float)((g_summaryList.cachedEntry.channelSummary.t.frequency * 2) - 1)),
		((float)g_summaryList.cachedEntry.sampleRate / (float)((g_summaryList.cachedEntry.channelSummary.v.frequency * 2) - 1)));
	}
	else // mode == WAVEFORM_MODE or MANUAL_CAL_MODE
	{
		sprintf(buff,"%5.2f %5.2f %5.2f Hz",
		((float)g_summaryList.cachedEntry.channelSummary.r.frequency / (float)10.0),
		((float)g_summaryList.cachedEntry.channelSummary.t.frequency / (float)10.0),
		((float)g_summaryList.cachedEntry.channelSummary.v.frequency / (float)10.0));
	}

	// Setup current column, Write string to screen,
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	if (g_displayAlternateResultState == VECTOR_SUM_RESULTS)
	{
		memset(&buff[0], 0, sizeof(buff));
		memset(&displayFormat[0], 0, sizeof(displayFormat));

		tempVS = sqrtf((float)g_summaryList.cachedEntry.vectorSumPeak) / (float)div;

		if ((g_sensorInfo.unitsFlag == IMPERIAL_TYPE) || (g_summaryList.cachedEntry.seismicSensorType == SENSOR_ACC))
		{
			if (g_summaryList.cachedEntry.seismicSensorType == SENSOR_ACC)
				strcpy(displayFormat, "mg/s");
			else
				strcpy(displayFormat, "in/s");
		}
		else // Metric
		{
			tempVS *= (float)METRIC;

			strcpy(displayFormat, "mm/s");
		}

		sprintf(buff,"VS %.2f %s", tempVS, displayFormat);
		wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
		WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
	}
	else if (g_displayAlternateResultState == PEAK_DISPLACEMENT_RESULTS)
	{
		memset(&buff[0], 0, sizeof(buff));
		memset(&displayFormat[0], 0, sizeof(displayFormat));

		if (g_summaryList.cachedEntry.channelSummary.r.displacement > g_summaryList.cachedEntry.channelSummary.v.displacement)
		{
			// R is max
			if (g_summaryList.cachedEntry.channelSummary.r.displacement > g_summaryList.cachedEntry.channelSummary.t.displacement)
			{
				tempPeakDisp = (float)g_summaryList.cachedEntry.channelSummary.r.displacement;
			}
			else // T is max
			{
				tempPeakDisp = (float)g_summaryList.cachedEntry.channelSummary.t.displacement;
			}
		}
		else
		{
			// V is max
			if (g_summaryList.cachedEntry.channelSummary.v.displacement > g_summaryList.cachedEntry.channelSummary.t.displacement)
			{
				tempPeakDisp = (float)g_summaryList.cachedEntry.channelSummary.r.displacement;
			}
			else // T is max
			{
				tempPeakDisp = (float)g_summaryList.cachedEntry.channelSummary.t.displacement;
			}
		}

		tempPeakDisp = (float)tempPeakDisp / (float)1000000 / (float)div;

		if ((g_sensorInfo.unitsFlag == IMPERIAL_TYPE) || (g_summaryList.cachedEntry.seismicSensorType == SENSOR_ACC))
		{
			if (g_summaryList.cachedEntry.seismicSensorType == SENSOR_ACC)
				strcpy(displayFormat, "mg");
			else
				strcpy(displayFormat, "in");
		}
		else // Metric
		{
			tempPeakDisp *= (float)METRIC;
			strcpy(displayFormat, "mm");
		}

		sprintf(buff,"PEAK DISP %5.4f %s", tempPeakDisp, displayFormat);
		wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
		WndMpWrtString((uint8*)buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
	}
	else if (g_displayAlternateResultState == PEAK_ACCELERATION_RESULTS)
	{
		if (g_summaryList.cachedEntry.channelSummary.r.acceleration > g_summaryList.cachedEntry.channelSummary.v.acceleration)
		{
			// R is max
			if (g_summaryList.cachedEntry.channelSummary.r.acceleration > g_summaryList.cachedEntry.channelSummary.t.acceleration)
			{
				tempPeakAcc = (float)g_summaryList.cachedEntry.channelSummary.r.acceleration;
			}
			else // T is max
			{
				tempPeakAcc = (float)g_summaryList.cachedEntry.channelSummary.t.acceleration;
			}
		}
		else
		{
			// V is max
			if (g_summaryList.cachedEntry.channelSummary.v.acceleration > g_summaryList.cachedEntry.channelSummary.t.acceleration)
			{
				tempPeakAcc = (float)g_summaryList.cachedEntry.channelSummary.v.acceleration;
			}
			else // T is max
			{
				tempPeakAcc = (float)g_summaryList.cachedEntry.channelSummary.t.acceleration;
			}
		}

		tempPeakAcc = (float)tempPeakAcc / (float)1000 / (float)div;

		if ((g_sensorInfo.unitsFlag == IMPERIAL_TYPE) || (g_summaryList.cachedEntry.seismicSensorType == SENSOR_ACC))
		{
			tempPeakAcc /= (float)ONE_GRAVITY_IN_INCHES;
		}
		else // Metric
		{
			tempPeakAcc *= (float)METRIC;
			tempPeakAcc /= (float)ONE_GRAVITY_IN_MM;
		}

		strcpy(displayFormat, "g");

		sprintf(buff,"PEAK ACC %5.4f %s", tempPeakAcc, displayFormat);
		wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
		WndMpWrtString((uint8*)buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
	}
	else // g_displayAlternateResultState == DEFAULT_RESULTS
	{
		//-------------------------------------------------------------
		// AIR
		memset(&buff[0], 0, sizeof(buff));
		sprintf(buff, "%s", getLangText(AIR_TEXT));
		wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
		WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		//-------------------------------------------------------------
		// Air
		memset(&buff[0], 0, sizeof(buff));

		// Display based on what the units current setting
		if (g_unitConfig.unitsOfAir == MILLIBAR_TYPE)
		{
			if (g_displayAlternateResultState != DEFAULT_ALTERNATE_RESULTS)
			{
				sprintf(buff,"%0.3f mb", HexToMB(g_summaryList.cachedEntry.channelSummary.a.peak, DATA_NORMALIZED, bitAccuracyScale));
			}
			else
			{
				sprintf(buff,"%0.1f dB", HexToDB(g_summaryList.cachedEntry.channelSummary.a.peak, DATA_NORMALIZED, bitAccuracyScale));
			}
		}
		else // Report Air in DB
		{
			if (g_displayAlternateResultState != DEFAULT_ALTERNATE_RESULTS)
			{
				sprintf(buff,"%0.1f dB", HexToDB(g_summaryList.cachedEntry.channelSummary.a.peak, DATA_NORMALIZED, bitAccuracyScale));
			}
			else
			{
				sprintf(buff,"%0.3f mb", HexToMB(g_summaryList.cachedEntry.channelSummary.a.peak, DATA_NORMALIZED, bitAccuracyScale));
			}
		}

		adjust = (uint8)strlen(buff);
		if (adjust > 8)
			adjust = 1;
		else
			adjust = 0;

		// Setup current column, Write string to screen,
		wnd_layout_ptr->curr_col = (uint16)(wnd_layout_ptr->next_col + 4);
		WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		//-------------------------------------------------------------
		// A FREQ
		memset(&buff[0], 0, sizeof(buff));

		if ((g_summaryList.cachedEntry.mode == BARGRAPH_MODE) || ((g_summaryList.cachedEntry.mode == COMBO_MODE) && (g_summaryList.cachedEntry.subMode == BARGRAPH_MODE)))
		{
			sprintf(buff,"%.1f Hz", ((float)g_summaryList.cachedEntry.sampleRate / (float)((g_summaryList.cachedEntry.channelSummary.a.frequency * 2) - 1)));
		}
		else // mode == WAVEFORM_MODE or MANUAL_CAL_MODE
		{
			sprintf(buff,"%.1f Hz", (float)(g_summaryList.cachedEntry.channelSummary.a.frequency)/(float)10.0);
		}

		// Setup current column, Write string to screen,
		wnd_layout_ptr->curr_col = (uint16)(wnd_layout_ptr->next_col + 6 - adjust);
		WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	}

	if (g_summaryList.cachedEntry.mode == MANUAL_CAL_MODE)
	{
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;

		if (calResults == PASSED)
		{
			length = (uint8)sprintf(buff,"** %s **", getLangText(PASSED_TEXT));
		}
		else
		{
			length = (uint8)sprintf(buff,"** %s **", getLangText(FAILED_TEXT));
		}

		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		WndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	}

	if (g_promtForCancelingPrintJobs == TRUE)
	{
		MessageBorder();
		MessageTitle(getLangText(VERIFY_TEXT));
		MessageText(getLangText(CANCEL_ALL_PRINT_JOBS_Q_TEXT));
		MessageChoice(MB_YESNO);

		if (g_monitorModeActiveChoice == MB_SECOND_CHOICE)
			MessageChoiceActiveSwap(MB_YESNO);
	}

	if (g_promtForLeavingMonitorMode == TRUE)
	{
		MessageBorder();
		MessageTitle(getLangText(WARNING_TEXT));
		MessageText(getLangText(DO_YOU_WANT_TO_LEAVE_MONITOR_MODE_Q_TEXT));
		MessageChoice(MB_YESNO);

		if (g_monitorModeActiveChoice == MB_SECOND_CHOICE)
			MessageChoiceActiveSwap(MB_YESNO);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CacheResultsEventInfo(EVT_RECORD* eventRecordToCache)
{
	memcpy(&g_resultsEventCache[g_resultsCacheIndex], eventRecordToCache, sizeof(EVT_RECORD));
	
	if (++g_resultsCacheIndex >= 50) { g_resultsCacheIndex = 0; }
}
