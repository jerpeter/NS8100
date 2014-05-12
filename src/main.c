///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2003-2014, All Rights Reserved
///
///	Author: Jeremy Peterson
///----------------------------------------------------------------------------

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
#define AVR32_IDLE_MODE		AVR32_PM_SMODE_IDLE
#define PBA_HZ				FOSC0
#define ONE_MS_RESOLUTION	1000

#define EEPROM_SPI_CS_NUM			0
#define RTC_SPI_CS_NUM				1
#define SDMMC_SPI_CS_NUM			2
#define AD_CTL_SPI_CS_NUM			3
#define EEPROM_SPI_MAX_SPEED		500000 //2100000 // Speed should be safe up to 2.1 MHz, needs to be tested
#define RTC_SPI_MAX_SPEED			1000000 // 1000000 // Speed should be safe up to 3.5 MHz, needs to be tested
#define SDMMC_SPI_MAX_SPEED			12000000 // Speed should be safe up to 12 MHz (150 KHz * 80x)
#define AD_CTL_SPI_MAX_SPEED		4000000 // Speed should be safe up to 10 MHz, needs to be tested

// Chip select defines
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

#define NRD_SETUP_SPECIAL     10 //10
#define NRD_PULSE_SPECIAL     250 //135 //135
#define NRD_CYCLE_SPECIAL     180 //180

#define NCS_RD_SETUP_SPECIAL  35 //10
#define NCS_RD_PULSE_SPECIAL  250 //150 //250

#define NWE_SETUP_SPECIAL     20 //20
#define NWE_PULSE_SPECIAL     110 //110
#define NWE_CYCLE_SPECIAL     165 //150

#define NCS_WR_SETUP_SPECIAL  35 //20
#define NCS_WR_PULSE_SPECIAL  230 //150 //230

