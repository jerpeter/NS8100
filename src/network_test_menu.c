////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   network_test_menu.c                                                      //
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


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Include Files                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "craft.h"
#include "network_test_menu.h"
#include "print_funcs.h"
#include "usart.h"
#include "gpio.h"
#include "cs8900.h"                              // Ethernet packet driver
#include "tcpip.h"                               // TCP/IP stack


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Local Scope Variables                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define HTTP_SEND_PAGE               0x01        // help flag

const unsigned char WebSide1[] =
{
//"HTTP/1.0 200 OK\r\n"                          // protocol ver 1.0, code 200, reason OK
//"Content-Type: text/html\r\n"                  // type of data we want to send
//"\0 "                                         // indicate end of HTTP-header
"<html>"
"<head>"
"<meta http-equiv=Pragma content=no-cache>"
"<title>NS8100</title>"
"</style>"
"</head>"
"<basefont face=arial>"
"<body><center>"
"This Works!!<br>"
"NOMIS Seismographs.<br>"
"<basefont face=arial>"
"</head>"
"<table width=100% cellspacing=0 cellpadding=0 border=0 bgcolor=#0000FF>"
"<tr>"
"<td width=10% cellspacing=0 cellpadding=0 border=0 bgcolor=blue colspan=3 valign=center height=100>"
"</td>"
"<td width=90% cellspacing=0 cellpadding=0 border=0 bgcolor=blue colspan=3 valign=center height=100>"
"<font color=white face=Arial size=6><b>NS8100</b></font>"
"</td>"
"</tr>"
"</table>"
"<br>"
"</center></body>  "
};

const unsigned char WebSide[] = {
"<html>\r\n"
"<head>\r\n"
"<meta http-equiv=Pragma content=no-cache>\r\n"
"<title>NS8100</title>\r\n"
"<!--\r\n"
"</style>\r\n"

"<basefont face=arial>\r\n"
"</head>\r\n"
"<table width=100% cellspacing=0 cellpadding=0 border=0 bgcolor=#0000FF>\r\n"
"<tr>\r\n"
"<td width=10% cellspacing=0 cellpadding=0 border=0 bgcolor=blue colspan=3 valign=center height=100>\r\n"
"</td>\r\n"
"<td width=90% cellspacing=0 cellpadding=0 border=0 bgcolor=blue colspan=3 valign=center height=100>\r\n"
"<font color=white face=Arial size=6><b>NS8100</b></font>\r\n"
"</td>\r\n"
"</tr>\r\n"
"</table>\r\n"
"<br>\r\n"

"<br>\r\n"
"<basefont face=arial>\r\n"
"<table width=100% cellspacing=0 cellpadding=0 border=0 bgcolor=white>\r\n"

"<tr>\r\n"

"<td width=5% cellspacing=0 cellpadding=0 border=0 bgcolor=white colspan=1 valign=top height=50>\r\n"
"<font color=black face=Arial size=3>\r\n"
"</font>\r\n"
"</td>\r\n"

"<td width=95% cellspacing=0 cellpadding=0 border=0 bgcolor=white colspan=1 valign=top align=left>\r\n"
"<font size=4>\r\n"
"<br><br>\r\n"
"</font>\r\n"

"<p><font size=4>Seismograph Details:<br></font></p>\r\n"
"<ul>\r\n"
"<li><font size=4>Serial Number: 040506</font></li>\r\n"
"<li><font size=4>IP Address: " ASCII_MYIP_1 "." ASCII_MYIP_2 "." ASCII_MYIP_3 "." ASCII_MYIP_4 " </font></li>\r\n"
"<li><font size=4>MAC Address: 01-02-03-04-05-06</font></li>\r\n"
"<li><font size=4>Calibration Date: 2-03-2009</font></li>\r\n"
"<li><font size=4>Lock code: 2310</font></li>\r\n"
"<li><font size=4>Hardware Version: 1.12</font></li>\r\n"
"<li><font size=4>Software Version: 1.20.01</font></li>\r\n"
"<li><font size=4>Battery voltage: 6.23 volts</font></li>\r\n"
"<li><font size=4>Memory Capacity: 1G</font></li>\r\n"
"<li><font size=4>USB: Enabled</font></li>\r\n"
"<li><font size=4>Network: Enabled</font></li>\r\n"
"</ul>\r\n"

"<p><font size=4>Seismograph Settings:<br></font></p>\r\n"
"<ul>\r\n"
"<li><font size=4>Mode: Waveform</font></li>\r\n"
"<li><font size=4>Customer: Nomis Seismographs</font></li>\r\n"
"<li><font size=4>Operator: Bill Dawson</font></li>\r\n"
"<li><font size=4>Location: Denver, Colorado</font></li>\r\n"
"<li><font size=4>Notes: ISEE Show</font></li>\r\n"
"<li><font size=4>Distance to Source: 100 ft</font></li>\r\n"
"<li><font size=4>Weight per Delay: 100 lbs</font></li>\r\n"
"<li><font size=4>Sensitivity: Low</font></li>\r\n"
"<li><font size=4>Seismic Trigger: 0.125 inches</font></li>\r\n"
"<li><font size=4>Air Trigger: None</font></li>\r\n"
"<li><font size=4>Sample Rate: 1024</font></li>\r\n"
"<li><font size=4>Record Time: 2 seconds</font></li>\r\n"
"<li><font size=4>Monitor Bargraph: Both</font></li>\r\n"
"<li><font size=4>Bar Scale: 100%</font></li>\r\n"
"<li><font size=4>Bar Interval: 1 second</font></li>\r\n"
"<li><font size=4>Summary Interval: 5 minutes</font></li>\r\n"
"<li><font size=4>LCD Impulse Rate: 10 seconds</font></li>\r\n"
"</ul>\r\n"

"<p><font size=4>Geophone 1:<br></font></p>\r\n"
"<ul>\r\n"
"<li><font size=4>Type: 10.24in</font></li>\r\n"
"<li><font size=4>Calibration Date: 1-15-2009</font></li>\r\n"
"<li><font size=4>Serial Number: 00004519</font></li>\r\n"
"</ul>\r\n"

"<p><font size=4>Geophone 2:<br></font></p>\r\n"
"<ul>\r\n"
"<li><font size=4>None</font></li>\r\n"
"</ul>\r\n"

"<p><font size=4>Microphone 1:<br></font></p>\r\n"
"<ul>\r\n"
"<li><font size=4>Type: A-Weighted</font></li>\r\n"
"<li><font size=4>Calibration Date: 1-22-2009</font></li>\r\n"
"<li><font size=4>Serial Number: 00000318</font></li>\r\n"
"</ul>\r\n"

"<p><font size=4>Microphone 2:<br></font></p>\r\n"
"<ul>\r\n"
"<li><font size=4>None</font></li>\r\n"
"</ul>\r\n"

"<p align=center><font size=3>\r\n"
"Nomis Seismographs Inc.<br>\r\n"
"3728 4th Avenue South<br>\r\n"
"Birmingham, Alabama 35222<br>\r\n"
"Toll Free: 800-749-2477<br>\r\n"
"Telephone: 205-592-2488<br>\r\n"
"FAX: 205-592-2455<br>\r\n"
"</font></p>\r\n"

"</td></tr></table><br>\r\n"
"</html>\r\n"
};

