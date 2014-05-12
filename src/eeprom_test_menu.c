////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   eeprom_test_menu.c                                                       //
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
//   $Date: 2009/11/09 19:26:50 $                                                                 //
//   $Revision: 1.4 $                                                             //
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
#include "eeprom_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
// GPIO pins used for eeprom interface
gpio_map_t EEPROM_SPI_GPIO_MAP =
{
    {EEPROM_SPI_SCK_PIN,  EEPROM_SPI_SCK_FUNCTION },  // SPI Clock.
    {EEPROM_SPI_MISO_PIN, EEPROM_SPI_MISO_FUNCTION},  // MISO.
    {EEPROM_SPI_MOSI_PIN, EEPROM_SPI_MOSI_FUNCTION},  // MOSI.
    {EEPROM_SPI_NPCS_PIN, EEPROM_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
};

static const unsigned char EEPROM_Info_Text[] =
{
"\n\r\n\r"
"   Part = AT25640_R               Voltage = 3.3V\n\r"
"   Interface = SPI1_CS0           Speed = 2MHz\n\r"
"   Ref. Des. = U75                Size = 8192 bytes\n\r"
"\n\r"
"\0"
};

const unsigned char EEPROM_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                           NS8100 EEPROM TEST MENU                            บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) EEPROM Information.\n\r"
"   2) Config Data Menu.\n\r"
"   3) Blank Check.\n\r"
"   4) Erase EEPROM.\n\r"
"   5) Write Data.\n\r"
"   6) Read Data.\n\r"
"\0"
};

static void (*EEPROM_Test_Menu_Functions[])(void) =
{
   EEPROM_Exit,
   EEPROM_Info,
   EEPROM_Config_Data_Menu,
   EEPROM_Blank_Check,
   EEPROM_Erase,
   EEPROM_Write,
   EEPROM_Read
};

const unsigned char EEPROM_Config_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                    NS8100 EEPROM CONFIGURATION TEST MENU                     บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) View Configuration Data.\n\r"
"   2) Edit Configuration Data.\n\r"
"   3) Erase CRC.\n\r"
"   4) Test CRC.\n\r"
"   5) Write CRC.\n\r"
"   6) Erase Configuration Data.\n\r"
"   7) Test Configuration.\n\r"
"   8) Restore Default Configuration Data.\n\r"
"\0"
};

static void (*EEPROM_Config_Menu_Functions[])(void) =
{
   EEPROM_Config_Exit,
   EEPROM_Config_View_Data,
   EEPROM_Config_Edit_Data,
   EEPROM_Config_Erase_CRC,
   EEPROM_Config_Test_CRC,
   EEPROM_Config_Write_CRC,
   EEPROM_Config_Erase_Data,
   EEPROM_Config_Test_Data,
   EEPROM_Config_Restore_Default_Data
};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 0
static void eeprom_resources_init(void)
{
   // GPIO pins used for eeprom interface
   static const gpio_map_t EEPROM_SPI_GPIO_MAP =
   {
      {EEPROM_SPI_SCK_PIN,  EEPROM_SPI_SCK_FUNCTION },  // SPI Clock.
      {EEPROM_SPI_MISO_PIN, EEPROM_SPI_MISO_FUNCTION},  // MISO.
      {EEPROM_SPI_MOSI_PIN, EEPROM_SPI_MOSI_FUNCTION},  // MOSI.
      {EEPROM_SPI_NPCS_PIN, EEPROM_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
   };

   // SPI options.
   spi_options_t spiOptions =
   {
      .reg          = EEPROM_SPI_NPCS,
      .baudrate     = EEPROM_SPI_MASTER_SPEED,
      .bits         = EEPROM_SPI_BITS,
      .spck_delay   = 0,
      .trans_delay  = 0,
      .stay_act     = 1,
      .spi_mode     = 0,
      .modfdis      = 1
   };

   // Assign I/Os to SPI.
   gpio_enable_module(EEPROM_SPI_GPIO_MAP,
                     sizeof(EEPROM_SPI_GPIO_MAP) / sizeof(EEPROM_SPI_GPIO_MAP[0]));

   // Initialize as master.
   spi_initMaster(EEPROM_SPI, &spiOptions);

   // Set SPI selection mode: variable_ps, pcs_decode, delay.
   spi_selectionMode(EEPROM_SPI, 0, 0, 0);

   // Enable SPI module.
   spi_enable(EEPROM_SPI);

   // Setup SPI registers according to spiOptions.
   spi_setupChipReg(EEPROM_SPI, &spiOptions, FOSC0);
}
#endif

static void eeprom_write_command(int command)
{
   spi_write(EEPROM_SPI,command);
}

static void eeprom_write_data(unsigned short register_address, int length, unsigned short *data)
{
   spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
   eeprom_write_command(EEPROM_WRITE_ENABLE);
   spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);

   spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
   eeprom_write_command(EEPROM_WRITE_DATA);
   spi_write(EEPROM_SPI, (register_address >> 8) & 0xFF);
   spi_write(EEPROM_SPI, register_address & 0xFF);

   while(length--)
   {
      spi_write(EEPROM_SPI, *data++);
   }
   spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
}

