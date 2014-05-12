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
//   $Date: 2012/04/26 01:10:04 $                                                                 //
//   $Revision: 1.2 $                                                             //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#if 1

#ifndef AD_TEST_MENU_H_
#define AD_TEST_MENU_H_


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
#define AD_CTL_SPI                  (&AVR32_SPI1)
#define AD_CTL_SPI_NPCS             3
#define AD_CTL_SPI_SCK_PIN          AVR32_SPI1_SCK_0_0_PIN
#define AD_CTL_SPI_SCK_FUNCTION     AVR32_SPI1_SCK_0_0_FUNCTION
#define AD_CTL_SPI_MISO_PIN         AVR32_SPI1_MISO_0_0_PIN
#define AD_CTL_SPI_MISO_FUNCTION    AVR32_SPI1_MISO_0_0_FUNCTION
#define AD_CTL_SPI_MOSI_PIN         AVR32_SPI1_MOSI_0_0_PIN
#define AD_CTL_SPI_MOSI_FUNCTION    AVR32_SPI1_MOSI_0_0_FUNCTION
#define AD_CTL_SPI_NPCS_PIN         AVR32_SPI1_NPCS_3_PIN
#define AD_CTL_SPI_NPCS_FUNCTION    AVR32_SPI1_NPCS_3_FUNCTION
#define AD_CTL_SPI_MASTER_SPEED     4000000 // Speed should be safe up to 10 MHz, needs to be tested
#define AD_CTL_SPI_BITS             8

#define AD_SPI                  (&AVR32_SPI0)
#define AD_SPI_NPCS             0
#define AD_SPI_SCK_PIN          AVR32_SPI0_SCK_0_0_PIN
#define AD_SPI_SCK_FUNCTION     AVR32_SPI0_SCK_0_0_FUNCTION
#define AD_SPI_MISO_PIN         AVR32_SPI0_MISO_0_0_PIN
#define AD_SPI_MISO_FUNCTION    AVR32_SPI0_MISO_0_0_FUNCTION
#define AD_SPI_MOSI_PIN         AVR32_SPI0_MOSI_0_0_PIN
#define AD_SPI_MOSI_FUNCTION    AVR32_SPI0_MOSI_0_0_FUNCTION
#define AD_SPI_NPCS_PIN         AVR32_SPI0_NPCS_0_0_PIN
#define AD_SPI_NPCS_FUNCTION    AVR32_SPI0_NPCS_0_0_FUNCTION
#define AD_SPI_MASTER_SPEED     33000000 //4 000 000
#define AD_SPI_BITS             16

void AD_Test_Menu( void );
void AD_Exit( void );
void AD_Info( void );
void AD_CS_Menu( void );
void AD_CLK_Menu( void );
void AD_DOUT_Menu( void );
void AD_DIN_Menu( void );
void AD_Read_Menu( void );
void AD_Gain_Menu( void );
void AD_Filter_Menu( void );
void AD_Air_Menu( void );
void AD_Cal_Menu( void );

void AD_Gain_Exit(void);
void AD_Gain_Set_RVT_Low(void);
void AD_Gain_Set_RVT_High(void);
void AD_Gain_Set_Air_Low(void);
void AD_Gain_Set_Air_High(void);

void AD_Filter_Exit(void);
void AD_Filter_Set_1(void);
void AD_Filter_Set_2(void);
void AD_Filter_Set_3(void);
void AD_Filter_Set_4(void);
void AD_Filter_Set_5(void);

void AD_CS_Exit(void);
void AD_CS_Continuous_Pulse(void);
void AD_CS_One_Pulse(void);
void AD_CS_Set_High(void);
void AD_CS_Set_Low(void);

void AD_CLK_Exit(void);
void AD_CLK_Continuous_Pulse(void);
void AD_CLK_One_Pulse(void);
void AD_CLK_Set_High(void);
void AD_CLK_Set_Low(void);

void AD_DOUT_Exit(void);
void AD_DOUT_Continuous_Pulse(void);
void AD_DOUT_One_Pulse(void);
void AD_DOUT_Set_High(void);
void AD_DOUT_Set_Low(void);

void AD_DIN_Exit(void);
void AD_DIN_Continuous_Pulse(void);
void AD_DIN_One_Pulse(void);
void AD_DIN_Set_High(void);
void AD_DIN_Set_Low(void);

void AD_Read_Exit(void);
void AD_Read_Cont_RVTA(void);
void AD_Read_Cont_Radial(void);
void AD_Read_Cont_Vertical(void);
void AD_Read_Cont_Transverse(void);
void AD_Read_Cont_Air(void);
void AD_Read_Once_RVTA(void);
void AD_Read_Once_Radial(void);
void AD_Read_Once_Vertical(void);
void AD_Read_Once_Transverse(void);
void AD_Read_Once_Air(void);
void AD_Read_Status(void);

void AD_Air_Exit(void);
void AD_Air_Set_A_Weighting(void);
void AD_Air_Set_Normal(void);

void AD_Cal_Exit(void);
void AD_Cal_Set_Continuous_Pulse(void);
void AD_Cal_Set_One_Pulse(void);
void AD_Cal_Set_Low(void);
void AD_Cal_Set_High(void);
void AD_Cal_Set_Middle(void);

#endif //AD_TEST_MENU_H_

#endif