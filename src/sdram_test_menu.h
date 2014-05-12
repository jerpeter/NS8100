////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   sdram_test_menu.h                                                        //
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

#if 0

#ifndef SDRAM_TEST_MENU_H_
#define SDRAM_TEST_MENU_H_


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
void SDRAM_Test_Menu(void);
void SDRAM_Exit(void);
void SDRAM_Info(void);
void SDRAM_Erase(void);
void SDRAM_Fill(void);
void SDRAM_Read(void);
void SDRAM_Write(void);
void SDRAM_Test(void);

static const unsigned char SDRAM_Info_Text[] =
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

const unsigned char SDRAM_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 SDRAM TEST MENU                             บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) SDRAM Information.\n\r"
"   2) Erase SDRAM.\n\r"
"   3) Fill SDRAM.\n\r"
"   4) Read SDRAM.\n\r"
"   5) Write SDRAM.\n\r"
"   6) Test SDRAM.\n\r"
"\0"
};

static void (*SDRAM_Test_Menu_Functions[])(void) =
{
   SDRAM_Exit,
   SDRAM_Info,
   SDRAM_Erase,
   SDRAM_Fill,
   SDRAM_Read,
   SDRAM_Write,
   SDRAM_Test
};

#endif //SDRAM_TEST_MENU_H_

#endif
