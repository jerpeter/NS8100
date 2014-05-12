////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   eeprom_test_menu.h                                                       //
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
//   $Date: 2009/11/09 19:26:50 $                                                                 //
//   $Revision: 1.4 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef EEPROM_TEST_MENU_H_
#define EEPROM_TEST_MENU_H_

#define EEPROM_SPI                  (&AVR32_SPI1)
#define EEPROM_SPI_NPCS             0
#define EEPROM_SPI_BITS             8
#define EEPROM_SPI_MASTER_SPEED     2100000 // Speed should be safe up to 2.1 MHz, needs to be tested
#define EEPROM_SPI_SCK_PIN          AVR32_SPI1_SCK_0_0_PIN
#define EEPROM_SPI_SCK_FUNCTION     AVR32_SPI1_SCK_0_0_FUNCTION
#define EEPROM_SPI_MISO_PIN         AVR32_SPI1_MISO_0_0_PIN
#define EEPROM_SPI_MISO_FUNCTION    AVR32_SPI1_MISO_0_0_FUNCTION
#define EEPROM_SPI_MOSI_PIN         AVR32_SPI1_MOSI_0_0_PIN
#define EEPROM_SPI_MOSI_FUNCTION    AVR32_SPI1_MOSI_0_0_FUNCTION
#define EEPROM_SPI_NPCS_PIN         AVR32_SPI1_NPCS_0_0_PIN
#define EEPROM_SPI_NPCS_FUNCTION    AVR32_SPI1_NPCS_0_0_FUNCTION

#define EEPROM_WRITE_ENABLE   0x06
#define EEPROM_WRITE_DISABLE  0x04
#define EEPROM_READ_DATA      0x03
#define EEPROM_WRITE_DATA     0x02
#define EEPROM_READ_STATUS    0x05
#define EEPROM_WRITE_STATUS   0x01

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "gpio.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Global Scope Declarations                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void EEPROM_Test_Menu(void);
void EEPROM_Exit(void);
void EEPROM_Info(void);
void EEPROM_Config_Data_Menu(void);
void EEPROM_Blank_Check(void);
void EEPROM_Erase(void);
void EEPROM_Write(void);
void EEPROM_Read(void);

void EEPROM_Config_Exit(void);
void EEPROM_Config_View_Data(void);
void EEPROM_Config_Edit_Data(void);
void EEPROM_Config_Erase_CRC(void);
void EEPROM_Config_Test_CRC(void);
void EEPROM_Config_Write_CRC(void);
void EEPROM_Config_Erase_Data(void);
void EEPROM_Config_Test_Data(void);
void EEPROM_Config_Restore_Default_Data(void);

#endif //EEPROM_TEST_MENU_H_

