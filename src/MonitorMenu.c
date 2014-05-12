///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: MonitorMenu.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/MonitorMenu.c,v $
///	$Revision: 1.1 $
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
#include "Rec.h"
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
// 430 structures and variables.
extern volatile ISPI_STATE_E _ISPI_State;
extern MSGS430_UNION msgs430;							// 430 Message structure.
extern uint8 g_sampleProcessing;								// State of the 430 HW
extern uint16* tailOfPreTrigBuff;						// End of the pre-Trigger buffer.
extern uint32 isTriggered;
extern uint32 processingCal;
extern uint16 gCalTestExpected;
extern uint8 g_doneTakingEvents;

// System Parameter information.
extern SYS_EVENT_STRUCT SysEvents_flags;				// System event flags.
extern REC_HELP_MN_STRUCT help_rec;						
extern REC_EVENT_MN_STRUCT trig_rec;
extern FACTORY_SETUP_STRUCT factory_setup_rec;
extern SENSOR_PARAMETERS_STRUCT* gp_SensorInfo;		// Sensor Information.
extern MN_EVENT_STRUCT mn_event_flags;

// Event data structures.
extern EVT_RECORD g_RamEventRecord;						// Event record in Ram, for the current event.
extern CALCULATED_DATA_STRUCT* g_bargraphSumIntervalWritePtr;	// To display bargraph summary information on the screen.

// Printer structure information and printer flags.
extern uint8 mmap[8][128];								// Print buffer.

// Menu data structures.
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern int32 active_menu;
extern USER_MENU_STRUCT modeMenu[];

// Bargraph Impulse references
extern uint16 aImpulsePeak;
extern uint16 rImpulsePeak;
extern uint16 vImpulsePeak;
extern uint16 tImpulsePeak;
extern uint32 vsImpulsePeak;
extern uint16 impulseMenuCount;
extern uint16 aJobPeak;
extern uint16 aJobFreq;
extern uint16 rJobPeak;
extern uint16 rJobFreq;
extern uint16 vJobPeak;
extern uint16 vJobFreq;
extern uint16 tJobPeak;
extern uint16 tJobFreq;
extern uint32 vsJobPeak;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
uint8 g_monitorOperationMode;
uint32 gTotalSamples;
uint16 manual_cal_flag = FALSE;
uint16 manualCalSampleCount = 0;
uint8 g_waitForUser = FALSE;
uint8 g_promtForLeavingMonitorMode = FALSE;
uint8 g_promtForCancelingPrintJobs = FALSE;
uint8 g_monitorModeActiveChoice = MB_FIRST_CHOICE;
uint8 g_monitorEscapeCheck = YES;
uint8 g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
uint8 g_displayAlternateResultState = DEFAULT_RESULTS;
uint8 g_bargraphForcedCal = NO;
uint8 g_skipAutoCalInWaveformAfterMidnightCal = NO;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void monitorMn (INPUT_MSG_STRUCT);
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

	if (active_menu == MONITOR_MENU)
	{
		monitorMnDsply(&wnd_layout);
		writeMapToLcd(mmap);
	}
}

