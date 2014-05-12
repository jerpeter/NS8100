////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   sdram_test_menu.c                                                        //
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
#include "sram_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "utils.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
volatile unsigned short *sram = ((void *)(AVR32_EBI_CS1_ADDRESS + 0x00100000));
unsigned long sram_size = ((AVR32_EBI_CS1_SIZE >> 4) - 0x00100000);

#define NWE_SETUP     20
#define NCS_WR_SETUP  20
#define NRD_SETUP     10
#define NCS_RD_SETUP  10
#define NWE_PULSE     110
#define NCS_WR_PULSE  230
#define NRD_PULSE     135
#define NCS_RD_PULSE  250
#define NWE_CYCLE     150
#define NRD_CYCLE     180

#define EXT_SM_SIZE             16
#define NCS_CONTROLLED_READ     FALSE
#define NCS_CONTROLLED_WRITE    FALSE
#define NWAIT_MODE              0
#define PAGE_MODE               0
#define PAGE_SIZE               0
#define SMC_8_BIT_CHIPS         FALSE
#define SMC_DBW                 16
#define TDF_CYCLES              0
#define TDF_OPTIM               0

// Configure the SM Controller with SM setup and timing information for all chip select
#define SMC_CS_SETUP(ncs) { \
  U32 nwe_setup    = ((NWE_SETUP    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_wr_setup = ((NCS_WR_SETUP * hsb_mhz_up + 999) / 1000); \
  U32 nrd_setup    = ((NRD_SETUP    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_rd_setup = ((NCS_RD_SETUP * hsb_mhz_up + 999) / 1000); \
  U32 nwe_pulse    = ((NWE_PULSE    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_wr_pulse = ((NCS_WR_PULSE * hsb_mhz_up + 999) / 1000); \
  U32 nrd_pulse    = ((NRD_PULSE    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_rd_pulse = ((NCS_RD_PULSE * hsb_mhz_up + 999) / 1000); \
  U32 nwe_cycle    = ((NWE_CYCLE    * hsb_mhz_up + 999) / 1000); \
  U32 nrd_cycle    = ((NRD_CYCLE    * hsb_mhz_up + 999) / 1000); \
                                                                 \
  /* Some coherence checks...                             */     \
  /* Ensures CS is active during Rd or Wr                 */     \
  if( ncs_rd_setup + ncs_rd_pulse < nrd_setup + nrd_pulse )      \
    ncs_rd_pulse = nrd_setup + nrd_pulse - ncs_rd_setup;         \
  if( ncs_wr_setup + ncs_wr_pulse < nwe_setup + nwe_pulse )      \
    ncs_wr_pulse = nwe_setup + nwe_pulse - ncs_wr_setup;         \
                                                                 \
  /* ncs_hold = n_cycle - ncs_setup - ncs_pulse           */     \
  /* n_hold   = n_cycle - n_setup - n_pulse               */     \
  /*                                                      */     \
  /* All holds parameters must be positive or null, so:   */     \
  /* nwe_cycle shall be >= ncs_wr_setup + ncs_wr_pulse    */     \
  if( nwe_cycle < ncs_wr_setup + ncs_wr_pulse )                  \
    nwe_cycle = ncs_wr_setup + ncs_wr_pulse;                     \
                                                                 \
  /* nwe_cycle shall be >= nwe_setup + nwe_pulse          */     \
  if( nwe_cycle < nwe_setup + nwe_pulse )                        \
    nwe_cycle = nwe_setup + nwe_pulse;                           \
                                                                 \
  /* nrd_cycle shall be >= ncs_rd_setup + ncs_rd_pulse    */     \
  if( nrd_cycle < ncs_rd_setup + ncs_rd_pulse )                  \
    nrd_cycle = ncs_rd_setup + ncs_rd_pulse;                     \
                                                                 \
  /* nrd_cycle shall be >= nrd_setup + nrd_pulse          */     \
  if( nrd_cycle < nrd_setup + nrd_pulse )                        \
    nrd_cycle = nrd_setup + nrd_pulse;                           \
                                                                 \
  AVR32_SMC.cs[ncs].setup = (nwe_setup    << AVR32_SMC_SETUP0_NWE_SETUP_OFFSET) | \
                            (ncs_wr_setup << AVR32_SMC_SETUP0_NCS_WR_SETUP_OFFSET) | \
                            (nrd_setup    << AVR32_SMC_SETUP0_NRD_SETUP_OFFSET) | \
                            (ncs_rd_setup << AVR32_SMC_SETUP0_NCS_RD_SETUP_OFFSET); \
  AVR32_SMC.cs[ncs].pulse = (nwe_pulse    << AVR32_SMC_PULSE0_NWE_PULSE_OFFSET) | \
                            (ncs_wr_pulse << AVR32_SMC_PULSE0_NCS_WR_PULSE_OFFSET) | \
                            (nrd_pulse    << AVR32_SMC_PULSE0_NRD_PULSE_OFFSET) | \
                            (ncs_rd_pulse << AVR32_SMC_PULSE0_NCS_RD_PULSE_OFFSET); \
  AVR32_SMC.cs[ncs].cycle = (nwe_cycle    << AVR32_SMC_CYCLE0_NWE_CYCLE_OFFSET) | \
                            (nrd_cycle    << AVR32_SMC_CYCLE0_NRD_CYCLE_OFFSET); \
  AVR32_SMC.cs[ncs].mode = (((NCS_CONTROLLED_READ) ? AVR32_SMC_MODE0_READ_MODE_NCS_CONTROLLED : \
                           AVR32_SMC_MODE0_READ_MODE_NRD_CONTROLLED) << AVR32_SMC_MODE0_READ_MODE_OFFSET) | \
                           (((NCS_CONTROLLED_WRITE) ? AVR32_SMC_MODE0_WRITE_MODE_NCS_CONTROLLED : \
                           AVR32_SMC_MODE0_WRITE_MODE_NWE_CONTROLLED) << AVR32_SMC_MODE0_WRITE_MODE_OFFSET) | \
                           (NWAIT_MODE << AVR32_SMC_MODE0_EXNW_MODE_OFFSET) | \
                           (((SMC_8_BIT_CHIPS) ? AVR32_SMC_MODE0_BAT_BYTE_WRITE : \
                           AVR32_SMC_MODE0_BAT_BYTE_SELECT) << AVR32_SMC_MODE0_BAT_OFFSET) | \
                           (((SMC_DBW <= 8 ) ? AVR32_SMC_MODE0_DBW_8_BITS  : \
                           (SMC_DBW <= 16) ? AVR32_SMC_MODE0_DBW_16_BITS : \
                           AVR32_SMC_MODE0_DBW_32_BITS) << AVR32_SMC_MODE0_DBW_OFFSET) | \
                           (TDF_CYCLES << AVR32_SMC_MODE0_TDF_CYCLES_OFFSET) | \
                           (TDF_OPTIM << AVR32_SMC_MODE0_TDF_MODE_OFFSET) | \
                           (PAGE_MODE << AVR32_SMC_MODE0_PMEN_OFFSET) | \
                           (PAGE_SIZE << AVR32_SMC_MODE0_PS_OFFSET); \
  smc_tab_cs_size[ncs] = EXT_SM_SIZE; \
  }


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
//void smc_enable_muxed_pins(void);
//static unsigned char smc_tab_cs_size[6];

#if 0
void smc_init(unsigned long hsb_hz)
{
  unsigned long hsb_mhz_up = (hsb_hz + 999999) / 1000000;

  // Setup SMC for NCS0-3
  SMC_CS_SETUP(0)
  SMC_CS_SETUP(1)
  SMC_CS_SETUP(2)
  SMC_CS_SETUP(3)

  // Put the multiplexed MCU pins used for the SM under control of the SMC.
  smc_enable_muxed_pins();
}

void smc_enable_muxed_pins(void)
{
  static const gpio_map_t SMC_EBI_GPIO_MAP =
  {
    // Enable data pins.
    {AVR32_EBI_DATA_0_PIN, AVR32_EBI_DATA_0_FUNCTION},
    {AVR32_EBI_DATA_1_PIN, AVR32_EBI_DATA_1_FUNCTION},
    {AVR32_EBI_DATA_2_PIN, AVR32_EBI_DATA_2_FUNCTION},
    {AVR32_EBI_DATA_3_PIN, AVR32_EBI_DATA_3_FUNCTION},
    {AVR32_EBI_DATA_4_PIN, AVR32_EBI_DATA_4_FUNCTION},
    {AVR32_EBI_DATA_5_PIN, AVR32_EBI_DATA_5_FUNCTION},
    {AVR32_EBI_DATA_6_PIN, AVR32_EBI_DATA_6_FUNCTION},
    {AVR32_EBI_DATA_7_PIN, AVR32_EBI_DATA_7_FUNCTION},
    {AVR32_EBI_DATA_8_PIN, AVR32_EBI_DATA_8_FUNCTION},
    {AVR32_EBI_DATA_9_PIN ,AVR32_EBI_DATA_9_FUNCTION},
    {AVR32_EBI_DATA_10_PIN, AVR32_EBI_DATA_10_FUNCTION},
    {AVR32_EBI_DATA_11_PIN, AVR32_EBI_DATA_11_FUNCTION},
    {AVR32_EBI_DATA_12_PIN, AVR32_EBI_DATA_12_FUNCTION},
    {AVR32_EBI_DATA_13_PIN, AVR32_EBI_DATA_13_FUNCTION},
    {AVR32_EBI_DATA_14_PIN, AVR32_EBI_DATA_14_FUNCTION},
    {AVR32_EBI_DATA_15_PIN, AVR32_EBI_DATA_15_FUNCTION},

    // Enable address pins.
    {AVR32_EBI_ADDR_0_PIN, AVR32_EBI_ADDR_0_FUNCTION},
    {AVR32_EBI_ADDR_1_PIN, AVR32_EBI_ADDR_1_FUNCTION},
    {AVR32_EBI_ADDR_2_PIN, AVR32_EBI_ADDR_2_FUNCTION},
    {AVR32_EBI_ADDR_3_PIN, AVR32_EBI_ADDR_3_FUNCTION},
    {AVR32_EBI_ADDR_4_PIN, AVR32_EBI_ADDR_4_FUNCTION},
    {AVR32_EBI_ADDR_5_PIN, AVR32_EBI_ADDR_5_FUNCTION},
    {AVR32_EBI_ADDR_6_PIN, AVR32_EBI_ADDR_6_FUNCTION},
    {AVR32_EBI_ADDR_7_PIN, AVR32_EBI_ADDR_7_FUNCTION},
    {AVR32_EBI_ADDR_8_PIN, AVR32_EBI_ADDR_8_FUNCTION},
    {AVR32_EBI_ADDR_9_PIN, AVR32_EBI_ADDR_9_FUNCTION},
    {AVR32_EBI_ADDR_10_PIN, AVR32_EBI_ADDR_10_FUNCTION},
    {AVR32_EBI_ADDR_11_PIN, AVR32_EBI_ADDR_11_FUNCTION},
    {AVR32_EBI_ADDR_12_PIN, AVR32_EBI_ADDR_12_FUNCTION},
    {AVR32_EBI_ADDR_13_PIN, AVR32_EBI_ADDR_13_FUNCTION},
    {AVR32_EBI_ADDR_14_PIN, AVR32_EBI_ADDR_14_FUNCTION},
    {AVR32_EBI_ADDR_15_PIN, AVR32_EBI_ADDR_15_FUNCTION},
    {AVR32_EBI_ADDR_16_PIN, AVR32_EBI_ADDR_16_FUNCTION},
    {AVR32_EBI_ADDR_17_PIN, AVR32_EBI_ADDR_17_FUNCTION},
    {AVR32_EBI_ADDR_18_PIN, AVR32_EBI_ADDR_18_FUNCTION},
    {AVR32_EBI_ADDR_19_PIN, AVR32_EBI_ADDR_19_FUNCTION},
    {AVR32_EBI_ADDR_20_1_PIN, AVR32_EBI_ADDR_20_1_FUNCTION},
    {AVR32_EBI_ADDR_21_1_PIN, AVR32_EBI_ADDR_21_1_FUNCTION},
    {AVR32_EBI_ADDR_22_1_PIN, AVR32_EBI_ADDR_22_1_FUNCTION},
    {AVR32_EBI_ADDR_23_PIN, AVR32_EBI_ADDR_23_FUNCTION},

    {AVR32_EBI_NWE0_0_PIN, AVR32_EBI_NWE0_0_FUNCTION},
    {AVR32_EBI_NWE1_0_PIN, AVR32_EBI_NWE1_0_FUNCTION},
    {AVR32_EBI_NRD_0_PIN, AVR32_EBI_NRD_0_FUNCTION},
    {AVR32_EBI_NCS_0_1_PIN, AVR32_EBI_NCS_0_1_FUNCTION},
    {AVR32_EBI_NCS_1_PIN, AVR32_EBI_NCS_1_FUNCTION},
    {AVR32_EBI_NCS_2_PIN, AVR32_EBI_NCS_2_FUNCTION},
    {AVR32_EBI_NCS_3_PIN, AVR32_EBI_NCS_3_FUNCTION},
 };

  gpio_enable_module(SMC_EBI_GPIO_MAP, sizeof(SMC_EBI_GPIO_MAP) / sizeof(SMC_EBI_GPIO_MAP[0]));
}
#endif

//static unsigned char smc_get_cs_size(unsigned char cs)
//{
//  return smc_tab_cs_size[cs];
//}

void SRAM_Test_Menu(void)
{
   // Initialize sram driver resources.
//   gpio_enable_gpio_pin(AVR32_PIN_PX12);

   //smc_init(FOSC0);
   Menu_Items = (sizeof(SRAM_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)SRAM_Test_Menu_Functions;
   Menu_String = (unsigned char *)SRAM_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  SDRAM_Exit                                                          //
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
void SRAM_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void SRAM_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = SRAM_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = SRAM_Info_Text[count];
		}
}

void SRAM_Erase(void)
{
   unsigned long progress_inc, i, j;

   progress_inc = (sram_size + 50) / 100;

   for (i = 0, j = 0; i < sram_size; i++)
   {
       if (i == j * progress_inc)
       {
           print_dbg("\rTesting SRAM write..............");
           print_dbg_ulong(j++);
           print_dbg_char('%');
       }
       sram[i] = 0x0000;
   }
   print_dbg("\n\rDevice erased.\n\r");
}

void SRAM_Fill(void)
{
   unsigned long  progress_inc, i, j;
   unsigned short data_word;
   unsigned char  input_buffer[10];
   int reply;

   progress_inc = (sram_size + 50) / 100;

   print_dbg("Set fill pattern (");
   print_dbg_short_hex(0x1234);
   print_dbg("): ");
   reply = Get_User_Input(input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     data_word = (input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     data_word =  ((input_buffer[0] - 0x30) << 4);
   	     data_word += (input_buffer[1] - 0x30);
   	  }
   	  if(reply == 3)
      {
   	     data_word =  ((input_buffer[0] - 0x30) << 8);
   	     data_word += ((input_buffer[1] - 0x30) << 4);
   	     data_word += (input_buffer[2] - 0x30);
   	  }
   	  if(reply == 4)
      {
   	     data_word =  ((input_buffer[0] - 0x30) << 12);
   	     data_word += ((input_buffer[1] - 0x30) << 8);
   	     data_word += ((input_buffer[2] - 0x30) << 4);
   	     data_word += (input_buffer[3] - 0x30);
   	  }
   }
   else
   {
      data_word = 0x1234;
   }

   for (i = 0, j = 0; i < sram_size; i++)
   {
       if (i == j * progress_inc)
       {
           print_dbg("\rTesting SRAM write..............");
           print_dbg_ulong(j++);
           print_dbg_char('%');
       }
       sram[i] = data_word;
   }

   print_dbg("\n\rDevice filled with pattern ");
   print_dbg_short_hex(data_word);
   print_dbg(".\n\r");
}

void SRAM_Read(void)
{
   unsigned long i;

   print_dbg("Device location 0 - 32:\n\r");
   for(i=0;i<16;i++)
   {
      print_dbg_short_hex(sram[i]);
      print_dbg(" ");
   }
   print_dbg("\n\r");

   for(i=16;i<32;i++)
   {
      print_dbg_short_hex(sram[i]);
      print_dbg(" ");
   }
   print_dbg("\n\r");
}

void SRAM_Write(void)
{
   unsigned long i;

   for (i = 0; i < 32; i++)
   {
      sram[i] = i;
   }
   print_dbg("Device location 0 - 32 written with count pattern.\n\r");
}


void SRAM_Test(void)
{
    unsigned long progress_inc, i, j, tmp, noErrors = 0;

    progress_inc = (sram_size + 50) / 1000;

    print_dbg("\n\rSRAM Addr: 0x");
    print_dbg_hex((unsigned long int)&sram[0]);

	volatile unsigned long int *extRamKey = (volatile unsigned long int*)0xD0000000;
    print_dbg("\n\rSRAM Data Key: 0x");
    print_dbg_hex((unsigned long int)*extRamKey);
    print_dbg("\r\n\n");

    for (i = 0, j = 0; i < sram_size / 10 ; i++)
    {
        if (i == j * progress_inc)
        {
            print_dbg("\rTesting SRAM write..............");
            print_dbg_ulong(j++);
            print_dbg_char('%');
        }
        sram[i] = i;
    }
    print_dbg("\rTesting SRAM write..............PASSED!\n\r");

    for (i = 0, j = 0; i < sram_size / 10; i++)
    {
        if (i == j * progress_inc)
        {
            print_dbg("\rTesting SRAM read...............");
            print_dbg_ulong(j++);
            print_dbg_char('%');
        }
        tmp = sram[i];
        if (tmp != (i & 0xFFFF))
        {
            noErrors++;
        }
    }

    if(noErrors == 0)
    {
        print_dbg("\rTesting SRAM read...............PASSED!\n\r");
    }
    else
    {
        print_dbg("\rTesting SRAM read...............FAILED! ");
        print_dbg_ulong(noErrors);
        print_dbg(" corrupted word(s)\n\r");
    }
}

void SRAM_Test_2M(void)
{
	volatile unsigned long int i = 0;
	volatile unsigned short int *sramAddr = (volatile unsigned short int*)0xD0200000;
    print_dbg("\n\rSRAM Test above 2M... \n");
    for (i = 0; i < 0x80000 ; i++) { *sramAddr++ = (unsigned short int)i; if((i % 0x400) == 0) print_dbg_char('+'); }
	sramAddr = (volatile unsigned short int*)0xD0200000;
    for (i = 0; i < 0x80000 ; i++) { if (*sramAddr++ != (unsigned short int)i) 
	{ print_dbg("\r\nTesting SRAM... FAILED! "); print_dbg_ulong(i); print_dbg_char(':'); print_dbg_ulong(*--sramAddr); print_dbg("\r\n"); return; }
		if((i % 0x400) == 0) print_dbg_char('?'); }
    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

#define TEST_32_BAADFOOD	0xBAADF00D
#define TEST_32_VALUE	0x87654321
void SRAM_Test_2M_by_32(void)
{
	unsigned long int i = 0, j = 0, breakCount = 0;
	unsigned long int *sramAddr = (unsigned long int*)0xD0200000;
	static unsigned long int static_test_BAADFOOD = 0xBAADF11D;
	static unsigned long int static_test_value = 0x98765432;
	unsigned long int test_BAADFOOD = 0xBAADF22D;
	unsigned long int test_value = 0x09876543;
	unsigned long int test32;
	unsigned long int *test32ptr = (unsigned long int*)0xD0200000;
	unsigned long int *test32ptr0 = (unsigned long int*)0xD0200000;
	unsigned long int *test32ptr1 = (unsigned long int*)0xD0200004;
	unsigned long int *test32ptr2 = (unsigned long int*)0xD0200008;
	unsigned long int *test32ptr3 = (unsigned long int*)0xD020000C;
	unsigned long int *test32ptr4 = (unsigned long int*)0xD0200010;
	unsigned long int *test32ptr5 = (unsigned long int*)0xD0200014;
	unsigned long int *test32ptr6 = (unsigned long int*)0xD0200018;
	unsigned long int *test32ptr7 = (unsigned long int*)0xD020001C;
	unsigned long int *test32ptr8 = (unsigned long int*)0xD0200020;
	unsigned long int *test32ptr9 = (unsigned long int*)0xD0200024;
	
    //-------------------------------------------------------------
	print_dbg("\n\r32-bit Storage element tests... \n");
	print_dbg("Define Baadfood (0xBAADF00D): ");
	print_dbg_hex(TEST_32_BAADFOOD);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Define 32 Value (0x87654321): ");
	print_dbg_hex(TEST_32_VALUE);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Static Baadfood (0xBAADF11D): ");
	print_dbg_hex(static_test_BAADFOOD);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Static 32 Value (0x98765432): ");
	print_dbg_hex(static_test_value);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Dynamic Baadfood (0xBAADF22D): ");
	print_dbg_hex(test_BAADFOOD);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Dynamic 32 Value (0x09876543): ");
	print_dbg_hex(test_value);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Value (0x1248ABCD): ");
	test32 = 0x1248ABCD;
	print_dbg_hex(test32);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value (0x80804040): ");
	*test32ptr = 0x80804040;
	print_dbg_hex(*test32ptr);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Location Inc x1: ");
	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex((unsigned long int)test32ptr);
		test32ptr++;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Location Inc x2: ");
	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex((unsigned long int)test32ptr);
		test32ptr += 2;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Location Inc x4: ");
	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex((unsigned long int)test32ptr);
		test32ptr += 4;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Stack 32 Ptr Inc w/ Self    : ");
	test32ptr = (unsigned long int*)0xD0200000;
	*test32ptr = 0x80804040;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex(*test32ptr);
		test32ptr++;
		*test32ptr = *(test32ptr - 1) + 1;
		print_dbg(" ");
	}
	print_dbg("\n");

	print_dbg("Reprint values              : ");
	print_dbg_hex(*((unsigned long int*)0xD0200000)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200004)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200008)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020000C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200010)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200014)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200018)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020001C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200020)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200024)); print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Stack 32 Ptr Inc w/ Value   : ");
	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = 0x10100000 + i;
		print_dbg_hex(*test32ptr);
		test32ptr++;
		print_dbg(" ");
	}
	print_dbg("\n");

	print_dbg("Reprint values              : ");
	print_dbg_hex(*((unsigned long int*)0xD0200000)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200004)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200008)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020000C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200010)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200014)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200018)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020001C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200020)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200024)); print_dbg(" ");
	print_dbg("\n");

