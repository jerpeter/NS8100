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
#include "Uart.h"
#include "Display.h"
#include "Common.h"
#include "InitDataBuffers.h"
#include "Summary.h"
#include "SysEvents.h"
#include "Board.h"
#include "PowerManagement.h"
#include "Record.h"
#include "SoftTimer.h"
#include "Keypad.h"
#include "TextTypes.h"
#include "Analog.h"
#include "RealTimeClock.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define CAL_SETUP_MN_TABLE_SIZE 8
#define CAL_SETUP_WND_STARTING_COL DEFAULT_COL_THREE
#define CAL_SETUP_WND_END_COL DEFAULT_END_COL
#define CAL_SETUP_WND_STARTING_ROW DEFAULT_MENU_ROW_ZERO // DEFAULT_MENU_ROW_TWO
#define CAL_SETUP_WND_END_ROW (DEFAULT_MENU_ROW_SEVEN)
#define CAL_SETUP_MN_TBL_START_LINE 0

enum {
	CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY,
	CAL_MENU_CALIBRATED_DISPLAY,
	CAL_MENU_CALIBRATED_CHAN_NOISE_PERCENT_DISPLAY,
	CAL_MENU_DISPLAY_SAMPLES
};

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern USER_MENU_STRUCT helpMenu[];
extern void Setup_8100_TC_Clock_ISR(uint32, TC_CHANNEL_NUM);
extern void Start_Data_Clock(TC_CHANNEL_NUM);
extern void Stop_Data_Clock(TC_CHANNEL_NUM);
extern void SetupADChannelConfig(uint32 sampleRate);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static uint32 s_calDisplayScreen = CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY;
static uint32 s_calSavedSampleRate = 0;

#if 0
static union {
	uint16 chan[4];
	struct {
		uint16 a;
		uint16 r;
		uint16 v;
		uint16 t;
	};
} s_calPreTrigData[256];
#endif

typedef struct {
	uint16 chan[4];
} CALIBRATION_DATA;