/****************************************
*	Function:	monitorMnProc
*	Purpose:
****************************************/
void monitorMnProc(INPUT_MSG_STRUCT msg,
	WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	REC_EVENT_MN_STRUCT temp_trig_rec;
	REC_HELP_MN_STRUCT temp_help_rec;
	//uint8 mbChoice = 0;
	FLASH_USAGE_STRUCT flashStats = getFlashUsageStats();

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_WITH_DATA_CMD):
			wnd_layout_ptr->start_col =    MONITOR_WND_STARTING_COL;
			wnd_layout_ptr->end_col =      MONITOR_WND_END_COL;
			wnd_layout_ptr->start_row =    MONITOR_WND_STARTING_ROW; 
			wnd_layout_ptr->end_row =      MONITOR_WND_END_ROW;
			mn_layout_ptr->curr_ln =       MONITOR_MN_TBL_START_LINE;
			mn_layout_ptr->top_ln =        MONITOR_MN_TBL_START_LINE;

			byteSet(&(mmap[0][0]), 0, sizeof(mmap));
			
			g_monitorOperationMode = (uint8)msg.data[0];

			// Check if flash wrapping is disabled and if there is space left in flash
			if (help_rec.flash_wrapping == NO)
			{
				if (((g_monitorOperationMode == WAVEFORM_MODE) && (flashStats.waveEventsLeft == 0)) ||
					((g_monitorOperationMode == BARGRAPH_MODE) && (flashStats.roomForBargraph == NO)) ||
					((g_monitorOperationMode == MANUAL_CAL_MODE) && (flashStats.manualCalsLeft == 0)))
				{
					// Unable to store any more data in the selected mode
					
					// Clear flag
					g_skipAutoCalInWaveformAfterMidnightCal = NO;

					// Jump back to main menu
					active_menu = MAIN_MENU;
					ACTIVATE_MENU_MSG();
					(*menufunc_ptrs[active_menu]) (mn_msg);
					
					overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT MONITOR.", (5 * SOFT_SECS));
					return;
				}
			}
			
			// Make sure the parameters are up to date based on the trigger setup information
			initSensorParameters(factory_setup_rec.sensor_type, (uint8)trig_rec.srec.sensitivity);

			switch(g_monitorOperationMode)
			{
				case WAVEFORM_MODE:
					mnStartTrigger(trig_rec.trec, START_TRIGGER_CMD, trig_rec.op_mode);
				break;   

				case BARGRAPH_MODE: 
					// For bargraph mode these have to be set.
					trig_rec.op_mode = BARGRAPH_MODE;
					
					// Set the default display mode to be the summary interval results
					g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
					
					if(help_rec.vector_sum == DISABLED)
					{
						g_displayAlternateResultState = DEFAULT_RESULTS;
					}
					
					if(help_rec.report_displacement == DISABLED)
					{
						g_displayAlternateResultState = DEFAULT_RESULTS;
					}

					// Check if the sample rate is not 1024
					if (trig_rec.trec.sample_rate != 1024)
					{
						trig_rec.trec.sample_rate = 1024;
					}	

					aImpulsePeak = rImpulsePeak = vImpulsePeak = tImpulsePeak = 0;
					aJobPeak = rJobPeak = vJobPeak = tJobPeak = 0;
					aJobFreq = rJobFreq = vJobFreq = tJobFreq = 0;
					vsImpulsePeak = vsJobPeak = 0;
					
					if ((trig_rec.berec.impulseMenuUpdateSecs < LCD_IMPULSE_TIME_MIN_VALUE) || 
						(trig_rec.berec.impulseMenuUpdateSecs > LCD_IMPULSE_TIME_MAX_VALUE))
					{
						trig_rec.berec.impulseMenuUpdateSecs = LCD_IMPULSE_TIME_DEFAULT_VALUE;
					}

					mnStartTrigger(trig_rec.trec, START_TRIGGER_CMD, trig_rec.op_mode);
				break;

				case MANUAL_TRIGGER_MODE:
					byteCpy((uint8*)&temp_trig_rec, &trig_rec, sizeof(REC_EVENT_MN_STRUCT));
					temp_trig_rec.trec.seismicTriggerLevel = MANUAL_TRIGGER_CHAR;
					temp_trig_rec.trec.soundTriggerLevel = MANUAL_TRIGGER_CHAR;

					mnStartTrigger(temp_trig_rec.trec, START_TRIGGER_CMD, trig_rec.op_mode);
				break;

				case COMBO_MODE:
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
								getRecData(&temp_help_rec, 0, REC_HELP_USER_MENU_TYPE);
								help_rec.auto_print = temp_help_rec.auto_print;

								active_menu = MAIN_MENU;
								ACTIVATE_MENU_MSG();
								(*menufunc_ptrs[active_menu]) (mn_msg);
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
					if (g_monitorOperationMode == BARGRAPH_MODE)
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
					if (g_monitorOperationMode == BARGRAPH_MODE)
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
								if(help_rec.vector_sum == ENABLED)				
									g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							}
							else
							{
								if(help_rec.report_displacement == ENABLED)
									g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS;
								else if(help_rec.vector_sum == ENABLED)
									g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							}
						break;
						
						case VECTOR_SUM_RESULTS:
							g_displayAlternateResultState = DEFAULT_RESULTS;
						break;
						
						case PEAK_DISPLACEMENT_RESULTS:
							if(help_rec.vector_sum == ENABLED)
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
							if(help_rec.vector_sum == ENABLED)
								g_displayAlternateResultState = VECTOR_SUM_RESULTS;
							else if((help_rec.report_displacement == ENABLED) && (g_displayBargraphResultsMode != IMPULSE_RESULTS))
								g_displayAlternateResultState = PEAK_DISPLACEMENT_RESULTS;
						break;
							
						case VECTOR_SUM_RESULTS:
							if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
							{
								g_displayAlternateResultState = DEFAULT_RESULTS;
							}
							else
							{
								if(help_rec.report_displacement == ENABLED)
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
			getRecData(&temp_help_rec, 0, REC_HELP_USER_MENU_TYPE);
			help_rec.auto_print = temp_help_rec.auto_print;

			active_menu = MAIN_MENU;
			ACTIVATE_MENU_MSG();
			(*menufunc_ptrs[active_menu]) (mn_msg);
			
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
	uint8 gainFactor = (uint8)((trig_rec.srec.sensitivity == LOW) ? 2 : 4);

	DATE_TIME_STRUCT time;

	wnd_layout_ptr->curr_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col =   wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row =   wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col =   wnd_layout_ptr->start_col;

	byteSet(&(mmap[0][0]), 0, sizeof(mmap));

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
		if (isTriggered)
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

	// Setup current column to center text
	wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
	// Write string to screen
	wndMpWrtString((uint8*)&buff[0], wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	if(g_monitorOperationMode == BARGRAPH_MODE)
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
		length = (uint8)sprintf(buff,"%s %.2f", getLangText(BATTERY_TEXT), convertedBatteryLevel(BATTERY_VOLTAGE));

		wnd_layout_ptr->curr_col =(uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

		wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;

		//-----------------------------------------------------------------------
		// Skip Line
		//-----------------------------------------------------------------------
		wndMpWrtString((uint8*)" ", wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		wnd_layout_ptr->curr_row = wnd_layout_ptr->next_row;
	}
	else if (g_monitorOperationMode == BARGRAPH_MODE)
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

		if (trig_rec.berec.barChannel != BAR_AIR_CHANNEL)
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

			div = (float)(gp_SensorInfo->ADCResolution * gp_SensorInfo->sensorAccuracy * gainFactor) / 
					(float)(factory_setup_rec.sensor_type);

			if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
			{
				tempR = ((float)g_bargraphSumIntervalWritePtr->r.peak / (float)div);
				tempT = ((float)g_bargraphSumIntervalWritePtr->t.peak / (float)div);
				tempV = ((float)g_bargraphSumIntervalWritePtr->v.peak / (float)div);
			}
			else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
			{
				tempR = ((float)rJobPeak / (float)div);
				tempT = ((float)tJobPeak / (float)div);
				tempV = ((float)vJobPeak / (float)div);
			}
			else // g_displayBargraphResultsMode == IMPULSE_RESULTS
			{
				tempR = ((float)rImpulsePeak / (float)div);
				tempT = ((float)tImpulsePeak / (float)div);
				tempV = ((float)vImpulsePeak / (float)div);
			}

			if ((gp_SensorInfo->unitsFlag == IMPERIAL_TYPE) || (factory_setup_rec.sensor_type == SENSOR_ACC))
			{
				if (factory_setup_rec.sensor_type == SENSOR_ACC)
					strcpy(buff, "mg/s |");
				else
					strcpy(buff, "in/s |");
			}
			else // gp_SensorInfo->unitsFlag == METRIC_TYPE
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
					if (g_bargraphSumIntervalWritePtr->r.frequency > 0)
					{
						tempR = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
								((float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1)));
					}
					if (g_bargraphSumIntervalWritePtr->v.frequency > 0)
					{
						tempV = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
								((float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1)));
					}
					if (g_bargraphSumIntervalWritePtr->t.frequency > 0)
					{
						tempT = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
								((float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1)));
					}
				}
				else // g_displayBargraphResultsMode == JOB_PEAK_RESULTS
				{
					if (rJobFreq > 0)
					{
						tempR = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
								((float)((rJobFreq * 2) - 1)));
					}
					if (vJobFreq > 0)
					{
						tempV = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
								((float)((vJobFreq * 2) - 1)));
					}
					if (tJobFreq > 0)
					{
						tempT = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
								((float)((tJobFreq * 2) - 1)));
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
		if ((g_displayAlternateResultState == VECTOR_SUM_RESULTS) && (trig_rec.berec.barChannel != BAR_AIR_CHANNEL))
		{
			if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
				tempVS = sqrtf((float)vsImpulsePeak) / (float)div;
			else if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
				tempVS = sqrtf((float)g_bargraphSumIntervalWritePtr->vectorSumPeak) / (float)div;
			else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
				tempVS = sqrtf((float)vsJobPeak) / (float)div;

			if ((gp_SensorInfo->unitsFlag == IMPERIAL_TYPE) || (factory_setup_rec.sensor_type == SENSOR_ACC))
			{
				if (factory_setup_rec.sensor_type == SENSOR_ACC)
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
		else if ((g_displayAlternateResultState == PEAK_DISPLACEMENT_RESULTS) && (trig_rec.berec.barChannel != BAR_AIR_CHANNEL))
		{
			if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
			{
				tempR = g_bargraphSumIntervalWritePtr->r.peak;
				tempV = g_bargraphSumIntervalWritePtr->v.peak;
				tempT = g_bargraphSumIntervalWritePtr->t.peak;

				rFreq = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
						((float)((g_bargraphSumIntervalWritePtr->r.frequency * 2) - 1)));
				vFreq = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
						((float)((g_bargraphSumIntervalWritePtr->v.frequency * 2) - 1)));
				tFreq = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
						((float)((g_bargraphSumIntervalWritePtr->t.frequency * 2) - 1)));
			}
			else if (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
			{
				tempR = rJobPeak;
				tempV = vJobPeak;
				tempT = tJobPeak;

				rFreq = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
						((float)((rJobFreq * 2) - 1)));
				vFreq = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
						((float)((vJobFreq * 2) - 1)));
				tFreq = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
						((float)((tJobFreq * 2) - 1)));
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

			if ((gp_SensorInfo->unitsFlag == IMPERIAL_TYPE) || (factory_setup_rec.sensor_type == SENSOR_ACC))
			{
				if (factory_setup_rec.sensor_type == SENSOR_ACC)
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
		else // g_displayAlternateResultState == DEFAULT_RESULTS || trig_rec.berec.barChannel == BAR_AIR_CHANNEL
		{
			// Check if the bar channel to display isn't just seismic
			if (trig_rec.berec.barChannel != BAR_SEISMIC_CHANNEL)
			{
				if (g_displayBargraphResultsMode == IMPULSE_RESULTS)
				{
					sprintf(buff, "%s %4.1f dB", getLangText(PEAK_AIR_TEXT), hexToDB(aImpulsePeak, DATA_NORMALIZED));
				}
				else // (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS) || (g_displayBargraphResultsMode == JOB_PEAK_RESULTS)
				{
					if (g_displayBargraphResultsMode == SUMMARY_INTERVAL_RESULTS)
					{
						if (g_bargraphSumIntervalWritePtr->a.frequency > 0)
						{
							tempA = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
									((float)((g_bargraphSumIntervalWritePtr->a.frequency * 2) - 1)));
						}

						sprintf(buff, "AIR %4.1f dB ", hexToDB(g_bargraphSumIntervalWritePtr->a.peak, DATA_NORMALIZED));
					}
					else // g_displayBargraphResultsMode == JOB_PEAK_RESULTS
					{
						if (aJobFreq > 0)
						{
							tempA = (float)((float)g_RamEventRecord.summary.parameters.sampleRate / 
									((float)((aJobFreq * 2) - 1)));
						}

						sprintf(buff, "AIR %4.1f dB ", hexToDB(aJobPeak, DATA_NORMALIZED));
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
		
			length = (uint8)sprintf(buff, "(%d %s)", trig_rec.berec.impulseMenuUpdateSecs, (trig_rec.berec.impulseMenuUpdateSecs == 1) ?
									getLangText(SECOND_TEXT) : getLangText(SECONDS_TEXT));

			wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));

			// Always display the refresh time on the last line
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			wndMpWrtString((uint8*)(&buff[0]),wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}

		// Keep impulse values refreshed based on LCD updates (every second)
		impulseMenuCount++;
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

/****************************************
*	Function:  mnStartTrigger()
*	Purpose:
****************************************/
void mnStartTrigger(TRIGGER_EVENT_DATA_STRUCT trig_mn, uint8 cmd_id, uint8 op_mode)
{ 
	UNUSED(cmd_id);

	// Check if any events are still stored in buffers and need to be stored into flash
	if (getSystemEventState(TRIGGER_EVENT))
	{
		while (getSystemEventState(TRIGGER_EVENT))
		{
			MoveWaveformEventToFlash();
		}
	}

	// Display a message to be patient while the software setups up the 430
	overlayMessage(getLangText(STATUS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT), 0);

	if (getPowerControlState(POWER_SHUTDOWN_ENABLE) == OFF)
	{
		debug("Start Trigger: Disabling Power Off key\n");

		// Disable the Power Off key
		powerControl(POWER_SHUTDOWN_ENABLE, ON);
	}

	// Assign a one second menu update timer
	assignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, menuUpdateTimerCallBack);

	// Where ever num_sensor_channels gets set gTotalSamples needs to be 
	// set also it is used to know the size of data burst expected from 
	// msp430 when collecting sample data.  

	// TODO: This should be moved to where the trigger data is setup.	  
	// g_numOfSensorChannels = 4; // hardcoded for test   
	// This creates a buffer size in an even integral of the number of sensors.
	gTotalSamples = (uint32)((SAMPLE_BUF_SIZE / gp_SensorInfo->numOfChannels) * gp_SensorInfo->numOfChannels);

	// This is for error checking, If these checks are true, the defaults are not being set.
	if (op_mode == WAVEFORM_MODE)
	{
		if ((help_rec.alarm_one_mode == ALARM_MODE_SEISMIC) || (help_rec.alarm_one_mode == ALARM_MODE_BOTH))
		{
			if (help_rec.alarm_one_seismic_lvl < trig_mn.seismicTriggerLevel)
			{
				help_rec.alarm_one_seismic_lvl = trig_mn.seismicTriggerLevel;
			}
		}

		if ((help_rec.alarm_two_mode == ALARM_MODE_SEISMIC) || (help_rec.alarm_two_mode == ALARM_MODE_BOTH))
		{
			if (help_rec.alarm_two_seismic_lvl < trig_mn.seismicTriggerLevel)
			{
				help_rec.alarm_two_seismic_lvl = trig_mn.seismicTriggerLevel;
			}
		}

		if ((help_rec.alarm_one_mode == ALARM_MODE_AIR) || (help_rec.alarm_one_mode == ALARM_MODE_BOTH))
		{
			if (help_rec.alarm_one_air_lvl < (uint32)trig_mn.soundTriggerLevel)
			{
				help_rec.alarm_one_air_lvl = (uint32)trig_mn.soundTriggerLevel;
			}
		}

		if ((help_rec.alarm_two_mode == ALARM_MODE_AIR) || (help_rec.alarm_two_mode == ALARM_MODE_BOTH))
		{
			if (help_rec.alarm_two_air_lvl < (uint32)trig_mn.soundTriggerLevel)
			{
				help_rec.alarm_two_air_lvl = (uint32)trig_mn.soundTriggerLevel;
			}
		}
	}

	//-------------------------
	// Debug Information
	//-------------------------

	if (op_mode == WAVEFORM_MODE)
	{
		debug("--- Waveform Mode Settings ---\n");
		debug("\tRecord Time: %d, Sample Rate: %d\n", trig_rec.trec.record_time, trig_mn.sample_rate);
		debug("\tSeismic Trigger Count: 0x%x, Air Trigger Level: 0x%x db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
	}
	else if (op_mode == BARGRAPH_MODE)
	{
		debug("--- Bargraph Mode Settings ---\n");
		//debug("\tRecord Time: %d, Sample Rate: %d\n", msgs430.startMsg430.total_record_time, trig_mn.sample_rate);
		//debug("\tBar Interval: %d secs, Summary Interval: %d mins\n", trig_rec.bgrec.barInterval, (trig_rec.bgrec.summaryInterval / 60));
	}
	else if (op_mode == MANUAL_TRIGGER_MODE)
	{
		debug("--- Manual Trigger Mode Settings ---\n");
		//debug("\tRecord Time: %d, Sample Rate: %d\n", msgs430.startMsg430.total_record_time, trig_mn.sample_rate);
		//debug("\tSeismic Trigger Count: %d, Air Trigger Level: %d db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
	}
	else if (op_mode == MANUAL_CAL_MODE)
	{
		debug("--- Manual Cal Mode Settings ---\n");
	}

#if 0
	if (gp_SensorInfo->numOfChannels == NUMBER_OF_CHANNELS_DEFAULT)
	{
		debug("--- Seismic Group 1 Setup ---\n");
		for (i = 0; i < NUMBER_OF_CHANNELS_DEFAULT; i++)
		{
			byteSet(&channelType[0], 0, sizeof(channelType));
			
			switch (msgs430.startMsg430.channel[i].channel_type)
			{
				case RADIAL_CHANNEL_TYPE	: strcpy(channelType, "Radial    "); break;
				case VERTICAL_CHANNEL_TYPE	: strcpy(channelType, "Vertical  "); break;
				case TRANSVERSE_CHANNEL_TYPE: strcpy(channelType, "Transverse"); break;
				case ACOUSTIC_CHANNEL_TYPE	: strcpy(channelType, "Acoustic  "); break;
			}

			debug("\tChan %d: <%s>, 430 A/D Input: %d, Trig: 0x%x, Alarm 1: 0x%x, Alarm 2:0x%x\n",
					(i + 1), channelType, msgs430.startMsg430.channel[i].channel_num,
					swapInt(msgs430.startMsg430.channel[i].trig_lvl_1),
					swapInt(msgs430.startMsg430.channel[i].trig_lvl_2),
					swapInt(msgs430.startMsg430.channel[i].trig_lvl_3));
		}
	}
#endif		
	debug("---------------------------\n");

	// Check if mode is Manual Cal
	if (op_mode == MANUAL_CAL_MODE)
	{
		// Raise flag
		manual_cal_flag = TRUE;
	}
	else // Waveform, Bargraph
	{
		// Clear flag
		manual_cal_flag = FALSE;
		manualCalSampleCount = 0;

		// Create a new monitor log entry
		newMonitorLogEntry(op_mode);
	}

	if (op_mode == BARGRAPH_MODE)
	{
		bargraphForcedCalibration();
	}

	// Initialize buffers and settings and gp_ramEventRecord
	debug("Init data buffers\n");
	InitDataBuffs(op_mode);

	// Setup Analog controls
	debug("Setup Analog controls\n");
	SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1);
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL);

	// Enable the A/D
	debug("Enable the A/D\n");
	powerControl(ANALOG_SLEEP_ENABLE, OFF);		

	// Get current A/D offsets for normalization
	debug("Get Channel offsets\n");
	GetChannelOffsets();

	// Gather 1/4 second worth of data before comparing samples to the trigger
	//LoadPretrigger();

	// Adjust the RTC Periodic interrupt rate to fire at the sample rate
	debug("Adjust RTC Periodic interrupt based on sample rate\n");

#if 1
	switch(trig_rec.trec.sample_rate)
	{
		case 1024: SetPeriodicInterruptFrequency(PULSE_1K_PER_SEC); break;
		case 2048: SetPeriodicInterruptFrequency(PULSE_2K_PER_SEC); break;
		case 4096: SetPeriodicInterruptFrequency(PULSE_4K_PER_SEC); break;
		case 8192: SetPeriodicInterruptFrequency(PULSE_8K_PER_SEC); break;

		default: SetPeriodicInterruptFrequency(PULSE_1K_PER_SEC); break;
	}
#endif

	// Set flag to Sampling, we are about to begin to sample.
	debug("Raise signal to start sampling\n");
	g_sampleProcessing = SAMPLING_STATE;

	// Send message to 430
	//ISPI_SendMsg(msgs430.startMsg430.cmd_id);
}

