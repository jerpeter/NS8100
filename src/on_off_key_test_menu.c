////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   on_off_key_test_menu.c                                                   //
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


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "craft.h"
#include "on_off_key_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "twi.h"
#include "m23018.h"


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
static void twi_resources_init(void)
{
  // GPIO pins used for TWI interface
  static const gpio_map_t TWI_GPIO_MAP =
  {
    {AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
    {AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}
  };

  // TWI options.
  twi_options_t opt;

  // options settings
  opt.pba_hz = FOSC0;
  opt.speed = TWI_SPEED;
  opt.chip = IO_ADDRESS_KPD;

  // TWI gpio pins configuration
  gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));

  // initialize TWI driver with options
  twi_master_init(&AVR32_TWI, &opt);
}

void On_Off_Key_Test_Menu(void)
{
   int state;

   twi_resources_init();
   init_mcp23018(IO_ADDRESS_KPD);
   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
   state |= 0x80;
   write_mcp23018(IO_ADDRESS_KPD, OLATA, state);

   Menu_Items = (sizeof(On_Off_Key_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)On_Off_Key_Test_Menu_Functions;
   Menu_String = (unsigned char *)On_Off_Key_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  On_Off_Key_Exit                                                     //
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
void On_Off_Key_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void On_Off_Key_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = On_Off_Key_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = On_Off_Key_Info_Text[count];
		}
}

void On_Off_Key_On_Menu(void)
{
   Menu_Items = (sizeof(On_Off_Key_On_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)On_Off_Key_On_Test_Menu_Functions;
   Menu_String = (unsigned char *)On_Off_Key_On_Test_Menu_Text;
}

void On_Off_Key_Off_Menu(void)
{
   Menu_Items = (sizeof(On_Off_Key_Off_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)On_Off_Key_Off_Test_Menu_Functions;
   Menu_String = (unsigned char *)On_Off_Key_Off_Test_Menu_Text;
}


void On_Off_Key_On_Menu_Exit(void)
{
   Menu_Items = (sizeof(On_Off_Key_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)On_Off_Key_Test_Menu_Functions;
   Menu_String = (unsigned char *)On_Off_Key_Test_Menu_Text;
}

void On_Off_Key_On_Menu_Read_Continuous(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {

	  print_dbg("On Key = ");

	  state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x04);
      if(state==0)
      {
	     print_dbg("On \r");
      }
      else
      {
	     print_dbg("Off\r");
      }
   }
   print_dbg("\n\r\n\r");
}

void On_Off_Key_On_Menu_Read_Once(void)
{
   int state;

   print_dbg("\n\rOn Key = ");

   state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x04);
   if(state==0)
   {
      print_dbg("On\n\r");
   }
   else
   {
      print_dbg("Off\n\r");
   }
   print_dbg("\n\r");
}


void On_Off_Key_Off_Menu_Exit(void)
{
   Menu_Items = (sizeof(On_Off_Key_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)On_Off_Key_Test_Menu_Functions;
   Menu_String = (unsigned char *)On_Off_Key_Test_Menu_Text;
}

void On_Off_Key_Off_Menu_Read_Continuous(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {

	  print_dbg("Off Key = ");

	  state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x02);
      if(state==0)
      {
	     print_dbg("On \r");
      }
      else
      {
	     print_dbg("Off\r");
      }
   }
   print_dbg("\n\r\n\r");
}
void On_Off_Key_Off_Menu_Read_Once(void)
{
   int state;

   print_dbg("\n\rOff Key = ");

   state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x02);
   if(state==0)
   {
      print_dbg("On\n\r");
   }
   else
   {
      print_dbg("Off\n\r");
   }
   print_dbg("\n\r");
}



