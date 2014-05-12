////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   sd_mmc_test_menu.c                                                       //
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
#include "sd_mmc_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "sd_mmc_spi.h"
#include "FAT32_Base.h"
#include "FAT32_Access.h"
#include "FAT32_Filelib.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
// GPIO pins used for SD/MMC interface
gpio_map_t SDMMC_SPI_GPIO_MAP =
{
	{SDMMC_SPI_SCK_PIN,  SDMMC_SPI_SCK_FUNCTION },  // SPI Clock.
	{SDMMC_SPI_MISO_PIN, SDMMC_SPI_MISO_FUNCTION},  // MISO.
	{SDMMC_SPI_MOSI_PIN, SDMMC_SPI_MOSI_FUNCTION},  // MOSI.
	{SDMMC_SPI_NPCS_PIN, SDMMC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
};

static const unsigned char SD_MMC_Info_Text[] =
{
"\n\r\n\r"
"   Interface = SPI1-CS2           Digital Voltage = 3.3\n\r"
"   Ref. Des. = J5                 Memory Speed = 80x\n\r"
"   File System = FAT32            Memory Capacity = 2G max\n\r"
"\n\r"
"\0"
};

const unsigned char SD_MMC_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 SD/MMC TEST MENU                            บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) SD/MMC Information.\n\r"
"   2) Turn On SDMMC Power.\n\r"
"   3) Turn Off SDMMC Power.\n\r"
"   4) File System Menu.\n\r"
"   5) Read write protect status.\n\r"
"   6) Read card detect status.\n\r"
"\0"
};

static void (*SD_MMC_Test_Menu_Functions[])(void) =
{
   SD_MMC_Exit,
   SD_MMC_Info,
   SD_MMC_Power_On,
   SD_MMC_Power_Off,
   SD_MMC_File_System_Menu,
   SD_MMC_Read_Write_Protect,
   SD_MMC_Read_Card_Detect
};

const unsigned char SD_MMC_File_System_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                     NS8100 SD/MMC FILE SYSTEM TEST MENU                      บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) Format SD/MMC.\n\r"
"   2) List Directory.\n\r"
"   3) Change Directory.\n\r"
"   4) Remove Directory.\n\r"
"   5) Make Directory.\n\r"
"   6) Delete File.\n\r"
"   7) Type File.\n\r"
"   8) Rename File.\n\r"
"   9) Copy File.\n\r"
"  10) Change Drive.\n\r"
"  11) List Volume of SD/MMC.\n\r"
"\0"
};

