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

	// Assign a one second menu update timer
	assignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, keypadLedUpdateTimerCallBack);

	// Where ever num_sensor_channels gets set g_totalSamples needs to be 
	// set also it is used to know the size of data burst expected from 
	// msp430 when collecting sample data.  

	// TODO: This should be moved to where the trigger data is setup.	  
	// g_numOfSensorChannels = 4; // hardcoded for test   
	// This creates a buffer size in an even integral of the number of sensors.
	g_totalSamples = (uint32)((SAMPLE_BUF_SIZE / g_sensorInfoPtr->numOfChannels) * g_sensorInfoPtr->numOfChannels);

	// This is for error checking, If these checks are true, the defaults are not being set.
	if ((op_mode == WAVEFORM_MODE) || (op_mode == COMBO_MODE))
	{
		if ((g_helpRecord.alarm_one_mode == ALARM_MODE_SEISMIC) || (g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarm_one_seismic_lvl < trig_mn.seismicTriggerLevel)
			{
				g_helpRecord.alarm_one_seismic_lvl = trig_mn.seismicTriggerLevel;
			}
		}

		if ((g_helpRecord.alarm_two_mode == ALARM_MODE_SEISMIC) || (g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarm_two_seismic_lvl < trig_mn.seismicTriggerLevel)
			{
				g_helpRecord.alarm_two_seismic_lvl = trig_mn.seismicTriggerLevel;
			}
		}

		if ((g_helpRecord.alarm_one_mode == ALARM_MODE_AIR) || (g_helpRecord.alarm_one_mode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarm_one_air_lvl < (uint32)trig_mn.soundTriggerLevel)
			{
				g_helpRecord.alarm_one_air_lvl = (uint32)trig_mn.soundTriggerLevel;
			}
		}

		if ((g_helpRecord.alarm_two_mode == ALARM_MODE_AIR) || (g_helpRecord.alarm_two_mode == ALARM_MODE_BOTH))
		{
			if (g_helpRecord.alarm_two_air_lvl < (uint32)trig_mn.soundTriggerLevel)
			{
				g_helpRecord.alarm_two_air_lvl = (uint32)trig_mn.soundTriggerLevel;
			}
		}
	}

	//-------------------------
	// Debug Information
	//-------------------------

	if (op_mode == WAVEFORM_MODE)
	{
		debug("--- Waveform Mode Settings ---\n");
		debug("\tRecord Time: %d, Sample Rate: %d, Channels: %d\n", g_triggerRecord.trec.record_time, trig_mn.sample_rate, g_sensorInfoPtr->numOfChannels);
		debug("\tSeismic Trigger Count: 0x%x, Air Trigger Level: 0x%x db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
	}
	else if (op_mode == BARGRAPH_MODE)
	{
		debug("--- Bargraph Mode Settings ---\n");
		//debug("\tRecord Time: %d, Sample Rate: %d\n", g_msgs430.startMsg430.total_record_time, trig_mn.sample_rate);
		debug("\tBar Interval: %d secs, Summary Interval: %d mins\n", g_triggerRecord.bgrec.barInterval, (g_triggerRecord.bgrec.summaryInterval / 60));
	}
	else if (op_mode == COMBO_MODE)
	{
		debug("--- Combo Mode Settings ---\n");
		debug("\tRecord Time: %d, Sample Rate: %d, Channels: %d\n", g_triggerRecord.trec.record_time, trig_mn.sample_rate, g_sensorInfoPtr->numOfChannels);
		debug("\tSeismic Trigger Count: 0x%x, Air Trigger Level: 0x%x db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
		debug("\tBar Interval: %d secs, Summary Interval: %d mins\n", g_triggerRecord.bgrec.barInterval, (g_triggerRecord.bgrec.summaryInterval / 60));
	}
	else if (op_mode == MANUAL_TRIGGER_MODE)
	{
		debug("--- Manual Trigger Mode Settings ---\n");
		//debug("\tRecord Time: %d, Sample Rate: %d\n", g_msgs430.startMsg430.total_record_time, trig_mn.sample_rate);
		//debug("\tSeismic Trigger Count: %d, Air Trigger Level: %d db\n", trig_mn.seismicTriggerLevel, trig_mn.soundTriggerLevel);
	}
	else if (op_mode == MANUAL_CAL_MODE)
	{
		debug("--- Manual Cal Mode Settings ---\n");
	}

#if 0 // Old 430 Code - Remove
	if (g_sensorInfoPtr->numOfChannels == NUMBER_OF_CHANNELS_DEFAULT)
	{
		debug("--- Seismic Group 1 Setup ---\n");
		for (i = 0; i < NUMBER_OF_CHANNELS_DEFAULT; i++)
		{
			byteSet(&channelType[0], 0, sizeof(channelType));
			
			switch (g_msgs430.startMsg430.channel[i].channel_type)
			{
				case RADIAL_CHANNEL_TYPE	: strcpy(channelType, "Radial    "); break;
				case VERTICAL_CHANNEL_TYPE	: strcpy(channelType, "Vertical  "); break;
				case TRANSVERSE_CHANNEL_TYPE: strcpy(channelType, "Transverse"); break;
				case ACOUSTIC_CHANNEL_TYPE	: strcpy(channelType, "Acoustic  "); break;
			}

			debug("\tChan %d: <%s>, 430 A/D Input: %d, Trig: 0x%x, Alarm 1: 0x%x, Alarm 2:0x%x\n",
					(i + 1), channelType, g_msgs430.startMsg430.channel[i].channel_num,
					swapInt(g_msgs430.startMsg430.channel[i].trig_lvl_1),
					swapInt(g_msgs430.startMsg430.channel[i].trig_lvl_2),
					swapInt(g_msgs430.startMsg430.channel[i].trig_lvl_3));
		}
	}
#endif		
	debug("---------------------------\n");

	// Check if mode is Manual Cal
	if (op_mode == MANUAL_CAL_MODE)
	{
		// Raise flag
		g_manualCalFlag = TRUE;
		g_manualCalSampleCount = MAX_CAL_SAMPLES;
	}
	else // Waveform, Bargraph, Combo
	{
		// Clear flag
		g_manualCalFlag = FALSE;
		g_manualCalSampleCount = 0;

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

	// Set the cutoff frequency based on sample rate
	switch (trig_mn.sample_rate)
	{
		case 512: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_LOW); break;
		case 1024: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1); break;
		case 2048: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_2); break;
		case 4096: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_3); break;

		// Handle 8K, 16K and 32K the same
		case 8192: case 16384: case 32768: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_4); break;
			
		// Default just in case it's a custom frequency
		default: SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1); break;
	}
	
	// Set the sensitivity based on the current settings
	if (g_triggerRecord.srec.sensitivity == LOW) { SetSeismicGainSelect(SEISMIC_GAIN_LOW); }
	else { SetSeismicGainSelect(SEISMIC_GAIN_HIGH); }
	
	if (g_factorySetupRecord.aweight_option == DISABLED) { SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL); }
	else { SetAcousticGainSelect(ACOUSTIC_GAIN_A_WEIGHTED);	}

	// Monitor some data.. oh yeah
	startDataCollection(trig_mn.sample_rate);
}

