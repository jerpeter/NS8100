///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved
///
///	$RCSfile: Keypad.c,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/Keypad.c,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Keypad.h"
#include "Menu.h"
#include "Rec.h"
#include "Common.h"
#include "Typedefs.h"
#include "Old_Board.h"
#include "SoftTimer.h"
#include "Display.h"
#include "PowerManagement.h"
#include "Uart.h"
#include "SysEvents.h"
#include "TextTypes.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define KEYPAD_ACCESS_DELAY 		125	// In usecs
#define CHECK_FOR_REPEAT_KEY_DELAY 	100	// 100 * 10 msec ticks = 1 sec
#define CHECK_FOR_COMBO_KEY_DELAY 	2	// 2 * 10 msec ticks = 20 mssec
#define WAIT_AFTER_COMBO_KEY_DELAY 	25	// 25 * 10 msec ticks = 250 mssec
#define REPEAT_DELAY 				10	// 10 * 10 msec ticks = 100 msecs

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern uint8 contrast_value;
extern uint8 g_sampleProcessing;
extern REC_HELP_MN_STRUCT help_rec;
extern SYS_EVENT_STRUCT SysEvents_flags;
extern int active_menu;
extern void (*menufunc_ptrs[]) (INPUT_MSG_STRUCT);
extern uint8 g_disableDebugPrinting;

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
#if SUPERGRAPH_UNIT // keypad key layout
uint8 keyTbl[8][8] = {
/*R0*/ {KEY_NONE, 	KEY_NONE,  	KEY_NONE, 	KEY_NONE,      	KEY_NONE,    	KEY_NONE,   KEY_NONE, 		KEY_NONE},
/*R1*/ {KEY_ENTER,	KEY_PLUS, 	KEY_MINUS, 	KEY_DOWNARROW, 	KEY_UPARROW, 	KEY_ESCAPE, KEY_HELP, 		KEY_BACKLIGHT},
/*R2*/ {'8',		'7', 		'6', 		'5', 			'4', 			'3', 		'2', 			'1'},
/*R3*/ {'0',		'9', 		'Y', 		'T', 			'R',			'E', 		'W', 			'Q'},
/*R4*/ {'P', 		'O', 		'I', 		'U', 			'F', 			'D', 		'S', 			'A'},
/*R5*/ {KEY_RETURN,	'L', 		'K', 		'J', 			'H', 			'G', 		'Z', 			KEY_SHIFT},
/*R6*/ {KEY_DELETE,	'.', 		'M', 		'N', 			'B', 			'V', 		'C',			'X'},
/*R7*/ {' ', 		KEY_NONE, 	KEY_NONE, 	KEY_NONE, 		KEY_NONE, 		KEY_NONE, 	KEY_PAPERFEED, 	KEY_CTRL}
};
#else // MINI keypad key layout
uint8 keyTbl[8][8] = {
//{KEY_BACKLIGHT, KEY_HELP, KEY_ESCAPE, KEY_UPARROW, KEY_DOWNARROW, KEY_MINUS, KEY_PLUS, KEY_ENTER}
{KEY_BACKLIGHT, KEY_HELP, KEY_ESCAPE, KEY_UPARROW, KEY_DOWNARROW, KEY_MINUS, KEY_PLUS, KEY_ENTER}
};
#endif

