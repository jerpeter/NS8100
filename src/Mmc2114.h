///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Mmc2114.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/Mmc2114.h,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------
#if 0
#ifndef _MMC2114_H_
#define _MMC2114_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "Typedefs.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------
/* Following are beginning address of chip on-board modules
Note that these address names end with an underscore '_' */
#define FLASH_  0x00000000
#define RAM_    0x00800000
#define PORTS_  0x00c00000  /* Digital I/O Ports */
#define CCM_    0x00c10000  /* Chip Configuration */
#define CS_     0x00c20000  /* Chip Selects */
#define CLOCK_  0x00c30000  /* Clocks */
#define RESET_  0x00c40000  /* Resets */
#define INTC_   0x00c50000  /* Interrupts */
#define EPORT_  0x00c60000  /* Edge Port */
#define WDT_    0x00c70000  /* Watchdog Timer */
#define PIT1_   0x00c80000  /* Programmable Interrupt Timer 1 */
#define PIT2_   0x00c90000  /* Programmable Interrupt Timer 2 */
#define QADC_   0x00ca0000  /* Queued Analog-to-digital Converter */
#define SPI_    0x00cb0000  /* Serial Peripheral Interface */
#define SCI1_   0x00cc0000  /* Serial Communications Interface 1 */
#define SCI2_   0x00cd0000  /* Serial Communications Interface 2 */
#define TIM1_   0x00ce0000  /* Timer 1 */
#define TIM2_   0x00cf0000  /* Timer 2 */
#define SGFM_   0x00d00000  /* FLASH Registers */
#define EXTMEM_ 0x80000000  /* External Memory */

/* PSR Bits */
#define PSR_S   bit31
#define SP      bit29+bit28
#define U3      bit27
#define U2      bit26
#define U1      bit25
#define U0      bit24
#define PSR_VEC bit22+bit21+bit20+bit19+bit18+bit17+bit16
#define TM      bit15+bit14
#define TP      bit13
#define TCTL    bit12
#define SC      bit10
#define MM      bit09
#define EE      bit08
#define IC      bit07
#define IE      bit06
#define FIE     bit04
#define AF      bit01
#define C       bit00


/*  Ports  **************************************************************************************
*
*  Address    Use                                                 Access
*  00C0_0000  reg_PORTA   Port A Output Data Register             Supervisor / User
*  00C0_0001  reg_PORTB   Port B Output Data Register             Supervisor / User
*  00C0_0002  reg_PORTC   Port C Output Data Register             Supervisor / User
*  00C0_0003  reg_PORTD   Port D Output Data Register             Supervisor / User
*  00C0_0004  reg_PORTE   Port E Output Data Register             Supervisor / User
*  00C0_0005  reg_PORTF   Port F Output Data Register             Supervisor / User
*  00C0_0006  reg_PORTG   Port G Output Data Register             Supervisor / User
*  00C0_0007  reg_PORTH   Port H Output Data Register             Supervisor / User
*  00C0_0008  reg_PORTI   Port I Output Data Register             Supervisor / User
*  00C0_0009  ...      reserved
*  00C0_000B
*  00C0_000C  reg_DDRA    Port A Data Direction Register          Supervisor / User
*  00C0_000D  reg_DDRB    Port B Data Direction Register          Supervisor / User
*  00C0_000E  reg_DDRC    Port C Data Direction Register          Supervisor / User
*  00C0_000F  reg_DDRD    Port D Data Direction Register          Supervisor / User
*  00C0_0010  reg_DDRE    Port E Data Direction Register          Supervisor / User
*  00C0_0011  reg_DDRF    Port F Data Direction Register          Supervisor / User
*  00C0_0012  reg_DDRG    Port G Data Direction Register          Supervisor / User
*  00C0_0013  reg_DDRH    Port H Data Direction Register          Supervisor / User
*  00C0_0014  reg_DDRI    Port I Data Direction Register          Supervisor / User
*  00C0_0015  ...      reserved
*  00C0_0017
*  00C0_0018  reg_PORTAP  Port A Pin Data Register                Supervisor / User
*  00C0_0018  reg_SETA    Port A Set Data Register                Supervisor / User
*  00C0_0019  reg_PORTBP  Port B Pin Data Register                Supervisor / User
*  00C0_0019  reg_SETB    Port B Set Data Register                Supervisor / User
*  00C0_001A  reg_PORTCP  Port C Pin Data Register                Supervisor / User
*  00C0_001A  reg_SETC    Port C Set Data Register                Supervisor / User
*  00C0_001B  reg_PORTDP  Port D Pin Data Register                Supervisor / User
*  00C0_001B  reg_SETD    Port D Set Data Register                Supervisor / User
*  00C0_001C  reg_PORTEP  Port E Pin Data Register                Supervisor / User
*  00C0_001C  reg_SETE    Port E Set Data Register                Supervisor / User
*  00C0_001D  reg_PORTFP  Port F Pin Data Register                Supervisor / User
*  00C0_001D  reg_SETF    Port F Set Data Register                Supervisor / User
*  00C0_001E  reg_PORTGP  Port G Pin Data Register                Supervisor / User
*  00C0_001E  reg_SETG    Port G Set Data Register                Supervisor / User
*  00C0_001F  reg_PORTHP  Port H Pin Data Register                Supervisor / User
*  00C0_001F  reg_SETH    Port H Set Data Register                Supervisor / User
*  00C0_0020  reg_PORTIP  Port I Pin Data Register                Supervisor / User
*  00C0_0020  reg_SETI    Port I Set Data Register                Supervisor / User
*  00C0_0021  ...      reserved
*  00C0_0023
*  00C0_0024  reg_CLRA    Port A Clear Output Data Register       Supervisor / User
*  00C0_0025  reg_CLRB    Port B Clear Output Data Register       Supervisor / User
*  00C0_0026  reg_CLRC    Port C Clear Output Data Register       Supervisor / User
*  00C0_0027  reg_CLRD    Port D Clear Output Data Register       Supervisor / User
*  00C0_0028  reg_CLRE    Port E Clear Output Data Register       Supervisor / User
*  00C0_0029  reg_CLRF    Port F Clear Output Data Register       Supervisor / User
*  00C0_002A  reg_CLRG    Port G Clear Output Data Register       Supervisor / User
*  00C0_002B  reg_CLRH    Port H Clear Output Data Register       Supervisor / User
*  00C0_002C  reg_CLRI    Port I Clear Output Data Register       Supervisor / User
*  00C0_002D  ...      reserved
*  00C0_002F
*  00C0_0030  reg_PCDPAR  Port C/D Pin Assignment Register        Supervisor / User
*  00C0_0031  reg_PEPAR   Port E Pin Assignment Register          Supervisor / User
*  00C0_0032  ...      reserved
*  00C0_003F
*  00C0_0040  ...         Port register space mirror
*  00C0_FFFF
*/


/* Bit names for general use */
#ifndef BIT_NAMES
#define BIT_NAMES
#define bit39 0x00000080
#define bit38 0x00000040
#define bit37 0x00000020
#define bit36 0x00000010
#define bit35 0x00000008
#define bit34 0x00000004
#define bit33 0x00000002
#define bit32 0x00000001
#define bit31 0x80000000
#define bit30 0x40000000
#define bit29 0x20000000
#define bit28 0x10000000
#define bit27 0x08000000
#define bit26 0x04000000
#define bit25 0x02000000
#define bit24 0x01000000
#define bit23 0x00800000
#define bit22 0x00400000
#define bit21 0x00200000
#define bit20 0x00100000
#define bit19 0x00080000
#define bit18 0x00040000
#define bit17 0x00020000
#define bit16 0x00010000
#define bit15 0x00008000
#define bit14 0x00004000
#define bit13 0x00002000
#define bit12 0x00001000
#define bit11 0x00000800
#define bit10 0x00000400
#define bit09 0x00000200
#define bit08 0x00000100
#define bit07 0x00000080
#define bit06 0x00000040
#define bit05 0x00000020
#define bit04 0x00000010
#define bit03 0x00000008
#define bit02 0x00000004
#define bit01 0x00000002
#define bit00 0x00000001
#endif  /* BIT_NAMES */

typedef union                /* Port x Output Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  PIN7      : 1; /* Port Pin 7 */
    bitfield  PIN6      : 1; /* Port Pin 6 */
    bitfield  PIN5      : 1; /* Port Pin 5 */
    bitfield  PIN4      : 1; /* Port Pin 4 */
    bitfield  PIN3      : 1; /* Port Pin 3 */
    bitfield  PIN2      : 1; /* Port Pin 2 */
    bitfield  PIN1      : 1; /* Port Pin 1 */
    bitfield  PIN0      : 1; /* Port Pin 0 */
  } bit;
} typ_PORTx;
                             /* Port Pin Bit Masks */
#define mask_PIN7  0x80
#define mask_PIN6  0x40
#define mask_PIN5  0x20
#define mask_PIN4  0x10
#define mask_PIN3  0x08
#define mask_PIN2  0x04
#define mask_PIN1  0x02
#define mask_PIN0  0x01

/* Port A Output Data Register */
#define reg_PORTA (*(volatile typ_PORTx*) (0x00C00000))
/* Port B Output Data Register */
#define reg_PORTB (*(volatile typ_PORTx*) (0x00C00001))
/* Port C Output Data Register */
#define reg_PORTC (*(volatile typ_PORTx*) (0x00C00002))
/* Port D Output Data Register */
#define reg_PORTD (*(volatile typ_PORTx*) (0x00C00003))
/* Port E Output Data Register */
#define reg_PORTE (*(volatile typ_PORTx*) (0x00C00004))
/* Port F Output Data Register */
#define reg_PORTF (*(volatile typ_PORTx*) (0x00C00005))
/* Port G Output Data Register */
#define reg_PORTG (*(volatile typ_PORTx*) (0x00C00006))
/* Port H Output Data Register */
#define reg_PORTH (*(volatile typ_PORTx*) (0x00C00007))
/* Port I Output Data Register */
#define reg_PORTI (*(volatile typ_PORTx*) (0x00C00008))
/* Port A - D Output Data Pointer */
#define reg_PORT_A_D (*(volatile INT32U*) (0x00C00000))
/* Port E - H Output Data Pointer */
#define reg_PORT_E_H (*(volatile INT32U*) (0x00C00004))

/* Port A Data Direction Register */
#define reg_DDRA (*(volatile typ_PORTx*) (0x00C0000C))
/* Port B Data Direction Register */
#define reg_DDRB (*(volatile typ_PORTx*) (0x00C0000D))
/* Port C Data Direction Register */
#define reg_DDRC (*(volatile typ_PORTx*) (0x00C0000E))
/* Port D Data Direction Register */
#define reg_DDRD (*(volatile typ_PORTx*) (0x00C0000F))
/* Port E Data Direction Register */
#define reg_DDRE (*(volatile typ_PORTx*) (0x00C00010))
/* Port F Data Direction Register */
#define reg_DDRF (*(volatile typ_PORTx*) (0x00C00011))
/* Port G Data Direction Register */
#define reg_DDRG (*(volatile typ_PORTx*) (0x00C00012))
/* Port H Data Direction Register */
#define reg_DDRH (*(volatile typ_PORTx*) (0x00C00013))
/* Port I Data Direction Register */
#define reg_DDRI (*(volatile typ_PORTx*) (0x00C00014))
/* Port A - D Data Direction Pointer */
#define reg_DDR_A_D (*(volatile INT32U*) (0x00C0000C))
/* Port E - H Data Direction Pointer */
#define reg_DDR_E_H (*(volatile INT32U*) (0x00C00010))

/* Port A Pin Data Register */
#define reg_PORTAP (*(volatile typ_PORTx*) (0x00C00018))
/* Port B Pin Data Register */
#define reg_PORTBP (*(volatile typ_PORTx*) (0x00C00019))
/* Port C Pin Data Register */
#define reg_PORTCP (*(volatile typ_PORTx*) (0x00C0001A))
/* Port D Pin Data Register */
#define reg_PORTDP (*(volatile typ_PORTx*) (0x00C0001B))
/* Port E Pin Data Register */
#define reg_PORTEP (*(volatile typ_PORTx*) (0x00C0001C))
/* Port F Pin Data Register */
#define reg_PORTFP (*(volatile typ_PORTx*) (0x00C0001D))
/* Port G Pin Data Register */
#define reg_PORTGP (*(volatile typ_PORTx*) (0x00C0001E))
/* Port H Pin Data Register */
#define reg_PORTHP (*(volatile typ_PORTx*) (0x00C0001F))
/* Port I Pin Data Register */
#define reg_PORTIP (*(volatile typ_PORTx*) (0x00C00020))

/* Port A Pin Set Data Register */
#define reg_SETA (*(volatile typ_PORTx*) (0x00C00018))
/* Port B Pin Set Data Register */
#define reg_SETB (*(volatile typ_PORTx*) (0x00C00019))
/* Port C Pin Set Data Register */
#define reg_SETC (*(volatile typ_PORTx*) (0x00C0001A))
/* Port D Pin Set Data Register */
#define reg_SETD (*(volatile typ_PORTx*) (0x00C0001B))
/* Port E Pin Set Data Register */
#define reg_SETE (*(volatile typ_PORTx*) (0x00C0001C))
/* Port F Pin Set Data Register */
#define reg_SETF (*(volatile typ_PORTx*) (0x00C0001D))
/* Port G Pin Set Data Register */
#define reg_SETG (*(volatile typ_PORTx*) (0x00C0001E))
/* Port H Pin Set Data Register */
#define reg_SETH (*(volatile typ_PORTx*) (0x00C0001F))
/* Port I Pin Set Data Register */
#define reg_SETI (*(volatile typ_PORTx*) (0x00C00020))

/* Port A Clear Output Data Register */
#define reg_CLRA (*(volatile typ_PORTx*) (0x00C00024))
/* Port B Clear Output Data Register */
#define reg_CLRB (*(volatile typ_PORTx*) (0x00C00025))
/* Port C Clear Output Data Register */
#define reg_CLRC (*(volatile typ_PORTx*) (0x00C00026))
/* Port D Clear Output Data Register */
#define reg_CLRD (*(volatile typ_PORTx*) (0x00C00027))
/* Port E Clear Output Data Register */
#define reg_CLRE (*(volatile typ_PORTx*) (0x00C00028))
/* Port F Clear Output Data Register */
#define reg_CLRF (*(volatile typ_PORTx*) (0x00C00029))
/* Port G Clear Output Data Register */
#define reg_CLRG (*(volatile typ_PORTx*) (0x00C0002A))
/* Port H Clear Output Data Register */
#define reg_CLRH (*(volatile typ_PORTx*) (0x00C0002B))
/* Port I Clear Output Data Register */
#define reg_CLRI (*(volatile typ_PORTx*) (0x00C0002C))

typedef union                /* Port C/D Pin Assignment Register */
{
  INT8U reg;
  struct
  {
    bitfield  PCDPA     : 1; /* Port C/D I7/6 Pin Assignment */
    bitfield  unused    : 7;
  } bit;
} typ_PCDPAR;
#define reg_PCDPAR (*(volatile typ_PCDPAR*) (0x00C00030))




typedef union                /* Port E Pin Assignment Register */
{
  INT8U reg;
  struct
  {
    bitfield  PEPA7     : 1; /* Port E Pin Assignment 7 */
    bitfield  PEPA6     : 1; /* Port E Pin Assignment 6 */
    bitfield  PEPA5     : 1; /* Port E Pin Assignment 5 */
    bitfield  PEPA4     : 1; /* Port E Pin Assignment 4 */
    bitfield  PEPA3     : 1; /* Port E Pin Assignment 3 */
    bitfield  PEPA2     : 1; /* Port E Pin Assignment 2 */
    bitfield  PEPA1     : 1; /* Port E Pin Assignment 1 */
    bitfield  PEPA0     : 1; /* Port E Pin Assignment 0 */
  } bit;
} typ_PEPAR;
#define reg_PEPAR (*(volatile typ_PEPAR*) (0x00C00031))

/*  Chip Configuration Module  ******************************************************************

*  Address     Use                                            Access
*  00C1_0000  reg_CCR  Chip Configuration Register            Supervisor Only
*  00C1_0002         reserved
*  00C1_0004  reg_RCON Reset Configuration Register           Supervisor Only
*  00C1_0006  reg_CIR  Chip Identification Register           Supervisor Only
*  00C1_0008  reg_CTR  Chip Test Register                     Supervisor Only
*  00C1_000A  ...      reserved
*  00C1_000B
*  00C1_000C  ...      not used
*  00C1_000F
*  00C1_0010  ..      not used
*  00C1_FFFF
*/