#if 1
extern void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM channel);
extern void Start_Data_Clock(TC_CHANNEL_NUM channel);
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
	debug("Getting channel offsets...\n");
	GetChannelOffsets();

	debug("Setup TC clocks...\n");
	// Setup ISR to clock the data sampling
	Setup_8100_TC_Clock_ISR(sampleRate, TC_SAMPLE_TIMER_CHANNEL);
	Setup_8100_TC_Clock_ISR(CAL_PULSE_FIXED_SAMPLE_RATE, TC_CALIBRATION_TIMER_CHANNEL);

	debug("Start sampling...\n");
	// Start the timer for collecting data
	Start_Data_Clock(TC_SAMPLE_TIMER_CHANNEL);

#if 0 // Pretrigger fill is now inside the ISR
	// Gather 1/4 second worth of data before comparing samples
	soft_usecWait(1 * SOFT_SECS);
#endif

	// Change state to start processing the samples
	debug("Raise signal to start sampling\n");
	g_sampleProcessing = ACTIVE_STATE;

#if 0 // ns7100
	// Send message to 430
	//ISPI_SendMsg(g_msgs430.startMsg430.cmd_id);
#endif
}

/****************************************
*	Function:	 stopMonitoring
*	Purpose:
****************************************/
void stopMonitoring(uint8 mode, uint8 operation)
{
	// Check if the unit is currently monitoring
	if (g_sampleProcessing == ACTIVE_STATE)
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
	if ((g_helpRecord.auto_monitor_mode != AUTO_NO_TIMEOUT) && (operation == EVENT_PROCESSING))
	{
		assignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_helpRecord.auto_monitor_mode * TICKS_PER_MIN), autoMonitorTimerCallBack);
	}
}

