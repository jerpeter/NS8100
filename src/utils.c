/********************************************************
 Name          : main.c
 Author        : Joseph Getz
 Copyright     : Your copyright notice
 Description   : EVK1100 template
 **********************************************************/

// Include Files
#include "board.h"
#include "usart.h"
#include "print_funcs.h"

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