#if 0
    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 2: ");
	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = 0x20200000 + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 2;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 4: ");
	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = 0x40400000 + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 4;
		print_dbg(" ");
	}
	print_dbg("\n");
#endif

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Stack 32 Ptr Inc w/ Self+Var: ");
	test32ptr = (unsigned long int*)0xD0200000;
	*test32ptr = 0xA0A00000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex(*test32ptr);
		test32ptr++;
		*test32ptr = *(test32ptr - 1) + i;
		print_dbg(" ");
	}
	print_dbg("\n");

	print_dbg("Reprint values              : ");
	print_dbg_hex(*((unsigned long int*)0xD0200000)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200004)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200008)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020000C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200010)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200014)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200018)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020001C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200020)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200024)); print_dbg(" ");
	print_dbg("\n");

#if 0
    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 2: ");
	test32ptr = (unsigned long int*)0xD0200000;
	*test32ptr = 0x20200000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = *test32ptr + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 2;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 4: ");
	test32ptr = (unsigned long int*)0xD0200000;
	*test32ptr = 0x40400000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = *test32ptr + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 4;
		print_dbg(" ");
	}
	print_dbg("\n");
#endif

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Preassigned Raw Values      : ");
	*((unsigned long int*)0xD0200000) = 0x5050001;
	*((unsigned long int*)0xD0200004) = 0x5050002;
	*((unsigned long int*)0xD0200008) = 0x5050003;
	*((unsigned long int*)0xD020000C) = 0x5050004;
	*((unsigned long int*)0xD0200010) = 0x5050005;
	*((unsigned long int*)0xD0200014) = 0x5050006;
	*((unsigned long int*)0xD0200018) = 0x5050007;
	*((unsigned long int*)0xD020001C) = 0x5050008;
	*((unsigned long int*)0xD0200020) = 0x5050009;
	*((unsigned long int*)0xD0200024) = 0x505000A;
	print_dbg_hex(*((unsigned long int*)0xD0200000));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200004));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200008));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020000C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200010));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200014));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200018));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020001C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200020));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200024));
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Test Ptr Value Individual   : ");
	*test32ptr0 = 0x80804040;
	*test32ptr1 = 0x80804041;
	*test32ptr2 = 0x80804042;
	*test32ptr3 = 0x80804043;
	*test32ptr4 = 0x80804044;
	*test32ptr5 = 0x80804045;
	*test32ptr6 = 0x80804046;
	*test32ptr7 = 0x80804047;
	*test32ptr8 = 0x80804048;
	*test32ptr9 = 0x80804049;
	print_dbg_hex(*test32ptr0);
	print_dbg(" ");
	print_dbg_hex(*test32ptr1);
	print_dbg(" ");
	print_dbg_hex(*test32ptr2);
	print_dbg(" ");
	print_dbg_hex(*test32ptr3);
	print_dbg(" ");
	print_dbg_hex(*test32ptr4);
	print_dbg(" ");
	print_dbg_hex(*test32ptr5);
	print_dbg(" ");
	print_dbg_hex(*test32ptr6);
	print_dbg(" ");
	print_dbg_hex(*test32ptr7);
	print_dbg(" ");
	print_dbg_hex(*test32ptr8);
	print_dbg(" ");
	print_dbg_hex(*test32ptr9);
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Preassigned Raw Values W/Slf: ");
	*((unsigned long int*)0xD0200000) = 0x5050001;
	*((unsigned long int*)0xD0200004) = *((unsigned long int*)0xD0200000) + 1;
	*((unsigned long int*)0xD0200008) = *((unsigned long int*)0xD0200004) + 1;
	*((unsigned long int*)0xD020000C) = *((unsigned long int*)0xD0200008) + 1;
	*((unsigned long int*)0xD0200010) = *((unsigned long int*)0xD020000C) + 1;
	*((unsigned long int*)0xD0200014) = *((unsigned long int*)0xD0200010) + 1;
	*((unsigned long int*)0xD0200018) = *((unsigned long int*)0xD0200014) + 1;
	*((unsigned long int*)0xD020001C) = *((unsigned long int*)0xD0200018) + 1;
	*((unsigned long int*)0xD0200020) = *((unsigned long int*)0xD020001C) + 1;
	*((unsigned long int*)0xD0200024) = *((unsigned long int*)0xD0200020) + 1;
	print_dbg_hex(*((unsigned long int*)0xD0200000));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200004));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200008));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020000C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200010));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200014));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200018));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD020001C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200020));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0200024));
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Test Ptr Value Indiv W/Self : ");
	*test32ptr0 = 0x80804040;
	*test32ptr1 = *test32ptr0 + 1;
	*test32ptr2 = *test32ptr1 + 1;
	*test32ptr3 = *test32ptr2 + 1;
	*test32ptr4 = *test32ptr3 + 1;
	*test32ptr5 = *test32ptr4 + 1;
	*test32ptr6 = *test32ptr5 + 1;
	*test32ptr7 = *test32ptr6 + 1;
	*test32ptr8 = *test32ptr7 + 1;
	*test32ptr9 = *test32ptr8 + 1;
	print_dbg_hex(*test32ptr0);
	print_dbg(" ");
	print_dbg_hex(*test32ptr1);
	print_dbg(" ");
	print_dbg_hex(*test32ptr2);
	print_dbg(" ");
	print_dbg_hex(*test32ptr3);
	print_dbg(" ");
	print_dbg_hex(*test32ptr4);
	print_dbg(" ");
	print_dbg_hex(*test32ptr5);
	print_dbg(" ");
	print_dbg_hex(*test32ptr6);
	print_dbg(" ");
	print_dbg_hex(*test32ptr7);
	print_dbg(" ");
	print_dbg_hex(*test32ptr8);
	print_dbg(" ");
	print_dbg_hex(*test32ptr9);
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0200000 20K: ");
	test32ptr = (unsigned long int*)0xD0200000;
	breakCount = 0;
	for (i = 0; i < 0x20000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 0x20000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 100)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0200000 20K: ");
	test32ptr = (unsigned long int*)0xD0200000;
	breakCount = 0;
	for (i = 0; i < 0x20000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 0x20000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 25)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0220000 20K: ");
	test32ptr = (unsigned long int*)0xD0220000;
	breakCount = 0;
	for (i = 0; i < 0x20000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0220000;
	for (i = 0; i < 0x20000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 25)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0200000 30K: ");
	test32ptr = (unsigned long int*)0xD0200000;
	breakCount = 0;
	for (i = 0; i < 0x30000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0200000;
	for (i = 0; i < 0x30000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 25)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Mem Test @ D0200000 Loop 20K: ");
	for (j = 0; j < 50; j++)
	{
		test32ptr = (unsigned long int*)0xD0200000;
		breakCount = 0;
		for (i = 0; i < 0x20000; i++)
		{
			*test32ptr++ = i;
		}

		test32ptr = (unsigned long int*)0xD0200000;
		for (i = 0; i < 0x20000; i++)
		{
			if (*test32ptr != i)
			{
				print_dbg("\nData mismatch! Addr: ");
				print_dbg_hex((unsigned long int)test32ptr);
				print_dbg(" Data: ");
				print_dbg_hex(*test32ptr);
				print_dbg(" i: ");
				print_dbg_hex(i);
			
				breakCount++;
			}

			if (breakCount > 10)
				break;

			test32ptr++;
		}

		print_dbg(".");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ Loop Inc 0x100: ");
	for (j = 0; j < 50; j++)
	{
		test32ptr = (unsigned long int*)0xD0200000;
		breakCount = 0;
		for (i = 0; i < (0x20000 + j * 0x100); i++)
		{
			*test32ptr++ = i;
		}

		test32ptr = (unsigned long int*)0xD0200000;
		for (i = 0; i < (0x20000 + j * 0x100); i++)
		{
			if (*test32ptr != i)
			{
				print_dbg("\nData mismatch! Addr: ");
				print_dbg_hex((unsigned long int)test32ptr);
				print_dbg(" Data: ");
				print_dbg_hex(*test32ptr);
				print_dbg(" i: ");
				print_dbg_hex(i);
			
				breakCount++;
			}

			if (breakCount > 10)
				break;

			test32ptr++;
		}

		if (breakCount > 10)
		{
			print_dbg("\nLoop Count for mismatch is: ");
			print_dbg_hex((0x20000 + j * 0x100));
			break;
		}			
		print_dbg(".");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
    print_dbg("\n\rSRAM Test above 2M (40K)... \n");
    for (i = 0; i < 0x40000; i++)
	{ 
		*sramAddr++ = i;
		if ((i % 0x200) == 0)
		print_dbg_char('+');
	}
	
	sramAddr = (unsigned long int*)0xD0200000;
    for (i = 0; i < 0x40000; i++) 
	{ 
		if (*sramAddr != i)
		{ 
			print_dbg("\r\nTesting SRAM... FAILED! "); 
			print_dbg_hex(i); print_dbg_char(':'); print_dbg_hex(*sramAddr); print_dbg("\r\n"); 
			print_dbg("\r\n"); 
			return;
		}

		sramAddr++;

		if((i % 0x200) == 0) print_dbg_char('?');
	}

    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

void SRAM_Test_1M_by_32(void)
{
	unsigned long int i = 0, j = 0, breakCount = 0;
	unsigned long int *sramAddr = (unsigned long int*)0xD0100000;
	static unsigned long int static_test_BAADFOOD = 0xBAADF11D;
	static unsigned long int static_test_value = 0x98765432;
	unsigned long int test_BAADFOOD = 0xBAADF22D;
	unsigned long int test_value = 0x09876543;
	unsigned long int test32;
	unsigned long int *test32ptr = (unsigned long int*)0xD0100000;
	unsigned long int *test32ptr0 = (unsigned long int*)0xD0100000;
	unsigned long int *test32ptr1 = (unsigned long int*)0xD0100004;
	unsigned long int *test32ptr2 = (unsigned long int*)0xD0100008;
	unsigned long int *test32ptr3 = (unsigned long int*)0xD010000C;
	unsigned long int *test32ptr4 = (unsigned long int*)0xD0100010;
	unsigned long int *test32ptr5 = (unsigned long int*)0xD0100014;
	unsigned long int *test32ptr6 = (unsigned long int*)0xD0100018;
	unsigned long int *test32ptr7 = (unsigned long int*)0xD010001C;
	unsigned long int *test32ptr8 = (unsigned long int*)0xD0100020;
	unsigned long int *test32ptr9 = (unsigned long int*)0xD0100024;
	
    //-------------------------------------------------------------
	print_dbg("\n\r32-bit Storage element tests... \n");
	print_dbg("Define Baadfood (0xBAADF00D): ");
	print_dbg_hex(TEST_32_BAADFOOD);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Define 32 Value (0x87654321): ");
	print_dbg_hex(TEST_32_VALUE);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Static Baadfood (0xBAADF11D): ");
	print_dbg_hex(static_test_BAADFOOD);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Static 32 Value (0x98765432): ");
	print_dbg_hex(static_test_value);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Dynamic Baadfood (0xBAADF22D): ");
	print_dbg_hex(test_BAADFOOD);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Dynamic 32 Value (0x09876543): ");
	print_dbg_hex(test_value);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Value (0x1248ABCD): ");
	test32 = 0x1248ABCD;
	print_dbg_hex(test32);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value (0x80804040): ");
	*test32ptr = 0x80804040;
	print_dbg_hex(*test32ptr);
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Location Inc x1: ");
	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex((unsigned long int)test32ptr);
		test32ptr++;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Location Inc x2: ");
	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex((unsigned long int)test32ptr);
		test32ptr += 2;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Location Inc x4: ");
	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex((unsigned long int)test32ptr);
		test32ptr += 4;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Stack 32 Ptr Inc w/ Self    : ");
	test32ptr = (unsigned long int*)0xD0100000;
	*test32ptr = 0x80804040;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex(*test32ptr);
		test32ptr++;
		*test32ptr = *(test32ptr - 1) + 1;
		print_dbg(" ");
	}
	print_dbg("\n");

	print_dbg("Reprint values              : ");
	print_dbg_hex(*((unsigned long int*)0xD0100000)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100004)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100008)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010000C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100010)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100014)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100018)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010001C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100020)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100024)); print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Stack 32 Ptr Inc w/ Value   : ");
	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = 0x10100000 + i;
		print_dbg_hex(*test32ptr);
		test32ptr++;
		print_dbg(" ");
	}
	print_dbg("\n");

	print_dbg("Reprint values              : ");
	print_dbg_hex(*((unsigned long int*)0xD0100000)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100004)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100008)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010000C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100010)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100014)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100018)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010001C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100020)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100024)); print_dbg(" ");
	print_dbg("\n");

