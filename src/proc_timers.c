////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   on_off_key_test_menu.c                                                   //
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
//   $Date: 2009/11/06 01:42:15 $                                                                 //
//   $Revision: 1.6 $                                                             //
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
#include "tc.h"
#include "RealTimeClock.h"
#include "intc.h"
#include "proc_timers.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "twi.h"
#include "m23018.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define TC_CHANNEL    0
extern void rtc_clear_interrupt(volatile avr32_rtc_t *rtc);
extern int rtc_init(volatile avr32_rtc_t *rtc, unsigned char osc_type, unsigned char psel);
extern void rtc_set_top_value(volatile avr32_rtc_t *rtc, unsigned long top);
extern void rtc_enable_interrupt(volatile avr32_rtc_t *rtc);
extern void rtc_enable(volatile avr32_rtc_t *rtc);

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Function Prototypes                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#if 0
static void twi_resources_init(void)
{
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
}
#endif

void Proc_Timers_Test_Menu(void)
{
   Menu_Items = (sizeof(Proc_Timers_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Proc_Timers_Test_Menu_Functions;
   Menu_String = (unsigned char *)Proc_Timers_Test_Menu_Text;
}

void Proc_Timers_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

volatile static int print_sec = 1;
volatile unsigned long int tc_tick=0;
static int sec = 0;

#if __GNUC__
__attribute__((__interrupt__)) void tc_irq_1( void )
#elif __ICCAVR32__
/* TC Interrupt  */
#pragma handler = AVR32_TC_IRQ_GROUP, 1
__interrupt void tc_irq( void )
#endif
{
  // Increment the ms seconds counter
  tc_tick++;

  // clear the interrupt flag
  AVR32_TC.channel[TC_CHANNEL].sr;

  // specify that an interrupt has been raised
  print_sec = 1;
}

void Proc_Timers_PiT_1(void)
{
   int uart_rx_character = 0;
	volatile avr32_tc_t *tc = &AVR32_TC;

	print_dbg("Proc Timer 1 Test:\n\r");

	// Options for waveform genration.
	tc_waveform_opt_t WAVEFORM_OPT =
	{
	.channel  = TC_CHANNEL,                        // Channel selection.

	.bswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOB.
	.beevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOB.
	.bcpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOB.
	.bcpb     = TC_EVT_EFFECT_NOOP,                // RB compare effect on TIOB.

	.aswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOA.
	.aeevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOA.
	.acpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOA: toggle.
	.acpa     = TC_EVT_EFFECT_NOOP,                // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).

	.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
	.enetrg   = FALSE,                             // External event trigger enable.
	.eevt     = 0,                                 // External event selection.
	.eevtedg  = TC_SEL_NO_EDGE,                    // External event edge selection.
	.cpcdis   = FALSE,                             // Counter disable when RC compare.
	.cpcstop  = FALSE,                             // Counter clock stopped with RC compare.

	.burst    = FALSE,                             // Burst signal selection.
	.clki     = FALSE,                             // Clock inversion.
	.tcclks   = TC_CLOCK_SOURCE_TC2                // Internal source clock 2 - connected to PBA/4
	};

	// Options for interrupt
	tc_interrupt_t TC_INTERRUPT =
	{
	.etrgs = 0,
	.ldrbs = 0,
	.ldras = 0,
	.cpcs  = 1,
	.cpbs  = 0,
	.cpas  = 0,
	.lovrs = 0,
	.covfs = 0
	};

#if 0 // Switch main clock
	// Switch main clock to external oscillator 0 (crystal) : 12MHz
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
#endif

#if 0 // USART config and startup
	static const gpio_map_t USART_GPIO_MAP =
	{
	{AVR32_USART1_RXD_0_PIN, AVR32_USART1_RXD_0_FUNCTION},
	{AVR32_USART1_TXD_0_PIN, AVR32_USART1_TXD_0_FUNCTION}
	};

	// USART options.
	static const usart_options_t USART_OPTIONS =
	{
	.baudrate     = 57600,
	.charlength   = 8,
	.paritytype   = USART_NO_PARITY,
	.stopbits     = USART_1_STOPBIT,
	.channelmode  = 0
	};

	// Assign GPIO pins to USART1.
	gpio_enable_module(USART_GPIO_MAP, sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

	// Initialize USART in RS232 mode at 12MHz.
	usart_init_rs232(&AVR32_USART1, &USART_OPTIONS, FPBA);

	// Clear a VT100 terminal screen
	usart_write_line(&AVR32_USART1, "\x1B[2J");
	// Welcome sentence
	usart_write_line(&AVR32_USART1, "ATMEL\r\n");
	usart_write_line(&AVR32_USART1, "AVR32 UC3 - TC example\r\n");
#endif

	Disable_global_interrupt();

	// The INTC driver has to be used only for GNU GCC for AVR32.
#if __GNUC__
	// Done in main
	// Initialize interrupt vectors.
	//INTC_init_interrupts();

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&tc_irq_1, AVR32_TC_IRQ0, 0);
#endif

	Enable_global_interrupt();

	// Initialize the timer/counter.
	tc_init_waveform(tc, &WAVEFORM_OPT);         // Initialize the timer/counter waveform.

	// Set the compare triggers.
	// Remember TC counter is 16-bits, so counting second is not possible.
	// We configure it to count ms.
	// We want: (1/(FPBA/4)) * RC = 1000 Hz => RC = (FPBA/4) / 1000 = 3000 to get an interrupt every 1ms
	tc_write_rc(tc, TC_CHANNEL, (FOSC0/2)/1000);  // Set RC value.

	tc_configure_interrupts(tc, TC_CHANNEL, &TC_INTERRUPT);

	// Start the timer/counter.
	tc_start(tc, TC_CHANNEL);                    // And start the timer/counter.

	tc_tick = 0;
	while((usart_read_char(DBG_USART, &uart_rx_character))!= USART_SUCCESS)
	{
		// Update the display on USART every second.
		if ((print_sec) && (!(tc_tick % 1000)))
		{
			print_dbg_ulong(tc_tick);
			print_dbg("\r");
		}
	}
}

//#if __GNUC__
__attribute__((__interrupt__))
//#elif __ICCAVR32__
/* RTC Interrupt  */
//#pragma handler = RTC_INT_GROUP, 1
//__interrupt
//#endif
void rtc_irq_2(void)
{
  // Increment the counter
  sec++;

  // clear the interrupt flag
  rtc_clear_interrupt(&AVR32_RTC);

  // specify that an interrupt has been raised
  print_sec = 1;
}

void Proc_Timers_PiT_2(void)
{
   int uart_rx_character = 0;

   print_dbg("Proc Timer 2 Test:\n\r\n\r");

	// Disable all interrupts. */
	Disable_global_interrupt();

	// The INTC driver has to be used only for GNU GCC for AVR32.
#if __GNUC__
	// Done in main
	// Initialize interrupt vectors.
	//INTC_init_interrupts();

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&rtc_irq_2, AVR32_RTC_IRQ, 0);
#endif

	// Initialize the RTC
	if (!rtc_init(&AVR32_RTC, 1, 0))
	{
		print_dbg("Error initializing the RTC\r\n");
		while(1);
	}
	// Set top value to 0 to generate an interrupt every second */
	rtc_set_top_value(&AVR32_RTC, 8192);
	// Enable the interruptions
	rtc_enable_interrupt(&AVR32_RTC);
	// Enable the RTC
	rtc_enable(&AVR32_RTC);

	// Enable global interrupts
	Enable_global_interrupt();

	sec = 0;
	while((usart_read_char(DBG_USART, &uart_rx_character))!= USART_SUCCESS)
	{
		if (print_sec)
		{
			print_dbg_ulong(sec / 120);
			print_dbg(":");
			print_dbg_ulong((sec / 2) % 60);
			print_dbg(" (");
			print_dbg_ulong(sec);
			print_dbg(")");
			print_dbg("\r");
			print_sec = 0;
		}
	}
}

void Proc_Timers_Test(void)
{
   print_dbg("Proc General Test:\n\r");
}

//=== Leech snippets =========================================================================================
#if 0
void (void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {

	  print_dbg("On Key = ");

	  state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x04);
      if(state==0)
      {
	     print_dbg("On \r");
      }
      else
      {
	     print_dbg("Off\r");
      }
   }
   print_dbg("\n\r\n\r");
}