typedef union                /* Chip Configuration Register */
{
  INT16U reg;
  struct
  {
    bitfield  LOAD      : 1; /* Pad Driver Load */
    bitfield  unused    : 1;
    bitfield  SHEN      : 1; /* Show Cycle Enable */
    bitfield  EMINT     : 1; /* Emulate Internal Address Space */
    bitfield  unused1   : 1;
    bitfield  MODE      : 3; /* Chip Configuration Mode */
    bitfield  unused2   : 1;
    bitfield  SZEN      : 1; /* TSIZ[1:0] Enable */
    bitfield  PSTEN     : 1; /* PSTAT[3:0] Enable */
    bitfield  SHINT     : 1; /* Show Interrupt */
    bitfield  BME       : 1; /* Bus Monitor External Enable */
    bitfield  BMD       : 1; /* Bus Monitor Debug Mode */
    bitfield  BMT       : 2; /* Bus Monitor Timing */
  } bit;
} typ_CCR;
#define reg_CCR (*(volatile typ_CCR*) (0x00C10000))

typedef union                /* Reset Configuration Register */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 8;
    bitfield  RPLLSEL   : 1; /* PLL Mode Select */
    bitfield  RPLLREF   : 1; /* PLL Reference */
    bitfield  RLOAD     : 1; /* Pad Driver Load */
    bitfield  unused1   : 1;
    bitfield  BOOTPS    : 1; /* Boot Port Size */
    bitfield  BOOTSEL   : 1; /* Boot Select */
    bitfield  unused2   : 1;
    bitfield  MODE      : 1; /* Chip Configuration Mode */
  } bit;
} typ_RCON;
#define reg_RCON (*(volatile typ_RCON*) (0x00C10004))

typedef union                /* Chip Identification Register */
{
  INT16U reg;
  struct
  {
    bitfield  PIN       : 8; /* Part Identification Number */
    bitfield  PRN       : 8; /* Part Revision Number */
  } bit;
} typ_CIR;
#define reg_CIR (*(volatile typ_CIR*) (0x00C10006))
                             /* Chip Test Register */
#define reg_CTR (*(volatile INT16U*) (0x00C10008))

/*  Chip Select Module  *************************************************************************
*
*  Address     Use                                                Access
*  00C2_0000  reg_CSCR0   Chip Select Control Register 0          Supervisor Only
*  00C2_0002  reg_CSCR1   Chip Select Control Register 1          Supervisor Only
*  00C2_0004  reg_CSCR2   Chip Select Control Register 2          Supervisor Only
*  00C2_0006  reg_CSCR3   Chip Select Control Register 3          Supervisor Only
*  00C2_0008  ...      not used
*  00C2_FFFF
*/

typedef union                /* Chip Select Control Register */
{
  INT16U reg;
  struct
  {
    bitfield  SO        : 1; /* Supervisor-Only */
    bitfield  RO        : 1; /* Read-Only */
    bitfield  PS        : 1; /* Port Size */
    bitfield  WWS       : 1; /* Write Wait State */
    bitfield  WE        : 1; /* Write Enable */
    bitfield  WS        : 3; /* Wait State */
    bitfield  unused    : 6;
    bitfield  TAEN      : 1; /* Transfer Acknowledge Enable */
    bitfield  CSEN      : 1; /* Chip Select Enable */
  } bit;
} typ_CSCRx;
                             /* Chip Select Control Register 0 */
#define reg_CSCR0 (*(volatile typ_CSCRx*) (0x00C20000))
                             /* Chip Select Control Register 1 */
#define reg_CSCR1 (*(volatile typ_CSCRx*) (0x00C20002))
                             /* Chip Select Control Register 2 */
#define reg_CSCR2 (*(volatile typ_CSCRx*) (0x00C20004))
                             /* Chip Select Control Register 3 */
#define reg_CSCR3 (*(volatile typ_CSCRx*) (0x00C20006))

/*  Clock Module  *******************************************************************************
*
*  Address    Use                                               Access
*  00C3_0000  reg_SYNCR    Synthesizer Control Register         Supervisor Only
*  00C3_0002  reg_SYNSR    Synthesizer Status Register          Supervisor Only
*  00C3_0003  reg_SYNTR    Synthesizer Test Register            Supervisor Only
*  00C3_0004  reg_SYNTR2   Synthesizer Test Register 2          Supervisor Only
*  00C3_0008  ...      not used
*  00C3_FFFF
*/

typedef union                /* Synthesizer Control Register */
{
  INT16U reg;
  struct
  {
    bitfield  LOLRE     : 1; /* Loss of Lock Reset Enable */
    bitfield  MFD       : 3; /* Multiplication Factor Divider */
    bitfield  LOCRE     : 1; /* Loss of Clock Reset Enable */
    bitfield  RFD       : 3; /* Reduced Frequency Divider */
    bitfield  LOCEN     : 1; /* Loss of Clock Enable */
    bitfield  DISCLK    : 1; /* Disable CLKOUT */
    bitfield  FWKUP     : 1; /* Fast Wakeup */
    bitfield  reserved  : 1;
    bitfield  STMPD     : 2; /* Stop Mode */
    bitfield  reserved1 : 2;
  } bit;
} typ_SYNCR;
#define reg_SYNCR (*(volatile typ_SYNCR*) (0x00C30000))

typedef union                /* Synthesizer Status Register */
{
  INT8U reg;
  struct
  {
    bitfield  PLLMODE   : 3;  /* Clock Mode : PLL Select : PLL Reference */
    bitfield  LOCKS     : 1;  /* Sticky PLL Lock Flag */
    bitfield  LOCK      : 1;  /* PLL Lock Flag */
    bitfield  LOCS      : 1;  /* Sticky Loss of Clock Flag */
    bitfield  unused    : 2;
  } bit;
} typ_SYNSR;
#define reg_SYNSR (*(volatile typ_SYNSR*) (0x00C30002))

/*  Reset Module  *******************************************************************************
*
*  Address    Use                                           Access
*  00C4_0000  reg_RCR    Reset Control Register             Supervisor / User
*  00C4_0001  reg_RSR    Reset Status Register              Supervisor / User
*  00C4_0002  reg_RTR    Reset Test Register                Supervisor / User
*  00C4_0003          reserved
*  00C4_0004  ...     not used
*  00C4_FFFF
*/

typedef union                /* Reset Control Register */
{
  INT8U reg;
  struct
  {
    bitfield  SOFTRST   : 1; /* Software Reset Request */
    bitfield  FRCRSTOUT : 1; /* Force RSTOUT pin */
    bitfield  unused    : 1;
    bitfield  LVDF      : 1; /* Low Voltage Detect Flag */
    bitfield  LVDIE     : 1; /* Low Voltage Detect Interrupt Enable */
    bitfield  LVDRE     : 1; /* Low Voltage Detect Reset Enable */
    bitfield  LVDSE     : 1; /* Low Voltage Detect Stop Enable */
    bitfield  LVDE      : 1; /* Low Voltage Detect Enable */
  } bit;
} typ_RCR;
#define reg_RCR (*(volatile typ_RCR*) (0x00C40000))

typedef union                /* Reset Status Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 1;
    bitfield  LVD       : 1; /* Low Voltage Reset Detect Flag */
    bitfield  SOFT      : 1; /* Software Reset Flag */
    bitfield  WDR       : 1; /* Watchdog Timer Reset Flag */
    bitfield  POR       : 1; /* Power-On Reset Flag */
    bitfield  EXT       : 1; /* External Reset Flag */
    bitfield  LOC       : 1; /* Loss of Clock Reset Flag */
    bitfield  LOL       : 1; /* Loss of Lock Reset Flag */
  } bit;
} typ_RSR;
#define reg_RSR (*(volatile typ_RSR*) (0x00C40001))

/*  Interrupt Controller Module  ****************************************************************
*
*  Address   Use                                              Access
*  00C5_0000  reg_ICR     Interrupt Control Register          Supervisor / User
*  00C5_0002  reg_ISR     Interrupt Status Register           Supervisor / User
*  00C5_0004  reg_IFRH    Interrupt Force Register High       Supervisor / User
*  00C5_0008  reg_IFRL    Interrupt Force Register Low        Supervisor / User
*  00C5_000C  reg_IPR     Interrupt Pending Register          Supervisor / User
*  00C5_0010  reg_NIER    Normal Interrupt Enable Register    Supervisor / User
*  00C5_0014  reg_NIPR    Normal Interrupt Pending Register   Supervisor / User
*  00C5_0018  reg_FIER    Fast Interrupt Enable Register      Supervisor / User
*  00C5_001C  reg_FIPR    Fast Interrupt Pending Register     Supervisor / User
*  00C5_0020  ...      not used
*  00C5_003F
*  00C5_0040  reg_PLSR0   Priority Level Select Register 0    Supervisor Only
*  00C5_0041  reg_PLSR1   Priority Level Select Register 1    Supervisor Only
*  00C5_0042  reg_PLSR2   Priority Level Select Register 2    Supervisor Only
*  00C5_0043  reg_PLSR3   Priority Level Select Register 3    Supervisor Only
*  00C5_0044  reg_PLSR4   Priority Level Select Register 4    Supervisor Only
*  00C5_0045  reg_PLSR5   Priority Level Select Register 5    Supervisor Only
*  00C5_0046  reg_PLSR6   Priority Level Select Register 6    Supervisor Only
*  00C5_0047  reg_PLSR7   Priority Level Select Register 7    Supervisor Only
*  00C5_0048  reg_PLSR8   Priority Level Select Register 8    Supervisor Only
*  00C5_0049  reg_PLSR9   Priority Level Select Register 9    Supervisor Only
*  00C5_004A  reg_PLSR10  Priority Level Select Register 10   Supervisor Only
*  00C5_004B  reg_PLSR11  Priority Level Select Register 11   Supervisor Only
*  00C5_004C  reg_PLSR12  Priority Level Select Register 12   Supervisor Only
*  00C5_004D  reg_PLSR13  Priority Level Select Register 13   Supervisor Only
*  00C5_004E  reg_PLSR14  Priority Level Select Register 14   Supervisor Only
*  00C5_004F  reg_PLSR15  Priority Level Select Register 15   Supervisor Only
*  00C5_0050  reg_PLSR16  Priority Level Select Register 16   Supervisor Only
*  00C5_0051  reg_PLSR17  Priority Level Select Register 17   Supervisor Only
*  00C5_0052  reg_PLSR18  Priority Level Select Register 18   Supervisor Only
*  00C5_0053  reg_PLSR19  Priority Level Select Register 19   Supervisor Only
*  00C5_0054  reg_PLSR20  Priority Level Select Register 20   Supervisor Only
*  00C5_0055  reg_PLSR21  Priority Level Select Register 21   Supervisor Only
*  00C5_0056  reg_PLSR22  Priority Level Select Register 22   Supervisor Only
*  00C5_0057  reg_PLSR23  Priority Level Select Register 23   Supervisor Only
*  00C5_0058  reg_PLSR24  Priority Level Select Register 24   Supervisor Only
*  00C5_0059  reg_PLSR25  Priority Level Select Register 25   Supervisor Only
*  00C5_005A  reg_PLSR26  Priority Level Select Register 26   Supervisor Only
*  00C5_005B  reg_PLSR27  Priority Level Select Register 27   Supervisor Only
*  00C5_005C  reg_PLSR28  Priority Level Select Register 28   Supervisor Only
*  00C5_005D  reg_PLSR29  Priority Level Select Register 29   Supervisor Only
*  00C5_005E  reg_PLSR30  Priority Level Select Register 30   Supervisor Only
*  00C5_005F  reg_PLSR31  Priority Level Select Register 31   Supervisor Only
*  00C5_0060  reg_PLSR32  Priority Level Select Register 32   Supervisor Only
*  00C5_0061  reg_PLSR33  Priority Level Select Register 33   Supervisor Only
*  00C5_0062  reg_PLSR34  Priority Level Select Register 34   Supervisor Only
*  00C5_0063  reg_PLSR35  Priority Level Select Register 35   Supervisor Only
*  00C5_0064  reg_PLSR36  Priority Level Select Register 36   Supervisor Only
*  00C5_0065  reg_PLSR37  Priority Level Select Register 37   Supervisor Only
*  00C5_0066  reg_PLSR38  Priority Level Select Register 38   Supervisor Only
*  00C5_0067  reg_PLSR39  Priority Level Select Register 39   Supervisor Only
*  00C5_0068  ...      not used
*  00C5_007F
*  00C5_0080  ...      not used
*  00C5_FFFF
*/

typedef union                /* Interrupt Control Register */
{
  INT16U reg;
  struct
  {
    bitfield  AE        : 1; /* Autovector Enable */
    bitfield  FVE       : 1; /* Fast Vector Enable */
    bitfield  ME        : 1; /* Mask Enable */
    bitfield  MFI       : 1; /* Mask Fast Interrupts */
    bitfield  unused    : 7;
    bitfield  MASK      : 5; /* Interrupt Mask */
  } bit;
} typ_ICR;
#define reg_ICR (*(volatile typ_ICR*) (0x00C50000))

typedef union                /* Interrupt Status Register */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 6;
    bitfield  INT       : 1; /* Normal Interrupt Request Flag */
    bitfield  FINT      : 1; /* Fast Interrupt Request Flag */
    bitfield  unused1   : 1;
    bitfield  VEC       : 7; /* Interrupt Vector Number */
  } bit;
} typ_ISR;
#define reg_ISR (*(volatile typ_ISR*) (0x00C50002))

typedef union                /* Interrupt Force Register High */
{
  INT32U reg;
  struct
  {
    bitfield  unused    : 24;
    bitfield  IF39      : 1; /* Interrupt Force 39 */
    bitfield  IF38      : 1; /* Interrupt Force 38 */
    bitfield  IF37      : 1; /* Interrupt Force 37 */
    bitfield  IF36      : 1; /* Interrupt Force 36 */
    bitfield  IF35      : 1; /* Interrupt Force 35 */
    bitfield  IF34      : 1; /* Interrupt Force 34 */
    bitfield  IF33      : 1; /* Interrupt Force 33 */
    bitfield  IF32      : 1; /* Interrupt Force 32 */
  } bit;
} typ_IFRH;
#define reg_IFRH (*(volatile typ_IFRH*) (0x00C50004))

typedef union                /* Interrupt Force Register Low */
{
  INT32U reg;
  struct
  {
    bitfield  IF31      : 1; /* Interrupt Force 31 */
    bitfield  IF30      : 1; /* Interrupt Force 30 */
    bitfield  IF29      : 1; /* Interrupt Force 29 */
    bitfield  IF28      : 1; /* Interrupt Force 28 */
    bitfield  IF27      : 1; /* Interrupt Force 27 */
    bitfield  IF26      : 1; /* Interrupt Force 26 */
    bitfield  IF25      : 1; /* Interrupt Force 25 */
    bitfield  IF24      : 1; /* Interrupt Force 24 */
    bitfield  IF23      : 1; /* Interrupt Force 23 */
    bitfield  IF22      : 1; /* Interrupt Force 22 */
    bitfield  IF21      : 1; /* Interrupt Force 21 */
    bitfield  IF20      : 1; /* Interrupt Force 20 */
    bitfield  IF19      : 1; /* Interrupt Force 19 */
    bitfield  IF18      : 1; /* Interrupt Force 18 */
    bitfield  IF17      : 1; /* Interrupt Force 17 */
    bitfield  IF16      : 1; /* Interrupt Force 16 */
    bitfield  IF15      : 1; /* Interrupt Force 15 */
    bitfield  IF14      : 1; /* Interrupt Force 14 */
    bitfield  IF13      : 1; /* Interrupt Force 13 */
    bitfield  IF12      : 1; /* Interrupt Force 12 */
    bitfield  IF11      : 1; /* Interrupt Force 11 */
    bitfield  IF10      : 1; /* Interrupt Force 10 */
    bitfield  IF9       : 1; /* Interrupt Force 9 */
    bitfield  IF8       : 1; /* Interrupt Force 8 */
    bitfield  IF7       : 1; /* Interrupt Force 7 */
    bitfield  IF6       : 1; /* Interrupt Force 6 */
    bitfield  IF5       : 1; /* Interrupt Force 5 */
    bitfield  IF4       : 1; /* Interrupt Force 4 */
    bitfield  IF3       : 1; /* Interrupt Force 3 */
    bitfield  IF2       : 1; /* Interrupt Force 2 */
    bitfield  IF1       : 1; /* Interrupt Force 1 */
    bitfield  IF0       : 1; /* Interrupt Force 0 */
  } bit;
} typ_IFRL;
#define reg_IFRL (*(volatile typ_IFRL*) (0x00C50008))

