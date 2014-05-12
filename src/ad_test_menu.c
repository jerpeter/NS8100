////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   ad_test_menu.c                                                           //
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
//   $Date: 2012/04/26 01:05:17 $                                                                 //
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
#include "ad_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
gpio_map_t AD_CTL_SPI_GPIO_MAP =
{
	{AD_CTL_SPI_SCK_PIN,  AD_CTL_SPI_SCK_FUNCTION },  // SPI Clock.
	{AD_CTL_SPI_MISO_PIN, AD_CTL_SPI_MISO_FUNCTION},  // MISO.
	{AD_CTL_SPI_MOSI_PIN, AD_CTL_SPI_MOSI_FUNCTION},  // MOSI.
	{AD_CTL_SPI_NPCS_PIN, AD_CTL_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
};

// GPIO pins used for SD/MMC interface
gpio_map_t AD_SPI_GPIO_MAP =
{
	{AD_SPI_SCK_PIN,  AD_SPI_SCK_FUNCTION },  // SPI Clock.
	{AD_SPI_MISO_PIN, AD_SPI_MISO_FUNCTION},  // MISO.
	{AD_SPI_MOSI_PIN, AD_SPI_MOSI_FUNCTION},  // MOSI.
	{AD_SPI_NPCS_PIN, AD_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
};

static const unsigned char AD_Info_Text[] =
{
"\n\r\n\r"
"   Part = AD7689                  Analog Voltage = 5V\n\r"
"   Interface = SPI0-CS0           Digital Voltage = 3.3\n\r"
"   Ref. Des. = U3                 Input Range = 5v\n\r"
"   Sampling Speed = 250K          Reference = External ADR125 (U9)\n\r"
"   Bits = 16\n\r"
"\n\r"
"   Channels:\n\r"
"     1) Radial            2) Transverse\n\r"
"     3) Vertical          4) Air(Normal/A-Weighted)\n\r"
"     5) Empty             6) Empty\n\r"
"     7) Empty             8) Empty\n\r"
"\n\r"
"   Control Register:\n\r"
"     Part = 74HC595               Interface = SPI1-CS3\n\r"
"     Ref. Des = U10               Digital Voltage = 3.3\n\r"
"\n\r"
"   Control Bits:\n\r"
"     1) Geophone Gain( 0 = High(2X), 1 = Low(1x))\n\r"
"     2) Air Type( 0 = Normal, 1 = A-Weighted)\n\r"
"     3-5) Filter 0,1 and Enable\n\r"
"        5  4  3   |   Range\n\r"
"        ----------+--------------\n\r"
"        0  x  x   |   Filter 1\n\r"
"        1  0  0   |   Filter 2\n\r"
"        1  0  1   |   Filter 3\n\r"
"        1  1  0   |   Filter 4\n\r"
"        1  1  1   |   Filter 5\n\r"
"     6) Empty\n\r"
"     7) Cal_Sig ( 0 = Cal Low(0V), 1 = Cal High(5V))\n\r"
"     8) Cal_Sig Enable ( 0 = Cal_Sig Off, 1 = Cal_Sig Active)\n\r"
"\n\r"
"\0"
};

static const unsigned char AD_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D TEST MENU                                บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) A/D Information.\n\r"
"   2) A/D CS Test.\n\r"
"   3) A/D CLK Test.\n\r"
"   4) A/D Data Out Test.\n\r"
"   5) A/D Data In Test.\n\r"
"   6) A/D Read Test.\n\r"
"   7) A/D Set Gain Test.\n\r"
"   8) A/D Set Filter Test.\n\r"
"   9) A/D Set Air Test.\n\r"
"  10) A/D Cal Pulse Test.\n\r"
"\0"
};

static void (*AD_Test_Menu_Functions[])(void) =
{
   AD_Exit,
   AD_Info,
   AD_CS_Menu,
   AD_CLK_Menu,
   AD_DOUT_Menu,
   AD_DIN_Menu,
   AD_Read_Menu,
   AD_Gain_Menu,
   AD_Filter_Menu,
   AD_Air_Menu,
   AD_Cal_Menu
};

static const unsigned char AD_CS_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 A/D CS MENU                                 บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Pulse CS continuously.\n\r"
"   2) Pulse CS one time.\n\r"
"   3) Set CS High.\n\r"
"   4) Set CS Low.\n\r"
"\0"
};

