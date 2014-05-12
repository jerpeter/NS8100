/********************************************************
 Name          : main.c
 Author        : Joseph Getz
 Copyright     : Your copyright notice
 Description   : EVK1100 template
 **********************************************************/

// Include Files
#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "sdramc.h"
#include "intc.h"
#include "utils.h"
#include "usart.h"
#include "print_funcs.h"
#include "craft.h"
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
#include "Mmc2114.h"
#include "Mmc2114_InitVals.h"
#include "Old_Board.h"
#include "Common.h"
#include "Display.h"
#include "Menu.h"
#include "Uart.h"
#include "Ispi.h"
#include "spi.h"
#include "ProcessBargraph.h"
#include "ProcessCombo.h"
#include "SysEvents.h"
#include "Rec.h"
#include "InitDataBuffers.h"
#include "PowerManagement.h"
#include "SoftTimer.h"
#include "Keypad.h"
#include "RealTimeClock.h"
#include "RemoteHandler.h"
#include "TextTypes.h"
#include "RemoteCommon.h"
#include "ad_test_menu.h"
#include "rtc_test_menu.h"
#include "usb_test_menu.h"
#include "sd_mmc_test_menu.h"
#include "eeprom_test_menu.h"
#include "twi.h"
#include "M23018.h"
#include "sd_mmc_spi.h"
#include "FAT32_Disk.h"
#include "FAT32_Access.h"
#include "adc.h"

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
extern void initProcessor(void);
extern SUMMARY_DATA *results_summtable_ptr;
extern int8 g_kpadCheckForKeyFlag;				// Flag to indicate key stroke in the ISR
extern uint32 g_kpadLookForKeyTickCount;		// Timer to count number of ticks for a key stroke.
extern volatile uint32 g_keypadTimerTicks;		//
extern POWER_MANAGEMENT_STRUCT powerManagement;	// Struct containing power management parameters
extern REC_HELP_MN_STRUCT help_rec;				// Struct containing system parameters
extern uint32 g_rtcSoftTimerTickCount;			// A global counter to count system ticks
extern uint8 g_factorySetupSequence;			//
extern uint32 isTriggered;
extern uint8 mmap[LCD_NUM_OF_ROWS][LCD_NUM_OF_BIT_COLUMNS];
extern uint8 g_autoDialoutState;
extern void flashc_set_wait_state(unsigned int);
extern void Setup_Soft_Timer_Tick_ISR(void);
extern void Setup_EIC_Keypad_ISR(void);
extern void Setup_EIC_System_ISR(void);
extern int rtc_init(volatile avr32_rtc_t *rtc, unsigned char osc_type, unsigned char psel);
extern void rtc_set_top_value(volatile avr32_rtc_t *rtc, unsigned long top);
extern void rtc_enable_interrupt(volatile avr32_rtc_t *rtc);
extern void rtc_enable(volatile avr32_rtc_t *rtc);

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define NRD_SETUP     30 //10
#define NRD_PULSE     30 //135
#define NRD_CYCLE     75 //180

#define NCS_RD_SETUP  0 //10
#define NCS_RD_PULSE  55 //250

#define NWE_SETUP     0 //20
#define NWE_PULSE     40 //110
#define NWE_CYCLE     55 //150

#define NCS_WR_SETUP  0 //20
#define NCS_WR_PULSE  45 //230

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

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
unsigned char smc_tab_cs_size[4];

// Sensor information and constants.
SENSOR_PARAMETERS_STRUCT g_SensorInfoStruct;
SENSOR_PARAMETERS_STRUCT* gp_SensorInfo = &g_SensorInfoStruct;

// Contains the event record in ram.
EVT_RECORD g_RamEventRecord;

// Factory Setup record.
FACTORY_SETUP_STRUCT factory_setup_rec;

// Structure to contain system paramters and system settings.
REC_EVENT_MN_STRUCT trig_rec;				// Contains trigger specific information.

// Menu specific structures
int active_menu;							// For active menu number/enum.
MN_EVENT_STRUCT mn_event_flags;				// Menu event flags, for main loop processing.
MN_TIMER_STRUCT mn_timer;					// Menu timer strucutre.

// System Event Flags, for main loopo processing.
SYS_EVENT_STRUCT SysEvents_flags;

MODEM_SETUP_STRUCT modem_setup_rec;			// Record for user input data.
MODEM_STATUS_STRUCT g_ModemStatus;			// Record for modem data processing.

