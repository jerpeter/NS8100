////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   trigger_test_menu.h                                                        //
//                                                                            //
// AUTHOR:                                                                    //
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
//   $Date: 2012/04/26 01:10:07 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_TEST_MENU_H_
#define TRIGGER_TEST_MENU_H_


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
void Trigger_Test_Menu( void );
void Trigger_Exit( void );
void Trigger_Info( void );
void Trigger_In_Menu( void );
void Trigger_Out_Menu( void );

static const unsigned char Trigger_Info_Text[] =
{
"\n\r\n\r"
"   Trigger Output     = PB05\n\r"
"   Trigger Transistor = Q23\n\r"
"   Trigger Output Pin = JP3 pin 1\n\r"
"\n\r"
"   Trigger Input      = GPA3, pin 20 of U27\n\r"
"   Trigger Input Pin  = JP3 pin 1\n\r"
"\n\r"
"\0"
};

static const unsigned char Trigger_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                        NS8100 TRIGGER TEST MENU                              บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Trigger Information.\n\r"
"   2) Triger In Test Menu.\n\r"
"   3) Trigger Out Test Menu.\n\r"
"\0"
};


static void (*Trigger_Test_Menu_Functions[])(void) =
{
   Trigger_Exit,
   Trigger_Info,
   Trigger_In_Menu,
   Trigger_Out_Menu
};


void Trigger_In_Exit(void);
void Trigger_In_Continuous_Read(void);
void Trigger_In_One_Read(void);


static const unsigned char Trigger_In_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         NS8100 TRIGGER IN TEST MENU                          บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Trigger In read continuously.\n\r"
"   2) Trigger In read one time.\n\r"
"\0"
};


static void (*Trigger_In_Menu_Functions[])(void) =
{
   Trigger_In_Exit,
   Trigger_In_Continuous_Read,
   Trigger_In_One_Read
};


void Trigger_Out_Exit(void);
void Trigger_Out_Continuous_Pulse(void);
void Trigger_Out_One_Pulse(void);
void Trigger_Out_Set_Low(void);
void Trigger_Out_Set_High(void);

static const unsigned char Trigger_Out_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                       NS8100 TRIGGER OUTPUT TEST MENU                        บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
   "   0) Exit.\n\r"
   "   1) Trigger Out pulse continuously.\n\r"
   "   2) Trigger Out pulse one time.\n\r"
   "   3) Trigger Out set Low.\n\r"
   "   4) Trigger Out set High.\n\r"
"\0"
};

static void (*Trigger_Out_Menu_Functions[])(void) =
{
    Trigger_Out_Exit,
    Trigger_Out_Continuous_Pulse,
    Trigger_Out_One_Pulse,
    Trigger_Out_Set_Low,
    Trigger_Out_Set_High
};
#endif //TRIGGER_TEST_MENU_H_
