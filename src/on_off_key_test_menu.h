////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   on_off_key_test_menu.h                                                   //
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
//   $Date: 2012/04/26 01:10:06 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef ON_OFF_KEY_TEST_MENU_H_
#define ON_OFF_KEY_TEST_MENU_H_


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
void On_Off_Key_Test_Menu(void);
void On_Off_Key_Exit(void);
void On_Off_Key_Info(void);
void On_Off_Key_On_Menu(void);
void On_Off_Key_Off_Menu(void);

static const unsigned char On_Off_Key_Info_Text[] =
{
"\n\r\n\r"
"   Common (+3VDC) = JP4 pin 12\n\r"
"   On Key         = JP4 pin 13\n\r"
"   Off Key        = JP4 pin 14\n\r"
"\n\r"
"\0"
};

const unsigned char On_Off_Key_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         NS8100 ON/OFF KEY TEST MENU                          บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) On/Off Key Information.\n\r"
"   2) On Key Test Menu.\n\r"
"   3) Off Key Test Menu.\n\r"
"\0"
};

static void (*On_Off_Key_Test_Menu_Functions[])(void) =
{
   On_Off_Key_Exit,
   On_Off_Key_Info,
   On_Off_Key_On_Menu,
   On_Off_Key_Off_Menu
};

void On_Off_Key_On_Menu_Exit(void);
void On_Off_Key_On_Menu_Read_Continuous(void);
void On_Off_Key_On_Menu_Read_Once(void);

const unsigned char On_Off_Key_On_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 ON KEY TEST MENU                            บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) On Key read continuously.\n\r"
"   2) On Key read once.\n\r"
"\0"
};

static void (*On_Off_Key_On_Test_Menu_Functions[])(void) =
{
   On_Off_Key_On_Menu_Exit,
   On_Off_Key_On_Menu_Read_Continuous,
   On_Off_Key_On_Menu_Read_Once
};

void On_Off_Key_Off_Menu_Exit(void);
void On_Off_Key_Off_Menu_Read_Continuous(void);
void On_Off_Key_Off_Menu_Read_Once(void);

const unsigned char On_Off_Key_Off_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 OFF KEY TEST MENU                           บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Off Key read continuously.\n\r"
"   2) Off Key read once.\n\r"
"\0"
};

static void (*On_Off_Key_Off_Test_Menu_Functions[])(void) =
{
   On_Off_Key_Off_Menu_Exit,
   On_Off_Key_Off_Menu_Read_Continuous,
   On_Off_Key_Off_Menu_Read_Once
};

#endif //ON_OFF_KEY_TEST_MENU_H_

#endif
