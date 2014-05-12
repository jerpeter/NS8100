////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   rtc_test_menu.h                                                          //
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
//   $Date: 2012/04/26 01:10:06 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef RTC_TEST_MENU_H_
#define RTC_TEST_MENU_H_

#define RTC_SPI                  (&AVR32_SPI1)
#define RTC_SPI_NPCS             1
#define RTC_SPI_BITS             8
#define RTC_SPI_MASTER_SPEED     1000000 // 1000000 // Speed should be safe up to 3.5 MHz, needs to be tested
#define RTC_SPI_SCK_PIN          AVR32_SPI1_SCK_0_0_PIN
#define RTC_SPI_SCK_FUNCTION     AVR32_SPI1_SCK_0_0_FUNCTION
#define RTC_SPI_MISO_PIN         AVR32_SPI1_MISO_0_0_PIN
#define RTC_SPI_MISO_FUNCTION    AVR32_SPI1_MISO_0_0_FUNCTION
#define RTC_SPI_MOSI_PIN         AVR32_SPI1_MOSI_0_0_PIN
#define RTC_SPI_MOSI_FUNCTION    AVR32_SPI1_MOSI_0_0_FUNCTION
#define RTC_SPI_NPCS_PIN         AVR32_SPI1_NPCS_1_0_PIN
#define RTC_SPI_NPCS_FUNCTION    AVR32_SPI1_NPCS_1_0_FUNCTION

#define RTC_WRITE   0x20
#define RTC_READ    0xA0

#define TENTH_SECOND_ADDRESS  0x00
#define SECOND_ADDRESS        0x01
#define MINUTE_ADDRESS        0x02
#define HOUR_ADDRESS          0x03
#define DAY_ADDRESS           0x04
#define DATE_ADDRESS          0x05
#define MONTH_ADDRESS         0x06
#define YEAR_ADDRESS          0x07
#define CONTROL_ADDRESS       0x08
#define WATCHDOG_ADDRESS      0x09
#define ALARM_MONTH_ADDRESS   0x0A
#define ALARM_DATE_ADDRESS    0x0B
#define ALARM_HOUR_ADDRESS    0x0C
#define ALARM_MINUTE_ADDRESS  0x0D
#define ALARM_SECOND_ADDRESS  0x0E
#define FLAGS_ADDRESS         0x0F
#define SQW_ADDRESS           0x13
#define FIRST_RTC_RAM         0x14

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "Typedefs.h"
#include "gpio.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Global Scope Declarations                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void RTC_Test_Menu(void);
void RTC_Exit(void);
void RTC_Info(void);
void RTC_Read_Time(void);
void RTC_Set_Time(void);
void RTC_Read_Date(void);
void RTC_Set_Date(void);
void RTC_Read_Alarm_Time(void);
void RTC_Set_Alarm_Time(void);
void RTC_Read_Alarm_Date(void);
void RTC_Set_Alarm_Date(void);
void RTC_Test_Alarm_Output(void);
void RTC_Test_Auto_On(void);
void RTC_Read_Registers(void);
void RTC_Fill_Ram(void);
void rtc_write(uint8 register_address, int length, uint16* data);
void rtc_read(uint8 register_address, int length, uint16* data);

#endif //RTC_TEST_MENU_H_