#define SMC_CS_SETUP_SPECIAL(ncs) { \
  U32 nwe_setup    = ((NWE_SETUP_SPECIAL    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_wr_setup = ((NCS_WR_SETUP_SPECIAL * hsb_mhz_up + 999) / 1000); \
  U32 nrd_setup    = ((NRD_SETUP_SPECIAL    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_rd_setup = ((NCS_RD_SETUP_SPECIAL * hsb_mhz_up + 999) / 1000); \
  U32 nwe_pulse    = ((NWE_PULSE_SPECIAL    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_wr_pulse = ((NCS_WR_PULSE_SPECIAL * hsb_mhz_up + 999) / 1000); \
  U32 nrd_pulse    = ((NRD_PULSE_SPECIAL    * hsb_mhz_up + 999) / 1000); \
  U32 ncs_rd_pulse = ((NCS_RD_PULSE_SPECIAL * hsb_mhz_up + 999) / 1000); \
  U32 nwe_cycle    = ((NWE_CYCLE_SPECIAL    * hsb_mhz_up + 999) / 1000); \
  U32 nrd_cycle    = ((NRD_CYCLE_SPECIAL    * hsb_mhz_up + 999) / 1000); \
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
void Setup_8100_EIC_External_RTC_ISR(void);
void Setup_8100_EIC_Keypad_ISR(void);
void Setup_8100_EIC_System_ISR(void);
void Setup_8100_Soft_Timer_Tick_ISR(void);
void Setup_8100_TC_Clock_ISR(uint32 sampleRate, TC_CHANNEL_NUM);
void Setup_8100_Usart_RS232_ISR(void);

//=============================================================================
// SPI_0_Init
//=============================================================================
void SPI_0_Init(void)
{
	const gpio_map_t spi0Map =
	{
		{AVR32_SPI0_SCK_0_0_PIN,  AVR32_SPI0_SCK_0_0_FUNCTION },  // SPI Clock.
		{AVR32_SPI0_MISO_0_0_PIN, AVR32_SPI0_MISO_0_0_FUNCTION},  // MISO.
		{AVR32_SPI0_MOSI_0_0_PIN, AVR32_SPI0_MOSI_0_0_FUNCTION},  // MOSI.
		{AVR32_SPI0_NPCS_0_0_PIN, AVR32_SPI0_NPCS_0_0_FUNCTION}   // Chip Select NPCS.
	};
  
	// SPI options.
	spi_options_t spiOptions =
	{
	.reg          = 0,
	.baudrate     = 33000000, //36000000, //33000000, // 33 MHz
	.bits         = 16,
	.spck_delay   = 0,
	.trans_delay  = 0,
	.stay_act     = 1,
	.spi_mode     = 0,
	.modfdis      = 1
	};

	// Assign I/Os to SPI.
	gpio_enable_module(spi0Map, sizeof(spi0Map) / sizeof(spi0Map[0]));

	// Initialize as master.
	spi_initMaster(&AVR32_SPI0, &spiOptions);

	// Set SPI selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(&AVR32_SPI0, 0, 0, 0);

	// Enable SPI module.
	spi_enable(&AVR32_SPI0);

	// Initialize AD driver with SPI clock (PBA).
	// Setup SPI registers according to spiOptions.
	spi_setupChipReg(&AVR32_SPI0, &spiOptions, FOSC0);
}

//=============================================================================
// SPI_init
//=============================================================================
void SPI_1_Init(void)
{
	// SPI 1 MAP Pin select	
	const gpio_map_t spi1Map =
	{
		{AVR32_SPI1_SCK_0_0_PIN, AVR32_SPI1_SCK_0_0_FUNCTION },		// SPI Clock.
		{AVR32_SPI1_MISO_0_0_PIN, AVR32_SPI1_MISO_0_0_FUNCTION},	// MISO.
		{AVR32_SPI1_MOSI_0_0_PIN, AVR32_SPI1_MOSI_0_0_FUNCTION},	// MOSI.
		{AVR32_SPI1_NPCS_3_PIN,	AVR32_SPI1_NPCS_3_FUNCTION},		// AD Control Chip Select NPCS.
		{AVR32_SPI1_NPCS_0_0_PIN, AVR32_SPI1_NPCS_0_0_FUNCTION},	// EEprom Chip Select NPCS.
		{AVR32_SPI1_NPCS_1_0_PIN, AVR32_SPI1_NPCS_1_0_FUNCTION},	// RTC Chip Select NPCS.
		{AVR32_SPI1_NPCS_2_0_PIN, AVR32_SPI1_NPCS_2_0_FUNCTION},	// SDMMC Chip Select NPCS.
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

	spiOptions.reg = AD_CTL_SPI_CS_NUM; // 3
	spiOptions.baudrate = AD_CTL_SPI_MAX_SPEED; // 4 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);

	spiOptions.reg = EEPROM_SPI_CS_NUM; // 0
	spiOptions.baudrate = EEPROM_SPI_MAX_SPEED; // 2.1 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);

	spiOptions.reg = RTC_SPI_CS_NUM; // 1
	spiOptions.baudrate = RTC_SPI_MAX_SPEED; // 1 MHz
	spi_setupChipReg(&AVR32_SPI1, &spiOptions, FOSC0);

	spiOptions.reg = SDMMC_SPI_CS_NUM; // 2
	spiOptions.baudrate = SDMMC_SPI_MAX_SPEED; // 12 MHz
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

#if EXTERNAL_SAMPLING_SOURCE
		// USB_VBOF - reassigned to External RTC interrupt
		{AVR32_EIC_EXTINT_1_PIN, AVR32_EIC_EXTINT_1_FUNCTION},
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

		// Usart 3 - RS485
		{AVR32_USART3_RXD_0_0_PIN, AVR32_USART3_RXD_0_0_FUNCTION},
		{AVR32_USART3_TXD_0_0_PIN, AVR32_USART3_TXD_0_0_FUNCTION},
		{AVR32_USART3_RTS_0_1_PIN, AVR32_USART3_RTS_0_1_FUNCTION},

		// Voltage monitor pins
		{AVR32_ADC_AD_2_PIN, AVR32_ADC_AD_2_FUNCTION},
		{AVR32_ADC_AD_3_PIN, AVR32_ADC_AD_3_FUNCTION}
	};

	gpio_enable_module(SMC_EBI_GPIO_MAP, sizeof(SMC_EBI_GPIO_MAP) / sizeof(SMC_EBI_GPIO_MAP[0]));
}
//=================================================================================================

//=================================================================================================
void initProcessorNoConnectPins(void)
{
	gpio_clr_gpio_pin(AVR32_PIN_PA00); // USART0_RXD
	gpio_clr_gpio_pin(AVR32_PIN_PA01); // USART0_TXD
	gpio_clr_gpio_pin(AVR32_PIN_PB19); // GPIO 51
	gpio_clr_gpio_pin(AVR32_EBI_SDA10_0_PIN);
	gpio_clr_gpio_pin(AVR32_EBI_RAS_0_PIN);
	gpio_clr_gpio_pin(AVR32_EBI_CAS_0_PIN);
	gpio_clr_gpio_pin(AVR32_EBI_SDWE_0_PIN);
	gpio_clr_gpio_pin(AVR32_EBI_SDCS_0_PIN);
	gpio_clr_gpio_pin(AVR32_EBI_NWAIT_0_PIN);

#if INTERNAL_SAMPLING_SOURCE
	// This pin is unused if the internal sampling source is configured. Set pin as an output to prevent it from floating
	gpio_clr_gpio_pin(AVR32_PIN_PA22); // USB_VBOF
#endif
}
//=================================================================================================

//=================================================================================================
void avr32_chip_select_init(unsigned long hsb_hz)
{
  unsigned long int hsb_mhz_up = (hsb_hz + 999999) / 1000000;

  // Setup all 4 chip selects
  SMC_CS_SETUP(0)			// LCD
  SMC_CS_SETUP(1)			// External RAM
  SMC_CS_SETUP_SPECIAL(2)	// Network/LAN
  SMC_CS_SETUP(3)			// Network/LAN Memory

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
	
    // Switch the main clock to the external oscillator 0 (12 MHz)
    pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

#if 1 // Normal (Set clock to 66 MHz)
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
#endif

#if 0 // Test (Network Active low SBHE)
    gpio_set_gpio_pin(AVR32_EBI_NWE1_0_PIN);
    gpio_clr_gpio_pin(AVR32_EBI_NWE1_0_PIN);
    gpio_set_gpio_pin(AVR32_EBI_NWE1_0_PIN);
#endif

#if 0 // Test (Network Active low SBHE - Hardware mod connecting pin GPIO 51 to network part)
    gpio_clr_gpio_pin(AVR32_PIN_PB19);
    //gpio_set_gpio_pin(AVR32_PIN_PB19);
    //gpio_clr_gpio_pin(AVR32_PIN_PB19);
    //gpio_set_gpio_pin(AVR32_PIN_PB19);
    //gpio_clr_gpio_pin(AVR32_PIN_PB19);
#endif

	// Chip Select Initialization
#if 1 // Normal
	avr32_chip_select_init(FOSC0);
#else // Test (12Mhz)
	avr32_chip_select_init(12000000);
#endif
	
	// Disable the unused and non connected clock 1
	pm_disable_clk1(&AVR32_PM);

	// With clock 1 disabled, configure GPIO lines to be outputs and low
	gpio_clr_gpio_pin(AVR32_PM_XIN1_0_PIN);
	gpio_clr_gpio_pin(AVR32_PM_XOUT1_0_PIN);

	soft_usecWait(1000);
}

//=================================================================================================
void InitKeypad(void)
{
	static const gpio_map_t EIC_GPIO_MAP =
	{
		{AVR32_EIC_EXTINT_4_PIN, AVR32_EIC_EXTINT_4_FUNCTION},
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
	if (twi_master_init(&AVR32_TWI, &opt) != TWI_SUCCESS)
	{
		debugErr("Two Wire Interface failed to initialize!\n");
	}

	soft_usecWait(25 * SOFT_MSECS);
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

//=================================================================================================
//	Function:	UsbDeviceManager
//=================================================================================================
enum {
	USB_INIT_DRIVER = 0,
	USB_NOT_CONNECTED = 1,
	USB_CONNECTED_AND_PROCESSING = 2
};
extern uint8 g_sampleProcessing;
#if 1 // Test - need access in main() to determine sleep level
uint8 usbMassStorageState = USB_INIT_DRIVER;
#endif
void UsbDeviceManager(void)
{
#if 0 // Test - need access in main() to determine sleep level
	static uint8 usbMassStorageState = USB_INIT_DRIVER;
#endif
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
			JUMP_TO_ACTIVE_MENU();
						
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
				JUMP_TO_ACTIVE_MENU();
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
	// Enable internal pull ups on the floating data lines
	//-------------------------------------------------------------------------
	gpio_enable_pin_pull_up(AVR32_PIN_PX00);
	gpio_enable_pin_pull_up(AVR32_PIN_PX01);
	gpio_enable_pin_pull_up(AVR32_PIN_PX02);
	gpio_enable_pin_pull_up(AVR32_PIN_PX03);
	gpio_enable_pin_pull_up(AVR32_PIN_PX04);
	gpio_enable_pin_pull_up(AVR32_PIN_PX05);
	gpio_enable_pin_pull_up(AVR32_PIN_PX06);
	gpio_enable_pin_pull_up(AVR32_PIN_PX07);
	gpio_enable_pin_pull_up(AVR32_PIN_PX08);
	gpio_enable_pin_pull_up(AVR32_PIN_PX09);
	gpio_enable_pin_pull_up(AVR32_PIN_PX10);
	gpio_enable_pin_pull_up(AVR32_PIN_PX35);
	gpio_enable_pin_pull_up(AVR32_PIN_PX36);
	gpio_enable_pin_pull_up(AVR32_PIN_PX37);
	gpio_enable_pin_pull_up(AVR32_PIN_PX38);
	gpio_enable_pin_pull_up(AVR32_PIN_PX39);

	//-------------------------------------------------------------------------
	// Clock and chip selects setup in custom _init_startup
	//-------------------------------------------------------------------------
	initProcessorNoConnectPins();
	
	//-------------------------------------------------------------------------
	// Set RTC Timestamp pin high (Active low signal)
	//-------------------------------------------------------------------------
	gpio_set_gpio_pin(AVR32_PIN_PB18);

	//-------------------------------------------------------------------------
	// Set Alarm 1 and Alarm 2 low (Active high signal)
	//-------------------------------------------------------------------------
	gpio_clr_gpio_pin(AVR32_PIN_PB06);
	gpio_clr_gpio_pin(AVR32_PIN_PB07);

	//-------------------------------------------------------------------------
	// Set Trigger Out low (Active high signal)
	//-------------------------------------------------------------------------
	gpio_clr_gpio_pin(AVR32_PIN_PB05);

	//-------------------------------------------------------------------------
	// Set USB LED Output low (Active high signal)
	//-------------------------------------------------------------------------
	gpio_clr_gpio_pin(AVR32_PIN_PB28);

	//-------------------------------------------------------------------------
	// Smart Sensor data in (Hardware pull up on signal)
	//-------------------------------------------------------------------------
	// Nothing needs to be done. Pin should default to an input on power up
	//gpio_enable_gpio_pin(AVR32_PIN_PB01)

	//-------------------------------------------------------------------------
    // Set SDATA and ADATA high
	//-------------------------------------------------------------------------
	gpio_set_gpio_pin(AVR32_PIN_PB02);
    gpio_set_gpio_pin(AVR32_PIN_PB03);

	//-------------------------------------------------------------------------
	// Init the SPI interfaces
	//-------------------------------------------------------------------------
	SPI_0_Init();
	SPI_1_Init();
	
	// Make sure SPI 0 and 1 inputs aren't floating
	gpio_enable_pin_pull_up(AVR32_SPI0_MISO_0_0_PIN);
	gpio_enable_pin_pull_up(AVR32_SPI1_MISO_0_0_PIN);

	//-------------------------------------------------------------------------
	// Turn on rs485 driver and receiver
	//-------------------------------------------------------------------------
	// Options for debug USART.
	usart_options_t usart_3_rs485_usart_options =
	{
		.baudrate = 38400,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};

	// Initialize it in RS485 mode.
	usart_init_rs485(&AVR32_USART3, &usart_3_rs485_usart_options, FOSC0);

	//-------------------------------------------------------------------------
	// Turn on rs232 driver and receiver (Active low controls)
	//-------------------------------------------------------------------------
	gpio_clr_gpio_pin(AVR32_PIN_PB08);
	gpio_clr_gpio_pin(AVR32_PIN_PB09);

	// Setup debug serial port
	usart_options_t usart_1_rs232_options =
	{
		#if 1 // Normal
		.baudrate = 115200,
		#else // Test (12Mhz)
		.baudrate = 38400,
		#endif
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};

	// Load the Help Record to get the stored Baud rate. Only dependency should be SPI
	getRecData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

	// Check if the Help Record is valid
	if (g_helpRecord.encode_ln == 0xA5A5)
	{
		// Set the baud rate to the user stored baud rate setting (initialized to 115200)
		switch (g_helpRecord.baud_rate)
		{
			case BAUD_RATE_57600: usart_1_rs232_options.baudrate = 57600; break;
			case BAUD_RATE_38400: usart_1_rs232_options.baudrate = 38400; break;
			case BAUD_RATE_19200: usart_1_rs232_options.baudrate = 19200; break;
			case BAUD_RATE_9600: usart_1_rs232_options.baudrate = 9600; break;
		}
	}

	// Initialize it in RS232 mode.
	#if 1 // Normal
	usart_init_rs232(&AVR32_USART1, &usart_1_rs232_options, FOSC0);
	#else // Test (12Mhz)
	usart_init_rs232(&AVR32_USART1, &usart_1_rs232_options, 12000000);
	#endif

	//-------------------------------------------------------------------------
	// Initialize the external RTC
	//-------------------------------------------------------------------------
	InitExternalRtc();

#if 1 // Normal
extern void Sleep8900(void);
extern void Sleep8900_LedOn(void);
	//-------------------------------------------------------------------------
	// Set LAN to Sleep
	//-------------------------------------------------------------------------
    gpio_clr_gpio_pin(AVR32_PIN_PB27); // Clear LAN Sleep pin (active low control)
	soft_usecWait(10 * SOFT_MSECS);
	
#if 0 // Test (LAN register map read)
	debug("\n\n");
	*((uint16*)0xC800030A) = 0x0000; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0000, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0002; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0002, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0020; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0020, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0022; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0022, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0024; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0024, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0026; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0026, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0028; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0028, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x002A; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x002A, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x002C; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x002C, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0030; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0030, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0034; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0034, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0040; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0040, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0042; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0042, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0102; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0102, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0104; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0104, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0106; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0106, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0108; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0108, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x010A; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x010A, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0112; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0112, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0114; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0114, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0116; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0116, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0118; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0118, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0120; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0120, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0124; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0124, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0128; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0128, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x012C; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x012C, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0130; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0130, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0132; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0132, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0134; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0134, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0136; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0136, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0138; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0138, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x013C; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x013C, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0144; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0144, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0146; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0146, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0150; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0150, *((uint16*)0xC800030C));
	*((uint16*)0xC800030A) = 0x0158; debug("Lan Address (0x%04x) returns Data: 0x%x\n", 0x0158, *((uint16*)0xC800030C));
	debug("\n");
#endif

	//Sleep8900();
extern void ToggleLedOn8900(void);
extern void ToggleLedOff8900(void);

	ToggleLedOn8900();
	soft_usecWait(250 * SOFT_MSECS);
	ToggleLedOff8900();
	soft_usecWait(250 * SOFT_MSECS);

	ToggleLedOn8900();
	soft_usecWait(250 * SOFT_MSECS);
	ToggleLedOff8900();
	soft_usecWait(250 * SOFT_MSECS);
	
	//Sleep8900_LedOn();
	Sleep8900();
#endif

	//-------------------------------------------------------------------------
	// Initialize the AD Control
	//-------------------------------------------------------------------------
	InitAnalogControl();

	//-------------------------------------------------------------------------
    // Turn on display
	//-------------------------------------------------------------------------
    gpio_set_gpio_pin(AVR32_PIN_PB22);
    gpio_set_gpio_pin(AVR32_PIN_PB21);

	// LCD Init
    Backlight_On();
    Backlight_High();
    Set_Contrast(24);
    InitDisplay();

	//-------------------------------------------------------------------------
	// Initialize the Internal Real Time Counter for half second tick used for state processing
	//-------------------------------------------------------------------------
	rtc_init(&AVR32_RTC, 1, 0);

	// Set top value to generate an interrupt every 1/2 second
	rtc_set_top_value(&AVR32_RTC, 8192);

	// Enable the Internal RTC
	rtc_enable(&AVR32_RTC);

	//-------------------------------------------------------------------------
	// Enable Processor A/D
	//-------------------------------------------------------------------------
	adc_configure(&AVR32_ADC);

	// Enable the A/D channels.
#if 0 // Can't use the driver enable because it's a single channel enable only and a write only register
	//adc_enable(&AVR32_ADC, VBAT_CHANNEL);
	//adc_enable(&AVR32_ADC, VIN_CHANNEL);
#else
	AVR32_ADC.cher = 0x0C; // Directly enable
#endif

	//-------------------------------------------------------------------------
	// Power on the SD Card and init the file system
	//-------------------------------------------------------------------------
	// Necessary ?
	// Set SD Power pin as GPIO
	//gpio_enable_gpio_pin(AVR32_PIN_PB15);
	
	// Necessary ?
	// Set SD Write Protect pin as GPIO (Active low signal)
	gpio_enable_gpio_pin(AVR32_PIN_PA07);
	
	// Necessary ?
	// Set SD Detect pin as GPIO
	gpio_enable_gpio_pin(AVR32_PIN_PA02);
	
	// Enable Power to SD
	gpio_set_gpio_pin(AVR32_PIN_PB15);

	// Wait for power to propagate
	soft_usecWait(10 * SOFT_MSECS);

	// Check if SD Detect pin 
	if (gpio_get_pin_value(AVR32_PIN_PA02) == ON)
	{
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

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
	//-------------------------------------------------------------------------
    pm_configure_usb_clock();

#if 0 // Moved to USB device manager
	// Init USB and Mass Storage drivers
    usb_task_init();
    device_mass_storage_task_init();
#endif

	//-------------------------------------------------------------------------
	// Init Keypad
	//-------------------------------------------------------------------------
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
	// Init External RTC Interrupt for interrupt source
	//-------------------------------------------------------------------------
#if 0 // Place at startup to prevent driving the line
	gpio_map_t EIC_GPIO_MAP =
	{
		{AVR32_EIC_EXTINT_1_PIN, AVR32_EIC_EXTINT_1_FUNCTION}
	};

	// Enable External Interrupt for MCP23018
	gpio_enable_module(EIC_GPIO_MAP, sizeof(EIC_GPIO_MAP) / sizeof(EIC_GPIO_MAP[0]));
#endif

#if EXTERNAL_SAMPLING_SOURCE
	// The internal pull up for this pin needs to be enables to bring the line high, otherwise the clock out will only reach half level
	gpio_enable_pin_pull_up(AVR32_EIC_EXTINT_1_PIN);
#endif

	//-------------------------------------------------------------------------
	//Display_Craft_Logo();
	//soft_usecWait(3 * SOFT_SECS);
	
#if 1
	//-------------------------------------------------------------------------
	// Pre-configure the A/D to prevent the unit from burning current charging internal reference (default config)
	// Enable the A/D
	debug("Enable the A/D\n");
	powerControl(ANALOG_SLEEP_ENABLE, OFF);

	// Delay to allow AD to power up/stabilize
	soft_usecWait(50 * SOFT_MSECS);

	// Setup the A/D Channel configuration
	debug("Setup A/D config and channels (External Ref, Temp On)\n");
extern void SetupADChannelConfig(uint32 sampleRate);
	SetupADChannelConfig(SAMPLE_RATE_DEFAULT);

#if 0 // Test
	//g_triggerRecord.trec.sample_rate = SAMPLE_RATE_DEFAULT;
	//GetChannelOffsets(SAMPLE_RATE_DEFAULT);
#endif

#if 0
	SAMPLE_DATA_STRUCT dummySample;

	//------------------------------Loop 1----------------------------------------------
	debug("Setup A/D config and channels (External Ref, Temp On)\n");
	SetupADChannelConfig(0x39D4);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp On)\n");
	SetupADChannelConfig(0x39DC);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Temp Off)\n");
	SetupADChannelConfig(0x39F4);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp Off)\n");
	SetupADChannelConfig(0x39FC);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);
	//----------------------------------------------------------------------------------
	
	//------------------------------Loop 2----------------------------------------------
	debug("Setup A/D config and channels (External Ref, Temp On)\n");
	SetupADChannelConfig(0x39D4);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp On)\n");
	SetupADChannelConfig(0x39DC);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Temp Off)\n");
	SetupADChannelConfig(0x39F4);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp Off)\n");
	SetupADChannelConfig(0x39FC);
	soft_usecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);
	//----------------------------------------------------------------------------------
#endif

	debug("Disable the A/D\n");
	powerControl(ANALOG_SLEEP_ENABLE, OFF);
#endif

#if 0
	//-------------------------------------------------------------------------
	// Test full power down and halt

	// Turn on the red keypad LED while loading
	write_mcp23018(IO_ADDRESS_KPD, GPIOA, ((read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | NO_LED_PINS));

	spi_reset(&AVR32_SPI1);
	gpio_clr_gpio_pin(AVR32_SPI1_MISO_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_MOSI_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_NPCS_3_PIN);

	debug("\nClosing up shop.\n\n");

    // Disable rs232 driver and receiver (Active low controls)
    gpio_set_gpio_pin(AVR32_PIN_PB08);
    gpio_set_gpio_pin(AVR32_PIN_PB09);
	
	//displayTimerCallBack();
	setLcdBacklightState(BACKLIGHT_OFF);

	//lcdPwTimerCallBack();
	powerControl(LCD_CONTRAST_ENABLE, OFF);
	clearLcdDisplay();
	clearControlLinesLcdDisplay();
	LcdClearPortReg();
	powerControl(LCD_POWER_ENABLE, OFF);

	gpio_clr_gpio_pin(AVR32_PIN_PB20);

	//SLEEP(AVR32_PM_SMODE_IDLE);
	SLEEP(AVR32_PM_SMODE_STOP);
	//SLEEP(AVR32_PM_SMODE_STANDBY);
	while (1) {}
#endif	
}

//=================================================================================================
//	Function:	PowerDownAndHalt
//=================================================================================================
void PowerDownAndHalt(void)
{
	// Enable the A/D
	debug("Enable the A/D\n");
	powerControl(ANALOG_SLEEP_ENABLE, OFF);

	// Delay to allow AD to power up/stabilize
	soft_usecWait(50 * SOFT_MSECS);

	debug("Setup A/D config and channels\n");
	// Setup the A/D Channel configuration
	extern void SetupADChannelConfig(uint32 sampleRate);
	SetupADChannelConfig(1024);

	debug("Disable the A/D\n");
	powerControl(ANALOG_SLEEP_ENABLE, OFF);

	spi_reset(&AVR32_SPI1);

	gpio_clr_gpio_pin(AVR32_SPI1_MISO_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_MOSI_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_NPCS_3_PIN);

	debug("\nClosing up shop.\n\n");

	// Disable rs232 driver and receiver (Active low controls)
	gpio_set_gpio_pin(AVR32_PIN_PB08);
	gpio_set_gpio_pin(AVR32_PIN_PB09);
	
	//displayTimerCallBack();
	setLcdBacklightState(BACKLIGHT_OFF);

	//lcdPwTimerCallBack();
	powerControl(LCD_CONTRAST_ENABLE, OFF);
	clearLcdDisplay();
	clearControlLinesLcdDisplay();
	LcdClearPortReg();
	powerControl(LCD_POWER_ENABLE, OFF);

	//SLEEP(AVR32_PM_SMODE_IDLE);
	SLEEP(AVR32_PM_SMODE_STOP);
	
	while (1) {}
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

#if 0 // Moved interrupt setup to the end of the BootloaderManager to allow for searching for Ctrl-B
	initCraftInterruptBuffers();
	Setup_8100_Usart_RS232_ISR();
#endif
	
	Enable_global_interrupt();
}

//=================================================================================================
//	Function:	InitSoftwareSettings_NS8100
//=================================================================================================
void InitSoftwareSettings_NS8100(void)
{
	INPUT_MSG_STRUCT mn_msg;

	//debug("Address of local variable: 0x%x\n", &mn_msg);

	debug("Init Software Settings\n");
    debug("Init Version Strings...\n");

	//-------------------------------------------------------------------------
	// Init version strings
	//-------------------------------------------------------------------------
#if 0 // ns7100
	initVersionStrings();
#endif

	//-------------------------------------------------------------------------
	// Init version msg
	//-------------------------------------------------------------------------
    debug("Init Version Msg...\n");
	initVersionMsg();

	//-------------------------------------------------------------------------
	// Init time msg
	//-------------------------------------------------------------------------
    debug("Init Time Msg...\n");
	initTimeMsg();

	//-------------------------------------------------------------------------
	// Load current time into cache
	//-------------------------------------------------------------------------
	updateCurrentTime();

	//-------------------------------------------------------------------------
	// Get the function address passed by the bootloader
	//-------------------------------------------------------------------------
    debug("Init Get Boot Function Addr...\n");
	getBootFunctionAddress();

	//-------------------------------------------------------------------------
	// Setup defaults, load records, init the language table
	//-------------------------------------------------------------------------
    debug("Init Setup Menu Defaults...\n");
	setupMnDef();

#if 1 // fix_ns8100
	// Have to recall Keypad init otherwise interrupt hangs (Following code needs keypad access)
	InitKeypad();
#endif

	//-------------------------------------------------------------------------
	// Check for Timer mode activation
	//-------------------------------------------------------------------------
    debug("Init Timer Mode Check...\n");
	if (timerModeActiveCheck() == TRUE)
	{
		debug("--- Timer Mode Startup ---\n");
		processTimerMode();

		// If here, the unit is in Timer mode, but did not power itself off yet
		// Enabling power off protection
		debug("Timer Mode: Enabling Power Off Protection\n");
		powerControl(POWER_OFF_PROTECTION_ENABLE, ON);
	}
	else
	{
		debug("--- Normal Startup ---\n");
	}

	//-------------------------------------------------------------------------
	// Init the cmd message handling buffers before initialization of the ports.
	//-------------------------------------------------------------------------
    debug("Init Cmd Msg Handler...\n");
	cmdMessageHandlerInit();

	//-------------------------------------------------------------------------
	// Init the input buffers and status flags for input craft data.
	//-------------------------------------------------------------------------
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

	//-------------------------------------------------------------------------
	// Signal remote end that RS232 Comm is available
	//-------------------------------------------------------------------------
	SET_RTS; SET_DTR; // Init signals for ready to send and terminal ready

	//-------------------------------------------------------------------------
	// Reset LCD timers
	//-------------------------------------------------------------------------
	resetSoftTimer(DISPLAY_ON_OFF_TIMER_NUM);
	resetSoftTimer(LCD_POWER_ON_OFF_TIMER_NUM);

	//-------------------------------------------------------------------------
	// Turn on the Green keypad LED when system init complete
	//-------------------------------------------------------------------------
	write_mcp23018(IO_ADDRESS_KPD, GPIOA, ((read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | GREEN_LED_PIN));

	//-------------------------------------------------------------------------
	// Assign a one second keypad led update timer
	//-------------------------------------------------------------------------
	assignSoftTimer(KEYPAD_LED_TIMER_NUM, ONE_SECOND_TIMEOUT, keypadLedUpdateTimerCallBack);

	//-------------------------------------------------------------------------
	// Jump to the true main menu
	//-------------------------------------------------------------------------
	debug("Jump to Main Menu\n");
	SETUP_MENU_MSG(MAIN_MENU);
	JUMP_TO_ACTIVE_MENU();
}

//=================================================================================================
//	Function:	testSnippetsExecLoop
//=================================================================================================
void testSnippetsBeforeInit(void)
{
#if 0 // Test (Enable serial and put processor in deep stop)
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

	debug("--- System Deep Stop ---\n");

	SLEEP(AVR32_PM_SMODE_DEEP_STOP);

	while (1) {;}
#endif
}

//=================================================================================================
//	Function:	testSnippetsAfterInit
//=================================================================================================
void testSnippetsAfterInit(void)
{

#if 0
	CMD_BUFFER_STRUCT inCmd;
	inCmd.msg[MESSAGE_HEADER_SIMPLE_LENGTH] = 0;
	handleVML(&inCmd);
#endif

#if 0 // Craft Test
	Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
	Menu_Functions = (unsigned long *)Main_Menu_Functions;
	Menu_String = (unsigned char *)&Main_Menu_Text;
#endif

#if 0 // Clear Internal RAM static variables 
	uint32 i = 0x28;
	while (i)
	{
		*((uint8*)i--) = 0;
	}
#endif

#if 0 // Test (Effective CS Low for SDMMC)
	while (1)
	{
		debug("SPI1 SDMMC CS Active (0)\n");
		spi_selectChip(&AVR32_SPI1, 2);
		spi_write(&AVR32_SPI1, 0x0000);
		soft_usecWait(5 * SOFT_SECS);


		debug("SPI1 SDMMC CS Inactive (1)\n");
		spi_unselectChip(&AVR32_SPI1, 2);
		soft_usecWait(5 * SOFT_SECS);
	}		
#endif

#if 0 // Test (SDMMC Reset to enable standby power)
	uint8 r1;
	uint32 retry = 0;
	uint32 timedAccess = 0;
	FL_FILE* monitorLogFile;
	MONITOR_LOG_ENTRY_STRUCT monitorLogEntry;
	int32 bytesRead = 0;

	while (1)
	{
		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);
	
		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------
		
		//---Init and idle-------------------------------------------------------------------
		debug("SDMMC Init and idle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------
		
		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------

		//---Init and FAT32 Cycle-------------------------------------------------------------------
		debug("SDMMC Init and FAT32 Init cycle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		timedAccess = g_rtcSoftTimerTickCount;
		
		while (g_rtcSoftTimerTickCount < (timedAccess + 10))
		{
			FAT32_InitDrive();
			if (FAT32_InitFAT() == FALSE)
			{
				debugErr("FAT32 Initialization failed!\n\r");
			}
		}
		//----------------------------------------------------------------------

		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------

		//---Init and Read cycle-------------------------------------------------------------------
		debug("SDMMC Init and Read cycle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}

		timedAccess = g_rtcSoftTimerTickCount;
		
		while (g_rtcSoftTimerTickCount < (timedAccess + 10))
		{
			monitorLogFile = fl_fopen("C:\\Logs\\TestLogRead.ns8", "r");
			if (monitorLogFile == NULL) { debugErr("Test Read file not found!\n"); }
			bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT));
			while (bytesRead > 0) { bytesRead = fl_fread(monitorLogFile, (uint8*)&monitorLogEntry, sizeof(MONITOR_LOG_ENTRY_STRUCT)); }
			fl_fclose(monitorLogFile);
		}		
		//----------------------------------------------------------------------

		//---Idle low power-------------------------------------------------------------------
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		retry = 0;
		do
		{
			r1 = sd_mmc_spi_send_command(MMC_GO_IDLE_STATE, 0);
			spi_write(SD_MMC_SPI,0xFF);
			retry++;
		}
		while(r1 != 0x01);
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		debug("SDMMC Cycles to enter idle state: %d\n", retry);

		soft_usecWait(5 * SOFT_SECS);
		//----------------------------------------------------------------------

		//---Init and Write cycle-------------------------------------------------------------------
		debug("SDMMC Init and Write cycle...\n", retry);
		spi_selectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);
		sd_mmc_spi_internal_init();
		spi_unselectChip(&AVR32_SPI1, SD_MMC_SPI_NPCS);

		FAT32_InitDrive();
		if (FAT32_InitFAT() == FALSE)
		{
			debugErr("FAT32 Initialization failed!\n\r");
		}

		timedAccess = g_rtcSoftTimerTickCount;
		
		while (g_rtcSoftTimerTickCount < (timedAccess + 10))
		{
			monitorLogFile = fl_fopen("C:\\Logs\\TestLogWrite.ns8", "a+");
			if (monitorLogFile == NULL) { debugErr("Test Write file not opened!\n"); }
			for (retry = 0; retry < 250; retry++)
			{
				fl_fwrite((uint8*)&(__monitorLogTbl[__monitorLogTblIndex]), sizeof(MONITOR_LOG_ENTRY_STRUCT), 1, monitorLogFile);
			}
			fl_fclose(monitorLogFile);
		}
		//----------------------------------------------------------------------
		}
#endif

#if 0 // Test (Ineffective CS Low for SDMMC)
	gpio_enable_gpio_pin(AVR32_PIN_PA14);
	gpio_enable_gpio_pin(AVR32_PIN_PA18);
	gpio_enable_gpio_pin(AVR32_PIN_PA19);
	gpio_enable_gpio_pin(AVR32_PIN_PA20);

	while (1)
	{
		debug("SPI1 CS's 0\n");
		//gpio_clr_gpio_pin(AVR32_PIN_PA14);
		//gpio_clr_gpio_pin(AVR32_PIN_PA18);
		gpio_clr_gpio_pin(AVR32_PIN_PA19);
		//gpio_clr_gpio_pin(AVR32_PIN_PA20);

		spi_selectChip(&AVR32_SPI1, 2);
		// Small delay before the RTC device is accessible
		soft_usecWait(500);
unsigned int i;
		for (i = 0; i < 10000; i++)
			spi_write(&AVR32_SPI1, 0x0000);
		spi_unselectChip(&AVR32_SPI1, 2);

		__monitorLogTblKey = 0;
		initMonitorLog();

		soft_usecWait(3 * SOFT_SECS);

		debug("SPI1 CS's 1\n");
		//gpio_set_gpio_pin(AVR32_PIN_PB14);
		gpio_set_gpio_pin(AVR32_PIN_PB18);
		//gpio_set_gpio_pin(AVR32_PIN_PB19);
		//gpio_set_gpio_pin(AVR32_PIN_PB20);

		soft_usecWait(3 * SOFT_SECS);
	}

#endif

#if 0 // Test (Timer mode)
	EnableRtcAlarm(0, 0, 0, 0);

	static RTC_MEM_MAP_STRUCT rtcMap;
	static uint8 clearAlarmFlag = 0x03; //0xEF; // Logic 0 on the bit will clear Alarm flag, bit 4
	static uint32 counter = 0;

	rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);
	
	if (rtcMap.control_2 & 0x10)
	{
		debug("RTC Alarm Flag indicates alarm condition raised. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
		
		rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
		debug("RTC Alarm Flag being cleared\n");

		rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);

		if (rtcMap.control_2 & 0x10)
		{
			debugWarn("RTC Alarm flag was not cleared successfully! (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
		}
		else
		{
			debug("RTC Alarm Flag cleared. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
		}		
	}
	else
	{
		rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
		debug("RTC Alarm Flag does not show an alarm condition. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
	}
#endif

#if 0 // Test (LCD off and Proc stop)
	debug("\n--- System Init Complete ---\n");
	soft_usecWait(10 * SOFT_SECS);
	displayTimerCallBack();
	lcdPwTimerCallBack();
	soft_usecWait(10 * SOFT_SECS);
	debug("--- System Deep Stop ---\n");

	SLEEP(AVR32_PM_SMODE_DEEP_STOP);

	while (1) {;}
#endif

#if 0 // Test (Keypad logic test)
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
}

//=================================================================================================
//	Function:	testSnippetsExecLoop
//=================================================================================================
void testSnippetsExecLoop(void)
{
#if 0 // Test (Timer mode)
		if (counter)
		{
			counter--;
			
			if (counter == 0)
			{
				rtcWrite(RTC_CONTROL_2_ADDR, 1, &clearAlarmFlag);
				debug("RTC Alarm Flag being cleared\n");

				rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);

				if (rtcMap.control_2 & 0x10)
				{
					debugWarn("RTC Alarm flag was not cleared successfully! (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
				}
				else
				{
					debug("RTC Alarm Flag cleared. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
				}		
			}
		}
		else
		{
			rtcRead(RTC_CONTROL_1_ADDR, 3, (uint8*)&rtcMap);
		
			if (rtcMap.control_2 & 0x10)
			{
				debug("RTC Alarm Flag indicates alarm condition raised. (0x%x, 0x%x, 0x%x)\n", rtcMap.control_1, rtcMap.control_2, rtcMap.control_3);
				counter = 1000000;
			}
		}
		
#endif

#if 0 // Test (Display temperature change readings)
		static uint16 s_tempReading = 0;
		
		// 0x49 to 0x4c
		if (abs((int)(g_currentTempReading - s_tempReading)) > 4)
		{
			debug("Temp update on change, Old: 0x%x, New: 0x%x\n", s_tempReading, g_currentTempReading);
			
			s_tempReading = g_currentTempReading;
		}
#endif

}

//=================================================================================================
//	Function:	Main
//	Purpose:	Application starting point
//=================================================================================================
int main(void)
{
	// Test code
	//testSnippetsBeforeInit();

    InitSystemHardware_NS8100();
#if 1 // Normal
	InitInterrupts_NS8100();
	InitSoftwareSettings_NS8100();
	BootLoadManager();
#endif

	int majorVer, minorVer;
	char buildVer;
	sscanf(&g_buildVersion[0], "%d.%d.%c", &majorVer, &minorVer, &buildVer);

	debug("--- System Init complete (Version %d.%d.%c) ---\n", majorVer, minorVer, buildVer);

#if 0 // External sampling source
extern void startExternalRTCClock(uint16 sampleRate);
	g_updateCounter = 1;
	startExternalRTCClock(TEST_SAMPLE_RATE);

	while (1==1)
	{
		g_updateCounter++;
		
		if (g_updateCounter % TEST_SAMPLE_RATE == 0)
		{
			debug("Tick tock (%d, %d)\n", TEST_SAMPLE_RATE, g_updateCounter / TEST_SAMPLE_RATE);
		}
		
		while (!usart_tx_empty(DBG_USART)) {;}
		SLEEP(AVR32_PM_SMODE_IDLE);
	}
#endif

	// Test code
	testSnippetsAfterInit();

#if 1 // Normal
 	// ==============
	// Executive loop
	// ==============
	while (1)
	{
		g_execCycles++;

		// Debug Test routines
		//craftTestMenuThruDebug();

		// Test code
		//testSnippetsExecLoop();

#if 1 // Normal operational cycle
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
		// Check if no System Events and LCD is off and Modem is not transferring
		if ((g_systemEventFlags.wrd == 0x0000) && (getPowerControlState(LCD_POWER_ENABLE) == OFF) &&
			(g_modemStatus.xferState == NOP_CMD))
		{
			// Sleepy time
#if 0 // Normal
			SLEEP(AVR32_IDLE_MODE);
#else // Test
			// Check if USB is connected
			if (usbMassStorageState == USB_CONNECTED_AND_PROCESSING)
			{
				// Can't operate the USB is the sleep mode is deeper than IDLE (due to HSB)
				SLEEP(AVR32_IDLE_MODE);
			}
			else // USB is not connected and a deeper sleep mode can be used
			{
				// Disable rs232 driver and receiver (Active low controls)
				gpio_set_gpio_pin(AVR32_PIN_PB08);
				gpio_set_gpio_pin(AVR32_PIN_PB09);

				SLEEP(AVR32_PM_SMODE_STANDBY);

				// Enable rs232 driver and receiver (Active low controls)
				gpio_clr_gpio_pin(AVR32_PIN_PB08);
				gpio_clr_gpio_pin(AVR32_PIN_PB09);
			}
#endif
		}
	}    
	// End of NS8100 Main
#endif

	// End of the world
	return (0);
}
