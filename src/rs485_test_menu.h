////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   rs485_test_menu.h                                                        //
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
//   $Date: 2009/12/10 01:14:07 $                                                                 //
//   $Revision: 1.4 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef RS485_TEST_MENU_H_
#define RS485_TEST_MENU_H_


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
void RS485_Test_Menu(void);
void RS485_Exit(void);
void RS485_Info(void);
void RS485_Transmit_Character_Once_Test(void);
void RS485_Transmit_Character_Continuous_Test(void);
void RS485_Echo_Received_Character_Once_Test(void);
void RS485_Echo_Received_Character_Continuous_Test(void);
void RS485_Pulse_Transmit_Enable_Once_Test(void);
void RS485_Pulse_Transmit_Enable_Continuous_Test(void);
void RS485_Loopback_Test(void);

static const unsigned char RS485_Test_Text[] =
{
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789!@#$%^&*()-=_+~`{}[]|:;'<>,.?"
};

static const unsigned char RS485_Info_Text[] =
{
"\n\r\n\r"
"   Part = LTC1485                 Voltage = 3.3V\n\r"
"   Interface = UART3              Ref. Des. = U78\n\r"
"   Speed = 115200                 Mode = RS485\n\r"
"\n\r"
"\0"
};


const unsigned char RS485_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                            NS8100 RS485 TEST MENU                            บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) RS485 Information.\n\r"
"   2) RS485 Transmit Character Once Test.\n\r"
"   3) RS485 Transmit Character Continuous Test.\n\r"
"   4) RS485 Echo Received Character Once Test.\n\r"
"   5) RS485 Echo Received Character Continuous Test.\n\r"
"   6) RS485 Pulse Transmit Enable Once Test.\n\r"
"   7) RS485 Pulse Transmit Enable Continuous Test.\n\r"
"   8) RS485 Loopback Test.\n\r"
"\0"
};

static void (*RS485_Test_Menu_Functions[])(void) =
{
   RS485_Exit,
   RS485_Info,
   RS485_Transmit_Character_Once_Test,
   RS485_Transmit_Character_Continuous_Test,
   RS485_Echo_Received_Character_Once_Test,
   RS485_Echo_Received_Character_Continuous_Test,
   RS485_Pulse_Transmit_Enable_Once_Test,
   RS485_Pulse_Transmit_Enable_Continuous_Test,
   RS485_Loopback_Test
};

#endif //RS485_TEST_MENU_H_