// Used as a circular buffer to continually caputer incomming data from the craft port.
CMD_BUFFER_STRUCT isrMessageBufferStruct;
CMD_BUFFER_STRUCT* gp_ISRMessageBuffer = &isrMessageBufferStruct;

void (*menufunc_ptrs[TOTAL_NUMBER_OF_MENUS]) (INPUT_MSG_STRUCT) = {
		mainMn, loadRecMn, summaryMn, monitorMn, resultsMn,
		overWriteMn, batteryMn, dateTimeMn, lcdContrastMn, timerModeTimeMn,
		timerModeDateMn, calSetupMn, userMn, monitorLogMn
};

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void InitSystemHardware(void);
void InitInterrupts(void);
void InitSoftwareSettings(void);
void SystemEventManager(void);
void MenuEventManager(void);
void CraftManager(void);
void MessageManager(void);
void FactorySetupManager(void);

#define PBA_HZ         FOSC0

extern const char craft_logo[];

//=============================================================================
//
//=============================================================================
static void Write_Block_To_Display (unsigned char *mmap_ptr)
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
	    pixel_byte_ptr = (unsigned char *)(mmap_ptr + (row_index * 128));

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

extern uint8 mmap[8][128];
void Display_Craft_Logo()
{
	Write_display(COMMAND_REGISTER, START_LINE_SET, FIRST_HALF_DISPLAY);
	Write_display(COMMAND_REGISTER, PAGE_ADDRESS_SET, FIRST_HALF_DISPLAY);
	Write_display(COMMAND_REGISTER, COLUMN_ADDRESS_SET, FIRST_HALF_DISPLAY);
	Write_Block_To_Display((unsigned char*)craft_logo);
}

//=============================================================================
//
//=============================================================================
void soft_delay(volatile unsigned long int counter)
{
	unsigned long int countdown = (counter << 2) + counter;
	
	for(; countdown > 0;)
	{
		countdown--;
	}
}

//=============================================================================
// SPI_init
//=============================================================================
void SPI_init(void)
{
	// SPI 1 MAP Pin select	
	const gpio_map_t spi1Map =
	{
		{AVR32_SPI1_SCK_0_0_PIN,  AVR32_SPI1_SCK_0_0_FUNCTION },	// SPI Clock.
		{AVR32_SPI1_MISO_0_0_PIN, AVR32_SPI1_MISO_0_0_FUNCTION},	// MISO.
		{AVR32_SPI1_MOSI_0_0_PIN, AVR32_SPI1_MOSI_0_0_FUNCTION},	// MOSI.
		{AD_CTL_SPI_NPCS_PIN, AD_CTL_SPI_NPCS_FUNCTION},			// AD Chip Select NPCS.
		{EEPROM_SPI_NPCS_PIN, EEPROM_SPI_NPCS_FUNCTION},			// EEprom Chip Select NPCS.
		{RTC_SPI_NPCS_PIN, RTC_SPI_NPCS_FUNCTION},					// RTC Chip Select NPCS.
		{SDMMC_SPI_NPCS_PIN, SDMMC_SPI_NPCS_FUNCTION},				// SCMMC Chip Select NPCS.
	};

	// Generic SPI options
	spi_options_t spiOptions =
	{
		.bits         = 8,
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI.
	gpio_enable_module(spi1Map, sizeof(spi1Map) / sizeof(spi1Map[0]));

	// Initialize as master.
	spi_initMaster(&AVR32_SPI1, &spiOptions);

	// Set SPI selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(&AVR32_SPI1, 0, 0, 0);

	// Enable SPI module.
	spi_enable(&AVR32_SPI1);

	spiOptions.reg = AD_CTL_SPI_NPCS; // 3
	spiOptions.baudrate = AD_CTL_SPI_MASTER_SPEED; // 4 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);

	spiOptions.reg = EEPROM_SPI_NPCS; // 0
	spiOptions.baudrate = EEPROM_SPI_MASTER_SPEED; // 2.1 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);

	spiOptions.reg = RTC_SPI_NPCS; // 1
	spiOptions.baudrate = RTC_SPI_MASTER_SPEED; // 1 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);

	spiOptions.reg = SDMMC_SPI_NPCS; // 2
	spiOptions.baudrate = SDMMC_SPI_MASTER_SPEED; // 12 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);
}

//=================================================================================================
void avr32_enable_muxed_pins(void)
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
		
		// Voltage monitor pins
		{AVR32_ADC_AD_2_PIN, AVR32_ADC_AD_2_FUNCTION},
		{AVR32_ADC_AD_3_PIN, AVR32_ADC_AD_3_FUNCTION}
	};

	gpio_enable_module(SMC_EBI_GPIO_MAP, sizeof(SMC_EBI_GPIO_MAP) / sizeof(SMC_EBI_GPIO_MAP[0]));
}
//=================================================================================================