/****************************************
*	Function:	 mnStopTrigger
*	Purpose:
****************************************/
void mnStopTrigger(void)
{
	g_sampleProcessing = IDLE_STATE;

	SetPeriodicInterruptFrequency(PULSE_2_PER_SEC);

	powerControl(ANALOG_SLEEP_ENABLE, ON);		

	clearSoftTimer(MENU_UPDATE_TIMER_NUM);

	// Check if not in Timer Mode and if the Power Off key is disabled
	if ((help_rec.timer_mode != ENABLED) && (getPowerControlState(POWER_SHUTDOWN_ENABLE) == ON))
	{
		// Set Power Shutdown control to be enabled
		debug("Stop Trigger: Re-Enabling Power Off key\n");
		powerControl(POWER_SHUTDOWN_ENABLE, OFF);
	}
}

/****************************************
*	Function:	 stopDataTransfer
*	Purpose:
****************************************/
void stopDataTransfer(void)
{
	g_sampleProcessing = IDLE_STATE;

	SetPeriodicInterruptFrequency(PULSE_2_PER_SEC);

	powerControl(ANALOG_SLEEP_ENABLE, OFF);		
}

/****************************************
*	Function:	 waitForEventProcessingToFinish
*	Purpose:
****************************************/
void waitForEventProcessingToFinish(void)
{
	//uint32 waitForCalCount = 0;

	if (isTriggered || processingCal || gCalTestExpected)
	{
		overlayMessage(getLangText(STATUS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT), 0);

		while (isTriggered || processingCal || gCalTestExpected)
		{
			// Just wait for the cal and end immediately afterwards
			soft_usecWait(250);
		}
	}
}