static void (*AD_CS_Menu_Functions[])(void) =
{
   AD_CS_Exit,
   AD_CS_Continuous_Pulse,
   AD_CS_One_Pulse,
   AD_CS_Set_High,
   AD_CS_Set_Low
};

static const unsigned char AD_CLK_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D CLK MENU                                 บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Pulse CLK continuously.\n\r"
"   2) Pulse CLK one time.\n\r"
"   3) Set CLK High.\n\r"
"   4) Set CLK Low.\n\r"
"\0"
};

static void (*AD_CLK_Menu_Functions[])(void) =
{
   AD_CLK_Exit,
   AD_CLK_Continuous_Pulse,
   AD_CLK_One_Pulse,
   AD_CLK_Set_High,
   AD_CLK_Set_Low
};

static const unsigned char AD_DOUT_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D DOUT MENU                                บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Pulse DOUT continuously.\n\r"
"   2) Pulse DOUT one time.\n\r"
"   3) Set DOUT High.\n\r"
"   4) Set DOUT Low.\n\r"
"\0"
};

static void (*AD_DOUT_Menu_Functions[])(void) =
{
   AD_DOUT_Exit,
   AD_DOUT_Continuous_Pulse,
   AD_DOUT_One_Pulse,
   AD_DOUT_Set_High,
   AD_DOUT_Set_Low
};

static const unsigned char AD_DIN_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 A/D DIN MENU                                บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Pulse DIN continuously.\n\r"
"   2) Pulse DIN one time.\n\r"
"   3) Set DIN High.\n\r"
"   4) Set DIN Low.\n\r"
"\0"
};

static void (*AD_DIN_Menu_Functions[])(void)=
{
   AD_DIN_Exit,
   AD_DIN_Continuous_Pulse,
   AD_DIN_One_Pulse,
   AD_DIN_Set_High,
   AD_DIN_Set_Low
};

static const unsigned char AD_Read_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D READ MENU                                บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Continuous R.V,T and A.\n\r"
"   2) Continuous Radial.\n\r"
"   3) Continuous Vertical.\n\r"
"   4) Continuous Transverse.\n\r"
"   5) Continuous Air.\n\r"
"   6) Once R.V,T and A.\n\r"
"   7) Once Radial.\n\r"
"   8) Once Vertical.\n\r"
"   9) Once Transverse.\n\r"
"  10) Once Air.\n\r"
"  11) Status Register.\n\r"
"\0"
};

static void (*AD_Read_Menu_Functions[])(void) =
{
 AD_Read_Exit,
 AD_Read_Cont_RVTA,
 AD_Read_Cont_Radial,
 AD_Read_Cont_Vertical,
 AD_Read_Cont_Transverse,
 AD_Read_Cont_Air,
 AD_Read_Once_RVTA,
 AD_Read_Once_Radial,
 AD_Read_Once_Vertical,
 AD_Read_Once_Transverse,
 AD_Read_Once_Air,
 AD_Read_Status
};

static const unsigned char AD_Gain_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D GAIN MENU                                บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Set R,V and T Gain Low.\n\r"
"   2) Set R,V and T Gain High.\n\r"
"   3) Set A Gain Low.\n\r"
"   4) Set A Gain High.\n\r"
"\0"
};

static void (*AD_Gain_Menu_Functions[])(void) =
{
   AD_Gain_Exit,
   AD_Gain_Set_RVT_Low,
   AD_Gain_Set_RVT_High,
   AD_Gain_Set_Air_Low,
   AD_Gain_Set_Air_High
};

static const unsigned char AD_Filter_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                         NS8100 A/D FILTER MENU                               บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Set R,V,T and A Filter 1.\n\r"
"   2) Set R,V,T and A Filter 2.\n\r"
"   3) Set R,V,T and A Filter 3.\n\r"
"   4) Set R,V,T and A Filter 4.\n\r"
"   5) Set R,V,T and A Filter 5.\n\r"
"\0"
};

static void (*AD_Filter_Menu_Functions[])(void)=
{
    AD_Filter_Exit,
    AD_Filter_Set_1,
    AD_Filter_Set_2,
    AD_Filter_Set_3,
    AD_Filter_Set_4,
    AD_Filter_Set_5
};