//=================================================================================================
void avr32_chip_select_init(unsigned long hsb_hz)
{
  unsigned long int hsb_mhz_up = (hsb_hz + 999999) / 1000000;

  // Setup all 4 chip selects
  SMC_CS_SETUP(0) // LCD
  SMC_CS_SETUP(1) // External RAM
  SMC_CS_SETUP(2) // Network/LAN
  SMC_CS_SETUP(3) // Network/LAN Manager

  // Put the multiplexed MCU pins used for the SM under control of the SMC.
  avr32_enable_muxed_pins();
}
//=================================================================================================

//=================================================================================================
void _init_startup(void)   
{   
    // Switch the main clock to the external oscillator 0
    pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	// Logic to change the clock source to PLL 0
    // PLL = 0, Multiplier = 10 (actual 11), Divider = 1 (actually 1), OSC = 0, 16 clocks to stabilize
	pm_pll_setup(&AVR32_PM, 0, 10, 1, 0, 16);
    pm_pll_set_option(&AVR32_PM, 0, 1, 1, 0); // PLL = 0, Freq = 1, Div2 = 1

	// Enable and lock
    pm_pll_enable(&AVR32_PM, 0);
    pm_wait_for_pll0_locked(&AVR32_PM);

    pm_gc_setup(&AVR32_PM, 0, 1, 0, 0, 0);
    pm_gc_enable(&AVR32_PM, 0);

    gpio_enable_module_pin(AVR32_PM_GCLK_0_1_PIN, AVR32_PM_GCLK_0_1_FUNCTION);
    pm_cksel(&AVR32_PM, 0, 0, 0, 0, 0, 0);
    flashc_set_wait_state(1);
    pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCSEL_PLL0);

	// Chip Select Initialization
	avr32_chip_select_init(FOSC0);
	
	soft_delay(1000);
}
//=================================================================================================
void InitKeypad(void)

