////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   rs485_test_menu.c                                                        //
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

#if 0
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "craft.h"
#include "Typedefs.h"
#include "rs485_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define RS485_USART                 (&AVR32_USART3)
#define RS485_USART_RX_PIN          AVR32_USART3_RXD_0_0_PIN
#define RS485_USART_RX_FUNCTION     AVR32_USART3_RXD_0_0_FUNCTION
#define RS485_USART_TX_PIN          AVR32_USART3_TXD_0_0_PIN
#define RS485_USART_TX_FUNCTION     AVR32_USART3_TXD_0_0_FUNCTION
#define RS485_USART_TXEN_PIN        AVR32_USART3_RTS_0_1_PIN
#define RS485_USART_TXEN_FUNCTION   AVR32_USART3_RTS_0_1_FUNCTION
#define RS485_USART_BAUDRATE        115200


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void init_rs485_ex(unsigned long baudrate, long pba_hz)
{
  static const gpio_map_t RS485_USART_GPIO_MAP =
  {
    {RS485_USART_RX_PIN, RS485_USART_RX_FUNCTION},
    {RS485_USART_TX_PIN, RS485_USART_TX_FUNCTION},
    {RS485_USART_TXEN_PIN, RS485_USART_TXEN_FUNCTION}
  };

  // Options for debug USART.
  usart_options_t rs485_usart_options =
  {
    .baudrate = baudrate,
    .charlength = 8,
    .paritytype = USART_NO_PARITY,
    .stopbits = USART_1_STOPBIT,
    .channelmode = USART_NORMAL_CHMODE
  };

  // Setup GPIO for debug USART.
  gpio_enable_module(RS485_USART_GPIO_MAP,
                     sizeof(RS485_USART_GPIO_MAP) / sizeof(RS485_USART_GPIO_MAP[0]));

  // Initialize it in RS485 mode.
  usart_init_rs485(RS485_USART, &rs485_usart_options, pba_hz);
}


void RS485_Test_Menu(void)
{
   init_rs485_ex(RS485_USART_BAUDRATE,FOSC0);
   Menu_Items = (sizeof(RS485_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)RS485_Test_Menu_Functions;
   Menu_String = (unsigned char *)RS485_Test_Menu_Text;
}



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  RS485_Exit                                                          //
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
void RS485_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void RS485_Info(void)
{
   int count;
   unsigned char character;

    count = 0;
	character = RS485_Info_Text[count];
	while(character != 0)
	{
		print_dbg_char(character);
		count++;
		character = RS485_Info_Text[count];
	}
}
void RS485_Transmit_Character_Once_Test(void){}
void RS485_Transmit_Character_Continuous_Test(void){}
void RS485_Echo_Received_Character_Once_Test(void){}
void RS485_Echo_Received_Character_Continuous_Test(void){}
void RS485_Pulse_Transmit_Enable_Once_Test(void){}
void RS485_Pulse_Transmit_Enable_Continuous_Test(void){}
void RS485_Loopback_Test(void)
{
   int count = 0;
   int out_character;
   int in_character;
   int status = PASSED;

   print_dbg("\n\rTesting RS485 Loopback. \n\r\n\r");

	out_character = RS485_Test_Text[count];
	while(out_character != 0)
	{
	    usart_putchar(DBG_USART, out_character);
	    usart_putchar(RS485_USART, out_character);
	    in_character = usart_getchar(RS485_USART);
        if(in_character != out_character)
        {
        	status = FAILED;
        }
		count++;
		out_character = RS485_Test_Text[count];
	}
    if(status == FAILED)
    {
        print_dbg("\n\r\n\rRS485 Loopback FAILED.\n\r");
    }
    else
    {
        print_dbg("\n\r\n\rRS485 Loopback PASSED.\n\r");
    }
}
#endif
