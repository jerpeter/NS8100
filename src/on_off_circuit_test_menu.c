////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   on_off_circuit_test_menu.c                                               //
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
#include "on_off_circuit_test_menu.h"
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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void On_Off_Circuit_Test_Menu(void)
{
	twi_resources_init();
    Menu_Items = (sizeof(On_Off_Circuit_Test_Menu_Functions)/sizeof(NONE))/4;
    Menu_Functions = (unsigned long *)On_Off_Circuit_Test_Menu_Functions;
    Menu_String = (unsigned char *)On_Off_Circuit_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  On_Off_Circuit_Exit                                                 //
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
void On_Off_Circuit_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void On_Off_Circuit_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = On_Off_Circuit_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = On_Off_Circuit_Info_Text[count];
		}
}
void On_Off_Circuit_Off_Lockout_Test(void)
{
	unsigned char state = 0;
	int uart_rx_character = 0;

	print_dbg("Power off protection: ");

	init_mcp23018(IO_ADDRESS_KPD);
	state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	state |= 0x80;
	write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
	
	print_dbg("Enabled\n\n");
	print_dbg("Until any key is pressed, you should not be able to power off the unti with the Off key\n\n");

	while((usart_read_char(DBG_USART, &uart_rx_character))!= USART_SUCCESS) {}	

	print_dbg("Power off protection: ");

	init_mcp23018(IO_ADDRESS_KPD);
	state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	state &= ~0x80;
	write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
	
	print_dbg("Disabled\n\n");
}

void On_Off_Circuit_Off_Test(void)
{
	unsigned char state = 0;
	int uart_rx_character = 0;
	unsigned long int countdownTimer = 8000000;

	print_dbg("WARNING: This test will turn the unit off.\n\n");
	print_dbg("Press any key to continue\n\n");

	while((usart_read_char(DBG_USART, &uart_rx_character))!= USART_SUCCESS) {}

	print_dbg("Powering off...\n\n");
	init_mcp23018(IO_ADDRESS_KPD);
	state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	state |= 0x40;
	write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
	
	while(countdownTimer--) {}
}


