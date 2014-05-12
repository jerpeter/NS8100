////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   trigger_test_menu.c                                                      //
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
#include "trigger_test_menu.h"
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
#define  TRIGGER_OUT_PIN  AVR32_PIN_PB05

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

void Trigger_Test_Menu(void)
{
   twi_resources_init();
   init_mcp23018(IO_ADDRESS_KPD);
   Menu_Items = (sizeof(Trigger_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Trigger_Test_Menu_Functions;
   Menu_String = (unsigned char *)Trigger_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  Trigger_Exit                                                        //
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
void Trigger_Exit( void )
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void Trigger_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = Trigger_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = Trigger_Info_Text[count];
		}
}

void Trigger_In_Menu(void)
{
   Menu_Items = (sizeof(Trigger_In_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Trigger_In_Menu_Functions;
   Menu_String = (unsigned char *)Trigger_In_Menu_Text;
}

void Trigger_In_Exit(void)
{
	   Menu_Items = (sizeof(Trigger_Test_Menu_Functions)/sizeof(NONE))/4;
	   Menu_Functions = (unsigned long *)Trigger_Test_Menu_Functions;
	   Menu_String = (unsigned char *)Trigger_Test_Menu_Text;
}

void Trigger_In_Continuous_Read(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {

	  print_dbg("Trigger in = ");

	  state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x08);
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

void Trigger_In_One_Read(void)
{
   int state;

   print_dbg("\n\rTrigger in = ");

   state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x08);
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


void Trigger_Out_Menu(void)
{
   Menu_Items = (sizeof(Trigger_Out_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Trigger_Out_Menu_Functions;
   Menu_String = (unsigned char *)Trigger_Out_Menu_Text;
}

void Trigger_Out_Exit(void)
{
	gpio_clr_gpio_pin(TRIGGER_OUT_PIN);
	Menu_Items = (sizeof(Trigger_Test_Menu_Functions)/sizeof(NONE))/4;
	Menu_Functions = (unsigned long *)Trigger_Test_Menu_Functions;
	Menu_String = (unsigned char *)Trigger_Test_Menu_Text;
}

void Trigger_Out_Continuous_Pulse(void)
{
   int count;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. ");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
      count=50;
      while(count-- > 0){}
	  gpio_clr_gpio_pin(TRIGGER_OUT_PIN);
      count=50;
      while(count-- > 0){}
      gpio_set_gpio_pin(TRIGGER_OUT_PIN);
   }
   print_dbg("\n\r\n\r");
}

void Trigger_Out_One_Pulse(void)
{
   int count;
   count=50;
   while(count-- > 0){}
   gpio_clr_gpio_pin(TRIGGER_OUT_PIN);
   count=50;
   while(count-- > 0){}
   gpio_set_gpio_pin(TRIGGER_OUT_PIN);
}

void Trigger_Out_Set_Low(void)
{
   gpio_set_gpio_pin(TRIGGER_OUT_PIN);
}

void Trigger_Out_Set_High(void)
{
   gpio_clr_gpio_pin(TRIGGER_OUT_PIN);
}