typedef union                /* Interrupt Pending Register */
{
  INT32U reg;
  struct
  {
    bitfield  IP31      : 1; /* Interrupt Pending 31 */
    bitfield  IP30      : 1; /* Interrupt Pending 30 */
    bitfield  IP29      : 1; /* Interrupt Pending 29 */
    bitfield  IP28      : 1; /* Interrupt Pending 28 */
    bitfield  IP27      : 1; /* Interrupt Pending 27 */
    bitfield  IP26      : 1; /* Interrupt Pending 26 */
    bitfield  IP25      : 1; /* Interrupt Pending 25 */
    bitfield  IP24      : 1; /* Interrupt Pending 24 */
    bitfield  IP23      : 1; /* Interrupt Pending 23 */
    bitfield  IP22      : 1; /* Interrupt Pending 22 */
    bitfield  IP21      : 1; /* Interrupt Pending 21 */
    bitfield  IP20      : 1; /* Interrupt Pending 20 */
    bitfield  IP19      : 1; /* Interrupt Pending 19 */
    bitfield  IP18      : 1; /* Interrupt Pending 18 */
    bitfield  IP17      : 1; /* Interrupt Pending 17 */
    bitfield  IP16      : 1; /* Interrupt Pending 16 */
    bitfield  IP15      : 1; /* Interrupt Pending 15 */
    bitfield  IP14      : 1; /* Interrupt Pending 14 */
    bitfield  IP13      : 1; /* Interrupt Pending 13 */
    bitfield  IP12      : 1; /* Interrupt Pending 12 */
    bitfield  IP11      : 1; /* Interrupt Pending 11 */
    bitfield  IP10      : 1; /* Interrupt Pending 10 */
    bitfield  IP9       : 1; /* Interrupt Pending 9 */
    bitfield  IP8       : 1; /* Interrupt Pending 8 */
    bitfield  IP7       : 1; /* Interrupt Pending 7 */
    bitfield  IP6       : 1; /* Interrupt Pending 6 */
    bitfield  IP5       : 1; /* Interrupt Pending 5 */
    bitfield  IP4       : 1; /* Interrupt Pending 4 */
    bitfield  IP3       : 1; /* Interrupt Pending 3 */
    bitfield  IP2       : 1; /* Interrupt Pending 2 */
    bitfield  IP1       : 1; /* Interrupt Pending 1 */
    bitfield  IP0       : 1; /* Interrupt Pending 0 */
  } bit;
} typ_IPR;
#define reg_IPR (*(volatile typ_IPR*) (0x00C5000C))

typedef union                /* Normal Interrupt Enable Register */
{
  INT32U reg;
  struct
  {
    bitfield  NIE31     : 1;  /* Normal Interrupt Enable 31 */
    bitfield  NIE30     : 1;  /* Normal Interrupt Enable 30 */
    bitfield  NIE29     : 1;  /* Normal Interrupt Enable 29 */
    bitfield  NIE28     : 1;  /* Normal Interrupt Enable 28 */
    bitfield  NIE27     : 1;  /* Normal Interrupt Enable 27 */
    bitfield  NIE26     : 1;  /* Normal Interrupt Enable 26 */
    bitfield  NIE25     : 1;  /* Normal Interrupt Enable 25 */
    bitfield  NIE24     : 1;  /* Normal Interrupt Enable 24 */
    bitfield  NIE23     : 1;  /* Normal Interrupt Enable 23 */
    bitfield  NIE22     : 1;  /* Normal Interrupt Enable 22 */
    bitfield  NIE21     : 1;  /* Normal Interrupt Enable 21 */
    bitfield  NIE20     : 1;  /* Normal Interrupt Enable 20 */
    bitfield  NIE19     : 1;  /* Normal Interrupt Enable 19 */
    bitfield  NIE18     : 1;  /* Normal Interrupt Enable 18 */
    bitfield  NIE17     : 1;  /* Normal Interrupt Enable 17 */
    bitfield  NIE16     : 1;  /* Normal Interrupt Enable 16 */
    bitfield  NIE15     : 1;  /* Normal Interrupt Enable 15 */
    bitfield  NIE14     : 1;  /* Normal Interrupt Enable 14 */
    bitfield  NIE13     : 1;  /* Normal Interrupt Enable 13 */
    bitfield  NIE12     : 1;  /* Normal Interrupt Enable 12 */
    bitfield  NIE11     : 1;  /* Normal Interrupt Enable 11 */
    bitfield  NIE10     : 1;  /* Normal Interrupt Enable 10 */
    bitfield  NIE9      : 1;  /* Normal Interrupt Enable 9 */
    bitfield  NIE8      : 1;  /* Normal Interrupt Enable 8 */
    bitfield  NIE7      : 1;  /* Normal Interrupt Enable 7 */
    bitfield  NIE6      : 1;  /* Normal Interrupt Enable 6 */
    bitfield  NIE5      : 1;  /* Normal Interrupt Enable 5 */
    bitfield  NIE4      : 1;  /* Normal Interrupt Enable 4 */
    bitfield  NIE3      : 1;  /* Normal Interrupt Enable 3 */
    bitfield  NIE2      : 1;  /* Normal Interrupt Enable 2 */
    bitfield  NIE1      : 1;  /* Normal Interrupt Enable 1 */
    bitfield  NIE0      : 1;  /* Normal Interrupt Enable 0 */
  } bit;
} typ_NIER;
#define reg_NIER (*(volatile typ_NIER*) (0x00C50010))

typedef union                /* Normal Interrupt Pending Register */
{
  INT32U reg;
  struct
  {
    bitfield  NIP31     : 1;  /* Normal Interrupt Pending 31 */
    bitfield  NIP30     : 1;  /* Normal Interrupt Pending 30 */
    bitfield  NIP29     : 1;  /* Normal Interrupt Pending 29 */
    bitfield  NIP28     : 1;  /* Normal Interrupt Pending 28 */
    bitfield  NIP27     : 1;  /* Normal Interrupt Pending 27 */
    bitfield  NIP26     : 1;  /* Normal Interrupt Pending 26 */
    bitfield  NIP25     : 1;  /* Normal Interrupt Pending 25 */
    bitfield  NIP24     : 1;  /* Normal Interrupt Pending 24 */
    bitfield  NIP23     : 1;  /* Normal Interrupt Pending 23 */
    bitfield  NIP22     : 1;  /* Normal Interrupt Pending 22 */
    bitfield  NIP21     : 1;  /* Normal Interrupt Pending 21 */
    bitfield  NIP20     : 1;  /* Normal Interrupt Pending 20 */
    bitfield  NIP19     : 1;  /* Normal Interrupt Pending 19 */
    bitfield  NIP18     : 1;  /* Normal Interrupt Pending 18 */
    bitfield  NIP17     : 1;  /* Normal Interrupt Pending 17 */
    bitfield  NIP16     : 1;  /* Normal Interrupt Pending 16 */
    bitfield  NIP15     : 1;  /* Normal Interrupt Pending 15 */
    bitfield  NIP14     : 1;  /* Normal Interrupt Pending 14 */
    bitfield  NIP13     : 1;  /* Normal Interrupt Pending 13 */
    bitfield  NIP12     : 1;  /* Normal Interrupt Pending 12 */
    bitfield  NIP11     : 1;  /* Normal Interrupt Pending 11 */
    bitfield  NIP10     : 1;  /* Normal Interrupt Pending 10 */
    bitfield  NIP9      : 1;  /* Normal Interrupt Pending 9 */
    bitfield  NIP8      : 1;  /* Normal Interrupt Pending 8 */
    bitfield  NIP7      : 1;  /* Normal Interrupt Pending 7 */
    bitfield  NIP6      : 1;  /* Normal Interrupt Pending 6 */
    bitfield  NIP5      : 1;  /* Normal Interrupt Pending 5 */
    bitfield  NIP4      : 1;  /* Normal Interrupt Pending 4 */
    bitfield  NIP3      : 1;  /* Normal Interrupt Pending 3 */
    bitfield  NIP2      : 1;  /* Normal Interrupt Pending 2 */
    bitfield  NIP1      : 1;  /* Normal Interrupt Pending 1 */
    bitfield  NIP0      : 1;  /* Normal Interrupt Pending 0 */
  } bit;
} typ_NIPR;
#define reg_NIPR (*(volatile typ_NIPR*) (0x00C50014))

typedef union                /* Fast Interrupt Enable Register */
{
  INT32U reg;
  struct
  {
    bitfield  FIE31     : 1;  /* Fast Interrupt Enable 31 */
    bitfield  FIE30     : 1;  /* Fast Interrupt Enable 30 */
    bitfield  FIE29     : 1;  /* Fast Interrupt Enable 29 */
    bitfield  FIE28     : 1;  /* Fast Interrupt Enable 28 */
    bitfield  FIE27     : 1;  /* Fast Interrupt Enable 27 */
    bitfield  FIE26     : 1;  /* Fast Interrupt Enable 26 */
    bitfield  FIE25     : 1;  /* Fast Interrupt Enable 25 */
    bitfield  FIE24     : 1;  /* Fast Interrupt Enable 24 */
    bitfield  FIE23     : 1;  /* Fast Interrupt Enable 23 */
    bitfield  FIE22     : 1;  /* Fast Interrupt Enable 22 */
    bitfield  FIE21     : 1;  /* Fast Interrupt Enable 21 */
    bitfield  FIE20     : 1;  /* Fast Interrupt Enable 20 */
    bitfield  FIE19     : 1;  /* Fast Interrupt Enable 19 */
    bitfield  FIE18     : 1;  /* Fast Interrupt Enable 18 */
    bitfield  FIE17     : 1;  /* Fast Interrupt Enable 17 */
    bitfield  FIE16     : 1;  /* Fast Interrupt Enable 16 */
    bitfield  FIE15     : 1;  /* Fast Interrupt Enable 15 */
    bitfield  FIE14     : 1;  /* Fast Interrupt Enable 14 */
    bitfield  FIE13     : 1;  /* Fast Interrupt Enable 13 */
    bitfield  FIE12     : 1;  /* Fast Interrupt Enable 12 */
    bitfield  FIE11     : 1;  /* Fast Interrupt Enable 11 */
    bitfield  FIE10     : 1;  /* Fast Interrupt Enable 10 */
    bitfield  FIE9      : 1;  /* Fast Interrupt Enable 9 */
    bitfield  FIE8      : 1;  /* Fast Interrupt Enable 8 */
    bitfield  FIE7      : 1;  /* Fast Interrupt Enable 7 */
    bitfield  FIE6      : 1;  /* Fast Interrupt Enable 6 */
    bitfield  FIE5      : 1;  /* Fast Interrupt Enable 5 */
    bitfield  FIE4      : 1;  /* Fast Interrupt Enable 4 */
    bitfield  FIE3      : 1;  /* Fast Interrupt Enable 3 */
    bitfield  FIE2      : 1;  /* Fast Interrupt Enable 2 */
    bitfield  FIE1      : 1;  /* Fast Interrupt Enable 1 */
    bitfield  FIE0      : 1;  /* Fast Interrupt Enable 0 */
  } bit;
} typ_FIER;
#define reg_FIER (*(volatile typ_FIER*) (0x00C50018))

typedef union                /* Fast Interrupt Pending Register */
{
  INT32U reg;
  struct
  {
    bitfield  FIP31     : 1;  /* Fast Interrupt Pending 31 */
    bitfield  FIP30     : 1;  /* Fast Interrupt Pending 30 */
    bitfield  FIP29     : 1;  /* Fast Interrupt Pending 29 */
    bitfield  FIP28     : 1;  /* Fast Interrupt Pending 28 */
    bitfield  FIP27     : 1;  /* Fast Interrupt Pending 27 */
    bitfield  FIP26     : 1;  /* Fast Interrupt Pending 26 */
    bitfield  FIP25     : 1;  /* Fast Interrupt Pending 25 */
    bitfield  FIP24     : 1;  /* Fast Interrupt Pending 24 */
    bitfield  FIP23     : 1;  /* Fast Interrupt Pending 23 */
    bitfield  FIP22     : 1;  /* Fast Interrupt Pending 22 */
    bitfield  FIP21     : 1;  /* Fast Interrupt Pending 21 */
    bitfield  FIP20     : 1;  /* Fast Interrupt Pending 20 */
    bitfield  FIP19     : 1;  /* Fast Interrupt Pending 19 */
    bitfield  FIP18     : 1;  /* Fast Interrupt Pending 18 */
    bitfield  FIP17     : 1;  /* Fast Interrupt Pending 17 */
    bitfield  FIP16     : 1;  /* Fast Interrupt Pending 16 */
    bitfield  FIP15     : 1;  /* Fast Interrupt Pending 15 */
    bitfield  FIP14     : 1;  /* Fast Interrupt Pending 14 */
    bitfield  FIP13     : 1;  /* Fast Interrupt Pending 13 */
    bitfield  FIP12     : 1;  /* Fast Interrupt Pending 12 */
    bitfield  FIP11     : 1;  /* Fast Interrupt Pending 11 */
    bitfield  FIP10     : 1;  /* Fast Interrupt Pending 10 */
    bitfield  FIP9      : 1;  /* Fast Interrupt Pending 9 */
    bitfield  FIP8      : 1;  /* Fast Interrupt Pending 8 */
    bitfield  FIP7      : 1;  /* Fast Interrupt Pending 7 */
    bitfield  FIP6      : 1;  /* Fast Interrupt Pending 6 */
    bitfield  FIP5      : 1;  /* Fast Interrupt Pending 5 */
    bitfield  FIP4      : 1;  /* Fast Interrupt Pending 4 */
    bitfield  FIP3      : 1;  /* Fast Interrupt Pending 3 */
    bitfield  FIP2      : 1;  /* Fast Interrupt Pending 2 */
    bitfield  FIP1      : 1;  /* Fast Interrupt Pending 1 */
    bitfield  FIP0      : 1;  /* Fast Interrupt Pending 0 */
  } bit;
} typ_FIPR;
#define reg_FIPR (*(volatile typ_FIPR*) (0x00C5001C))

                             /* Priority Level Select Registers */
#define reg_PLSR0 (*(volatile INT8U*) (0x00C50040))	  /* QADC Queue 1 conversion pause */
#define reg_PLSR1 (*(volatile INT8U*) (0x00C50041))	  /* QADC Queue 1 conversion complete */
#define reg_PLSR2 (*(volatile INT8U*) (0x00C50042))	  /* QADC Queue 2 conversion pause */
#define reg_PLSR3 (*(volatile INT8U*) (0x00C50043))	  /* QADC Queue 2 conversion complete */
#define reg_PLSR4 (*(volatile INT8U*) (0x00C50044))	  /* SPI Mode fault */
#define reg_PLSR5 (*(volatile INT8U*) (0x00C50045))	  /* SPI Transfer complete */
#define reg_PLSR6 (*(volatile INT8U*) (0x00C50046))	  /* SCI1 Transmit data register empty */
#define reg_PLSR7 (*(volatile INT8U*) (0x00C50047))	  /* SCI1 Transmit complete */
#define reg_PLSR8 (*(volatile INT8U*) (0x00C50048))	  /* SCI1 Receive data register full */
#define reg_PLSR9 (*(volatile INT8U*) (0x00C50049))	  /* SCI1 Receiver overrun */
#define reg_PLSR10 (*(volatile INT8U*) (0x00C5004A))	/* SCI1 Receiver line idle */
#define reg_PLSR11 (*(volatile INT8U*) (0x00C5004B))	/* SCI2 Transmit data register empty */
#define reg_PLSR12 (*(volatile INT8U*) (0x00C5004C))	/* SCI2 Transmit complete */
#define reg_PLSR13 (*(volatile INT8U*) (0x00C5004D))	/* SCI2 Receive data register full */
#define reg_PLSR14 (*(volatile INT8U*) (0x00C5004E))	/* SCI2 Receiver overrun */
#define reg_PLSR15 (*(volatile INT8U*) (0x00C5004F))	/* SCI2 Receiver line idle */
#define reg_PLSR16 (*(volatile INT8U*) (0x00C50050))	/* TIM1 Timer channel 0*/
#define reg_PLSR17 (*(volatile INT8U*) (0x00C50051))	/* TIM1 Timer channel 1 */
#define reg_PLSR18 (*(volatile INT8U*) (0x00C50052))	/* TIM1 Timer channel 2 */
#define reg_PLSR19 (*(volatile INT8U*) (0x00C50053))	/* TIM1 Timer channel 3 */
#define reg_PLSR20 (*(volatile INT8U*) (0x00C50054))	/* TIM1 Timer overflow */
#define reg_PLSR21 (*(volatile INT8U*) (0x00C50055))	/* TIM1 Pulse accumulator input */
#define reg_PLSR22 (*(volatile INT8U*) (0x00C50056))	/* TIM1 Pulse accumulator overflow */
#define reg_PLSR23 (*(volatile INT8U*) (0x00C50057))	/* TIM2 Timer channel 0 */
#define reg_PLSR24 (*(volatile INT8U*) (0x00C50058))	/* TIM2 Timer channel 1  */
#define reg_PLSR25 (*(volatile INT8U*) (0x00C50059))	/* TIM2 Timer channel 2  */
#define reg_PLSR26 (*(volatile INT8U*) (0x00C5005A))	/* TIM2 Timer channel 3  */
#define reg_PLSR27 (*(volatile INT8U*) (0x00C5005B))	/* TIM2 Timer overflow */
#define reg_PLSR28 (*(volatile INT8U*) (0x00C5005C))	/* TIM2 Pulse accumulator input */
#define reg_PLSR29 (*(volatile INT8U*) (0x00C5005D))	/* TIM2 Pulse accumulator overflow */
#define reg_PLSR30 (*(volatile INT8U*) (0x00C5005E))	/* PIT1 interrupt flag */
#define reg_PLSR31 (*(volatile INT8U*) (0x00C5005F))	/* PIT2 interrupt flag */
#define reg_PLSR32 (*(volatile INT8U*) (0x00C50060))	/* Edge port flag 0 / LVD interrupt */
#define reg_PLSR33 (*(volatile INT8U*) (0x00C50061))	/* Edge port flag 1 / SGFM Command Complete (CCIF) / SGFM Command Buffer Empty (CBEIF) */
#define reg_PLSR34 (*(volatile INT8U*) (0x00C50062))	/* Edge port flag 2 */
#define reg_PLSR35 (*(volatile INT8U*) (0x00C50063))	/* Edge port flag 3 */
#define reg_PLSR36 (*(volatile INT8U*) (0x00C50064))	/* Edge port flag 4 */
#define reg_PLSR37 (*(volatile INT8U*) (0x00C50065))	/* Edge port flag 5 */
#define reg_PLSR38 (*(volatile INT8U*) (0x00C50066))	/* Edge port flag 6 */
#define reg_PLSR39 (*(volatile INT8U*) (0x00C50067))	/* Edge port flag 7 */

