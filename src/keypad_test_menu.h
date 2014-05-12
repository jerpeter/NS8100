////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   keypad_test_menu.h                                                       //
//                                                                            //
// AUTHOR:                                                                    //
//                                                                            //
//   Joseph R. Getz                                                           //
//   O'Dell E. Martin                                                         //
//   Benjamin D. Taylor                                                       //
//                                                                            //
// REVISION:                                                                  //
//                                                                            //
//   $Author: jgetz $                                                               //
//   $Date: 2009/11/05 00:19:54 $                                                                 //
//   $Revision: 1.4 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef KEYPAD_TEST_MENU_H_
#define KEYPAD_TEST_MENU_H_


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Global Scope Declarations                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void Keypad_Test_Menu( void );
void Keypad_Exit( void );
void Keypad_Info( void );
void Keypad_Continuous_Read( void );
void Keypad_One_Read( void );
void Keypad_LED_Off( void );
void Keypad_LED_Red( void );
void Keypad_LED_Green( void );

static const unsigned char Keypad_Info_Text[] =
{
"\n\r\n\r"
"   Common (GND)   = JP4 pin 11\n\r"
"   Backlight Key  = JP4 pin 10\n\r"
"   Help Key       = JP4 pin 9\n\r"
"   Escape Key     = JP4 pin 8\n\r"
"   Up Key         = JP4 pin 7\n\r"
"   Down Key       = JP4 pin 6\n\r"
"   Minus Key      = JP4 pin 5\n\r"
"   Plus Key       = JP4 pin 4\n\r"
"   Enter Key      = JP4 pin 3\n\r"
"\n\r"
"   LED Off        = JP4 pin 1 = GND   pin 2 = GND\n\r"
"   Red LED        = JP4 pin 1 = +3VDC pin 2 = GND\n\r"
"   Green LED      = JP4 pin 1 = GND   pin 2 = +3VDC\n\r"
"\n\r"
"\0"
};

static const unsigned char Keypad_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         NS8100 KEYPAD TEST MENU                              บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Keypad Information.\n\r"
"   2) Keypad read continuously.\n\r"
"   3) Keypad read once.\n\r"
"   4) Keypad LED Off.\n\r"
"   5) Keypad LED Red.\n\r"
"   6) Keypad LED Green.\n\r"
"\0"
};


static void (*Keypad_Test_Menu_Functions[])(void) =
{
   Keypad_Exit,
   Keypad_Info,
   Keypad_Continuous_Read,
   Keypad_One_Read,
   Keypad_LED_Off,
   Keypad_LED_Red,
   Keypad_LED_Green
};


#endif //KEYPAD_TEST_MENU_H_
