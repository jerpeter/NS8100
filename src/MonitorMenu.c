///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: MonitorMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:54 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/MonitorMenu.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Menu.h"
#include "Uart.h"
#include "Display.h"
#include "Common.h"
#include "Ispi.h"
#include "InitDataBuffers.h"
#include "Msgs430.h"
#include "Summary.h"
#include "SysEvents.h"
#include "Board.h"
#include "PowerManagement.h"
#include "Record.h"
#include "SoftTimer.h"
#include "Keypad.h"
#include "ProcessBargraph.h"
#include "TextTypes.h"
#include "EventProcessing.h"
#include "Analog.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define MONITOR_MN_TABLE_SIZE 8
#define MONITOR_WND_STARTING_COL DEFAULT_COL_THREE 
#define MONITOR_WND_END_COL DEFAULT_END_COL      
#define MONITOR_WND_STARTING_ROW DEFAULT_MENU_ROW_ZERO
#define MONITOR_WND_END_ROW (DEFAULT_MENU_ROW_SEVEN)               
#define MONITOR_MN_TBL_START_LINE 0
#define TOTAL_DOTS 4

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void monitorMn(INPUT_MSG_STRUCT);
void monitorMnDsply(WND_LAYOUT_STRUCT *);
void monitorMnProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
uint16 seisTriggerConvert(float);
uint16 airTriggerConvert(uint32);

/****************************************
*	Function:	monitorMn
*	Purpose:
****************************************/
void monitorMn(INPUT_MSG_STRUCT msg)
{   
	static WND_LAYOUT_STRUCT wnd_layout;
	static MN_LAYOUT_STRUCT mn_layout;

	monitorMnProc(msg, &wnd_layout, &mn_layout);

	if (g_activeMenu == MONITOR_MENU)
	{
		monitorMnDsply(&wnd_layout);
		writeMapToLcd(g_mmap);
	}
}

