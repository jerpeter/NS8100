/********************************************************
 Name          : main.c
 Author        : WISCH Systems
 Copyright     : Your copyright notice
 Description   : EVK1100 template
 **********************************************************/

// Include Files
#include "usart.h"
#include "print_funcs.h"
#include "craft.h"

void (*Main_Menu_Functions[MAIN_MENU_FUNCTIONS_ITEMS])(void) =
{
  NONE,					// 1
  AD_Test_Menu,			// 2
  NSMARTS_Test_Menu,	// 3
  Alarm_Test_Menu,		// 4
  Trigger_Test_Menu,	// 5
  Keypad_Test_Menu,		// 6 
  On_Off_Key_Test_Menu, // 7
  Voltage_Monitor_Test_Menu, // 8
  SD_MMC_Test_Menu,		// 9
  RTC_Test_Menu,		// 10
  EEPROM_Test_Menu,		// 11
  SRAM_Test_Menu,		// 12
  LCD_Test_Menu,		// 13
  Network_Test_Menu,	// 14
  On_Off_Circuit_Test_Menu,	// 15
  RS485_Test_Menu,		// 16
  RS232_Test_Menu,		// 17
  USB_Test_Menu,		// 18
  Proc_Timers_Test_Menu	// 19
};

void NONE( void )
{
    print_dbg("\n\r\n\r");
}

void Command_Prompt(void)
{
   print_dbg("\n\r  NCI>");
}

int Get_User_Input(unsigned char *str)
{
    unsigned char *b = str;
    unsigned char end = FALSE;
    unsigned char  data;
    int  count = 0;

    do
    {
        data = usart_getchar(DBG_USART);
        switch( data )
        {
	        case '\b':
	        case 0x7e:
	            if( count )
	            {
	                count--;
	                b--;
	                print_dbg("\b \b");
                }
                break;

	        case '\r':
            case '\n':
//                if( count )
//                {
                    *b = 0;
	                print_dbg("\r\n");
                    end = TRUE;
//                }
                break;

            default:
                if( count < 80 )
                {
                    count++;
                    *b++ = data;
                    print_dbg_char(data);
                }
                break;
	    }
	}while( !end );

    *b = 0;

    return count;
}