#define reg_PLSR_0_3 (*(volatile INT32U*) (0x00C50040))
#define reg_PLSR_4_7 (*(volatile INT32U*) (0x00C50044))
#define reg_PLSR_8_11 (*(volatile INT32U*) (0x00C50048))
#define reg_PLSR_12_15 (*(volatile INT32U*) (0x00C5004C))
#define reg_PLSR_16_19 (*(volatile INT32U*) (0x00C50050))
#define reg_PLSR_20_23 (*(volatile INT32U*) (0x00C50054))
#define reg_PLSR_24_27 (*(volatile INT32U*) (0x00C50058))
#define reg_PLSR_28_31 (*(volatile INT32U*) (0x00C5005C))
#define reg_PLSR_32_35 (*(volatile INT32U*) (0x00C50060))
#define reg_PLSR_36_39 (*(volatile INT32U*) (0x00C50064))

/*  Edge Port Module  ***************************************************************************
*
*  Address    Use                                                 Access
*  00C6_0000  reg_EPPAR   EPORT Pin Assignment Register           Supervisor Only
*  00C6_0002  reg_EPDDR   EPORT Data Direction Register           Supervisor Only
*  00C6_0003  reg_EPIER   EPORT Interrupt Enable Register         Supervisor Only
*  00C6_0004  reg_EPDR    EPORT Port Data Register                Supervisor / User
*  00C6_0005  reg_EPPDR   EPORT Port Pin Data Register            Supervisor / User
*  00C6_0006  reg_EPFR    EPORT Port Flag Register                Supervisor / User
*  00C6_0007         reserved
*  00C6_0008  ...      not used
*  00C6_FFFF
*/

typedef union                /* EPORT Pin Assignment Register */
{
  INT16U reg;
  struct
  {
    bitfield  EPPA7     : 2;  /* EPORT Pin Assignment Select 7 */
    bitfield  EPPA6     : 2;  /* EPORT Pin Assignment Select 6 */
    bitfield  EPPA5     : 2;  /* EPORT Pin Assignment Select 5 */
    bitfield  EPPA4     : 2;  /* EPORT Pin Assignment Select 4 */
    bitfield  EPPA3     : 2;  /* EPORT Pin Assignment Select 3 */
    bitfield  EPPA2     : 2;  /* EPORT Pin Assignment Select 2 */
    bitfield  EPPA1     : 2;  /* EPORT Pin Assignment Select 1 */
    bitfield  EPPA0     : 2;  /* EPORT Pin Assignment Select 0 */
  } bit;
} typ_EPPAR;
#define reg_EPPAR (*(volatile typ_EPPAR*) (0x00C60000))

typedef union                /* EPORT Data Direction Register */
{
  INT8U reg;
  struct
  {
    bitfield  EPDD7     : 1;  /* EPORT Data Direction Bit 7 */
    bitfield  EPDD6     : 1;  /* EPORT Data Direction Bit 6 */
    bitfield  EPDD5     : 1;  /* EPORT Data Direction Bit 5 */
    bitfield  EPDD4     : 1;  /* EPORT Data Direction Bit 4 */
    bitfield  EPDD3     : 1;  /* EPORT Data Direction Bit 3 */
    bitfield  EPDD2     : 1;  /* EPORT Data Direction Bit 2 */
    bitfield  EPDD1     : 1;  /* EPORT Data Direction Bit 1 */
    bitfield  EPDD0     : 1;  /* EPORT Data Direction Bit 0 */
  } bit;
} typ_EPDDR;
#define reg_EPDDR (*(volatile typ_EPDDR*) (0x00C60002))

typedef union                /* EPORT Interrupt Enable Register */
{
  INT8U reg;
  struct
  {
    bitfield  EPIE7     : 1;  /* EPORT Interrupt Enable Bit 7 */
    bitfield  EPIE6     : 1;  /* EPORT Interrupt Enable Bit 6 */
    bitfield  EPIE5     : 1;  /* EPORT Interrupt Enable Bit 5 */
    bitfield  EPIE4     : 1;  /* EPORT Interrupt Enable Bit 4 */
    bitfield  EPIE3     : 1;  /* EPORT Interrupt Enable Bit 3 */
    bitfield  EPIE2     : 1;  /* EPORT Interrupt Enable Bit 2 */
    bitfield  EPIE1     : 1;  /* EPORT Interrupt Enable Bit 1 */
    bitfield  EPIE0     : 1;  /* EPORT Interrupt Enable Bit 0 */
  } bit;
} typ_EPIER;
#define reg_EPIER (*(volatile typ_EPIER*) (0x00C60003))

typedef union                /* EPORT Port Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  EPD7      : 1;  /* EPORT Data Bit 7 */
    bitfield  EPD6      : 1;  /* EPORT Data Bit 6 */
    bitfield  EPD5      : 1;  /* EPORT Data Bit 5 */
    bitfield  EPD4      : 1;  /* EPORT Data Bit 4 */
    bitfield  EPD3      : 1;  /* EPORT Data Bit 3 */
    bitfield  EPD2      : 1;  /* EPORT Data Bit 2 */
    bitfield  EPD1      : 1;  /* EPORT Data Bit 1 */
    bitfield  EPD0      : 1;  /* EPORT Data Bit 0 */
  } bit;
} typ_EPDR;
#define reg_EPDR (*(volatile typ_EPDR*) (0x00C60004))

typedef union                /* EPORT Port Pin Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  EPPD7     : 1;  /* EPORT Pin Data Bit 7 */
    bitfield  EPPD6     : 1;  /* EPORT Pin Data Bit 6 */
    bitfield  EPPD5     : 1;  /* EPORT Pin Data Bit 5 */
    bitfield  EPPD4     : 1;  /* EPORT Pin Data Bit 4 */
    bitfield  EPPD3     : 1;  /* EPORT Pin Data Bit 3 */
    bitfield  EPPD2     : 1;  /* EPORT Pin Data Bit 2 */
    bitfield  EPPD1     : 1;  /* EPORT Pin Data Bit 1 */
    bitfield  EPPD0     : 1;  /* EPORT Pin Data Bit 0 */
  } bit;
} typ_EPPDR;
#define reg_EPPDR (*(volatile typ_EPPDR*) (0x00C60005))

typedef union                /* EPORT Port Flag Register */
{
  INT8U reg;
  struct
  {
    bitfield  EPF7      : 1;  /* EPORT Flag Bit 7 */
    bitfield  EPF6      : 1;  /* EPORT Flag Bit 6 */
    bitfield  EPF5      : 1;  /* EPORT Flag Bit 5 */
    bitfield  EPF4      : 1;  /* EPORT Flag Bit 4 */
    bitfield  EPF3      : 1;  /* EPORT Flag Bit 3 */
    bitfield  EPF2      : 1;  /* EPORT Flag Bit 2 */
    bitfield  EPF1      : 1;  /* EPORT Flag Bit 1 */
    bitfield  EPF0      : 1;  /* EPORT Flag Bit 0 */
  } bit;
} typ_EPFR;
#define reg_EPFR (*(volatile typ_EPFR*) (0x00C60006))

/*  Watchdog Timer Module  **********************************************************************
*
*  Address   Use                                               Access
*  00C7_0000  reg_WCR    Watchdog Control Register             Supervisor Only
*  00C7_0002  reg_WMR    Watchdog Modulus Register             Supervisor Only
*  00C7_0004  reg_WCNTR  Watchdog Count Register               Supervisor / User
*  00C7_0006  reg_WSR    Watchdog Service Register             Supervisor / User
*  00C7_0008  ...      not used
*  00C7_FFFF
*/

typedef union                /* Watchdog Control Register */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 12;
    bitfield  WAIT      : 1;  /* Wait Mode */
    bitfield  DOZE      : 1;  /* Doze Mode */
    bitfield  DBG       : 1;  /* Debug Mode */
    bitfield  EN        : 1;  /* Watchdog Enable */
  } bit;
} typ_WCR;
#define reg_WCR (*(volatile typ_WCR*) (0x00C70000))

                             /* Watchdog Modulus Register */
#define reg_WMR (*(volatile INT16U*) (0x00C70002))

                             /* Watchdog Count Register */
#define reg_WCNTR (*(volatile INT16U*) (0x00C70004))

                             /* Watchdog Service Register */
#define reg_WSR (*(volatile INT16U*) (0x00C70006))

/*  Programmable Interrupt Timer Modules  *******************************************************
*
*  Address    Use                                                 Access
*  00C8_0000  reg_PCSR1   PIT1 Control and Status Register        Supervisor Only
*  00C8_0002  reg_PMR1    PIT1 Modulus Register                   Supervisor Only
*  00C8_0004  reg_PCNTR1  PIT1 Count Register                     Supervisor / User
*  00C8_0006  ...      not used
*  00C8_0007
*  00C8_0008  ...      not used
*  00C8_FFFF
*
*  00C9_0000  reg_PCSR2   PIT2 Control and Status Register        Supervisor Only
*  00C9_0002  reg_PMR2    PIT2 Modulus Register                   Supervisor Only
*  00C9_0004  reg_PCNTR2  PIT2 Count Register                     Supervisor / User
*  00C9_0006  ...      not used
*  00C9_0007
*  00C9_0008  ...      not used
*  00C9_FFFF
*/

typedef union                /* PIT Control and Status Register */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  PRE       : 4;  /* Prescaler */
    bitfield  unused1   : 1;
    bitfield  PDOZE     : 1;  /* Doze Mode */
    bitfield  PDBG      : 1;  /* Debug Mode */
    bitfield  OVW       : 1;  /* Overwrite */
    bitfield  PIE       : 1;  /* PIT Interrupt Enable */
    bitfield  PIF       : 1;  /* PIT Interrupt Flag */
    bitfield  RLD       : 1;  /* Reload */
    bitfield  EN        : 1;  /* PIT Enable */
  } bit;
} typ_PCSR;
                             /* PIT1 Control and Status Register */
#define reg_PCSR1 (*(volatile typ_PCSR*) (0x00C80000))
                             /* PIT2 Control and Status Register */
#define reg_PCSR2 (*(volatile typ_PCSR*) (0x00C90000))

                             /* PIT1 Modulus Register */
#define reg_PMR1 (*(volatile INT16U*) (0x00C80002))
                             /* PIT2 Modulus Register */
#define reg_PMR2 (*(volatile INT16U*) (0x00C90002))

                             /* PIT1 Count Register */
#define reg_PCNTR1 (*(volatile INT16U*) (0x00C80004))
                             /* PIT2 Count Register */
#define reg_PCNTR2 (*(volatile INT16U*) (0x00C90004))

/*  Queued Analog-to-Digital Converter Module  **************************************************
*
*  Address    Use                                                    Access
*  00CA_0000  reg_QADCMCR  QADC Module Configuration Register        Supervisor Only
*  00CA_0002  ...      reserved
*  00CA_0005
*  00CA_0006  reg_PORTQA  QADC Port A Data Register                  Supervisor / User
*  00CA_0007  reg_PORTQB  QADC Port B Data Register                  Supervisor / User
*  00CA_0008  reg_DDRQA   QADC Port A Data Direction Register        Supervisor / User
*  00CA_0009  reg_DDRQB   QADC Port B Data Direction Register        Supervisor / User
*  00CA_000A  reg_QACR0   QADC Control Register 0                    Supervisor / User
*  00CA_000C  reg_QACR1   QADC Control Register 1                    Supervisor / User
*  00CA_000E  reg_QACR2   QADC Control Register 2                    Supervisor / User
*  00CA_0010  reg_QASR0   QADC Status Register 0                     Supervisor / User
*  00CA_0012  reg_QASR1   QADC Status Register 1                     Supervisor / User
*  00CA_0014  ...      reserved
*  00CA_01FF
*  00CA_0200  reg_CCW ...  Conversion Command Word Table             Supervisor / User
*  00CA_027F
*  00CA_0280  reg_RJURR ... Right-Justified Unsigned Result Table    Supervisor / User
*  00CA_02FF
*  00CA_0300  reg_LJSRR ... Left-Justified Signed Result Table       Supervisor / User
*  00CA_037F
*  00CA_0380  reg_LJURR ... Left-Justified Unsigned Result Table     Supervisor / User
*  00CA_03FF
*  00CA_0400  ...      not used
*  00CA_FFFF
*/

typedef union                /* QADC Module Configuration Register */
{
  INT16U reg;
  struct
  {
    bitfield  QSTOP     : 1;  /* Stop Enable */
    bitfield  QDBG      : 1;  /* Debug Enable */
    bitfield  unused    : 6;
    bitfield  SUPV      : 1;  /* Supervisor/Unrestricted Data Space */
    bitfield  unused1   : 7;
  } bit;
} typ_QADCMCR;
#define reg_QADCMCR (*(volatile typ_QADCMCR*) (0x00CA0000))

typedef union                /* QADC Port A Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 3;
    bitfield  PQA4      : 1;  /* Port QA Data Bit 4 */
    bitfield  PQA3      : 1;  /* Port QA Data Bit 3 */
    bitfield  unused1   : 1;
    bitfield  PQA1      : 1;  /* Port QA Data Bit 1 */
    bitfield  PQA0      : 1;  /* Port QA Data Bit 0 */
  } bit;
} typ_PORTQA;
#define reg_PORTQA (*(volatile typ_PORTQA*) (0x00CA0006))

typedef union                /* QADC Port B Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  PQB3      : 1;  /* Port QB Data Bit 3 */
    bitfield  PQB2      : 1;  /* Port QB Data Bit 2 */
    bitfield  PQB1      : 1;  /* Port QB Data Bit 1 */
    bitfield  PQB0      : 1;  /* Port QB Data Bit 0 */
  } bit;
} typ_PORTQB;
#define reg_PORTQB (*(volatile typ_PORTQB*) (0x00CA0007))

typedef union                /* QADC Port A Data Direction Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 3;
    bitfield  DDQA4     : 1;  /* Port QA Data Direction Bit 4 */
    bitfield  DDQA3     : 1;  /* Port QA Data Direction Bit 3 */
    bitfield  unused1   : 1;
    bitfield  DDQA1     : 1;  /* Port QA Data Direction Bit 1 */
    bitfield  DDQA0     : 1;  /* Port QA Data Direction Bit 0 */
  } bit;
} typ_DDRQA;
#define reg_DDRQA (*(volatile typ_DDRQA*) (0x00CA0008))

typedef union                /* QADC Port A Data Direction Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  DDQB3     : 1;  /* Port QB Data Direction Bit 3 */
    bitfield  DDQB2     : 1;  /* Port QB Data Direction Bit 2 */
    bitfield  DDQB1     : 1;  /* Port QB Data Direction Bit 1 */
    bitfield  DDQB0     : 1;  /* Port QB Data Direction Bit 0 */
  } bit;
} typ_DDRQB;
#define reg_DDRQB (*(volatile typ_DDRQB*) (0x00CA0009))