/****************************************
*	Function:	 stopDataCollection
*	Purpose:
****************************************/
extern void Stop_Data_Clock(TC_CHANNEL_NUM);
void stopDataCollection(void)
{
	g_sampleProcessing = IDLE_STATE;

	Stop_Data_Clock(TC_SAMPLE_TIMER_CHANNEL);

	powerControl(ANALOG_SLEEP_ENABLE, ON);		

	clearSoftTimer(MENU_UPDATE_TIMER_NUM);
	clearSoftTimer(KEYPAD_LED_TIMER_NUM);

	// Check if not in Timer Mode and if the Power Off key is disabled
	if ((g_helpRecord.timer_mode != ENABLED) && (getPowerControlState(POWER_SHUTDOWN_ENABLE) == ON))
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

	Stop_Data_Clock(TC_SAMPLE_TIMER_CHANNEL);

	powerControl(ANALOG_SLEEP_ENABLE, OFF);		
}

/****************************************
*	Function:	 waitForEventProcessingToFinish
*	Purpose:
****************************************/
void waitForEventProcessingToFinish(void)
{
	//uint32 waitForCalCount = 0;

	if (g_isTriggered || g_processingCal || g_calTestExpected)
	{
		debug("IsTriggered: %d, ProcCal: %d, CalTestExp: %d", g_isTriggered, g_processingCal, g_calTestExpected);
		
		overlayMessage(getLangText(STATUS_TEXT), getLangText(PLEASE_BE_PATIENT_TEXT), 0);

		while (g_isTriggered || g_processingCal || g_calTestExpected)
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
    uint8 gainFactor = (uint8)((g_triggerRecord.srec.sensitivity == LOW) ? 2 : 4);
    float convertToHex = (float)(g_factorySetupRecord.sensor_type)/(float)(gainFactor * SENSOR_ACCURACY_DEFAULT);
    
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
	//uint32 manualCalProgressCheck = MAX_CAL_SAMPLES;
	//uint8 holdOpMode;

	// Check if currently monitoring
	if (g_sampleProcessing == ACTIVE_STATE)
	{
		// Check if Waveform or Combo mode and not handling a cached event
		if (((g_triggerRecord.op_mode == WAVEFORM_MODE) || (g_triggerRecord.op_mode == COMBO_MODE)) && 
			(getSystemEventState(TRIGGER_EVENT) == NO))
		{
			// Check if still waiting for an event and not processing a cal and not waiting for a cal
			if ((g_isTriggered == NO) && (g_processingCal == NO) && (g_calTestExpected == NO))
			{
				getFlashUsageStats(&flashStats);
				
				// fix_ns8100
				if ((g_helpRecord.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
				{
					overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
				}
				else
				{
					// Stop data transfer
					stopDataClock();

					// Perform Cal while in monitor mode
					overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 0);

					InitDataBuffs(MANUAL_CAL_MODE);
					g_manualCalFlag = TRUE;
					g_manualCalSampleCount = MAX_CAL_SAMPLES;

					// Make sure Cal pulse is generated at low sensitivity
					SetSeismicGainSelect(SEISMIC_GAIN_LOW);

#if 0 // fix_ns8100
					// Set flag to Sampling, we are about to begin to sample.
					g_sampleProcessing = ACTIVE_STATE;
#else
					startDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
#endif

					// No longer needed, handled in the ISR for Cal
					//GenerateCalSignal();

					// Wait for the Cal Pulse to complete, 250ms + 100ms
					soft_usecWait(350 * SOFT_MSECS);

					// Just make absolutely sure we are done with the Cal pulse
					while ((volatile uint32)g_manualCalSampleCount != 0) { }
#if 0 // fix_ns8100
					{
						// After 350ms, the Cal pulse should have decremented the Manual Cal Sample count
						if ((volatile uint32)g_manualCalSampleCount == manualCalProgressCheck)
							break; // There's a problem, break out and hope for the best, but that won't happen
		
						// Cache the current value and try again
						manualCalProgressCheck = g_manualCalSampleCount;
						soft_usecWait(5 * SOFT_MSECS);
					}
#endif
					// Stop data transfer
					stopDataClock();

					if (getSystemEventState(MANUEL_CAL_EVENT))
						MoveManuelCalToFlash();

					InitDataBuffs(g_triggerRecord.op_mode);
					g_manualCalFlag = FALSE;
					g_manualCalSampleCount = 0;

#if 0 // fix_ns8100
					// Set flag to Sampling, we are about to begin to sample.
					g_sampleProcessing = ACTIVE_STATE;

					// Send message to 430
					ISPI_SendMsg(g_msgs430.startMsg430.cmd_id);
#else
					// Make sure to reset the gain after the Cal pulse (which is generated at low sensitivity)
					if (g_triggerRecord.srec.sensitivity == SEISMIC_GAIN_HIGH)
					{
						SetSeismicGainSelect(SEISMIC_GAIN_HIGH);
					}

					startDataCollection(g_triggerRecord.trec.sample_rate);
#endif
				}
			}
		}					
	}
	else // Performing Cal outside of monitor mode
	{
		getFlashUsageStats(&flashStats);
		
		// fix_ns8100
		if ((g_helpRecord.flash_wrapping == NO) && (flashStats.manualCalsLeft == 0))
		{
			overlayMessage(getLangText(WARNING_TEXT), "FLASH MEMORY IS FULL. (WRAPPING IS DISABLED) CAN NOT CALIBRATE.", (5 * SOFT_SECS));
		}
		else
		{
			overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 0);

#if 0 // ns7100
			// Temp set mode to waveform to force the 430 ISR to call ProcessWaveformData instead of ProcessBargraphData 
			// after the calibration finishes to prevent a lockup when bargraph references globals that are not inited yet
			holdOpMode = g_triggerRecord.op_mode;

			g_triggerRecord.op_mode = WAVEFORM_MODE;

			// Stop data transfer
			stopDataCollection();

			// Issue a Cal Pulse message to the 430
			startMonitoring(g_triggerRecord.trec, MANUAL_CAL_PULSE_CMD, MANUAL_CAL_MODE);

			// Wait until after the Cal Pulse has completed, 250ms to be safe (just less than 100 ms to complete)
			soft_usecWait(250 * SOFT_MSECS);

			// Stop data transfer
			stopDataCollection();
#endif

			InitDataBuffs(MANUAL_CAL_MODE);
			g_manualCalFlag = TRUE;
			g_manualCalSampleCount = MAX_CAL_SAMPLES;

			// Make sure Cal pulse is generated at low sensitivity
			SetSeismicGainSelect(SEISMIC_GAIN_LOW);

			startDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
			
			// Wait for the Cal Pulse to complete, 250ms + 100ms
			soft_usecWait(350 * SOFT_MSECS);

			// Just make absolutely sure we are done with the Cal pulse
			while ((volatile uint32)g_manualCalSampleCount != 0) { }
#if 0 // fix_ns8100
			{
				// After 350ms, the Cal pulse should have decremented the Manual Cal Sample count
				if ((volatile uint32)g_manualCalSampleCount == manualCalProgressCheck)
					break; // There's a problem, break out and hope for the best, but that won't happen
		
				// Cache the current value and try again
				manualCalProgressCheck = g_manualCalSampleCount;
				soft_usecWait(5 * SOFT_MSECS);
			}
#endif
			// Stop data transfer
			stopDataClock();

			if (getSystemEventState(MANUEL_CAL_EVENT))
				MoveManuelCalToFlash();

			g_manualCalFlag = FALSE;
			g_manualCalSampleCount = 0;

#if 0
			// Restore Op mode
			g_triggerRecord.op_mode = holdOpMode;
#endif
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
	//uint32 manualCalProgressCheck = MAX_CAL_SAMPLES;

	overlayMessage(getLangText(STATUS_TEXT), "PERFORMING MANUAL CAL", 0);

	g_bargraphForcedCal = YES;

	InitDataBuffs(MANUAL_CAL_MODE);
	g_manualCalFlag = TRUE;
	g_manualCalSampleCount = MAX_CAL_SAMPLES;

	// Make sure Cal pulse is generated at low sensitivity
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	
#if 0 // ns7100
	// Temp set mode to waveform to force the 430 ISR to call ProcessWaveformData instead of ProcessBargraphData 
	// after the calibration finishes to prevent a lockup when bargraph references globals that are not inited yet
	g_triggerRecord.op_mode = WAVEFORM_MODE;

	// Set flag to Sampling, we are about to begin to sample.
	g_sampleProcessing = ACTIVE_STATE;

	// Send message to 430
	//ISPI_SendMsg(MANUAL_CAL_PULSE_CMD);
#else // ns8100
	startDataCollection(MANUAL_CAL_DEFAULT_SAMPLE_RATE);
#endif

	// No longer needed, handled in the ISR for Cal
	//GenerateCalSignal();

	// Wait for the Cal Pulse to complete, 250ms + 100ms
	soft_usecWait(350 * SOFT_MSECS);

	// Just make absolutely sure we are done with the Cal pulse
	while ((volatile uint32)g_manualCalSampleCount != 0) { }
#if 0 // fix_ns8100
	{
		// After 350ms, the Cal pulse should have decremented the Manual Cal Sample count
		if ((volatile uint32)g_manualCalSampleCount == manualCalProgressCheck)
			break; // There's a problem, break out and hope for the best, but that won't happen
		
		// Cache the current value and try again
		manualCalProgressCheck = g_manualCalSampleCount;
		soft_usecWait(5 * SOFT_MSECS);
	}
#endif

	// Stop data transfer
	stopDataClock();

	g_manualCalFlag = FALSE;
	g_manualCalSampleCount = 0;

	if (getSystemEventState(MANUEL_CAL_EVENT))
		MoveManuelCalToFlash();

	if (getMenuEventState(RESULTS_MENU_EVENT)) 
	{
		clearMenuEventFlag(RESULTS_MENU_EVENT);

#if 0 // fix_ns8100
		g_activeMenu = RESULTS_MENU;

		RESULTS_MENU_MANUEL_CAL_MSG();

		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
#else
		UNUSED(mn_msg);
#endif
	}

	// Wait until after the Cal Pulse has completed
	soft_usecWait(1 * SOFT_SECS);

	g_activeMenu = MONITOR_MENU;

	g_bargraphForcedCal = NO;

	// Reset the mode back to bargraph
	g_triggerRecord.op_mode = BARGRAPH_MODE;

	updateMonitorLogEntry();
}
