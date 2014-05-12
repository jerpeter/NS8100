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
//   $Date: 2012/04/26 01:10:07 $                                                                 //
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
"  12) Display SID text file.\n\r"
"  13) Display Log text file.\n\r"
"  14) Add Log entry to text file.\n\r"
"  15) Test Standard Fat Driver.\n\r"
"  16) Test Standard Fat Read.\n\r"
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
   SD_MMC_File_System_Volume,
   SD_MMC_Display_SID_text,
   SD_MMC_Display_Log_text,
   SD_MMC_Add_Log_text,
   SD_MMC_Test_Standard_Fat_Driver,
   SD_MMC_Test_Standard_Fat_Read
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
    unsigned int fatPresent=FALSE;

	sd_mmc_resources_init();
   
    FAT32_InitDrive();
    if(FAT32_InitFAT())
    {
        print_dbg("Passed FAT32 Initialization.\n\r");
        fatPresent = TRUE;

		Menu_Items = (sizeof(SD_MMC_File_System_Test_Menu_Functions)/sizeof(NONE))/4;
		Menu_Functions = (unsigned long *)SD_MMC_File_System_Test_Menu_Functions;
		Menu_String = (unsigned char *)SD_MMC_File_System_Test_Menu_Text;

    }
    else
    {
        print_dbg("Failed FAT32 Initialization.\n\r\n\r");
    }
   
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

void SD_MMC_File_System_Format(void)
{
	// Empty
}