typedef union                /* QADC Control Register 0 */
{
  INT16U reg;
  struct
  {
    bitfield  MUX       : 1;  /* Externally Multiplexed Mode */
    bitfield  unused    : 2;
    bitfield  TRG       : 1;  /* Trigger Assignment */
    bitfield  unused1   : 5;
    bitfield  QPR       : 7;  /* Prescaler */
  } bit;
} typ_QACR0;
#define reg_QACR0 (*(volatile typ_QACR0*) (0x00CA000A))

typedef union                /* QADC Control Register 1 */
{
  INT16U reg;
  struct
  {
    bitfield  CIE1      : 1;  /* Queue 1 Completion Interrupt Enable */
    bitfield  PIE1      : 1;  /* Queue 1 Pause Interrupt Enable */
    bitfield  SSE1      : 1;  /* Queue 1 Single-Scan Enable */
    bitfield  MQ1       : 5;  /* Queue 1 Operating Mode */
    bitfield  unused    : 8;
  } bit;
} typ_QACR1;
#define reg_QACR1 (*(volatile typ_QACR1*) (0x00CA000C))

typedef union                /* QADC Control Register 2 */
{
  INT16U reg;
  struct
  {
    bitfield  CIE2      : 1;  /* Queue 2 Completion Interrupt Enable */
    bitfield  PIE2      : 1;  /* Queue 2 Pause Interrupt Enable */
    bitfield  SSE2      : 1;  /* Queue 2 Single-Scan Enable */
    bitfield  MQ2       : 5;  /* Queue 2 Operating Mode */
    bitfield  RESUME    : 1;  /* Queue 2 Resume */
    bitfield  BQ        : 7;  /* Beginning of Queue 2 */
  } bit;
} typ_QACR2;
#define reg_QACR2 (*(volatile typ_QACR2*) (0x00CA000E))

typedef union                /* QADC Status Register 0 */
{
  INT16U reg;
  struct
  {
    bitfield  CF1       : 1;  /* Queue 1 Completion Flag */
    bitfield  PF1       : 1;  /* Queue 1 Pause Flag */
    bitfield  CF2       : 1;  /* Queue 2 Completion Flag */
    bitfield  PF2       : 1;  /* Queue 2 Pause Flag */
    bitfield  TOR1      : 1;  /* Queue 1 Trigger Overrun */
    bitfield  TOR2      : 1;  /* Queue 2 Trigger Overrun */
    bitfield  QS        : 4;  /* Queue Status */
    bitfield  CWP       : 6;  /* Command Word Pointer */
  } bit;
} typ_QASR0;
#define reg_QASR0 (*(volatile typ_QASR0*) (0x00CA0010))

typedef union                /* QADC Status Register 1 */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 2;
    bitfield  CWPQ1     : 6;  /* Queue 1 Command Word Pointer */
    bitfield  unused1   : 2;
    bitfield  CWPQ2     : 6;  /* Queue 2 Command Word Pointer */
  } bit;
} typ_QASR1;
#define reg_QASR1 (*(volatile typ_QASR1*) (0x00CA0012))

typedef union                /* Conversion Command Word */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 6;
    bitfield  P         : 1;  /* Pause */
    bitfield  BYP       : 1;  /* Sample Amplifier Bypass */
    bitfield  IST       : 2;  /* Input Sample Time */
    bitfield  CHAN      : 6;  /* Channel Number */
  } bit;
} typ_CCW;
#define reg_CCW0 (*(volatile typ_CCW*)(0x00ca0200))
#define reg_CCW1 (*(volatile typ_CCW*)(0x00ca0202))
#define reg_CCW2 (*(volatile typ_CCW*)(0x00ca0204))
#define reg_CCW3 (*(volatile typ_CCW*)(0x00ca0206))
#define reg_CCW4 (*(volatile typ_CCW*)(0x00ca0208))
#define reg_CCW5 (*(volatile typ_CCW*)(0x00ca020a))
#define reg_CCW6 (*(volatile typ_CCW*)(0x00ca020c))
#define reg_CCW7 (*(volatile typ_CCW*)(0x00ca020e))
#define reg_CCW8 (*(volatile typ_CCW*)(0x00ca0210))
#define reg_CCW9 (*(volatile typ_CCW*)(0x00ca0212))
#define reg_CCW10 (*(volatile typ_CCW*)(0x00ca0214))
#define reg_CCW11 (*(volatile typ_CCW*)(0x00ca0216))
#define reg_CCW12 (*(volatile typ_CCW*)(0x00ca0218))
#define reg_CCW13 (*(volatile typ_CCW*)(0x00ca021a))
#define reg_CCW14 (*(volatile typ_CCW*)(0x00ca021c))
#define reg_CCW15 (*(volatile typ_CCW*)(0x00ca021e))
#define reg_CCW16 (*(volatile typ_CCW*)(0x00ca0220))
#define reg_CCW17 (*(volatile typ_CCW*)(0x00ca0222))
#define reg_CCW18 (*(volatile typ_CCW*)(0x00ca0224))
#define reg_CCW19 (*(volatile typ_CCW*)(0x00ca0226))
#define reg_CCW20 (*(volatile typ_CCW*)(0x00ca0228))
#define reg_CCW21 (*(volatile typ_CCW*)(0x00ca022a))
#define reg_CCW22 (*(volatile typ_CCW*)(0x00ca022c))
#define reg_CCW23 (*(volatile typ_CCW*)(0x00ca022e))
#define reg_CCW24 (*(volatile typ_CCW*)(0x00ca0230))
#define reg_CCW25 (*(volatile typ_CCW*)(0x00ca0232))
#define reg_CCW26 (*(volatile typ_CCW*)(0x00ca0234))
#define reg_CCW27 (*(volatile typ_CCW*)(0x00ca0236))
#define reg_CCW28 (*(volatile typ_CCW*)(0x00ca0238))
#define reg_CCW29 (*(volatile typ_CCW*)(0x00ca023a))
#define reg_CCW30 (*(volatile typ_CCW*)(0x00ca023c))
#define reg_CCW31 (*(volatile typ_CCW*)(0x00ca023e))
#define reg_CCW32 (*(volatile typ_CCW*)(0x00ca0240))
#define reg_CCW33 (*(volatile typ_CCW*)(0x00ca0242))
#define reg_CCW34 (*(volatile typ_CCW*)(0x00ca0244))
#define reg_CCW35 (*(volatile typ_CCW*)(0x00ca0246))
#define reg_CCW36 (*(volatile typ_CCW*)(0x00ca0248))
#define reg_CCW37 (*(volatile typ_CCW*)(0x00ca024a))
#define reg_CCW38 (*(volatile typ_CCW*)(0x00ca024c))
#define reg_CCW39 (*(volatile typ_CCW*)(0x00ca024e))
#define reg_CCW40 (*(volatile typ_CCW*)(0x00ca0250))
#define reg_CCW41 (*(volatile typ_CCW*)(0x00ca0252))
#define reg_CCW42 (*(volatile typ_CCW*)(0x00ca0254))
#define reg_CCW43 (*(volatile typ_CCW*)(0x00ca0256))
#define reg_CCW44 (*(volatile typ_CCW*)(0x00ca0258))
#define reg_CCW45 (*(volatile typ_CCW*)(0x00ca025a))
#define reg_CCW46 (*(volatile typ_CCW*)(0x00ca025c))
#define reg_CCW47 (*(volatile typ_CCW*)(0x00ca025e))
#define reg_CCW48 (*(volatile typ_CCW*)(0x00ca0260))
#define reg_CCW49 (*(volatile typ_CCW*)(0x00ca0262))
#define reg_CCW50 (*(volatile typ_CCW*)(0x00ca0264))
#define reg_CCW51 (*(volatile typ_CCW*)(0x00ca0266))
#define reg_CCW52 (*(volatile typ_CCW*)(0x00ca0268))
#define reg_CCW53 (*(volatile typ_CCW*)(0x00ca026a))
#define reg_CCW54 (*(volatile typ_CCW*)(0x00ca026c))
#define reg_CCW55 (*(volatile typ_CCW*)(0x00ca026e))
#define reg_CCW56 (*(volatile typ_CCW*)(0x00ca0270))
#define reg_CCW57 (*(volatile typ_CCW*)(0x00ca0272))
#define reg_CCW58 (*(volatile typ_CCW*)(0x00ca0274))
#define reg_CCW59 (*(volatile typ_CCW*)(0x00ca0276))
#define reg_CCW60 (*(volatile typ_CCW*)(0x00ca0278))
#define reg_CCW61 (*(volatile typ_CCW*)(0x00ca027a))
#define reg_CCW62 (*(volatile typ_CCW*)(0x00ca027c))
#define reg_CCW63 (*(volatile typ_CCW*)(0x00ca027e))

typedef union                /* Right-Justified Unsigned Result Table */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 6;
    bitfield  RESULT    : 10; /* Result */
  } bit;
} typ_RJURR;
#define reg_RJURR0 (*(volatile typ_RJURR*) (0x00ca0280))
#define reg_RJURR1 (*(volatile typ_RJURR*) (0x00ca0282))
#define reg_RJURR2 (*(volatile typ_RJURR*) (0x00ca0284))
#define reg_RJURR3 (*(volatile typ_RJURR*) (0x00ca0286))
#define reg_RJURR4 (*(volatile typ_RJURR*) (0x00ca0288))
#define reg_RJURR5 (*(volatile typ_RJURR*) (0x00ca028a))
#define reg_RJURR6 (*(volatile typ_RJURR*) (0x00ca028c))
#define reg_RJURR7 (*(volatile typ_RJURR*) (0x00ca028e))
#define reg_RJURR8 (*(volatile typ_RJURR*) (0x00ca0290))
#define reg_RJURR9 (*(volatile typ_RJURR*) (0x00ca0292))
#define reg_RJURR10 (*(volatile typ_RJURR*) (0x00ca0294))
#define reg_RJURR11 (*(volatile typ_RJURR*) (0x00ca0296))
#define reg_RJURR12 (*(volatile typ_RJURR*) (0x00ca0298))
#define reg_RJURR13 (*(volatile typ_RJURR*) (0x00ca029a))
#define reg_RJURR14 (*(volatile typ_RJURR*) (0x00ca029c))
#define reg_RJURR15 (*(volatile typ_RJURR*) (0x00ca02ae))
#define reg_RJURR16 (*(volatile typ_RJURR*) (0x00ca02a0))
#define reg_RJURR17 (*(volatile typ_RJURR*) (0x00ca02a2))
#define reg_RJURR18 (*(volatile typ_RJURR*) (0x00ca02a4))
#define reg_RJURR19 (*(volatile typ_RJURR*) (0x00ca02a6))
#define reg_RJURR20 (*(volatile typ_RJURR*) (0x00ca02a8))
#define reg_RJURR21 (*(volatile typ_RJURR*) (0x00ca02aa))
#define reg_RJURR22 (*(volatile typ_RJURR*) (0x00ca02ac))
#define reg_RJURR23 (*(volatile typ_RJURR*) (0x00ca02ae))
#define reg_RJURR24 (*(volatile typ_RJURR*) (0x00ca02b0))
#define reg_RJURR25 (*(volatile typ_RJURR*) (0x00ca02b2))
#define reg_RJURR26 (*(volatile typ_RJURR*) (0x00ca02b4))
#define reg_RJURR27 (*(volatile typ_RJURR*) (0x00ca02b6))
#define reg_RJURR28 (*(volatile typ_RJURR*) (0x00ca02b8))
#define reg_RJURR29 (*(volatile typ_RJURR*) (0x00ca02ba))
#define reg_RJURR30 (*(volatile typ_RJURR*) (0x00ca02bc))
#define reg_RJURR31 (*(volatile typ_RJURR*) (0x00ca02be))
#define reg_RJURR32 (*(volatile typ_RJURR*) (0x00ca02c0))
#define reg_RJURR33 (*(volatile typ_RJURR*) (0x00ca02c2))
#define reg_RJURR34 (*(volatile typ_RJURR*) (0x00ca02c4))
#define reg_RJURR35 (*(volatile typ_RJURR*) (0x00ca02c6))
#define reg_RJURR36 (*(volatile typ_RJURR*) (0x00ca02c8))
#define reg_RJURR37 (*(volatile typ_RJURR*) (0x00ca02ca))
#define reg_RJURR38 (*(volatile typ_RJURR*) (0x00ca02cc))
#define reg_RJURR39 (*(volatile typ_RJURR*) (0x00ca02ce))
#define reg_RJURR40 (*(volatile typ_RJURR*) (0x00ca02d0))
#define reg_RJURR41 (*(volatile typ_RJURR*) (0x00ca02d2))
#define reg_RJURR42 (*(volatile typ_RJURR*) (0x00ca02d4))
#define reg_RJURR43 (*(volatile typ_RJURR*) (0x00ca02d6))
#define reg_RJURR44 (*(volatile typ_RJURR*) (0x00ca02d8))
#define reg_RJURR45 (*(volatile typ_RJURR*) (0x00ca02da))
#define reg_RJURR46 (*(volatile typ_RJURR*) (0x00ca02dc))
#define reg_RJURR47 (*(volatile typ_RJURR*) (0x00ca02de))
#define reg_RJURR48 (*(volatile typ_RJURR*) (0x00ca02e0))
#define reg_RJURR49 (*(volatile typ_RJURR*) (0x00ca02e2))
#define reg_RJURR50 (*(volatile typ_RJURR*) (0x00ca02e4))
#define reg_RJURR51 (*(volatile typ_RJURR*) (0x00ca02e6))
#define reg_RJURR52 (*(volatile typ_RJURR*) (0x00ca02e8))
#define reg_RJURR53 (*(volatile typ_RJURR*) (0x00ca02ea))
#define reg_RJURR54 (*(volatile typ_RJURR*) (0x00ca02ec))
#define reg_RJURR55 (*(volatile typ_RJURR*) (0x00ca02ee))
#define reg_RJURR56 (*(volatile typ_RJURR*) (0x00ca02f0))
#define reg_RJURR57 (*(volatile typ_RJURR*) (0x00ca02f2))
#define reg_RJURR58 (*(volatile typ_RJURR*) (0x00ca02f4))
#define reg_RJURR59 (*(volatile typ_RJURR*) (0x00ca02f6))
#define reg_RJURR60 (*(volatile typ_RJURR*) (0x00ca02f8))
#define reg_RJURR61 (*(volatile typ_RJURR*) (0x00ca02fa))
#define reg_RJURR62 (*(volatile typ_RJURR*) (0x00ca02fc))
#define reg_RJURR63 (*(volatile typ_RJURR*) (0x00ca02fe))