#if 0
uint8 keyTbl[8][8] = {
/*R0*/ {KEY_NONE,		KEY_NONE,	 KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R1*/ {KEY_BACKLIGHT,	KEY_HELP, 	 KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R2*/ {KEY_ESCAPE, 	KEY_UPARROW, KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R3*/ {KEY_DOWNARROW,	KEY_MINUS, 	 KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R4*/ {KEY_PLUS,		KEY_ENTER,	 KEY_NONE, 	KEY_NONE, 	KEY_NONE,	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R5*/ {KEY_NONE,		KEY_NONE,	 KEY_NONE, 	KEY_NONE, 	KEY_NONE,	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R6*/ {KEY_NONE,		KEY_NONE,	 KEY_NONE, 	KEY_NONE, 	KEY_NONE,	KEY_NONE, 	KEY_NONE, 	KEY_NONE},
/*R7*/ {KEY_NONE,		KEY_NONE,	 KEY_NONE, 	KEY_NONE, 	KEY_NONE,	KEY_NONE, 	KEY_NONE, 	KEY_NONE}
};
#endif

uint32 num_speed = 1;
uint32 specialKey;
uint32 fixed_special_speed;
uint8 g_lcdBacklightFlag  = ENABLED;
uint8 g_lcdPowerFlag = ENABLED;
uint8 g_kpadProcessingFlag = DEACTIVATED;
uint8 g_kpadCheckForKeyFlag = DEACTIVATED;
uint8 g_factorySetupSequence = SEQ_NOT_STARTED;
uint8 g_kpadLastKeyPressed = 0;
volatile uint32 g_keypadTimerTicks = 0;
uint32 g_kpadKeyRepeatCount = 0;
uint32 g_kpadLookForKeyTickCount = 0;
uint8 keyMap[8];

/****************************************
*	Function:	isr_keypad
*	Purpose:
****************************************/
#include "M23018.h"
BOOL keypad(void)
{
	INPUT_MSG_STRUCT msg;
	INPUT_MSG_STRUCT* p_msg = &msg;

	//volatile uint8* keypadAddress = (uint8*)KEYPAD_ADDRESS;

	uint8 rowMask = 0;
	uint8 keyPressed = KEY_NONE;
	uint8 numKeysPressed = 0;
	uint8 i = 0;
	//uint8 j = 0;
	uint8 ctrlKeyPressed = NO;
	uint8 shiftKeyPressed = NO;

	static uint32 lookForComboKeyRepeatTickCount = 0;

	// Prevents the interrupt routine from setting the system keypress flag.
	g_kpadProcessingFlag = ACTIVATED;
	// Set flag to run keypad again to check for repeating keys or ctrl/shift key combos
	g_kpadCheckForKeyFlag = ACTIVATED;

	// Check if debug printing has been turned off
	if (g_disableDebugPrinting)
	{
		// Wait to make sure key is still depressed (not a bouncing signal on release)
		soft_usecWait(3 * SOFT_MSECS);
	}

	// Start the PIT timer
	if (checkPitTimer(KEYPAD_TIMER) == DISABLED)
		startPitTimer(KEYPAD_TIMER);

	// Read the keys
	//keyMap[0] = (uint8)(~(*keypadAddress));

	// fix_ns8100
	keyMap[0] = read_mcp23018(IO_ADDRESS_KPD, GPIOB);

	debug("Key Pressed: %x\n", keyMap[0]);

	// Re-read keys and mask in to catch signal bouncing
	//keyMap[0] &= *keypadAddress;

	if (SUPERGRAPH_UNIT)
	{
		// Check for ctrl key, row 7 column 7 (zero based)
		if (keyMap[7] & 0x80)
		{
			ctrlKeyPressed = YES;
			numKeysPressed++;

			// Clear the ctrl key from the map
			keyMap[7] &= ~0x80;
		}

		// Check for shift key, row 5 column 7 (zero based)
		if (keyMap[5] & 0x80)
		{
			shiftKeyPressed = YES;
			numKeysPressed++;

			// Clear the shift key from the map
			keyMap[5] &= ~0x80;
		}
	}

	// Find keys by locating the 1's in the map
	for (rowMask = 0x01, i = 0; i < TOTAL_KEYPAD_KEYS; i++)
	{
		if (keyMap[0] & rowMask)
		{
			//debug("Key Found: Row:%d, value is 0x%x\n", i, keyTbl[0][i]);

			keyPressed = keyTbl[0][i];
			numKeysPressed++;
		}

		rowMask <<= 1;
	}

	keyMap[0] = 0x00;

	// Check if we found any keys
	if (numKeysPressed == 0)
	{
		// Done looking for keys
		g_kpadCheckForKeyFlag = DEACTIVATED;
		g_kpadProcessingFlag = DEACTIVATED;

		// Done looking fort keys, disable the PIT timer
		stopPitTimer(KEYPAD_TIMER);

		return(PASSED);
	}

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

	// Check if the same key is still being pressed
	if ((keyPressed != KEY_NONE) && (g_kpadLastKeyPressed == keyPressed))
	{
		// Process repeating key

		// Set delay for 100ms before looking for key again
		g_kpadLookForKeyTickCount = REPEAT_DELAY + g_keypadTimerTicks;

		// The following determines the scrolling adjustment magnitude
		// which is determined by multiplying adjustment by num_speed.
		g_kpadKeyRepeatCount++;

		if (g_kpadKeyRepeatCount < 10)      { num_speed = 1; }
		else if (g_kpadKeyRepeatCount < 20) { num_speed = 10; }
		else if (g_kpadKeyRepeatCount < 30) { num_speed = 50; }
		else if (g_kpadKeyRepeatCount < 40) { num_speed = 100; }
		else if (g_kpadKeyRepeatCount < 50) { num_speed = 1000; }
		else
		{
			fixed_special_speed = 10000;
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
				p_msg->data[0] = getShiftChar(keyPressed);
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
		sendInputMsg(p_msg);

		// Done processing repeating key
	}
	else // New key pressed
	{
		// Store current key for later comparison
		g_kpadLastKeyPressed = keyPressed;

		// Reset variables
		num_speed = 1;
		g_kpadKeyRepeatCount = 0;
		fixed_special_speed = 0;

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

		// Process new key
		if (keyPressed != KEY_NONE)
		{
			if (keyPressed == KEY_BACKLIGHT)
			{
				p_msg->cmd = BACK_LIGHT_CMD;
				p_msg->length = 0;
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
						p_msg->data[0] = getShiftChar(keyPressed);
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
				if (g_sampleProcessing == SAMPLING_STATE)
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
#if 0 // fix_ns8100
				// Check if the ON key is currently being pressed at the same time the enter key was detected
				if ((reg_EPPDR.bit.EPPD0 == 0x00) && (keyPressed == ENTER_KEY))
				{
					// Reset the factory setup process
					g_factorySetupSequence = SEQ_NOT_STARTED;

					// Issue a Ctrl-C for Manual Calibration
					p_msg->cmd = CTRL_CMD;
					p_msg->data[0] = 'C';
				}
				else if (g_factorySetupSequence != PROCESS_FACTORY_SETUP)
				{
					// Reset the sequence
					g_factorySetupSequence = SEQ_NOT_STARTED;
				}
#endif
				// Enqueue the message
				sendInputMsg(p_msg);
			}
		}
	}

	g_kpadProcessingFlag = DEACTIVATED;
	return(PASSED);
}

/****************************************
*	Function:	keypressEventMgr
*	Purpose:
****************************************/
void keypressEventMgr(void)
{
	// Check if the LCD Power was turned off
	if (g_lcdPowerFlag == DISABLED)
	{
		g_lcdPowerFlag = ENABLED;
		raiseSystemEventFlag(UPDATE_MENU_EVENT);
		setLcdContrast(contrast_value);
		powerControl(LCD_POWER_ENABLE, ON);
		initLcdDisplay();					// Setup LCD segments and clear display buffer
		assignSoftTimer(LCD_PW_ON_OFF_TIMER_NUM, (uint32)(help_rec.lcd_timeout * TICKS_PER_MIN), lcdPwTimerCallBack);

		// Check if the unit is monitoring, if so, reassign the monitor update timer
		if (g_sampleProcessing == SAMPLING_STATE)
		{
			debug("Keypress Timer Mgr: enabling Monitor Update Timer.\n");
			assignSoftTimer(MENU_UPDATE_TIMER_NUM, ONE_SECOND_TIMEOUT, menuUpdateTimerCallBack);
		}
	}
	else // Reassign the LCD Power countdown timer
	{
		assignSoftTimer(LCD_PW_ON_OFF_TIMER_NUM, (uint32)(help_rec.lcd_timeout * TICKS_PER_MIN), lcdPwTimerCallBack);
	}

	// Check if the LCD Backlight was turned off
	if (g_lcdBacklightFlag == DISABLED)
	{
		g_lcdBacklightFlag = ENABLED;
		setLcdBacklightState(BACKLIGHT_DIM);
		assignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, displayTimerCallBack);
	}
	else // Reassign the LCD Backlight countdown timer
	{
		assignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, displayTimerCallBack);
	}

	// Check if Auto Monitor is active and not in monitor mode
	if ((help_rec.auto_monitor_mode != AUTO_NO_TIMEOUT) && (g_sampleProcessing != SAMPLING_STATE))
	{
		assignSoftTimer(AUTO_MONITOR_TIMER_NUM, (uint32)(help_rec.auto_monitor_mode * TICKS_PER_MIN), autoMonitorTimerCallBack);
	}
}

/****************************************
*	Function:	getShiftChar
*	Purpose:	If in here, we want the shifted char if available.
*
*	ProtoType:	void getShiftChar (uint8)
*	Input:
*	Output:
****************************************/
uint8 getShiftChar(uint8 inputChar)
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

/****************************************
*	Function:	handleCtrlKeyCombination
*	Purpose:	handle ctrl-keys
****************************************/
uint8 handleCtrlKeyCombination(uint8 inputChar)
{
	switch (inputChar)
	{
		case ESC_KEY:
		break;

		case 'C':
			{
				handleManualCalibration();
			}
		break;

		default:
			break;
	}

	return (KEY_NONE);
}

/****************************************
*	Function:	initKeypad
*	Purpose:	Initialize the keypad IO device
****************************************/
void initKeypad(void)
{
#if 0 // fix_ns8100
	uint8* keypadAddress = (uint8*)KEYPAD_ADDRESS;

	// Activate all the keypad rows
	if (*keypadAddress != 0xFF)
	{
		debugErr("A Keypad key is stuck or broken (0x%x)\n", (uint8)(~(*keypadAddress)));
	}
#endif
}

/****************************************
*	Function:	getKeypadKey
*	Purpose:	Returns the active key on the keypad
****************************************/
uint8 getKeypadKey(uint8 mode)
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
		keyPressed = scanKeypad();
		// If there is a key, wait until it's depressed
		while (keyPressed != KEY_NONE)
		{
			soft_usecWait(1000);
			keyPressed = scanKeypad();
		}

		// Wait for a key to be pressed
		keyPressed = scanKeypad();
		while (keyPressed == KEY_NONE)
		{
			soft_usecWait(1000);
			keyPressed = scanKeypad();
		}

		// Wait for a key to be pressed
		while (scanKeypad() != KEY_NONE)
		{
			soft_usecWait(1000);
		}
	}
	else // mode = CHECK_ONCE_FOR_KEY
	{
		// Check once if there is a key depressed
		keyPressed = scanKeypad();

		if (keyPressed == KEY_NONE)
		{
			soft_usecWait(1000);

			g_kpadProcessingFlag = DEACTIVATED;
			clearSystemEventFlag(KEYPAD_EVENT);
			return (0);
		}
	}

	soft_usecWait(1000);

	// Reassign the LCD Power countdown timer
	assignSoftTimer(LCD_PW_ON_OFF_TIMER_NUM, (uint32)(help_rec.lcd_timeout * TICKS_PER_MIN), lcdPwTimerCallBack);

	// Reassign the LCD Backlight countdown timer
	assignSoftTimer(DISPLAY_ON_OFF_TIMER_NUM, LCD_BACKLIGHT_TIMEOUT, displayTimerCallBack);

	g_kpadProcessingFlag = DEACTIVATED;

	// Prevent a bouncing key from causing any action after this
	clearSystemEventFlag(KEYPAD_EVENT);

	return (keyPressed);
}

/****************************************
*	Function:	scanKeypad
*	Purpose:	Returns the active key on the keypad
****************************************/
#include "Mmc2114_Registers.h"
uint8 messageBoxKeyInput = 0;
extern uint8 messageBoxActiveFlag;
uint8 scanKeypad(void)
{
	//volatile uint8* keypadAddress = (uint8*)KEYPAD_ADDRESS;

	uint8 rowMask = 0;
	uint8 keyPressed = KEY_NONE;
	uint8 i = 0;

	//keyMap[0] = (uint8)(~(*keypadAddress));
	keyMap[0] = read_mcp23018(IO_ADDRESS_KPD, GPIOB);

	//debug("Scan Keypad: Key: %x\n", keyMap[0]);

	// Find keys by locating the 1's in the map
	for (rowMask = 0x01, i = 0; i < TOTAL_KEYPAD_KEYS; i++)
	{
		if (keyMap[0] & rowMask)
		{
			keyPressed = keyTbl[0][i];
		}

		rowMask <<= 1;
	}

	if (messageBoxActiveFlag == YES)
	{
		keyPressed = messageBoxKeyInput;
	}

	return (keyPressed);
}