const unsigned char GetResponse[] =              // 1st thing our server sends to a client
{
  "HTTP/1.0 200 OK\r\n "                          // protocol ver 1.0, code 200, reason OK
  "Content-Type: text/html\r\n "                  // type of data we want to send
  " "                                         // indicate end of HTTP-header
};

unsigned char *PWebSide;                         // pointer to webside

unsigned int HTTPBytesToSend;                    // bytes left to send

unsigned char HTTPStatus;                        // status byte

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

//unsigned char g_smc_tab_cs_size[6];

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

// copies bytes from source to dest
static void memcopyswap(void *Dest, void *Source, unsigned int Size)
{
   unsigned char * Src = Source;
   unsigned char * Des = Dest;
   while (Size > 1)
   {
      *(Des+1) = *Src;
	  *Des = *(Src+1);
      Src += 2;
      Des += 2;
	  Size -= 2;
   }

   if (Size)                                      // check for leftover byte...
   {
      *Des = *Src;
//      *(Des+1) = *Src;
//	  *Des = 0x20;
   }
}                                                // for the highbyte

// This function implements a very simple dynamic HTTP-server.
// It waits until connected, then sends a HTTP-header and the
// HTML-code stored in memory. Before sending, it replaces
// some special strings with dynamic values.
// NOTE: For strings crossing page boundaries, replacing will
// not work. In this case, simply add some extra lines
// (e.g. CR and LFs) to the HTML-code.
static void HTTPServer(void)
{
  if (SocketStatus & SOCK_CONNECTED)             // check if somebody has connected to our TCP
  {
    if (SocketStatus & SOCK_DATA_AVAILABLE)      // check if remote TCP sent data
      TCPReleaseRxBuffer();                      // and throw it away

    if (SocketStatus & SOCK_TX_BUF_RELEASED)     // check if buffer is free for TX
    {
      if (!(HTTPStatus & HTTP_SEND_PAGE))        // init byte-counter and pointer to webside
      {                                          // if called the 1st time
        HTTPBytesToSend = sizeof(WebSide) - 1;   // get HTML length, ignore trailing zero
        PWebSide = (unsigned char *)WebSide;     // pointer to HTML-code
      }

      if (HTTPBytesToSend > MAX_TCP_TX_DATA_SIZE)     // transmit a segment of MAX_SIZE
      {
//        if (!(HTTPStatus & HTTP_SEND_PAGE))           // 1st time, include HTTP-header
//        {
//          memcopyswap(TCP_TX_BUF, (unsigned short *)GetResponse, sizeof(GetResponse) - 1);
//          memcopyswap(TCP_TX_BUF + (sizeof(GetResponse) - 1), (unsigned short *)PWebSide, MAX_TCP_TX_DATA_SIZE - (sizeof(GetResponse) + 1));
//          memcpy(TCP_TX_BUF, (unsigned short *)GetResponse, sizeof(GetResponse) - 1);
//          memcpy(TCP_TX_BUF + (sizeof(GetResponse) - 1), (unsigned short *)PWebSide, MAX_TCP_TX_DATA_SIZE - (sizeof(GetResponse) + 1));
//          HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE - (sizeof(GetResponse) + 1);
//          PWebSide += MAX_TCP_TX_DATA_SIZE - (sizeof(GetResponse) + 1);
//        }
//        else
//        {
          memcopyswap(TCP_TX_BUF, (unsigned short *)PWebSide, MAX_TCP_TX_DATA_SIZE);
//          memcpy(TCP_TX_BUF, (unsigned short *)PWebSide, MAX_TCP_TX_DATA_SIZE);
          HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE;
          PWebSide += MAX_TCP_TX_DATA_SIZE;
//        }

        TCPTxDataCount = (unsigned short)MAX_TCP_TX_DATA_SIZE;   // bytes to xfer
//        InsertDynamicValues();                   // exchange some strings...
        TCPTransmitTxBuffer();                   // xfer buffer
      }
      else if (HTTPBytesToSend)                  // transmit leftover bytes
      {
          memcopyswap(TCP_TX_BUF, (unsigned short *)PWebSide, HTTPBytesToSend);
//        memcpy(TCP_TX_BUF, (unsigned short *)PWebSide, HTTPBytesToSend);
        TCPTxDataCount = HTTPBytesToSend;        // bytes to xfer
//        InsertDynamicValues();                   // exchange some strings...
        TCPTransmitTxBuffer();                   // send last segment
        TCPClose();                              // and close connection
        HTTPBytesToSend = 0;                     // all data sent
      }

      HTTPStatus |= HTTP_SEND_PAGE;              // ok, 1st loop executed
    }
  }
  else
    HTTPStatus &= ~HTTP_SEND_PAGE;               // reset help-flag if not connected
}

