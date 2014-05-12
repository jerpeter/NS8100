////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// MODULE NAME:                                                               //
//                                                                            //
//   cs8900.c                                                                 //
//                                                                            //
// AUTHOR:                                                                    //
//                                                                            //
//   JG                                                                       //
//                                                                            //
// DATE:                                                                      //
//                                                                            //
//   10/01/2008                                                               //
//                                                                            //
// HISTORY:                                                                   //
//                                                                            //
//   1.0 - Initial code generation and checking                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#include "cs8900.h"
#include "Typedefs.h"
#include "board.h"
#include "cycle_counter.h"
#include "intc.h"
#include "gpio.h"

// Counter of COUNT/COMPARE matches.
static volatile unsigned int u32NbCompareIrqTrigger = 0;

#define NB_CLOCK_CYCLE_DELAY_SHORT    3000000   // 240 ms if fCPU==12MHz

// constants
const unsigned char MyMAC[] =                    // "M1-M2-M3-M4-M5-M6"
{
  MYMAC_2, MYMAC_1,
  MYMAC_4, MYMAC_3,
  MYMAC_6, MYMAC_5
};

const TInitSeq InitSeq[] =
{
  {PP_IA, MYMAC_1 + (MYMAC_2 << 8)},               // set our MAC as Individual Address
  {PP_IA + 2, MYMAC_3 + (MYMAC_4 << 8)},
  {PP_IA + 4, MYMAC_5 + (MYMAC_6 << 8)},
  {PP_LineCTL, SERIAL_RX_ON | SERIAL_TX_ON},       // configure the Physical Interface
  {PP_RxCTL, RX_OK_ACCEPT | RX_IA_ACCEPT | RX_BROADCAST_ACCEPT}
};

extern unsigned short ISNGenHigh;                         // upper word of our Initial Sequence Number

extern unsigned char TCPTimer;                          // inc'd each 262ms

__attribute__((__interrupt__))
static void compare_irq_handler(void)
{
  // Count the number of times this IRQ handler is called.
//  u32NbCompareIrqTrigger++;
  ISNGenHigh++;                                // upper 16 bits of initial sequence number
  TCPTimer++;                                  // timer for retransmissions
//  Set_sys_compare(NB_CLOCK_CYCLE_DELAY_SHORT);
  Set_sys_compare(Get_sys_count() + NB_CLOCK_CYCLE_DELAY_SHORT);
}

void counter_init(void)
{
   U32 u32CompareVal;
   U32 u32CountVal;

   u32CountVal = Get_sys_count();

   u32CompareVal = u32CountVal + NB_CLOCK_CYCLE_DELAY_SHORT; // WARNING: MUST FIT IN 32bits.

   INTC_register_interrupt(&compare_irq_handler, AVR32_CORE_COMPARE_IRQ, AVR32_INTC_INT2);

   Set_sys_compare(u32CompareVal); // GO
}


// configure port-pins for use with LAN-controller,
// reset it and send the configuration-sequence
// (InitSeq[])
void Init8900(void)
{
   unsigned int i;

   //turn sleep off
   gpio_set_gpio_pin(AVR32_PIN_PB27);

   Write8900(ADD_PORT, PP_SelfCTL);
   Write8900(DATA_PORT, POWER_ON_RESET);          // Reset the Ethernet-Controller

   Write8900(ADD_PORT, PP_SelfST);
   while (!(Read8900(DATA_PORT) & INIT_DONE));    // wait until chip-reset is done

   for (i = 0; i < sizeof InitSeq / sizeof (TInitSeq); i++) // configure the CS8900
   {
      Write8900(ADD_PORT, InitSeq[i].Addr);
      Write8900(DATA_PORT, InitSeq[i].Data);
   }
}


// writes a word in little-endian byte order to
// a specified port-address
void Write8900(unsigned short *Address, unsigned int Data)
{
	volatile unsigned short *cs8900 = Address;

   *cs8900 = Data;                                  // write low order byte to data bus
}


// writes a word in little-endian byte order to TX_FRAME_PORT
void WriteFrame8900(unsigned int Data)
{
	volatile unsigned short *cs8900 = TX_FRAME_PORT;

    *cs8900 = Data;                                  // write low order byte to data bus
}


