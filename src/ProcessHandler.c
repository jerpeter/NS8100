///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2012, All Rights Reserved 
///
///	$RCSfile: $
///	$Author: jpeterson $
///	$Date: $
///
///	$Source: $
///	$Revision: $
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
uint32 gTotalSamples;
uint16 manual_cal_flag = FALSE;
uint16 manualCalSampleCount = 0;
uint8 g_bargraphForcedCal = NO;
uint8 g_skipAutoCalInWaveformAfterMidnightCal = NO;
//uint8 g_waitForUser = FALSE;
//uint8 g_promtForLeavingMonitorMode = FALSE;
//uint8 g_promtForCancelingPrintJobs = FALSE;
//uint8 g_monitorModeActiveChoice = MB_FIRST_CHOICE;
//uint8 g_monitorEscapeCheck = YES;
//uint8 g_displayBargraphResultsMode = SUMMARY_INTERVAL_RESULTS;
//uint8 g_displayAlternateResultState = DEFAULT_RESULTS;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
uint16 seisTriggerConvert(float);
uint16 airTriggerConvert(uint32);
void startDataCollection(uint32 sampleRate);

/****************************************
*	Function:  startMonitoring()
*	Purpose:
****************************************/
void startMonitoring(TRIGGER_EVENT_DATA_STRUCT trig_mn, uint8 cmd_id, uint8 op_mode)
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
	if ((op_mode == WAVEFORM_MODE) || (op_mode == COMBO_MODE))
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
		debug("\tRecord Time: %d, Sample Rate: %d, Channels: %d\n", trig_rec.trec.record_time, trig_mn.sample_rate, gp_SensorInfo->numOfChannels);
		debug("\tSeismic Trigger Count: 0x%x, Air Trigger Level: 0x%x db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
	}
	else if (op_mode == BARGRAPH_MODE)
	{
		debug("--- Bargraph Mode Settings ---\n");
		//debug("\tRecord Time: %d, Sample Rate: %d\n", msgs430.startMsg430.total_record_time, trig_mn.sample_rate);
		debug("\tBar Interval: %d secs, Summary Interval: %d mins\n", trig_rec.bgrec.barInterval, (trig_rec.bgrec.summaryInterval / 60));
	}
	else if (op_mode == COMBO_MODE)
	{
		debug("--- Combo Mode Settings ---\n");
		debug("\tRecord Time: %d, Sample Rate: %d, Channels: %d\n", trig_rec.trec.record_time, trig_mn.sample_rate, gp_SensorInfo->numOfChannels);
		debug("\tSeismic Trigger Count: 0x%x, Air Trigger Level: 0x%x db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
		debug("\tBar Interval: %d secs, Summary Interval: %d mins\n", trig_rec.bgrec.barInterval, (trig_rec.bgrec.summaryInterval / 60));
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

#if 0 // Old 430 Code - Remove
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
	else // Waveform, Bargraph, Combo
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

	startDataCollection(trig_mn.sample_rate);
}

#if 1
extern void Setup_Data_Clock_ISR(uint32 sampleRate);
extern void Start_Data_Clock(void);
extern void AD_Init(void);
#endif
/****************************************
*	Function:	 startDataCollection
*	Purpose:
****************************************/
void startDataCollection(uint32 sampleRate)
{
	// Enable the A/D
	debug("Enable the A/D\n");
	powerControl(ANALOG_SLEEP_ENABLE, OFF);		

	// Initialize the A/D
	AD_Init();
	
	// Get current A/D offsets for normalization
	debug("Getting Channel offsets\n");
	GetChannelOffsets();

	// Setup ISR to clock the data sampling
	Setup_Data_Clock_ISR(sampleRate);

	// Start the timer for collecting data
	Start_Data_Clock();

	// Gather 1/4 second worth of data before comparing samples
	soft_usecWait(1 * SOFT_SECS);

	// Change state to start processing the samples
	debug("Raise signal to start sampling\n");
	g_sampleProcessing = SAMPLING_STATE;

	// Send message to 430
	//ISPI_SendMsg(msgs430.startMsg430.cmd_id);
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
		else if ((mode == BARGRAPH_MODE) && (operation == EVENT_PROCESSING))
		{
			raiseSystemEventFlag(BARGRAPH_EVENT);
		}

		// Stop the data transfers
		stopDataCollection();

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

#if 1 // fix_ns8100
extern void Stop_Data_Clock(void);
#endif
/****************************************
*	Function:	 stopDataCollection
*	Purpose:
****************************************/
void stopDataCollection(void)
{
	g_sampleProcessing = IDLE_STATE;

	Stop_Data_Clock();

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
*	Function:	 stopDataClock
*	Purpose:
****************************************/
void stopDataClock(void)
{
	g_sampleProcessing = IDLE_STATE;

	Stop_Data_Clock();

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
		debug("IsTriggered: %d, ProcCal: %d, CalTestExp: %d", isTriggered, processingCal, gCalTestExpected);
		
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
				getFlashUsageStats(&flashStats);
				
				if ((help_rec.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
				{
					overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
				}
				else
				{
					// Stop data transfer
					stopDataClock();

					// Perform Cal while in monitor mode
					overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 1);

					// Issue a Cal Pulse message to the 430
					InitDataBuffs(MANUAL_CAL_MODE);
					manual_cal_flag = TRUE;

#if 0 // fix_ns8100
					// Set flag to Sampling, we are about to begin to sample.
					g_sampleProcessing = SAMPLING_STATE;
#else
					startDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
#endif

					// Generate Cal Signal
					GenerateCalSignal();

					// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
					soft_usecWait(250 * SOFT_MSECS);

					// Stop data transfer
					stopDataClock();

					if (getSystemEventState(MANUEL_CAL_EVENT))
						MoveManuelCalToFlash();

					InitDataBuffs(trig_rec.op_mode);
					manual_cal_flag = FALSE;
					manualCalSampleCount = 0;

#if 0 // fix_ns8100
					// Set flag to Sampling, we are about to begin to sample.
					g_sampleProcessing = SAMPLING_STATE;

					// Send message to 430
					ISPI_SendMsg(msgs430.startMsg430.cmd_id);
#else
					startDataCollection(trig_rec.trec.sample_rate);
#endif
				}
			}
		}					
	}
	else // Performing Cal outside of monitor mode
	{
#if 0 // fix_ns8100
		getFlashUsageStats(&flashStats);
		
		if ((help_rec.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
		{
			overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
		}
		else
		{
#endif
			overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", (1 * SOFT_SECS));

			// Temp set mode to waveform to force the 430 ISR to call ProcessWaveformData instead of ProcessBargraphData 
			// after the calibration finishes to prevent a lockup when bargraph references globals that are not inited yet
			holdOpMode = trig_rec.op_mode;
			trig_rec.op_mode = WAVEFORM_MODE;

			// Stop data transfer
			stopDataCollection();

			// Issue a Cal Pulse message to the 430
			startMonitoring(trig_rec.trec, MANUAL_CAL_PULSE_CMD, MANUAL_CAL_MODE);

			// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
			soft_usecWait(250 * SOFT_MSECS);

			// Stop data transfer
			stopDataCollection();
			
			// Restore Op mode
			trig_rec.op_mode = holdOpMode;
#if 0 // fix_ns8100
		}
#endif
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

#if 0 // fix_ns8100
	// Set flag to Sampling, we are about to begin to sample.
	g_sampleProcessing = SAMPLING_STATE;

	// Send message to 430
	//ISPI_SendMsg(MANUAL_CAL_PULSE_CMD);
#else
	startDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
#endif

	GenerateCalSignal();

	// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
	soft_usecWait(250 * SOFT_MSECS);

	// Stop data transfer
	stopDataClock();

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
