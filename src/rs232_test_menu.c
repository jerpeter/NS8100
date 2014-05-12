////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   rs232_test_menu.c                                                        //
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


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "craft.h"
#include "rs232_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void RS232_Test_Menu(void)
{
   Menu_Items = (sizeof(RS232_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)RS232_Test_Menu_Functions;
   Menu_String = (unsigned char *)RS232_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  RS232_Exit                                                          //
//                                                                            //
//       Leaves current menu and returns to the previous menu.                //
//                                                                            //
// RETURN:                                                                    //
//                                                                            //
//     void                                                                   //
//                                                                            //
// ARGUMENTS:                                                                 //
//                                                                            //
//     void                                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void RS232_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void RS232_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = RS232_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = RS232_Info_Text[count];
		}
}
void RS232_Transmit_Test(void){}
void RS232_Receive_Test(void){}
void RS232_Pulse_RTS_Continuous_Test(void){}
void RS232_Pulse_DTR_Continuous_Test(void){}
void RS232_Read_CTS_Continuous_Test(void){}
void RS232_Read_DSR_Continuous_Test(void){}
void RS232_Read_DCD_Continuous_Test(void){}
void RS232_Read_RI_Continuous_Test(void){}
void RS232_DCD_Wake_Test(void){}