/****************************************
*	Function:	monitorMnProc
*	Purpose:
****************************************/
uint8 g_showRVTA = NO;
void monitorMnProc(INPUT_MSG_STRUCT msg,
	WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	REC_EVENT_MN_STRUCT temp_g_triggerRecord;
	REC_HELP_MN_STRUCT temp_g_helpRecord;
	//uint8 mbChoice = 0;
	FLASH_USAGE_STRUCT flashStats;
	
	getFlashUsageStats(&flashStats);

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_WITH_DATA_CMD):
			wnd_layout_ptr->start_col =    MONITOR_WND_STARTING_COL;
			wnd_layout_ptr->end_col =      MONITOR_WND_END_COL;
			wnd_layout_ptr->start_row =    MONITOR_WND_STARTING_ROW; 
			wnd_layout_ptr->end_row =      MONITOR_WND_END_ROW;
			mn_layout_ptr->curr_ln =       MONITOR_MN_TBL_START_LINE;
			mn_layout_ptr->top_ln =        MONITOR_MN_TBL_START_LINE;

			byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));
			
			g_monitorOperationMode = (uint8)msg.data[0];

			// Check if flash wrapping is disabled and if there is space left in flash
			if (g_helpRecord.flash_wrapping == NO)
			{
				if (((g_monitorOperationMode == WAVEFORM_MODE) && (flashStats.waveEventsLeft == 0)) ||
					((g_monitorOperationMode == BARGRAPH_MODE) && (flashStats.roomForBargraph == NO)) ||
					((g_monitorOperationMode == MANUAL_CAL_MODE) && (flashStats.manualCalsLeft == 0)))
				{
					// Unable to store any more data in the selected mode
					
					// Clear flag
					g_skipAutoCalInWaveformAfterMidnightCal = NO;

					// Jump back to main menu
					g_activeMenu = MAIN_MENU;
					ACTIVATE_MENU_MSG();
					(*menufunc_ptrs[g_activeMenu]) (mn_msg);
					
					overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT MONITOR.", (5 * SOFT_SECS));
					return;
				}
			}
			
			// Make sure the parameters are up to date based on the trigger setup information
			initSensorParameters(g_factorySetupRecord.sensor_type, (uint8)g_triggerRecord.srec.sensitivity);

			switch(g_monitorOperationMode)
			{
				case WAVEFORM_MODE:
					startMonitoring(g_triggerRecord.trec, START_TRIGGER_CMD, g_triggerRecord.op_mode);
				break;   

				case BARGRAPH_MODE: 
					// fix_ns8100 - Look into the assignment, why does it exist since it should already be set?
					// For bargraph mode these have to be set.
					g_triggerRecord.op_mode = BARGRAPH_MODE;
					
					// Set the default display mode to be the summary interval results
					g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
					
					if(g_helpRecord.vector_sum == DISABLED)
					{
						g_displayAlternateResultState = DEFAULT_RESULTS;
					}
					
					if(g_helpRecord.report_displacement == DISABLED)
					{
						g_displayAlternateResultState = DEFAULT_RESULTS;
					}

#if 0 // ns7100
					// Check if the sample rate is not 1024
					if (g_triggerRecord.trec.sample_rate != 1024)
					{
						g_triggerRecord.trec.sample_rate = 1024;
					}	
#else // ns8100
					// Check if the sample rate is greater than max working sample rate
					if (g_triggerRecord.trec.sample_rate > 4096)
					{
						g_triggerRecord.trec.sample_rate = 1024;
					}	
#endif
					g_aImpulsePeak = g_rImpulsePeak = g_vImpulsePeak = g_tImpulsePeak = 0;
					g_aJobPeak = g_rJobPeak = g_vJobPeak = g_tJobPeak = 0;
					g_aJobFreq = g_rJobFreq = g_vJobFreq = g_tJobFreq = 0;
					g_vsImpulsePeak = g_vsJobPeak = 0;
					
					if ((g_triggerRecord.berec.impulseMenuUpdateSecs < LCD_IMPULSE_TIME_MIN_VALUE) || 
						(g_triggerRecord.berec.impulseMenuUpdateSecs > LCD_IMPULSE_TIME_MAX_VALUE))
					{
						g_triggerRecord.berec.impulseMenuUpdateSecs = LCD_IMPULSE_TIME_DEFAULT_VALUE;
					}

					startMonitoring(g_triggerRecord.trec, START_TRIGGER_CMD, g_triggerRecord.op_mode);
				break;

				case MANUAL_TRIGGER_MODE:
					byteCpy((uint8*)&temp_g_triggerRecord, &g_triggerRecord, sizeof(REC_EVENT_MN_STRUCT));
					temp_g_triggerRecord.trec.seismicTriggerLevel = MANUAL_TRIGGER_CHAR;
					temp_g_triggerRecord.trec.soundTriggerLevel = MANUAL_TRIGGER_CHAR;

					startMonitoring(temp_g_triggerRecord.trec, START_TRIGGER_CMD, g_triggerRecord.op_mode);
				break;

				case COMBO_MODE:
					// Set the default display mode to be the summary interval results
					g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
					
					if(g_helpRecord.vector_sum == DISABLED)
					{
						g_displayAlternateResultState = DEFAULT_RESULTS;
					}
					
					if(g_helpRecord.report_displacement == DISABLED)
					{
						g_displayAlternateResultState = DEFAULT_RESULTS;
					}

#if 0 // ns7100
					// Check if the sample rate is not 1024
					if (g_triggerRecord.trec.sample_rate != 1024)
					{
						g_triggerRecord.trec.sample_rate = 1024;
					}	
#else // ns8100
					// Check if the sample rate is greater than max working sample rate
					if (g_triggerRecord.trec.sample_rate > 4096)
					{
						g_triggerRecord.trec.sample_rate = 1024;
					}	
#endif
					g_aImpulsePeak = g_rImpulsePeak = g_vImpulsePeak = g_tImpulsePeak = 0;
					g_aJobPeak = g_rJobPeak = g_vJobPeak = g_tJobPeak = 0;
					g_aJobFreq = g_rJobFreq = g_vJobFreq = g_tJobFreq = 0;
					g_vsImpulsePeak = g_vsJobPeak = 0;
					
					if ((g_triggerRecord.berec.impulseMenuUpdateSecs < LCD_IMPULSE_TIME_MIN_VALUE) || 
						(g_triggerRecord.berec.impulseMenuUpdateSecs > LCD_IMPULSE_TIME_MAX_VALUE))
					{
						g_triggerRecord.berec.impulseMenuUpdateSecs = LCD_IMPULSE_TIME_DEFAULT_VALUE;
					}

					startMonitoring(g_triggerRecord.trec, START_TRIGGER_CMD, g_triggerRecord.op_mode);
					break;

				default:
					break;    
			} 
		break;

		case (KEYPRESS_MENU_CMD):
			if (g_waitForUser == TRUE)
			{
				switch (msg.data[0])
				{
					case (ENTER_KEY):
					case (ESC_KEY):
						g_waitForUser = FALSE;
					break;
				}
				
				// Done processing
				return;
			}

			if ((g_promtForCancelingPrintJobs == TRUE) || (g_promtForLeavingMonitorMode == TRUE))
			{
				switch (msg.data[0])
				{
					case (ENTER_KEY):
						if (g_promtForCancelingPrintJobs == TRUE)
						{
							// Done handling cancel print jobs, now handle leaving monitor mode
							g_promtForCancelingPrintJobs = FALSE;
							g_promtForLeavingMonitorMode = TRUE;
						}
						else if (g_promtForLeavingMonitorMode == TRUE)
						{
							if (g_monitorModeActiveChoice == MB_FIRST_CHOICE)
							{
								stopMonitoring(g_monitorOperationMode, EVENT_PROCESSING);

								// Restore the auto_print value just in case the user escaped from a printout
								getRecData(&temp_g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
								g_helpRecord.auto_print = temp_g_helpRecord.auto_print;

								g_activeMenu = MAIN_MENU;
								ACTIVATE_MENU_MSG();
								(*menufunc_ptrs[g_activeMenu]) (mn_msg);
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
				
				// Done processing
				return;
			}

			switch (msg.data[0])
			{
				case (ENTER_KEY):
					break;
					
				case (ESC_KEY):
					g_monitorEscapeCheck = YES;
					g_promtForLeavingMonitorMode = TRUE;
				break;     
				
				case (DOWN_ARROW_KEY):
#if 1
						g_showRVTA = YES;
#endif					
					if ((g_monitorOperationMode == BARGRAPH_MODE) || (g_monitorOperationMode == COMBO_MODE))
					{
						// Check if at the Impulse Results screen
						if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
						{
							// Change to the Summary Interval Results Screen
							g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
						}
						// Check if at the Summary Interval Results screen
						else if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
						{
							// Change to the Job Peak Results Screen
							g_displayBargraphResultsMode = JOB_PEAK_RESULTS;
						}
					}
				break;
					
				case (UP_ARROW_KEY):
#if 1
						g_showRVTA = NO;
#endif					
					if ((g_monitorOperationMode == BARGRAPH_MODE) || (g_monitorOperationMode == COMBO_MODE))
					{
						// Check if at the Job Peak Results screen
						if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
						{
							// Change to the Summary Interval Results Screen
							g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
						}
						// Check if at the Summary Interval Results screen
						else if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
						{
							// Change to the Impulse Results Screen
							g_displayBargraphResultsMode = IMPULSE_RESULTS;
							
							// Check if results mode is Peak Displacement
							if (g_displayAlternateResultState == PEAK_DISPLACEMENT_RESULTS)
							{
								// Change it
								g_displayAlternateResultState = DEFAULT_RESULTS;
							}
						}
					}
				break;
					
				case (MINUS_KEY):
					switch (g_displayAlternateResultState)
					{
						case DEFAULT_RESULTS:
							if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
							{
								if(g_helpRecord.vector_sum == ENABLED)				
									g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							}
							else
							{
								if(g_helpRecord.report_displacement == ENABLED)
									g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS;
								else if(g_helpRecord.vector_sum == ENABLED)
									g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							}
						break;
						
						case VECTOR_SUM_RESULTS:
							g_displayAlternateResultState = DEFAULT_RESULTS;
						break;
						
						case PEAK_DISPLACEMENT_RESULTS:
							if(g_helpRecord.vector_sum == ENABLED)
								g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							else
								g_displayAlternateResultState = DEFAULT_RESULTS;
						break;
					}
				break;

				case (PLUS_KEY):
					switch (g_displayAlternateResultState)
					{
						case DEFAULT_RESULTS: 
							if(g_helpRecord.vector_sum == ENABLED)
								g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							else if((g_helpRecord.report_displacement == ENABLED) && (g_displayBargraphResultsMode != IMPULSE_RESULTS))
								g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS;
						break;
							
						case VECTOR_SUM_RESULTS:
							if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
							{
								g_displayAlternateResultState = DEFAULT_RESULTS;
							}
							else
							{
								if(g_helpRecord.report_displacement == ENABLED)
									g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS;
								else
									g_displayAlternateResultState = DEFAULT_RESULTS;
							}
						break;
						
						case PEAK_DISPLACEMENT_RESULTS:
							g_displayAlternateResultState = DEFAULT_RESULTS;
						break;
					}
				break;

				default:
					break;
			}
		break;
        
        case STOP_MONITORING_CMD:
			stopMonitoring(g_monitorOperationMode, EVENT_PROCESSING);

			// Restore the auto_print value just in case the user escaped from a printout
			getRecData(&temp_g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);
			g_helpRecord.auto_print = temp_g_helpRecord.auto_print;

			g_activeMenu = MAIN_MENU;
			ACTIVATE_MENU_MSG();
			(*menufunc_ptrs[g_activeMenu]) (mn_msg);
			
			overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) MONITOR STOPPED.", (5 * SOFT_SECS));
		break;
             
		default:
			break;             
	}  
}

/****************************************
*	Function:	monitorMnDsply
*	Purpose:
****************************************/
void monitorMnDsply(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
	char buff[50];
	uint8 dotBuff[TOTAL_DOTS];
	uint8 length, i = 0;
	static uint8 dotState = 0;
	char displayFormat[22];
	float div, tempR, tempV, tempT, tempA, tempVS, tempPeakDisp, normalize_max_peak;
	float rFreq, vFreq, tFreq, tempFreq;
	uint8 arrowChar;
	uint8 gainFactor = (uint8)((g_triggerRecord.srec.sensitivity == LOW) ? 2 : 4);

	DATE_TIME_STRUCT time;

	wnd_layout_ptr->curr_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col =   wnd_layout_ptr->start_col;

	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	//-----------------------------------------------------------------------
	// PRINT MONITORING
	//-----------------------------------------------------------------------
	byteSet(&buff[0], 0, sizeof(buff));
	byteSet(&dotBuff[0], 0, sizeof(dotBuff));
	
	for (i = 0; i < dotState; i++)
	{
		dotBuff[i] = '.';
	}
	for (; i < (TOTAL_DOTS - 1); i++)
	{
		dotBuff[i] = ' ';
	}

	if (++dotState >= TOTAL_DOTS)
		dotState = 0;

	if (g_monitorOperationMode == WAVEFORM_MODE)
	{
		if (g_isTriggered)
		{
			length = (uint8)sprintf((char*)buff, "%s%s (W)", getLangText(PROCESSING_TEXT), dotBuff);
		}
		else
		{
			length = (uint8)sprintf((char*)buff, "%s%s (W)", getLangText(MONITORING_TEXT), dotBuff);
		}
	}
	else if (g_monitorOperationMode == BARGRAPH_MODE)
	{
		length = (uint8)sprintf((char*)buff, "%s%s (B)", getLangText(MONITORING_TEXT), dotBuff);
	}
	else if (g_monitorOperationMode == COMBO_MODE)
	{
		if (g_isTriggered)
		{
			length = (uint8)sprintf((char*)buff, "%s%s (C)", getLangText(PROCESSING_TEXT), dotBuff);
		}
		else
		{
			length = (uint8)sprintf((char*)buff, "%s%s (C)", getLangText(MONITORING_TEXT), dotBuff);
		}

	}

	// Setup current column to center text
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	// Write string to screen
	wndMpWrtString((uint8*)&buff[0], wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	if((g_monitorOperationMode == BARGRAPH_MODE) || (g_monitorOperationMode == COMBO_MODE))
	{
		if(g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
			arrowChar = BOTH_ARROWS_CHAR;
		else if(g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
			arrowChar = UP_ARROW_CHAR;
		else // g_displayBargraphResultsMode == IMPULSE_RESULTS
			arrowChar = DOWN_ARROW_CHAR;
				
		sprintf(buff, "%c", arrowChar);                     
	    wnd_layout_ptr->curr_col = 120;
	    wndMpWrtString((uint8*)&buff[0], wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	}

	// Advance to next row
	wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

	// Check mode
	if (g_monitorOperationMode == WAVEFORM_MODE)
	{
		//-----------------------------------------------------------------------
		// Skip Line
		//-----------------------------------------------------------------------
		wndMpWrtString((uint8*)" ", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Date
		//-----------------------------------------------------------------------
		byteSet(&buff[0], 0, sizeof(buff));
		time = getCurrentTime();
		convertTimeStampToString(buff, &time, REC_DATE_TIME_MONITOR);          
		length = (uint8)strlen(buff);

		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Time
		//-----------------------------------------------------------------------
		byteSet(&buff[0], 0, sizeof(buff));
		length = (uint8)sprintf(buff,"%02d:%02d:%02d",time.hour, time.min, time.sec);                    

		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Skip Line
		//-----------------------------------------------------------------------
		wndMpWrtString((uint8*)" ", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Battery Voltage
		//-----------------------------------------------------------------------
		byteSet(&buff[0], 0, sizeof(buff));	
		length = (uint8)sprintf(buff,"%s %.2f", getLangText(BATTERY_TEXT), getExternalVoltageLevelAveraged(BATTERY_VOLTAGE));

		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Skip Line
		//-----------------------------------------------------------------------
		wndMpWrtString((uint8*)" ", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

#if 1 // Show hidden RTVA Values
		if (g_showRVTA == YES)
		{
			//-----------------------------------------------------------------------
			// Show RTVA
			//-----------------------------------------------------------------------
			byteSet(&buff[0], 0, sizeof(buff));	
			length = (uint8)sprintf(buff," x%3x x%3x x%3x x%3x", 
				((((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->r) & 0x0FFF),
				((((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->v) & 0x0FFF),
				((((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->t) & 0x0FFF),
				((((SAMPLE_DATA_STRUCT*)g_tailOfQuarterSecBuff)->a) & 0x0FFF));

			wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

			wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
#endif
	}
	else if ((g_monitorOperationMode == BARGRAPH_MODE) || (g_monitorOperationMode == COMBO_MODE))
	{
		//-----------------------------------------------------------------------
		// Date and Time
		//-----------------------------------------------------------------------
		byteSet(&buff[0], 0, sizeof(buff));
		time = getCurrentTime();

		convertTimeStampToString(buff, &time, REC_DATE_TIME_TYPE);          
		length = (uint8)strlen(buff);
		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Skip Line
		//-----------------------------------------------------------------------
		wndMpWrtString((uint8*)" ", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Print Result Type Header
		//-----------------------------------------------------------------------
		byteSet(&buff[0], 0, sizeof(buff));	

		if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
		{
			length = (uint8)sprintf(buff, "%s", getLangText(SUMMARY_INTERVAL_TEXT));
		}
		else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
		{
			length = (uint8)sprintf(buff, "%s", getLangText(JOB_PEAK_RESULTS_TEXT));
		}
		else // g_displayBargraphResultsMode == IMPULSE_RESULTS
		{
			length = (uint8)sprintf(buff, "%s", getLangText(IMPULSE_RESULTS_TEXT));
		}
				
		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		if (g_triggerRecord.berec.barChannel != BAR_AIR_CHANNEL)
		{
			//-----------------------------------------------------------------------
			// Max Results Header
			//-----------------------------------------------------------------------
			byteSet(&buff[0], 0, sizeof(buff));	
			length = (uint8)sprintf(buff, "PEAK | R  | T  | V  |");

			wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

			wndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

			//-----------------------------------------------------------------------
			// Peak Results
			//-----------------------------------------------------------------------
			byteSet(&buff[0], 0, sizeof(buff));
			byteSet(&displayFormat[0], 0, sizeof(displayFormat));

			div = (float)(g_sensorInfoPtr->ADCResolution * g_sensorInfoPtr->sensorAccuracy * gainFactor) / 
					(float)(g_factorySetupRecord.sensor_type);

			if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
			{
				if (g_triggerRecord.op_mode == BARGRAPH_MODE)
				{
					tempR = ((float)g_bargraphSumIntervalWritePtr->r.peak / (float)div);
					tempT = ((float)g_bargraphSumIntervalWritePtr->t.peak / (float)div);
					tempV = ((float)g_bargraphSumIntervalWritePtr->v.peak / (float)div);
				}
				else if (g_triggerRecord.op_mode == COMBO_MODE)
				{
					tempR = ((float)g_comboSumIntervalWritePtr->r.peak / (float)div);
					tempT = ((float)g_comboSumIntervalWritePtr->t.peak / (float)div);
					tempV = ((float)g_comboSumIntervalWritePtr->v.peak / (float)div);
				}
			}
			else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
			{
				tempR = ((float)g_rJobPeak / (float)div);
				tempT = ((float)g_tJobPeak / (float)div);
				tempV = ((float)g_vJobPeak / (float)div);
			}
			else // g_displayBargraphResultsMode == IMPULSE_RESULTS
			{
				tempR = ((float)g_rImpulsePeak / (float)div);
				tempT = ((float)g_tImpulsePeak / (float)div);
				tempV = ((float)g_vImpulsePeak / (float)div);
			}

			if ((g_sensorInfoPtr->unitsFlag == IMPERIAL_TYPE) || (g_factorySetupRecord.sensor_type == SENSOR_ACC))
			{
				if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
					strcpy(buff, "mg/s |");
				else
					strcpy(buff, "in/s |");
			}
			else // g_sensorInfoPtr->unitsFlag == METRIC_TYPE
			{
				tempR *= (float)METRIC;
				tempT *= (float)METRIC;
				tempV *= (float)METRIC;

				strcpy(buff, "mm/s |");
			}		

			// Make sure formatting is correct
			if (tempR > 100)
				sprintf(displayFormat, "%4d|", (int)tempR);
			else
				sprintf(displayFormat, "%4.1f|", tempR);
			strcat(buff, displayFormat);
		
			if (tempT > 100)
				sprintf(displayFormat, "%4d|", (int)tempT);
			else
				sprintf(displayFormat, "%4.1f|", tempT);
			strcat(buff, displayFormat);

			if (tempV > 100)
				sprintf(displayFormat, "%4d|", (int)tempV);
			else
				sprintf(displayFormat, "%4.1f|", tempV);
			strcat(buff, displayFormat);

			length = 21;
			wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

			wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
			wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

			//-----------------------------------------------------------------------
			// Peak Freq Results
			//-----------------------------------------------------------------------
			byteSet(&buff[0], 0, sizeof(buff));
			byteSet(&displayFormat[0], 0, sizeof(displayFormat));
			tempR = tempV = tempT = (float)0.0;

			if (g_displayBargraphResultsMode != IMPULSE_RESULTS)
			{
				if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
				{
					if (g_triggerRecord.op_mode == BARGRAPH_MODE)
					{
						if (g_bargraphSumIntervalWritePtr->r.frequency > 0)
						{
							tempR = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1)));
						}
						if (g_bargraphSumIntervalWritePtr->v.frequency > 0)
						{
							tempV = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1)));
						}
						if (g_bargraphSumIntervalWritePtr->t.frequency > 0)
						{
							tempT = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1)));
						}
					}
					else if (g_triggerRecord.op_mode == COMBO_MODE)
					{
						if (g_comboSumIntervalWritePtr->r.frequency > 0)
						{
							tempR = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_comboSumIntervalWritePtr->r.frequency * 2) - 1)));
						}
						if (g_comboSumIntervalWritePtr->v.frequency > 0)
						{
							tempV = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_comboSumIntervalWritePtr->v.frequency * 2) - 1)));
						}
						if (g_comboSumIntervalWritePtr->t.frequency > 0)
						{
							tempT = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_comboSumIntervalWritePtr->t.frequency * 2) - 1)));
						}
					}
				}
				else // g_displayBargraphResultsMode == JOB_PEAK_RESULTS
				{
					if (g_rJobFreq > 0)
					{
						tempR = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
								((float)((g_rJobFreq * 2) - 1)));
					}
					if (g_vJobFreq > 0)
					{
						tempV = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
								((float)((g_vJobFreq * 2) - 1)));
					}
					if (g_tJobFreq > 0)
					{
						tempT = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
								((float)((g_tJobFreq * 2) - 1)));
					}
				}

				strcpy(buff, "F(Hz)|");

				// Make sure formatting is correct
				if (tempR > 100)
					sprintf(displayFormat, "%4d|", (int)tempR);
				else
					sprintf(displayFormat, "%4.1f|", tempR);
				strcat(buff, displayFormat);
			
				if (tempT > 100)
					sprintf(displayFormat, "%4d|", (int)tempT);
				else
					sprintf(displayFormat, "%4.1f|", tempT);
				strcat(buff, displayFormat);

				if (tempV > 100)
					sprintf(displayFormat, "%4d|", (int)tempV);
				else
					sprintf(displayFormat, "%4.1f|", tempV);
				strcat(buff, displayFormat);

				length = 21;
				wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

				wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
				wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
			}
		}

		//-----------------------------------------------------------------------
		// Max Air/Max Vector Sum Results
		//-----------------------------------------------------------------------
		byteSet(&buff[0], 0, sizeof(buff));
		byteSet(&displayFormat[0], 0, sizeof(displayFormat));
		tempA = (float)0.0;

		// Check if displaying the vector sum and if the bar channel isn't just air
		if ((g_displayAlternateResultState == VECTOR_SUM_RESULTS) && (g_triggerRecord.berec.barChannel != BAR_AIR_CHANNEL))
		{
			if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
				tempVS = sqrtf((float)g_vsImpulsePeak) / (float)div;
			else if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
			{
				if (g_triggerRecord.op_mode == BARGRAPH_MODE)
					tempVS = sqrtf((float)g_bargraphSumIntervalWritePtr->vectorSumPeak) / (float)div;
				else if (g_triggerRecord.op_mode == COMBO_MODE)
					tempVS = sqrtf((float)g_comboSumIntervalWritePtr->vectorSumPeak) / (float)div;
			}			
			else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
				tempVS = sqrtf((float)g_vsJobPeak) / (float)div;

			if ((g_sensorInfoPtr->unitsFlag == IMPERIAL_TYPE) || (g_factorySetupRecord.sensor_type == SENSOR_ACC))
			{
				if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
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
		}
		else if ((g_displayAlternateResultState == PEAK_DISPLACEMENT_RESULTS) && (g_triggerRecord.berec.barChannel != BAR_AIR_CHANNEL))
		{
			if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
			{
				if (g_triggerRecord.op_mode == BARGRAPH_MODE)
				{
					tempR = g_bargraphSumIntervalWritePtr->r.peak;
					tempV = g_bargraphSumIntervalWritePtr->v.peak;
					tempT = g_bargraphSumIntervalWritePtr->t.peak;

					rFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
							((float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1)));
					vFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
							((float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1)));
					tFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
							((float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1)));
				}
				else if (g_triggerRecord.op_mode == COMBO_MODE)
				{
					tempR = g_comboSumIntervalWritePtr->r.peak;
					tempV = g_comboSumIntervalWritePtr->v.peak;
					tempT = g_comboSumIntervalWritePtr->t.peak;

					rFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
							((float)((g_comboSumIntervalWritePtr->r.frequency * 2) - 1)));
					vFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
							((float)((g_comboSumIntervalWritePtr->v.frequency * 2) - 1)));
					tFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
							((float)((g_comboSumIntervalWritePtr->t.frequency * 2) - 1)));
				}
			}
			else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
			{
				tempR = g_rJobPeak;
				tempV = g_vJobPeak;
				tempT = g_tJobPeak;

				rFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
						((float)((g_rJobFreq * 2) - 1)));
				vFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
						((float)((g_vJobFreq * 2) - 1)));
				tFreq = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
						((float)((g_tJobFreq * 2) - 1)));
			}

			if (tempR > tempV)
			{
				if (tempR > tempT)
				{
					// R is max
				    normalize_max_peak = (float)tempR / (float)div;
				    tempFreq = rFreq;
				}
				else
				{
					// T is max
				    normalize_max_peak = (float)tempT / (float)div;
				    tempFreq = tFreq;
				}
			}
			else
			{
				if (tempV > tempT)
				{
					// V is max
				    normalize_max_peak = (float)tempV / (float)div;
				    tempFreq = vFreq;
				}
				else
				{
					// T is max
				    normalize_max_peak = (float)tempT / (float)div;
				    tempFreq = tFreq;
				}
			}

			tempPeakDisp = (float)normalize_max_peak / ((float)2 * (float)PI * (float)tempFreq);

			if ((g_sensorInfoPtr->unitsFlag == IMPERIAL_TYPE) || (g_factorySetupRecord.sensor_type == SENSOR_ACC))
			{
				if (g_factorySetupRecord.sensor_type == SENSOR_ACC)
					strcpy(displayFormat, "mg");
				else
					strcpy(displayFormat, "in");
			}
			else // Metric
			{
				tempPeakDisp *= (float)METRIC;

				strcpy(displayFormat, "mm");
			}

			sprintf(buff,"PEAK DISP %6f %s", tempPeakDisp, displayFormat);
		}
		else // g_displayAlternateResultState == DEFAULT_RESULTS || g_triggerRecord.berec.barChannel == BAR_AIR_CHANNEL
		{
			// Check if the bar channel to display isn't just seismic
			if (g_triggerRecord.berec.barChannel != BAR_SEISMIC_CHANNEL)
			{
				if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
				{
					sprintf(buff, "%s %4.1f dB", getLangText(PEAK_AIR_TEXT), hexToDB(g_aImpulsePeak, DATA_NORMALIZED));
				}
				else // (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS) || (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
				{
					if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
					{
						if (g_triggerRecord.op_mode == BARGRAPH_MODE)
						{
							if (g_bargraphSumIntervalWritePtr->a.frequency > 0)
							{
								tempA = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
										((float)((g_bargraphSumIntervalWritePtr->a.frequency * 2) - 1)));
							}

							sprintf(buff, "AIR %4.1f dB ", hexToDB(g_bargraphSumIntervalWritePtr->a.peak, DATA_NORMALIZED));
						}
						else if (g_triggerRecord.op_mode == COMBO_MODE)
						{
							if (g_comboSumIntervalWritePtr->a.frequency > 0)
							{
								tempA = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
										((float)((g_comboSumIntervalWritePtr->a.frequency * 2) - 1)));
							}

							sprintf(buff, "AIR %4.1f dB ", hexToDB(g_comboSumIntervalWritePtr->a.peak, DATA_NORMALIZED));
						}
					}
					else // g_displayBargraphResultsMode == JOB_PEAK_RESULTS
					{
						if (g_aJobFreq > 0)
						{
							tempA = (float)((float)g_pendingBargraphRecord.summary.parameters.sampleRate / 
									((float)((g_aJobFreq * 2) - 1)));
						}

						sprintf(buff, "AIR %4.1f dB ", hexToDB(g_aJobPeak, DATA_NORMALIZED));
					}

					if (tempA > 100)
						sprintf(displayFormat, "%3d(Hz)", (int)tempA);
					else
						sprintf(displayFormat, "%3.1f(Hz)", tempA);
					strcat(buff, displayFormat);
				}
			}
		}

		length = 21;
		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]), wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
		{
			byteSet(&buff[0], 0, sizeof(buff));
		
			length = (uint8)sprintf(buff, "(%d %s)", g_triggerRecord.berec.impulseMenuUpdateSecs, (g_triggerRecord.berec.impulseMenuUpdateSecs == 1) ?
									getLangText(SECOND_TEXT) : getLangText(SECONDS_TEXT));

			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

			// Always display the refresh time on the last line
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}

		// Keep impulse values refreshed based on LCD updates (every second)
		g_impulseMenuCount++;
	}
	
	if (g_promtForCancelingPrintJobs == TRUE)
	{
		messageBorder();
		messageTitle(getLangText(VERIFY_TEXT));
		messageText(getLangText(CANCEL_ALL_PRINT_JOBS_Q_TEXT));
		messageChoice(MB_YESNO);

		if (g_monitorModeActiveChoice == MB_SECOND_CHOICE)
			messageChoiceActiveSwap(MB_YESNO);
	}
	
	if (g_promtForLeavingMonitorMode == TRUE)
	{
		messageBorder();
		messageTitle(getLangText(WARNING_TEXT));
		messageText(getLangText(DO_YOU_WANT_TO_LEAVE_MONITOR_MODE_Q_TEXT));
		messageChoice(MB_YESNO);

		if (g_monitorModeActiveChoice == MB_SECOND_CHOICE)
			messageChoiceActiveSwap(MB_YESNO);
	}
}