static const unsigned char AD_Air_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D AIR MENU                                 บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Set A-Weighting.\n\r"
"   2) Set Normal.\n\r"
"\0"
};

static void (*AD_Air_Menu_Functions[])(void) =
{
   AD_Air_Exit,
   AD_Air_Set_A_Weighting,
   AD_Air_Set_Normal,
};

static const unsigned char AD_Cal_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                          NS8100 A/D CAL MENU                                 บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Continuous Pulse.\n\r"
"   2) One Time Pulse.\n\r"
"   3) Set Cal Low.\n\r"
"   4) Set Cal High.\n\r"
"   5) Set Cal Middle.\n\r"
"\0"
};

static void (*AD_Cal_Menu_Functions[])(void) =
{
   AD_Cal_Exit,
   AD_Cal_Set_Continuous_Pulse,
   AD_Cal_Set_One_Pulse,
   AD_Cal_Set_Low,
   AD_Cal_Set_High,
   AD_Cal_Set_Middle
};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 0
static void AD_control_resources_init(void)
{
  // SPI options.
  spi_options_t spiOptions =
  {
    .reg          = AD_CTL_SPI_NPCS,
    .baudrate     = AD_CTL_SPI_MASTER_SPEED,  // Defined in conf_sd_mmc.h.
    .bits         = AD_CTL_SPI_BITS,          // Defined in conf_sd_mmc.h.
    .spck_delay   = 0,
    .trans_delay  = 0,
    .stay_act     = 1,
    .spi_mode     = 0,
    .modfdis      = 1
  };

  // Assign I/Os to SPI.
  gpio_enable_module(AD_CTL_SPI_GPIO_MAP,
                     sizeof(AD_CTL_SPI_GPIO_MAP) / sizeof(AD_CTL_SPI_GPIO_MAP[0]));

  // Initialize as master.
  spi_initMaster(AD_CTL_SPI, &spiOptions);

  // Set SPI selection mode: variable_ps, pcs_decode, delay.
  spi_selectionMode(AD_CTL_SPI, 0, 0, 0);

  // Enable SPI module.
  spi_enable(AD_CTL_SPI);

  // Initialize AD driver with SPI clock (PBA).
  // Setup SPI registers according to spiOptions.
  spi_setupChipReg(AD_CTL_SPI, &spiOptions, FOSC0);
}
#endif

#if 1
static void AD_resources_init(void)
{
  // SPI options.
  spi_options_t spiOptions =
  {
    .reg          = AD_SPI_NPCS,
    .baudrate     = AD_SPI_MASTER_SPEED,  // Defined in conf_sd_mmc.h.
    .bits         = AD_SPI_BITS,          // Defined in conf_sd_mmc.h.
    .spck_delay   = 0,
    .trans_delay  = 0,
    .stay_act     = 1,
    .spi_mode     = 0,
    .modfdis      = 1
  };

  // Assign I/Os to SPI.
  gpio_enable_module(AD_SPI_GPIO_MAP,
                     sizeof(AD_SPI_GPIO_MAP) / sizeof(AD_SPI_GPIO_MAP[0]));

  // Initialize as master.
  spi_initMaster(AD_SPI, &spiOptions);

  // Set SPI selection mode: variable_ps, pcs_decode, delay.
  spi_selectionMode(AD_SPI, 0, 0, 0);

  // Enable SPI module.
  spi_enable(AD_SPI);

  // Initialize AD driver with SPI clock (PBA).
  // Setup SPI registers according to spiOptions.
  spi_setupChipReg(AD_SPI, &spiOptions, FOSC0);
}
#endif

static void AD_control_write(unsigned int control_byte)
{
	spi_selectChip(AD_CTL_SPI, AD_CTL_SPI_NPCS);
	spi_write(AD_CTL_SPI, (unsigned short) control_byte);
    spi_unselectChip(AD_CTL_SPI, AD_CTL_SPI_NPCS);
}

static void AD_config_write(unsigned int config_word)
{
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, ((unsigned short) config_word << 2));
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
}

