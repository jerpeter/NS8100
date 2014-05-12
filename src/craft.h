////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   ad_test_menu.h                                                           //
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
//   $Date: 2010/04/28 18:13:17 $                                                                 //
//   $Revision: 1.7 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef CRAFT_MENU_H_
#define CRAFT_MENU_H_

// Include Files
// TODO:
void NONE( void );


extern void AD_Test_Menu(void);
extern void NSMARTS_Test_Menu(void);
extern void Alarm_Test_Menu(void);
extern void Trigger_Test_Menu(void);
extern void Keypad_Test_Menu(void);
extern void On_Off_Key_Test_Menu(void);
extern void Voltage_Monitor_Test_Menu(void);
extern void SD_MMC_Test_Menu(void);
extern void RTC_Test_Menu(void);
extern void EEPROM_Test_Menu(void);
extern void SRAM_Test_Menu(void);
extern void LCD_Test_Menu(void);
extern void Network_Test_Menu(void);
extern void On_Off_Circuit_Test_Menu(void);
extern void RS485_Test_Menu(void);
extern void RS232_Test_Menu(void);
extern void USB_Test_Menu(void);
extern void Proc_Timers_Test_Menu(void);

int Menu_Items;
unsigned char *Menu_String;
unsigned long *Menu_Functions;

static const unsigned char Main_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 CRAFT INTERFACE                              บ\n\r"
"  บ                                                                              บ\n\r"
"  บ                         Version 01.70 12/08/2009                             บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   1) A/D System Tests.\n\r"
"   2) NSmarts Sensor Tests.\n\r"
"   3) Alarm Output Tests.\n\r"
"   4) Trigger In/Out Tests.\n\r"
"   5) Keypad Tests.\n\r"
"   6) On/Off Key detect Tests.\n\r"
"   7) Voltage Monitoring Tests.\n\r"
"   8) SD_MMC Tests.\n\r"
"   9) RTC Tests.\n\r"
"  10) Configuration EEPROM Tests.\n\r"
"  11) SRAM Tests.\n\r"
"  12) LCD Tests.\n\r"
"  13) Network Tests.\n\r"
"  14) On/Off Circuit Tests.\n\r"
"  15) RS485 Tests.\n\r"
"  16) RS232 Tests.\n\r"
"  17) USB Tests.\n\r"
"  18) Proc Timers.\n\r"
"\0"
};

#define MAIN_MENU_FUNCTIONS_ITEMS 19
extern void (*Main_Menu_Functions[])(void);

#endif //CRAFT_MENU_H_

