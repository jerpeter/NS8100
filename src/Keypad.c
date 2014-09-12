///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Keypad.h"
#include "Menu.h"
#include "Record.h"
#include "Common.h"
#include "Typedefs.h"
#include "SoftTimer.h"
#include "Display.h"
#include "PowerManagement.h"
#include "Uart.h"
#include "SysEvents.h"
#include "TextTypes.h"
#include "M23018.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define CHECK_FOR_REPEAT_KEY_DELAY 	750		// 750 * 1 msec ticks = 750 msecs
#define CHECK_FOR_COMBO_KEY_DELAY 	20		// 20 * 1 msec ticks = 20 mssec
#define WAIT_AFTER_COMBO_KEY_DELAY 	250		// 250 * 1 msec ticks = 250 mssec
#define REPEAT_DELAY 				100		// 100 * 1 msec ticks = 100 msecs
#define DEBOUNCE_READS 				15		// 100 * 1 msec ticks = 100 msecs

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern void Start_Data_Clock(TC_CHANNEL_NUM);
extern void Stop_Data_Clock(TC_CHANNEL_NUM);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------
static uint32 s_fixedSpecialSpeed;
static uint8 s_keyMap[8];

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void CycleSleepMode(void)
{
	if (g_sleepModeState == AVR32_PM_SMODE_STOP) { g_sleepModeState = AVR32_PM_SMODE_IDLE; debug("Sleep mode level is now: Idle\n"); }
	else if (g_sleepModeState == AVR32_PM_SMODE_IDLE) { g_sleepModeState = AVR32_PM_SMODE_FROZEN; debug("Sleep mode level is now: Frozen\n"); }
	else if (g_sleepModeState == AVR32_PM_SMODE_FROZEN) { g_sleepModeState = AVR32_PM_SMODE_STANDBY; debug("Sleep mode level is now: Standby\n"); }
	else if (g_sleepModeState == AVR32_PM_SMODE_STANDBY) { g_sleepModeState = AVR32_PM_SMODE_STOP; debug("Sleep mode level is now: Stop\n"); }
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
BOOLEAN KeypadProcessing(uint8 keySource)
{
	INPUT_MSG_STRUCT msg;
	INPUT_MSG_STRUCT* p_msg = &msg;
	uint8 rowMask = 0;
	uint8 keyPressed = KEY_NONE;
	uint8 numKeysPressed = 0;
	uint8 i = 0;
	uint8 ctrlKeyPressed = NO;
	uint8 shiftKeyPressed = NO;

	static uint32 lookForComboKeyRepeatTickCount = 0;

	// Prevents the interrupt routine from setting the system keypress flag.
	g_kpadProcessingFlag = ACTIVATED;
	// Set flag to run keypad again to check for repeating keys or ctrl/shift key combos
	g_kpadCheckForKeyFlag = ACTIVATED;

	// Check if the key timer has already been started and key processing source is a key interrupt
	if ((keySource == KEY_SOURCE_IRQ) && (g_tcTypematicTimerActive == YES))
	{
#if 0 // Test
		// Dealing with a key release, need to check for a debouncing key
		s_keyMap[1] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
		SoftUsecWait(5 * SOFT_MSECS);
		s_keyMap[2] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
		SoftUsecWait(5 * SOFT_MSECS);
		s_keyMap[3] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	
		// Logic AND all the keypad reads to filter any signal flopping
		s_keyMap[0] = (s_keyMap[1] & s_keyMap[2] & s_keyMap[3]);
#else
		s_keyMap[0] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);

		if (s_keyMap[0])
		{
			for (i = 0; i < DEBOUNCE_READS; i++)
			{
				s_keyMap[0] &= ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
				SoftUsecWait(i * 50);
			}
		}
#endif
	}
	else // Keypad interrupt and key timer hasn't been started, or key processing source was the timer
	{
		// Key is being pressed, and only one read should be sufficient for detecting the key
		s_keyMap[0] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	}

	if (s_keyMap[0]) { debugRaw(" (Key Pressed: %x)", s_keyMap[0]); }
	else { debugRaw(" (Key Release)", s_keyMap[0]); }

	//---------------------------------------------------------------------------------
	// Find key
	//---------------------------------------------------------------------------------
	// Find keys by locating the 1's in the map
	for (rowMask = 0x01, i = 0; i < TOTAL_KEYPAD_KEYS; i++)
	{
		if (s_keyMap[0] & rowMask)
		{
			//debug("Key Found: Row:%d, value is 0x%x\n", i, g_keypadTable[0][i]);

			keyPressed = g_keypadTable[0][i];
			numKeysPressed++;
		}

		rowMask <<= 1;
	}

	s_keyMap[0] = 0x00;

	// Check if any keys were discovered
	if (numKeysPressed == 0)
	{
		// Done looking for keys
		g_kpadProcessingFlag = DEACTIVATED;
		g_kpadCheckForKeyFlag = DEACTIVATED;

		// Clear last key pressed
		g_kpadLastKeyPressed = KEY_NONE;

		while (g_kpadInterruptWhileProcessing == YES)
		{
			g_kpadInterruptWhileProcessing = NO;
			ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
		}

		// Disable the key timer
		Stop_Data_Clock(TC_TYPEMATIC_TIMER_CHANNEL);

		// No keys detected, done processing
		return(PASSED);
	}

	// Key detected, begin higher level processing
	// Check if the key timer hasn't been activated already
	if (g_tcTypematicTimerActive == NO)
	{
		// Start the key timer
		Start_Data_Clock(TC_TYPEMATIC_TIMER_CHANNEL);
	}

#if 0
	if (SUPERGRAPH_UNIT)
	{
		// Check if multiple keys were found
		if (numKeysPressed > 1)
		{
			if (ctrlKeyPressed == YES)
				debug("Keypad: Handling ctrl key combination\n");
			if (shiftKeyPressed == YES)
				debug("Keypad: Handling shift key combination\n");

			// Check if processing a combo key for the second time (under a second)
			if (((shiftKeyPressed == YES) || (ctrlKeyPressed == YES)) && (keyPressed != KEY_NONE))
			{
				// Check if the 1 second repeat delay hasn't been meet
				if (lookForComboKeyRepeatTickCount > g_keypadTimerTicks)
				{
					// Reset delay to 20 msecs before looking again
					g_kpadLookForKeyTickCount = CHECK_FOR_COMBO_KEY_DELAY + g_keypadTimerTicks;

					// Finished processing
					return (PASSED);
				}
			}
		}
	}
#endif

	// Check specific condition after a combo sequence where the special key was released before the regular key
	if ((numKeysPressed == 1) && (g_kpadLastKeyPressed == keyPressed) &&
		(shiftKeyPressed == NO) && (ctrlKeyPressed == NO))
	{
		// Check if a combination sequence was completed less than one second from start before this
		if (lookForComboKeyRepeatTickCount > g_keypadTimerTicks)
		{
			// Prevent condition from occurring twice
			lookForComboKeyRepeatTickCount = g_keypadTimerTicks;

			// Reset delay to 250 msecs before looking again
			g_kpadLookForKeyTickCount = WAIT_AFTER_COMBO_KEY_DELAY + g_keypadTimerTicks;

			// Finished processing
			return (PASSED);
		}
	}

	//---------------------------------------------------------------------------------
	// Process repeating key
	//---------------------------------------------------------------------------------
	// Check if the same key is still being pressed
	if ((keyPressed != KEY_NONE) && (g_kpadLastKeyPressed == keyPressed))
	{
		// Process repeating key

		// Set delay before looking for key again
		g_kpadLookForKeyTickCount = REPEAT_DELAY + g_keypadTimerTicks;

		// The following determines the scrolling adjustment magnitude
		// which is determined by multiplying adjustment by g_keypadNumberSpeed.
		g_kpadKeyRepeatCount++;

		if (g_kpadKeyRepeatCount < 10)      { g_keypadNumberSpeed = 1; }
		else if (g_kpadKeyRepeatCount < 20) { g_keypadNumberSpeed = 10; }
		else if (g_kpadKeyRepeatCount < 30) { g_keypadNumberSpeed = 50; }
		else if (g_kpadKeyRepeatCount < 40) { g_keypadNumberSpeed = 100; }
		else if (g_kpadKeyRepeatCount < 50) { g_keypadNumberSpeed = 1000; }
		else
		{
			s_fixedSpecialSpeed = 10000;
		}

		if (SUPERGRAPH_UNIT)
		{
			p_msg->length = 1;

			if (ctrlKeyPressed == ON)
			{
				p_msg->cmd = CTRL_CMD;
			}
			else
			{
				p_msg->cmd = KEYPRESS_MENU_CMD;
			}

			if (shiftKeyPressed == ON)
			{
				p_msg->data[0] = GetShiftChar(keyPressed);
				//debug("Keypad: Shifted key: %c\n", (char)p_msg->data[0]);
			}
			else
			{
				p_msg->data[0] = keyPressed;
			}
		}
		else // MINIGRAPH_UNIT
		{
			p_msg->length = 1;
			p_msg->cmd = KEYPRESS_MENU_CMD;
			p_msg->data[0] = keyPressed;
		}

		if (g_factorySetupSequence != PROCESS_FACTORY_SETUP)
		{
			// Reset the sequence
			g_factorySetupSequence = SEQ_NOT_STARTED;
		}

		// Enqueue the message
		SendInputMsg(p_msg);

		// Done processing repeating key
	}
	//---------------------------------------------------------------------------------
	// Process new key
	//---------------------------------------------------------------------------------
	else // New key pressed
	{
		// Store current key for later comparison
		g_kpadLastKeyPressed = keyPressed;

		// Reset variables
		g_keypadNumberSpeed = 1;
		g_kpadKeyRepeatCount = 0;
		s_fixedSpecialSpeed = 0;

		if (SUPERGRAPH_UNIT)
		{
			// Check if we are dealing with a combination sequence
			if ((ctrlKeyPressed == YES) || (shiftKeyPressed == YES))
			{
				// Set delay to 20 msecs before looking to possibly complete the combination key capture
				g_kpadLookForKeyTickCount = CHECK_FOR_COMBO_KEY_DELAY + g_keypadTimerTicks;

				// Check if still waiting for a key to be pressed
				if (keyPressed == KEY_NONE)
				{
					// Reset combination sequence repeat delay
					lookForComboKeyRepeatTickCount = g_keypadTimerTicks;
				}
				else // completed combination key sequence
				{
					// Set delay to handle waiting 1 sec before considering a combination sequence repeating
					lookForComboKeyRepeatTickCount = CHECK_FOR_REPEAT_KEY_DELAY + g_keypadTimerTicks;
				}
			}
			else
			{
				// Set delay for 1 sec before considering key repeating
				g_kpadLookForKeyTickCount = CHECK_FOR_REPEAT_KEY_DELAY + g_keypadTimerTicks;
			}
		}
		else
		{
			// Set delay for 1 sec before considering key repeating
			g_kpadLookForKeyTickCount = CHECK_FOR_REPEAT_KEY_DELAY + g_keypadTimerTicks;
		}

		//---------------------------------------------------------------------------------
		// Send new key message for action
		//---------------------------------------------------------------------------------
		// Process new key
		if (keyPressed != KEY_NONE)
		{
			if (keyPressed == KEY_BACKLIGHT)
			{
#if 0 // Test (Keypad override of the backlight key for testing)
				if (g_sampleProcessing == ACTIVE_STATE)
				{
					// Check if not processing an event
					if (g_busyProcessingEvent == NO)
					{
						// Signal the start of an event
						g_externalTrigger = YES;
					}
				}
				else
				{
					//RTC_MEM_MAP_STRUCT rtcMap;
					//ExternalRtcRead(RTC_CONTROL_1_ADDR, 2, &rtcMap.control_1);
					//debug("RTC Control 1: 0x%x, Control 2: 0x%x\n", rtcMap.control_1, rtcMap.control_2);

					//__monitorLogTblKey = 0;
					//InitMonitorLog();
					
					//g_triggerRecord.trec.sample_rate += 1024;
					//debug("New sample rate: %d\n", g_triggerRecord.trec.sample_rate);
				}
				
				p_msg->cmd = 0;
				p_msg->length = 0;
#else
				p_msg->cmd = BACK_LIGHT_CMD;
				p_msg->length = 0;
#endif
			}
			else // all other keys
			{
				p_msg->length = 1;

				if (SUPERGRAPH_UNIT)
				{
					if (ctrlKeyPressed == ON)
					{
						p_msg->cmd = CTRL_CMD;
					}
					else
					{
						p_msg->cmd = KEYPRESS_MENU_CMD;
					}

					if (shiftKeyPressed == ON)
					{
						p_msg->data[0] = GetShiftChar(keyPressed);
						//debug("Keypad: Shifted key: %c\n", (char)p_msg->data[0]);
					}
					else
					{
						p_msg->data[0] = keyPressed;
					}
				}
				else
				{
					p_msg->data[0] = keyPressed;
					p_msg->cmd = KEYPRESS_MENU_CMD;
				}
			}

			//---------------------------------------------------------------------------------
			// Factory setup staging
			//---------------------------------------------------------------------------------
			// Handle factory setup special sequence
			if ((g_factorySetupSequence == STAGE_1) && ((SUPERGRAPH_UNIT && (keyPressed == 'F')) ||
				(keyPressed == KEY_UPARROW)))
			{
				g_factorySetupSequence = STAGE_2;
			}
			else if ((g_factorySetupSequence == STAGE_2) && ((SUPERGRAPH_UNIT && (keyPressed == 'S')) ||
				(keyPressed == KEY_DOWNARROW)))
			{
				// Check if actively in Monitor mode
				if (g_sampleProcessing == ACTIVE_STATE)
				{
					// Don't allow access to the factory setup
					g_factorySetupSequence = SEQ_NOT_STARTED;
				}
				else // Not in Monitor mode
				{
					// Allow access to factory setup
					g_factorySetupSequence = ENTER_FACTORY_SETUP;
				}
			}
			else
			{
				// Check if the On key is being pressed
				if (ReadMcp23018(IO_ADDRESS_KPD, GPIOA) & 0x04)
				{
					// Reset the factory setup process
					g_factorySetupSequence = SEQ_NOT_STARTED;

					if (keyPressed == ENTER_KEY)
					{
						// Issue a Ctrl-C for Manual Calibration
						p_msg->cmd = CTRL_CMD;
						p_msg->data[0] = 'C';
					}
#if 1 // Test
					else if (keyPressed == ESC_KEY)
					{
extern uint8 quickBootEntryJump;
extern void BootLoadManager(void);
						if (g_sampleProcessing == IDLE_STATE)
						{
							quickBootEntryJump = YES;
							BootLoadManager();
						}
					}
					else if (keyPressed == HELP_KEY)
					{
#if 1 // Test
						CycleSleepMode();
#endif
					}
					else if (keyPressed == KEY_BACKLIGHT)
					{
						DisplayTimerCallBack();
						LcdPwTimerCallBack();
						
						g_kpadProcessingFlag = DEACTIVATED;

						while (g_kpadInterruptWhileProcessing == YES)
						{
							g_kpadInterruptWhileProcessing = NO;
							ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
						}

						return(PASSED);
					}
#endif
				}
				else if (g_factorySetupSequence != PROCESS_FACTORY_SETUP)
				{
					// Reset the sequence
					g_factorySetupSequence = SEQ_NOT_STARTED;
				}

				// Enqueue the message
				SendInputMsg(p_msg);
			}
		}
	}

	g_kpadProcessingFlag = DEACTIVATED;

	while (g_kpadInterruptWhileProcessing == YES)
	{
		g_kpadInterruptWhileProcessing = NO;
		ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	}

	return(PASSED);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void KeypressEventMgr(void)
{
	// Check if the LCD Power was turned off
	if (g_lcdPowerFlag == DISABLED)
	{
		g_lcdPowerFlag = ENABLED;
		raiseSystemEventFlag(UPDATE_MENU_EVENT);
		SetLcdContrast(g_contrast_value);
		PowerControl(LCD_POWER_ENABLE, ON);
		InitLcdDisplay();					// Setup LCD segments and clear display buffer
		AssignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_helpRecord.lcdTimeout * TICKS_PER_MIN), LcdPwTimerCallBack);

		// Check if the unit is monitoring, if so, reassign the monitor update timer
		if (g_sampleProcessing == ACTIVE_STATE)
		{
			debug("Keypress Timer Mgr: enabling Monitor Update Timer.\n");
			AssignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, MenuUpdateTimerCallBack);
		}
	}
	else // Reassign the LCD Power countdown timer
	{
		AssignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_helpRecord.lcdTimeout * TICKS_PER_MIN), LcdPwTimerCallBack);
	}

	// Check if the LCD Backlight was turned off
	if (g_lcdBacklightFlag == DISABLED)
	{
		g_lcdBacklightFlag = ENABLED;
		SetLcdBacklightState(BACKLIGHT_DIM);
		AssignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, DisplayTimerCallBack);
	}
	else // Reassign the LCD Backlight countdown timer
	{
		AssignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, DisplayTimerCallBack);
	}

	// Check if Auto Monitor is active and not in monitor mode
	if ((g_helpRecord.autoMonitorMode != AUTO_NO_TIMEOUT) && (g_sampleProcessing != ACTIVE_STATE))
	{
		AssignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(g_helpRecord.autoMonitorMode * TICKS_PER_MIN), AutoMonitorTimerCallBack);
	}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 GetShiftChar(uint8 inputChar)
{
	switch (inputChar)
	{
		case '1': return ('?'); break;
		case '2': return ('@'); break;
		case '3': return ('#'); break;
		case '4': return ('/'); break;
		case '5': return ('%'); break;
		case '6': return ('^'); break;
		case '7': return ('&'); break;
		case '8': return ('*'); break;
		case '9': return ('('); break;
		case '0': return (')'); break;
		case 'Q': return (':'); break;
		case 'W': return (','); break;
		case 'E': return ('+'); break;
		case 'I': return ('<'); break;
		case 'O': return ('>'); break;
		case 'P': return ('-'); break;
		case KEY_SHIFT:	return (KEY_NONE); break;
		case KEY_CTRL: return (KEY_NONE); break;
		default:
			break;
	}

	return (inputChar);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 HandleCtrlKeyCombination(uint8 inputChar)
{
	switch (inputChar)
	{
		case ESC_KEY:
		break;

		case 'C':
			{
				HandleManualCalibration();
			}
		break;

		default:
			break;
	}

	return (KEY_NONE);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 GetKeypadKey(uint8 mode)
{
	//uint8 columnSelection = 0;
	//uint8 columnIndex = 0;
	uint8 keyPressed = KEY_NONE;
	//uint8 lookForKey = 1;
	//uint8 foundKeyDepressed = 0;
	//uint8 data = 0;

	g_kpadProcessingFlag = ACTIVATED;

	if (mode == WAIT_FOR_KEY)
	{
		keyPressed = ScanKeypad();
		// If there is a key, wait until it's depressed
		while (keyPressed != KEY_NONE)
		{
			SoftUsecWait(1000);
			keyPressed = ScanKeypad();
		}

		// Wait for a key to be pressed
		keyPressed = ScanKeypad();
		while (keyPressed == KEY_NONE)
		{
			SoftUsecWait(1000);
			keyPressed = ScanKeypad();
		}

		// Wait for a key to be pressed
		while (ScanKeypad() != KEY_NONE)
		{
			SoftUsecWait(1000);
		}
	}
	else // mode = CHECK_ONCE_FOR_KEY
	{
		// Check once if there is a key depressed
		keyPressed = ScanKeypad();

		if (keyPressed == KEY_NONE)
		{
			SoftUsecWait(1000);

			g_kpadProcessingFlag = DEACTIVATED;
			clearSystemEventFlag(KEYPAD_EVENT);

			while (g_kpadInterruptWhileProcessing == YES)
			{
				g_kpadInterruptWhileProcessing = NO;
				ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
			}

			return (0);
		}
	}

	SoftUsecWait(1000);

	// Reassign the LCD Power countdown timer
	AssignSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM, (uint32)(g_helpRecord.lcdTimeout * TICKS_PER_MIN), LcdPwTimerCallBack);

	// Reassign the LCD Backlight countdown timer
	AssignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, DisplayTimerCallBack);

	g_kpadProcessingFlag = DEACTIVATED;

	// Prevent a bouncing key from causing any action after this
	clearSystemEventFlag(KEYPAD_EVENT);

	while (g_kpadInterruptWhileProcessing == YES)
	{
		g_kpadInterruptWhileProcessing = NO;
		ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	}

	return (keyPressed);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
uint8 ScanKeypad(void)
{
	uint8 rowMask = 0;
	uint8 keyPressed = KEY_NONE;
	uint8 i = 0;

	//debug("MB: Reading keypad...\n");

	//SoftUsecWait(250 * SOFT_MSECS);
	
#if 0 // Test (Keypad read before hardware mod in attempt to find a keypress)
	ReadMcp23018(IO_ADDRESS_KPD, INTFB);
#endif

#if 0 // Normal
	s_keyMap[0] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
#else // Test
	s_keyMap[1] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	s_keyMap[2] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	s_keyMap[3] = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	
	s_keyMap[0] = (s_keyMap[1] & s_keyMap[2] & s_keyMap[3]);
#endif

	//debug("Scan Keypad: Key: %x\n", s_keyMap[0]);

	// Find keys by locating the 1's in the map
	for (rowMask = 0x01, i = 0; i < TOTAL_KEYPAD_KEYS; i++)
	{
		if (s_keyMap[0] & rowMask)
		{
			keyPressed = g_keypadTable[0][i];
		}

		rowMask <<= 1;
	}

	return (keyPressed);
}