#if 0
    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 2: ");
	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = 0x20200000 + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 2;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 4: ");
	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = 0x40400000 + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 4;
		print_dbg(" ");
	}
	print_dbg("\n");
#endif

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Stack 32 Ptr Inc w/ Self+Var: ");
	test32ptr = (unsigned long int*)0xD0100000;
	*test32ptr = 0xA0A00000;
	for (i = 0; i < 10; i++)
	{
		print_dbg_hex(*test32ptr);
		test32ptr++;
		*test32ptr = *(test32ptr - 1) + i;
		print_dbg(" ");
	}
	print_dbg("\n");

	print_dbg("Reprint values              : ");
	print_dbg_hex(*((unsigned long int*)0xD0100000)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100004)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100008)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010000C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100010)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100014)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100018)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010001C)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100020)); print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100024)); print_dbg(" ");
	print_dbg("\n");

#if 0
    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 2: ");
	test32ptr = (unsigned long int*)0xD0100000;
	*test32ptr = 0x20200000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = *test32ptr + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 2;
		print_dbg(" ");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Stack 32 Ptr Value Inc Ptr 4: ");
	test32ptr = (unsigned long int*)0xD0100000;
	*test32ptr = 0x40400000;
	for (i = 0; i < 10; i++)
	{
		*test32ptr = *test32ptr + i;
		print_dbg_hex(*test32ptr);
		test32ptr += 4;
		print_dbg(" ");
	}
	print_dbg("\n");
