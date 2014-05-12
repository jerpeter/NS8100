////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   nsmarts_test_menu.c                                                      //
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

#if 0
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "craft.h"
#include "nsmarts_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define  SEISMIC_SENSOR   0
#define  AIR_SENSOR       1

#define  SA_DATA_IN_PIN    AVR32_PIN_PB01
#define  S_DATA_OUT_PIN    AVR32_PIN_PB02
#define  A_DATA_OUT_PIN    AVR32_PIN_PB03

#define  READ_ROM_COMMAND     0x33
#define  MATCH_ROM_COMMAND    0x55
#define  SEARCH_ROM_COMMAND   0xF0
#define  SKIP_ROM_COMMAND     0xCC

#define  WRITE_STATUS_COMMAND  0x55
#define  WRITE_MEMORY_COMMAND  0x0F
#define  READ_DATA_COMMAND     0xC3
#define  READ_STATUS_COMMAND   0xAA
#define  READ_MEMORY_COMMAND   0xF0

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
static void NSMARTS_Write_One(int Sensor)
{
	int count;

	if(Sensor == AIR_SENSOR)
    {
    	gpio_clr_gpio_pin(A_DATA_OUT_PIN);
    	count=15;
    	while(count-- > 0){}
    	gpio_set_gpio_pin(A_DATA_OUT_PIN);
    	count=45;
    	while(count-- > 0){}
    }
    else
    {
    	gpio_clr_gpio_pin(S_DATA_OUT_PIN);
    	count=15;
    	while(count-- > 0){}
    	gpio_set_gpio_pin(S_DATA_OUT_PIN);
    	count=45;
    	while(count-- > 0){}
    }
}

static void NSMARTS_Write_Zero(int Sensor)
{
	int count;

	if(Sensor == AIR_SENSOR)
    {
    	gpio_clr_gpio_pin(A_DATA_OUT_PIN);
    	count=60;
    	while(count-- > 0){}
    	gpio_set_gpio_pin(A_DATA_OUT_PIN);
    }
    else
    {
    	gpio_clr_gpio_pin(S_DATA_OUT_PIN);
    	count=60;
    	while(count-- > 0){}
    	gpio_set_gpio_pin(S_DATA_OUT_PIN);
    }
}

static unsigned char NSMARTS_Read_Bit(int Sensor)
{
	unsigned char state;
	int count;

	if(Sensor == AIR_SENSOR)
    {
    	gpio_clr_gpio_pin(A_DATA_OUT_PIN);
    	count=1;
    	while(count-- > 0){}
    	gpio_set_gpio_pin(A_DATA_OUT_PIN);
    	count=14;
    	while(count-- > 0){}
   	    state = gpio_get_pin_value(SA_DATA_IN_PIN);
    	count=45;
    	while(count-- > 0){}
    }
    else
    {
    	gpio_clr_gpio_pin(S_DATA_OUT_PIN);
    	count=1;
    	while(count-- > 0){}
    	gpio_set_gpio_pin(S_DATA_OUT_PIN);
    	count=14;
    	while(count-- > 0){}
   	    state = gpio_get_pin_value(SA_DATA_IN_PIN);
    	count=45;
    	while(count-- > 0){}
    }
    return state;
}

static void NSMARTS_Write_Byte(int Sensor, unsigned char Data)
{
    if(Data & 0x01)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x02)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x04)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x08)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x10)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x20)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x40)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
    if(Data & 0x80)
    {
    	NSMARTS_Write_One(Sensor);
    }
    else
    {
    	NSMARTS_Write_Zero(Sensor);
    }
}

static unsigned char NSMARTS_Read_Byte(int Sensor)
{
    unsigned char data = 0;

	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x01;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x02;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x04;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x08;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x10;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x20;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x40;
	}
	if(NSMARTS_Read_Bit(Sensor))
	{
	    data += 0x80;
	}
    return data;
}

void NSMARTS_Test_Menu(void)
{
    Menu_Items = (sizeof(NSMARTS_Test_Menu_Functions)/sizeof(NONE))/4;
    Menu_Functions = (unsigned long *)NSMARTS_Test_Menu_Functions;
    Menu_String = (unsigned char *)NSMARTS_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  NSMARTS_Exit                                                        //
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
void NSMARTS_Exit( void )
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void NSMARTS_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = NSMARTS_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = NSMARTS_Info_Text[count];
		}
}