#include "FAT32_Opts.h"
extern unsigned short int eventDataBuffer[];
void SD_MMC_File_System_List_Directory(void)
{
	FAT32_DIRLIST* dirList = (FAT32_DIRLIST*)&(eventDataBuffer[0]);
    unsigned int fatPresent=FALSE;
	unsigned long int dirStartCluster;
	unsigned int entriesFound = 0;
	unsigned long int totalSize = 0;
	FL_FILE* SID_file;
	unsigned long int fileSize;
	char fileName[50];
	
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

	print_dbg("Address of EventBuff: 0x");
	print_dbg_hex((unsigned long int)&eventDataBuffer[0]);
	print_dbg(", Aligned x4: ");
	if (((unsigned long int)&eventDataBuffer[0]) % 4 == 0) print_dbg("Yes\r\n"); else print_dbg("No\r\n"); 

	print_dbg("Address of dirList: 0x");
	print_dbg_hex((unsigned long int)dirList);
	print_dbg(", Aligned x4: ");
	if (((unsigned long int)dirList) % 4 == 0) print_dbg("Yes\r\n"); else print_dbg("No\r\n"); 

//--------------------------------------------------------------------------------
    if(fatPresent == TRUE)
    {
        FAT32_ShowFATDetails();
        print_dbg("\n\rDirectory of C:\\");
        ListDirectory(FAT32_GetRootCluster(), dirList, 1);
    }

	print_dbg("----------------------------------\n\r");
    print_dbg("Buffer Contents:\n\r");
	entriesFound = 0;
	totalSize = 0;
	
	while(dirList[entriesFound].type != FAT32_END_LIST)
	{
		if (dirList[entriesFound].type == FAT32_DIR)
		{
			print_dbg("DIR : ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg("\r\n");			
			entriesFound++;
		}
		else if (dirList[entriesFound].type == FAT32_FILE)
		{
			print_dbg("FILE: ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg(", Size: ");			
			print_dbg_ulong(dirList[entriesFound].size);			
			print_dbg("\r\n");			
			totalSize += dirList[entriesFound].size;

			sprintf(fileName, "C:\\%s", dirList[entriesFound].name);
			SID_file = fl_fopen(fileName, "r");
	
			if (SID_file == NULL)
				print_dbg("Error: SID File not found!\r\n");
			else
			{
				print_dbg("File Contents:\r\n");

				fileSize = SID_file->filelength;
				while (fileSize--)
				{
					print_dbg_char(fl_fgetc(SID_file));
				}
		
				print_dbg("\n\rSID Size:");
				print_dbg_ulong(SID_file->filelength);
				print_dbg("\n\r");

				fl_fclose(SID_file);
			}		

			entriesFound++;
		}
	}
	print_dbg("Total File Size: ");			
	print_dbg_ulong(totalSize);			
	print_dbg("\r\n");			

//--------------------------------------------------------------------------------
    if(fatPresent == TRUE)
    {
		fl_directory_start_cluster("C:\\Events", &dirStartCluster);
        print_dbg("\n\rDirectory of C:\\Events");
        ListDirectory(dirStartCluster, dirList, 1);
	}

	print_dbg("----------------------------------\n\r");
    print_dbg("Buffer Contents:\n\r");
	entriesFound = 0;
	totalSize = 0;

	while(dirList[entriesFound].type != FAT32_END_LIST)
	{
		if (dirList[entriesFound].type == FAT32_DIR)
		{
			print_dbg("DIR : ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg("\r\n");			
			entriesFound++;
		}
		else if (dirList[entriesFound].type == FAT32_FILE)
		{
			print_dbg("FILE: ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg(", Size: ");			
			print_dbg_ulong(dirList[entriesFound].size);			
			print_dbg("\r\n");			
			totalSize += dirList[entriesFound].size;
			entriesFound++;
		}
	}
	print_dbg("Total File Size: ");			
	print_dbg_ulong(totalSize);			
	print_dbg("\r\n");			

//--------------------------------------------------------------------------------
    if(fatPresent == TRUE)
    {
		fl_directory_start_cluster("C:\\Logs", &dirStartCluster);
        print_dbg("\n\rDirectory of C:\\Logs");
        ListDirectory(dirStartCluster, dirList, 1);
	}

	print_dbg("----------------------------------\n\r");
    print_dbg("Buffer Contents:\n\r");
	entriesFound = 0;
	totalSize = 0;

	while(dirList[entriesFound].type != FAT32_END_LIST)
	{
		if (dirList[entriesFound].type == FAT32_DIR)
		{
			print_dbg("DIR : ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg("\r\n");			
			entriesFound++;
		}
		else if (dirList[entriesFound].type == FAT32_FILE)
		{
			print_dbg("FILE: ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg(", Size: ");			
			print_dbg_ulong(dirList[entriesFound].size);			
			print_dbg("\r\n");			
			totalSize += dirList[entriesFound].size;
			entriesFound++;
		}
	}
	print_dbg("Total File Size: ");			
	print_dbg_ulong(totalSize);			
	print_dbg("\r\n");			

//--------------------------------------------------------------------------------
    if(fatPresent == TRUE)
    {
		fl_directory_start_cluster("C:\\System", &dirStartCluster);
        print_dbg("\n\rDirectory of C:\\System");
        ListDirectory(dirStartCluster, dirList, 1);
	}

	print_dbg("----------------------------------\n\r");
    print_dbg("Buffer Contents:\n\r");
	entriesFound = 0;
	totalSize = 0;

	while(dirList[entriesFound].type != FAT32_END_LIST)
	{
		if (dirList[entriesFound].type == FAT32_DIR)
		{
			print_dbg("DIR : ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg("\r\n");			
			entriesFound++;
		}
		else if (dirList[entriesFound].type == FAT32_FILE)
		{
			print_dbg("FILE: ");			
			print_dbg(dirList[entriesFound].name);			
			print_dbg(", Size: ");			
			print_dbg_ulong(dirList[entriesFound].size);			
			print_dbg("\r\n");			
			totalSize += dirList[entriesFound].size;
			entriesFound++;
		}
	}
	print_dbg("Total File Size: ");			
	print_dbg_ulong(totalSize);			
	print_dbg("\r\n");			

    print_dbg("\n\r");
}

#include "navigation.h"
#include "fat.h"
#include "fs_com.h"
void SD_MMC_Test_Standard_Fat_Driver(void)
{
	nav_drive_set(LUN_2);

	if (fat_check_device() == TRUE)
	{
	    print_dbg("Fat Check successful.\n\r");
	}
	else
	{
	    print_dbg("Fat Check unsuccessful! Error code: ");
		print_dbg_hex(fs_g_status);
	    print_dbg("\n\r");
	}

	if (fat_check_mount_noopen() == TRUE)
	{
	    print_dbg("Fat Check Mount successful.\n\r");
	}
	else
	{
	    print_dbg("Fat Check Mount unsuccessful! Error code: ");
		print_dbg_hex(fs_g_status);
	    print_dbg("\n\r");
	}

	if (fat_mount() == TRUE)
	{
	    print_dbg("Fat Mount successful.\n\r");
	}
	else
	{
	    print_dbg("Fat Mount unsuccessful! Error code: ");
		print_dbg_hex(fs_g_status);
	    print_dbg("\n\r");
	}
	
	print_dbg("Fat Free Sectors: ");
	print_dbg_hex(fat_read_fat32_FSInfo());
	print_dbg("\n\r");

	print_dbg("Fat Free Space: ");
	print_dbg_hex(fat_getfreespace());
	print_dbg("\n\r");

	print_dbg("Fat Free Space Percent: ");
	print_dbg_ulong(100 * fat_getfreespace_percent());
	print_dbg("\n\r");
}

#include "fsaccess.h"
void SD_MMC_Test_Standard_Fat_Read(void)
{
	int SID_file;
	char buff[100];
	int returnSize, i = 0;

	print_dbg("Attempting to open SID File...\r\n");
	SID_file = open("C:\\SID.txt", O_RDONLY);
	
	if (SID_file == -1)
		print_dbg("Error: SID File not found!\r\n");
	else
	{
		print_dbg("SID Contents:\r\n");

		returnSize = read(SID_file, &buff[0], 27);

		while (i < returnSize)
		{
			print_dbg_char(buff[i++]);
		}
		
		print_dbg("\n\rSID Size:");
		print_dbg_ulong(returnSize);
		print_dbg("\n\r");

		close(SID_file);
	}
}

void SD_MMC_Display_SID_text(void)
{
	FL_FILE* SID_file;
	unsigned long int fileSize;

	print_dbg("Attempting to open SID File...\r\n");
	SID_file = fl_fopen("C:\\SID     .txt", "r");
	
	if (SID_file == NULL)
		print_dbg("Error: SID File not found!\r\n");
	else
	{
		print_dbg("SID Contents:\r\n");

		fileSize = SID_file->filelength;
		while (fileSize--)
		{
			print_dbg_char(fl_fgetc(SID_file));
		}
		
		print_dbg("\n\rSID Size:");
		print_dbg_ulong(SID_file->filelength);
		print_dbg("\n\r");

		fl_fclose(SID_file);
	}		
}

void SD_MMC_Display_Log_text(void)
{
	FL_FILE* Log_file;
	unsigned long int fileSize;

	print_dbg("Attempting to open Log File...\r\n");
	Log_file = fl_fopen("C:\\Logs\\Testlog.txt", "r");
	
	if (Log_file == NULL)
		print_dbg("Error: Log File not found!\r\n");
	else
	{
		print_dbg("Log Contents:\r\n");

		fileSize = Log_file->filelength;
		while (fileSize--)
		{
			print_dbg_char(fl_fgetc(Log_file));
		}
		
		print_dbg("\n\rLog Size:");
		print_dbg_ulong(Log_file->filelength);
		print_dbg("\n\r");

		fl_fclose(Log_file);
	}		
}

void SD_MMC_Add_Log_text(void)
{
	FL_FILE* Log_file;
	unsigned int fileSize;
	char addLogEntry[50];

	print_dbg("Attempting to open Log File...\r\n");
	Log_file = fl_fopen("C:\\Logs\\Testlog.txt", "a+");
	
	if (Log_file == NULL)
		print_dbg("Error: Log File not found!\r\n");
	else
	{
		memset(&addLogEntry[0], 0, sizeof(addLogEntry));
		fileSize = Log_file->filelength;
		fileSize -= 11;
		fileSize /= 19;
		fileSize++;
		
		sprintf(addLogEntry, "Test log entry #%d\n", fileSize);		
		fl_fputs(addLogEntry, Log_file);

		fl_fclose(Log_file);

		print_dbg("Log File modified\r\n");
	}		
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



