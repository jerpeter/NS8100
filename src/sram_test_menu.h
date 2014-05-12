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
//   $Author: jgetz $                                                         //
//   $Date: 2012/04/26 01:10:07 $                                             //
//   $Revision: 1.2 $                                                         //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef SRAM_TEST_MENU_H_
#define SRAM_TEST_MENU_H_


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
void smc_init(unsigned long hsb_hz);
void SRAM_Test_Menu(void);
void SRAM_Exit(void);
void SRAM_Info(void);
void SRAM_Erase(void);
void SRAM_Fill(void);
void SRAM_Read(void);
void SRAM_Write(void);
void SRAM_Test(void);
void SRAM_Test_2M(void);
void SRAM_Test_3M(void);
void SRAM_Test_4M(void);
void SRAM_Test_5M(void);
void SRAM_Test_6M(void);
void SRAM_Test_7M(void);
void SRAM_Test_2M_by_32(void);
void SRAM_Test_1M_by_32(void);
void SRAM_Test_32(void);

static const unsigned char SRAM_Info_Text[] =
{
"\n\r\n\r"
"   Part = CY62187DV30LL           Digital Voltage = 3.3V\n\r"
"   Interface = EBI_NCS1           Ram Size: 8M bytes\n\r"
"   Ref. Des. = U74                Ram Width: 16 bits\n\r"
"   Access Time = 70nS             Address: 0xD0000000\n\r"
"\n\r"
"\0"
};

const unsigned char SRAM_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 SRAM TEST MENU                              บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) SRAM Information.\n\r"
"   2) Erase SRAM.\n\r"
"   3) Fill SRAM.\n\r"
"   4) Read SRAM.\n\r"
"   5) Write SRAM.\n\r"
"   6) Test SRAM.\n\r"
"   7) Test SRAM @ 2M by 16.\n\r"
"   8) Test SRAM @ 3M by 16.\n\r"
"   9) Test SRAM @ 4M by 16.\n\r"
"  10) Test SRAM @ 5M by 16.\n\r"
"  11) Test SRAM @ 6M by 16.\n\r"
"  12) Test SRAM @ 7M by 16.\n\r"
"  13) Test SRAM @ 2M by 32.\n\r"
"  14) Test SRAM @ 1M by 32.\n\r"
"  15) Test SRAM 32.\n\r"
"\0"
};

static void (*SRAM_Test_Menu_Functions[])(void) =
{
   SRAM_Exit,
   SRAM_Info,
   SRAM_Erase,
   SRAM_Fill,
   SRAM_Read,
   SRAM_Write,
   SRAM_Test,
   SRAM_Test_2M,
   SRAM_Test_3M,
   SRAM_Test_4M,
   SRAM_Test_5M,
   SRAM_Test_6M,
   SRAM_Test_7M,
   SRAM_Test_2M_by_32,
   SRAM_Test_1M_by_32,
   SRAM_Test_32
};

#endif //SRAM_TEST_MENU_H_

#endif
