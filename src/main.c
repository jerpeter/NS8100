/********************************************************
 Name          : main.c
 Author        : JP
 **********************************************************/

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "sdramc.h"
#include "intc.h"
#include "usart.h"
#include "print_funcs.h"
#include "craft.h"
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
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
#include "Record.h"
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
#include "usb_task.h"
#include "device_mass_storage_task.h"
#include "usb_drv.h"
#include "FAT32_FileLib.h"
#include "srec.h"
#include "flashc.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define PBA_HZ         FOSC0
#define ONE_MS_RESOLUTION	1000

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
  g_smc_tab_cs_size[ncs] = EXT_SM_SIZE; \
  }

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------
#include "Globals.h"
extern int rtc_init(volatile avr32_rtc_t *rtc, unsigned char osc_type, unsigned char psel);
extern void rtc_set_top_value(volatile avr32_rtc_t *rtc, unsigned long top);
extern void rtc_enable(volatile avr32_rtc_t *rtc);

///----------------------------------------------------------------------------
///	Local Scope Globals
///----------------------------------------------------------------------------

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
void Setup_8100_EIC_Keypad_ISR(void);
void Setup_8100_EIC_System_ISR(void);
void Setup_8100_Soft_Timer_Tick_ISR(void);
void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM);
void Setup_8100_Usart_RS232_ISR(void);

//=============================================================================
//
//=============================================================================
#if 1 // fix_ns8100
void soft_delay(volatile unsigned long int counter)
{
	unsigned long int countdown = (counter << 2) + counter;
	
	for(; countdown > 0;)
	{
		countdown--;
	}
}
#endif

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
		
#if 0 
		// EIC
		{AVR32_EIC_EXTINT_5_PIN, AVR32_EIC_EXTINT_5_FUNCTION},

		// TWI
		{AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
		{AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION},
#endif

		// Usart 1 - RS232
		{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION},
		{AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION},
		{AVR32_USART1_RI_0_PIN, AVR32_USART1_RI_0_FUNCTION},
		{AVR32_USART1_DTR_0_PIN, AVR32_USART1_DTR_0_FUNCTION},
		{AVR32_USART1_DSR_0_PIN, AVR32_USART1_DSR_0_FUNCTION},
		{AVR32_USART1_DCD_0_PIN, AVR32_USART1_DCD_0_FUNCTION},
		{AVR32_USART1_RTS_0_0_PIN, AVR32_USART1_RTS_0_0_FUNCTION},
		{AVR32_USART1_CTS_0_0_PIN, AVR32_USART1_CTS_0_0_FUNCTION},

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
#define AVR32_WDT_KEY_VALUE_ASSERT		0x55000000
#define AVR32_WDT_KEY_VALUE_DEASSERT	0xAA000000
#define AVR32_WDT_DISABLE_VALUE			0x00000000

void _init_startup(void)
{   
	// Disable watchdog if reset from bootloader
	AVR32_WDT.ctrl = (AVR32_WDT_KEY_VALUE_ASSERT | AVR32_WDT_DISABLE_VALUE);
	AVR32_WDT.ctrl = (AVR32_WDT_KEY_VALUE_DEASSERT | AVR32_WDT_DISABLE_VALUE);
	
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
#if 1
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
#endif

	// TWI options.
	twi_options_t opt;

	// options settings
	opt.pba_hz = FOSC0;
	opt.speed = TWI_SPEED;
	opt.chip = IO_ADDRESS_KPD;

#if 1
	// TWI gpio pins configuration
	gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));
#endif

	// initialize TWI driver with options
	if (twi_master_init(&AVR32_TWI, &opt) != TWI_SUCCESS)
	{
		debugErr("Two Wire Interface failed to initialize!\n");
	}

	soft_usecWait(25 * SOFT_MSECS);
	//soft_usecWait(250 * SOFT_MSECS);
	//soft_usecWait(1 * SOFT_SECS);

	init_mcp23018(IO_ADDRESS_KPD);

	soft_usecWait(25 * SOFT_MSECS);
}