/****************************************
*	Function:	 seisTriggerConvert
*	Purpose:
****************************************/
uint16 seisTriggerConvert(float seismicTriggerLevel)
{
    uint16 seisTriggerVal;
    uint8 gainFactor = (uint8)((trig_rec.srec.sensitivity == LOW) ? 2 : 4);
    float convertToHex = (float)(factory_setup_rec.sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_DEFAULT);
    
    convertToHex = (float)ADC_RESOLUTION / (float)convertToHex;
  
	if ((seismicTriggerLevel != NO_TRIGGER_CHAR) && (seismicTriggerLevel != MANUAL_TRIGGER_CHAR))
    {
		// Convert the trigger level into a hex value for the 430 processor board.
		seisTriggerVal = (uint16)(((float)convertToHex * (float)seismicTriggerLevel) + (float)0.5);	
			
		//debug("seisTriggerConvert: seismicTriggerLevel=%f seisTriggerVal = 0x%x convertToHex=%d\n", 
		//	seismicTriggerLevel, seisTriggerVal, convertToHex);
	}
	else
	{
		seisTriggerVal = (uint16)seismicTriggerLevel;
	}

	return (swapInt(seisTriggerVal));
}

/****************************************
*	Function:	 airTriggerConvert
*	Purpose:
****************************************/
uint16 airTriggerConvert(uint32 airTriggerLevel)
{
	// Check if the air trigger level is set for no trigger
	if ((airTriggerLevel != NO_TRIGGER_CHAR) && (airTriggerLevel != MANUAL_TRIGGER_CHAR))
	{
		// Convert the float db to an offset from 0 to 2048
		airTriggerLevel = (uint32)dbToHex((float)airTriggerLevel);
	}

	// Swap the air trigger level so that the 430 can read the word value
	return (swapInt((uint16)airTriggerLevel));
}