#endif

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Preassigned Raw Values      : ");
	*((unsigned long int*)0xD0100000) = 0x5050001;
	*((unsigned long int*)0xD0100004) = 0x5050002;
	*((unsigned long int*)0xD0100008) = 0x5050003;
	*((unsigned long int*)0xD010000C) = 0x5050004;
	*((unsigned long int*)0xD0100010) = 0x5050005;
	*((unsigned long int*)0xD0100014) = 0x5050006;
	*((unsigned long int*)0xD0100018) = 0x5050007;
	*((unsigned long int*)0xD010001C) = 0x5050008;
	*((unsigned long int*)0xD0100020) = 0x5050009;
	*((unsigned long int*)0xD0100024) = 0x505000A;
	print_dbg_hex(*((unsigned long int*)0xD0100000));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100004));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100008));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010000C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100010));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100014));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100018));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010001C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100020));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100024));
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Test Ptr Value Individual   : ");
	*test32ptr0 = 0x80804040;
	*test32ptr1 = 0x80804041;
	*test32ptr2 = 0x80804042;
	*test32ptr3 = 0x80804043;
	*test32ptr4 = 0x80804044;
	*test32ptr5 = 0x80804045;
	*test32ptr6 = 0x80804046;
	*test32ptr7 = 0x80804047;
	*test32ptr8 = 0x80804048;
	*test32ptr9 = 0x80804049;
	print_dbg_hex(*test32ptr0);
	print_dbg(" ");
	print_dbg_hex(*test32ptr1);
	print_dbg(" ");
	print_dbg_hex(*test32ptr2);
	print_dbg(" ");
	print_dbg_hex(*test32ptr3);
	print_dbg(" ");
	print_dbg_hex(*test32ptr4);
	print_dbg(" ");
	print_dbg_hex(*test32ptr5);
	print_dbg(" ");
	print_dbg_hex(*test32ptr6);
	print_dbg(" ");
	print_dbg_hex(*test32ptr7);
	print_dbg(" ");
	print_dbg_hex(*test32ptr8);
	print_dbg(" ");
	print_dbg_hex(*test32ptr9);
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Preassigned Raw Values W/Slf: ");
	*((unsigned long int*)0xD0100000) = 0x5050001;
	*((unsigned long int*)0xD0100004) = *((unsigned long int*)0xD0100000) + 1;
	*((unsigned long int*)0xD0100008) = *((unsigned long int*)0xD0100004) + 1;
	*((unsigned long int*)0xD010000C) = *((unsigned long int*)0xD0100008) + 1;
	*((unsigned long int*)0xD0100010) = *((unsigned long int*)0xD010000C) + 1;
	*((unsigned long int*)0xD0100014) = *((unsigned long int*)0xD0100010) + 1;
	*((unsigned long int*)0xD0100018) = *((unsigned long int*)0xD0100014) + 1;
	*((unsigned long int*)0xD010001C) = *((unsigned long int*)0xD0100018) + 1;
	*((unsigned long int*)0xD0100020) = *((unsigned long int*)0xD010001C) + 1;
	*((unsigned long int*)0xD0100024) = *((unsigned long int*)0xD0100020) + 1;
	print_dbg_hex(*((unsigned long int*)0xD0100000));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100004));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100008));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010000C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100010));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100014));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100018));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD010001C));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100020));
	print_dbg(" ");
	print_dbg_hex(*((unsigned long int*)0xD0100024));
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("Test Ptr Value Indiv W/Self : ");
	*test32ptr0 = 0x80804040;
	*test32ptr1 = *test32ptr0 + 1;
	*test32ptr2 = *test32ptr1 + 1;
	*test32ptr3 = *test32ptr2 + 1;
	*test32ptr4 = *test32ptr3 + 1;
	*test32ptr5 = *test32ptr4 + 1;
	*test32ptr6 = *test32ptr5 + 1;
	*test32ptr7 = *test32ptr6 + 1;
	*test32ptr8 = *test32ptr7 + 1;
	*test32ptr9 = *test32ptr8 + 1;
	print_dbg_hex(*test32ptr0);
	print_dbg(" ");
	print_dbg_hex(*test32ptr1);
	print_dbg(" ");
	print_dbg_hex(*test32ptr2);
	print_dbg(" ");
	print_dbg_hex(*test32ptr3);
	print_dbg(" ");
	print_dbg_hex(*test32ptr4);
	print_dbg(" ");
	print_dbg_hex(*test32ptr5);
	print_dbg(" ");
	print_dbg_hex(*test32ptr6);
	print_dbg(" ");
	print_dbg_hex(*test32ptr7);
	print_dbg(" ");
	print_dbg_hex(*test32ptr8);
	print_dbg(" ");
	print_dbg_hex(*test32ptr9);
	print_dbg(" ");
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0100000 20K: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x20000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x20000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 100)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0100000 20K: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x20000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x20000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 25)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0120000 20K: ");
	test32ptr = (unsigned long int*)0xD0120000;
	breakCount = 0;
	for (i = 0; i < 0x20000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0120000;
	for (i = 0; i < 0x20000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 25)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ 0xD0100000 30K: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x30000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x30000; i++)
	{
		if (*test32ptr != i)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)test32ptr);
			print_dbg(" Data: ");
			print_dbg_hex(*test32ptr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			
			breakCount++;
		}
		else
		{
			if (i % 0x100 == 0)
				print_dbg(".");
		}

		if (breakCount > 25)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Mem Test @ D0100000 Loop 20K: ");
	for (j = 0; j < 50; j++)
	{
		test32ptr = (unsigned long int*)0xD0100000;
		breakCount = 0;
		for (i = 0; i < 0x20000; i++)
		{
			*test32ptr++ = i;
		}

		test32ptr = (unsigned long int*)0xD0100000;
		for (i = 0; i < 0x20000; i++)
		{
			if (*test32ptr != i)
			{
				print_dbg("\nData mismatch! Addr: ");
				print_dbg_hex((unsigned long int)test32ptr);
				print_dbg(" Data: ");
				print_dbg_hex(*test32ptr);
				print_dbg(" i: ");
				print_dbg_hex(i);
			
				breakCount++;
			}

			if (breakCount > 10)
				break;

			test32ptr++;
		}

		print_dbg(".");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Memory Test @ Loop Inc 0x100: ");
	for (j = 0; j < 50; j++)
	{
		test32ptr = (unsigned long int*)0xD0100000;
		breakCount = 0;
		for (i = 0; i < (0x20000 + j * 0x100); i++)
		{
			*test32ptr++ = i;
		}

		test32ptr = (unsigned long int*)0xD0100000;
		for (i = 0; i < (0x20000 + j * 0x100); i++)
		{
			if (*test32ptr != i)
			{
				print_dbg("\nData mismatch! Addr: ");
				print_dbg_hex((unsigned long int)test32ptr);
				print_dbg(" Data: ");
				print_dbg_hex(*test32ptr);
				print_dbg(" i: ");
				print_dbg_hex(i);
			
				breakCount++;
			}

			if (breakCount > 10)
				break;

			test32ptr++;
		}

		if (breakCount > 10)
		{
			print_dbg("\nLoop Count for mismatch is: ");
			print_dbg_hex((0x20000 + j * 0x100));
			break;
		}			
		print_dbg(".");
	}
	print_dbg("\n");

    //-------------------------------------------------------------
    print_dbg("\n\rSRAM Test above 2M (40K)... \n");
    for (i = 0; i < 0x40000; i++)
	{ 
		*sramAddr++ = i;
		if ((i % 0x200) == 0)
		print_dbg_char('+');
	}
	
	sramAddr = (unsigned long int*)0xD0100000;
    for (i = 0; i < 0x40000; i++) 
	{ 
		if (*sramAddr != i)
		{ 
			print_dbg("\r\nTesting SRAM... FAILED! "); 
			print_dbg_hex(i); print_dbg_char(':'); print_dbg_hex(*sramAddr); print_dbg("\r\n"); 
			print_dbg("\r\n"); 
			return;
		}

		sramAddr++;

		if((i % 0x200) == 0) print_dbg_char('?');
	}

    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

void SRAM_Test_3M(void)
{
	volatile unsigned long int i = 0;
	volatile unsigned short int *sramAddr = (volatile unsigned short int*)0xD0300000;
    print_dbg("\n\rSRAM Test above 3M... \n");
    for (i = 0; i < 0x80000 ; i++) { *sramAddr++ = (unsigned short int)i; if((i % 0x400) == 0) print_dbg_char('+'); }
	sramAddr = (volatile unsigned short int*)0xD0300000;
    for (i = 0; i < 0x80000 ; i++) { if (*sramAddr++ != (unsigned short int)i) 
	{ print_dbg("\r\nTesting SRAM... FAILED! "); print_dbg_ulong(i); print_dbg_char(':'); print_dbg_ulong(*--sramAddr); print_dbg("\r\n"); return; }
		if((i % 0x400) == 0) print_dbg_char('?'); }
    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

void SRAM_Test_4M(void)
{
	volatile unsigned long int i = 0;
	volatile unsigned short int *sramAddr = (volatile unsigned short int*)0xD0400000;
    print_dbg("\n\rSRAM Test above 4M... \n");
    for (i = 0; i < 0x80000 ; i++) { *sramAddr++ = (unsigned short int)i; if((i % 0x400) == 0) print_dbg_char('+'); }
	sramAddr = (volatile unsigned short int*)0xD0400000;
    for (i = 0; i < 0x80000 ; i++) { if (*sramAddr++ != (unsigned short int)i) 
	{ print_dbg("\r\nTesting SRAM... FAILED! "); print_dbg_ulong(i); print_dbg_char(':'); print_dbg_ulong(*--sramAddr); print_dbg("\r\n"); return; }
		if((i % 0x400) == 0) print_dbg_char('?'); }
    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

void SRAM_Test_5M(void)
{
	volatile unsigned long int i = 0;
	volatile unsigned short int *sramAddr = (volatile unsigned short int*)0xD0500000;
    print_dbg("\n\rSRAM Test above 5M... \n");
    for (i = 0; i < 0x80000 ; i++) { *sramAddr++ = (unsigned short int)i; if((i % 0x400) == 0) print_dbg_char('+'); }
	sramAddr = (volatile unsigned short int*)0xD0500000;
    for (i = 0; i < 0x80000 ; i++) { if (*sramAddr++ != (unsigned short int)i) 
	{ print_dbg("\r\nTesting SRAM... FAILED! "); print_dbg_ulong(i); print_dbg_char(':'); print_dbg_ulong(*--sramAddr); print_dbg("\r\n"); return; }
		if((i % 0x400) == 0) print_dbg_char('?'); }
    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

void SRAM_Test_6M(void)
{
	volatile unsigned long int i = 0;
	volatile unsigned short int *sramAddr = (volatile unsigned short int*)0xD0600000;
    print_dbg("\n\rSRAM Test above 6M... \n");
    for (i = 0; i < 0x80000 ; i++) { *sramAddr++ = (unsigned short int)i; if((i % 0x400) == 0) print_dbg_char('+'); }
	sramAddr = (volatile unsigned short int*)0xD0600000;
    for (i = 0; i < 0x80000 ; i++) { if (*sramAddr++ != (unsigned short int)i) 
	{ print_dbg("\r\nTesting SRAM... FAILED! "); print_dbg_ulong(i); print_dbg_char(':'); print_dbg_ulong(*--sramAddr); print_dbg("\r\n"); return; }
		if((i % 0x400) == 0) print_dbg_char('?'); }
    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}

void SRAM_Test_7M(void)
{
	volatile unsigned long int i = 0;
	volatile unsigned short int *sramAddr = (volatile unsigned short int*)0xD0700000;
    print_dbg("\n\rSRAM Test above 7M... \n");
    for (i = 0; i < 0x80000 ; i++) { *sramAddr++ = (unsigned short int)i; if((i % 0x400) == 0) print_dbg_char('+'); }
	sramAddr = (volatile unsigned short int*)0xD0700000;
    for (i = 0; i < 0x80000 ; i++) { if (*sramAddr++ != (unsigned short int)i) 
	{ print_dbg("\r\nTesting SRAM... FAILED! "); print_dbg_ulong(i); print_dbg_char(':'); print_dbg_ulong(*--sramAddr); print_dbg("\r\n"); return; }
		if((i % 0x400) == 0) print_dbg_char('?'); }
    print_dbg("\rTesting SRAM..............PASSED!\n\r");
}




