////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   Alarm_test_menu.c                                                        //
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
//   $Date: 2009/11/05 00:19:54 $                                                                 //
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
#include "alarm_test_menu.h"
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
void Alarm_Test_Menu(void)
{
       Menu_Items = (sizeof(Alarm_Test_Menu_Functions)/sizeof(NONE))/4;
       Menu_Functions = (unsigned long *)Alarm_Test_Menu_Functions;
       Menu_String = (unsigned char *)Alarm_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  Alarm_Exit                                                        //
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
void Alarm_Exit( void )
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void Alarm_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = Alarm_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = Alarm_Info_Text[count];
		}
}

void Alarm_Output_1_Menu(void)
{
   Menu_Items = (sizeof(Alarm_Output_1_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Alarm_Output_1_Menu_Functions;
   Menu_String = (unsigned char *)Alarm_Output_1_Menu_Text;
}

void Alarm_Output_2_Menu(void)
{
   Menu_Items = (sizeof(Alarm_Output_2_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Alarm_Output_2_Menu_Functions;
   Menu_String = (unsigned char *)Alarm_Output_2_Menu_Text;
}


void Alarm_Output_1_Exit(void)
{
    gpio_clr_gpio_pin(ALARM_1_PIN);
    Menu_Items = (sizeof(Alarm_Test_Menu_Functions)/sizeof(NONE))/4;
    Menu_Functions = (unsigned long *)Alarm_Test_Menu_Functions;
    Menu_String = (unsigned char *)Alarm_Test_Menu_Text;
}

void Alarm_Output_1_Continuous_Pulse(void)
{
   int count;
   int *uart_rx_character = NULL;

   print_dbg("\n\rPress any key to exit. ");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
      count=50;
      while(count-- > 0){}
	  gpio_clr_gpio_pin(ALARM_1_PIN);
      count=50;
      while(count-- > 0){}
      gpio_set_gpio_pin(ALARM_1_PIN);
   }

   print_dbg("\n\r\n\r");
}

void Alarm_Output_1_One_Pulse(void)
{
   int count;
   count=50;
   while(count-- > 0){}
   gpio_clr_gpio_pin(ALARM_1_PIN);
   count=50;
   while(count-- > 0){}
   gpio_set_gpio_pin(ALARM_1_PIN);
}

void Alarm_Output_1_Set_Low(void)
{
   gpio_set_gpio_pin(ALARM_1_PIN);
}

void Alarm_Output_1_Set_High(void)
{
   gpio_clr_gpio_pin(ALARM_1_PIN);
}

void Alarm_Output_2_Exit(void)
{
    gpio_clr_gpio_pin(ALARM_2_PIN);
    Menu_Items = (sizeof(Alarm_Test_Menu_Functions)/sizeof(NONE))/4;
    Menu_Functions = (unsigned long *)Alarm_Test_Menu_Functions;
    Menu_String = (unsigned char *)Alarm_Test_Menu_Text;
}

void Alarm_Output_2_Continuous_Pulse(void)
{
   int count;
   int *uart_rx_character = NULL;

   print_dbg("\n\rPress any key to exit. ");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
      count=50;
      while(count-- > 0){}
	  gpio_clr_gpio_pin(ALARM_2_PIN);
      count=50;
      while(count-- > 0){}
      gpio_set_gpio_pin(ALARM_2_PIN);
   }

   print_dbg("\n\r\n\r");
}

void Alarm_Output_2_One_Pulse(void)
{
   int count;
   count=50;
   while(count-- > 0){}
   gpio_clr_gpio_pin(ALARM_2_PIN);
   count=50;
   while(count-- > 0){}
   gpio_set_gpio_pin(ALARM_2_PIN);
}

void Alarm_Output_2_Set_Low(void)
{
   gpio_set_gpio_pin(ALARM_2_PIN);
}

void Alarm_Output_2_Set_High(void)
{
   gpio_clr_gpio_pin(ALARM_2_PIN);
}


