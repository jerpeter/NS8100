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

#if 0
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
  g_smc_tab_cs_size[ncs] = EXT_SM_SIZE; \
  }

#if 0
static unsigned long int memPatter[] = 
{
  0xABABABAB, 0x77073096, 0xEE0E612C, 0x990951BA,
  0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
  0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
  0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
  0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
  0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
  0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
  0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
  0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
  0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,

  0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
  0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
  0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
  0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
  0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
  0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
  0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
  0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
  0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
  0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,

  0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
  0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
  0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
  0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
  0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
  0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
  0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
  0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
  0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
  0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,

  0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
  0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
  0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
  0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
  0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
  0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
  0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
  0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
  0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
  0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,

  0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
  0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
  0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
  0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
  0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
  0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
  0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
  0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
  0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
  0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,

  0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
  0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
  0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
  0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
  0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
  0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
  0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
  0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
  0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
  0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,

  0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
  0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
  0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
  0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
}
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
//void smc_enable_muxed_pins(void);
//static unsigned char g_smc_tab_cs_size[6];

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
//  return g_smc_tab_cs_size[cs];
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
   unsigned char  g_input_buffer[10];
   int reply;

   progress_inc = (sram_size + 50) / 100;

   print_dbg("Set fill pattern (");
   print_dbg_short_hex(0x1234);
   print_dbg("): ");
   reply = Get_User_Input(g_input_buffer);
   if( reply > 0)
   {
   	  if(reply == 1)
      {
   	     data_word = (g_input_buffer[0] - 0x30);
   	  }
   	  if(reply == 2)
      {
   	     data_word =  ((g_input_buffer[0] - 0x30) << 4);
   	     data_word += (g_input_buffer[1] - 0x30);
   	  }
   	  if(reply == 3)
      {
   	     data_word =  ((g_input_buffer[0] - 0x30) << 8);
   	     data_word += ((g_input_buffer[1] - 0x30) << 4);
   	     data_word += (g_input_buffer[2] - 0x30);
   	  }
   	  if(reply == 4)
      {
   	     data_word =  ((g_input_buffer[0] - 0x30) << 12);
   	     data_word += ((g_input_buffer[1] - 0x30) << 8);
   	     data_word += ((g_input_buffer[2] - 0x30) << 4);
   	     data_word += (g_input_buffer[3] - 0x30);
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
	print_dbg("Memory Test @ 0xD0100000 10K: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x10000; i++)
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
	print_dbg("Mem Test @ 0xD010 10K Off 20: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0x20000; i < 0x30000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0x20000; i < 0x30000; i++)
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

		if (breakCount > 10)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Mem Test @ 0xD010 10K Off 30: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0x30000; i < 0x40000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0x30000; i < 0x40000; i++)
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

		if (breakCount > 10)
			break;

		test32ptr++;
	}
	print_dbg("\n");

    //-------------------------------------------------------------
	print_dbg("\n");
	print_dbg("Mem Test @ 0xD010 10K Off 40: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0x40000; i < 0x50000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0x40000; i < 0x50000; i++)
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

		if (breakCount > 10)
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
	print_dbg("Mem Test @ 0xD010 40K Small : ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x40000; i++)
	{
		*test32ptr++ = (i % 2);
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x40000; i++)
	{
		if (*test32ptr != (i % 2))
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
	print_dbg("Mem Test 0xD010 30K by 10K  : ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i;
	}
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i;
	}
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x10000; i++)
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

	for (i = 0; i < 0x10000; i++)
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

	for (i = 0; i < 0x10000; i++)
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
	print_dbg("Mem Test 0xD010 30K by 10K I: ");
	test32ptr = (unsigned long int*)0xD0100000;
	breakCount = 0;
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i;
	}
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i + (unsigned long int)0x10000;
	}
	for (i = 0; i < 0x10000; i++)
	{
		*test32ptr++ = i + (unsigned long int)0x20000;
	}

	test32ptr = (unsigned long int*)0xD0100000;
	for (i = 0; i < 0x10000; i++)
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

	for (i = 0; i < 0x10000; i++)
	{
		if (*test32ptr != (i + (unsigned long int)0x10000))
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

	for (i = 0; i < 0x10000; i++)
	{
		if (*test32ptr != (i + (unsigned long int)0x20000))
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

void testMem8(unsigned char *startAddr, unsigned char *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int breakCount = 0;
	unsigned char *testPtr8 = (unsigned char*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 100;

    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 8-bit While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	while (1)
	{
		*testPtr8 = (unsigned char)i; 

		testPtr8++; 
		i += increment;

		if(testPtr8 == (unsigned char*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 
	testPtr8 = (unsigned char*)startAddr;
	
	while (1)
	{
		if(*testPtr8 != (unsigned char)i)
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr8);
			print_dbg(" Data: "); print_dbg_hex(*testPtr8);
			print_dbg(" i: "); print_dbg_hex(i);
			breakCount++;
		}
		else
		{
			if ((((unsigned long int)testPtr8) % printDiv) == 0)
				print_dbg(".");
		}

		testPtr8++; 
		i += increment;
		
		if (breakCount > breakCountEnd) break; 
		if(testPtr8 == (unsigned char*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem16(unsigned short int *startAddr, unsigned short int *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int breakCount = 0;
	unsigned short int *testPtr16 = (unsigned short int*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 100;

    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 16-bit While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");
	
    //-------------------------------------------------------------
	// Write
	while (1)
	{
		*testPtr16 = (unsigned short int)i; 

		testPtr16++; 
		i += increment;

		if(testPtr16 == (unsigned short int*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 
	testPtr16 = (unsigned short int*)startAddr;
	
	while (1)
	{
		if(*testPtr16 != (unsigned short int)i)
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr16);
			print_dbg(" Data: "); print_dbg_hex(*testPtr16);
			print_dbg(" i: "); print_dbg_hex(i);
			breakCount++;
		}
		else
		{
			if (((unsigned long int)testPtr16) % printDiv == 0)
				print_dbg(".");
		}

		testPtr16++; 
		i += increment;
		
		if (breakCount > breakCountEnd) break; 
		if(testPtr16 == (unsigned short int*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem16_Adjust(unsigned short int *startAddr, unsigned short int *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0, j = 0;
	unsigned long int breakCount = 0;
	unsigned short int *testPtr16 = (unsigned short int*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 100;

    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 16-bit Adjust While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");
	
    //-------------------------------------------------------------
	// Write
	while (1)
	{
		*testPtr16 = (unsigned short int)i; 

		testPtr16++; 
		i += (increment + j);

		if((unsigned short int)i == 0)
			j++;

		if(testPtr16 == (unsigned short int*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0;
	j = 0;
	testPtr16 = (unsigned short int*)startAddr;
	
	while (1)
	{
		if(*testPtr16 != (unsigned short int)i)
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr16);
			print_dbg(" Data: "); print_dbg_hex(*testPtr16);
			print_dbg(" i: "); print_dbg_hex(i);
			breakCount++;
		}
		else
		{
			if (((unsigned long int)testPtr16) % printDiv == 0)
				print_dbg(".");
		}

		testPtr16++; 
		i += (increment + j);
		
		if((unsigned short int)i == 0)
			j++;
		
		if (breakCount > breakCountEnd) break; 
		if(testPtr16 == (unsigned short int*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32(unsigned long int *startAddr, unsigned long int *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int breakCount = 0;
	unsigned long int *testPtr32 = (unsigned long int*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 50;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	while (1)
	{
		*testPtr32 = i; 

		testPtr32++; 
		i += increment;

		if(testPtr32 == (unsigned long int*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 
	testPtr32 = (unsigned long int*)startAddr;
	
	while (1)
	{
		if(*testPtr32 != i)
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr32);
			print_dbg(" Data: "); print_dbg_hex(*testPtr32);
			print_dbg(" i: "); print_dbg_hex(i);
			breakCount++;
		}
		else
		{
			if (((unsigned long int)testPtr32) % printDiv == 0)
				print_dbg(".");
		}
		
		testPtr32++;
		i += increment;
		
		if (breakCount > breakCountEnd) break; 
		if(testPtr32 == (unsigned long int*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_Block_Validate(unsigned long int *startAddr, unsigned long int *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0, incorrectCount = 0, correctCount = 0;
	unsigned long int *incorrectStartAddr, *correctStartAddr;
	unsigned long int breakCount = 0;
	unsigned long int *testPtr32 = (unsigned long int*)startAddr;
	//unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 50;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    //print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	while (1)
	{
		*testPtr32 = i; 

		testPtr32++; 
		i += increment;

		if(testPtr32 == (unsigned long int*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 
	testPtr32 = (unsigned long int*)startAddr;
	
	while (1)
	{
		if(*testPtr32 != i) // Data incorrect
		{
			if(correctCount != 0)
			{
				print_dbg("\nData -Correct- Addr: "); print_dbg_hex((unsigned long int)correctStartAddr);
				print_dbg(", # -Correct- bytes: "); print_dbg_hex((correctCount * 4));

				correctCount = 0;
			}

			if(incorrectCount == 0)
			{
				incorrectStartAddr = testPtr32;
			}			

			incorrectCount++;
		}
		else // Data correct
		{
			if(incorrectCount != 0)
			{
				print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)incorrectStartAddr);
				print_dbg(", # Incorrect bytes: "); print_dbg_hex((incorrectCount * 4));

				incorrectCount = 0;
			}

			if(correctCount == 0)
			{
				correctStartAddr = testPtr32;
			}

			correctCount++;
		}

		//if (((unsigned long int)testPtr32) % printDiv == 0)
		//	print_dbg(".");
		
		testPtr32++;
		i += increment;
		
		//if (breakCount > breakCountEnd) break; 
		if(testPtr32 == (unsigned long int*)endAddr) break;
	}

	if(correctCount != 0)
	{
		print_dbg("\nData -Correct- Addr: "); print_dbg_hex((unsigned long int)correctStartAddr);
		print_dbg(", # -Correct- bytes: "); print_dbg_hex((correctCount * 4));
	}

	if(incorrectCount != 0)
	{
		print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)incorrectStartAddr);
		print_dbg(", # Incorrect bytes: "); print_dbg_hex((incorrectCount * 4));
	}

	if (breakCount == 0)
		print_dbg("\nFinished.");

	print_dbg("\n");
}

void testMem8x32(unsigned char *startAddr, unsigned char *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0, temp = 0;
	unsigned long int breakCount = 0;
	unsigned char *testPtr8 = (unsigned char*)startAddr;
	unsigned long int *testPtr32 = (unsigned long int*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 100;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 8-bit Read 32 While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write 
	while (1)
	{
		*testPtr8 = (unsigned char)i; 

		testPtr8++; 
		i += increment;

		if(testPtr8 == (unsigned char*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 

	while (1)
	{
		temp = ((i << 24) & 0xFF000000);
		i += increment;
		temp |= ((i << 16) & 0x00FF0000);
		i += increment;
		temp |= ((i << 8) & 0x0000FF00);
		i += increment;
		temp |= (i & 0x000000FF);
		i += increment;

		// Check 32 of combined 16's
		if(*testPtr32 != temp)
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr32);
			print_dbg(" Data: "); print_dbg_hex(*testPtr32);
			print_dbg(" i: "); print_dbg_hex(temp);
			breakCount++;
		}
		else
		{
			if (((unsigned long int)testPtr32) % printDiv == 0)
				print_dbg(".");
		}
		
		testPtr32++; 
		
		if (breakCount > breakCountEnd) break; 
		if(testPtr32 == (unsigned long int*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem16x32(unsigned short int *startAddr, unsigned short int *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0, temp = 0;
	unsigned long int breakCount = 0;
	unsigned short int *testPtr16 = (unsigned short int*)startAddr;
	unsigned long int *testPtr32 = (unsigned long int*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 100;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 16-bit Read 32 While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write 
	while (1)
	{
		*testPtr16 = (unsigned short int)i; 

		testPtr16++; 
		i += increment;

		if(testPtr16 == (unsigned short int*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 

	while (1)
	{
		temp = (i << 16);
		i += increment;
		temp |= (i & 0x0000FFFF);
		i += increment;

		// Check 32 of combined 16's
		if(*testPtr32 != temp)
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr32);
			print_dbg(" Data: "); print_dbg_hex(*testPtr32);
			print_dbg(" i: "); print_dbg_hex(temp);
			breakCount++;
		}
		else
		{
			if (((unsigned long int)testPtr32) % printDiv == 0)
				print_dbg(".");
		}
		
		testPtr32++; 
		
		if (breakCount > breakCountEnd) break; 
		if(testPtr32 == (unsigned long int*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32x16(unsigned long int *startAddr, unsigned long int *endAddr, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int breakCount = 0;
	unsigned long int *testPtr32 = (unsigned long int*)startAddr;
	unsigned short int *testPtr16 = (unsigned short int*)startAddr;
	unsigned long int printDiv = ((unsigned long int)endAddr - (unsigned long int)startAddr) / 100;

    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit Read 16 While Loop Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", End: "); print_dbg_hex((unsigned long int)endAddr);
    print_dbg(", Increment: "); print_dbg_ulong(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");
	
    //-------------------------------------------------------------
	// Write
	while (1)
	{
		*testPtr32 = i; 

		testPtr32++; 
		i += increment;

		if(testPtr32 == (unsigned long int*)endAddr)
			break;
	}
	
    //-------------------------------------------------------------
	// Verify 
	i = 0; 

	while (1)
	{
		// Check Upper 16
		if(*testPtr16 != (unsigned short int)(i >> 16))
		{
			print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)testPtr16);
			print_dbg(" Data: "); print_dbg_hex(*testPtr16);
			print_dbg(" i: "); print_dbg_hex(i >> 16);
			testPtr16++;
			print_dbg(", Addr: "); print_dbg_hex((unsigned long int)testPtr16);
			print_dbg(" Data: "); print_dbg_hex(*testPtr16);
			print_dbg(" i: "); print_dbg_hex((unsigned short int)i);
			breakCount++;
		}
		else // Check Lower 16
		{
			testPtr16++;

			if(*testPtr16 != (unsigned short int)i)
			{
				print_dbg("\nData mismatch! Addr: "); print_dbg_hex((unsigned long int)(testPtr16 - 1));
				print_dbg(" Data: "); print_dbg_hex(*(testPtr16 - 1));
				print_dbg(" i: "); print_dbg_hex(i >> 16);

				print_dbg(", Addr: "); print_dbg_hex((unsigned long int)testPtr16);
				print_dbg(" Data: "); print_dbg_hex(*testPtr16);
				print_dbg(" i: "); print_dbg_hex((unsigned short int)i);
				breakCount++;
			}
			else // Both Upper and Lower ok
			{
				if (((unsigned long int)testPtr16) % printDiv == 0)
					print_dbg(".");
			}
		}		

		testPtr16++;
		i += increment;

		if (breakCount > breakCountEnd) break; 
		if(testPtr16 == (unsigned short int*)endAddr) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_Filter(unsigned long int *startAddr, unsigned long int index, unsigned long int filter, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int *testPtr = (unsigned long int*)startAddr;
	unsigned long int breakCount = 0;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit Filter Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", Count: "); print_dbg_hex(index);
    print_dbg(", Filter: "); print_dbg_hex(filter);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	for (i = 0; i < index; i++)
	{
		*testPtr++ = (i & filter);
	}
	
    //-------------------------------------------------------------
	// Verify 
	testPtr = (unsigned long int*)startAddr;
	for (i = 0; i < index; i++)
	{
		if (*testPtr != (i & filter))
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)testPtr);
			print_dbg(" Data: ");
			print_dbg_hex(*testPtr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			breakCount++;
		}
		else 
		{ 
			if (i % (index / 100) == 0) 
				print_dbg(".");
		}

		if (breakCount > breakCountEnd) break;
		testPtr++;
	}
	
	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_Const_Addr(unsigned long int *startAddr, unsigned long int index, unsigned long int constValue, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int *testPtr = (unsigned long int*)startAddr;
	unsigned long int breakCount = 0;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit Const Val Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", Count: "); print_dbg_hex(index);
    print_dbg(", Const Val: "); print_dbg_hex(constValue);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	for (i = 0; i < index; i++)
	{
		*testPtr = i;
	}

	constValue = --i;	
	*((unsigned long int*)0x00000000) = (unsigned long int)0xFFFFFFFF;

    //-------------------------------------------------------------
	// Verify 
	for (i = 0; i < index; i++)
	{
		if (*testPtr != constValue)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)testPtr);
			print_dbg(" Data: ");
			print_dbg_hex(*testPtr);
			print_dbg(" i: ");
			print_dbg_hex(constValue);
			breakCount++;
		}
		else 
		{ 
			if (i % (index / 100) == 0) 
				print_dbg(".");
		}

		if (breakCount > breakCountEnd) break;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_Const(unsigned long int *startAddr, unsigned long int index, unsigned long int constValue, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int *testPtr = (unsigned long int*)startAddr;
	unsigned long int breakCount = 0;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit Const Val Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", Count: "); print_dbg_hex(index);
    print_dbg(", Const Val: "); print_dbg_hex(constValue);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	for (i = 0; i < index; i++)
	{
		*testPtr++ = constValue;
	}
	
    //-------------------------------------------------------------
	// Verify 
	testPtr = (unsigned long int*)startAddr;
	for (i = 0; i < index; i++)
	{
		if (*testPtr != constValue)
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)testPtr);
			print_dbg(" Data: ");
			print_dbg_hex(*testPtr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			breakCount++;
		}
		else 
		{ 
			if (i % (index / 100) == 0) 
				print_dbg(".");
		}

		if (breakCount > breakCountEnd) break;
		testPtr++;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_Const_Upper(unsigned long int *startAddr, unsigned long int index, unsigned long int constValue, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int *testPtr = (unsigned long int*)startAddr;
	unsigned long int breakCount = 0;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit Const Val Upper Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", Count: "); print_dbg_hex(index);
    print_dbg(", Const Val: "); print_dbg_hex(constValue);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	for (i = 0; i < index; i++)
	{
		*testPtr++ = (constValue << 16) | (unsigned short int)i;
	}
	
    //-------------------------------------------------------------
	// Verify 
	testPtr = (unsigned long int*)startAddr;
	for (i = 0; i < index; i++)
	{
		if (*testPtr != ((constValue << 16) | (unsigned short int)i))
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)testPtr);
			print_dbg(" Data: ");
			print_dbg_hex(*testPtr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			breakCount++;
		}
		else 
		{ 
			if (i % (index / 100) == 0) 
				print_dbg(".");
		}

		if (breakCount > breakCountEnd) break;
		testPtr++;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_Const_Lower(unsigned long int *startAddr, unsigned long int index, unsigned long int constValue, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int *testPtr = (unsigned long int*)startAddr;
	unsigned long int breakCount = 0;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit Const Val Lower Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", Count: "); print_dbg_hex(index);
    print_dbg(", Const Val: "); print_dbg_hex(constValue);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	for (i = 0; i < index; i++)
	{
		*testPtr++ = (constValue | (((unsigned short int)i) << 16));
	}
	
    //-------------------------------------------------------------
	// Verify 
	testPtr = (unsigned long int*)startAddr;
	for (i = 0; i < index; i++)
	{
		if (*testPtr != (constValue | (((unsigned short int)i) << 16)))
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)testPtr);
			print_dbg(" Data: ");
			print_dbg_hex(*testPtr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			breakCount++;
		}
		else 
		{ 
			if (i % (index / 100) == 0) 
				print_dbg(".");
		}

		if (breakCount > breakCountEnd) break;
		testPtr++;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void testMem32_2x_Region(unsigned long int *startAddr, unsigned long int index, unsigned long int increment, unsigned long int breakCountEnd)
{
	unsigned long int i = 0;
	unsigned long int *testPtr = (unsigned long int*)startAddr;
	unsigned long int breakCount = 0;
	
    //-------------------------------------------------------------
    print_dbg("\n\rSRAM 32-bit 2x Region Test, Start: "); print_dbg_hex((unsigned long int)startAddr);
    print_dbg(", Count: "); print_dbg_hex(index);
    print_dbg(", Increment: "); print_dbg_hex(increment);
    print_dbg(", Break #: "); print_dbg_ulong(breakCountEnd);
    print_dbg("...\n");

    //-------------------------------------------------------------
	// Write
	for (i = 0; i < index; i++)
	{
		*testPtr++ = i + increment;
	}

    //-------------------------------------------------------------
	// Verify 
	testPtr = (unsigned long int*)((unsigned long int)startAddr + 0x100000);
	for (i = 0; i < index; i++)
	{
		*testPtr++ = i + increment * 2;
	}
	
	testPtr = (unsigned long int*)startAddr;
	for (i = 0; i < index; i++)
	{
		if (*testPtr != (i + increment))
		{
			print_dbg("\nData mismatch! Addr: ");
			print_dbg_hex((unsigned long int)testPtr);
			print_dbg(" Data: ");
			print_dbg_hex(*testPtr);
			print_dbg(" i: ");
			print_dbg_hex(i);
			breakCount++;
		}
		else 
		{ 
			if (i % (index / 100) == 0) 
				print_dbg(".");
		}

		if (breakCount > breakCountEnd) break;
		testPtr++;
	}

	if (breakCount == 0)
		print_dbg("Passed.");

	print_dbg("\n");
}

void SRAM_Test_32(void)
{
	testMem8((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 1, 10);
	testMem8((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 2, 10);
	testMem8((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 4, 10);
	testMem8((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 8, 10);

	testMem16((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 1, 10);
	testMem16((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 2, 10);
	testMem16((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 4, 10);
	testMem16((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 8, 10);

	testMem16_Adjust((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 1, 10);
	testMem16_Adjust((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 2, 10);
	testMem16_Adjust((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 4, 10);
	testMem16_Adjust((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 8, 10);

	testMem32((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 1, 10);
	testMem32((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 2, 10);
	testMem32((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 4, 10);
	testMem32((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 8, 10);

#if 0
	testMem32_Const_Addr((unsigned long int*)0xD0000000, 0x1, 0x12345678, 10);
	testMem32_Const_Addr((unsigned long int*)0xD0000000, 0x10, 0x12345678, 10);
	testMem32_Const_Addr((unsigned long int*)0xD0000000, 0x100, 0x12345678, 10);
	testMem32_Const_Addr((unsigned long int*)0xD0000000, 0x1000, 0x12345678, 10);
	testMem32_Const_Addr((unsigned long int*)0xD0000000, 0x10000, 0x12345678, 10);
	testMem32_Const_Addr((unsigned long int*)0xD0000000, 0x100000, 0x12345678, 10);

	testMem32_Const((unsigned long int*)0xD0000000, 0x200000, 0x12345678, 10);
	testMem32_Const((unsigned long int*)0xD0000000, 0x200000, 0x87654321, 10);
	testMem32_Const((unsigned long int*)0xD0000000, 0x200000, 0x43215678, 10);
	testMem32_Const((unsigned long int*)0xD0000000, 0x200000, 0x43218765, 10);

	testMem32_Const_Upper((unsigned long int*)0xD0000000, 0x200000, 0x1234, 10);
	testMem32_Const_Upper((unsigned long int*)0xD0000000, 0x200000, 0x5678, 10);
	testMem32_Const_Upper((unsigned long int*)0xD0000000, 0x200000, 0x00FF, 10);
	testMem32_Const_Upper((unsigned long int*)0xD0000000, 0x200000, 0xFFFF, 10);

	testMem32_Const_Lower((unsigned long int*)0xD0000000, 0x200000, 0x1234, 10);
	testMem32_Const_Lower((unsigned long int*)0xD0000000, 0x200000, 0x5678, 10);
	testMem32_Const_Lower((unsigned long int*)0xD0000000, 0x200000, 0x00FF, 10);
	testMem32_Const_Lower((unsigned long int*)0xD0000000, 0x200000, 0xFFFF, 10);
#endif

	testMem8x32((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 1, 10);
	testMem8x32((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 2, 10);
	testMem8x32((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 4, 10);
	testMem8x32((unsigned char*)0xD0000000, (unsigned char*)0xD0800000, 8, 10);

	testMem16x32((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 1, 10);
	testMem16x32((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 2, 10);
	testMem16x32((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 4, 10);
	testMem16x32((unsigned short int*)0xD0000000, (unsigned short int*)0xD0800000, 8, 10);

	testMem32x16((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 1, 10);
	testMem32x16((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 2, 10);
	testMem32x16((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 4, 10);
	testMem32x16((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 8, 10);

	testMem32_Block_Validate((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 1, 0x1000);
	testMem32_Block_Validate((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 2, 0x1000);
	testMem32_Block_Validate((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 4, 0x1000);
	testMem32_Block_Validate((unsigned long int*)0xD0000000, (unsigned long int*)0xD0800000, 8, 0x1000);

	testMem32((unsigned long int*)0xD0000000, (unsigned long int*)0xD0020000, 1, 10);
	testMem32((unsigned long int*)0xD0010000, (unsigned long int*)0xD0030000, 2, 10);
	testMem32((unsigned long int*)0xD0020000, (unsigned long int*)0xD0040000, 4, 10);
	testMem32((unsigned long int*)0xD0030000, (unsigned long int*)0xD0050000, 8, 10);
	testMem32((unsigned long int*)0xD0000000, (unsigned long int*)0xD0020000, 1, 10);
	testMem32((unsigned long int*)0xD0010000, (unsigned long int*)0xD0030000, 2, 10);
	testMem32((unsigned long int*)0xD0020000, (unsigned long int*)0xD0040000, 4, 10);
	testMem32((unsigned long int*)0xD0030000, (unsigned long int*)0xD0050000, 8, 10);
	
	print_dbg("Finished Testing.\nDone.\n");
}

#if 0
	*((unsigned long int*)0xD0100000) = 0x12345678;
	print_dbg("\nData @ 0xD0100000: "); print_dbg_hex(*((unsigned long int*)0xD0100000));
	print_dbg(" @0xD0120000: "); print_dbg_hex(*((unsigned long int*)0xD0120000));
	*((unsigned long int*)0xD0120000) = 0x87654321;
	print_dbg("\nData @ 0xD0100000: "); print_dbg_hex(*((unsigned long int*)0xD0100000));
	print_dbg(" @0xD0120000: "); print_dbg_hex(*((unsigned long int*)0xD0120000));
	print_dbg("\n");
#endif

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
#endif
