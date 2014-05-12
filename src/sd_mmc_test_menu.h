////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   sd_mmc_test_menu.h                                                       //
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
//   $Revision: 1.3 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#ifndef SD_MMC_TEST_MENU_H_
#define SD_MMC_TEST_MENU_H_

#define SD_WRITE_PROTECT_PIN  AVR32_PIN_PA07
#define SD_DETECT_PIN         AVR32_PIN_PA02
#define SD_POWER_PIN          AVR32_PIN_PB15
#define SDMMC_SPI                  (&AVR32_SPI1)
#define SDMMC_SPI_NPCS             2
#define SDMMC_SPI_BITS             8
#define SDMMC_SPI_MASTER_SPEED     12000000 // Speed should be safe up to 12 MHz (150 KHz * 80x)
#define SDMMC_SPI_SCK_PIN          AVR32_SPI1_SCK_0_0_PIN
#define SDMMC_SPI_SCK_FUNCTION     AVR32_SPI1_SCK_0_0_FUNCTION
#define SDMMC_SPI_MISO_PIN         AVR32_SPI1_MISO_0_0_PIN
#define SDMMC_SPI_MISO_FUNCTION    AVR32_SPI1_MISO_0_0_FUNCTION
#define SDMMC_SPI_MOSI_PIN         AVR32_SPI1_MOSI_0_0_PIN
#define SDMMC_SPI_MOSI_FUNCTION    AVR32_SPI1_MOSI_0_0_FUNCTION
#define SDMMC_SPI_NPCS_PIN         AVR32_SPI1_NPCS_2_0_PIN
#define SDMMC_SPI_NPCS_FUNCTION    AVR32_SPI1_NPCS_2_0_FUNCTION

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
void SD_MMC_Test_Menu(void);
void SD_MMC_Exit(void);
void SD_MMC_Info(void);
void SD_MMC_Power_On(void);
void SD_MMC_Power_Off(void);
void SD_MMC_File_System_Menu(void);
void SD_MMC_Read_Write_Protect(void);
void SD_MMC_Read_Card_Detect(void);

void SD_MMC_File_System_Exit(void);
void SD_MMC_File_System_Format(void);
void SD_MMC_File_System_List_Directory(void);
void SD_MMC_File_System_Change_Directory(void);
void SD_MMC_File_System_Remove_Directory(void);
void SD_MMC_File_System_Make_Directory(void);
void SD_MMC_File_System_Delete(void);
void SD_MMC_File_System_Type(void);
void SD_MMC_File_System_Rename(void);
void SD_MMC_File_System_Copy(void);
void SD_MMC_File_System_Change_Drive(void);
void SD_MMC_File_System_Volume(void);
void SD_MMC_Display_SID_text(void);
void SD_MMC_Display_Log_text(void);
void SD_MMC_Add_Log_text(void);
void SD_MMC_Test_Standard_Fat_Driver(void);
void SD_MMC_Test_Standard_Fat_Read(void);
void SD_MMC_Copy_Test_File(void);
void SD_MMC_Inc_Buffer_Size_Test_File(void);

#endif //SD_MMC_TEST_MENU_H_
