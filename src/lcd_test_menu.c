////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   lcd_test_menu.c                                                          //
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
#include "lcd_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "lcd.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
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

unsigned char g_smc_tab_cs_size[6];

static int contrast_level = 0;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 0
void smc_enable_muxed_pins(void);

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

static void smc_enable_muxed_pins(void)
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

static void Write_Block_To_Display (unsigned char *g_mmap_ptr)
{
    unsigned char segment;
    unsigned char *pixel_byte_ptr;
    unsigned char col_index;
    unsigned char row_index;
    unsigned char data;

	row_index = 0;
    while(row_index < 8)
	{
	    col_index = 0;
	    pixel_byte_ptr = (unsigned char *)(g_mmap_ptr + (row_index * 128));

        segment = FIRST_HALF_DISPLAY;
        Write_display(COMMAND_REGISTER, PAGE_ADDRESS_SET + row_index, segment);
        Write_display(COMMAND_REGISTER, COLUMN_ADDRESS_SET + col_index, segment);

        while(col_index<128)
        {
            if(segment == FIRST_HALF_DISPLAY)
            {
                if( (col_index) > 63) /*NOTE: BORDER is 64 its zero based 0 - 63 */
                {
                    segment = SECOND_HALF_DISPLAY;
                    Write_display(COMMAND_REGISTER, PAGE_ADDRESS_SET + row_index, segment);
                    Write_display(COMMAND_REGISTER, COLUMN_ADDRESS_SET + (col_index - 64), segment);
                }
            }
            else
            {
                if( (col_index) > 128) /* NOTE SEG2 BORDER IS 128 0 - 127 */
                {
                    break; /* NO WORD WRAP */
                }
            }
            data = *(pixel_byte_ptr + col_index);
            Write_display( DATA_REGISTER, *(pixel_byte_ptr + col_index), segment);
           col_index++;
        }/* End Of while((col_index<SEGMENT_TWO_BLOCK_BORDER) */

        row_index++;
    }/*End Of while(row_index < 8) */
}


void LCD_Test_Menu(void)
{
   // Initialize sram driver resources.
   //smc_init(FOSC0);
   
   //Turn on display
   gpio_set_gpio_pin(AVR32_PIN_PB21);

   Backlight_On();
   Backlight_High();
   Set_Contrast(24);
   InitDisplay();
   Menu_Items = (sizeof(LCD_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)LCD_Test_Menu_Functions;
   Menu_String = (unsigned char *)LCD_Test_Menu_Text;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  LCD_Exit                                                            //
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
void LCD_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void LCD_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = LCD_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = LCD_Info_Text[count];
		}
}

void LCD_Write_Character(void)
{
   unsigned char g_input_buffer[10];
   int reply;

   print_dbg("Character to write ( ):");
   reply = Get_User_Input(g_input_buffer);
   if( reply > 0)
   {
   	  g_input_buffer[reply] = 0x00;
   }
   else
   {
   	  g_input_buffer[0] = 0x00;
   }
   WriteLCD_smText( 4, 4, g_input_buffer, NORMAL_LCD);
   print_dbg("\n\rCharacter [");
   print_dbg_char(g_input_buffer[0]);
   print_dbg("] written to LCD\n\r");
}

void LCD_Write_String(void)
{
   unsigned char g_input_buffer[40];
   int reply;

   print_dbg("String to write ( ):");
   reply = Get_User_Input(g_input_buffer);
   if( reply > 0)
   {
   	  g_input_buffer[reply] = 0x00;
   }
   else
   {
   	  g_input_buffer[0] = 0x00;
   }
   WriteLCD_smText( 0, 1, g_input_buffer, NORMAL_LCD);
   print_dbg("\n\rString [");
   print_dbg((char*)g_input_buffer);
   print_dbg("] written to LCD\n\r");

}
void LCD_Draw_Vertical_Line(void)
{
    WriteLCD_Vline( 64, 63, 0, LINECOLOR_BLACK);
}

void LCD_Draw_Horizontal_Line(void)
{
    WriteLCD_Hline( 32, 0, 127, LINECOLOR_BLACK);
}

void LCD_Power_On_Off(void)
{
    static uint8 power_state = 0;

    if(power_state == 0)
    {
   	   //turn on power
 	   gpio_set_gpio_pin(AVR32_PIN_PB21);
       power_state = 1;
       Set_Contrast(24);
       InitDisplay();
       print_dbg("Display power = on.\n\r");
    }
    else
    {
  	   //turn off power
   	   gpio_clr_gpio_pin(AVR32_PIN_PB21);
       power_state = 0;
       print_dbg("Display power = off.\n\r");
    }
}

void LCD_Backlight_On_Off_Low_High(void)
{
    static uint8 light_state = 0;

    if(light_state == 0)
    {
   	   //turn off backlight
       Backlight_Off();
       light_state = 1;
       print_dbg("Display backlight = off.\n\r");
    }
    else if(light_state == 1)
    {
   	   //turn on low backlight
       Backlight_Low();
       Backlight_On();
       light_state = 2;
       print_dbg("Display backlight = low.\n\r");
    }
    else
    {
   	   //turn on high backlight
       Backlight_High();
       Backlight_On();
       light_state = 0;
       print_dbg("Display backlight = high.\n\r");
    }
}

void LCD_Clear(void)
{
    // now clear the display
    ClearLCDscreen();
}
void LCD_Set_Contrast(void)
{
   unsigned char g_input_buffer[40];
   int reply;

   print_dbg("Contrast level (0 to 32): ");
   reply = Get_User_Input(g_input_buffer);
   if( reply > 0)
   {
	   contrast_level = atoi((char*)g_input_buffer);
	   Set_Contrast(contrast_level);
	   print_dbg("\n\rContrast level [");
	   print_dbg_ulong(contrast_level);
	   print_dbg("] set.\n\r");
   }
   else
   {
	   print_dbg("\n\rContrast level not set.\n\r");
   }

}
void LCD_Read_Contrast(void)
{
   print_dbg("\n\rContrast level = ");
   print_dbg_ulong(contrast_level);
   print_dbg(".\n\r");
}

void LCD_Test_Logo(void)
{
    Write_display(COMMAND_REGISTER, START_LINE_SET, FIRST_HALF_DISPLAY);
    Write_display(COMMAND_REGISTER, PAGE_ADDRESS_SET, FIRST_HALF_DISPLAY);
    Write_display(COMMAND_REGISTER, COLUMN_ADDRESS_SET, FIRST_HALF_DISPLAY);

    Write_Block_To_Display((unsigned char*)sign_on_logo);
}
#endif
