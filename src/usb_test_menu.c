////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   usb_test_menu.c                                                          //
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
//   $Date: 2009/12/08 01:13:37 $                                                                 //
//   $Revision: 1.5 $                                                             //
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
#include "usb_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "sd_mmc_spi.h"
#include "pm.h"
#include "intc.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "conf_usb.h"
#include "device_mass_storage_task.h"
#include "scsi_decoder.h"
#include "sd_mmc_test_menu.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define SD_WRITE_PROTECT_PIN  AVR32_PIN_PA07
#define SD_DETECT_PIN         AVR32_PIN_PA02
#define SD_POWER_PIN          AVR32_PIN_PB15

static const unsigned char USB_Info_Text[] =
{
"\n\r\n\r"
"   Mode = Device                  Class = Mass Storage\n\r"
"   Speed = Full\n\r"
"\n\r"
"\0"
};

static unsigned char USB_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                             NS8100 USB TEST MENU                             บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) USB Information.\n\r"
"   2) USB Turn On Mass Storage Test.\n\r"
"   3) USB Turn Off Mass Storage Test.\n\r"
"\0"
};

static void (*USB_Test_Menu_Functions[])(void) =
{
   USB_Exit,
   USB_Info,
   USB_On_Mass_Storage_Test,
   USB_Off_Mass_Storage_Test
};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 1
void sd_mmc_resources_init(void)
{
  // GPIO pins used for SD/MMC interface
  static const gpio_map_t SDMMC_SPI_GPIO_MAP =
  {
    {SDMMC_SPI_SCK_PIN,  SDMMC_SPI_SCK_FUNCTION },  // SPI Clock.
    {SDMMC_SPI_MISO_PIN, SDMMC_SPI_MISO_FUNCTION},  // MISO.
    {SDMMC_SPI_MOSI_PIN, SDMMC_SPI_MOSI_FUNCTION},  // MOSI.
    {SDMMC_SPI_NPCS_PIN, SDMMC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
  };

  // SPI options.
  spi_options_t spiOptions =
  {
    .reg          = SDMMC_SPI_NPCS,
    .baudrate     = SDMMC_SPI_MASTER_SPEED,  // Defined in conf_sd_mmc.h.
    .bits         = SDMMC_SPI_BITS,          // Defined in conf_sd_mmc.h.
    .spck_delay   = 0,
    .trans_delay  = 0,
    .stay_act     = 1,
    .spi_mode     = 0,
    .modfdis      = 1
  };

  // Assign I/Os to SPI.
  gpio_enable_module(SDMMC_SPI_GPIO_MAP,
                     sizeof(SDMMC_SPI_GPIO_MAP) / sizeof(SDMMC_SPI_GPIO_MAP[0]));

  // Initialize as master.
  spi_initMaster(SDMMC_SPI, &spiOptions);

  // Set SPI selection mode: variable_ps, pcs_decode, delay.
  spi_selectionMode(SDMMC_SPI, 0, 0, 0);

  // Enable SPI module.
  spi_enable(SDMMC_SPI);

  // Initialize SD/MMC driver with SPI clock (PBA).
  sd_mmc_spi_init(spiOptions, FOSC0);
}
#endif

void USB_Test_Menu(void)
{
	gpio_enable_gpio_pin(SD_POWER_PIN);
	gpio_enable_gpio_pin(SD_WRITE_PROTECT_PIN);
	gpio_enable_gpio_pin(SD_DETECT_PIN);
	gpio_set_gpio_pin(SD_POWER_PIN);

	//sd_mmc_resources_init();

	// Initialize USB clock.
	pm_configure_usb_clock();

	// Done in main
	// Initialize interrupt handling.
	//INTC_init_interrupts();

	Menu_Items = (sizeof(USB_Test_Menu_Functions)/sizeof(NONE))/4;
	Menu_Functions = (unsigned long *)USB_Test_Menu_Functions;
	Menu_String = (unsigned char *)USB_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  USB_Exit                                                            //
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
void USB_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void USB_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = USB_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = USB_Info_Text[count];
		}
}
void USB_On_Mass_Storage_Test(void)
{
    int *uart_rx_character = 0;
    usb_task_init();
    device_mass_storage_task_init();

    print_dbg("\n\rUSB started, press any key to exit USB test. \n\r");

    while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
    {
        usb_task();
        device_mass_storage_task();
    }
}

void USB_Off_Mass_Storage_Test(void)
{
   print_dbg("\n\rUSB disabled. \n\r");
   Disable_global_interrupt();
   Usb_disable();
}