void AD_Init(void)
{
    int temp;

	//AD_control_resources_init();
	AD_resources_init();
	AD_control_write(0x00);

	// Setup config for 4 Chan + Temp + Read back config + others
	//AD_config_write(0x39B4);

	// Setup config for 4 Chan, No Temp, No read back
	AD_config_write(0x39FF);

	//Delay for 1.2us at least
	for(temp=0;temp<100;temp++) {}

	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, 0x0000);
    spi_unselectChip(AD_SPI, AD_SPI_NPCS);

	for(temp=0;temp<100;temp++) {}
}

void AD_Test_Menu(void)
{
      Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
      Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
      Menu_String = (unsigned char *)AD_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  AD_Exit                                                             //
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
void AD_Exit( void )
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void AD_Info(void)
{
    int count;
    unsigned char character;

    count = 0;
	character = AD_Info_Text[count];
	while(character != 0)
	{
		print_dbg_char(character);
		count++;
		character = AD_Info_Text[count];
	}
}

void AD_CS_Menu(void)
{
   Menu_Items = (sizeof(AD_CS_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_CS_Menu_Functions;
   Menu_String = (unsigned char *)AD_CS_Menu_Text;
}

void AD_CLK_Menu(void)
{
   Menu_Items = (sizeof(AD_CLK_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_CLK_Menu_Functions;
   Menu_String = (unsigned char *)AD_CLK_Menu_Text;
}

void AD_DOUT_Menu(void)
{
   Menu_Items = (sizeof(AD_DOUT_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_DOUT_Menu_Functions;
   Menu_String = (unsigned char *)AD_DOUT_Menu_Text;
}

void AD_DIN_Menu(void)
{
   Menu_Items = (sizeof(AD_DIN_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_DIN_Menu_Functions;
   Menu_String = (unsigned char *)AD_DIN_Menu_Text;
}

void AD_Read_Menu(void)
{
   Menu_Items = (sizeof(AD_Read_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_Read_Menu_Functions;
   Menu_String = (unsigned char *)AD_Read_Menu_Text;
}

void AD_Gain_Menu(void)
{
   Menu_Items = (sizeof(AD_Gain_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_Gain_Menu_Functions;
   Menu_String = (unsigned char *)AD_Gain_Menu_Text;
}

void AD_Filter_Menu(void)
{
   Menu_Items = (sizeof(AD_Filter_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_Filter_Menu_Functions;
   Menu_String = (unsigned char *)AD_Filter_Menu_Text;
}

void AD_Air_Menu(void)
{
   Menu_Items = (sizeof(AD_Air_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_Air_Menu_Functions;
   Menu_String = (unsigned char *)AD_Air_Menu_Text;
}

void AD_Cal_Menu(void)
{
   Menu_Items = (sizeof(AD_Cal_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)AD_Cal_Menu_Functions;
   Menu_String = (unsigned char *)AD_Cal_Menu_Text;
}


void AD_CS_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}

void AD_CS_Continuous_Pulse(void)
{
    int c;
	print_dbg("\r\n\n\rPress any key to exit.");
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
        gpio_clr_gpio_pin(AD_SPI_NPCS_PIN);
        gpio_set_gpio_pin(AD_SPI_NPCS_PIN);
    }
}

void AD_CS_One_Pulse(void)
{
	gpio_clr_gpio_pin(AD_SPI_NPCS_PIN);
    gpio_set_gpio_pin(AD_SPI_NPCS_PIN);
}

void AD_CS_Set_High(void)
{
    gpio_set_gpio_pin(AD_SPI_NPCS_PIN);
}

void AD_CS_Set_Low(void)
{
	gpio_clr_gpio_pin(AD_SPI_NPCS_PIN);
}

void AD_CLK_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}

void AD_CLK_Continuous_Pulse(void)
{
    int c;
	print_dbg("\r\n\n\rPress any key to exit.");
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
        gpio_clr_gpio_pin(AD_SPI_SCK_PIN);
        gpio_set_gpio_pin(AD_SPI_SCK_PIN);
    }
}

void AD_CLK_One_Pulse(void)
{
    gpio_clr_gpio_pin(AD_SPI_SCK_PIN);
    gpio_set_gpio_pin(AD_SPI_SCK_PIN);
}

void AD_CLK_Set_High(void)
{
    gpio_set_gpio_pin(AD_SPI_SCK_PIN);
}

void AD_CLK_Set_Low(void)
{
    gpio_clr_gpio_pin(AD_SPI_SCK_PIN);
}

void AD_DOUT_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}
void AD_DOUT_Continuous_Pulse(void)
{
	//Stub
}

void AD_DOUT_One_Pulse(void)
{
	//Stub
}

void AD_DOUT_Set_High(void)
{
	//Stub
}

void AD_DOUT_Set_Low(void)
{
	//Stub
}


void AD_DIN_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}

void AD_DIN_Continuous_Pulse(void)
{
    int c;
	print_dbg("\r\n\n\rPress any key to exit.");
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
        gpio_clr_gpio_pin(AD_SPI_MOSI_PIN);
        gpio_set_gpio_pin(AD_SPI_MOSI_PIN);
    }
}

void AD_DIN_One_Pulse(void)
{
    gpio_clr_gpio_pin(AD_SPI_MOSI_PIN);
    gpio_set_gpio_pin(AD_SPI_MOSI_PIN);
}

void AD_DIN_Set_High(void)
{
    gpio_set_gpio_pin(AD_SPI_MOSI_PIN);
}

void AD_DIN_Set_Low(void)
{
    gpio_clr_gpio_pin(AD_SPI_MOSI_PIN);
}

void AD_Read_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}
void AD_Read_Cont_RVTA(void)
{
	unsigned short temp,temp1;
    int c;

    print_dbg("\r\n\n\rPress any key to exit.\n\r");
	AD_Init();
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
    	// Chan 0
		spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 1
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 2
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 3
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 4
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 5
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 5
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 7
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("   ");

    	// Chan 8
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg(" ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("\r\n");
    }
}

void AD_Read_Cont_Radial(void)
{
	unsigned short temp,temp1;
    int c;

    print_dbg("\r\n\n\rPress any key to exit.\n\r");
	AD_Init();
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, (0x3C48 << 2));
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg("  ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("\r\n");
    }
}

void AD_Read_Cont_Vertical(void)
{
	unsigned short temp,temp1;
    int c;

    print_dbg("\r\n\n\rPress any key to exit.\n\r");
	AD_Init();
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, (0x3CC8 << 2));
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg("  ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("\r\n");
    }
}

void AD_Read_Cont_Transverse(void)
{
	unsigned short temp,temp1;
    int c;

    print_dbg("\r\n\n\rPress any key to exit.\n\r");
	AD_Init();
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, (0x3D48 << 2));
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg("  ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("\r\n");
    }
}

void AD_Read_Cont_Air(void)
{
	unsigned short temp,temp1;
    int c;

    print_dbg("\r\n\n\rPress any key to exit.\n\r");
	AD_Init();
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, (0x3DC8 << 2));
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
    	spi_selectChip(AD_SPI, AD_SPI_NPCS);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp);
    	spi_write(AD_SPI, 0x0000);
    	spi_read(AD_SPI, &temp1);
    	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
    	print_dbg_short_hex(temp);
    	print_dbg("  ");
    	print_dbg_short_hex(temp1 >> 2);
    	print_dbg("\r\n");
    }
}

void AD_Read_Once_RVTA(void)
{
	unsigned short temp,temp1;

	print_dbg("\r\n");
	AD_Init();

	spi_selectChip(AD_SPI, AD_SPI_NPCS);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp1);
   	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
   	print_dbg_short_hex(temp);
   	print_dbg("  ");
   	print_dbg_short_hex(temp1 >> 2);
   	print_dbg("  ");

   	spi_selectChip(AD_SPI, AD_SPI_NPCS);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp1);
   	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
   	print_dbg_short_hex(temp);
   	print_dbg("  ");
   	print_dbg_short_hex(temp1 >> 2);
   	print_dbg("  ");

   	spi_selectChip(AD_SPI, AD_SPI_NPCS);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp1);
   	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
   	print_dbg_short_hex(temp);
   	print_dbg("  ");
   	print_dbg_short_hex(temp1 >> 2);
   	print_dbg("  ");

   	spi_selectChip(AD_SPI, AD_SPI_NPCS);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp);
   	spi_write(AD_SPI, 0x0000);
   	spi_read(AD_SPI, &temp1);
   	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
   	print_dbg_short_hex(temp);
   	print_dbg("  ");
   	print_dbg_short_hex(temp1 >> 2);
   	print_dbg("\r\n");
}

