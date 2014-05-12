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
//   $Date: 2010/04/28 21:13:17 $                                             //
//   $Revision: 1.3 $                                                         //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
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
   SRAM_Test
};

#endif //SRAM_TEST_MENU_H_