typedef union                /* Left-Justified Signed Result Table */
{
  INT16U reg;
  struct
  {
    bitfield  S           : 1;  /* Sign */
    bitfield  RESULT      : 10; /* Result */
    bitfield  unused      : 5;
  } bit;
} typ_LJSRR;
#define reg_LJSRR0 (*(volatile typ_LJSRR*)(0x00ca0300))
#define reg_LJSRR1 (*(volatile typ_LJSRR*)(0x00ca0302))
#define reg_LJSRR2 (*(volatile typ_LJSRR*)(0x00ca0304))
#define reg_LJSRR3 (*(volatile typ_LJSRR*)(0x00ca0306))
#define reg_LJSRR4 (*(volatile typ_LJSRR*)(0x00ca0308))
#define reg_LJSRR5 (*(volatile typ_LJSRR*)(0x00ca030a))
#define reg_LJSRR6 (*(volatile typ_LJSRR*)(0x00ca030c))
#define reg_LJSRR7 (*(volatile typ_LJSRR*)(0x00ca030e))
#define reg_LJSRR8 (*(volatile typ_LJSRR*)(0x00ca0310))
#define reg_LJSRR9 (*(volatile typ_LJSRR*)(0x00ca0312))
#define reg_LJSRR10 (*(volatile typ_LJSRR*)(0x00ca0314))
#define reg_LJSRR11 (*(volatile typ_LJSRR*)(0x00ca0316))
#define reg_LJSRR12 (*(volatile typ_LJSRR*)(0x00ca0318))
#define reg_LJSRR13 (*(volatile typ_LJSRR*)(0x00ca031a))
#define reg_LJSRR14 (*(volatile typ_LJSRR*)(0x00ca031c))
#define reg_LJSRR15 (*(volatile typ_LJSRR*)(0x00ca031e))
#define reg_LJSRR16 (*(volatile typ_LJSRR*)(0x00ca0320))
#define reg_LJSRR17 (*(volatile typ_LJSRR*)(0x00ca0322))
#define reg_LJSRR18 (*(volatile typ_LJSRR*)(0x00ca0324))
#define reg_LJSRR19 (*(volatile typ_LJSRR*)(0x00ca0326))
#define reg_LJSRR20 (*(volatile typ_LJSRR*)(0x00ca0328))
#define reg_LJSRR21 (*(volatile typ_LJSRR*)(0x00ca032a))
#define reg_LJSRR22 (*(volatile typ_LJSRR*)(0x00ca032c))
#define reg_LJSRR23 (*(volatile typ_LJSRR*)(0x00ca032e))
#define reg_LJSRR24 (*(volatile typ_LJSRR*)(0x00ca0330))
#define reg_LJSRR25 (*(volatile typ_LJSRR*)(0x00ca0332))
#define reg_LJSRR26 (*(volatile typ_LJSRR*)(0x00ca0334))
#define reg_LJSRR27 (*(volatile typ_LJSRR*)(0x00ca0336))
#define reg_LJSRR28 (*(volatile typ_LJSRR*)(0x00ca0338))
#define reg_LJSRR29 (*(volatile typ_LJSRR*)(0x00ca033a))
#define reg_LJSRR30 (*(volatile typ_LJSRR*)(0x00ca033c))
#define reg_LJSRR31 (*(volatile typ_LJSRR*)(0x00ca033e))
#define reg_LJSRR32 (*(volatile typ_LJSRR*)(0x00ca0340))
#define reg_LJSRR33 (*(volatile typ_LJSRR*)(0x00ca0342))
#define reg_LJSRR34 (*(volatile typ_LJSRR*)(0x00ca0344))
#define reg_LJSRR35 (*(volatile typ_LJSRR*)(0x00ca0346))
#define reg_LJSRR36 (*(volatile typ_LJSRR*)(0x00ca0348))
#define reg_LJSRR37 (*(volatile typ_LJSRR*)(0x00ca034a))
#define reg_LJSRR38 (*(volatile typ_LJSRR*)(0x00ca034c))
#define reg_LJSRR39 (*(volatile typ_LJSRR*)(0x00ca034e))
#define reg_LJSRR40 (*(volatile typ_LJSRR*)(0x00ca0350))
#define reg_LJSRR41 (*(volatile typ_LJSRR*)(0x00ca0352))
#define reg_LJSRR42 (*(volatile typ_LJSRR*)(0x00ca0354))
#define reg_LJSRR43 (*(volatile typ_LJSRR*)(0x00ca0356))
#define reg_LJSRR44 (*(volatile typ_LJSRR*)(0x00ca0358))
#define reg_LJSRR45 (*(volatile typ_LJSRR*)(0x00ca035a))
#define reg_LJSRR46 (*(volatile typ_LJSRR*)(0x00ca035c))
#define reg_LJSRR47 (*(volatile typ_LJSRR*)(0x00ca035e))
#define reg_LJSRR48 (*(volatile typ_LJSRR*)(0x00ca0360))
#define reg_LJSRR49 (*(volatile typ_LJSRR*)(0x00ca0362))
#define reg_LJSRR50 (*(volatile typ_LJSRR*)(0x00ca0364))
#define reg_LJSRR51 (*(volatile typ_LJSRR*)(0x00ca0366))
#define reg_LJSRR52 (*(volatile typ_LJSRR*)(0x00ca0368))
#define reg_LJSRR53 (*(volatile typ_LJSRR*)(0x00ca036a))
#define reg_LJSRR54 (*(volatile typ_LJSRR*)(0x00ca036c))
#define reg_LJSRR55 (*(volatile typ_LJSRR*)(0x00ca036e))
#define reg_LJSRR56 (*(volatile typ_LJSRR*)(0x00ca0370))
#define reg_LJSRR57 (*(volatile typ_LJSRR*)(0x00ca0372))
#define reg_LJSRR58 (*(volatile typ_LJSRR*)(0x00ca0374))
#define reg_LJSRR59 (*(volatile typ_LJSRR*)(0x00ca0376))
#define reg_LJSRR60 (*(volatile typ_LJSRR*)(0x00ca0378))
#define reg_LJSRR61 (*(volatile typ_LJSRR*)(0x00ca037a))
#define reg_LJSRR62 (*(volatile typ_LJSRR*)(0x00ca037c))
#define reg_LJSRR63 (*(volatile typ_LJSRR*)(0x00ca037e))


typedef union                /* Left-Justified Signed Result Table */

{
  INT16U reg;
  struct
  {
    bitfield  RESULT    : 10; /* Result */
    bitfield  unused    : 6;
  } bit;
} typ_LJURR;

#define reg_LJURR0 (*(volatile typ_LJURR*)(0x00ca0380))
#define reg_LJURR1 (*(volatile typ_LJURR*)(0x00ca0382))
#define reg_LJURR2 (*(volatile typ_LJURR*)(0x00ca0384))
#define reg_LJURR3 (*(volatile typ_LJURR*)(0x00ca0386))
#define reg_LJURR4 (*(volatile typ_LJURR*)(0x00ca0388))
#define reg_LJURR5 (*(volatile typ_LJURR*)(0x00ca038a))
#define reg_LJURR6 (*(volatile typ_LJURR*)(0x00ca038c))
#define reg_LJURR7 (*(volatile typ_LJURR*)(0x00ca038e))
#define reg_LJURR8 (*(volatile typ_LJURR*)(0x00ca0390))
#define reg_LJURR9 (*(volatile typ_LJURR*)(0x00ca0392))
#define reg_LJURR10 (*(volatile typ_LJURR*)(0x00ca0394))
#define reg_LJURR11 (*(volatile typ_LJURR*)(0x00ca0396))
#define reg_LJURR12 (*(volatile typ_LJURR*)(0x00ca0398))
#define reg_LJURR13 (*(volatile typ_LJURR*)(0x00ca039a))
#define reg_LJURR14 (*(volatile typ_LJURR*)(0x00ca039c))
#define reg_LJURR15 (*(volatile typ_LJURR*)(0x00ca03ae))
#define reg_LJURR16 (*(volatile typ_LJURR*)(0x00ca03a0))
#define reg_LJURR17 (*(volatile typ_LJURR*)(0x00ca03a2))
#define reg_LJURR18 (*(volatile typ_LJURR*)(0x00ca03a4))
#define reg_LJURR19 (*(volatile typ_LJURR*)(0x00ca03a6))
#define reg_LJURR20 (*(volatile typ_LJURR*)(0x00ca03a8))
#define reg_LJURR21 (*(volatile typ_LJURR*)(0x00ca03aa))
#define reg_LJURR22 (*(volatile typ_LJURR*)(0x00ca03ac))
#define reg_LJURR23 (*(volatile typ_LJURR*)(0x00ca03ae))
#define reg_LJURR24 (*(volatile typ_LJURR*)(0x00ca03b0))
#define reg_LJURR25 (*(volatile typ_LJURR*)(0x00ca03b2))
#define reg_LJURR26 (*(volatile typ_LJURR*)(0x00ca03b4))
#define reg_LJURR27 (*(volatile typ_LJURR*)(0x00ca03b6))
#define reg_LJURR28 (*(volatile typ_LJURR*)(0x00ca03b8))
#define reg_LJURR29 (*(volatile typ_LJURR*)(0x00ca03ba))
#define reg_LJURR30 (*(volatile typ_LJURR*)(0x00ca03bc))
#define reg_LJURR31 (*(volatile typ_LJURR*)(0x00ca03be))
#define reg_LJURR32 (*(volatile typ_LJURR*)(0x00ca03c0))
#define reg_LJURR33 (*(volatile typ_LJURR*)(0x00ca03c2))
#define reg_LJURR34 (*(volatile typ_LJURR*)(0x00ca03c4))
#define reg_LJURR35 (*(volatile typ_LJURR*)(0x00ca03c6))
#define reg_LJURR36 (*(volatile typ_LJURR*)(0x00ca03c8))
#define reg_LJURR37 (*(volatile typ_LJURR*)(0x00ca03ca))
#define reg_LJURR38 (*(volatile typ_LJURR*)(0x00ca03cc))
#define reg_LJURR39 (*(volatile typ_LJURR*)(0x00ca03ce))
#define reg_LJURR40 (*(volatile typ_LJURR*)(0x00ca03d0))
#define reg_LJURR41 (*(volatile typ_LJURR*)(0x00ca03d2))
#define reg_LJURR42 (*(volatile typ_LJURR*)(0x00ca03d4))
#define reg_LJURR43 (*(volatile typ_LJURR*)(0x00ca03d6))
#define reg_LJURR44 (*(volatile typ_LJURR*)(0x00ca03d8))
#define reg_LJURR45 (*(volatile typ_LJURR*)(0x00ca03da))
#define reg_LJURR46 (*(volatile typ_LJURR*)(0x00ca03dc))
#define reg_LJURR47 (*(volatile typ_LJURR*)(0x00ca03de))
#define reg_LJURR48 (*(volatile typ_LJURR*)(0x00ca03e0))
#define reg_LJURR49 (*(volatile typ_LJURR*)(0x00ca03e2))
#define reg_LJURR50 (*(volatile typ_LJURR*)(0x00ca03e4))
#define reg_LJURR51 (*(volatile typ_LJURR*)(0x00ca03e6))
#define reg_LJURR52 (*(volatile typ_LJURR*)(0x00ca03e8))
#define reg_LJURR53 (*(volatile typ_LJURR*)(0x00ca03ea))
#define reg_LJURR54 (*(volatile typ_LJURR*)(0x00ca03ec))
#define reg_LJURR55 (*(volatile typ_LJURR*)(0x00ca03ee))
#define reg_LJURR56 (*(volatile typ_LJURR*)(0x00ca03f0))
#define reg_LJURR57 (*(volatile typ_LJURR*)(0x00ca03f2))
#define reg_LJURR58 (*(volatile typ_LJURR*)(0x00ca03f4))
#define reg_LJURR59 (*(volatile typ_LJURR*)(0x00ca03f6))
#define reg_LJURR60 (*(volatile typ_LJURR*)(0x00ca03f8))
#define reg_LJURR61 (*(volatile typ_LJURR*)(0x00ca03fa))
#define reg_LJURR62 (*(volatile typ_LJURR*)(0x00ca03fc))
#define reg_LJURR63 (*(volatile typ_LJURR*)(0x00ca03fe))



/*  Serial Peripheral Interface Module  *********************************************************
*
*  Address    Use                                             Access
*  00CB_0000  reg_SPICR1  SPI Control Register 1              Supervisor / User
*  00CB_0001  reg_SPICR2  SPI Control Register 2              Supervisor / User
*  00CB_0002  reg_SPIBR   SPI Baud Rate Register              Supervisor / User
*  00CB_0003  reg_SPISR   SPI Status Register                 Supervisor / User
*  00CB_0004         reserved
*  00CB_0005  reg_SPIDR   SPI Data Register                   Supervisor / User
*  00CB_0006  reg_SPUPURD SPI Pullup and Reduced Drive Register Supervisor / User
*  00CB_0007  reg_SPIPORT SPI Port Data Register              Supervisor / User
*  00CB_0008  reg_SPIDDR  SPI Port Data Direction Register    Supervisor / User
*  00CB_0009  ...      reserved
*  00CB_000F
*  00CB_0010  ...      not used
*  00CB_FFFF
*/

typedef union                /* SPI Control Register 1 */
{
  INT8U reg;
  struct
  {
    bitfield  SPIE      : 1;  /* SPI Interrupt Enable */
    bitfield  SPE       : 1;  /* SPI System Enable */
    bitfield  SWOM      : 1;  /* SPI Wired-OR Mode */
    bitfield  MSTR      : 1;  /* Master Mode */
    bitfield  CPOL      : 1;  /* Clock Polarity */
    bitfield  CPHA      : 1;  /* Clock Phase */
    bitfield  SSOE      : 1;  /* Slave Select Output Enable */
    bitfield  LSBFE     : 1;  /* LSB-First Enable */
  } bit;
} typ_SPICR1;
#define reg_SPICR1 (*(volatile typ_SPICR1*) (0x00CB0000))

typedef union                /* SPI Control Register 2 */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 6;
    bitfield  SPISDOZ   : 1;  /* SPI Stop in Doze */
    bitfield  SPC0      : 1;  /* Serial Pin Control */
  } bit;
} typ_SPICR2;
#define reg_SPICR2 (*(volatile typ_SPICR2*) (0x00CB0001))

typedef union                /* SPI Baud Rate Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 1;
    bitfield  SPPR      : 3;  /* SPI Baud Rate Preselection */
    bitfield  unused1   : 1;
    bitfield  SPR       : 3;  /* SPI Baud Rate */
  } bit;
} typ_SPIBR;
#define reg_SPIBR (*(volatile typ_SPIBR*) (0x00CB0002))

typedef union                /* SPI Status Register */
{
  INT8U reg;
  struct
  {
    bitfield  SPIF      : 1;  /* SPI Interrupt Flag */
    bitfield  WCOL      : 1;  /* Write Collision Flag */
    bitfield  unused    : 1;
    bitfield  MODF      : 1;  /* Mode Fault Flag */
    bitfield  unused1   : 4;
  } bit;
} typ_SPISR;
#define reg_SPISR (*(volatile typ_SPISR*) (0x00CB0003))

                             /* SPI Data Register */
#define reg_SPIDR (*(volatile INT8U*) (0x00CB0005)) 

typedef union                /* SPI Pullup and Reduced Drive Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 2;
    bitfield  reserved5 : 1;
    bitfield  RDSP      : 1;  /* SPI Port Reduced Drive Control */
    bitfield  unused1   : 2;
    bitfield  reserved1 : 1;
    bitfield  PUPSP     : 1;  /* SPI Port Pullup Enable */
  } bit;
} typ_SPIPURD;
#define reg_SPIPURD (*(volatile typ_SPIPURD*) (0x00CB0006))

typedef union                /* SPI Port Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  reserved  : 4;
    bitfield  PORTSP3   : 1;  /* SPI Port Data Bit 3 */
    bitfield  PORTSP2   : 1;  /* SPI Port Data Bit 2 */
    bitfield  PORTSP1   : 1;  /* SPI Port Data Bit 1 */
    bitfield  PORTSP0   : 1;  /* SPI Port Data Bit 0 */
  } bit;
} typ_SPIPORT;
#define reg_SPIPORT (*(volatile typ_SPIPORT*) (0x00CB0007))

typedef union                /* SPI Port Data Direction Register */
{
  INT8U reg;
  struct
  {
    bitfield  reserved  : 4;
    bitfield  DDRSP3    : 1;  /* Data Direction Bit 3 */
    bitfield  DDRSP2    : 1;  /* Data Direction Bit 2 */
    bitfield  DDRSP1    : 1;  /* Data Direction Bit 1 */
    bitfield  DDRSP0    : 1;  /* Data Direction Bit 0 */
  } bit;
} typ_SPIDDR;
#define reg_SPIDDR (*(volatile typ_SPIDDR*) (0x00CB0008))

/*  Serial Communications Interface Modules  ****************************************************
*
*  Address    Use                                              Access
*  00CC_0000  reg_SCI1BD   SCI1 Baud Rate Register             Supervisor / User
*  00CC_0002  reg_SCI1CR1  SCI1 Control Register 1             Supervisor / User
*  00CC_0003  reg_SCI1CR2  SCI1 Control Register 2             Supervisor / User
*  00CC_0004  reg_SCI1SR1  SCI1 Status Register 1              Supervisor / User
*  00CC_0005  reg_SCI1SR2  SCI1 Status Register 2              Supervisor / User
*  00CC_0006  reg_SCI1DRH  SCI1 Data Register High             Supervisor / User
*  00CC_0007  reg_SCI1DRL  SCI1 Data Register Low              Supervisor / User
*  00CC_0008  reg_SCI1PURD SCI1 Pullup and Reduced Drive Register Supervisor / User
*  00CC_0009  reg_SCI1PORT SCI1 Port Data Register             Supervisor / User
*  00CC_000A  reg_SCI1DDR  SCI1 Data Direction Register        Supervisor / User
*  00CC_000B  ...      reserved
*  00CC_000F
*  00CC_0010  ...      not used
*  00CC_FFFF
*
*  00CD_0000  reg_SCI2BD   SCI2 Baud Rate Register             Supervisor / User
*  00CD_0002  reg_SCI2CR1  SCI2 Control Register 1             Supervisor / User
*  00CD_0003  reg_SCI2CR2  SCI2 Control Register 2             Supervisor / User
*  00CD_0004  reg_SCI2SR1  SCI2 Status Register 1              Supervisor / User
*  00CD_0005  reg_SCI2SR2  SCI2 Status Register 2              Supervisor / User
*  00CD_0006  reg_SCI2DRH  SCI2 Data Register High             Supervisor / User
*  00CD_0007  reg_SCI2DRL  SCI2 Data Register Low              Supervisor / User
*  00CD_0008  reg_SCI2PURD SCI2 Pullup and Reduced Drive Register Supervisor / User
*  00CD_0009  reg_SCI2PORT SCI2 Port Data Register             Supervisor / User
*  00CD_000A  reg_SCI2DDR  SCI2 Data Direction Register        Supervisor / User
*  00CD_000B  ...      reserved
*  00CD_000F
*  00CD_0010  ...      not used
*  00CD_FFFF
*/

                             /* SCI1 Baud Rate Register */