void AD_Read_Once_Radial(void)
{
	unsigned short temp,temp1;

	print_dbg("\r\n");
	AD_Init();
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, (0x3C48 << 2));
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);

	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp1);
	print_dbg_short_hex(temp);
	print_dbg("  ");
	print_dbg_short_hex(temp1 >> 2);
	print_dbg("\r\n");
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
}

void AD_Read_Once_Vertical(void)
{
	unsigned short temp,temp1;

	print_dbg("\r\n");
   	spi_write(AD_SPI, (0x3CC8 << 2));
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp1);
	print_dbg_short_hex(temp);
	print_dbg("  ");
	print_dbg_short_hex(temp1 >> 2);
	print_dbg("\r\n");
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
}

void AD_Read_Once_Transverse(void)
{
	unsigned short temp,temp1;

	print_dbg("\r\n");
   	spi_write(AD_SPI, (0x3D48 << 2));
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp1);
	print_dbg_short_hex(temp);
	print_dbg("  ");
	print_dbg_short_hex(temp1 >> 2);
	print_dbg("\r\n");
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
}

void AD_Read_Once_Air(void)
{
	unsigned short temp,temp1;

	print_dbg("\r\n");
   	spi_write(AD_SPI, (0x3DC8 << 2));
	spi_selectChip(AD_SPI, AD_SPI_NPCS);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp);
	spi_write(AD_SPI, 0x0000);
	spi_read(AD_SPI, &temp1);
	print_dbg_short_hex(temp);
	print_dbg("  ");
	print_dbg_short_hex(temp1 >> 2);
	print_dbg("\r\n");
	spi_unselectChip(AD_SPI, AD_SPI_NPCS);
}