{
	static const gpio_map_t EIC_GPIO_MAP =
	{
		{AVR32_EIC_EXTINT_5_PIN, AVR32_EIC_EXTINT_5_FUNCTION}
	};

	// GPIO pins used for TWI interface
	static const gpio_map_t TWI_GPIO_MAP =
	{
		{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
		{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}
	};

	// Enable External Interrupt for MCP23018
	gpio_enable_module(EIC_GPIO_MAP, sizeof(EIC_GPIO_MAP) / sizeof(EIC_GPIO_MAP[0]));

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

	soft_usecWait(250 * SOFT_MSECS);

	init_mcp23018(IO_ADDRESS_KPD);

	soft_usecWait(250 * SOFT_MSECS);
}

//-----------------------------------------------------------------------------
unsigned char input_buffer[120];
void craftTestMenuThruDebug(void)
{
	int count;
    int character;
    int reply=0;

	if(usart_test_hit(&AVR32_USART1))
	{
		if(Get_User_Input(input_buffer) > 0)
		{
        	reply = atoi((char *)input_buffer);
		}

		if(reply < Menu_Items)
		{
        	((void(*)())*(Menu_Functions + reply))();
		}

    	count = 0;
    	character = Menu_String[count];
    	while(character != 0)
    	{
    		print_dbg_char(character);
    		count++;
    		character = Menu_String[count];
    	}
		Command_Prompt();
	}
}

//-----------------------------------------------------------------------------
volatile uint32 tempTick;
void testKeypad(void)
{
	uint8 keyScan;

	//InitKeypad();

	//print_dbg("\r\nAttempting to read the Keypad... ");
	keyScan = read_mcp23018(IO_ADDRESS_KPD, GPIOB);
	if(keyScan)
	{
		debug("Found character: %c", keyScan);
			
		keypad();

		tempTick = g_rtcSoftTimerTickCount;
		while(tempTick == g_rtcSoftTimerTickCount) {}
	}
}

//=================================================================================================
//	Function:	InitSystemHardware_NS8100
//=================================================================================================
void InitSystemHardware_NS8100(void)
{
#if 0 // Now handled in _init_startup
	// Logic to change the clock source to PLL 0
    // Switch the main clock to the external oscillator 0
    pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

    // PLL = 0, Multiplier = 10 (actual 11), Divider = 1 (actually 1), OSC = 0, 16 clocks to stabilize
	pm_pll_setup(&AVR32_PM, 0, 10, 1, 0, 16);
    pm_pll_set_option(&AVR32_PM, 0, 1, 1, 0); // PLL = 0, Freq = 1, Div2 = 1

	// Enable and lock
    pm_pll_enable(&AVR32_PM, 0);
    pm_wait_for_pll0_locked(&AVR32_PM);

    pm_gc_setup(&AVR32_PM, 0, 1, 0, 0, 0);
    pm_gc_enable(&AVR32_PM, 0);

    gpio_enable_module_pin(AVR32_PM_GCLK_0_1_PIN, AVR32_PM_GCLK_0_1_FUNCTION);
    pm_cksel(&AVR32_PM, 0, 0, 0, 0, 0, 0);
    flashc_set_wait_state(1);
    pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCSEL_PLL0);

    // Initialize all 4 Chip Selects, LCD, External SRAM, LAN and LAN Manager
    //smc_init(FOSC0);
#endif

	//-------------------------------------------------------------------------
	// Set RTC Timestamp pin high
	gpio_set_gpio_pin(AVR32_PIN_PB18);

	//-------------------------------------------------------------------------
	// Set LAN to Sleep
    gpio_clr_gpio_pin(AVR32_PIN_PB27);
	
	// Set LAN and LAN Mem CS High
	gpio_set_gpio_pin(AVR32_EBI_NCS_2_PIN);
	gpio_set_gpio_pin(AVR32_EBI_NCS_3_PIN);	

	//-------------------------------------------------------------------------
    // Turn on rs232 driver and receiver on NS8100 board ?
    gpio_clr_gpio_pin(AVR32_PIN_PB08);
    gpio_clr_gpio_pin(AVR32_PIN_PB09);

    // Setup debug serial port
    init_dbg_rs232(FOSC0);

	//-------------------------------------------------------------------------
    // Initialize USB clock.
    pm_configure_usb_clock();

	//-------------------------------------------------------------------------
	// Init the SPI interface
	SPI_init();

	//-------------------------------------------------------------------------
    // Turn on display
    gpio_set_gpio_pin(AVR32_PIN_PB21);

	// LCD Init
    Backlight_On();
    Backlight_High();
    Set_Contrast(24);
    InitDisplay();

	//-------------------------------------------------------------------------
	// Initialize the Real Time Counter
	if (!rtc_init(&AVR32_RTC, 1, 0))
	{
		debugErr("Error initializing the RTC\n");
		while(1);
	}

	// Set top value to 0 to generate an interrupt every second */
	rtc_set_top_value(&AVR32_RTC, 8192);

	// Enable the RTC
	rtc_enable(&AVR32_RTC);

	//-------------------------------------------------------------------------
	// Init Keypad
	InitKeypad();
	
	// Primer read
	uint8 keyScan = read_mcp23018(IO_ADDRESS_KPD, GPIOB);
	if (keyScan)
	{
		debugWarn("Keypad key being pressed, likely a bug. Key: %x", keyScan);
	}

	// Enable Processor A/D
	adc_configure(&AVR32_ADC);

	// Enable the A/D channels.
	adc_enable(&AVR32_ADC, VIN_CHANNEL);
	adc_enable(&AVR32_ADC, VBAT_CHANNEL);

	SD_MMC_Power_On();

	spi_selectChip(SDMMC_SPI, SD_MMC_SPI_NPCS);
	sd_mmc_spi_internal_init();
	spi_unselectChip(SDMMC_SPI, SD_MMC_SPI_NPCS);

    FAT32_InitDrive();
    if (FAT32_InitFAT() == FALSE)
    {
        debugErr("FAT32 Initialization failed!\n\r");
    }

	//-------------------------------------------------------------------------
	//Display_Craft_Logo();
	//soft_delay(3 * SOFT_SECS);
}

//=================================================================================================
//	Function:	InitInterrupts_NS8100
//=================================================================================================
void InitInterrupts_NS8100(void)
{
    Disable_global_interrupt();

    //Setup interrupt vectors
    INTC_init_interrupts();

	Setup_Soft_Timer_Tick_ISR();
	Setup_EIC_Keypad_ISR();
	Setup_EIC_System_ISR();

	Enable_global_interrupt();
}