static CALIBRATION_DATA* s_calibrationData;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void CalSetupMn(INPUT_MSG_STRUCT);
void CalSetupMnDsply(WND_LAYOUT_STRUCT*);
void CalSetupMnProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void MnStartCal(void);
void MnStopCal(void);

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CalSetupMn(INPUT_MSG_STRUCT msg)
{
	static WND_LAYOUT_STRUCT wnd_layout;
	static MN_LAYOUT_STRUCT mn_layout;
	volatile uint32 tempTicks = 0;
	//uint8 cursorLine = 0;
	volatile uint32 key = 0;
	uint8 mbChoice = 0;
	INPUT_MSG_STRUCT mn_msg;
	uint8 previousMode = g_triggerRecord.op_mode;

	uint8 choice = MessageBox(getLangText(VERIFY_TEXT), "START CAL DIAGNOSTICS?", MB_YESNO);

	if (choice == MB_FIRST_CHOICE)
	{
		CalSetupMnProc(msg, &wnd_layout, &mn_layout);
	}

	if (g_activeMenu == CAL_SETUP_MENU)
	{
		if (choice == MB_FIRST_CHOICE)
		{
			while ((key != ENTER_KEY) && (key != ESC_KEY))
			{
				// Check if a half second has gone by
				if (tempTicks != g_rtcSoftTimerTickCount)
				{
					// Update the current time since we never leave the loop
					UpdateCurrentTime();

					// Create the Cal Setup menu
					CalSetupMnDsply(&wnd_layout);

					WriteMapToLcd(g_mmap);

					// Set to current half second
					tempTicks = g_rtcSoftTimerTickCount;
				}

				key = GetKeypadKey(CHECK_ONCE_FOR_KEY);

				SoftUsecWait(10 * SOFT_MSECS);

				switch (key)
				{
					case UP_ARROW_KEY:
						if (s_calDisplayScreen == CAL_MENU_CALIBRATED_DISPLAY)
						{
							// Clear the stored offsets so that the A/D channel data is raw
							memset(&g_channelOffset, 0, sizeof(OFFSET_DATA_STRUCT));
							
							// Clear the Pretrigger buffer
							SoftUsecWait(250 * SOFT_MSECS);
							
							//debug("Cal Menu Screen 1 selected\n");
							s_calDisplayScreen = CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY;
						}
						else if (s_calDisplayScreen == CAL_MENU_CALIBRATED_CHAN_NOISE_PERCENT_DISPLAY)
						{
							//debug("Cal Menu Screen 2 selected\n");
							s_calDisplayScreen = CAL_MENU_CALIBRATED_DISPLAY;
						}
						else if (s_calDisplayScreen == CAL_MENU_DISPLAY_SAMPLES)
						{
							//debug("Cal Menu Screen 2 selected\n");
							s_calDisplayScreen = CAL_MENU_CALIBRATED_CHAN_NOISE_PERCENT_DISPLAY;
						}

						key = 0;
						break;

					case DOWN_ARROW_KEY:
						if (s_calDisplayScreen == CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY)
						{
							// Stop A/D data collection clock
#if INTERNAL_SAMPLING_SOURCE
							Stop_Data_Clock(TC_CALIBRATION_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
							StopExternalRtcClock();
#endif
							
							// Alert the system operator that the unit is calibrating
							OverlayMessage(getLangText(STATUS_TEXT), getLangText(CALIBRATING_TEXT), 0);
							
							// Get new channel offsets
							GetChannelOffsets(CALIBRATION_FIXED_SAMPLE_RATE);
							
							// Restart the data collection clock
#if INTERNAL_SAMPLING_SOURCE
							Start_Data_Clock(TC_CALIBRATION_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
							StartExternalRtcClock(SAMPLE_RATE_1K);
#endif
							
							// Clear the Pretrigger buffer
							SoftUsecWait(250 * SOFT_MSECS);

							//debug("Cal Menu Screen 2 selected\n");
							s_calDisplayScreen = CAL_MENU_CALIBRATED_DISPLAY;
						}
						else if (s_calDisplayScreen == CAL_MENU_CALIBRATED_DISPLAY)
						{
							//debug("Cal Menu Screen 3 selected\n");
							s_calDisplayScreen = CAL_MENU_CALIBRATED_CHAN_NOISE_PERCENT_DISPLAY;
						}
						else if (s_calDisplayScreen == CAL_MENU_CALIBRATED_CHAN_NOISE_PERCENT_DISPLAY)
						{
							//debug("Cal Menu Screen 3 selected\n");
							s_calDisplayScreen = CAL_MENU_DISPLAY_SAMPLES;
						}

						key = 0;
						break;

					case ENTER_KEY:
					case ESC_KEY:
						// Not really used, keys are grabbed in CalSetupMn
						mbChoice = MessageBox(getLangText(STATUS_TEXT), getLangText(ARE_YOU_DONE_WITH_CAL_SETUP_Q_TEXT), MB_YESNO);
						if ((mbChoice == MB_SECOND_CHOICE) || (mbChoice == MB_NO_ACTION))
						{
							// Don't leave
							key = 0;
						}
						break;
						
					case HELP_KEY:
						// Not really used, keys are grabbed in CalSetupMn
						mbChoice = MessageBox(getLangText(SAMPLE_RATE_TEXT), "", MB_YESNO);
						if ((mbChoice == MB_SECOND_CHOICE) || (mbChoice == MB_NO_ACTION))
						{
							// Don't leave
							key = 0;
						}
						break;
				}
			}

			MnStopCal();
			
			// Reestablish the previously stored sample rate
			g_triggerRecord.trec.sample_rate = s_calSavedSampleRate;

			if (MessageBox(getLangText(CONFIRM_TEXT), getLangText(DO_YOU_WANT_TO_SAVE_THE_CAL_DATE_Q_TEXT), MB_YESNO) == MB_FIRST_CHOICE)
			{
				// Store Calibration Date
				g_factorySetupRecord.cal_date = GetCurrentTime();
			}
		}

		SaveRecordData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);

		g_factorySetupSequence = SEQ_NOT_STARTED;

		MessageBox(getLangText(STATUS_TEXT), getLangText(FACTORY_SETUP_COMPLETE_TEXT), MB_OK);

		// Restore the previous mode
		g_triggerRecord.op_mode = previousMode;

		// Reset default screen to non calibrated
		//debug("Cal Menu Screen 1 selected\n");
		s_calDisplayScreen = CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY;

		SETUP_MENU_MSG(MAIN_MENU);
		JUMP_TO_ACTIVE_MENU();
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CalSetupMnProc(INPUT_MSG_STRUCT msg,
	WND_LAYOUT_STRUCT *wnd_layout_ptr, MN_LAYOUT_STRUCT *mn_layout_ptr)
{
	INPUT_MSG_STRUCT mn_msg;
	uint32 data;
	uint8 mbChoice;

	switch (msg.cmd)
	{
		case (ACTIVATE_MENU_CMD):
			wnd_layout_ptr->start_col =    CAL_SETUP_WND_STARTING_COL;
			wnd_layout_ptr->end_col =      CAL_SETUP_WND_END_COL;
			wnd_layout_ptr->start_row =    CAL_SETUP_WND_STARTING_ROW;
			wnd_layout_ptr->end_row =      CAL_SETUP_WND_END_ROW;
			mn_layout_ptr->curr_ln =       CAL_SETUP_MN_TBL_START_LINE;
			mn_layout_ptr->top_ln =        CAL_SETUP_MN_TBL_START_LINE;

			OverlayMessage("STATUS", "PLEASE WAIT...", 0);

			// Save the currently stored sample rate to later be reverted
			s_calSavedSampleRate = g_triggerRecord.trec.sample_rate;
			
			// Set the sample rate to a fixed 1K
			g_triggerRecord.trec.sample_rate = SAMPLE_RATE_1K;

			// Fool system and initialize buffers and pointers as if a waveform
			InitDataBuffs(WAVEFORM_MODE);

#if 0 // ns7100
			MnStopCal();
#endif

			// Hand setup A/D data collection and start the data clock
			MnStartCal();
			
			// Allow Pretrigger buffer to fill up
			SoftUsecWait(250 * SOFT_MSECS);
		break;

		case (KEYPRESS_MENU_CMD):
			data = msg.data[0];

			switch (data)
			{
				case (ENTER_KEY):
					break;

				case (ESC_KEY):
					mbChoice = MessageBox(getLangText(WARNING_TEXT), getLangText(DO_YOU_WANT_TO_LEAVE_CAL_SETUP_MODE_Q_TEXT), MB_YESNO);
					if ((mbChoice == MB_SECOND_CHOICE) || (mbChoice == MB_NO_ACTION))
					{
						// Dont leave, escape to continue
						break;
					}

					SETUP_USER_MENU_MSG(&helpMenu, CONFIG);
					JUMP_TO_ACTIVE_MENU();
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CalSetupMnDsply(WND_LAYOUT_STRUCT *wnd_layout_ptr)
{
	uint8 buff[50];
	uint8 length;
	uint16 i = 0, j = 0;
	int32 chanMin[4];
	int32 chanMax[4];
	int32 chanAvg[4];
	int16 chanMed[4][13];
	uint16 sampleDataMidpoint = 0x8000;

	DATE_TIME_STRUCT time;

	wnd_layout_ptr->curr_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->curr_col = wnd_layout_ptr->start_col;
	wnd_layout_ptr->next_row = wnd_layout_ptr->start_row;
	wnd_layout_ptr->next_col = wnd_layout_ptr->start_col;

	ByteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	//memcpy(&s_calPreTrigData[0], &g_startOfPretriggerBuff[0], (256 * 4 * 2));
	s_calibrationData = (CALIBRATION_DATA*)g_startOfPretriggerBuff;

	// Zero the Med
	memset(&chanMed[0][0], 0, sizeof(chanMed));

	// Set the Min and Max to mid level, Avg to zero
	for (i = 0; i < 4; i++)
	{
		chanMin[i] = 0xFFFF;
		chanMax[i] = 0x0000;
		chanAvg[i] = 0;
	}

	// Find the min and max, and add the counts of all the samples
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 4; j++)
		{
			if (s_calibrationData[i].chan[j] < chanMin[j]) chanMin[j] = s_calibrationData[i].chan[j];
			if (s_calibrationData[i].chan[j] > chanMax[j]) chanMax[j] = s_calibrationData[i].chan[j];
			chanAvg[j] += s_calibrationData[i].chan[j];

			if ((s_calibrationData[i].chan[j] >= (sampleDataMidpoint - 6)) && (s_calibrationData[i].chan[j] <= (sampleDataMidpoint + 6)))
			{
				if (chanMed[j][(s_calibrationData[i].chan[j] - (sampleDataMidpoint - 6))] < 255)
					chanMed[j][(s_calibrationData[i].chan[j] - (sampleDataMidpoint - 6))]++;
			}
		}
	}

	for (i = 0; i < 4; i++)
	{
		chanMin[i] -= sampleDataMidpoint;
		chanMax[i] -= sampleDataMidpoint;

		chanAvg[i] /= 256;
		chanAvg[i] -= sampleDataMidpoint;

		for (j = 0; j < 13; j++)
		{
			chanMed[i][j] *= 100;
			chanMed[i][j] /= 256;
		}
	}

	if ((s_calDisplayScreen == CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY) || (s_calDisplayScreen == CAL_MENU_CALIBRATED_DISPLAY) || (CAL_MENU_DISPLAY_SAMPLES))
	{
		// PRINT CAL_SETUP
		ByteSet(&buff[0], 0, sizeof(buff));
		length = (uint8)sprintf((char*)buff, "-%s-", getLangText(CAL_SETUP_TEXT));
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// DATE AND TIME
		ByteSet(&buff[0], 0, sizeof(buff));
		time = GetCurrentTime();
		ConvertTimeStampToString((char*)buff, &time, REC_DATE_TIME_TYPE);
		length = (uint8)strlen((char*)buff);
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
		WndMpWrtString(buff,wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);

		if (s_calDisplayScreen == CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY)
		{
			// PRINT Table separator
			ByteSet(&buff[0], 0, sizeof(buff));
			//sprintf((char*)buff, "--------------------");
			sprintf((char*)buff, "-----RAW NO CAL-----");
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);
		}
		else if (s_calDisplayScreen == CAL_MENU_CALIBRATED_DISPLAY)
		{
			// PRINT Table separator
			ByteSet(&buff[0], 0, sizeof(buff));
			//sprintf((char*)buff, "--------------------");
			sprintf((char*)buff, "-----CALIBRATED-----");
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);
		}
		else // CAL_MENU_DISPLAY_SAMPLES
		{
			// PRINT Table separator
			ByteSet(&buff[0], 0, sizeof(buff));
			//sprintf((char*)buff, "--------------------");
			sprintf((char*)buff, "-----RAW SAMPLES-----");
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);
		}

		if (s_calDisplayScreen == CAL_MENU_DISPLAY_SAMPLES)
		{
			// PRINT Table header
			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "C|  1st|  2nd|  3rd|");
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);

			// PRINT R,V,T,A Min, Max and Avg
			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "R| %04x| %04x| %04x|", s_calibrationData[0].chan[1], s_calibrationData[1].chan[1], s_calibrationData[2].chan[1]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "V| %04x| %04x| %04x|", s_calibrationData[0].chan[2], s_calibrationData[1].chan[2], s_calibrationData[2].chan[2]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "T| %04x| %04x| %04x|", s_calibrationData[0].chan[3], s_calibrationData[1].chan[3], s_calibrationData[2].chan[3]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "A| %04x| %04x| %04x|", s_calibrationData[0].chan[0], s_calibrationData[1].chan[0], s_calibrationData[2].chan[0]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
		else // ((s_calDisplayScreen == CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY) || (s_calDisplayScreen == CAL_MENU_CALIBRATED_DISPLAY))
		{
			// PRINT Table header
			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "C|  Min|  Max|  Avg|");
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);

			// PRINT R,V,T,A Min, Max and Avg
			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "R|%+5ld|%+5ld|%+5ld|", chanMin[1], chanMax[1], chanAvg[1]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "V|%+5ld|%+5ld|%+5ld|", chanMin[2], chanMax[2], chanAvg[2]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "T|%+5ld|%+5ld|%+5ld|", chanMin[3], chanMax[3], chanAvg[3]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

			ByteSet(&buff[0], 0, sizeof(buff));
			sprintf((char*)buff, "A|%+5ld|%+5ld|%+5ld|", chanMin[0], chanMax[0], chanAvg[0]);
			wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
			WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
		}
	}
	else // s_calDisplayScreen == CAL_MENU_CALIBRATED_CHAN_NOISE_PERCENT_DISPLAY
	{
		// R
		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "R%%|%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[1][5], chanMed[1][4], chanMed[1][3],
				chanMed[1][2], chanMed[1][1], chanMed[1][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[1][6],
				chanMed[1][7], chanMed[1][8], chanMed[1][9],
				chanMed[1][10], chanMed[1][11], chanMed[1][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// V
		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "V%%|%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[2][5], chanMed[2][4], chanMed[2][3],
				chanMed[2][2], chanMed[2][1], chanMed[2][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[2][6],
				chanMed[2][7], chanMed[2][8], chanMed[2][9],
				chanMed[2][10], chanMed[2][11], chanMed[2][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// T
		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "T%%|%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[3][5], chanMed[3][4], chanMed[3][3],
				chanMed[3][2], chanMed[3][1], chanMed[3][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[3][6],
				chanMed[3][7], chanMed[3][8], chanMed[3][9],
				chanMed[3][10], chanMed[3][11], chanMed[3][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// A
		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "A%%|%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[0][5], chanMed[0][4], chanMed[0][3],
				chanMed[0][2], chanMed[0][1], chanMed[0][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		ByteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[0][6],
				chanMed[0][7], chanMed[0][8], chanMed[0][9],
				chanMed[0][10], chanMed[0][11], chanMed[0][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
		WndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MnStartCal(void)
{
	// Setup Analog controls
	SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1);
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL);

#if 0 // Necessary? Probably need 1 sec for changes, however 1 sec worth of samples thrown away with getting channel offsets 
	// Delay for Analog cutoff and gain select changes to propagate
	SoftUsecWait(50 * SOFT_MSECS);
#endif

	// Enable the A/D
	debug("Enable the A/D\n");
	PowerControl(ANALOG_SLEEP_ENABLE, OFF);

	// Delay to allow AD to power up/stabilize
	SoftUsecWait(50 * SOFT_MSECS);

	// Setup AD Channel config
	SetupADChannelConfig(CALIBRATION_FIXED_SAMPLE_RATE);

#if 0 // Now skipped since the default menu display (CAL_MENU_DEFAULT_NON_CALIBRATED_DISPLAY) does not use channel offsets
	// Get channel offsets
	GetChannelOffsets(CALIBRATION_FIXED_SAMPLE_RATE);
#endif

#if INTERNAL_SAMPLING_SOURCE
	// Setup ISR to clock the data sampling
	Setup_8100_TC_Clock_ISR(CALIBRATION_FIXED_SAMPLE_RATE, TC_CALIBRATION_TIMER_CHANNEL);

	// Start the timer for collecting data
	Start_Data_Clock(TC_CALIBRATION_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
	StartExternalRtcClock(SAMPLE_RATE_1K);
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void MnStopCal(void)
{
#if INTERNAL_SAMPLING_SOURCE
	Stop_Data_Clock(TC_CALIBRATION_TIMER_CHANNEL);
#elif EXTERNAL_SAMPLING_SOURCE
	StopExternalRtcClock();
#endif

	PowerControl(ANALOG_SLEEP_ENABLE, ON);		
}
