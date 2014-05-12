////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   nsmarts_test_menu.h                                                      //
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
#ifndef NSMARTS_TEST_MENU_H_
#define NSMARTS_TEST_MENU_H_


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
void NSMARTS_Test_Menu( void );
void NSMARTS_Exit( void );
void NSMARTS_Info( void );
void NSMARTS_Seismic_Menu( void );
void NSMARTS_Air_Menu( void );

static const unsigned char NSMARTS_Info_Text[] =
{
"\n\r\n\r"
"   Part = DS2431\n\r"
"   Voltage = Battery Voltage\n\r"
	"\n\r"
"   Interface Seismic = PB01 (Data In)\n\r"
"                       PB02 (Data Out)\n\r"
	"\n\r"
"   Interface Air     = PB01 (Data In)\n\r"
"                       PB03 (Data Out)\n\r"
"\n\r"
"\0"
};

static const unsigned char NSMARTS_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                        NS8100 NSMARTS TEST MENU                              บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) NSMARTS Information.\n\r"
"   2) NSMARTS Seismic Sensor Test.\n\r"
"   3) NSMARTS Air Sensor Test.\n\r"
"\0"
};

static void (*NSMARTS_Test_Menu_Functions[])(void) =
{
   NSMARTS_Exit,
   NSMARTS_Info,
   NSMARTS_Seismic_Menu,
   NSMARTS_Air_Menu,
};

void NSMARTS_Seismic_Exit(void);
void NSMARTS_Seismic_Continuous_Pulse(void);
void NSMARTS_Seismic_One_Pulse(void);
void NSMARTS_Seismic_Set_Low(void);
void NSMARTS_Seismic_Set_High(void);
void NSMARTS_Seismic_Reset_Sensor(void);
void NSMARTS_Seismic_Detect_Sensor(void);
void NSMARTS_Seismic_Read_Info(void);
void NSMARTS_Seismic_Test_Read_Write(void);


static const unsigned char NSMARTS_Seismic_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         8100 NSMARTS SEISMIC MENU                            บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Pulse continuously.\n\r"
"   2) Pulse one time.\n\r"
"   3) Set High.\n\r"
"   4) Set Low.\n\r"
"   5) Reset Sensor.\n\r"
"   6) Detect Sensor.\n\r"
"   7) Read Info.\n\r"
"   8) Test Read_Write.\n\r"
"\0"
};

static void (*NSMARTS_Seismic_Menu_Functions[])(void) =
{
	NSMARTS_Seismic_Exit,
	NSMARTS_Seismic_Continuous_Pulse,
	NSMARTS_Seismic_One_Pulse,
	NSMARTS_Seismic_Set_High,
	NSMARTS_Seismic_Set_Low,
	NSMARTS_Seismic_Reset_Sensor,
	NSMARTS_Seismic_Detect_Sensor,
	NSMARTS_Seismic_Read_Info,
	NSMARTS_Seismic_Test_Read_Write
};


void NSMARTS_Air_Exit(void);
void NSMARTS_Air_Continuous_Pulse(void);
void NSMARTS_Air_One_Pulse(void);
void NSMARTS_Air_Set_Low(void);
void NSMARTS_Air_Set_High(void);
void NSMARTS_Air_Reset_Sensor(void);
void NSMARTS_Air_Detect_Sensor(void);
void NSMARTS_Air_Read_Info(void);
void NSMARTS_Air_Test_Read_Write(void);

static const unsigned char NSMARTS_Air_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 NSMARTS AIR MENU                             บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
	"   0) Exit.\n\r"
	"   1) Pulse continuously.\n\r"
	"   2) Pulse one time.\n\r"
	"   3) Set High.\n\r"
	"   4) Set Low.\n\r"
	"   5) Reset Sensor.\n\r"
	"   6) Detect Sensor.\n\r"
	"   7) Read Info.\n\r"
	"   8) Test Read_Write.\n\r"
"\0"
};

static void (*NSMARTS_Air_Menu_Functions[])(void) =
{
		NSMARTS_Air_Exit,
		NSMARTS_Air_Continuous_Pulse,
		NSMARTS_Air_One_Pulse,
		NSMARTS_Air_Set_High,
		NSMARTS_Air_Set_Low,
		NSMARTS_Air_Reset_Sensor,
		NSMARTS_Air_Detect_Sensor,
		NSMARTS_Air_Read_Info,
		NSMARTS_Air_Test_Read_Write
};
#endif //NSMARTS_TEST_MENU_H_
