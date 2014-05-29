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
#include "lcd.h"
#include <stdio.h>

// Added in NS7100 includes
#include <stdlib.h>
#include <string.h>
#include "Typedefs.h"
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
#include "rtc.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
#define AVR32_IDLE_MODE		AVR32_PM_SMODE_IDLE
#define PBA_HZ				FOSC0
#define ONE_MS_RESOLUTION	1000

#define AVR32_WDT_KEY_VALUE_ASSERT		0x55000000
#define AVR32_WDT_KEY_VALUE_DEASSERT	0xAA000000
#define AVR32_WDT_DISABLE_VALUE			0x00000000

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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Avr32_enable_muxed_pins(void)
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
		
#if 1
		// EIC
		{AVR32_EIC_EXTINT_4_PIN, AVR32_EIC_EXTINT_4_FUNCTION},
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitProcessorNoConnectPins(void)
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

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void Avr32_chip_select_init(unsigned long hsb_hz)
{
	unsigned long int hsb_mhz_up = (hsb_hz + 999999) / 1000000;

	// Setup all 4 chip selects
	SMC_CS_SETUP(0)			// LCD
	SMC_CS_SETUP(1)			// External RAM
	SMC_CS_SETUP_SPECIAL(2)	// Network/LAN
	SMC_CS_SETUP(3)			// Network/LAN Memory

	// Put the multiplexed MCU pins used for the SM under control of the SMC.
	Avr32_enable_muxed_pins();
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void _init_startup(void)
{
	// Disable watchdog if reset from bootloader
	AVR32_WDT.ctrl = (AVR32_WDT_KEY_VALUE_ASSERT | AVR32_WDT_DISABLE_VALUE);
	AVR32_WDT.ctrl = (AVR32_WDT_KEY_VALUE_DEASSERT | AVR32_WDT_DISABLE_VALUE);
	
#if 0 // Test external 12 MHz oscillator
	pm_enable_osc0_ext_clock(&AVR32_PM);
#endif

	// Switch the main clock to the external oscillator 0 (12 MHz)
#if 0 // Normal
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
#else // Test
	pm_switch_to_osc0(&AVR32_PM, FOSC0, AVR32_PM_OSCCTRL0_STARTUP_0_RCOSC);
#endif

#if 1 // Normal (Set clock to 66 MHz)
	// Logic to change the clock source to PLL 0
	// PLL = 0, Multiplier = 10 (actual 11), Divider = 1 (actually 1), OSC = 0, 16 clocks to stabilize
	pm_pll_setup(&AVR32_PM, 0, 10, 1, 0, 16);
	pm_pll_set_option(&AVR32_PM, 0, 1, 1, 0); // PLL = 0, Freq = 1, Div2 = 1

	// Enable and lock
	pm_pll_enable(&AVR32_PM, 0);
	pm_wait_for_pll0_locked(&AVR32_PM);

#if 0 // Left over code for setting up the main clock
	pm_gc_setup(&AVR32_PM, 0, 1, 0, 0, 0);
	pm_gc_enable(&AVR32_PM, 0);
	gpio_enable_module_pin(AVR32_PM_GCLK_0_1_PIN, AVR32_PM_GCLK_0_1_FUNCTION);
#endif

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
	Avr32_chip_select_init(FOSC0);
#else // Test (12Mhz)
	Avr32_chip_select_init(12000000);
#endif
	
	// Disable the unused and non connected clock 1
	pm_disable_clk1(&AVR32_PM);

	// With clock 1 disabled, configure GPIO lines to be outputs and low
	gpio_clr_gpio_pin(AVR32_PM_XIN1_0_PIN);
	gpio_clr_gpio_pin(AVR32_PM_XOUT1_0_PIN);

#if 0 // Test
	pm_disable_clk32(&AVR32_PM);
	
	gpio_clr_gpio_pin(AVR32_PM_XIN32_0_PIN);
	gpio_clr_gpio_pin(AVR32_PM_XOUT32_0_PIN);
#endif

	SoftUsecWait(1000);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitKeypad(void)
{
#if 0
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

	// TWI gpio pins configuration
	gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));
#endif

	// TWI options.
	twi_options_t opt;

	// options settings
	opt.pba_hz = FOSC0;
	opt.speed = TWI_SPEED;
	opt.chip = IO_ADDRESS_KPD;

	// initialize TWI driver with options
	if (twi_master_init(&AVR32_TWI, &opt) != TWI_SUCCESS)
	{
		debugErr("Two Wire Interface failed to initialize!\n");
	}

	SoftUsecWait(25 * SOFT_MSECS);
	InitMcp23018(IO_ADDRESS_KPD);
	SoftUsecWait(25 * SOFT_MSECS);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitPullupsOnFloatingIOLines(void)
{
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
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitSerial485(void)
{
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
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitSerial232(void)
{
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
	GetRecordData(&g_helpRecord, DEFAULT_RECORD, REC_HELP_USER_MENU_TYPE);

	// Check if the Help Record is valid
	if (g_helpRecord.validationKey == 0xA5A5)
	{
		// Set the baud rate to the user stored baud rate setting (initialized to 115200)
		switch (g_helpRecord.baudRate)
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
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitLANToSleep(void)
{
#if 1 // Normal
extern void Sleep8900(void);
extern void Sleep8900_LedOn(void);
	gpio_clr_gpio_pin(AVR32_PIN_PB27); // Clear LAN Sleep pin (active low control)
	SoftUsecWait(10 * SOFT_MSECS);

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
	SoftUsecWait(250 * SOFT_MSECS);
	ToggleLedOff8900();
	SoftUsecWait(250 * SOFT_MSECS);

	ToggleLedOn8900();
	SoftUsecWait(250 * SOFT_MSECS);
	ToggleLedOff8900();
	SoftUsecWait(250 * SOFT_MSECS);

	//Sleep8900_LedOn();
	Sleep8900();
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitExternalKeypad(void)
{
	InitKeypad();
	
	// Primer read
	uint8 keyScan = ReadMcp23018(IO_ADDRESS_KPD, GPIOB);
	if (keyScan)
	{
		debugWarn("Keypad key being pressed, likely a bug. Key: %x", keyScan);
	}

	// Turn on the red keypad LED while loading
	WriteMcp23018(IO_ADDRESS_KPD, GPIOA, ((ReadMcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | RED_LED_PIN));

}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitInternalRTC(void)
{
	rtc_init(&AVR32_RTC, 1, 0);

	// Set top value to generate an interrupt every 1/2 second
	rtc_set_top_value(&AVR32_RTC, 8192);

	// Enable the Internal RTC
	rtc_enable(&AVR32_RTC);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitInternalAD(void)
{
	adc_configure(&AVR32_ADC);

	// Enable the A/D channels.
#if 0 // Can't use the driver enable because it's a single channel enable only and a write only register
	//adc_enable(&AVR32_ADC, VBAT_CHANNEL);
	//adc_enable(&AVR32_ADC, VIN_CHANNEL);
#else
	AVR32_ADC.cher = 0x0C; // Directly enable
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitSDAndFileSystem(void)
{
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
	SoftUsecWait(10 * SOFT_MSECS);

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
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitExternalAD(void)
{
	// Enable the A/D
	debug("Enable the A/D\n");
	PowerControl(ANALOG_SLEEP_ENABLE, OFF);

	// Delay to allow AD to power up/stabilize
	SoftUsecWait(50 * SOFT_MSECS);

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
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp On)\n");
	SetupADChannelConfig(0x39DC);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Temp Off)\n");
	SetupADChannelConfig(0x39F4);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp Off)\n");
	SetupADChannelConfig(0x39FC);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);
	//----------------------------------------------------------------------------------
	
	//------------------------------Loop 2----------------------------------------------
	debug("Setup A/D config and channels (External Ref, Temp On)\n");
	SetupADChannelConfig(0x39D4);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp On)\n");
	SetupADChannelConfig(0x39DC);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Temp Off)\n");
	SetupADChannelConfig(0x39F4);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);

	debug("Setup A/D config and channels (External Ref, Internal Buffer, Temp Off)\n");
	SetupADChannelConfig(0x39FC);
	SoftUsecWait(50 * SOFT_MSECS);
	GetAnalogConfigReadback();
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	ReadAnalogData(&dummySample); debug("A/D Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", dummySample.a, dummySample.r, dummySample.v, dummySample.t, g_currentTempReading);
	GetChannelOffsets(SAMPLE_RATE_DEFAULT);
	//----------------------------------------------------------------------------------
#endif

	debug("Disable the A/D\n");
	PowerControl(ANALOG_SLEEP_ENABLE, OFF);
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void TestPowerDownAndStop(void)
{
	// Turn off the keypad LED
	WriteMcp23018(IO_ADDRESS_KPD, GPIOA, ((ReadMcp23018(IO_ADDRESS_KPD, GPIOA) & 0xCF) | NO_LED_PINS));

	spi_reset(&AVR32_SPI1);
	gpio_clr_gpio_pin(AVR32_SPI1_MISO_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_MOSI_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_NPCS_3_PIN);

	debug("\nClosing up shop.\n\n");

	// Disable rs232 driver and receiver (Active low controls)
	gpio_set_gpio_pin(AVR32_PIN_PB08);
	gpio_set_gpio_pin(AVR32_PIN_PB09);
	
	//DisplayTimerCallBack();
	SetLcdBacklightState(BACKLIGHT_OFF);

	//LcdPwTimerCallBack();
	PowerControl(LCD_CONTRAST_ENABLE, OFF);
	ClearLcdDisplay();
	ClearControlLinesLcdDisplay();
	LcdClearPortReg();
	PowerControl(LCD_POWER_ENABLE, OFF);

	gpio_clr_gpio_pin(AVR32_PIN_PB20);

	//SLEEP(AVR32_PM_SMODE_IDLE);
	SLEEP(AVR32_PM_SMODE_STOP);
	//SLEEP(AVR32_PM_SMODE_STANDBY);
	while (1) {}
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void KillClocksToModules(void)
{
	// Leave active: SYSTIMER; Disable: OCD
	AVR32_PM.cpumask = 0x0100;
	
	// Leave active: EBI, PBA & PBB BRIDGE, FLASHC; Disable: PDCA, MACB, USBB
	AVR32_PM.hsbmask = 0x0047;
	
	// Leave active: TC, TWI, SPI0, SPI1, ADC, PM/RTC/EIC, GPIO, INTC; Disable: ABDAC, SSC, PWM, USART 0 & 1 & 2 & 3, PDCA
	AVR32_PM.pbamask = 0x40FB;
	
	// Leave active: SMC, FLASHC, HMATRIX; Disable: SDRAMC, MACB, USBB
	AVR32_PM.pbbmask = 0x0015;

#if 0 // Enable USART1 for debug
	AVR32_PM.pbamask = 0x42FB;
#endif

#if 0 // Test
	// Disable rs232 driver and receiver (Active low controls)
	gpio_set_gpio_pin(AVR32_PIN_PB08);
	gpio_set_gpio_pin(AVR32_PIN_PB09);
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void InitSystemHardware_NS8100(void)
{
	//-------------------------------------------------------------------------
	// Enable internal pull ups on the floating data lines
	//-------------------------------------------------------------------------
	InitPullupsOnFloatingIOLines();
	
	//-------------------------------------------------------------------------
	// Clock and chip selects setup in custom _init_startup
	//-------------------------------------------------------------------------
	InitProcessorNoConnectPins();
	
	//-------------------------------------------------------------------------
	// Set RTC Timestamp pin high (Active low signal)
	//-------------------------------------------------------------------------
	PowerControl(RTC_TIMESTAMP, OFF); //gpio_set_gpio_pin(AVR32_PIN_PB18);

	//-------------------------------------------------------------------------
	// Set Alarm 1 and Alarm 2 low (Active high signal)
	//-------------------------------------------------------------------------
	PowerControl(ALARM_1_ENABLE, OFF); //gpio_clr_gpio_pin(AVR32_PIN_PB06);
	PowerControl(ALARM_2_ENABLE, OFF); //gpio_clr_gpio_pin(AVR32_PIN_PB07);

	//-------------------------------------------------------------------------
	// Set Trigger Out low (Active high signal)
	//-------------------------------------------------------------------------
	PowerControl(TRIGGER_OUT, OFF); //gpio_clr_gpio_pin(AVR32_PIN_PB05);

	//-------------------------------------------------------------------------
	// Set USB LED Output low (Active high signal)
	//-------------------------------------------------------------------------
	PowerControl(USB_LED, OFF); //gpio_clr_gpio_pin(AVR32_PIN_PB28);

	//-------------------------------------------------------------------------
	// Smart Sensor data in (Hardware pull up on signal)
	//-------------------------------------------------------------------------
	// Nothing needs to be done. Pin should default to an input on power up
	//gpio_enable_gpio_pin(AVR32_PIN_PB01)

	//-------------------------------------------------------------------------
	// Set SDATA and ADATA high
	//-------------------------------------------------------------------------
	PowerControl(SDATA, OFF); //gpio_set_gpio_pin(AVR32_PIN_PB02);
	PowerControl(ADATA, OFF); //gpio_set_gpio_pin(AVR32_PIN_PB03);

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
	InitSerial485();
	
	// Make sure 485 lines aren't floating
	gpio_enable_pin_pull_up(AVR32_USART3_RXD_0_0_PIN);
	gpio_enable_pin_pull_up(AVR32_USART3_TXD_0_0_PIN);
	gpio_enable_pin_pull_up(AVR32_USART3_RTS_0_1_PIN);

	//-------------------------------------------------------------------------
	// Turn on rs232 driver and receiver (Active low controls)
	//-------------------------------------------------------------------------
	PowerControl(SERIAL_232_DRIVER_ENABLE, ON); //gpio_clr_gpio_pin(AVR32_PIN_PB08);
	PowerControl(SERIAL_232_RECEIVER_ENABLE, ON); //gpio_clr_gpio_pin(AVR32_PIN_PB09);
	InitSerial232();
	
	//-------------------------------------------------------------------------
	// Initialize the external RTC
	//-------------------------------------------------------------------------
	InitExternalRtc();

	//-------------------------------------------------------------------------
	// Set LAN to Sleep
	//-------------------------------------------------------------------------
	InitLANToSleep();

	//-------------------------------------------------------------------------
	// Initialize the AD Control
	//-------------------------------------------------------------------------
	InitAnalogControl();

	//-------------------------------------------------------------------------
	// Init the LCD display
	//-------------------------------------------------------------------------
	PowerControl(LCD_CONTRAST_ENABLE, ON); //gpio_set_gpio_pin(AVR32_PIN_PB22);
	PowerControl(LCD_POWER_ENABLE, ON); //gpio_set_gpio_pin(AVR32_PIN_PB21);
	Backlight_On();
	Backlight_High();
	Set_Contrast(24);
	InitDisplay();

	//-------------------------------------------------------------------------
	// Initialize the Internal Real Time Counter for half second tick used for state processing
	//-------------------------------------------------------------------------
	InitInternalRTC();

	//-------------------------------------------------------------------------
	// Enable Processor A/D
	//-------------------------------------------------------------------------
	InitInternalAD();

	//-------------------------------------------------------------------------
	// Power on the SD Card and init the file system
	//-------------------------------------------------------------------------
	InitSDAndFileSystem();

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
#if 0 // Moved to software init becasue the MCP23018 doesn't like to be init'ed here
	InitExternalKeypad();
#endif

	//-------------------------------------------------------------------------
	// Init External RTC Interrupt for interrupt source
	//-------------------------------------------------------------------------
#if EXTERNAL_SAMPLING_SOURCE
	// The internal pull up for this pin needs to be enables to bring the line high, otherwise the clock out will only reach half level
	gpio_enable_pin_pull_up(AVR32_EIC_EXTINT_1_PIN);
#endif

	//-------------------------------------------------------------------------
	// Enable pullups on input pins that may be floating
	//-------------------------------------------------------------------------
	// USB ID
	gpio_enable_pin_pull_up(AVR32_PIN_PA21);

	// RS232
	gpio_enable_pin_pull_up(AVR32_USART1_RXD_0_0_PIN);
	gpio_enable_pin_pull_up(AVR32_USART1_DCD_0_PIN);
	gpio_enable_pin_pull_up(AVR32_USART1_DSR_0_PIN);
	gpio_enable_pin_pull_up(AVR32_USART1_CTS_0_0_PIN);
	gpio_enable_pin_pull_up(AVR32_USART1_RI_0_PIN);

#if 0 // Current went up with this change
	// Crystal inputs
	gpio_enable_pin_pull_up(AVR32_PIN_PC00);
	gpio_enable_pin_pull_up(AVR32_PIN_PC01);
	gpio_enable_pin_pull_up(AVR32_PIN_PC02);
	gpio_enable_pin_pull_up(AVR32_PIN_PC03);
	gpio_enable_pin_pull_up(AVR32_PIN_PC04);
	gpio_enable_pin_pull_up(AVR32_PIN_PC05);
#endif

	//-------------------------------------------------------------------------
	// Init and configure the A/D to prevent the unit from burning current charging internal reference (default config)
	InitExternalAD();

#if 0 // Test
	//-------------------------------------------------------------------------
	// Kill clocks to Internal Processor modules that aren't absolutely necessary
	KillClocksToModules();
#endif

	//-------------------------------------------------------------------------
	// Set the power savings mode based on the saved setting
	AdjustPowerSavings();

#if 0
	//-------------------------------------------------------------------------
	// Test full power down and stop
	TestPowerDownAndStop();
#endif
}

///----------------------------------------------------------------------------
///	Function Break
///----------------------------------------------------------------------------
void PowerDownAndHalt(void)
{
	// Enable the A/D
	debug("Enable the A/D\n");
	PowerControl(ANALOG_SLEEP_ENABLE, OFF);

	// Delay to allow AD to power up/stabilize
	SoftUsecWait(50 * SOFT_MSECS);

	debug("Setup A/D config and channels\n");
	// Setup the A/D Channel configuration
	extern void SetupADChannelConfig(uint32 sampleRate);
	SetupADChannelConfig(1024);

	debug("Disable the A/D\n");
	PowerControl(ANALOG_SLEEP_ENABLE, OFF);

	spi_reset(&AVR32_SPI1);

	gpio_clr_gpio_pin(AVR32_SPI1_MISO_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_MOSI_0_0_PIN);
	gpio_clr_gpio_pin(AVR32_SPI1_NPCS_3_PIN);

	debug("\nClosing up shop.\n\n");

	// Disable rs232 driver and receiver (Active low controls)
	gpio_set_gpio_pin(AVR32_PIN_PB08);
	gpio_set_gpio_pin(AVR32_PIN_PB09);
	
	//DisplayTimerCallBack();
	SetLcdBacklightState(BACKLIGHT_OFF);

	//LcdPwTimerCallBack();
	PowerControl(LCD_CONTRAST_ENABLE, OFF);
	ClearLcdDisplay();
	ClearControlLinesLcdDisplay();
	LcdClearPortReg();
	PowerControl(LCD_POWER_ENABLE, OFF);

	SLEEP(AVR32_PM_SMODE_STOP);
	
	while (1) {}
}