static void (*SD_MMC_File_System_Test_Menu_Functions[])(void) =
{
   SD_MMC_File_System_Exit,
   SD_MMC_File_System_Format,
   SD_MMC_File_System_List_Directory,
   SD_MMC_File_System_Change_Directory,
   SD_MMC_File_System_Remove_Directory,
   SD_MMC_File_System_Make_Directory,
   SD_MMC_File_System_Delete,
   SD_MMC_File_System_Type,
   SD_MMC_File_System_Rename,
   SD_MMC_File_System_Copy,
   SD_MMC_File_System_Change_Drive,
   SD_MMC_File_System_Volume
};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 1
static void sd_mmc_resources_init(void)
{
#if 0
  // GPIO pins used for SD/MMC interface
  static const gpio_map_t SD_MMC_SPI_GPIO_MAP =
  {
    {SD_MMC_SPI_SCK_PIN,  SD_MMC_SPI_SCK_FUNCTION },  // SPI Clock.
    {SD_MMC_SPI_MISO_PIN, SD_MMC_SPI_MISO_FUNCTION},  // MISO.
    {SD_MMC_SPI_MOSI_PIN, SD_MMC_SPI_MOSI_FUNCTION},  // MOSI.
    {SD_MMC_SPI_NPCS_PIN, SD_MMC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
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
  gpio_enable_module(SD_MMC_SPI_GPIO_MAP,
                     sizeof(SD_MMC_SPI_GPIO_MAP) / sizeof(SD_MMC_SPI_GPIO_MAP[0]));

  // Initialize as master.
  spi_initMaster(SD_MMC_SPI, &spiOptions);

  // Set SPI selection mode: variable_ps, pcs_decode, delay.
  spi_selectionMode(SD_MMC_SPI, 0, 0, 0);

  // Enable SPI module.
  spi_enable(SD_MMC_SPI);

  // Initialize SD/MMC driver with SPI clock (PBA).
	sd_mmc_spi_init(spiOptions, FOSC0);
#endif

	spi_selectChip(SDMMC_SPI, SD_MMC_SPI_NPCS);
	sd_mmc_spi_internal_init();
	spi_unselectChip(SDMMC_SPI, SD_MMC_SPI_NPCS);
}
#endif

void SD_MMC_Test_Menu(void)
{
   SD_MMC_Power_On();
   Menu_Items = (sizeof(SD_MMC_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)SD_MMC_Test_Menu_Functions;
   Menu_String = (unsigned char *)SD_MMC_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  SD_MMC_Exit                                                         //
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
void SD_MMC_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void SD_MMC_Info(void)
{
   int count;
   unsigned char character;

   count = 0;
   character = SD_MMC_Info_Text[count];
   while(character != 0)
   {
      print_dbg_char(character);
      count++;
      character = SD_MMC_Info_Text[count];
   }
}

void SD_MMC_Power_On(void)
{
	gpio_enable_gpio_pin(SD_POWER_PIN);
	gpio_enable_gpio_pin(SD_WRITE_PROTECT_PIN);
	gpio_enable_gpio_pin(SD_DETECT_PIN);
	gpio_set_gpio_pin(SD_POWER_PIN);
}

void SD_MMC_Power_Off(void)
{
	gpio_clr_gpio_pin(SD_POWER_PIN);
}

void SD_MMC_File_System_Menu(void)
{
   sd_mmc_resources_init();
   Menu_Items = (sizeof(SD_MMC_File_System_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)SD_MMC_File_System_Test_Menu_Functions;
   Menu_String = (unsigned char *)SD_MMC_File_System_Test_Menu_Text;
}

void SD_MMC_Read_Write_Protect(void)
{
   int state;

   print_dbg("\n\rWrite Protect = ");

   state = gpio_get_pin_value(SD_WRITE_PROTECT_PIN);
   if(state==0)
   {
	   print_dbg("Off\n\r");
   }
   else
   {
	   print_dbg("On\n\r");
   }
}

void SD_MMC_Read_Card_Detect(void)
{
   int state;

   print_dbg("\n\rSD Detect = ");

   state = gpio_get_pin_value(SD_DETECT_PIN);
   if(state==0)
   {
	   print_dbg("Off\n\r");
   }
   else
   {
	   print_dbg("On\n\r");
   }
}


void SD_MMC_File_System_Exit(void)
{
   Menu_Items = (sizeof(SD_MMC_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)SD_MMC_Test_Menu_Functions;
   Menu_String = (unsigned char *)SD_MMC_Test_Menu_Text;
}

void SD_MMC_File_System_Format(void){}
void SD_MMC_File_System_List_Directory(void)
{
    unsigned int fatPresent=FALSE;

	// Read Card capacity
    print_dbg("\n\rDetecting SD_MMC capacity........");
    sd_mmc_spi_get_capacity();
    print_dbg_ulong(capacity >> 20);
    print_dbg(" MBytes\n\r");

    FAT32_InitDrive();
    if(FAT32_InitFAT())
    {
        print_dbg("Passed FAT32 Initialization.\n\r");
        fatPresent = TRUE;
    }
    else
    {
        print_dbg("Failed FAT32 Initialization.\n\r\n\r");
    }
    if(fatPresent == TRUE)
    {
        FAT32_ShowFATDetails();
        print_dbg("\n\rDirectory of C:\\");
        ListDirectory(FAT32_GetRootCluster());
    }
    print_dbg("\n\r");
}

void SD_MMC_File_System_Change_Directory(void){}
void SD_MMC_File_System_Remove_Directory(void){}
void SD_MMC_File_System_Make_Directory(void){}
void SD_MMC_File_System_Delete(void){}
void SD_MMC_File_System_Type(void){}
void SD_MMC_File_System_Rename(void){}
void SD_MMC_File_System_Copy(void){}
void SD_MMC_File_System_Change_Drive(void){}
void SD_MMC_File_System_Volume(void){}