//=================================================================================================
//	Function:	InitSoftwareSettings_NS8100
//=================================================================================================
void InitSoftwareSettings_NS8100(void)
{
	INPUT_MSG_STRUCT mn_msg;
	char buff[50];
	//uint16 i = 0;

	debug("Init Software Settings\n");

#if 0 // fix_ns8100
	GetParameterMemory((uint8*)&buff[0], 0, 50);
	for (i=0;i<50;i++)
	{
		debug("Parameter Mem at: 0x%x is: 0x%x\n", i, buff[i]);
	}
	for (i=0;i<50;i++)
	{
		buff[i] = (char)(50 - i);
	}
	SaveParameterMemory((uint8*)&buff[0], 0, 50);
	GetParameterMemory((uint8*)&buff[0], 0, 50);
	for (i=0;i<50;i++)
	{
		debug("Parameter Mem at: 0x%x is: 0x%x\n", i, buff[i]);
	}
#endif

    debug("Init Version Strings...\n");

	// Init version strings
	initVersionStrings();

    debug("Init Version Msg...\n");

	// Init version msg
	initVersionMsg();

    debug("Init Time Msg...\n");

	// Init time msg
	initTimeMsg();

	// Load current time into cache
	updateCurrentTime();

    debug("Init Get Boot Function Addr...\n");

	// Get the function address passed by the bootloader
	getBootFunctionAddress();

    debug("Init Setup Menu Defaults...\n");

	debug("Setup Menu Defaults\n");
	// Setup defaults, load records, init the language table
	setupMnDef();

    debug("Init Timer Mode Check...\n");

	debug("Check Timer Mode\n");
	// Check for Timer mode activation
	if (timerModeActiveCheck() == TRUE)
	{
		debug("--- Timer Mode Startup ---\n");
		processTimerMode();

		// If here, the unit is in Timer mode, but did not power itself off yet
		// Disable the Power Off key
		debug("Timer Mode: Disabling Power Off key\n");
		powerControl(POWER_SHUTDOWN_ENABLE, OFF);
	}
	else
	{
		debug("--- Normal Startup ---\n");
	}

    debug("Init Cmd Msg Handler...\n");

	debug("Cmd/Craft Init\n");
	// Init the cmd message handling buffers before initialization of the ports.
	cmdMessageHandlerInit();

    debug("Init Craft Init Status Flags...\n");

	// Init the input buffers and status flags for input craft data.
	craftInitStatusFlags();

    debug("Init LCD Message...\n");

	debug("LCD message\n");
	// Overlay a message to alert the user that the system is checking the sensors
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	overlayMessage(getLangText(STATUS_TEXT), buff, 0);

    debug("Init Jump to Main Menu...\n");

	debug("Jump to Main Menu\n");
	// Jump to the true main menu
	active_menu = MAIN_MENU;
	ACTIVATE_MENU_MSG();
	(*menufunc_ptrs[active_menu]) (mn_msg);
}

//=================================================================================================
//	Function:	Main
//	Purpose:	Application starting point
//=================================================================================================
extern void Setup_Data_Clock_ISR(uint32 sampleRate);
extern void Start_Data_Clock(void);
extern void AD_Init(void);
int main(void)
{
    InitSystemHardware_NS8100();
	InitInterrupts_NS8100();
	InitSoftwareSettings_NS8100();

#if 1 // fix_ns8100
	// Have to recall Keypad init otherwise interrupt hangs
	InitKeypad();
#endif

	//debug("Unit Type: %s\n", SUPERGRAPH_UNIT ? "Supergraph" : "Minigraph");
	debug("--- System Init complete ---\n");

	Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
	Menu_Functions = (unsigned long *)Main_Menu_Functions;
	Menu_String = (unsigned char *)&Main_Menu_Text;

 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
		// Debug Test routines
		craftTestMenuThruDebug();

		//debugRaw("k");
		//testKeypad();

#if 1
		// Handle system events
	    SystemEventManager();

		// Handle menu events
	    MenuEventManager();

		// Handle craft processing
		CraftManager();

		// Handle messages to be processed
		MessageManager();

		// Handle processing the factory setup
		FactorySetupManager();
#endif
		// Check if no System Event and Lcd and Printer power are off
		if ((SysEvents_flags.wrd == 0x0000) && !(powerManagement.reg & 0x60) &&
			(g_ModemStatus.xferState == NOP_CMD))
		{
			// Sleep
#if 0 // fix_ns8100
			Wait();
#endif
		}
	}    
	// End of NS7100 Main

	return (0);
}
