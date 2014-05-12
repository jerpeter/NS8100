////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   keypad_test_menu.c                                                       //
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


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "craft.h"
#include "keypad_test_menu.h"
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

void Keypad_Test_Menu(void)
{
   twi_resources_init();
   print_dbg("twi_resources_init(): Passed\n");
   init_mcp23018(IO_ADDRESS_KPD);
   print_dbg("init_mcp23018(): Passed\n");
   Menu_Items = (sizeof(Keypad_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Keypad_Test_Menu_Functions;
   Menu_String = (unsigned char *)Keypad_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  Keypad_Exit                                                        //
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
void Keypad_Exit( void )
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void Keypad_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = Keypad_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = Keypad_Info_Text[count];
		}
}

void Keypad_Continuous_Read(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   print_dbg("Light  Help   Esc    Up     Down   Minus  Plus   Enter\n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
	  state = read_mcp23018(IO_ADDRESS_KPD, GPIOB);

	  if((state & 0x01) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x02) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x04) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x08) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x10) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x20) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x40) == 0)
      {
	     print_dbg("  1    ");
      }
      else
      {
 	     print_dbg("  0    ");
      }

	  if((state & 0x80) == 0)
      {
	     print_dbg("  1    \r");
      }
      else
      {
 	     print_dbg("  0    \r");
      }
   }
   print_dbg("\n\r\n\r");
}

void Keypad_One_Read(void)
{
   int state;

   print_dbg("Light  Help   Esc    Up     Down   Minus  Plus   Enter\n\r");

   state = read_mcp23018(IO_ADDRESS_KPD, GPIOB);

   if((state & 0x01) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x02) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x04) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x08) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x10) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x20) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x40) == 0)
   {
      print_dbg("  1    ");
   }
   else
   {
      print_dbg("  0    ");
   }

   if((state & 0x80) == 0)
   {
      print_dbg("  1    \r");
   }
   else
   {
      print_dbg("  0    \r");
   }
   print_dbg("\n\r\n\r");
}

void Keypad_LED_Off( void )
{
   int state;

   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
   state &= 0xC0;
   //write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
}

void Keypad_LED_Red( void )
{
   int state;

   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
   state &= 0xC0;
   state |= 0x10;
   //write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
}

void Keypad_LED_Green( void )
{
   int state;

   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
   state &= 0xC0;
   state |= 0x20;
   //write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
}

void Keypad_Toggle_LED_Red(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state |= 0x10;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);

		//soft_delay(250 * SOFT_MSECS);
	
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state &= ~0x10;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
   }	   
}

void Keypad_Toggle_LED_Green(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state |= 0x20;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);

		//soft_delay(250 * SOFT_MSECS);
	
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state &= ~0x20;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
   }	   
}

void Keypad_Toggle_P_Off(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state |= 0x40;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);

		//soft_delay(250 * SOFT_MSECS);
	
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state &= ~0x40;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
   }	   
}

void Keypad_Toggle_P_Off_En(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state |= 0x80;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);

		//soft_delay(250 * SOFT_MSECS);
	
	   state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	   state &= ~0x80;
	   write_mcp23018(IO_ADDRESS_KPD, GPIOA, state);
   }	   
}
