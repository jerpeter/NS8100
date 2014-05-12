////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   rtc_test_menu.c                                                          //
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
#include "rtc_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "utils.h"
#include "RealTimeClock.h"
#include "twi.h"
#include "m23018.h"
#include "spi.h"

#define RTC_TIMESTAMP_PIN AVR32_PIN_PB18

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
// GPIO pins used for RTC interface
gpio_map_t RTC_SPI_GPIO_MAP =
{
    {RTC_SPI_SCK_PIN,  RTC_SPI_SCK_FUNCTION },  // SPI Clock.
    {RTC_SPI_MISO_PIN, RTC_SPI_MISO_FUNCTION},  // MISO.
    {RTC_SPI_MOSI_PIN, RTC_SPI_MOSI_FUNCTION},  // MOSI.
    {RTC_SPI_NPCS_PIN, RTC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
};

const unsigned char RTC_Info_Text[] =
{
"\n\r\n\r"
"   Part = M41ST95                 Battery Voltage = 3V\n\r"
"   Interface = SPI1_CS1           Digital Voltage = 3.3V\n\r"
"   Ref. Des. = U33                Communication Speed = 2MHz\n\r"
"   User Ram Bytes = 44\n\r"
"\n\r"
"   RTC Memory Map\n\r"
"   00) 100ths of a Second\n\r"
"   01) Seconds\n\r"
"   02) Minutes\n\r"
"   03) Century/Hours\n\r"
"   04) Day\n\r"
"   05) Date\n\r"
"   06) Month\n\r"
"   07) Year\n\r"
"   08) Control\n\r"
"   09) Watchdog\n\r"
"   0A) Alarm Month\n\r"
"   0B) Alarm Date\n\r"
"   0C) Alarm Hours\n\r"
"   0D) Alarm Minutes\n\r"
"   0E) Alarm Seconds\n\r"
"   0F) Flags\n\r"
"   10) Reserved\n\r"
"   11) Reserved\n\r"
"   12) Reserved\n\r"
"   13) SQW\n\r"
"\n\r"
"\0"
};

const unsigned char RTC_Test_Menu_Text[] =
{
"\n\r"
"  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป\n\r"
"  บ                                                                              บ\n\r"
"  บ                            NS8100 RTC TEST MENU                              บ\n\r"
"  บ                                                                              บ\n\r"
"  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\r"
"\n\r\n\r"
"   0) Exit.\n\r"
"   1) RTC Information.\n\r"
"   2) Read Time.\n\r"
"   3) Set Time.\n\r"
"   4) Read Date.\n\r"
"   5) Set Date.\n\r"
"   6) Read Alarm Time.\n\r"
"   7) Set Alarm Time.\n\r"
"   8) Read Alarm Date.\n\r"
"   9) Set Alarm Date.\n\r"
"  10) Test Alarm Output.\n\r"
"  11) Test Auto On.\n\r"
"  12) Read RTC Registers.\n\r"
"  13) Fill RTC Ram.\n\r"
"\0"
};

void (*RTC_Test_Menu_Functions[])(void) =
{
   RTC_Exit,
   RTC_Info,
   RTC_Read_Time,
   RTC_Set_Time,
   RTC_Read_Date,
   RTC_Set_Date,
   RTC_Read_Alarm_Time,
   RTC_Set_Alarm_Time,
   RTC_Read_Alarm_Date,
   RTC_Set_Alarm_Date,
   RTC_Test_Alarm_Output,
   RTC_Test_Auto_On,
   RTC_Read_Registers,
   RTC_Fill_Ram
};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 0
static void rtc_resources_init(void)
{
   // GPIO pins used for RTC interface
   static const gpio_map_t RTC_SPI_GPIO_MAP =
   {
      {RTC_SPI_SCK_PIN,  RTC_SPI_SCK_FUNCTION },  // SPI Clock.
      {RTC_SPI_MISO_PIN, RTC_SPI_MISO_FUNCTION},  // MISO.
      {RTC_SPI_MOSI_PIN, RTC_SPI_MOSI_FUNCTION},  // MOSI.
      {RTC_SPI_NPCS_PIN, RTC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
   };

   // SPI options.
   spi_options_t spiOptions =
   {
      .reg          = RTC_SPI_NPCS,
      .baudrate     = RTC_SPI_MASTER_SPEED,
      .bits         = RTC_SPI_BITS,
      .spck_delay   = 0,
      .trans_delay  = 0,
      .stay_act     = 1,
      .spi_mode     = 0,
      .modfdis      = 1
   };

   // Assign I/Os to SPI.
   gpio_enable_module(RTC_SPI_GPIO_MAP,
                     sizeof(RTC_SPI_GPIO_MAP) / sizeof(RTC_SPI_GPIO_MAP[0]));

   // Initialize as master.
   spi_initMaster(RTC_SPI, &spiOptions);

   // Set SPI selection mode: variable_ps, pcs_decode, delay.
   spi_selectionMode(RTC_SPI, 0, 0, 0);

   // Enable SPI module.
   spi_enable(RTC_SPI);

   // Setup SPI registers according to spiOptions.
   spi_setupChipReg(RTC_SPI, &spiOptions, FOSC0);
}
#endif

void rtcDelay(void)
{
	volatile uint32 i = 0;
	
	for(i=0;i<100;i++) {}
}

void rtc_write(uint8 register_address, int length, uint16* data)
{
	spi_selectChip(RTC_SPI, RTC_SPI_NPCS);
	rtcDelay();
	spi_write(RTC_SPI, (RTC_WRITE | (register_address & 0x1F)));

	while(length--)
	{
		rtcDelay();
		spi_write(RTC_SPI, *data);
		data++;
	}

	spi_unselectChip(RTC_SPI, RTC_SPI_NPCS);
}

void rtc_read(uint8 register_address, int length, uint16* data)
{
	spi_selectChip(RTC_SPI, RTC_SPI_NPCS);
	rtcDelay();
	spi_write(RTC_SPI,(RTC_READ | (register_address & 0x1F)));

	while(length--)
	{
		rtcDelay();

		spi_write(RTC_SPI, 0xFF);
		spi_read(RTC_SPI, data);
		data++;
	}

	spi_unselectChip(RTC_SPI, RTC_SPI_NPCS);
}

void RTC_Test_Menu(void)
{
	//rtc_resources_init();

#if 0
	unsigned short int control_1 = 0x04;
	rtc_write(0, 1, (uint8*)&control_1);
#endif

	Menu_Items = (sizeof(RTC_Test_Menu_Functions)/sizeof(NONE))/4;
	Menu_Functions = (unsigned long *)RTC_Test_Menu_Functions;
	Menu_String = (unsigned char *)RTC_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  RTC_Exit                                                            //
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
void RTC_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void RTC_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = RTC_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = RTC_Info_Text[count];
		}
}

void RTC_Read_Registers(void)
{
   int count=0;
   unsigned short data[64];

   rtc_read(RTC_CONTROL_1_ADDR, 64, (uint16*)&data[0]);
   // display value to user
   print_dbg("Data is:\n\r");
   while(count < 64)
   {
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg(" ");
      print_dbg_ulong(data[count++]);
      print_dbg("\n\r");
   }
   print_dbg("\n\r");
}

void RTC_Fill_Ram(void)
{
#if 0
   int count;
   unsigned short data[64];

   for(count=0;count<64;count++)
   {
	   data[count]=count;
   }
   rtc_write(FIRST_RTC_RAM, 44, data);
#else
   print_dbg("Not available at this time.\n\r");
#endif   
}

void RTC_Read_Time(void)
{
	int hours;
	int minutes;
	int seconds;
	unsigned short data[10];

    int c;
	print_dbg("\r\n\n\rPress any key to exit.\n\r");
    while(usart_read_char(DBG_USART, &c) == USART_RX_EMPTY)
    {
		//rtc_read(RTC_SECONDS_ADDR_TEMP, 1, (uint8*)&data[0]);
		//rtc_read(RTC_MINUTES_ADDR_TEMP, 1, (uint8*)&data[1]);
		//rtc_read(RTC_HOURS_ADDR_TEMP, 1, (uint8*)&data[2]);
	    rtc_read(RTC_SECONDS_ADDR_TEMP, 3, (uint16*)&data[0]);

		seconds = data[0];
		minutes = data[1];
		hours = data[2];

		print_dbg("Time is ");
		print_dbg_char(0x30 + ((data[2] >> 4) & 0x0001));
		print_dbg_char(0x30 + (data[2] & 0x0F));
		print_dbg(":");

		print_dbg_char(0x30 + ((data[1] >> 4) & 0x0007));
		print_dbg_char(0x30 + (data[1] & 0x000F));
		print_dbg(":");

		print_dbg_char(0x30 + ((data[0] >> 4) & 0x0007));
		print_dbg_char(0x30 + (data[0] & 0x000F));
		print_dbg("\r");
    }

	print_dbg("\r\n");
}

void RTC_Set_Time(void)
{
   unsigned char input_buffer[10];
   unsigned short data[10];
   int reply;
   int read_hours, hours;
   int read_minutes, minutes;
   int read_seconds, seconds;

	//rtc_read(RTC_SECONDS_ADDR_TEMP, 1, (uint8*)&data[0]);
	//rtc_read(RTC_MINUTES_ADDR_TEMP, 1, (uint8*)&data[1]);
	//rtc_read(RTC_HOURS_ADDR_TEMP, 1, (uint8*)&data[2]);
   rtc_read(RTC_SECONDS_ADDR_TEMP, 3, (uint16*)&data[0]);
   read_seconds = data[0];
   read_minutes = data[1];
   read_hours = data[2];
   seconds=0;
   minutes=0;
   hours=0;

   print_dbg("Set Hours (");
	print_dbg_char(0x30 + ((data[2] >> 4) & 0x01));
	print_dbg_char(0x30 + (data[2] & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     hours = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     hours = ((input_buffer[0] - 0x30) << 4);
   	     hours += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      hours = read_hours;
   }

   print_dbg("Set Minutes (");
	print_dbg_char(0x30 + ((data[1] >> 4) & 0x07));
	print_dbg_char(0x30 + (data[1] & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     minutes = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     minutes = ((input_buffer[0] - 0x30) << 4);
   	     minutes += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      minutes = read_minutes;
   }

   print_dbg("Set Seconds (");
	print_dbg_char(0x30 + ((data[0] >> 4) & 0x07));
	print_dbg_char(0x30 + (data[0] & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     seconds = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     seconds = ((input_buffer[0] - 0x30) << 4);
   	     seconds += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      seconds = read_seconds;
   }

   data[0]=seconds;
   data[1]=minutes;
   data[2]=hours;

	//rtc_write(RTC_SECONDS_ADDR_TEMP, 1, (uint8*)&data[0]);
	//rtc_write(RTC_MINUTES_ADDR_TEMP, 1, (uint8*)&data[1]);
	//rtc_write(RTC_HOURS_ADDR_TEMP, 1, (uint8*)&data[2]);
   rtc_write(RTC_SECONDS_ADDR_TEMP, 3, (uint16*)&data[0]);
}

void RTC_Read_Date(void)
{
   int day;
   int weekday;
   int month;
   int year;
   unsigned short data[10];

   rtc_read(RTC_DAYS_ADDR, 4, (uint16*)&data[0]);
   day = data[0];
   weekday = data[1];
   month = data[2];
   year = data[3];

   print_dbg("Date is ");
	print_dbg_char(0x30 + ((data[2] >> 4) & 0x01));
	print_dbg_char(0x30 + (data[2] & 0x0F));
   print_dbg("-");

	print_dbg_char(0x30 + ((data[0] >> 4) & 0x03));
	print_dbg_char(0x30 + (data[0] & 0x0F));
   print_dbg("-20");

	print_dbg_char(0x30 + ((data[3] >> 4) & 0x0F));
	print_dbg_char(0x30 + (data[3] & 0x0F));
   print_dbg("\n\r");
}

void RTC_Set_Date(void)
{
   unsigned char input_buffer[10];
   unsigned short data[10];
   int reply;
   int read_day, day;
   int read_weekday, weekday;
   int read_month, month;
   int read_year, year;

   rtc_read(RTC_DAYS_ADDR, 4, (uint16*)&data[0]);
   read_day = data[0];
   read_weekday = data[1];
   read_month = data[2];
   read_year = data[3];
   day=0;
   weekday=0;
   month=0;
   year=0;

   print_dbg("Set Month (");
	print_dbg_char(0x30 + ((data[2] >> 4) & 0x01));
	print_dbg_char(0x30 + (data[2] & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     month = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     month = ((input_buffer[0] - 0x30) << 4);
   	     month += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      month = read_month;
   }

   print_dbg("Set Day (");
	print_dbg_char(0x30 + ((data[0] >> 4) & 0x03));
	print_dbg_char(0x30 + (data[0] & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     day = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     day = ((input_buffer[0] - 0x30) << 4);
   	     day += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      day = read_day;
   }

   print_dbg("Set Year (");
	print_dbg_char(0x30 + ((data[3] >> 4) & 0x0F));
	print_dbg_char(0x30 + (data[3] & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     year = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     year = ((input_buffer[0] - 0x30) << 4);
   	     year += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      year = read_year;
   }

   data[0]=day;
   data[2]=month;
   data[3]=year;
   rtc_write(RTC_DAYS_ADDR, 4, (uint16*)&data[0]);
}

void RTC_Read_Alarm_Time(void)
{
   int hours;
   int minutes;
   int seconds;
   unsigned short data[10];

   rtc_read(ALARM_HOUR_ADDRESS, 3, (uint16*)&data[0]);
   hours = data[0];
   minutes = data[1];
   seconds = data[2];

   print_dbg("Alarm Time is ");
   print_dbg_char(0x30 +((hours >> 4)& 0xF));
   print_dbg_char(0x30 + (hours & 0x0F));
   print_dbg(":");

   print_dbg_char(0x30 +((minutes >> 4)& 0xF));
   print_dbg_char(0x30 + (minutes & 0x0F));
   print_dbg(":");

   print_dbg_char(0x30 +((seconds >> 4)& 0xF));
   print_dbg_char(0x30 + (seconds & 0x0F));
   print_dbg("\n\r");

}
void RTC_Set_Alarm_Time(void)
{
   unsigned char input_buffer[10];
   unsigned short data[10];
   int reply;
   int read_hours, hours;
   int read_minutes, minutes;
   int read_seconds, seconds;

   rtc_read(ALARM_HOUR_ADDRESS, 3, (uint16*)&data[0]);
   read_hours = data[0];
   read_minutes = data[1];
   read_seconds = data[2];
   seconds=0;
   minutes=0;
   hours=0;

   print_dbg("Set Alarm Hours (");
   print_dbg_char(0x30 +((read_hours >> 4)& 0xF));
   print_dbg_char(0x30 + (read_hours & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     hours = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     hours = ((input_buffer[0] - 0x30) << 4);
   	     hours += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      hours = read_hours;
   }

   print_dbg("Set Alarm Minutes (");
   print_dbg_char(0x30 +((read_minutes >> 4)& 0xF));
   print_dbg_char(0x30 + (read_minutes & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     minutes = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     minutes = ((input_buffer[0] - 0x30) << 4);
   	     minutes += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      minutes = read_minutes;
   }

   print_dbg("Set Alarm Seconds (");
   print_dbg_char(0x30 +((read_seconds >> 4)& 0xF));
   print_dbg_char(0x30 + (read_seconds & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     seconds = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     seconds = ((input_buffer[0] - 0x30) << 4);
   	     seconds += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      seconds = read_seconds;
   }

   data[0]=hours;
   data[1]=minutes;
   data[2]=seconds;
   rtc_write(ALARM_HOUR_ADDRESS, 3, (uint16*)&data[0]);
}

void RTC_Read_Alarm_Date(void)
{
   int date;
   int month;
   int year;
   unsigned short data[10];

   rtc_read(YEAR_ADDRESS, 5, (uint16*)&data[0]);
   year = data[0];
   month = data[3];
   date = data[4];

   print_dbg("Date is ");
   print_dbg_char(0x30 +((month >> 4)& 0x1));
   print_dbg_char(0x30 + (month & 0x0F));
   print_dbg("-");

   print_dbg_char(0x30 +((date >> 4)& 0x3));
   print_dbg_char(0x30 + (date & 0x0F));
   print_dbg("-20");

   print_dbg_char(0x30 +((year >> 4)& 0xF));
   print_dbg_char(0x30 + (year & 0x0F));
   print_dbg("\n\r");

}
void RTC_Set_Alarm_Date(void)
{
   unsigned char input_buffer[10];
   unsigned short data[10];
   int reply;
   int read_date, date;
   int read_month, month;

   rtc_read(ALARM_MONTH_ADDRESS, 2, (uint16*)&data[0]);
   read_month = data[0];
   read_date = data[1];
   date=0;
   month=0;

   print_dbg("Set Month (");
   print_dbg_char(0x30 +((read_month >> 4)& 0xF));
   print_dbg_char(0x30 + (read_month & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     month = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     month = ((input_buffer[0] - 0x30) << 4);
   	     month += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      month = read_month;
   }

   print_dbg("Set Date (");
   print_dbg_char(0x30 +((read_date >> 4)& 0xF));
   print_dbg_char(0x30 + (read_date & 0x0F));
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     date = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     date = ((input_buffer[0] - 0x30) << 4);
   	     date += (input_buffer[1] - 0x30);
   	  }
   }
   else
   {
      date = read_date;
   }

   data[0]=month;
   data[1]=date;
   rtc_write(ALARM_MONTH_ADDRESS, 2, (uint16*)&data[0]);
}

void RTC_Test_Alarm_Output(void)
{
   uint16 data[10];
   int month;
   int date;
   int hour;
   int minute;
   int second;

   rtc_read(SECOND_ADDRESS, 7, (uint16*)&data[0]);
   month = data[5];
   date = data[4];
   hour = data[2];
   minute = data[1];
   second = data[0];

   data[0] = month + 0xA0; //turn on alarm and battery mode
   data[1] = date;
   data[2] = hour;
   data[3] = minute + 2;
   data[4] = second;
   rtc_write(ALARM_MONTH_ADDRESS, 5, (uint16*)&data[0]);
   print_dbg("Turn power off and wait 2 minutes for system to power back on.\n\r");
}

//=============================================================================================
static void powerUnitOffNow(void)
{
	int state = 0;
	unsigned long int countdownTimer = 8000000;

	// GPIO pins used for TWI interface
	static const gpio_map_t TWI_GPIO_MAP =
	{
		{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
		{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}
	};

	// TWI options.
	twi_options_t opt;

	// options settings
	opt.pba_hz = FOSC0;
	opt.speed = TWI_SPEED;
	opt.chip = IO_ADDRESS_KPD;

	// TWI gpio pins configuration
	gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));

	// initialize TWI driver with options
	twi_master_init(&AVR32_TWI, &opt);

	init_mcp23018(IO_ADDRESS_KPD);
	state = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
	state |= 0x40;
	write_mcp23018(IO_ADDRESS_KPD, OLATA, state);
	
	while(countdownTimer--) {}
}

//=============================================================================================
void RTC_Test_Auto_On(void)
{
	unsigned short currentTime[5];
	unsigned short control2 = 0;
	unsigned int i = 0;

	// Read current time registers
	rtc_read(RTC_SECONDS_ADDR_TEMP, 1, (uint16*)&currentTime[0]);

	print_dbg("Current Seconds register: ");
	print_dbg_char(0x30 +((currentTime[0] >> 4) & 0xF));
	print_dbg_char(0x30 + (currentTime[0] & 0x0F));
	print_dbg("\r\n");

	// Set alarm time
#if 0
	// If secs > 30
	if((currentTime[0] >> 4) >= 3)
	{
		currentTime[0] = (currentTime[0] & 0x0F) | ((((currentTime[0] & 0x7F) >> 4) - 3) << 4);
	}
	else // secs < 30
	{
		currentTime[0] = (currentTime[0] & 0x0F) | ((((currentTime[0] & 0x7F) >> 4) + 3) << 4);
	}
#else
	if((currentTime[0] >> 4) >= 5)
	{
		currentTime[0] = (currentTime[0] & 0x0F) | ((((currentTime[0] & 0x7F) >> 4) - 5) << 4);
	}
	else // secs < 30
	{
		currentTime[0] = (currentTime[0] & 0x0F) | ((((currentTime[0] & 0x7F) >> 4) + 1) << 4);
	}
#endif

	print_dbg("Future Alarm Seconds register: ");
	print_dbg_char(0x30 +((currentTime[0] >> 4) & 0xF));
	print_dbg_char(0x30 + (currentTime[0] & 0x0F));
	print_dbg("\r\n");

	// Enable alarm bit by clearing it
	for(i = 0; i < 1; i++)
	{
		currentTime[i] &= ~0x80;
	}		
	
	rtc_write(RTC_SECOND_ALARM_ADDR, 1, (uint16*)&currentTime[0]);

	rtc_read(RTC_SECOND_ALARM_ADDR, 1, (uint16*)&currentTime[0]);
	print_dbg("Current Second Alarm register: 0x");
	print_dbg_hex(currentTime[0]);
	print_dbg("\r\n");

	// Clear int flags
	control2 = 0xF8;
	rtc_write(RTC_CONTROL_2_ADDR, 1, (uint16*)&control2);
	rtc_read(RTC_CONTROL_2_ADDR, 1, (uint16*)&control2);
	print_dbg("Control 2 register after clearing Interrupt Flags: 0x");
	print_dbg_hex(control2);
	print_dbg("\r\n");

	// Enable int alarm
	control2 = 0x02;
	rtc_write(RTC_CONTROL_2_ADDR, 1, (uint16*)&control2);
	rtc_read(RTC_CONTROL_2_ADDR, 1, (uint16*)&control2);
	print_dbg("Control 2 register after enabling Alarm Interrupt: 0x");
	print_dbg_hex(control2);
	print_dbg("\r\n");

#if 0
	print_dbg("\r\nWatching for Alarm interrupt flag:\r\n");
	while(1)
	{
		rtc_read(RTC_CONTROL_2_ADDR, 1, (uint16*)&control2);
		print_dbg("Control 2 register after enabling Alarm Interrupt: 0x");
		print_dbg_hex(control2);
		print_dbg("\r");
	}
#endif

	print_dbg("Rebooting unit in 10 seconds.\n\n");

	// Turn unit off
	powerUnitOffNow();
}