void AD_Read_Status(void)
{
	//Stub
}

void AD_Gain_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}
void AD_Gain_Set_RVT_Low(void)
{
	//AD_control_resources_init();
	AD_control_write(0x01);
}

void AD_Gain_Set_RVT_High(void)
{
	//AD_control_resources_init();
	AD_control_write(0x00);
}

void AD_Gain_Set_Air_Low(void)
{
	//AD_control_resources_init();
	AD_control_write(0x02);
}

void AD_Gain_Set_Air_High(void)
{
	//AD_control_resources_init();
	AD_control_write(0x00);
}

void AD_Filter_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}
void AD_Filter_Set_1(void)
{
	//AD_control_resources_init();
	AD_control_write(0x10);
}

void AD_Filter_Set_2(void)
{
	//AD_control_resources_init();
	AD_control_write(0x0C);
}

void AD_Filter_Set_3(void)
{
	//AD_control_resources_init();
	AD_control_write(0x08);
}

void AD_Filter_Set_4(void)
{
	//AD_control_resources_init();
	AD_control_write(0x04);
}

void AD_Filter_Set_5(void)
{
	//AD_control_resources_init();
	AD_control_write(0x00);
}

void AD_Air_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}
void AD_Air_Set_A_Weighting(void)
{
	//Stub
}

void AD_Air_Set_Normal(void)
{
	//Stub
}

void AD_Cal_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)AD_Test_Menu_Functions;
   Menu_String = (unsigned char *)AD_Test_Menu_Text;
}
void AD_Cal_Set_Continuous_Pulse(void)
{
	int c;
	int count;

	print_dbg("\r\n\n\rPress any key to exit.");
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
		//AD_control_resources_init();
		AD_control_write(0x80);
		count=50;
		while(count-- > 0){}
		//AD_control_resources_init();
		AD_control_write(0xC0);
		count=50;
		while(count-- > 0){}
    }

	//AD_control_resources_init();
	AD_control_write(0x00);
}

void AD_Cal_Set_One_Pulse(void)
{
	int count;

	//AD_control_resources_init();
	AD_control_write(0x80);
	count=50;
	while(count-- > 0){}
	//AD_control_resources_init();
	AD_control_write(0xC0);
	count=50;
	while(count-- > 0){}

	//AD_control_resources_init();
	AD_control_write(0x00);
}	
	
void AD_Cal_Set_Low(void)
{
	//AD_control_resources_init();
	AD_control_write(0x80);
}

void AD_Cal_Set_High(void)
{
	//AD_control_resources_init();
	AD_control_write(0xC0);
}

void AD_Cal_Set_Middle(void)
{
	//AD_control_resources_init();
	AD_control_write(0x00);
}
#endif