// copies bytes from MCU-memory to frame port
// NOTES: * an odd number of byte may only be transfered
//          if the frame is written to the end!
//        * MCU-memory MUST start at word-boundary
void CopyToFrame8900(void *Source, unsigned int Size)
{
   unsigned short temp;

   while (Size > 1)
   {
      temp = (*(unsigned char *)Source++) << 8;
      temp |= *(unsigned char *)Source++;
	  WriteFrame8900(temp);    // write leftover byte (the LAN-controller
      Size -= 2;
   }

   if (Size)                                      // if odd num. of bytes...
   {
      WriteFrame8900(*(unsigned char *)Source);    // write leftover byte (the LAN-controller
   }
}                                                // ignores the highbyte)


// reads a word in little-endian byte order from
// a specified port-address
unsigned int Read8900(unsigned short *Address)
{
   unsigned int ReturnValue = 0;
   volatile unsigned short *cs8900 = Address;

   ReturnValue = *cs8900;                                  // write low order byte to data bus

   return ReturnValue;
}


// reads a word in little-endian byte order from RX_FRAME_PORT
unsigned int ReadFrame8900(void)
{
   unsigned int ReturnValue = 0;
//   unsigned int temp;
   volatile unsigned short *cs8900 = RX_FRAME_PORT;

//   temp = *cs8900;                                  // write low order byte to data bus
//   ReturnValue = ((temp & 0xFF) << 8);                                  // write low order byte to data bus
//   ReturnValue |= ((temp & 0xFF00)>>8);
   ReturnValue = *cs8900;
   return ReturnValue;
}


// reads a word in big-endian byte order from RX_FRAME_PORT
// (useful to avoid permanent byte-swapping while reading
// TCP/IP-data)
unsigned int ReadFrameBE8900(void)
{
   unsigned int ReturnValue = 0;
   unsigned int temp;
   volatile unsigned short *cs8900 = RX_FRAME_PORT;

   temp = *cs8900;                                  // write low order byte to data bus
   ReturnValue = ((temp & 0xFF) << 8);                                  // write low order byte to data bus
   ReturnValue |= ((temp & 0xFF00)>>8);

   return ReturnValue;
}


// reads a word in little-endian byte order from
// a specified port-address
// NOTE: this func. xfers the high-byte 1st, must be used to
//       access some special registers (e.g. RxStatus)
unsigned int ReadHB1ST8900(unsigned short *Address)
{
   unsigned int ReturnValue = 0;
   volatile unsigned short *cs8900 = Address;

   ReturnValue = *cs8900;                                  // write low order byte to data bus

   return ReturnValue;
}


// copies bytes from frame port to MCU-memory
// NOTES: * an odd number of byte may only be transfered
//          if the frame is read to the end!
//        * MCU-memory MUST start at word-boundary
void CopyFromFrame8900(void *Dest, unsigned int Size)
{
   unsigned short temp;

   while (Size > 1)
   {
      temp = ReadFrame8900();
      *(unsigned char *)Dest++ = (temp >> 8) & 0xFF;
      *(unsigned char *)Dest++ = (temp & 0xFF);
      Size -= 2;
    }

   if (Size)                                      // check for leftover byte...
   {
	   *(unsigned char *)Dest = ReadFrame8900();    // the LAN-Controller will return 0
   }
}                                                // for the highbyte


// does a dummy read on frame-I/O-port
// NOTE: only an even number of bytes is read!
void DummyReadFrame8900(unsigned int Size)       // discards an EVEN number of bytes
{                                                // from RX-fifo
   while (Size > 1)
   {
      ReadFrame8900();
      Size -= 2;
   }
}


// requests space in CS8900's on-chip memory for
// storing an outgoing frame
void RequestSend(unsigned int FrameSize)
{
  Write8900(TX_CMD_PORT, TX_START_ALL_BYTES);
  Write8900(TX_LEN_PORT, FrameSize);
}


// check if CS8900 is ready to accept the
// frame we want to send
unsigned int Rdy4Tx(void)
{
  Write8900(ADD_PORT, PP_BusST);
  return (Read8900(DATA_PORT) & READY_FOR_TX_NOW);
}