void Network_Test_Menu(void)
{
    // Set Strobe Byte High Enable
	gpio_set_gpio_pin(AVR32_EBI_NWE1_0_PIN);

    //turn off lansleep
    gpio_set_gpio_pin(AVR32_PIN_PB27);

    // Toggle Strobe Byte High Enable
    gpio_clr_gpio_pin(AVR32_EBI_NWE1_0_PIN);
    gpio_set_gpio_pin(AVR32_EBI_NWE1_0_PIN);

    // Initialize network driver resources.
    //smc_init(FOSC0);

    //gpio_clr_gpio_pin(AVR32_EBI_NCS_2_PIN);

    //TCPLowLevelInit(); //after TCPLowLevelInit()
    HTTPStatus = 0;                                // clear HTTP-server's flag register
    TCPLocalPort = TCP_PORT_HTTP;                  // set port we want to listen to

    Menu_Items = (sizeof(Network_Test_Menu_Functions)/sizeof(NONE))/4;
   Menu_Functions = (unsigned long *)Network_Test_Menu_Functions;
   Menu_String = (unsigned char *)Network_Test_Menu_Text;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// NAME:  Network_Exit                                                        //
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
void Network_Exit(void)
{
   Menu_Items = MAIN_MENU_FUNCTIONS_ITEMS;
   Menu_Functions = (unsigned long *)Main_Menu_Functions;
   Menu_String = (unsigned char *)Main_Menu_Text;
}

void Network_Info(void)
{
	  int count;
	    unsigned char character;

	    count = 0;
		character = Network_Info_Text[count];
		while(character != 0)
		{
			print_dbg_char(character);
			count++;
			character = Network_Info_Text[count];
		}
}
void Network_Self_Test(void)
{
   int *uart_rx_character = 0;

   print_dbg("\n\rIP is ");
   print_dbg_ulong(MYIP_1);
   print_dbg(".");
   print_dbg_ulong(MYIP_2);
   print_dbg(".");
   print_dbg_ulong(MYIP_3);
   print_dbg(".");
   print_dbg_ulong(MYIP_4);

   print_dbg("\n\rReady to answer PING and serve WEB page. \n\r");
   print_dbg("\n\rPress any key to exit. \n\r");

   while((usart_read_char(DBG_USART, uart_rx_character))!= USART_SUCCESS)
   {

      if (!(SocketStatus & SOCK_ACTIVE))
	  {
         TCPPassiveOpen();
	  }
      // handle network and WEB stack events
   	  DoNetworkStuff();
      HTTPServer();
   }
}
void Network_Transmit_Test(void){}
void Network_Receive_Test(void){}
void Network_LED_Test(void){}
void Network_EEPROM_Test(void){}
void Network_8_Bit_Test(void){}
void Network_16_Bit_Test(void){}
void Network_Memory_Test(void){}
void Network_Ping_IP_Test(void){}
void Network_Read_Home_Page(void){}
void Network_Read_MAC(void){}
void Network_Set_MAC(void){}
void Network_Read_IP(void){}
void Network_Set_IP(void){}