#define reg_SCI1BD (*(volatile INT16U*) (0x00CC0000))
                             /* SCI2 Baud Rate Register */
#define reg_SCI2BD (*(volatile INT16U*) (0x00CD0000))

typedef union                /* SCI Control Register 1 */
{
  INT8U reg;
  struct
  {
    bitfield  LOOPS     : 1;  /* Loop Select */
    bitfield  WOMS      : 1;  /* Wired-OR Mode Select */
    bitfield  RSRC      : 1;  /* Receiver Source */
    bitfield  M         : 1;  /* Data Format Mode */
    bitfield  WAKE      : 1;  /* Wakeup */
    bitfield  ILT       : 1;  /* Idle Line Type */
    bitfield  PE        : 1;  /* Parity Enable */
    bitfield  PT        : 1;  /* Parity Type */
  } bit;
} typ_SCICR1;
                             /* SCI1 Control Register 1 */
#define reg_SCI1CR1 (*(volatile typ_SCICR1*) (0x00CC0002))
                             /* SCI2 Control Register 1 */
#define reg_SCI2CR1 (*(volatile typ_SCICR1*) (0x00CD0002))

typedef union                /* SCI Control Register 2 */
{
  INT8U reg;
  struct
  {
    bitfield  TIE       : 1;  /* Transmitter Interrupt Enable */
    bitfield  TCIE      : 1;  /* Transmission Complete Interrupt Enable */
    bitfield  RIE       : 1;  /* Receiver Interrupt Enable */
    bitfield  ILIE      : 1;  /* Idle Line Interrupt Enable */
    bitfield  TE        : 1;  /* Transmitter Enable */
    bitfield  RE        : 1;  /* Receiver Enable */
    bitfield            : 1;  /* Receiver Wakeup */
    bitfield  SBK       : 1;  /* Send Break */
  } bit;
} typ_SCICR2;
                             /* SCI1 Control Register 2 */
#define reg_SCI1CR2 (*(volatile typ_SCICR2*) (0x00CC0003))
                             /* SCI2 Control Register 2 */
#define reg_SCI2CR2 (*(volatile typ_SCICR2*) (0x00CD0003))

typedef union                /* SCI Status Register 1 */
{
  INT8U reg;
  struct
  {
    bitfield  TDRE      : 1;  /* Transmit Data Register Empty Flag */
    bitfield  TC        : 1;  /* Transmit Complete Flag */
    bitfield  RDRF      : 1;  /* Receive Data Register Full Flag */
    bitfield  IDLE      : 1;  /* Idle Line Flag */
    bitfield  OR        : 1;  /* Overrun Flag */
    bitfield  NF        : 1;  /* Noise Flag */
    bitfield  FE        : 1;  /* Framing Error Flag */
    bitfield  PF        : 1;  /* Parity Error Flag */
  } bit;
} typ_SCISR1;
                             /* SCI1 Status Register 1 */
#define reg_SCI1SR1 (*(volatile typ_SCISR1*) (0x00CC0004))
                             /* SCI2 Status Register 1 */
#define reg_SCI2SR1 (*(volatile typ_SCISR1*) (0x00CD0004))

typedef union                /* SCI Status Register 2 */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 7;
    bitfield  RAF       : 1;  /* Receiver Active Flag */
  } bit;
} typ_SCISR2;
                             /* SCI1 Status Register 2 */
#define reg_SCI1SR2 (*(volatile typ_SCISR2*) (0x00CC0005))
                             /* SCI2 Status Register 2 */
#define reg_SCI2SR2 (*(volatile typ_SCISR2*) (0x00CD0005))

typedef union                /* SCI Data Register High */
{
  INT8U reg;
  struct
  {
    bitfield  R8        : 1;  /* Receive Bit 8 */
    bitfield  T8        : 1;  /* Transmit Bit 8 */
    bitfield  unused    : 6;
  } bit;
} typ_SCIDRH;
                             /* SCI1 Data Register High */
#define reg_SCI1DRH (*(volatile typ_SCIDRH*) (0x00CC0006))
                             /* SCI2 Data Register High */
#define reg_SCI2DRH (*(volatile typ_SCIDRH*) (0x00CD0006))
                             /* SCI1 Data Register Low */
#define reg_SCI1DRL (*(volatile INT8U*) (0x00CC0007))
                             /* SCI2 Data Register Low */
#define reg_SCI2DRL (*(volatile INT8U*) (0x00CD0007))

typedef union                /* SCI Pullup and Reduced Drive Register */
{
  INT8U reg;
  struct
  {
    bitfield  SCISDOZ   : 1;  /* SCI Stop in Doze Mode */
    bitfield  unused    : 1;
    bitfield  reserved5 : 1;
    bitfield  RDPSCI    : 1;  /* Reduced Drive Control */
    bitfield  unused1   : 2;
    bitfield  reserved1 : 1;
    bitfield  PUPSCI    : 1;  /* Pullup Enable */
  } bit;
} typ_SCIPURD;
                             /* SCI1 Control Register 2 */
#define reg_SCI1PURD (*(volatile typ_SCIPURD*) (0x00CC0008))
                             /* SCI2 Control Register 2 */
#define reg_SCI2PURD (*(volatile typ_SCIPURD*) (0x00CD0008))

typedef union                /* SCI Port Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  reserved  : 6;
    bitfield  PORTSC1   : 1;  /* SCI Port Data Bit 1 */
    bitfield  PORTSC0   : 1;  /* SCI Port Data Bit 0 */
  } bit;
} typ_SCIPORT;
                             /* SCI1 Port Data Register */
#define reg_SCI1PORT (*(volatile typ_SCIPORT*) (0x00CC0009))
                             /* SCI2 Port Data Register */
#define reg_SCI2PORT (*(volatile typ_SCIPORT*) (0x00CD0009))

typedef union                /* SCI Data Direction Register */
{
  INT8U reg;
  struct
  {
    bitfield  reserved  : 6;
    bitfield  DDRSC1    : 1;  /* SCI Port Data Direction Bit 1 */
    bitfield  DDRSC0    : 1;  /* SCI Port Data Direction Bit 0 */
  } bit;
} typ_SCIDDR;
                             /* SCI1 Data Direction Register */
#define reg_SCI1DDR (*(volatile typ_SCIDDR*) (0x00CC000A))
                             /* SCI2 Data Direction Register */
#define reg_SCI2DDR (*(volatile typ_SCIDDR*) (0x00CD000A))

/*  Timer Modules  ******************************************************************************
*
*  Address    Use                                                           Access
*  00CE_0000  reg_TIM1IOS   TIM1 Input Capture / Output Compare Select Reg. Supervisor Only
*  00CE_0001  reg_TIM1CFORC TIM1 Compare Force Register                     Supervisor Only
*  00CE_0002  reg_TIM1OC3M  TIM1 Output Compare 3 Mask Register             Supervisor Only
*  00CE_0003  reg_TIM1OC3D  TIM1 Output Compare 3 Data Register             Supervisor Only
*  00CE_0004  reg_TIM1CNT   TIM1 Counter Register                           Supervisor Only
*  00CE_0006  reg_TIM1SCR1  TIM1 System Control Register 1                  Supervisor Only
*  00CE_0007         reserved
*  00CE_0008  reg_TIM1TOV   TIM1 Toggle on Overflow Register                Supervisor Only
*  00CE_0009  reg_TIM1CTL1  TIM1 Control Register 1                         Supervisor Only
*  00CE_000A         reserved
*  00CE_000B  reg_TIM1CTL2  TIM1 Control Register 2                         Supervisor Only
*  00CE_000C  reg_TIM1IE    TIM1 Interrupt Enable Register                  Supervisor Only
*  00CE_000D  reg_TIM1SCR2  TIM1 System Control Register 2                  Supervisor Only
*  00CE_000E  reg_TIM1FLG1  TIM1 Flag Register 1                            Supervisor Only
*  00CE_000F  reg_TIM1FLG2  TIM1 Flag Register 2                            Supervisor Only
*  00CE_0010  reg_TIM1C0    TIM1 Channel 0 Register                         Supervisor Only
*  00CE_0012  reg_TIM1C1    TIM1 Channel 1 Register                         Supervisor Only
*  00CE_0014  reg_TIM1C2    TIM1 Channel 2 Register                         Supervisor Only
*  00CE_0016  reg_TIM1C3    TIM1 Channel 3 Register                         Supervisor Only
*  00CE_0018  reg_TIM1PACTL TIM1 Pulse Accumulator Control Register         Supervisor Only
*  00CE_0019  reg_TIM1PAFLG TIM1 Pulse Accumulator Flag Register            Supervisor Only
*  00CE_001A  reg_TIM1PACNT TIM1 Pulse Accumulator Counter Register         Supervisor Only
*  00CE_001C         reserved
*  00CE_001D  reg_TIM1PORT  TIM1 Port Data Register                         Supervisor Only
*  00CE_001E  reg_TIM1DDR   TIM1 Port Data Direction Register               Supervisor Only
*  00CE_001F  reg_TIM1TST   TIM1 Test Register                              Supervisor Only
*  00CE_0020  ...      not used
*  00CE_FFFF
*
*  00CF_0000  reg_TIM2IOS   TIM2 Input Capture / Output Compare Select Reg. Supervisor Only
*  00CF_0001  reg_TIM2CFORC TIM2 Compare Force Register                     Supervisor Only
*  00CF_0002  reg_TIM2OC3M  TIM2 Output Compare 3 Mask Register             Supervisor Only
*  00CF_0003  reg_TIM2OC3D  TIM2 Output Compare 3 Data Register             Supervisor Only
*  00CF_0004  reg_TIM2CNT   TIM2 Counter Register                           Supervisor Only
*  00CF_0006  reg_TIM2SCR1  TIM2 System Control Register 1                  Supervisor Only
*  00CF_0007         reserved
*  00CF_0008  reg_TIM2TOV   TIM2 Toggle on Overflow Register                Supervisor Only
*  00CF_0009  reg_TIM2CTL1  TIM2 Control Register 1                         Supervisor Only
*  00CF_000A         reserved
*  00CF_000B  reg_TIM2CTL2  TIM2 Control Register 2                         Supervisor Only
*  00CF_000C  reg_TIM2IE    TIM2 Interrupt Enable Register                  Supervisor Only
*  00CF_000D  reg_TIM2SCR2  TIM2 System Control Register 2                  Supervisor Only
*  00CF_000E  reg_TIM2FLG1  TIM2 Flag Register 1                            Supervisor Only
*  00CF_000F  reg_TIM2FLG2  TIM2 Flag Register 2                            Supervisor Only
*  00CF_0010  reg_TIM2C0    TIM2 Channel 0 Register                         Supervisor Only
*  00CF_0012  reg_TIM2C1    TIM2 Channel 1 Register                         Supervisor Only
*  00CF_0014  reg_TIM2C2    TIM2 Channel 2 Register                         Supervisor Only
*  00CF_0016  reg_TIM2C3    TIM2 Channel 3 Register                         Supervisor Only
*  00CF_0018  reg_TIM2PACTL TIM2 Pulse Accumulator Control Register         Supervisor Only
*  00CF_0019  reg_TIM2PAFLG TIM2 Pulse Accumulator Flag Register            Supervisor Only
*  00CF_001A  reg_TIM2PACNT TIM2 Pulse Accumulator Counter Register         Supervisor Only
*  00CF_001C         reserved
*  00CF_001D  reg_TIM2PORT  TIM2 Port Data Register                         Supervisor Only
*  00CF_001E  reg_TIM2DDR   TIM2 Port Data Direction Register               Supervisor Only
*  00CF_001F  reg_TIM2TST   TIM2 Test Register                              Supervisor Only
*  00CF_0020  ...      not used
*  00CF_FFFF
*/

typedef union                /* TIM Input Capture / Output Compare Select Reg */
{
  INT8U reg;
  struct
  {
    bitfield  unused  : 4;
    bitfield  IOS3  : 1;      /* I/O Select 3 */
    bitfield  IOS2  : 1;      /* I/O Select 2 */
    bitfield  IOS1  : 1;      /* I/O Select 1 */
    bitfield  IOS0  : 1;      /* I/O Select 0 */
  } bit;
} typ_TIMIOS;
                             /* TIM1 Input Capture / Output Compare Select Reg */
#define reg_TIM1IOS (*(volatile typ_TIMIOS*) (0x00CE0000))
                             /* TIM2 Input Capture / Output Compare Select Reg */
#define reg_TIM2IOS (*(volatile typ_TIMIOS*) (0x00CF0000))

typedef union                /* TIM Compare Force Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  FOC3      : 1;  /* Force Output Compare 3 */
    bitfield  FOC2      : 1;  /* Force Output Compare 2 */
    bitfield  FOC1      : 1;  /* Force Output Compare 1 */
    bitfield  FOC0      : 1;  /* Force Output Compare 0 */
  } bit;
} typ_TIMCFORC;
                             /* TIM1 Compare Force Register */
#define reg_TIM1CFORC (*(volatile typ_TIMCFORC*) (0x00CE0001))
                             /* TIM2 Compare Force Register */
#define reg_TIM2CFORC (*(volatile typ_TIMCFORC*) (0x00CF0001))

typedef union                /* TIM Output Compare 3 Mask Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  OC3M3     : 1;  /* Output Compare 3 Mask 3 */
    bitfield  OC3M2     : 1;  /* Output Compare 3 Mask 2 */
    bitfield  OC3M1     : 1;  /* Output Compare 3 Mask 1 */
    bitfield  OC3M0     : 1;  /* Output Compare 3 Mask 0 */
  } bit;
} typ_TIMOC3M;
                             /* TIM1 Output Compare 3 Mask Register */
#define reg_TIM1OC3M (*(volatile typ_TIMOC3M*) (0x00CE0002))
                             /* TIM2 Output Compare 3 Mask Register */
#define reg_TIM2OC3M (*(volatile typ_TIMOC3M*) (0x00CF0002))

typedef union                /* TIM Output Compare 3 Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  OC3D3     : 1;  /* Output Compare 3 Data Bit 3 */
    bitfield  OC3D2     : 1;  /* Output Compare 3 Data Bit 2 */
    bitfield  OC3D1     : 1;  /* Output Compare 3 Data Bit 1 */
    bitfield  OC3D0     : 1;  /* Output Compare 3 Data Bit 0 */
  } bit;
} typ_TIMOC3D;
                             /* TIM1 Output Compare 3 Data Register */
#define reg_TIM1OC3D (*(volatile typ_TIMOC3D*) (0x00CE0003))
                             /* TIM2 Output Compare 3 Data Register */
#define reg_TIM2OC3D (*(volatile typ_TIMOC3D*) (0x00CF0003))

                             /* TIM1 Force / OC3 Initialization Pointer */
#define reg_TIM1FOC (*(volatile INT32U*) (0x00CE0000))
                             /* TIM2 Force / OC3 Initialization Pointer */
#define reg_TIM2FOC (*(volatile INT32U*) (0x00CF0000))

                             /* TIM1 Counter Register */
#define reg_TIM1CNT (*(volatile INT16U*) (0x00CE0004))
                             /* TIM2 Counter Register */
#define reg_TIM2CNT (*(volatile INT16U*) (0x00CF0004))

typedef union                /* TIM System Control Register 1 */
{
  INT8U reg;
  struct
  {
    bitfield  TIMEN     : 1;  /* Timer Enable */
    bitfield  unused    : 2;
    bitfield  TFFCA     : 1;  /* Timer Fast Flag Clear All */
    bitfield  unused1   : 4;
  } bit;
} typ_TIMSCR1;
                             /* TIM1 System Control Register 1 */
#define reg_TIM1SCR1 (*(volatile typ_TIMSCR1*) (0x00CE0006))
                        /* TIM2 System Control Register 1 */
#define reg_TIM2SCR1 (*(volatile typ_TIMSCR1*) (0x00CF0006))

typedef union                /* TIM Toggle on Overflow Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  TOV3      : 1;  /* Toggle-On-Overflow 3 */
    bitfield  TOV2      : 1;  /* Toggle-On-Overflow 2 */
    bitfield  TOV1      : 1;  /* Toggle-On-Overflow 1 */
    bitfield  TOV0      : 1;  /* Toggle-On-Overflow 0 */
  } bit;
} typ_TIMTOV;
                             /* TIM1 Toggle on Overflow Register */