/****************************************
*	Function:	 stopMonitoring
*	Purpose:
****************************************/
void stopMonitoring(uint8 mode, uint8 operation)
{
	// Check if the unit is currently monitoring
	if (g_sampleProcessing == SAMPLING_STATE)
	{
		if (mode == WAVEFORM_MODE)
		{
			// Set flag to prevent any more incoming events from being processed
			g_doneTakingEvents = YES;

			// Wait for any triggered events to finish sending
			waitForEventProcessingToFinish();

			// Need to stop all print jobs
		}
#if 0
		else if ((mode == BARGRAPH_MODE) && (operation == EVENT_PROCESSING))
		{
			raiseSystemEventFlag(BARGRAPH_EVENT);
		}
#endif
		// Stop the 430 data transfers
		mnStopTrigger();

		// Reset the Waveform flag
		g_doneTakingEvents = NO;

		if ((mode == WAVEFORM_MODE) && (operation == FINISH_PROCESSING))
		{
			while (getSystemEventState(TRIGGER_EVENT))
			{
				debug("Trigger Event 1\n");
				MoveWaveformEventToFlash();
			}
		}
		//else if ((mode == BARGRAPH_MODE) && (operation == FINISH_PROCESSING))
		else if (mode == BARGRAPH_MODE)
		{
			// Handle the end of a Bargraph event
			EndBargraph();
		}
		
		// Check if any events are waiting to still be processed
		if (!getSystemEventState(TRIGGER_EVENT))
		{
			closeMonitorLogEntry();
		}
	}
	
	// Check if Auto Monitor is active and not in monitor mode
	if ((help_rec.auto_monitor_mode != AUTO_NO_TIMEOUT) && (operation == EVENT_PROCESSING))
	{
		assignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(help_rec.auto_monitor_mode * TICKS_PER_MIN), autoMonitorTimerCallBack);
	}
}

