////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   alarm_test_menu.h                                                        //
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
//   $Date: 2012/04/26 01:10:04 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#if 0
#ifndef ALARM_TEST_MENU_H_
#define ALARM_TEST_MENU_H_


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define  ALARM_1_PIN       AVR32_PIN_PB06
#define  ALARM_2_PIN       AVR32_PIN_PB07

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Global Scope Declarations                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void Alarm_Test_Menu( void );
void Alarm_Exit( void );
void Alarm_Info( void );
void Alarm_Output_1_Menu( void );
void Alarm_Output_2_Menu( void );

static const unsigned char Alarm_Info_Text[] =
{
"\n\r\n\r"
"   Alarm 1 Output     = PB06\n\r"
"   Alarm 1 Transistor = Q22\n\r"
"   Alarm 1 Output Pin = JP3 pin 8\n\r"
"\n\r"
"   Alarm 2 Output     = PB07\n\r"
"   Alarm 2 Transistor = Q21\n\r"
"   Alarm 2 Output Pin = JP3 pin 10\n\r"
"\n\r"
"\0"
};

static const unsigned char Alarm_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         NS8100 ALARM TEST MENU                               บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Alarm Information.\n\r"
"   2) Alarm Output 1 Test.\n\r"
"   3) Alarm Output 2 Test.\n\r"
"\0"
};

static void (*Alarm_Test_Menu_Functions[])(void) =
{
   Alarm_Exit,
   Alarm_Info,
   Alarm_Output_1_Menu,
   Alarm_Output_2_Menu
};

void Alarm_Output_1_Exit(void);
void Alarm_Output_1_Continuous_Pulse(void);
void Alarm_Output_1_One_Pulse(void);
void Alarm_Output_1_Set_Low(void);
void Alarm_Output_1_Set_High(void);


static const unsigned char Alarm_Output_1_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                        NS8100 ALARM 1 OUTPUT MENU                             บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Pulse continuously.\n\r"
"   2) Pulse one time.\n\r"
"   3) Set Low.\n\r"
"   4) Set High.\n\r"
"\0"
};

static void (*Alarm_Output_1_Menu_Functions[])(void) =
{
	Alarm_Output_1_Exit,
	Alarm_Output_1_Continuous_Pulse,
	Alarm_Output_1_One_Pulse,
	Alarm_Output_1_Set_Low,
	Alarm_Output_1_Set_High
};


void Alarm_Output_2_Exit(void);
void Alarm_Output_2_Continuous_Pulse(void);
void Alarm_Output_2_One_Pulse(void);
void Alarm_Output_2_Set_Low(void);
void Alarm_Output_2_Set_High(void);

static const unsigned char Alarm_Output_2_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 ALARM 2 OUTPUT MENU                          บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
	"   0) Exit.\n\r"
	"   1) Pulse continuously.\n\r"
	"   2) Pulse one time.\n\r"
	"   3) Set Low.\n\r"
	"   4) Set High.\n\r"
"\0"
};

static void (*Alarm_Output_2_Menu_Functions[])(void) =
{
	Alarm_Output_2_Exit,
	Alarm_Output_2_Continuous_Pulse,
	Alarm_Output_2_One_Pulse,
	Alarm_Output_2_Set_Low,
	Alarm_Output_2_Set_High
};
#endif //ALARM_TEST_MENU_H_

#endif