void On_Off_Key_On_Menu_Read_Once(void)
{
   int state;

   print_dbg("\n\rOn Key = ");

   state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x04);
   if(state==0)
   {
      print_dbg("On\n\r");
   }
   else
   {
      print_dbg("Off\n\r");
   }
   print_dbg("\n\r");
}

void On_Off_Key_Off_Menu_Read_Continuous(void)
{
   int state;
   int *uart_rx_character = 0;

   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {

	  print_dbg("Off Key = ");

	  state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x02);
      if(state==0)
      {
	     print_dbg("On \r");
      }
      else
      {
	     print_dbg("Off\r");
      }
   }
   print_dbg("\n\r\n\r");
}
void On_Off_Key_Off_Menu_Read_Once(void)
{
   int state;

   print_dbg("\n\rOff Key = ");

   state = (read_mcp23018(IO_ADDRESS_KPD, GPIOA) & 0x02);
   if(state==0)
   {
      print_dbg("On\n\r");
   }
   else
   {
      print_dbg("Off\n\r");
   }
   print_dbg("\n\r");
}
#endif

//=== Example 3 ==============================================================================================
#if 0
#include <avr32/io.h>
#if __GNUC__
#  include "intc.h"
#endif
#include "compiler.h"
#include "board.h"
#include "pm.h"
#include "gpio.h"
#include "tc.h"
#include "usart.h"