/****************************************
*	Function:	 handleManualCalibration
*	Purpose:
****************************************/
void handleManualCalibration(void)
{
	FLASH_USAGE_STRUCT flashStats;
	uint8 holdOpMode;

	// Check if currently monitoring
	if (g_sampleProcessing == SAMPLING_STATE)
	{
		// Check if Waveform mode and not handling an cached event
		if ((trig_rec.op_mode == WAVEFORM_MODE) && (getSystemEventState(TRIGGER_EVENT) == NO))
		{
			// Check if still waiting for an event and not processing a cal and not waiting for a cal
			if ((isTriggered == NO) && (processingCal == NO) && (gCalTestExpected == NO))
			{
				flashStats = getFlashUsageStats();
				
				if ((help_rec.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
				{
					overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
				}
				else
				{
					// Stop data transfer
					stopDataTransfer();

					// Perform Cal while in monitor mode
					overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 1);

					// Issue a Cal Pulse message to the 430
					InitDataBuffs(MANUAL_CAL_MODE);
					manual_cal_flag = TRUE;

					SetPeriodicInterruptFrequency(PULSE_1K_PER_SEC);

					// Set flag to Sampling, we are about to begin to sample.
					g_sampleProcessing = SAMPLING_STATE;

					// Generate Cal Signal
					GenerateCalSignal();

					// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
					soft_usecWait(250 * SOFT_MSECS);

					// Stop data transfer
					stopDataTransfer();

					if (getSystemEventState(MANUEL_CAL_EVENT))
						MoveManuelCalToFlash();

					InitDataBuffs(trig_rec.op_mode);
					manual_cal_flag = FALSE;
					manualCalSampleCount = 0;

					// Set flag to Sampling, we are about to begin to sample.
					g_sampleProcessing = SAMPLING_STATE;

					// Send message to 430
					ISPI_SendMsg(msgs430.startMsg430.cmd_id);
				}
			}
		}					
	}
	else // Performing Cal outside of monitor mode
	{
		flashStats = getFlashUsageStats();
		
		if ((help_rec.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
		{
			overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
		}
		else
		{
			overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", (1 * SOFT_SECS));

			// Temp set mode to waveform to force the 430 ISR to call ProcessWaveformData instead of ProcessBargraphData 
			// after the calibration finishes to prevent a lockup when bargraph references globals that are not inited yet
			holdOpMode = trig_rec.op_mode;
			trig_rec.op_mode = WAVEFORM_MODE;

			// Stop data transfer
			mnStopTrigger();

			// Issue a Cal Pulse message to the 430
			mnStartTrigger(trig_rec.trec, MANUAL_CAL_PULSE_CMD, MANUAL_CAL_MODE);

			// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
			soft_usecWait(250 * SOFT_MSECS);

			// Stop data transfer
			mnStopTrigger();
			
			// Restore Op mode
			trig_rec.op_mode = holdOpMode;
		}
	}
}

/****************************************
*	Function:	 bargraphForcedCalibration
*	Purpose:
****************************************/
void bargraphForcedCalibration(void)
{
	INPUT_MSG_STRUCT mn_msg;

	overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 0);

	g_bargraphForcedCal = YES;

	// Issue a Cal Pulse message to the 430
	InitDataBuffs(MANUAL_CAL_MODE);
	manual_cal_flag = TRUE;

	// Temp set mode to waveform to force the 430 ISR to call ProcessWaveformData instead of ProcessBargraphData 
	// after the calibration finishes to prevent a lockup when bargraph references globals that are not inited yet
	trig_rec.op_mode = WAVEFORM_MODE;

	SetPeriodicInterruptFrequency(PULSE_1K_PER_SEC);

	// Set flag to Sampling, we are about to begin to sample.
	g_sampleProcessing = SAMPLING_STATE;

	// Send message to 430
	//ISPI_SendMsg(MANUAL_CAL_PULSE_CMD);
	GenerateCalSignal();

	// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
	soft_usecWait(250 * SOFT_MSECS);

	// Stop data transfer
	stopDataTransfer();

	if (getSystemEventState(MANUEL_CAL_EVENT))
		MoveManuelCalToFlash();

	if (getMenuEventState(RESULTS_MENU_EVENT)) 
	{
		clearMenuEventFlag(RESULTS_MENU_EVENT);

		active_menu = RESULTS_MENU;

		RESULTS_MENU_MANUEL_CAL_MSG();

		(*menufunc_ptrs[active_menu]) (mn_msg);
	}

	// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
	soft_usecWait(3 * SOFT_SECS);

	active_menu = MONITOR_MENU;

	g_bargraphForcedCal = NO;

	// Reset the mode back to bargraph
	trig_rec.op_mode = BARGRAPH_MODE;

	updateMonitorLogEntry();
}
