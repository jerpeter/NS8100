////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   voltage_monitor_test_menu.h                                              //
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
//   $Date: 2012/04/26 01:10:08 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef VOLTAGE_MONITOR_TEST_MENU_H_
#define VOLTAGE_MONITOR_TEST_MENU_H_


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
void Voltage_Monitor_Test_Menu(void);
void Voltage_Monitor_Exit(void);
void Voltage_Monitor_Info(void);
void Voltage_Monitor_Battery_Menu(void);
void Voltage_Monitor_Vin_Menu(void);

static const unsigned char Voltage_Monitor_Info_Text[] =
{
"\n\r\n\r"
"   Battery Voltage   = J13 pin 1 and 2\n\r"
"   External Voltage  = JP3 pin 12, 14 or 16\n\r"
"\n\r"
"   A/D Channels\n\r"
"   Battery Voltage   = Channel 2\n\r"
"   External Voltage  = Channel 3\n\r"
"\n\r"
"\0"
};

const unsigned char Voltage_Monitor_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                      NS8100 VOLTAGE MONITOR TEST MENU                        บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Voltage Monitor Information.\n\r"
"   2) Battery Test Menu.\n\r"
"   3) VIN Test Menu.\n\r"
"\0"
};

static void (*Voltage_Monitor_Test_Menu_Functions[])(void) =
{
   Voltage_Monitor_Exit,
   Voltage_Monitor_Info,
   Voltage_Monitor_Battery_Menu,
   Voltage_Monitor_Vin_Menu
};

void Voltage_Monitor_Battery_Menu_Exit(void);
void Voltage_Monitor_Battery_Read_Continuous(void);
void Voltage_Monitor_Battery_Read_Once(void);

const unsigned char Voltage_Monitor_Battery_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 BATTERY TEST MENU                           บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Battery voltage read continuously.\n\r"
"   2) Battery voltage read once.\n\r"
"\0"
};

static void (*Voltage_Monitor_Battery_Test_Menu_Functions[])(void) =
{
   Voltage_Monitor_Battery_Menu_Exit,
   Voltage_Monitor_Battery_Read_Continuous,
   Voltage_Monitor_Battery_Read_Once
};

void Voltage_Monitor_Vin_Menu_Exit(void);
void Voltage_Monitor_Vin_Read_Continuous(void);
void Voltage_Monitor_Vin_Read_Once(void);

const unsigned char Voltage_Monitor_Vin_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                             NS8100 VIN TEST MENU                             บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) External voltage read continuously.\n\r"
"   2) External voltage read once.\n\r"
"\0"
};

static void (*Voltage_Monitor_Vin_Test_Menu_Functions[])(void) =
{
   Voltage_Monitor_Vin_Menu_Exit,
   Voltage_Monitor_Vin_Read_Continuous,
   Voltage_Monitor_Vin_Read_Once
};

#endif //VOLTAGE_MONITOR_TEST_MENU_H_