#define FPBA    FOSC0
#define TC_CHANNEL    0

// To specify we have to print a new time
volatile static int print_sec = 1;

volatile U32 tc_tick=0;

#if __GNUC__
__attribute__((__interrupt__)) void tc_irq( void )
#elif __ICCAVR32__
/* TC Interrupt  */
#pragma handler = AVR32_TC_IRQ_GROUP, 1
__interrupt void tc_irq( void )
#endif

{
  // Increment the ms seconds counter
  tc_tick++;

  // clear the interrupt flag
  AVR32_TC.channel[TC_CHANNEL].sr;

  // specify that an interrupt has been raised
  print_sec = 1;
  // Toggle GPIO pin 0 (this pin is used as a regular GPIO pin).
  gpio_tgl_gpio_pin(0); // Toggle GPIO pin 0 (this pin is used as a regular GPIO pin).
}

/*!
 * \brief print_i function : convert the given number to an ASCII decimal representation.
 */
char *print_i(char *str, int n)
{
    int i = 10;

    str[i] = '\0';
    do
    {
      str[--i] = '0' + n%10;
      n /= 10;
    }while(n);

    return &str[i];
}

/*! \brief Main function. Execution starts here.
 */
int main(void)
{
  char temp[20];
  char *ptemp;

  volatile avr32_tc_t *tc = &AVR32_TC;

  // Options for waveform genration.
  static const tc_waveform_opt_t WAVEFORM_OPT =
  {
    .channel  = TC_CHANNEL,                        // Channel selection.

    .bswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOB.
    .beevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOB.
    .bcpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOB.
    .bcpb     = TC_EVT_EFFECT_NOOP,                // RB compare effect on TIOB.

    .aswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOA.
    .aeevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOA.
    .acpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOA: toggle.
    .acpa     = TC_EVT_EFFECT_NOOP,                // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).

    .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
    .enetrg   = FALSE,                             // External event trigger enable.
    .eevt     = 0,                                 // External event selection.
    .eevtedg  = TC_SEL_NO_EDGE,                    // External event edge selection.
    .cpcdis   = FALSE,                             // Counter disable when RC compare.
    .cpcstop  = FALSE,                             // Counter clock stopped with RC compare.

    .burst    = FALSE,                             // Burst signal selection.
    .clki     = FALSE,                             // Clock inversion.
    .tcclks   = TC_CLOCK_SOURCE_TC2                // Internal source clock 2 - connected to PBA/4
  };

  static const tc_interrupt_t TC_INTERRUPT =
  {
    .etrgs = 0,
    .ldrbs = 0,
    .ldras = 0,
    .cpcs  = 1,
    .cpbs  = 0,
    .cpas  = 0,
    .lovrs = 0,
    .covfs = 0
  };

  // Switch main clock to external oscillator 0 (crystal) : 12MHz
  pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

  static const gpio_map_t USART_GPIO_MAP =
  {
    {AVR32_USART1_RXD_0_PIN, AVR32_USART1_RXD_0_FUNCTION},
    {AVR32_USART1_TXD_0_PIN, AVR32_USART1_TXD_0_FUNCTION}
  };

  // USART options.
  static const usart_options_t USART_OPTIONS =
  {
    .baudrate     = 57600,
    .charlength   = 8,
    .paritytype   = USART_NO_PARITY,
    .stopbits     = USART_1_STOPBIT,
    .channelmode  = 0
  };

  // Assign GPIO pins to USART1.
  gpio_enable_module(USART_GPIO_MAP, sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

  // Initialize USART in RS232 mode at 12MHz.
  usart_init_rs232(&AVR32_USART1, &USART_OPTIONS, FPBA);

  // Clear a VT100 terminal screen
  usart_write_line(&AVR32_USART1, "\x1B[2J");
  // Welcome sentence
  usart_write_line(&AVR32_USART1, "ATMEL\r\n");
  usart_write_line(&AVR32_USART1, "AVR32 UC3 - TC example\r\n");

  Disable_global_interrupt();

  // The INTC driver has to be used only for GNU GCC for AVR32.
#if __GNUC__
  // Done in main
  // Initialize interrupt vectors.
  //INTC_init_interrupts();

  // Register the RTC interrupt handler to the interrupt controller.
  INTC_register_interrupt(&tc_irq, AVR32_TC_IRQ0, INT0);
#endif

  Enable_global_interrupt();

  // Initialize the timer/counter.
  tc_init_waveform(tc, &WAVEFORM_OPT);         // Initialize the timer/counter waveform.

  // Set the compare triggers.
  // Remember TC counter is 16-bits, so counting second is not possible.
  // We configure it to count ms.
  // We want: (1/(FPBA/4)) * RC = 1000 Hz => RC = (FPBA/4) / 1000 = 3000 to get an interrupt every 1ms
  tc_write_rc(tc, TC_CHANNEL, (FPBA/4)/1000);  // Set RC value.

  tc_configure_interrupts(tc, TC_CHANNEL, &TC_INTERRUPT);

  // Start the timer/counter.
  tc_start(tc, TC_CHANNEL);                    // And start the timer/counter.

  while(1)
  {
    // Update the display on USART every second.
    if ((print_sec) && (!(tc_tick%1000)))
    {
      // Set cursor to the position (1; 5)
      usart_write_line(&AVR32_USART1, "\x1B[5;1H");
      ptemp = print_i(temp, tc_tick);
      usart_write_line(&AVR32_USART1, "Timer: ");
      usart_write_line(&AVR32_USART1, ptemp);
      usart_write_line(&AVR32_USART1, " ms");
      print_sec = 0;
    }
  }
}
#endif

//=== Example ================================================================================================
#if 0
#include <avr32/io.h>
#if __GNUC__
#  include "intc.h"
#endif
#include "board.h"
#include "compiler.h"
#include "RealTimeClock.h"
#include "usart.h"
#include "gpio.h"
#include "pm.h"

/*! \name USART Settings
 */
//! @{
#if BOARD == EVK1100
#  define EXAMPLE_USART               (&AVR32_USART1)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART1_RXD_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART1_RXD_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART1_TXD_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART1_TXD_0_FUNCTION
#elif BOARD == EVK1101
#  define EXAMPLE_USART               (&AVR32_USART1)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART1_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART1_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART1_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART1_TXD_0_0_FUNCTION
#endif
//! @}

// To specify we have to print a new time
volatile static int print_sec = 1;

// Time counter
static int sec = 0;

#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
/* RTC Interrupt  */
#pragma handler = RTC_INT_GROUP, 1
__interrupt
#endif
void rtc_irq(void)
{
  // Increment the minutes counter
  sec++;

  // clear the interrupt flag
  rtc_clear_interrupt(&AVR32_RTC);

  // specify that an interrupt has been raised
  print_sec = 1;
}

/*!
 * \brief print_i function : convert the given number to an ASCII decimal representation.
 */
char *print_i(char *str, int n)
{
  int i = 10;

  str[i] = '\0';
  do
  {
    str[--i] = '0' + n%10;
    n /= 10;
  }while(n);

  return &str[i];
}

/*!
 * \brief main function : do init and loop (poll if configured so)
 */
int main(int argc, char **argv)
{
  char temp[20];
  char *ptemp;
  pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

  static const gpio_map_t USART_GPIO_MAP =
  {
    {EXAMPLE_USART_RX_PIN, EXAMPLE_USART_RX_FUNCTION},
    {EXAMPLE_USART_TX_PIN, EXAMPLE_USART_TX_FUNCTION}
  };

  // USART options
  static const usart_options_t USART_OPTIONS =
  {
    .baudrate     = 57600,
    .charlength   = 8,
    .paritytype   = USART_NO_PARITY,
    .stopbits     = USART_1_STOPBIT,
    .channelmode  = 0
  };

  // Assign GPIO pins to USART0.
  gpio_enable_module(USART_GPIO_MAP,
                     sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

  // Initialize USART in RS232 mode at 12MHz.
  usart_init_rs232(EXAMPLE_USART, &USART_OPTIONS, 12000000);

  // Clear a VT100 terminal screen
  usart_write_line(EXAMPLE_USART, "\x1B[2J");
  // Welcome sentence
  usart_write_line(EXAMPLE_USART, "ATMEL\r\n");
  usart_write_line(EXAMPLE_USART, "AVR32 UC3 - RTC example\r\n");

  usart_write_line(EXAMPLE_USART, "RTC 32 KHz oscillator program test.\r\n");

  // Disable all interrupts. */
  Disable_global_interrupt();

  // The INTC driver has to be used only for GNU GCC for AVR32.
#if __GNUC__
  // Initialize interrupt vectors.
  INTC_init_interrupts();

  // Register the RTC interrupt handler to the interrupt controller.
  INTC_register_interrupt(&rtc_irq, AVR32_RTC_RTC_IRQ, INT0);
#endif

  // Initialize the RTC
  if (!rtc_init(&AVR32_RTC, RTC_OSC_32KHZ, RTC_PSEL_32KHZ_1HZ))
  {
    usart_write_line(&AVR32_USART0, "Error initializing the RTC\r\n");
    while(1);
  }
  // Set top value to 0 to generate an interruption every seconds */
  rtc_set_top_value(&AVR32_RTC, 0);
  // Enable the interruptions
  rtc_enable_interrupt(&AVR32_RTC);
  // Enable the RTC
  rtc_enable(&AVR32_RTC);

  // Enable global interrupts
  Enable_global_interrupt();

  while(1)
  {
    if (print_sec)
    {
      // Set cursor to the position (1; 5)
      usart_write_line(EXAMPLE_USART, "\x1B[5;1H");
      ptemp = print_i(temp, sec);
      usart_write_line(EXAMPLE_USART, "Timer: ");
      usart_write_line(EXAMPLE_USART, ptemp);
      usart_write_line(EXAMPLE_USART, "s");
      print_sec = 0;
    }
  }
}
#endif
