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
#ifndef PROC_TIMERS_H_
#define PROC_TIMERS_H_


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
void Proc_Timers_Test_Menu(void);
void Proc_Timers_Exit(void);
void Proc_Timers_PiT_1(void);
void Proc_Timers_PiT_2(void);
void Proc_Timers_Test(void);

const unsigned char Proc_Timers_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         NS8100 PROC TIMERS MENU                              บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Test PiT 1.\n\r"
"   2) Test PiT 2.\n\r"
"   3) Test.\n\r"
"\0"
};

static void (*Proc_Timers_Test_Menu_Functions[])(void) =
{
   Proc_Timers_Exit,
   Proc_Timers_PiT_1,
   Proc_Timers_PiT_2,
   Proc_Timers_Test
};


#endif //ON_OFF_KEY_TEST_MENU_H_