static void eeprom_read_data(unsigned short register_address, int length, unsigned short *data)
{
   spi_selectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
   eeprom_write_command(EEPROM_READ_DATA);
   spi_write(EEPROM_SPI, (register_address >> 8) & 0xFF);
   spi_write(EEPROM_SPI, register_address & 0xFF);

   while(length--)
   {
      spi_write(EEPROM_SPI,0xFF);
      spi_read(EEPROM_SPI, (unsigned short*)data);
      data++;
   }
   spi_unselectChip(EEPROM_SPI, EEPROM_SPI_NPCS);
}

void EEPROM_Test_Menu(void)
{
	//eeprom_resources_init();
	
    Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
    Menu_Functions = (unsigned long *)EEPROM_Test_Menu_Functions;
    Menu_String = (unsigned char *)EEPROM_Test_Menu_Text;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  EEPROM_Exit                                                         //
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
void EEPROM_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void EEPROM_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = EEPROM_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = EEPROM_Info_Text[count];
		}
}

void EEPROM_Config_Data_Menu(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)EEPROM_Config_Menu_Functions;
   Menu_String = (unsigned char *)EEPROM_Config_Menu_Text;
}

void EEPROM_Blank_Check(void)
{
   int count;
   unsigned short data[32];

   eeprom_read_data(0, 32, &data[0]);

   for(count=0;count<32;count++)
   {
	   if((data[count] & 0xFF) != 0xFF)
	   {
		  print_dbg("Device blank check FAILED. ");
		  print_dbg_short_hex(count);
		  print_dbg("\r\n");
		   return;
	   }
   }

     print_dbg("Device blank check PASSED.\n\r");
}

void EEPROM_Erase(void)
{
   int count;
   unsigned short data[32];

   for(count=0;count<32;count++)
   {
      data[count] = 0x00FF;
   }
   eeprom_write_data(0, 32, &data[0]);
   print_dbg("Device erased.\n\r");
}

void EEPROM_Write(void)
{
   int count;
   unsigned short data[32];

   for(count=0;count<32;count++)
   {
	   data[count] = count;
   }
   eeprom_write_data(0, 32, &data[0]);
   print_dbg("Device location 0 - 32 written with count pattern.\n\r");
}
void EEPROM_Read(void)
{
   int count;
   unsigned short data[32];

   eeprom_read_data(0, 32, &data[0]);
   print_dbg("Device location 0 - 32:\n\r");
   for(count=0;count<16;count++)
   {
	   print_dbg_short_hex(data[count]);
	   print_dbg(" ");
   }
   print_dbg("\n\r");

   for(count=16;count<32;count++)
   {
	   print_dbg_short_hex(data[count]);
	   print_dbg(" ");
   }
   print_dbg("\n\r");
}


void EEPROM_Config_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)EEPROM_Test_Menu_Functions;
   Menu_String = (unsigned char *)EEPROM_Test_Menu_Text;
}

void EEPROM_Config_View_Data(void){}
void EEPROM_Config_Edit_Data(void){}
void EEPROM_Config_Erase_CRC(void){}
void EEPROM_Config_Test_CRC(void){}
void EEPROM_Config_Write_CRC(void){}
void EEPROM_Config_Erase_Data(void){}
void EEPROM_Config_Test_Data(void){}
void EEPROM_Config_Restore_Default_Data(void){}



