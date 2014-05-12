////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   voltage_monitor_test_menu.c                                              //
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
#include "voltage_monitor_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "adc.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define  VIN_CHANNEL     2
#define  VIN_PIN         AVR32_ADC_AD_2_PIN
#define  VIN_FUNCTION    AVR32_ADC_AD_2_FUNCTION

#define  VBAT_CHANNEL    3
#define  VBAT_PIN        AVR32_ADC_AD_3_PIN
#define  VBAT_FUNCTION   AVR32_ADC_AD_3_FUNCTION

static const gpio_map_t ADC_GPIO_MAP =
{
   {VIN_PIN, VIN_FUNCTION},
   {VBAT_PIN, VBAT_FUNCTION}
};

volatile avr32_adc_t *adc = &AVR32_ADC; // ADC IP registers address

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void Voltage_Monitor_Test_Menu(void)
{
   // Assign and enable GPIO pins to the ADC function.
   gpio_enable_module(ADC_GPIO_MAP, sizeof(ADC_GPIO_MAP) / sizeof(ADC_GPIO_MAP[0]));

   // configure ADC
   adc_configure(adc);

   // Enable the ADC channels.
   adc_enable(adc,VIN_CHANNEL);
   adc_enable(adc,VBAT_CHANNEL);

   Menu_Items = (sizeof(Voltage_Monitor_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Voltage_Monitor_Test_Menu_Functions;
   Menu_String = (unsigned char *)Voltage_Monitor_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  Voltage_Monitor_Exit                                                //
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
void Voltage_Monitor_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void Voltage_Monitor_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = Voltage_Monitor_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = Voltage_Monitor_Info_Text[count];
		}
}

void Voltage_Monitor_Battery_Menu(void)
{
   Menu_Items = (sizeof(Voltage_Monitor_Battery_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Voltage_Monitor_Battery_Test_Menu_Functions;
   Menu_String = (unsigned char *)Voltage_Monitor_Battery_Test_Menu_Text;
}

void Voltage_Monitor_Vin_Menu(void)
{
   Menu_Items = (sizeof(Voltage_Monitor_Vin_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Voltage_Monitor_Vin_Test_Menu_Functions;
   Menu_String = (unsigned char *)Voltage_Monitor_Vin_Test_Menu_Text;
}


void Voltage_Monitor_Battery_Menu_Exit(void)
{
   Menu_Items = (sizeof(Voltage_Monitor_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Voltage_Monitor_Test_Menu_Functions;
   Menu_String = (unsigned char *)Voltage_Monitor_Test_Menu_Text;
}

void Voltage_Monitor_Battery_Read_Continuous(void)
{
   unsigned long int adc_value = 0;
   int whole;
   int decimal;
   int *uart_rx_character = 0;
	unsigned int i = 0;
   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
		// Need to average 25 reads
		for(i = 0; i < 25; i++)
		{
			adc_start(adc);
			adc_value += adc_get_value(adc, VBAT_CHANNEL);
		}

		adc_value /= 25;
		
		// Raw A/D value / 1023 * 3.3 * 3
		whole = adc_value / 103;
		decimal = ((adc_value % 103) * 100) / 103;

		// display value to user
		print_dbg("Battery Voltage = ");
		print_dbg_ulong(whole);
		print_dbg(".");
		print_dbg_ulong(decimal);
		print_dbg(" (0x");
		print_dbg_ulong(adc_value);
		print_dbg(")");
		print_dbg("\r");
   }
   print_dbg("\n\r\n\r");
}

void Voltage_Monitor_Battery_Read_Once(void)
{
	unsigned long int adc_value = 0;
	int whole;
	int decimal;
	unsigned int i = 0;

	for(i = 0; i < 25; i++)
	{
		adc_start(adc);
		adc_value += adc_get_value(adc, VBAT_CHANNEL);
	}

	adc_value /= 25;

	// Raw A/D value / 1023 * 3.3 * 3
	whole = adc_value / 103;
	decimal = ((adc_value % 103) * 100) / 103;

	// display value to user
	print_dbg("Battery Voltage = ");
	print_dbg_ulong(whole);
	print_dbg(".");
	print_dbg_ulong(decimal);
	print_dbg(" (0x");
	print_dbg_ulong(adc_value);
	print_dbg(")");
	print_dbg("\r\n");
}


void Voltage_Monitor_Vin_Menu_Exit(void)
{
   Menu_Items = (sizeof(Voltage_Monitor_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Voltage_Monitor_Test_Menu_Functions;
   Menu_String = (unsigned char *)Voltage_Monitor_Test_Menu_Text;
}

void Voltage_Monitor_Vin_Read_Continuous(void)
{
   int adc_value;
   int whole;
   int decimal;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {
      adc_start(adc);
      adc_value = adc_get_value(adc, VIN_CHANNEL);
      adc_value *= 1300;
      adc_value /= 1000;
      whole = adc_value / 100;
      decimal = adc_value - (whole * 100);

      // display value to user
      print_dbg("External Voltage = ");
      print_dbg_ulong(whole);
      print_dbg(".");
      print_dbg_ulong(decimal);
      print_dbg("\r");
   }
   print_dbg("\n\r\n\r");
}
void Voltage_Monitor_Vin_Read_Once(void)
{
   int adc_value;
   int whole;
   int decimal;

   adc_start(adc);
   adc_value = adc_get_value(adc, VIN_CHANNEL);
   adc_value *= 1300;
   adc_value /= 1000;
   whole = adc_value / 100;
   decimal = adc_value - (whole * 100);
   // display value to user
   print_dbg("External Voltage = ");
   print_dbg_ulong(whole);
   print_dbg(".");
   print_dbg_ulong(decimal);
   print_dbg("\r\n");
}