#define reg_TIM1TOV (*(volatile typ_TIMTOV*) (0x00CE0008))
                             /* TIM2 Toggle on Overflow Register */
#define reg_TIM2TOV (*(volatile typ_TIMTOV*) (0x00CF0008))

typedef union                /* TIM Control Register 1 */
{
  INT8U reg;
  struct
  {
    bitfield  OM3_OL3   : 2;  /* Output Mode / Output Level 3 */
    bitfield  OM2_OL2   : 2;  /* Output Mode / Output Level 2 */
    bitfield  OM1_OL1   : 2;  /* Output Mode / Output Level 1 */
    bitfield  OM0_OL0   : 2;  /* Output Mode / Output Level 0 */
  } bit;
} typ_TIMCTL1;
                             /* TIM1 Control Register 1 */
#define reg_TIM1CTL1 (*(volatile typ_TIMCTL1*) (0x00CE0009))
                             /* TIM2 Control Register 1 */
#define reg_TIM2CTL1 (*(volatile typ_TIMCTL1*) (0x00CF0009))

typedef union                /* TIM Control Register 2 */
{
  INT8U reg;
  struct
  {
    bitfield  EDG3      : 2;  /* Input Capture Edge Control 3 */
    bitfield  EDG2      : 2;  /* Input Capture Edge Control 2 */
    bitfield  EDG1      : 2;  /* Input Capture Edge Control 1 */
    bitfield  EDG0      : 2;  /* Input Capture Edge Control 0 */
  } bit;
} typ_TIMCTL2;
                             /* TIM1 Control Register 2 */
#define reg_TIM1CTL2 (*(volatile typ_TIMCTL2*) (0x00CE000B))
                             /* TIM2 Control Register 2 */
#define reg_TIM2CTL2 (*(volatile typ_TIMCTL2*) (0x00CF000B))

                             /* TIM1 TOV / Control Initialization Pointer */
#define reg_TIM1TOVCTL (*(volatile INT32U*) (0x00CE0008))
                             /* TIM2 TOV / Control Initialization Pointer */
#define reg_TIM2TOVCTL (*(volatile INT32U*) (0x00CF0008))

typedef union                /* TIM Interrupt Enable Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  C3I       : 1;  /* Channel 3 Interrupt Enable */
    bitfield  C2I       : 1;  /* Channel 2 Interrupt Enable */
    bitfield  C1I       : 1;  /* Channel 1 Interrupt Enable */
    bitfield  C0I       : 1;  /* Channel 0 Interrupt Enable */
  } bit;
} typ_TIMIE;
                             /* TIM1 Interrupt Enable Register */
#define reg_TIM1IE (*(volatile typ_TIMIE*) (0x00CE000C))
                             /* TIM2 Interrupt Enable Register */
#define reg_TIM2IE (*(volatile typ_TIMIE*) (0x00CF000C))

typedef union                /* TIM System Control Register 2 */
{
  INT8U reg;
  struct
  {
    bitfield  TOI       : 1;  /* Timer Overflow Interrupt Enable */
    bitfield  unused    : 1;
    bitfield  PUPT      : 1;  /* Timer Pullup Enable */
    bitfield  RDPT      : 1;  /* Timer Drive Reduction */
    bitfield  TCRE      : 1;  /* Timer Counter Reset Enable */
    bitfield  PR        : 3;  /* Prescaler */
  } bit;
} typ_TIMSCR2;
                             /* TIM1 System Control Register 2 */
#define reg_TIM1SCR2 (*(volatile typ_TIMSCR2*) (0x00CE000D))
                             /* TIM2 System Control Register 2 */
#define reg_TIM2SCR2 (*(volatile typ_TIMSCR2*) (0x00CF000D))

typedef union                /* TIM Flag Register 1 */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  C3F       : 1;  /* Channel 3 Flag */
    bitfield  C2F       : 1;  /* Channel 2 Flag */
    bitfield  C1F       : 1;  /* Channel 1 Flag */
    bitfield  C0F       : 1;  /* Channel 0 Flag */
  } bit;
} typ_TIMFLG1;
                             /* TIM1 Flag Register 1 */
#define reg_TIM1FLG1 (*(volatile typ_TIMFLG1*) (0x00CE000E))
                             /* TIM2 Flag Register 1 */
#define reg_TIM2FLG1 (*(volatile typ_TIMFLG1*) (0x00CF000E))

typedef union                /* TIM Flag Register 2 */
{
  INT8U reg;
  struct
  {
    bitfield  TOF       : 1;  /* Timer Overflow Flag */
    bitfield  unused    : 7;
  } bit;
} typ_TIMFLG2;
                             /* TIM1 Flag Register 2 */
#define reg_TIM1FLG2 (*(volatile typ_TIMFLG2*) (0x00CE000F))
                             /* TIM2 Flag Register 2 */
#define reg_TIM2FLG2 (*(volatile typ_TIMFLG2*) (0x00CF000F))

                             /* TIM1 Channel 0 Register */
#define reg_TIM1C0 (*(volatile INT16U*) (0x00CE0010))
                             /* TIM1 Channel 1 Register */
#define reg_TIM1C1 (*(volatile INT16U*) (0x00CE0012))
                             /* TIM1 Channel 2 Register */
#define reg_TIM1C2 (*(volatile INT16U*) (0x00CE0014))
                             /* TIM1 Channel 3 Register */
#define reg_TIM1C3 (*(volatile INT16U*) (0x00CE0016))
                             /* TIM2 Channel 0 Register */
#define reg_TIM2C0 (*(volatile INT16U*) (0x00CF0010))
                             /* TIM2 Channel 1 Register */
#define reg_TIM2C1 (*(volatile INT16U*) (0x00CF0012))
                             /* TIM2 Channel 2 Register */
#define reg_TIM2C2 (*(volatile INT16U*) (0x00CF0014))
                             /* TIM2 Channel 3 Register */
#define reg_TIM2C3 (*(volatile INT16U*) (0x00CF0016))

typedef union                /* TIM Pulse Accumulator Control Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 1;
    bitfield  PAE       : 1;  /* Pulse Accumulator Enable */
    bitfield  PAMOD     : 1;  /* Pulse Accumulator Mode */
    bitfield  PEDGE     : 1;  /* Pulse Accumulator Edge */
    bitfield  CLK       : 2;  /* Clock Select */
    bitfield  PAOVI     : 1;  /* Pulse Accumulator Overflow Interrupt Enable */
    bitfield  PAI       : 1;  /* Pulse Accumulator Input Interrupt Enable */
  } bit;
} typ_TIMPACTL;
                             /* TIM1 Pulse Accumulator Control Register */
#define reg_TIM1PACTL (*(volatile typ_TIMPACTL*) (0x00CE0018))
                             /* TIM2 Pulse Accumulator Control Register */
#define reg_TIM2PACTL (*(volatile typ_TIMPACTL*) (0x00CF0018))

typedef union                /* TIM Pulse Accumulator Flag Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 6;
    bitfield  PAOVF     : 1;  /* Pulse Accumulator Overflow Flag */
    bitfield  PAIF      : 1;  /* Pulse Accumulator Input Flag */
  } bit;
} typ_TIMPAFLG;
                             /* TIM1 Pulse Accumulator Flag Register */
#define reg_TIM1PAFLG (*(volatile typ_TIMPAFLG*) (0x00CE0019))
                             /* TIM2 Pulse Accumulator Flag Register */
#define reg_TIM2PAFLG (*(volatile typ_TIMPAFLG*) (0x00CF0019))

                             /* TIM1 Pulse Accumulator Counter Register */
#define reg_TIM1PACNT (*(volatile INT16U*) (0x00CE001A))
                             /* TIM2 Pulse Accumulator Counter Register */
#define reg_TIM2PACNT (*(volatile INT16U*) (0x00CF001A))

typedef union                /* TIM Port Data Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  PORTT3    : 1;  /* TIMPORT Input Capture / Output Compare Data 3 */
    bitfield  PORTT2    : 1;  /* TIMPORT Input Capture / Output Compare Data 2 */
    bitfield  PORTT1    : 1;  /* TIMPORT Input Capture / Output Compare Data 1 */
    bitfield  PORTT0    : 1;  /* TIMPORT Input Capture / Output Compare Data 0 */
  } bit;
} typ_TIMPORT;
                             /* TIM1 Port Data Register */
#define reg_TIM1PORT (*(volatile typ_TIMPORT*) (0x00CE001D))
                             /* TIM2 Port Data Register */
#define reg_TIM2PORT (*(volatile typ_TIMPORT*) (0x00CF001D))

typedef union                /* TIM Port Data Direction Register */
{
  INT8U reg;
  struct
  {
    bitfield  unused    : 4;
    bitfield  DDRT3     : 1;  /* TIMPORT Data Direction 3 */
    bitfield  DDRT2     : 1;  /* TIMPORT Data Direction 2 */
    bitfield  DDRT1     : 1;  /* TIMPORT Data Direction 1 */
    bitfield  DDRT0     : 1;  /* TIMPORT Data Direction 0 */
  } bit;
} typ_TIMDDR;
                             /* TIM1 Port Data Direction Register */
#define reg_TIM1DDR (*(volatile typ_TIMDDR*) (0x00CE001E))
                             /* TIM2 Port Data Direction Register */
#define reg_TIM2DDR (*(volatile typ_TIMDDR*) (0x00CF001E))

/*  FLASH Memory Control Module  ****************************************************************
*
*  Address    Use                                           Access
*  00D0_0000  reg_SGFMMCR  SGFM Module Control Register     Supervisor Only
*  00D0_0002  reg_SGFMCLKD SGFM Clock Divider Register      Supervisor Only
*  00D0_0004  reg_SGFMTST  SGFM Test Register               Supervisor Only - Test Mode
*  00D0_0008  reg_SGFMSEC  SGFM Security Register           Supervisor Only
*  00D0_000C  reg_SGFMMNTR SGFM Monitor Data Register       Supervisor Only - Test Mode
*  00D0_0010  reg_SGFMPROT SGFM Protection Register         Supervisor Only
*  00D0_0014  reg_SGFMSACC SGFM Supervisor Access Register  Supervisor Only
*  00D0_0016  reg_SGFMDACC SGFM Data Access Register        Supervisor Only
*  00D0_0018  reg_SGFMTSTAT SGFM Test Status Register       Supervisor Only - Test Mode
*  00D0_001C  reg_SGFMUSTAT SGFM User Status Register       Supervisor Only
*  00D0_0020  reg_SGFMCMD  SGFM Command Register            Supervisor Only
*  00D0_0024  reg_SGFMCTL  SGFM Control Register            Supervisor Only - Test Mode
*  00D0_0026  reg_SGFMADR  SGFM Address Register            Supervisor Only - Test Mode
*  00D0_0028  reg_SGFMDATA SGFM Data Register               Supervisor Only - Test Mode
*  00D0_002C  ...      notused
*  00D0_FFFF
*/



/* SGFM Configuration Field */
typedef struct                /* SGFM Configuration Field */
{
  INT32U    BACKDOORWORD1;    /* Backdoor word 1 */ 
  INT32U    BACKDOORWORD2;    /* Backdoor word 2 */
  INT16U    SGFMB0SPESP;      /* SGFM Block 0 Program/Erase Space Restrictions */    
  INT16U    Reserved1;
  INT16U    SGFMB0SUSP;       /* SGFM Block 0 Supervisor/User Space Restrictions */
  INT16U    SGFMB0PDSP;       /* SGFM Block 0 Program/Data Space Restrictions */
  INT16U    SGFMB1SPESP;      /* SGFM Block 1 Program/Erase Space Restrictions */    
  INT16U    Reserved2;
  INT16U    SGFMB1SUSP;       /* SGFM Block 1 Supervisor/User Space Restrictions */
  INT16U    SGFMB1PDSP;       /* SGFM Block 1 Program/Data Space Restrictions */
  INT16U    SGFMB2SPESP;      /* SGFM Block 2 Program/Erase Space Restrictions */    
  INT16U    Reserved3;
  INT16U    SGFMB2SUSP;       /* SGFM Block 2 Supervisor/User Space Restrictions */
  INT16U    SGFMB2PDSP;       /* SGFM Block 2 Program/Data Space Restrictions */
  INT16U    SGFMB3SPESP;      /* SGFM Block 3 Program/Erase Space Restrictions */    
  INT16U    Reserved4;
  INT16U    SGFMB3SUSP;       /* SGFM Block 3 Supervisor/User Space Restrictions */
  INT16U    SGFMB3PDSP;       /* SGFM Block 3 Program/Data Space Restrictions */
  INT32U    SGFMSECWORD;      /* SGFM Security Word */
} typ_SGFM_CONFIG_FLD;
#define SGFM_CONFIG_FLD (*(volatile typ_SGFM_CONFIG_FLD*) (FLASH_ + 0x00000200))


typedef union                 /* SGFM Module Control Register */
{
  INT16U reg;
  struct
  {
    bitfield  unused    : 1;
    bitfield  FRZ       : 1;  /* FLASH Freeze Enable */
    bitfield  unused1   : 1;
    bitfield  EME       : 1;  /* FLASH Emulation Enable */
    bitfield  unused2   : 1;
    bitfield  LOCK      : 1;  /* Write Lock Control */
    bitfield  unused3   : 2;
    bitfield  CBEIE     : 1;  /* Command Buffer Empty Interrupt Enable */
    bitfield  CCIE      : 1;  /* Command Complete Interrupt Enable */
    bitfield  KEYACC    : 1;  /* Enable Security Key Writing */
    bitfield  unused4   : 3;
    bitfield  BKSEL     : 2;  /* Register Bank Select Space */
  } bit;
} typ_SGFMCR;
#define reg_SGFMCR (*(volatile typ_SGFMCR*) (0x00D00000))

typedef union                 /* SGFM Clock Divider Register */
{
  INT16U reg;
  struct
  {
    bitfield  DIVLD     : 1;  /* Clock Divider Loaded Bit */
    bitfield  PRDIV     : 1;  /* Enable Prescaler Divide-by-8 */
    bitfield  DIV       : 6;  /* Clock Divider Field */
    bitfield  unused    : 8;
  } bit;
} typ_SGFMCLKD;
#define reg_SGFMCLKD (*(volatile typ_SGFMCLKD*) (0x00D00002))

typedef union                 /* SGFM Security Register */
{
  INT32U reg;
  struct
  {
    bitfield  KEYEN     : 1;  /* Enable Back Door Key to Security */
    bitfield  SECSTAT   : 1;  /* FLASH Security Status */
    bitfield  unused    : 14; 
    bitfield  SEC       : 16; /* Security Field */
  } bit;
} typ_SGFMSEC;
#define reg_SGFMSEC (*(volatile typ_SGFMSEC*) (0x00D00008))

typedef union                 /* SGFM Protection Register */
{
  INT16U reg;
  struct
  {
    bitfield  PROT      : 16; /* Sector Protection */
  } bit;
} typ_SGFMPROT;
#define reg_SGFMPROT (*(volatile typ_SGFMPROT*) (0x00D00010))

typedef union                 /* SGFM Supervisor Access Register */
{
  INT16U reg;
  struct
  {
    bitfield  SUPV      : 16; /* Supervisor Access Space Assignment */
  } bit;
} typ_SGFMSACC;
#define reg_SGFMSACC (*(volatile typ_SGFMSACC*) (0x00D00014))

typedef union                 /* SGFM Data Access Register */
{
  INT16U reg;
  struct
  {
    bitfield  DATA      : 16; /* Data Address Space Assignment  */
  } bit;
} typ_SGFMDACC;
#define reg_SGFMDACC (*(volatile typ_SGFMDACC*) (0x00D00016))

typedef union                 /* SGFM User Status Register */
{
  INT16U reg;
  struct
  {
    bitfield  CBEIF     : 1;  /* Command Buffer Empty Interrupt Flag */
    bitfield  CCIF      : 1;  /* Command Complete Interrupt Flag */
    bitfield  PVIOL     : 1;  /* Protection Violation Flag */
    bitfield  ACCERR    : 1;  /* Access Error Flag */
    bitfield  unused    : 1;  
    bitfield  BLANK     : 1;  /* Erase Verify Flag */
    bitfield  unused1   : 10; 
  } bit;
} typ_SGFMUSTAT;
#define reg_SGFMUSTAT (*(volatile typ_SGFMUSTAT*) (0x00D0001C))

typedef union                 /* SGFM Command Register */
{
  INT16U reg;
  struct
  {
    bitfield unused     : 1;  
    bitfield CMD        : 7;  /* Command Field */
    bitfield unused1    : 8;  
  } bit;
} typ_SGFMCMD;
#define reg_SGFMCMD (*(volatile typ_SGFMCMD*) (0x00D00020))

#endif // _MMC2114_H_
#endif