void NSMARTS_Seismic_Menu(void)
{
   gpio_set_gpio_pin(A_DATA_OUT_PIN);
   Menu_Items = (sizeof(NSMARTS_Seismic_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)NSMARTS_Seismic_Menu_Functions;
   Menu_String = (unsigned char *)NSMARTS_Seismic_Menu_Text;
}

void NSMARTS_Air_Menu(void)
{
   gpio_set_gpio_pin(S_DATA_OUT_PIN);
   Menu_Items = (sizeof(NSMARTS_Air_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)NSMARTS_Air_Menu_Functions;
   Menu_String = (unsigned char *)NSMARTS_Air_Menu_Text;
}


void NSMARTS_Seismic_Exit(void)
{
   Menu_Items = (sizeof(NSMARTS_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)NSMARTS_Test_Menu_Functions;
   Menu_String = (unsigned char *)NSMARTS_Test_Menu_Text;
}

void NSMARTS_Seismic_Continuous_Pulse(void)
{
   int count;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. ");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
      count=100;
      while(count-- > 0){}
	  gpio_clr_gpio_pin(S_DATA_OUT_PIN);
      count=100;
      while(count-- > 0){}
      gpio_set_gpio_pin(S_DATA_OUT_PIN);
   }
   print_dbg("\n\r\n\r");
}

void NSMARTS_Seismic_One_Pulse(void)
{
   int count;
   count=100;
   while(count-- > 0){}
   gpio_clr_gpio_pin(S_DATA_OUT_PIN);
   count=100;
   while(count-- > 0){}
   gpio_set_gpio_pin(S_DATA_OUT_PIN);
}

void NSMARTS_Seismic_Set_Low(void)
{
  gpio_clr_gpio_pin(S_DATA_OUT_PIN);
}

void NSMARTS_Seismic_Set_High(void)
{
  gpio_set_gpio_pin(S_DATA_OUT_PIN);
}
void NSMARTS_Seismic_Reset_Sensor(void)
{
   int count;

   gpio_clr_gpio_pin(S_DATA_OUT_PIN);
   count=500;
   while(count-- > 0){}
   gpio_set_gpio_pin(S_DATA_OUT_PIN);
}

void NSMARTS_Seismic_Detect_Sensor(void)
{
   int count;
   int state = 0;

   NSMARTS_Seismic_Reset_Sensor();
   count = 65;
   while(count-- > 0){}
   state = gpio_get_pin_value(SA_DATA_IN_PIN);
   print_dbg("\n\rSeismic sensor = ");
   if(state==0)
   {
	   print_dbg("Present.\n\r");
   }
   else
   {
	   print_dbg("Absent.\n\r");
   }
   print_dbg("\n\r");
}

void NSMARTS_Seismic_Read_Info(void)
{
	int i;
	unsigned char data[8];

    NSMARTS_Seismic_Reset_Sensor();

    print_dbg("\n\rData from Seismic sensor = ");
	NSMARTS_Write_Byte(SEISMIC_SENSOR, READ_ROM_COMMAND);

	data[0] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
	print_dbg_char_hex(data[0]);
    print_dbg(" ");

    data[6] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
    data[5] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
    data[4] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
    data[3] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
    data[2] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
    data[1] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
    for(i=1;i<7;i++)
    {
        print_dbg_char_hex(data[i]);
        print_dbg(" ");
    }
    data[7] = NSMARTS_Read_Byte(SEISMIC_SENSOR);
	print_dbg_char_hex(data[7]);
    print_dbg("\n\r\n\r");
}

void NSMARTS_Seismic_Test_Read_Write(void){}

void NSMARTS_Air_Exit(void)
{
   Menu_Items = (sizeof(NSMARTS_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)NSMARTS_Test_Menu_Functions;
   Menu_String = (unsigned char *)NSMARTS_Test_Menu_Text;
}
void NSMARTS_Air_Continuous_Pulse(void)
{
   int count;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. ");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
      count=100;
      while(count-- > 0){}
	  gpio_clr_gpio_pin(A_DATA_OUT_PIN);
      count=100;
      while(count-- > 0){}
      gpio_set_gpio_pin(A_DATA_OUT_PIN);
   }
   print_dbg("\n\r\n\r");
}

void NSMARTS_Air_One_Pulse(void)
{
	int count;
	count=100;
	while(count-- > 0){}
	gpio_clr_gpio_pin(A_DATA_OUT_PIN);
	count=100;
	while(count-- > 0){}
	gpio_set_gpio_pin(A_DATA_OUT_PIN);
}

void NSMARTS_Air_Set_Low(void)
{
   gpio_clr_gpio_pin(A_DATA_OUT_PIN);
}

void NSMARTS_Air_Set_High(void)
{
   gpio_set_gpio_pin(A_DATA_OUT_PIN);
}

void NSMARTS_Air_Reset_Sensor(void)
{
   int count;

   gpio_clr_gpio_pin(A_DATA_OUT_PIN);
   count=500;
   while(count-- > 0){}
   gpio_set_gpio_pin(A_DATA_OUT_PIN);
}

void NSMARTS_Air_Detect_Sensor(void)
{
   int count;
   int state = 0;

   NSMARTS_Air_Reset_Sensor();
   count = 65;
   while(count-- > 0){}
   state = gpio_get_pin_value(SA_DATA_IN_PIN);
   print_dbg("\n\rAir sensor = ");
   if(state==0)
   {
	   print_dbg("Present.\n\r");
   }
   else
   {
	   print_dbg("Absent.\n\r");
   }
   print_dbg("\n\r");
}

void NSMARTS_Air_Read_Info(void)
{
	int i;
	unsigned char data[8];

    NSMARTS_Air_Reset_Sensor();

    print_dbg("\n\rData from Air sensor = ");
	NSMARTS_Write_Byte(AIR_SENSOR, READ_ROM_COMMAND);

	data[0] = NSMARTS_Read_Byte(AIR_SENSOR);
	print_dbg_char_hex(data[0]);
    print_dbg(" ");

    data[6] = NSMARTS_Read_Byte(AIR_SENSOR);
    data[5] = NSMARTS_Read_Byte(AIR_SENSOR);
    data[4] = NSMARTS_Read_Byte(AIR_SENSOR);
    data[3] = NSMARTS_Read_Byte(AIR_SENSOR);
    data[2] = NSMARTS_Read_Byte(AIR_SENSOR);
    data[1] = NSMARTS_Read_Byte(AIR_SENSOR);
    for(i=1;i<7;i++)
    {
        print_dbg_char_hex(data[i]);
        print_dbg(" ");
    }
    data[7] = NSMARTS_Read_Byte(AIR_SENSOR);
	print_dbg_char_hex(data[7]);
    print_dbg("\n\r\n\r");
}
void NSMARTS_Air_Test_Read_Write(void){}
#endif
