///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: CalibrationSetupMenu.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:46 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/CalibrationSetupMenu.c,v $
///	$Revision: 1.2 $
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

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define CAL_SETUP_MN_TABLE_SIZE 8
#define CAL_SETUP_WND_STARTING_COL DEFAULT_COL_THREE
#define CAL_SETUP_WND_END_COL DEFAULT_END_COL
#define CAL_SETUP_WND_STARTING_ROW DEFAULT_MENU_ROW_ZERO // DEFAULT_MENU_ROW_TWO
#define CAL_SETUP_WND_END_ROW (DEFAULT_MENU_ROW_SEVEN)
#define CAL_SETUP_MN_TBL_START_LINE 0

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
static uint8 s_calDisplayScreen = 0;
static uint32 s_calSavedSampleRate = 0;
static union {
	uint16 chan[4];
	struct {
		uint16 a;
		uint16 r;
		uint16 v;
		uint16 t;
	};
} s_calPreTrigData[256];

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void calSetupMn(INPUT_MSG_STRUCT);
void calSetupMnDsply(WND_LAYOUT_STRUCT*);
void calSetupMnProc(INPUT_MSG_STRUCT, WND_LAYOUT_STRUCT*, MN_LAYOUT_STRUCT*);
void mnStartCal(void);
void mnStopCal(void);

/****************************************
*	Function:	calSetupMn
*	Purpose:
****************************************/
void calSetupMn(INPUT_MSG_STRUCT msg)
{
	static WND_LAYOUT_STRUCT wnd_layout;
	static MN_LAYOUT_STRUCT mn_layout;
	volatile uint32 tempTicks = 0;
	//uint8 cursorLine = 0;
	volatile uint32 key = 0;
	uint8 mbChoice = 0;
	INPUT_MSG_STRUCT mn_msg;
	uint8 previousMode = g_triggerRecord.op_mode;

	uint8 choice = messageBox(getLangText(VERIFY_TEXT), "START CAL DIAGNOSTICS?", MB_YESNO);

	if (choice == MB_FIRST_CHOICE)
	{
		calSetupMnProc(msg, &wnd_layout, &mn_layout);
		s_calDisplayScreen = 0;
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
					updateCurrentTime();

					// Create the Cal Setup menu
					calSetupMnDsply(&wnd_layout);

					writeMapToLcd(g_mmap);

					// Set to current half second
					tempTicks = g_rtcSoftTimerTickCount;
				}

				key = getKeypadKey(CHECK_ONCE_FOR_KEY);

				switch (key)
				{
					case UP_ARROW_KEY:
						s_calDisplayScreen = 0;
						break;
					case DOWN_ARROW_KEY:
						s_calDisplayScreen = 1;
						break;
					case ENTER_KEY :
					case ESC_KEY :
						// Not really used, keys are grabbed in calSetupMn
						mbChoice = messageBox(getLangText(STATUS_TEXT), getLangText(ARE_YOU_DONE_WITH_CAL_SETUP_Q_TEXT), MB_YESNO);
						if ((mbChoice == MB_SECOND_CHOICE) || (mbChoice == MB_NO_ACTION))
						{
							// Dont leave
							key = 0;
						}
						break;
				}
			}

			mnStopCal();
			g_triggerRecord.trec.sample_rate = s_calSavedSampleRate;

			if (messageBox(getLangText(CONFIRM_TEXT), getLangText(DO_YOU_WANT_TO_SAVE_THE_CAL_DATE_Q_TEXT), MB_YESNO) == MB_FIRST_CHOICE)
			{
				// Store Calibration Date
				g_factorySetupRecord.cal_date = getCurrentTime();
			}
		}

		saveRecData(&g_factorySetupRecord, DEFAULT_RECORD, REC_FACTORY_SETUP_TYPE);

		g_factorySetupSequence = SEQ_NOT_STARTED;

		messageBox(getLangText(STATUS_TEXT), getLangText(FACTORY_SETUP_COMPLETE_TEXT), MB_OK);

		// Restore the previous mode
		g_triggerRecord.op_mode = previousMode;

		g_activeMenu = MAIN_MENU;
		ACTIVATE_MENU_MSG();
		(*menufunc_ptrs[g_activeMenu]) (mn_msg);
	}
}

