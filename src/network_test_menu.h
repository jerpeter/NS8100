////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   network_test_menu.h                                                      //
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
//   $Date: 2012/04/26 01:10:05 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef NETWORK_TEST_MENU_H_
#define NETWORK_TEST_MENU_H_

#include "tcpip.h"

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
void Network_Test_Menu(void);
void Network_Exit(void);
void Network_Info(void);
void Network_Self_Test(void);
void Network_Transmit_Test(void);
void Network_Receive_Test(void);
void Network_LED_Test(void);
void Network_EEPROM_Test(void);
void Network_8_Bit_Test(void);
void Network_16_Bit_Test(void);
void Network_Memory_Test(void);
void Network_Ping_IP_Test(void);
void Network_Read_Home_Page(void);
void Network_Read_MAC(void);
void Network_Set_MAC(void);
void Network_Read_IP(void);
void Network_Set_IP(void);

static const unsigned char Network_Info_Text[] =
{
"\n\r\n\r"
"   Part = CS8900A                 Analog Voltage = 3.3V\n\r"
"   Interface = NCS2, NCS3         Digital Voltage = 3.3\n\r"
"   Ref. Des. = U38                Network Speed = 10Mbit\n\r"
"   IP = "
ASCII_MYIP_1 "."
ASCII_MYIP_2 "."
ASCII_MYIP_3 "."
ASCII_MYIP_4
"\n\r"
"\n\r"
"\0"
};

const unsigned char Network_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 NETWORK TEST MENU                           บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Network Information.\n\r"
"   2) Network Self Test.\n\r"
"   3) Network Transmit Test.\n\r"
"   4) Network Receive Test.\n\r"
"   5) Network LED Test.\n\r"
"   6) Network EEPROM Test.\n\r"
"   7) Network 8-Bit Test.\n\r"
"   8) Network 16-Bit Test.\n\r"
"   9) Network Memory Test.\n\r"
"  10) Network Ping IP Test.\n\r"
"  11) Network Read Home Page.\n\r"
"  12) Network Read MAC.\n\r"
"  13) Network Set MAC.\n\r"
"  14) Network Read IP.\n\r"
"  15) Network Set IP.\n\r"
"\0"
};

static void (*Network_Test_Menu_Functions[])(void) =
{
Network_Exit,
Network_Info,
Network_Self_Test,
Network_Transmit_Test,
Network_Receive_Test,
Network_LED_Test,
Network_EEPROM_Test,
Network_8_Bit_Test,
Network_16_Bit_Test,
Network_Memory_Test,
Network_Ping_IP_Test,
Network_Read_Home_Page,
Network_Read_MAC,
Network_Set_MAC,
Network_Read_IP,
Network_Set_IP
};

#endif //NETWORK_TEST_MENU_H_
