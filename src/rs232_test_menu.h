////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   rs232_test_menu.h                                                        //
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
//   $Date: 2009/11/05 00:19:55 $                                                                 //
//   $Revision: 1.3 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef RS232_TEST_MENU_H_
#define RS232_TEST_MENU_H_


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
void RS232_Test_Menu(void);
void RS232_Exit(void);
void RS232_Info(void);
void RS232_Transmit_Test(void);
void RS232_Receive_Test(void);
void RS232_Pulse_RTS_Continuous_Test(void);
void RS232_Pulse_DTR_Continuous_Test(void);
void RS232_Read_CTS_Continuous_Test(void);
void RS232_Read_DSR_Continuous_Test(void);
void RS232_Read_DCD_Continuous_Test(void);
void RS232_Read_RI_Continuous_Test(void);
void RS232_DCD_Wake_Test(void);

static const unsigned char RS232_Info_Text[] =
{
"\n\r\n\r"
"   Part = AD7689                  Analog Voltage = 5V\n\r"
"   Interface = SPI0-CS0           Digital Voltage = 3.3\n\r"
"   Ref. Des. = U3                 Input Range = 5v\n\r"
"   Sampling Speed = 250K          Reference = External ADR125 (U9)\n\r"
"   Bits = 16\n\r"
"\n\r"
"   Channels:\n\r"
"     1) Radial            2) Transverse\n\r"
"     3) Vertical          4) Air(Normal/A-Weighted)\n\r"
"     5) Empty             6) Empty\n\r"
"     7) Empty             8) Empty\n\r"
"\n\r"
"   Control Register:\n\r"
"     Part = 74HC595               Interface = SPI1-CS3\n\r"
"     Ref. Des = U10               Digital Voltage = 3.3\n\r"
"\n\r"
"   Control Bits:\n\r"
"     1) Geophone Gain( 0 = High(2X), 1 = Low(1x))\n\r"
"     2) Air Type( 0 = Normal, 1 = A-Weighted)\n\r"
"     3-5) Filter 0,1 and Enable\n\r"
"        5  4  3   |   Range\n\r"
"        ----------+--------------\n\r"
"        0  x  x   |   Filter 1\n\r"
"        1  0  0   |   Filter 2\n\r"
"        1  0  1   |   Filter 3\n\r"
"        1  1  0   |   Filter 4\n\r"
"        1  1  1   |   Filter 5\n\r"
"     6) Empty\n\r"
"     7) Cal_Sig ( 0 = Cal Low(0V), 1 = Cal High(5V))\n\r"
"     8) Cal_Sig Enable ( 0 = Cal_Sig Off, 1 = Cal_Sig Active)\n\r"
"\n\r"
"\0"
};


const unsigned char RS232_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                            NS8100 RS232 TEST MENU                            บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) RS232 Information.\n\r"
"   2) RS232 Transmit Test.\n\r"
"   3) RS232 Receive Test.\n\r"
"   4) RS232 Pulse RTS Continuous Test.\n\r"
"   5) RS232 Pulse DTR Continuous Test.\n\r"
"   6) RS232 Read CTS Continuous Test.\n\r"
"   7) RS232 Read DSR Continuous Test.\n\r"
"   8) RS232 Read DCD Continuous Test.\n\r"
"   9) RS232 Read RI Continuous Test.\n\r"
"   10) RS232 DCD Wake Test.\n\r"
"\0"
};

static void (*RS232_Test_Menu_Functions[])(void) =
{
   RS232_Exit,
   RS232_Info,
   RS232_Transmit_Test,
   RS232_Receive_Test,
   RS232_Pulse_RTS_Continuous_Test,
   RS232_Pulse_DTR_Continuous_Test,
   RS232_Read_CTS_Continuous_Test,
   RS232_Read_DSR_Continuous_Test,
   RS232_Read_DCD_Continuous_Test,
   RS232_Read_RI_Continuous_Test,
   RS232_DCD_Wake_Test
};

#endif //RS232_TEST_MENU_H_