/****************************************
*	Function:	calSetupMnProc
*	Purpose:
****************************************/
void calSetupMnProc(INPUT_MSG_STRUCT msg,
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

			overlayMessage("STATUS", "PLEASE WAIT...", 0);

			// Fool ISR into filling up quarter sec buffer
			s_calSavedSampleRate = g_triggerRecord.trec.sample_rate;
			g_triggerRecord.trec.sample_rate = 1024;
			InitDataBuffs(WAVEFORM_MODE);

			mnStopCal();
			mnStartCal();
			// Allow pre-trigger buffer to fill up
			soft_usecWait(250 * SOFT_MSECS);
		break;

		case (KEYPRESS_MENU_CMD):
			data = msg.data[0];

			switch (data)
			{
				case (ENTER_KEY):
					break;

				case (ESC_KEY):

					mbChoice = messageBox(getLangText(WARNING_TEXT), getLangText(DO_YOU_WANT_TO_LEAVE_CAL_SETUP_MODE_Q_TEXT), MB_YESNO);
					if ((mbChoice == MB_SECOND_CHOICE) || (mbChoice == MB_NO_ACTION))
					{
						// Dont leave, escape to continue
						break;
					}

					ACTIVATE_USER_MENU_MSG(&helpMenu, CONFIG);
					(*menufunc_ptrs[g_activeMenu]) (mn_msg);
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

}

/****************************************
*	Function:	calSetupMnDsply
*	Purpose:
****************************************/
void calSetupMnDsply(WND_LAYOUT_STRUCT *wnd_layout_ptr)
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

	byteSet(&(g_mmap[0][0]), 0, sizeof(g_mmap));

	if (g_sampleProcessing == ACTIVE_STATE)
	{
#if 0 // ns7100
		// If IDLE, wait for IDLE state to finish
		while (ISPI_GetISPI_State() == ISPI_IDLE){ /*spinBar();*/ }

		// If not IDLE, wait for 430 data processing to finish before moving on
		while (ISPI_GetISPI_State() != ISPI_IDLE){ /*spinBar();*/ }
#else // fix_ns8100

#endif
	}

	memcpy(&s_calPreTrigData[0], &g_startOfQuarterSecBuff[0], (256 * 4 * 2));

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
			if (s_calPreTrigData[i].chan[j] < chanMin[j]) chanMin[j] = s_calPreTrigData[i].chan[j];
			if (s_calPreTrigData[i].chan[j] > chanMax[j]) chanMax[j] = s_calPreTrigData[i].chan[j];
			chanAvg[j] += s_calPreTrigData[i].chan[j];

			if ((s_calPreTrigData[i].chan[j] >= (sampleDataMidpoint - 6)) && (s_calPreTrigData[i].chan[j] <= (sampleDataMidpoint + 6)))
			{
				if (chanMed[j][(s_calPreTrigData[i].chan[j] - (sampleDataMidpoint - 6))] < 255)
					chanMed[j][(s_calPreTrigData[i].chan[j] - (sampleDataMidpoint - 6))]++;
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

	if (s_calDisplayScreen == 0)
	{
		// PRINT CAL_SETUP
		byteSet(&buff[0], 0, sizeof(buff));
		length = (uint8)sprintf((char*)buff, "-%s-", getLangText(CAL_SETUP_TEXT));
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// DATE AND TIME
		byteSet(&buff[0], 0, sizeof(buff));
		time = getCurrentTime();
		convertTimeStampToString((char*)buff, &time, REC_DATE_TIME_TYPE);
		length = (uint8)strlen((char*)buff);
		wnd_layout_ptr->curr_col = (uint16)(((wnd_layout_ptr->end_col)/2) - ((length * SIX_COL_SIZE)/2));
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
		wndMpWrtString(buff,wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);

#if 0 // ns7100
		if (g_sampleProcessing != ACTIVE_STATE)
		{
			return;
		}
#endif		

		// PRINT Table separator
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "--------------------");
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);

		// PRINT Table header
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "C|  Min|  Max|  Avg|");
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT,REG_LN);

		// PRINT R,V,T,A last sample value, .1 sec avg, .25 sec avg
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "R|%+5ld|%+5ld|%+5ld|", chanMin[1], chanMax[1], chanAvg[1]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "V|%+5ld|%+5ld|%+5ld|", chanMin[2], chanMax[2], chanAvg[2]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "T|%+5ld|%+5ld|%+5ld|", chanMin[3], chanMax[3], chanAvg[3]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "A|%+5ld|%+5ld|%+5ld|", chanMin[0], chanMax[0], chanAvg[0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);
	}
	else
	{
		// R
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "R |%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[1][5], chanMed[1][4], chanMed[1][3],
				chanMed[1][2], chanMed[1][1], chanMed[1][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ZERO;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[1][6],
				chanMed[1][7], chanMed[1][8], chanMed[1][9],
				chanMed[1][10], chanMed[1][11], chanMed[1][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_ONE;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// V
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "V |%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[2][5], chanMed[2][4], chanMed[2][3],
				chanMed[2][2], chanMed[2][1], chanMed[2][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_TWO;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[2][6],
				chanMed[2][7], chanMed[2][8], chanMed[2][9],
				chanMed[2][10], chanMed[2][11], chanMed[2][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_THREE;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// T
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "T |%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[3][5], chanMed[3][4], chanMed[3][3],
				chanMed[3][2], chanMed[3][1], chanMed[3][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FOUR;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[3][6],
				chanMed[3][7], chanMed[3][8], chanMed[3][9],
				chanMed[3][10], chanMed[3][11], chanMed[3][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_FIVE;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		// A
		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "A |%2d|%2d|%2d|%2d|%2d|%2d",
				chanMed[0][5], chanMed[0][4], chanMed[0][3],
				chanMed[0][2], chanMed[0][1], chanMed[0][0]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SIX;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

		byteSet(&buff[0], 0, sizeof(buff));
		sprintf((char*)buff, "%2d|%2d|%2d|%2d|%2d|%2d|%2d", chanMed[0][6],
				chanMed[0][7], chanMed[0][8], chanMed[0][9],
				chanMed[0][10], chanMed[0][11], chanMed[0][12]);
		wnd_layout_ptr->curr_row = DEFAULT_MENU_ROW_SEVEN;
		wndMpWrtString(buff, wnd_layout_ptr, SIX_BY_EIGHT_FONT, REG_LN);

	}
}

/****************************************
*	Function:  mnStartCal()
*	Purpose:
****************************************/
void mnStartCal(void)
{
	// fix_ns8100
	// Setup AD Channel config
	SetupADChannelConfig(CAL_PULSE_FIXED_SAMPLE_RATE);
	GetChannelOffsets();

	// Setup Analog controls
	SetAnalogCutoffFrequency(ANALOG_CUTOFF_FREQ_1);
	SetSeismicGainSelect(SEISMIC_GAIN_LOW);
	SetAcousticGainSelect(ACOUSTIC_GAIN_NORMAL);

	// Setup ISR to clock the data sampling
	Setup_8100_TC_Clock_ISR(CAL_PULSE_FIXED_SAMPLE_RATE, TC_CALIBRATION_TIMER_CHANNEL);

	// Start the timer for collecting data
	Start_Data_Clock(TC_CALIBRATION_TIMER_CHANNEL);
}

/****************************************
*	Function:  mnStopCal()
*	Purpose:
****************************************/
void mnStopCal(void)
{
#if 0 // ns7100
	if (g_sampleProcessing == ACTIVE_STATE)
	{
		// If IDLE, wait for IDLE state to finish
		while (ISPI_GetISPI_State() == ISPI_IDLE){ /*spinBar()*/;}

		// If not IDLE, wait for 430 data processing to finish before moving on
		while (ISPI_GetISPI_State() != ISPI_IDLE){ /*spinBar()*/;}
	}

	while (ISPI_SendMsg(STOP_TRIGGER_CMD)){;}

	g_sampleProcessing = IDLE_STATE;
#else // fix_ns8100
	Stop_Data_Clock(TC_CALIBRATION_TIMER_CHANNEL);
#endif
}