//-----------------------------------------------------------------------------
uint8 craft_g_input_buffer[120];
BOOLEAN processCraftCmd = NO;
void craftTestMenuThruDebug(void)
{
	int count;
    int character;
    int reply=0;

	//if(usart_test_hit(&AVR32_USART1))
	if (processCraftCmd == YES)
	{
		if(Get_User_Input((uint8*)&g_input_buffer[0]) > 0)
		{
        	reply = atoi((char *)g_input_buffer);
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
		
		processCraftCmd = NO;
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
//	Function:	UsbDeviceManager
//=================================================================================================
enum {
	USB_INIT_DRIVER = 0,
	USB_NOT_CONNECTED = 1,
	USB_CONNECTED_AND_PROCESSING = 2
};
extern volatile uint8 g_sampleProcessing;
void UsbDeviceManager(void)
{
	static uint8 usbMassStorageState = USB_INIT_DRIVER;
	INPUT_MSG_STRUCT mn_msg;
	
	// Check if the USB and Mass Storage Driver have never been initialized
	if (usbMassStorageState == USB_INIT_DRIVER)
	{
		debug("Init USB Mass Storage Driver...\n");

		// Init the USB and Mass Storage driver			
		usb_task_init();
		device_mass_storage_task_init();

		// Set state to ready to process
		usbMassStorageState = USB_NOT_CONNECTED;	
	}

	// Check if USB Cable is plugged in and not monitoring and not handling a trigger
	if ((Is_usb_vbus_high()) && (g_sampleProcessing != ACTIVE_STATE) && (!getSystemEventState(TRIGGER_EVENT)) && 
			g_fileProcessActiveUsbLockout == OFF)
	{
		// Check if USB and Mass Storage driver init needs to occur
		if (usbMassStorageState == USB_NOT_CONNECTED)
		{
			overlayMessage("USB STATUS", "USB CABLE WAS CONNECTED", 1 * SOFT_SECS);

			// Recall the current active menu to repaint the display
			mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
			(*menufunc_ptrs[g_activeMenu]) (mn_msg);
						
			debug("USB Mass Storage Driver Re-Init\n");
			// Init the USB and Mass Storage driver
			usb_task_init();
			device_mass_storage_task_init();

			// Set state to ready to process
			usbMassStorageState = USB_CONNECTED_AND_PROCESSING;	
		}

		// Call Usb and Device Storage drivers if connected to check and handle incoming actions
		usb_task();
		device_mass_storage_task();
	}
	else // USB Cable is not plugged in
	{
		// Check if the USB was plugged in prior
		if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
		{
			if (g_sampleProcessing == ACTIVE_STATE)
			{
				overlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR MONITORING", 1000 * SOFT_MSECS);
			}
			else if (g_fileProcessActiveUsbLockout == ON)
			{
				overlayMessage("USB STATUS", "USB CONNECTION DISABLED FOR FILE OPERATION", 1000 * SOFT_MSECS);
			}
			else
			{
				overlayMessage("USB STATUS", "USB CABLE WAS DISCONNECTED", 1000 * SOFT_MSECS);

				// Recall the current active menu to repaint the display
				mn_msg.cmd = 0; mn_msg.length = 0; mn_msg.data[0] = 0;
				(*menufunc_ptrs[g_activeMenu]) (mn_msg);
			}							

			debug("USB Cable Unplugged\n");
			Usb_disable();
		}
		
		usbMassStorageState = USB_NOT_CONNECTED;
	}	
}

//=================================================================================================
//	Function:	BootLoadManager
//=================================================================================================
#define APPLICATION    (((void *)AVR32_EBI_CS1_ADDRESS) + 0x00700000)
const char default_boot_name[] = {
	"Boot.s\0"
};

uint8 quickBootEntryJump = NO;
void BootLoadManager(void)
{
	int16 usart_status;
	int craft_char;
	char textBuffer[50];
	static void (*func)(void);
	func = (void(*)())APPLICATION;
	FL_FILE* file;
	uint32 i;

	usart_status = USART_RX_EMPTY;
	i = 0;

	usart_status = usart_read_char(DBG_USART, &craft_char);

#if 0
	if (craft_char == CTRL_B)
#else
	if ((craft_char == CTRL_B) || (quickBootEntryJump == YES))
#endif
	{
#if 1
		if (quickBootEntryJump == YES)
			overlayMessage("BOOTLOADER", "HIDDEN ENTRY...", 2 * SOFT_SECS);
		else
#endif
		overlayMessage("BOOTLOADER", "FOUND CTRL_B...", 2 * SOFT_SECS);

		sprintf(textBuffer,"C:\\System\\%s", default_boot_name);
		file = fl_fopen(textBuffer, "r");

		while (file == NULL)
		{
			//WriteLCD_smText( 0, 64, (unsigned char *)"Connect USB..", NORMAL_LCD);
			overlayMessage("BOOTLOADER", "BOOT FILE NOT FOUND. CONNECT THE USB CABLE", 0);

			usb_task_init();
			device_mass_storage_task_init();

			while(!Is_usb_vbus_high()) {}

			overlayMessage("BOOTLOADER", "WRITE BOOT.S TO SYSTEM DIR (REMOVE CABLE WHEN DONE)", 0);

			usb_task_init();
			device_mass_storage_task_init();

			while (Is_usb_vbus_high())
			{
				usb_task();
				device_mass_storage_task();
			}
			
			soft_usecWait(250 * SOFT_MSECS);
			
			file = fl_fopen(textBuffer, "r");
		}

		// Display initializing message
		debug("Starting Boot..\n");

		overlayMessage("BOOTLOADER", "STARTING BOOTLOADER...", 2 * SOFT_SECS);

		clearLcdDisplay();

		if (unpack_srec(file) == -1)
		{
			debugErr("SREC unpack unsuccessful!\n");
		}

		fl_fclose(file);

		Disable_global_interrupt();

		//Jump to boot application code
		func();
	}
	
	initCraftInterruptBuffers();
	Setup_8100_Usart_RS232_ISR();
}

//=================================================================================================
//	Function:	InitSystemHardware_NS8100
//=================================================================================================
void InitSystemHardware_NS8100(void)
{
	//-------------------------------------------------------------------------
	// Clock and chip selects setup in custom _init_startup

	//-------------------------------------------------------------------------
	// Set RTC Timestamp pin high
	gpio_set_gpio_pin(AVR32_PIN_PB18);

	//-------------------------------------------------------------------------
	// Set LAN to Sleep
    gpio_clr_gpio_pin(AVR32_PIN_PB27);
	
	//-------------------------------------------------------------------------
	// Set LAN and LAN Mem CS High
	gpio_set_gpio_pin(AVR32_EBI_NCS_2_PIN);
	gpio_set_gpio_pin(AVR32_EBI_NCS_3_PIN);	

	//-------------------------------------------------------------------------
    // Turn on rs232 driver and receiver on NS8100 board ?
    gpio_clr_gpio_pin(AVR32_PIN_PB08);
    gpio_clr_gpio_pin(AVR32_PIN_PB09);

    // Setup debug serial port
	usart_options_t usart_1_rs232_options =
	{
		.baudrate = 115200,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};

	// Initialize it in RS232 mode.
	usart_init_rs232(&AVR32_USART1, &usart_1_rs232_options, FOSC0);

	// Init signals for ready to send and terminal ready
	SET_RTS; SET_DTR;

	//-------------------------------------------------------------------------
	// Init the SPI interface
	SPI_init();
	
	//-------------------------------------------------------------------------
	// Initialize the external RTC
	InitExternalRtc();

	//-------------------------------------------------------------------------
    // Turn on display
    gpio_set_gpio_pin(AVR32_PIN_PB21);

	// LCD Init
    Backlight_On();
    Backlight_High();
    Set_Contrast(24);
    InitDisplay();

	//-------------------------------------------------------------------------
	// Initialize the Real Time Counter for half second tick used for state processing
	if (!rtc_init(&AVR32_RTC, 1, 0))
	{
		debugErr("Error initializing the RTC\n");
		while(1);
	}

	// Set top value to generate an interrupt every 1/2 second */
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

	// Turn on the red keypad LED while loading
	write_mcp23018(IO_ADDRESS_KPD, GPIOA, ((read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | RED_LED_PIN));

	//-------------------------------------------------------------------------
	// Enable Processor A/D
	adc_configure(&AVR32_ADC);

	// Enable the A/D channels.
	adc_enable(&AVR32_ADC, VBAT_CHANNEL);
	adc_enable(&AVR32_ADC, VIN_CHANNEL);

	//-------------------------------------------------------------------------
	// Power on the SD Card and init the file system
	SD_MMC_Power_On();

	// Wait for power to propogate
	soft_usecWait(10 * SOFT_MSECS);

	// Check if SD Detect pin 
	if (gpio_get_pin_value(AVR32_PIN_PA02) == ON)
	{
		spi_selectChip(SDMMC_SPI, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(SDMMC_SPI, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}
	}
	else
	{
		debugErr("\n\nSD Card not detected!\n");
	}

	//-------------------------------------------------------------------------
    // Initialize USB clock.
    pm_configure_usb_clock();

#if 0 // Moved to USB device manager
	// Init USB and Mass Storage drivers
    usb_task_init();
    device_mass_storage_task_init();
#endif

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

	Setup_8100_Soft_Timer_Tick_ISR();
	Setup_8100_TC_Clock_ISR(ONE_MS_RESOLUTION, TC_TYPEMATIC_TIMER_CHANNEL);

	Setup_8100_EIC_Keypad_ISR();
	Setup_8100_EIC_System_ISR();

	// Moved interrupt setup to the end of the BootloaderManager to allow for searching for Ctrl-B
	//initCraftInterruptBuffers();
	//Setup_8100_Usart_RS232_ISR();
	
	Enable_global_interrupt();
}

//=================================================================================================
//	Function:	InitSoftwareSettings_NS8100
//=================================================================================================
void InitSoftwareSettings_NS8100(void)
{
	INPUT_MSG_STRUCT mn_msg;

	debug("Init Software Settings\n");
    debug("Init Version Strings...\n");

	// Init version strings
	initVersionStrings();

	// Init version msg
    debug("Init Version Msg...\n");
	initVersionMsg();

	// Init time msg
    debug("Init Time Msg...\n");
	initTimeMsg();

	// Load current time into cache
	updateCurrentTime();

	// Get the function address passed by the bootloader
    debug("Init Get Boot Function Addr...\n");
	getBootFunctionAddress();

	// Setup defaults, load records, init the language table
    debug("Init Setup Menu Defaults...\n");
	setupMnDef();

	// Check for Timer mode activation
#if 0
    debug("Init Timer Mode Check...\n");
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
#else
	debugWarn("WARNING: Timer mode check logic disabled!\n");
#endif

	// Init the cmd message handling buffers before initialization of the ports.
    debug("Init Cmd Msg Handler...\n");
	cmdMessageHandlerInit();

	// Init the input buffers and status flags for input craft data.
    debug("Init Craft Init Status Flags...\n");
	craftInitStatusFlags();

#if 0 // ns7100
	// Overlay a message to alert the user that the system is checking the sensors
    debug("Init LCD Message...\n");
	char buff[50];
	byteSet(&buff[0], 0, sizeof(buff));
	sprintf((char*)buff, "%s %s", getLangText(SENSOR_CHECK_TEXT), getLangText(ZEROING_SENSORS_TEXT));
	overlayMessage(getLangText(STATUS_TEXT), buff, 0);
#endif

	// Reset LCD timers
	resetSoftTimer(DISPLAY_ON_OFF_TIMER_NUM);
	resetSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM);

#if 1 // fix_ns8100
	// Have to recall Keypad init otherwise interrupt hangs
	InitKeypad();
#endif

	// Turn on the Green keypad LED when system init complete
	write_mcp23018(IO_ADDRESS_KPD, GPIOA, ((read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | GREEN_LED_PIN));

	// Assign a one second keypad led update timer
	assignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, keypadLedUpdateTimerCallBack);

#if 0 // Logic test
	uint8 keypadState;
	while (1 == 1)
	{
		keypadState = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
		if (keypadState & GREEN_LED_PIN)
			debug("Green LED Active\n");
		if (keypadState & RED_LED_PIN)
			debug("Red LED Active\n");

		debug("Turning on the Green LED only\n");
		keypadState |= GREEN_LED_PIN;
		keypadState &= ~RED_LED_PIN;
		write_mcp23018(IO_ADDRESS_KPD, GPIOA, keypadState);

		soft_usecWait(3 * SOFT_SECS);

		keypadState = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
		if (keypadState & GREEN_LED_PIN)
			debug("Green LED Active\n");
		if (keypadState & RED_LED_PIN)
			debug("Red LED Active\n");

		debug("Turning on the Red LED only\n");
		keypadState &= ~GREEN_LED_PIN;
		keypadState |= RED_LED_PIN;
		write_mcp23018(IO_ADDRESS_KPD, GPIOA, keypadState);

		soft_usecWait(3 * SOFT_SECS);

		keypadState = read_mcp23018(IO_ADDRESS_KPD, GPIOA);
		if (keypadState & GREEN_LED_PIN)
			debug("Green LED Active\n");
		if (keypadState & RED_LED_PIN)
			debug("Red LED Active\n");

		debug("Turning off both LEDs\n");
		keypadState &= ~GREEN_LED_PIN;
		keypadState &= ~RED_LED_PIN;
		write_mcp23018(IO_ADDRESS_KPD, GPIOA, keypadState);

		soft_usecWait(3 * SOFT_SECS);
	}
#endif

	debug("Jump to Main Menu\n");
	// Jump to the true main menu
	g_activeMenu = MAIN_MENU;
	ACTIVATE_MENU_MSG();
	(*menufunc_ptrs[g_activeMenu]) (mn_msg);
}

//=================================================================================================
//	Function:	Main
//	Purpose:	Application starting point
//=================================================================================================
extern void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM);
extern void Start_Data_Clock(TC_CHANNEL_NUM);
extern void AD_Init(void);
int main(void)
{
    InitSystemHardware_NS8100();
	InitInterrupts_NS8100();
	InitSoftwareSettings_NS8100();
	BootLoadManager();

	//debug("Unit Type: %s\n", "NS8100 Minigraph");
	debug("--- System Init complete ---\n");

#if 1
	Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
	Menu_Functions = (unsigned long *)Main_Menu_Functions;
	Menu_String = (unsigned char *)&Main_Menu_Text;
#endif

 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
		g_execCycles++;

		// Debug Test routines
		//craftTestMenuThruDebug();

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

		// Handle USB device
		UsbDeviceManager();
		
		// Handle processing the factory setup
		FactorySetupManager();
#endif
		// Check if no System Events and Lcd is off and Modem is not transfering
		if ((g_systemEventFlags.wrd == 0x0000) && (getPowerControlState(LCD_POWER_ENABLE) == OFF) &&
			(g_modemStatus.xferState == NOP_CMD))
		{
			// Sleep
#if 0 // fix_ns8100
			Wait();
#endif
		}
	}    
	// End of NS8100 Main

	// End of the world
	return (0);
}
