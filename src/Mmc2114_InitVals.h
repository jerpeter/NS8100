///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Mmc2114_InitVals.h,v $
///	$Author: lking $
///	$Date: 2011/07/30 17:30:07 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/source/Mmc2114_InitVals.h,v $
///	$Revision: 1.1 $
///----------------------------------------------------------------------------
#if 1
#ifndef _MMC2114_INITVALS_H_
#define _MMC2114_INITVALS_H_

///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "MMC2114.h"
#include "Typedefs.h"

/*******************************************************************************
*   Macro/Symbol Definitions                                                   *
*******************************************************************************/
/*  M-Core M210 Processor Status Register bit mask definitions                */
#define mmc_PSR_C   (0x00000001)  /* Condition Code/Carry                     */
#define mmc_PSR_AF  (0x00000002)  /* Alternate File Enable                    */
#define mmc_PSR_FE  (0x00000010)  /* Fast Interrupt Enable                    */
#define mmc_PSR_IE  (0x00000040)  /* Interrupt Enable                         */
#define mmc_PSR_IC  (0x00000080)  /* Interrupt Control                        */
#define mmc_PSR_EE  (0x00000100)  /* Exception Enable                         */
#define mmc_PSR_MM  (0x00000200)  /* Misalignment Exception Mask              */
#define mmc_PSR_SC  (0x00000400)  /* Spare Control                            */
#define mmc_PSR_TC  (0x00001000)  /* Translation Control                      */
#define mmc_PSR_TP  (0x00002000)  /* Trace Pending                            */
#define mmc_PSR_TM  (0x0000C000)  /* Trace Mode                               */
#define mmc_PSR_VEC (0x007F0000)  /* Vector Number                            */
#define mmc_PSR_U0  (0x01000000)  /* Hardware Accelerator Control 0           */
#define mmc_PSR_U1  (0x02000000)  /* Hardware Accelerator Control 1           */
#define mmc_PSR_U2  (0x04000000)  /* Hardware Accelerator Control 2           */
#define mmc_PSR_U3  (0x08000000)  /* Hardware Accelerator Control 3           */
#define mmc_PSR_S   (0x80000000)  /* Supervisor Mode                          */

/*  Ports  ****************************************************************** */
/*  Port A */
/*  reg_PORTA       Port A Output Data Register */
#define init_PORTA (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRA        Port A Data Direction Register */
#define init_DDRA (0x00) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  Port B */
/*  reg_PORTB       Port B Output Data Register */
#define init_PORTB (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRB        Port B Data Direction Register */
#define init_DDRB (0x00) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  Port C */
/*  reg_PORTC       Port C Output Data Register */
#define init_PORTC (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRC        Port C Data Direction Register */
// All pins set as inputs
//#define init_DDRC (0x00) /*         (0b00000000)       */
// All pins set as outputs
#define init_DDRC (0xFF) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_PCDPAR      Port C/D Pin Assignment Register */
/* Note that 0x80 is what the emulator wants - 
                       this is not a default value */
// 0x80 makes lines data (which are flating)
//#define init_PCDPAR (0x80) /*       (0b10000000)        */
// 0x00 makes lines set as digital I/O
#define init_PCDPAR (0x00) /*       (0b10000000)        */

/*                                     |_________ PCDPA */

/*  Port D */
/*  reg_PORTD       Port D Output Data Register */
#define init_PORTD (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRD        Port D Data Direction Register */
// All pins set as inputs
//#define init_DDRD (0x00) /*         (0b00000000)       */
// All pins set as outputs
#define init_DDRD (0xFF) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

#define init_PORT_A_D (uint32)((init_PORTA<<24)+(init_PORTB<<16)+(init_PORTC<<8)+      \
                      (init_PORTD))
#define init_DDR_A_D (uint32)((init_DDRA<<24)+(init_DDRB<<16)+(init_DDRC<<8)+          \
                     (init_DDRD))

/*  Port E */
/*  reg_PORTE       Port E Output Data Register */
//#define init_PORTE (0x00) /*        (0b00000000)       */
#define init_PORTE (0x18) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRE        Port E Data Direction Register */
/*#define init_DDRE (0x00)          (0b00000000)  */
#define init_DDRE (0xFF) /*         (0b00011111)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_PEPAR       Port E Pin Assignment Register */
/* Note that 0xFF is what the emulator wants - 
                       this is not a default value */
/*#define init_PEPAR (0xFF) */ /*   (0b11100000)        */                       
#define init_PEPAR (0x00) /*        (0b11100000)        */
/*                                     ||||||||__ PEPA0 */
/*                                     |||||||___ PEPA1 */
/*                                     ||||||____ PEPA2 */
/*                                     |||||_____ PEPA3 */
/*                                     ||||______ PEPA4 */
/*                                     |||_______ PEPA5 */
/*                                     ||________ PEPA6 */
/*                                     |_________ PEPA7 */

/*  Port F */
/*  reg_PORTF       Port F Output Data Register */
#define init_PORTF (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRF        Port F Data Direction Register */
#define init_DDRF (0x00) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  Port G */
/*  reg_PORTG       Port G Output Data Register */
#define init_PORTG (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRG        Port G Data Direction Register */
#define init_DDRG (0x00) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  Port H */
/*  reg_PORTH       Port H Output Data Register */
#define init_PORTH (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRH        Port H Data Direction Register */
#define init_DDRH (0x00) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

#define init_PORT_E_H   ((init_PORTE<<24)+(init_PORTF<<16)+(init_PORTG<<8)+    \
                         (init_PORTH))
#define init_DDR_E_H    ((init_DDRE<<24)+(init_DDRF<<16)+(init_DDRG<<8)+       \
                         (init_DDRH))

/*  Port I */
/*  reg_PORTI       Port I Output Data Register */
#define init_PORTI (0x00) /*        (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  reg_DDRI        Port I Data Direction Register */
#define init_DDRI (0x00) /*         (0b00000000)       */
/*                                     ||||||||__ PIN0 */
/*                                     |||||||___ PIN1 */
/*                                     ||||||____ PIN2 */
/*                                     |||||_____ PIN3 */
/*                                     ||||______ PIN4 */
/*                                     |||_______ PIN5 */
/*                                     ||________ PIN6 */
/*                                     |_________ PIN7 */

/*  Chip Configuration Module  ************************************************/
/*  reg_CCR         Chip Configuration Register */
/*#define init_CCR (0x0000) *//*    (0b0000000000001000)        */
#define init_CCR (0x8000) /*        (0b1000000001000000)        */
/*                                     | || ||| |||||||__ BMT   */
/*                                     | || ||| |||||____ BMD   */
/*                                     | || ||| ||||_____ BME   */
/*                                     | || ||| |||______ SHINT */
/*                                     | || ||| ||_______ PSTEN */
/*                                     | || ||| |________ SZEN  */
/*                                     | || |||__________ MODE  */
/*                                     | ||______________ EMINT */
/*                                     | |_______________ SHEN  */
/*                                     |_________________ LOAD  */

/*  reg_RCON        Reset Configuration Register */
#define mmc_testNormalPLLMode       (reg_RCON.bit.RPLLSEL == 1)
#define mmc_testPLLCrystalReference (reg_RCON.bit.RPLLREF == 1)
#define mmc_testSingleChipMode      (reg_RCON.bit.MODE == 0)

/*  reg_CIR         Chip Identification Register */
#define mmc_PartIdentification      (reg_CIR.bit.PIN)
#define mmc_caseMMC2114             (0x17)
#define mmc_PartRevision            (reg_CIR.bit.PRN)

/*  reg_CTR         Chip Test Register */
#define init_CTR (0x0000) /*        (0b0000000000000000) */

/*  Chip Select Module  *******************************************************/
/*  reg_CSCR0       Chip Select Control Register 0 */
/* Value used here is not the default value - it is used to
		be consistent with Codewarrior's init file for the CMB2107	*/
#define init_CSCR0  (0x0203)
/* #define init_CSCR0  (0x3403)       (0b0011010000000011) */
/*                                     ||||||||      ||__ CSEN */
/*                                     ||||||||      |___ TAEN */
/*                                     ||||||||__________ WS */
/*                                     |||||_____________ WE */
/*                                     ||||______________ WWS */
/*                                     |||_______________ PS */
/*                                     ||________________ RO */
/*                                     |_________________ SO */

/*  reg_CSCR1       Chip Select Control Register 1 */
/* Value used here is not the default value - it is used to
		be consistent with Codewarrior's init file for the CMB2107	*/
#define init_CSCR1  (0x0203)
/*#define init_CSCR1  (0x3103)        (0b0011000100000011) */
/*                                       ||||||||      ||__ CSEN */
/*                                       ||||||||      |___ TAEN */
/*                                       ||||||||__________ WS */
/*                                       |||||_____________ WE */
/*                                       ||||______________ WWS */
/*                                       |||_______________ PS */
/*                                       ||________________ RO */
/*                                       |_________________ SO */

/*  reg_CSCR2       Chip Select Control Register 2 */
/* Value used here is not the default value - it is used to
		be consistent with Codewarrior's init file for the CMB2107	*/
#define init_CSCR2  (0x1703)
/*#define init_CSCR2  (0x3403)        (0b0011010000000011) */
/*                                       ||||||||      ||__ CSEN */
/*                                       ||||||||      |___ TAEN */
/*                                       ||||||||__________ WS */
/*                                       |||||_____________ WE */
/*                                       ||||______________ WWS */
/*                                       |||_______________ PS */
/*                                       ||________________ RO */
/*                                       |_________________ SO */

/*  reg_CSCR3       Chip Select Control Register 3 */
/* Value used here is not the default value - it is used to
		be consistent with Codewarrior's init file for the CMB2107	*/
#define init_CSCR3  (0x1703)
/*#define init_CSCR3  (0x3403)        (0b0011010000000011) */
/*                                       ||||||||      ||__ CSEN */
/*                                       ||||||||      |___ TAEN */
/*                                       ||||||||__________ WS */
/*                                       |||||_____________ WE */
/*                                       ||||______________ WWS */
/*                                       ||+--------------- PS */
/*                                       |+---------------- RO */
/*                                       +----------------- SO */

/*  Clock Module  *************************************************************/
/*  reg_SYNCR       Synthesizer Control Register */
/* Note that this initial value is not the default value - it is
		used to be consistent with Codewarrior initialization (x4)	*/

//#define init_SYNCR  (0x0100) // x1 - 4MHz       //(0b0000000100000000)
//#define init_SYNCR  (0x0000) // x2 - 8MHz       //(0b0000000100000000)
//#define init_SYNCR  (0x1000) // x3 - 12MHz       //(0b0000000100000000)
//#define init_SYNCR  (0x2000) // x4 - 16MHz       //(0b0000000100000000)
//#define init_SYNCR  (0x3000) // x5 - 20MHz       //(0b0000000100000000)
//#define init_SYNCR  (0x4000) // x6 - 24MHz       //(0b0000000100000000)
//#define init_SYNCR  (0x5000) // x7 - 28MHz       //(0b0000000100000000)
#define init_SYNCR  (0x6000) // x8 - 32MHz      //(0b0000000100000000)
//#define init_SYNCR  (0x0140)       //(0b0000000100000000)
/*#define init_SYNCR  (0x6000)        (0b0010000000000000) */
/*                                       ||||||||||| ||____ STMPD */
/*                                       |||||||||||_______ FWKUP */
/*                                       ||||||||||________ DISCLK */
/*                                       |||||||||_________ LOCEN */
/*                                       ||||||||__________ RFD */
/*                                       |||||_____________ LOCRE */
/*                                       ||||______________ MFD */
/*                                       |_________________ LOLRE */

/*  reg_SYNSR       Synthesizer Status Register */
#define mmc_SystemClockMode         (reg_SYNSR.bit.PLLMODE)
#define mmc_caseExternalClock       (0b000)
#define mmc_caseOne2OnePLL          (0b100)
#define mmc_casePLLwithClock        (0b110)
#define mmc_casePLLwithCrystal      (0b111)
#define mmc_testPLLisLocked         (reg_SYNSR.bit.LOCK == 1)

/*  Reset Module  *************************************************************/
/*  reg_RCR         Reset Control Register */
#define init_RCR  (0x07) /*          (0b00000111) */
/*                                      || |||||__ LVDE */
/*                                      || ||||___ LVDSE */
/*                                      || |||____ LVDRE */
/*                                      || ||_____ LVDIE */
/*                                      || |______ LVDF */
/*                                      ||________ FRCRSTOUT */
/*                                      |_________ SOFTRST */
#define mmc_enSoftResetRequest      (reg_RCR.bit.SOFTRST = 1)

/*  reg_RSR         Reset Status Register */
#define mmc_testLowVoltageDetectReset (reg_RSR.bit.LVD == 1)
#define mmc_testSoftReset           (reg_RSR.bit.SOFT == 1)
#define mmc_testWatchdogReset       (reg_RSR.bit.WDR == 1)
#define mmc_testPowerOnReset        (reg_RSR.bit.POR == 1)
#define mmc_testExternalReset       (reg_RSR.bit.EXT == 1)
#define mmc_testLossOfClockReset    (reg_RSR.bit.LOC == 1)
#define mmc_testLossOfLockReset     (reg_RSR.bit.LOL == 1)

/*  Interrupt Controller Module  **********************************************/

/*  reg_ICR         Interrupt Control Register */
/*   This initialization configures the Interrupt Controller for vectored 
    interrupts, which is presumed to be the more common way to handle 
    interrupts.  This is different from the default state of this register which
    configures for autovector interrupts.                           */
/*  Set FVE to enable separate fast vectored interrupt vectors */
#define init_ICR  (0x0000)     /*   (0b0000000000000000) */
/*                                     ||||       |||||__ MASK */
/*                                     ||||______________ MFI  */
/*                                     |||_______________ ME   */
/*                                     ||________________ FVE  Unique fast/norm vector numbers */
/*                                     |_________________ AE   Use vectored interrupts */

/*  reg_ISR         Interrupt Status Register */
#define mmc_testNormalInterruptReq  (reg_ISR.bit.INT == 1)
#define mmc_testFastInterruptReq    (reg_ISR.bit.FINT == 1)
#define mmc_InterruptVector         (reg_ISR.bit.VEC)

/*  reg_IFRH        Interrupt Force Register High */
#define init_IFRH    (0x0000) /*     (0b0000000000000000) */
#define mmc_enForceInterruptReq39   (reg_IFRH.bit.IF39 = 1)
#define mmc_enForceInterruptReq38   (reg_IFRH.bit.IF38 = 1)
#define mmc_enForceInterruptReq37   (reg_IFRH.bit.IF37 = 1)
#define mmc_enForceInterruptReq36   (reg_IFRH.bit.IF36 = 1)
#define mmc_enForceInterruptReq35   (reg_IFRH.bit.IF35 = 1)
#define mmc_enForceInterruptReq34   (reg_IFRH.bit.IF34 = 1)
#define mmc_enForceInterruptReq33   (reg_IFRH.bit.IF33 = 1)
#define mmc_enForceInterruptReq32   (reg_IFRH.bit.IF32 = 1)

/*  reg_IFRL        Interrupt Force Register Low */
#define init_IFRL    (0x0000) /*     (0b0000000000000000) */
#define mmc_enForceInterruptReq31   (reg_IFRL.bit.IF31 = 1)
#define mmc_enForceInterruptReq30   (reg_IFRL.bit.IF30 = 1)
#define mmc_enForceInterruptReq29   (reg_IFRL.bit.IF29 = 1)
#define mmc_enForceInterruptReq28   (reg_IFRL.bit.IF28 = 1)
#define mmc_enForceInterruptReq27   (reg_IFRL.bit.IF27 = 1)
#define mmc_enForceInterruptReq26   (reg_IFRL.bit.IF26 = 1)
#define mmc_enForceInterruptReq25   (reg_IFRL.bit.IF25 = 1)
#define mmc_enForceInterruptReq24   (reg_IFRL.bit.IF24 = 1)
#define mmc_enForceInterruptReq23   (reg_IFRL.bit.IF23 = 1)
#define mmc_enForceInterruptReq22   (reg_IFRL.bit.IF22 = 1)
#define mmc_enForceInterruptReq21   (reg_IFRL.bit.IF21 = 1)
#define mmc_enForceInterruptReq20   (reg_IFRL.bit.IF20 = 1)
#define mmc_enForceInterruptReq19   (reg_IFRL.bit.IF19 = 1)
#define mmc_enForceInterruptReq18   (reg_IFRL.bit.IF18 = 1)
#define mmc_enForceInterruptReq17   (reg_IFRL.bit.IF17 = 1)
#define mmc_enForceInterruptReq16   (reg_IFRL.bit.IF16 = 1)
#define mmc_enForceInterruptReq15   (reg_IFRL.bit.IF15 = 1)
#define mmc_enForceInterruptReq14   (reg_IFRL.bit.IF14 = 1)
#define mmc_enForceInterruptReq13   (reg_IFRL.bit.IF13 = 1)
#define mmc_enForceInterruptReq12   (reg_IFRL.bit.IF12 = 1)
#define mmc_enForceInterruptReq11   (reg_IFRL.bit.IF11 = 1)
#define mmc_enForceInterruptReq10   (reg_IFRL.bit.IF10 = 1)
#define mmc_enForceInterruptReq9    (reg_IFRL.bit.IF9 = 1)
#define mmc_enForceInterruptReq8    (reg_IFRL.bit.IF8 = 1)
#define mmc_enForceInterruptReq7    (reg_IFRL.bit.IF7 = 1)
#define mmc_enForceInterruptReq6    (reg_IFRL.bit.IF6 = 1)
#define mmc_enForceInterruptReq5    (reg_IFRL.bit.IF5 = 1)
#define mmc_enForceInterruptReq4    (reg_IFRL.bit.IF4 = 1)
#define mmc_enForceInterruptReq3    (reg_IFRL.bit.IF3 = 1)
#define mmc_enForceInterruptReq2    (reg_IFRL.bit.IF2 = 1)
#define mmc_enForceInterruptReq1    (reg_IFRL.bit.IF1 = 1)
#define mmc_enForceInterruptReq0    (reg_IFRL.bit.IF0 = 1)

/*  reg_IPR         Interrupt Pending Register */
#define mmc_testInterruptPending31  (reg_IPR.bit.IP31 == 1)
#define mmc_testInterruptPending30  (reg_IPR.bit.IP30 == 1)
#define mmc_testInterruptPending29  (reg_IPR.bit.IP29 == 1)
#define mmc_testInterruptPending28  (reg_IPR.bit.IP28 == 1)
#define mmc_testInterruptPending27  (reg_IPR.bit.IP27 == 1)
#define mmc_testInterruptPending26  (reg_IPR.bit.IP26 == 1)
#define mmc_testInterruptPending25  (reg_IPR.bit.IP25 == 1)
#define mmc_testInterruptPending24  (reg_IPR.bit.IP24 == 1)
#define mmc_testInterruptPending23  (reg_IPR.bit.IP23 == 1)
#define mmc_testInterruptPending22  (reg_IPR.bit.IP22 == 1)
#define mmc_testInterruptPending21  (reg_IPR.bit.IP21 == 1)
#define mmc_testInterruptPending20  (reg_IPR.bit.IP20 == 1)
#define mmc_testInterruptPending19  (reg_IPR.bit.IP19 == 1)
#define mmc_testInterruptPending18  (reg_IPR.bit.IP18 == 1)
#define mmc_testInterruptPending17  (reg_IPR.bit.IP17 == 1)
#define mmc_testInterruptPending16  (reg_IPR.bit.IP16 == 1)
#define mmc_testInterruptPending15  (reg_IPR.bit.IP15 == 1)
#define mmc_testInterruptPending14  (reg_IPR.bit.IP14 == 1)
#define mmc_testInterruptPending13  (reg_IPR.bit.IP13 == 1)
#define mmc_testInterruptPending12  (reg_IPR.bit.IP12 == 1)
#define mmc_testInterruptPending11  (reg_IPR.bit.IP11 == 1)
#define mmc_testInterruptPending10  (reg_IPR.bit.IP10 == 1)
#define mmc_testInterruptPending9   (reg_IPR.bit.IP9 == 1)
#define mmc_testInterruptPending8   (reg_IPR.bit.IP8 == 1)
#define mmc_testInterruptPending7   (reg_IPR.bit.IP7 == 1)
#define mmc_testInterruptPending6   (reg_IPR.bit.IP6 == 1)
#define mmc_testInterruptPending5   (reg_IPR.bit.IP5 == 1)
#define mmc_testInterruptPending4   (reg_IPR.bit.IP4 == 1)
#define mmc_testInterruptPending3   (reg_IPR.bit.IP3 == 1)
#define mmc_testInterruptPending2   (reg_IPR.bit.IP2 == 1)
#define mmc_testInterruptPending1   (reg_IPR.bit.IP1 == 1)
#define mmc_testInterruptPending0   (reg_IPR.bit.IP0 == 1)

/*  reg_NIER        Normal Interrupt Enable Register */
#define init_NIER    (0x0000) /*     (0b0000000000000000) */
#define mmc_enNormalInterrupt31     (reg_NIER.bit.NIE31 = 1)
#define mmc_enNormalInterrupt30     (reg_NIER.bit.NIE30 = 1)
#define mmc_enNormalInterrupt29     (reg_NIER.bit.NIE29 = 1)
#define mmc_enNormalInterrupt28     (reg_NIER.bit.NIE28 = 1)
#define mmc_enNormalInterrupt27     (reg_NIER.bit.NIE27 = 1)
#define mmc_enNormalInterrupt26     (reg_NIER.bit.NIE26 = 1)
#define mmc_enNormalInterrupt25     (reg_NIER.bit.NIE25 = 1)
#define mmc_enNormalInterrupt24     (reg_NIER.bit.NIE24 = 1)
#define mmc_enNormalInterrupt23     (reg_NIER.bit.NIE23 = 1)
#define mmc_enNormalInterrupt22     (reg_NIER.bit.NIE22 = 1)
#define mmc_enNormalInterrupt21     (reg_NIER.bit.NIE21 = 1)
#define mmc_enNormalInterrupt20     (reg_NIER.bit.NIE20 = 1)
#define mmc_enNormalInterrupt19     (reg_NIER.bit.NIE19 = 1)
#define mmc_enNormalInterrupt18     (reg_NIER.bit.NIE18 = 1)
#define mmc_enNormalInterrupt17     (reg_NIER.bit.NIE17 = 1)
#define mmc_enNormalInterrupt16     (reg_NIER.bit.NIE16 = 1)
#define mmc_enNormalInterrupt15     (reg_NIER.bit.NIE15 = 1)
#define mmc_enNormalInterrupt14     (reg_NIER.bit.NIE14 = 1)
#define mmc_enNormalInterrupt13     (reg_NIER.bit.NIE13 = 1)
#define mmc_enNormalInterrupt12     (reg_NIER.bit.NIE12 = 1)
#define mmc_enNormalInterrupt11     (reg_NIER.bit.NIE11 = 1)
#define mmc_enNormalInterrupt10     (reg_NIER.bit.NIE10 = 1)
#define mmc_enNormalInterrupt9      (reg_NIER.bit.NIE9 = 1)
#define mmc_enNormalInterrupt8      (reg_NIER.bit.NIE8 = 1)
#define mmc_enNormalInterrupt7      (reg_NIER.bit.NIE7 = 1)
#define mmc_enNormalInterrupt6      (reg_NIER.bit.NIE6 = 1)
#define mmc_enNormalInterrupt5      (reg_NIER.bit.NIE5 = 1)
#define mmc_enNormalInterrupt4      (reg_NIER.bit.NIE4 = 1)
#define mmc_enNormalInterrupt3      (reg_NIER.bit.NIE3 = 1)
#define mmc_enNormalInterrupt2      (reg_NIER.bit.NIE2 = 1)
#define mmc_enNormalInterrupt1      (reg_NIER.bit.NIE1 = 1)
#define mmc_enNormalInterrupt0      (reg_NIER.bit.NIE0 = 1)

/*  reg_NIPR        Normal Interrupt Pending Register */
#define mmc_testNormalIntPending31  (reg_NIPR.bit.NIP31 == 1)
#define mmc_testNormalIntPending30  (reg_NIPR.bit.NIP30 == 1)
#define mmc_testNormalIntPending29  (reg_NIPR.bit.NIP29 == 1)
#define mmc_testNormalIntPending28  (reg_NIPR.bit.NIP28 == 1)
#define mmc_testNormalIntPending27  (reg_NIPR.bit.NIP27 == 1)
#define mmc_testNormalIntPending26  (reg_NIPR.bit.NIP26 == 1)
#define mmc_testNormalIntPending25  (reg_NIPR.bit.NIP25 == 1)
#define mmc_testNormalIntPending24  (reg_NIPR.bit.NIP24 == 1)
#define mmc_testNormalIntPending23  (reg_NIPR.bit.NIP23 == 1)
#define mmc_testNormalIntPending22  (reg_NIPR.bit.NIP22 == 1)
#define mmc_testNormalIntPending21  (reg_NIPR.bit.NIP21 == 1)
#define mmc_testNormalIntPending20  (reg_NIPR.bit.NIP20 == 1)
#define mmc_testNormalIntPending19  (reg_NIPR.bit.NIP19 == 1)
#define mmc_testNormalIntPending18  (reg_NIPR.bit.NIP18 == 1)
#define mmc_testNormalIntPending17  (reg_NIPR.bit.NIP17 == 1)
#define mmc_testNormalIntPending16  (reg_NIPR.bit.NIP16 == 1)
#define mmc_testNormalIntPending15  (reg_NIPR.bit.NIP15 == 1)
#define mmc_testNormalIntPending14  (reg_NIPR.bit.NIP14 == 1)
#define mmc_testNormalIntPending13  (reg_NIPR.bit.NIP13 == 1)
#define mmc_testNormalIntPending12  (reg_NIPR.bit.NIP12 == 1)
#define mmc_testNormalIntPending11  (reg_NIPR.bit.NIP11 == 1)
#define mmc_testNormalIntPending10  (reg_NIPR.bit.NIP10 == 1)
#define mmc_testNormalIntPending9   (reg_NIPR.bit.NIP9 == 1)
#define mmc_testNormalIntPending8   (reg_NIPR.bit.NIP8 == 1)
#define mmc_testNormalIntPending7   (reg_NIPR.bit.NIP7 == 1)
#define mmc_testNormalIntPending6   (reg_NIPR.bit.NIP6 == 1)
#define mmc_testNormalIntPending5   (reg_NIPR.bit.NIP5 == 1)
#define mmc_testNormalIntPending4   (reg_NIPR.bit.NIP4 == 1)
#define mmc_testNormalIntPending3   (reg_NIPR.bit.NIP3 == 1)
#define mmc_testNormalIntPending2   (reg_NIPR.bit.NIP2 == 1)
#define mmc_testNormalIntPending1   (reg_NIPR.bit.NIP1 == 1)
#define mmc_testNormalIntPending0   (reg_NIPR.bit.NIP0 == 1)

/*  reg_FIER        Fast Interrupt Enable Register */
#define init_FIER    (0x0000) /*     (0b0000000000000000) */
#define mmc_enFastInterrupt31       (reg_FIER.bit.FIE31 = 1)
#define mmc_enFastInterrupt30       (reg_FIER.bit.FIE30 = 1)
#define mmc_disFastInterrupt30      (reg_FIER.bit.FIE30 = 0)
#define mmc_enFastInterrupt29       (reg_FIER.bit.FIE29 = 1)
#define mmc_enFastInterrupt28       (reg_FIER.bit.FIE28 = 1)
#define mmc_enFastInterrupt27       (reg_FIER.bit.FIE27 = 1)
#define mmc_enFastInterrupt26       (reg_FIER.bit.FIE26 = 1)
#define mmc_enFastInterrupt25       (reg_FIER.bit.FIE25 = 1)
#define mmc_enFastInterrupt24       (reg_FIER.bit.FIE24 = 1)
#define mmc_enFastInterrupt23       (reg_FIER.bit.FIE23 = 1)
#define mmc_enFastInterrupt22       (reg_FIER.bit.FIE22 = 1)
#define mmc_enFastInterrupt21       (reg_FIER.bit.FIE21 = 1)
#define mmc_enFastInterrupt20       (reg_FIER.bit.FIE20 = 1)
#define mmc_enFastInterrupt19       (reg_FIER.bit.FIE19 = 1)
#define mmc_enFastInterrupt18       (reg_FIER.bit.FIE18 = 1)
#define mmc_enFastInterrupt17       (reg_FIER.bit.FIE17 = 1)
#define mmc_enFastInterrupt16       (reg_FIER.bit.FIE16 = 1)
#define mmc_enFastInterrupt15       (reg_FIER.bit.FIE15 = 1)
#define mmc_enFastInterrupt14       (reg_FIER.bit.FIE14 = 1)
#define mmc_enFastInterrupt13       (reg_FIER.bit.FIE13 = 1)
#define mmc_enFastInterrupt12       (reg_FIER.bit.FIE12 = 1)
#define mmc_enFastInterrupt11       (reg_FIER.bit.FIE11 = 1)
#define mmc_enFastInterrupt10       (reg_FIER.bit.FIE10 = 1)
#define mmc_enFastInterrupt9        (reg_FIER.bit.FIE9 = 1)
#define mmc_enFastInterrupt8        (reg_FIER.bit.FIE8 = 1)
#define mmc_enFastInterrupt7        (reg_FIER.bit.FIE7 = 1)
#define mmc_enFastInterrupt6        (reg_FIER.bit.FIE6 = 1)
#define mmc_enFastInterrupt5        (reg_FIER.bit.FIE5 = 1)
#define mmc_enFastInterrupt4        (reg_FIER.bit.FIE4 = 1)
#define mmc_enFastInterrupt3        (reg_FIER.bit.FIE3 = 1)
#define mmc_enFastInterrupt2        (reg_FIER.bit.FIE2 = 1)
#define mmc_enFastInterrupt1        (reg_FIER.bit.FIE1 = 1)
#define mmc_enFastInterrupt0        (reg_FIER.bit.FIE0 = 1)

/*  reg_FIPR        Fast Interrupt Pending Register */
#define mmc_testFastIntPending31    (reg_FIPR.bit.FIP31 == 1)
#define mmc_testFastIntPending30    (reg_FIPR.bit.FIP30 == 1)
#define mmc_testFastIntPending29    (reg_FIPR.bit.FIP29 == 1)
#define mmc_testFastIntPending28    (reg_FIPR.bit.FIP28 == 1)
#define mmc_testFastIntPending27    (reg_FIPR.bit.FIP27 == 1)
#define mmc_testFastIntPending26    (reg_FIPR.bit.FIP26 == 1)
#define mmc_testFastIntPending25    (reg_FIPR.bit.FIP25 == 1)
#define mmc_testFastIntPending24    (reg_FIPR.bit.FIP24 == 1)
#define mmc_testFastIntPending23    (reg_FIPR.bit.FIP23 == 1)
#define mmc_testFastIntPending22    (reg_FIPR.bit.FIP22 == 1)
#define mmc_testFastIntPending21    (reg_FIPR.bit.FIP21 == 1)
#define mmc_testFastIntPending20    (reg_FIPR.bit.FIP20 == 1)
#define mmc_testFastIntPending19    (reg_FIPR.bit.FIP19 == 1)
#define mmc_testFastIntPending18    (reg_FIPR.bit.FIP18 == 1)
#define mmc_testFastIntPending17    (reg_FIPR.bit.FIP17 == 1)
#define mmc_testFastIntPending16    (reg_FIPR.bit.FIP16 == 1)
#define mmc_testFastIntPending15    (reg_FIPR.bit.FIP15 == 1)
#define mmc_testFastIntPending14    (reg_FIPR.bit.FIP14 == 1)
#define mmc_testFastIntPending13    (reg_FIPR.bit.FIP13 == 1)
#define mmc_testFastIntPending12    (reg_FIPR.bit.FIP12 == 1)
#define mmc_testFastIntPending11    (reg_FIPR.bit.FIP11 == 1)
#define mmc_testFastIntPending10    (reg_FIPR.bit.FIP10 == 1)
#define mmc_testFastIntPending9     (reg_FIPR.bit.FIP9 == 1)
#define mmc_testFastIntPending8     (reg_FIPR.bit.FIP8 == 1)
#define mmc_testFastIntPending7     (reg_FIPR.bit.FIP7 == 1)
#define mmc_testFastIntPending6     (reg_FIPR.bit.FIP6 == 1)
#define mmc_testFastIntPending5     (reg_FIPR.bit.FIP5 == 1)
#define mmc_testFastIntPending4     (reg_FIPR.bit.FIP4 == 1)
#define mmc_testFastIntPending3     (reg_FIPR.bit.FIP3 == 1)
#define mmc_testFastIntPending2     (reg_FIPR.bit.FIP2 == 1)
#define mmc_testFastIntPending1     (reg_FIPR.bit.FIP1 == 1)
#define mmc_testFastIntPending0     (reg_FIPR.bit.FIP0 == 1)

/*  reg_PLSR0       Priority Level Select Register 0 */
/*      interrupt source:   ADC     PF1     Queue 1 conversion pause */
#define init_PLSR0                  (0x08)

/*  reg_PLSR1       Priority Level Select Register 1 */
/*      interrupt source:   ADC     CF1     Queue 1 conversion complete */
#define init_PLSR1                  (0x09)

/*  reg_PLSR2       Priority Level Select Register 2 */
/*      interrupt source:   ADC     PF2     Queue 2 conversion pause */
#define init_PLSR2                  (0x0a)

/*  reg_PLSR3       Priority Level Select Register 3 */
/*      interrupt source:   ADC     CF1     Queue 2 conversion complete */
#define init_PLSR3                  (0x0b)

#define init_PLSR_0_3   ((init_PLSR0<<24)+(init_PLSR1<<16)+(init_PLSR2<<8)+(init_PLSR3))

/*  reg_PLSR4       Priority Level Select Register 4 */
/*      interrupt source:   SPI     MODF    Mode fault */
#define init_PLSR4                  (0x0c)

/*  reg_PLSR5       Priority Level Select Register 5 */
/*      interrupt source:   SPI     SPIF    Transfer complete */
#define init_PLSR5                  (0x0d)

/*  reg_PLSR6       Priority Level Select Register 6 */
/*      interrupt source:   SCI1    TDRE    Transmit data register empty */
#define init_PLSR6                  (0x0e)

/*  reg_PLSR7       Priority Level Select Register 7 */
/*      interrupt source:   SCI1    TC      Transmit complete */
#define init_PLSR7                  (0x0f)

#define init_PLSR_4_7   ((init_PLSR4<<24)+(init_PLSR5<<16)+(init_PLSR6<<8)+(init_PLSR7))

/*  reg_PLSR8       Priority Level Select Register 8 */
/*      interrupt source:   SCI1    RDRF    Receive data register full */
#define init_PLSR8                  (0x15)

/*  reg_PLSR9       Priority Level Select Register 9 */
/*      interrupt source:   SCI1    OR      Receiver overrun */
#define init_PLSR9                  (0x11)

/*  reg_PLSR10      Priority Level Select Register 10 */
/*      interrupt source:   SCI1    IDLE    Reciever line idle */
#define init_PLSR10                 (0x12)

/*  reg_PLSR11      Priority Level Select Register 11 */
/*      interrupt source:   SCI2    TDRE    Transmit data register empty */
#define init_PLSR11                 (0x13)

#define init_PLSR_8_11  ((init_PLSR8<<24)+(init_PLSR9<<16)+(init_PLSR10<<8)+(init_PLSR11))

/*  reg_PLSR12      Priority Level Select Register 12 */
/*      interrupt source:   SCI2    TC      Transmit complete */
#define init_PLSR12                 (0x14)

/*  reg_PLSR13      Priority Level Select Register 13 */
/*      interrupt source:   SCI2    RDRF    Receive data register full */
#define init_PLSR13                 (0x01)

/*  reg_PLSR14      Priority Level Select Register 14 */
/*      interrupt source:   SCI2    OR      Receiver overrun */
#define init_PLSR14                 (0x16)

/*  reg_PLSR15      Priority Level Select Register 15 */
/*      interrupt source:   SCI2    IDLE    Reciever line idle */
#define init_PLSR15                 (0x17)

#define init_PLSR_12_15 ((init_PLSR12<<24)+(init_PLSR13<<16)+(init_PLSR14<<8)+(init_PLSR15))

/*  reg_PLSR16      Priority Level Select Register 16 */
/*      interrupt source:   TIM1    C0F     Timer channel 0 */
#define init_PLSR16                 (0x18)

/*  reg_PLSR17      Priority Level Select Register 17 */
/*      interrupt source:   TIM1    C1F     Timer channel 1 */
#define init_PLSR17                 (0x19)

/*  reg_PLSR18      Priority Level Select Register 18 */
/*      interrupt source:   TIM1    C2F     Timer channel 2 */
#define init_PLSR18                 (0x1a)

/*  reg_PLSR19      Priority Level Select Register 19 */
/*      interrupt source:   TIM1    C3F     Timer channel 3 */
#define init_PLSR19                 (0x1b)

#define init_PLSR_16_19 ((init_PLSR16<<24)+(init_PLSR17<<16)+(init_PLSR18<<8)+(init_PLSR19))

/*  reg_PLSR20      Priority Level Select Register 20 */
/*      interrupt source:   TIM1    TOF     Timer overflow */
#define init_PLSR20                 (0x1c)

/*  reg_PLSR21      Priority Level Select Register 21 */
/*      interrupt source:   TIM1    PAIF    Pulse accumulator input */
#define init_PLSR21                 (0x1d)

/*  reg_PLSR22      Priority Level Select Register 22 */
/*      interrupt source:   TIM1    PAOVF   Pulse accumulator overflow */
#define init_PLSR22                 (0x1f)

/*  reg_PLSR23      Priority Level Select Register 23 */
/*      interrupt source:   TIM2    C0F     Timer channel 0 */
#define init_PLSR23                 (0x1f)

#define init_PLSR_20_23 ((init_PLSR20<<24)+(init_PLSR21<<16)+(init_PLSR22<<8)+(init_PLSR23))

/*  reg_PLSR24      Priority Level Select Register 24 */
/*      interrupt source:   TIM2    C1F     Timer channel 1 */
#define init_PLSR24                 (0x1f)

/*  reg_PLSR25      Priority Level Select Register 25 */
/*      interrupt source:   TIM2    C2F     Timer channel 2 */
#define init_PLSR25                 (0x1f)

/*  reg_PLSR26      Priority Level Select Register 26 */
/*      interrupt source:   TIM2    C3F     Timer channel 3 */
#define init_PLSR26                 (0x1f)

/*  reg_PLSR27      Priority Level Select Register 27 */
/*      interrupt source:   TIM2    TOF     Timer overflow */
#define init_PLSR27                 (0x1f)

#define init_PLSR_24_27 ((init_PLSR24<<24)+(init_PLSR25<<16)+(init_PLSR26<<8)+(init_PLSR27))

/*  reg_PLSR28      Priority Level Select Register 28 */
/*      interrupt source:   TIM2    PAIF    Pulse accumulator input */
#define init_PLSR28                 (0x1f)

/*  reg_PLSR29      Priority Level Select Register 29 */
/*      interrupt source:   TIM2    PAOVF   Pulse accumulator overflow */
#define init_PLSR29                 (0x1f)

/*  reg_PLSR30      Priority Level Select Register 30 */
/*      interrupt source:   PIT1    PIF     PIT interrupt flag */
/* Set to priority level 31 interrupt */
#define init_PLSR30                 (0x1d) // was 0x1f

/*  reg_PLSR31      Priority Level Select Register 31 */
/*      interrupt source:   PIT2    PIF     PIT interrupt flag */
#define init_PLSR31                 (0x1c)

#define init_PLSR_28_31 ((init_PLSR28<<24)+(init_PLSR29<<16)+(init_PLSR30<<8)+(init_PLSR31))

/*  reg_PLSR32      Priority Level Select Register 32 */
/*      interrupt source:   EPORT   EPF0    Edge port flag 0 */ // On Key IRQ
#define init_PLSR32                 (0x04)

/*  reg_PLSR33      Priority Level Select Register 33 */
/*      interrupt source:   EPORT   EPF1    Edge port flag 1 */ // Off Key IRQ
#define init_PLSR33                 (0x05)

/*  reg_PLSR34      Priority Level Select Register 34 */
/*      interrupt source:   EPORT   EPF2    Edge port flag 2 */ // LAN IRQ
#define init_PLSR34                 (0x06)

/*  reg_PLSR35      Priority Level Select Register 35 */
/*      interrupt source:   EPORT   EPF3    Edge port flag 3 */ // Keypad IRQ
#define init_PLSR35                 (0x00)

#define init_PLSR_32_35 ((init_PLSR32<<24)+(init_PLSR33<<16)+(init_PLSR34<<8)+(init_PLSR35))

/*  reg_PLSR36      Priority Level Select Register 36 */
/*      interrupt source:   EPORT   EPF4    Edge port flag 4 */ // USB Host IRQ
#define init_PLSR36                 (0x07)

/*  reg_PLSR37      Priority Level Select Register 37 */
/*      interrupt source:   EPORT   EPF5    Edge port flag 5 */ // USB Device IRQ
#define init_PLSR37                 (0x08)

/*  reg_PLSR38      Priority Level Select Register 38 */
/*      interrupt source:   EPORT   EPF6    Edge port flag 6 */ // RTC IRQ
#define init_PLSR38                 (0x03)

/*  reg_PLSR39      Priority Level Select Register 39 */
/*      interrupt source:   EPORT   EPF7    Edge port flag 7 */ // Ext. Trig IRQ
#define init_PLSR39                 (0x1e)

#define init_PLSR_36_39 ((init_PLSR36<<24)+(init_PLSR37<<16)+(init_PLSR38<<8)+(init_PLSR39))

/*  Edge Port Module  *********************************************************/
/*  reg_EPPAR       EPORT Pin Assignment Register */
/*#define init_EPPAR (0x0000)   */   /* (0b0000000000000000)        */
//#define init_EPPAR (0x420A)          /* (0b0100001000001010)        */
#define init_EPPAR (0xAA0A)          /* (0b0110101000001010)        */
/*                                         ||||||||||||||||__ EPPA0 */
/* EPPAx| INTx Pin Configuration           ||||||||||||||____ EPPA1 */
/* -----|------------------------          ||||||||||||______ EPPA2 */
/*   00 | level-sensitive                  ||||||||||________ EPPA3 */
/*   01 | rising-edge triggered            ||||||||__________ EPPA4 */
/*   10 | falling-edge triggered           ||||||____________ EPPA5 */
/*   11 | rising or falling edge triggered ||||______________ EPPA6 */
/*                                         ||________________ EPPA7 */

/*  reg_EPDDR       EPORT Data Direction Register */
#define init_EPDDR (0x00)   /*      (0b00000000) */
/*                                     ||||||||__ EPDD0 */
/*                                     |||||||___ EPDD1 */
/*                                     ||||||____ EPDD2 */
/*                                     |||||_____ EPDD3 */
/*                                     ||||______ EPDD4 */
/*                                     |||_______ EPDD5 */
/*                                     ||________ EPDD6 */
/*                                     |_________ EPDD7 */

/*  reg_EPIER       EPORT Interrupt Enable Register */
//#define init_EPIER (0x00) /*        (0b00000000) */
#define init_EPIER (0xff) /*        (0b00010000) */
/*                                     ||||||||__ EPIE0 */
/*                                     |||||||___ EPIE1 */
/*                                     ||||||____ EPIE2 */
/*                                     |||||_____ EPIE3 */
/*                                     ||||______ EPIE4 */
/*                                     |||_______ EPIE5 */
/*                                     ||________ EPIE6 */
/*                                     |_________ EPIE7 */

/*  reg_EPDR        EPORT Port Data Register */
#define init_EPDR (0xFF) /*         (0b11111111) */
/*                                     ||||||||__ EPD0 */
/*                                     |||||||___ EPD1 */
/*                                     ||||||____ EPD2 */
/*                                     |||||_____ EPD3 */
/*                                     ||||______ EPD4 */
/*                                     |||_______ EPD5 */
/*                                     ||________ EPD6 */
/*                                     |_________ EPD7 */

/*  reg_EPFR        EPORT Port Flag Register */

// Note: Changed these defines to fix a bug where the bit operations on the port interrupt 
// register actually cause other interrupt flags to be cleared before being processed.
// The old code is commented out to the side.
#define mmc_clear_EPF0_int reg_EPFR.reg = 0x01; //(reg_EPFR.bit.EPF0 = 1)
#define mmc_clear_EPF1_int reg_EPFR.reg = 0x02; //(reg_EPFR.bit.EPF1 = 1)
#define mmc_clear_EPF2_int reg_EPFR.reg = 0x04; //(reg_EPFR.bit.EPF2 = 1)
#define mmc_clear_EPF3_int reg_EPFR.reg = 0x08; //(reg_EPFR.bit.EPF3 = 1)
#define mmc_clear_EPF4_int reg_EPFR.reg = 0x10; //(reg_EPFR.bit.EPF4 = 1)
#define mmc_clear_EPF5_int reg_EPFR.reg = 0x20; //(reg_EPFR.bit.EPF5 = 1)
#define mmc_clear_EPF6_int reg_EPFR.reg = 0x40; //(reg_EPFR.bit.EPF6 = 1)
#define mmc_clear_EPF7_int reg_EPFR.reg = 0x80; //(reg_EPFR.bit.EPF7 = 1)

/*  Watchdog Timer Module  ****************************************************/
/*  reg_WCR         Watchdog Control Register */
/*   This is a write-once-after-reset register.  This initialization disables 
    the watchdog in all modes.  This is different from the default for this 
    register, which enables the watchdog in normal mode.  If watchdog operation 
    is desired, then this line must be changed!!!              */
#define init_WCR (0x000E)       /*  (0b0000000000001110)       */
/*                                                 ||||__ EN   */
/*                                                 |||___ DBG  */
/*                                                 ||____ DOZE */
/*                                                 |_____ WAIT */

/*  reg_WMR         Watchdog Modulus Register */
/*   This is a write-once-after-reset register.  This initialization sets the
    watchdog timeout for the maximum duration.   This is the same as the default
    for this register.  If another timeout value is desired, then this line must
    be changed!!!                                            */
#define init_WMR (0xFFFF)

/*  reg_WSR         Watchdog Service Register */
#define mmc_ResetWatchdog         reg_WSR = 0x5555; reg_WSR = 0xAAAA

/*  reg_WCNTR       Watchdog Count Register */
#define mmc_WatchdogCount         (reg_WCNTR)

/*  Programmable Interrupt Timer Modules  *************************************/
/*  reg_PCSR1       PIT1 Control and Status Register */
/* Prescaler = 32768; Enable PIT1 interrupts; Make sure PIF is cleared */
/* Automatically reload modulus value into PIT ctr; Enable PIT1 */
/* Allow immediate overwrite of counter by write to Mod Reg */
#define init_PCSR1 (0x051E)     /*  (0b0000111101011110)        */
/*                                         |||| |||||||__ EN    */
/*                                         |||| ||||||___ RLD   */
/*                                         |||| |||||____ PIF   */
/*                                         |||| ||||_____ PIE   */
/*                                         |||| |||______ OVW   */
/*                                         |||| ||_______ PDBG  */
/*                                         |||| |________ PDOZE */
/*                                         ||||__________ PRE   */

/*  reg_PMR1        PIT1 Modulus Register */
#define init_PMR1    (0x03E8) // Set for 1ms at a 32MHz clock

#define mmc_clear_PIT1_int (reg_PCSR1.bit.PIF = 1)

/*  reg_PCNTR1      PIT1 Count Register */
#define mmc_PIT1Count         (reg_PCNTR1)

/*  reg_PCSR2       PIT2 Control and Status Register */
#define init_PCSR2  (0x071E)     /* (0b0000000001000000) */
/*                                         |||| |||||||__ EN    */
/*                                         |||| ||||||___ RLD   */
/*                                         |||| |||||____ PIF   */
/*                                         |||| ||||_____ PIE   */
/*                                         |||| |||______ OVW   */
/*                                         |||| ||_______ PDBG  */
/*                                         |||| |________ PDOZE */
/*                                         ||||__________ PRE   */

/*  reg_PMR2        PIT2 Modulus Register */
#define init_PMR2    (0x0271) // 0x0271, which corresponds to 10 msec with system clock divider of 128
#define mmc_clear_PIT2_int (reg_PCSR2.bit.PIF = 1)

/*  reg_PCNTR2      PIT2 Count Register */
#define mmc_PIT2Count         (reg_PCNTR2)

/*  Queued Analog-to-Digital Converter Module  ********************************/
/*  reg_QADCMCR     QADC Module Configuration Register */
//#define init_QADCMCR (0x0080) /*    (0b0000000010000000) */
#define init_QADCMCR (0x0000) /*    (0b0000000000000000) */
/*                                     ||      |_________ SUPV */
/*                                     ||________________ QDBG */
/*                                     |_________________ QSTOP */

/*  reg_PORTQA      QADC Port A Data Register */
#define init_PORTQA  (0x00) /*      (0b00000000) */
/*                                        || ||__ PQA0 */
/*                                        || |___ PQA1 */
/*                                        ||_____ PQA3 */
/*                                        |______ PQA4 */

/*  reg_PORTQB      QADC Port B Data Register */
#define init_PORTQB (0x00) /*       (0b00000000) */
/*                                         ||||__ PQB0 */
/*                                         |||___ PQB1 */
/*                                         ||____ PQB2 */
/*                                         |_____ PQB3 */

/*  reg_DDRQA       QADC Port A Data Direction Register */
// Set both PQA0 and PQA1 to be digital outputs
#define init_DDRQA  (0x1B) /*     (0b00000000) */
/*                                      || ||__________ DDQA0 */
/*                                      || |___________ DDQA1 */
/*                                      ||_____________ DDQA3 */
/*                                      |______________ DDQA4 */

/*  reg_DDRQB       QADC Port B Data Direction Register */
#define init_DDRQB  (0x08) /*     (0b00001000) */
/*                                       ||||__________ DDQB0 */
/*                                       |||___________ DDQB1 */
/*                                       ||____________ DDQB2 */
/*                                       |_____________ DDQB3 */

/*  reg_QACR0       QADC Control Register 0 */
//#define init_QACR0 (0x0013) /*      (0b0000000000010011) */
//#define init_QACR0 (0x000e) /*      (0b0000000000001110) */
#define init_QACR0 (0x0007) /*      (0b0000000000001111) */
/*                                     |  |     |||||||__ QPR */
/*                                     |  |______________ TRG */
/*                                     |_________________ MUX */

/*  reg_QACR1       QADC Control Register 1 */
#define init_QACR1 (0x0000) /*      (0b0000000000000000) */
/*                                     ||||||||__________ MQ1 */
/*                                     |||_______________ SSE1 */
/*                                     ||________________ PIE1 */
/*                                     |_________________ CIE1 */

/*  reg_QACR2       QADC Control Register 2 */
//#define init_QACR2 (0x007F) /*      (0b0000000001111111) */
#define init_QACR2 (0x1100) /*      (0b0001000100000000) */
/*                                     ||||||||||||||||__ BQ */
/*                                     |||||||||_________ RESUME */
/*                                     ||||||||__________ MQ2 */
/*                                     |||_______________ SSE2 */
/*                                     ||________________ PIE2 */
/*                                     |_________________ CIE2 */
#define mmc_QADC_BeginningOfQueue2  (reg_QACR2.bit.BQ)

/*  reg_QASR0       QADC Status Register 0 */
#define mmc_testQADC_Queue1Complete (reg_QASR0.bit.CF1 == 1)
#define mmc_testQADC_Queue1Pause    (reg_QASR0.bit.PF1 == 1)
#define mmc_testQADC_Queue2Complete (reg_QASR0.bit.CF2 == 1)
#define mmc_testQADC_Queue2Pause    (reg_QASR0.bit.PF2 == 1)
#define mmc_testQADC_Queue1TrigOR   (reg_QASR0.bit.TOR1 == 1)
#define mmc_testQADC_Queue2TrigOR   (reg_QASR0.bit.TOR2 == 1)
#define mmc_QADC_QueueStatus        (reg_QASR0.bit.QS)
#define mmc_QADC_CommandWordPtr     (reg_QASR0.bit.CWP)

/*  reg_QASR1       QADC Status Register 1 */
#define mmc_QADC1_CommandWordPtr   (reg_QASR1.bit.CWPQ1)
#define mmc_QADC2_CommandWordPtr   (reg_QASR1.bit.CWPQ2)

/*  reg_CCW         Conversion Command Word Table */
/*  init_CCW?? (0x0000)              (0b0000000000000000) */
/*                                            ||||||||||__ CHAN  */
/*                                            ||||________ IST   */
/*                                            ||__________ BYP   */
/*                                            |___________ P     */
#define init_CCW0  (0x0080) /*        (0b0000000000000000) */
#define init_CCW1  (0x0081) /*        (0b0000000000000000) */
#define init_CCW2  (0x0080) /*        (0b0000000000000000) */
#define init_CCW3  (0x0081) /*        (0b0000000000000000) */
#define init_CCW4  (0x003f) /*        (0b0000000000000000) */
#define init_CCW5  (0x0000) /*        (0b0000000000000000) */
#define init_CCW6  (0x0000) /*        (0b0000000000000000) */
#define init_CCW7  (0x0000) /*        (0b0000000000000000) */
#define init_CCW8  (0x0000) /*        (0b0000000000000000) */
#define init_CCW9  (0x0000) /*        (0b0000000000000000) */
#define init_CCW10 (0x0000) /*        (0b0000000000000000) */
#define init_CCW11 (0x0000) /*        (0b0000000000000000) */
#define init_CCW12 (0x0000) /*        (0b0000000000000000) */
#define init_CCW13 (0x0000) /*        (0b0000000000000000) */
#define init_CCW14 (0x0000) /*        (0b0000000000000000) */
#define init_CCW15 (0x0000) /*        (0b0000000000000000) */
#define init_CCW16 (0x0000) /*        (0b0000000000000000) */
#define init_CCW17 (0x0000) /*        (0b0000000000000000) */
#define init_CCW18 (0x0000) /*        (0b0000000000000000) */
#define init_CCW19 (0x0000) /*        (0b0000000000000000) */
#define init_CCW20 (0x0000) /*        (0b0000000000000000) */
#define init_CCW21 (0x0000) /*        (0b0000000000000000) */
#define init_CCW22 (0x0000) /*        (0b0000000000000000) */
#define init_CCW23 (0x0000) /*        (0b0000000000000000) */
#define init_CCW24 (0x0000) /*        (0b0000000000000000) */
#define init_CCW25 (0x0000) /*        (0b0000000000000000) */
#define init_CCW26 (0x0000) /*        (0b0000000000000000) */
#define init_CCW27 (0x0000) /*        (0b0000000000000000) */
#define init_CCW28 (0x0000) /*        (0b0000000000000000) */
#define init_CCW29 (0x0000) /*        (0b0000000000000000) */
#define init_CCW30 (0x0000) /*        (0b0000000000000000) */
#define init_CCW31 (0x0000) /*        (0b0000000000000000) */
#define init_CCW32 (0x0000) /*        (0b0000000000000000) */
#define init_CCW33 (0x0000) /*        (0b0000000000000000) */
#define init_CCW34 (0x0000) /*        (0b0000000000000000) */
#define init_CCW35 (0x0000) /*        (0b0000000000000000) */
#define init_CCW36 (0x0000) /*        (0b0000000000000000) */
#define init_CCW37 (0x0000) /*        (0b0000000000000000) */
#define init_CCW38 (0x0000) /*        (0b0000000000000000) */
#define init_CCW39 (0x0000) /*        (0b0000000000000000) */
#define init_CCW40 (0x0000) /*        (0b0000000000000000) */
#define init_CCW41 (0x0000) /*        (0b0000000000000000) */
#define init_CCW42 (0x0000) /*        (0b0000000000000000) */
#define init_CCW43 (0x0000) /*        (0b0000000000000000) */
#define init_CCW44 (0x0000) /*        (0b0000000000000000) */
#define init_CCW45 (0x0000) /*        (0b0000000000000000) */
#define init_CCW46 (0x0000) /*        (0b0000000000000000) */
#define init_CCW47 (0x0000) /*        (0b0000000000000000) */
#define init_CCW48 (0x0000) /*        (0b0000000000000000) */
#define init_CCW49 (0x0000) /*        (0b0000000000000000) */
#define init_CCW50 (0x0000) /*        (0b0000000000000000) */
#define init_CCW51 (0x0000) /*        (0b0000000000000000) */
#define init_CCW52 (0x0000) /*        (0b0000000000000000) */
#define init_CCW53 (0x0000) /*        (0b0000000000000000) */
#define init_CCW54 (0x0000) /*        (0b0000000000000000) */
#define init_CCW55 (0x0000) /*        (0b0000000000000000) */
#define init_CCW56 (0x0000) /*        (0b0000000000000000) */
#define init_CCW57 (0x0000) /*        (0b0000000000000000) */
#define init_CCW58 (0x0000) /*        (0b0000000000000000) */
#define init_CCW59 (0x0000) /*        (0b0000000000000000) */
#define init_CCW60 (0x0000) /*        (0b0000000000000000) */
#define init_CCW61 (0x0000) /*        (0b0000000000000000) */
#define init_CCW62 (0x0000) /*        (0b0000000000000000) */
#define init_CCW63 (0x0000) /*        (0b0000000000000000) */


/*  Serial Peripheral Interface Module  ***************************************/
/*  reg_SPICR1      SPI Control Register 1 */
#define init_SPICR1 (0x04) /*       (0b00000100) */
/*                                     ||||||||__ LSBFE */
/*                                     |||||||___ SSOE */
/*                                     ||||||____ CPHA */
/*                                     |||||_____ CPOL */
/*                                     ||||______ MSTR */
/*                                     |||_______ SWOM */
/*                                     ||________ SPE */
/*                                     |_________ SPIE */

/*  reg_SPICR2      SPI Control Register 2 */
#define init_SPICR2 (0x00) /*       (0b00000000) */
/*                                           ||__ SPC0 */
/*                                           |___ SPISDOZ */

/*  reg_SPIBR       SPI Baud Rate Register */
#define init_SPIBR (0x00) /*        (0b00000000) */
/*                                      ||| |||__ SPR */
/*                                      |||______ SPPR */

/*  reg_SPISR       SPI Status Register */
#define mmc_testSPI_Interrupt       (reg_SPISR.bit.SPIF == 1)
#define mmc_testSPI_WriteCollision  (reg_SPISR.bit.WCOL == 1)
#define mmc_testSPI_ModeFault       (reg_SPISR.bit.MODF == 1)

/*  reg_SPUPURD     SPI Pullup and Reduced Drive Register */
#define init_SPIPURD (0x00) /*      (0b00000000) */
/*                                        |   |__ PUPSP */
/*                                        |______ RDSP */

/*  reg_SPIPORT     SPI Port Data Register */
#define init_SPIPORT (0x00) /*      (0b00000000) */
/*                                         ||||__ PORTSP0 */
/*                                         |||___ PORTSP1 */
/*                                         ||____ PORTSP2 */
/*                                         |_____ PORTSP3 */

/*  reg_SPIDDR      SPI Port Data Direction Register */
#define init_SPIDDR (0x00) /*       (0b00000000) */
/*                                         ||||__ DDRSP0 */
/*                                         |||___ DDRSP1 */
/*                                         ||____ DDRSP2 */
/*                                         |_____ DDRSP3 */

/*  Serial Communications Interface Modules  **********************************/
/*  SCI 1 */
/*  reg_SCI1BD      SCI1 Baud Rate Register */
#define init_SCI1BD                 (0x0004)

/*  reg_SCI1CR1     SCI1 Control Register 1 */
#define init_SCI1CR1 (0x00) /*      (0b00000000) */
/*                                     ||||||||__ PT */
/*                                     |||||||___ PE */
/*                                     ||||||____ ILT */
/*                                     |||||_____ WAKE */
/*                                     ||||______ M */
/*                                     |||_______ RSRC */
/*                                     ||________ WOMS */
/*                                     |_________ LOOPS */

/*  reg_SCI1CR2     SCI1 Control Register 2 */
#define init_SCI1CR2 (0x00) /*      (0b00000000) */
/*                                     ||||||||__ SBK */
/*                                     |||||||___ RWU */
/*                                     ||||||____ RE */
/*                                     |||||_____ TE */
/*                                     ||||______ ILIE */
/*                                     |||_______ RIE */
/*                                     ||________ TCIE */
/*                                     |_________ TIE */

/*  reg_SCI1SR1     SCI1 Status Register 1 */
#define mmc_testSCI1_TxD_Empty      (reg_SCI1SR1.bit.TDRE == 1)
#define mmc_testSCI1_TxD_Complete   (reg_SCI1SR1.bit.TC == 1)
#define mmc_testSCI1_RxD_Full       (reg_SCI1SR1.bit.RDRF == 1)
#define mmc_testSCI1_RxD_Idle       (reg_SCI1SR1.bit.IDLE == 1)
#define mmc_testSCI1_Overrun        (reg_SCI1SR1.bit.OR == 1)
#define mmc_testSCI1_Noise          (reg_SCI1SR1.bit.NF == 1)
#define mmc_testSCI1_FramingError   (reg_SCI1SR1.bit.FE == 1)
#define mmc_testSCI1_ParityError    (reg_SCI1SR1.bit.PF == 1)

/*  reg_SCI1SR2     SCI1 Status Register 2 */
#define mmc_testSCI1_RxD_Active     (reg_SCI1SR2.bit.RAF == 1)

/*  reg_SCI1DRH     SCI1 Data Register High */
#define mmc_SCI1_RxD8               (reg_SCI1DRH.bit.R8)
#define mmc_SCI1_TxD8               (reg_SCI1DRH.bit.T8)

/*  reg_SCI1DRL     SCI1 Data Register Low */
#define mmc_SCI1_Data               (reg_SCI1DRL)

/*  reg_SCI1PURD    SCI1 Pullup and Reduced Drive Register */
#define init_SCI1PURD (0x00) /*     (0b10000000) */
/*                                     |  |   |__ PUPSCI */
/*                                     |  |______ RDPSCI */
/*                                     |_________ SCISDOZ */

/*  reg_SCI1PORT    SCI1 Port Data Register */
#define init_SCI1PORT (0x00) /*     (0b00000000) */
/*                                           ||__ PORTSC0 */
/*                                           |___ PORTSC1 */

/*  reg_SCI1DDR     SCI1 Data Direction Register */
#define init_SCI1DDR (0x00) /*      (0b00000000) */
/*                                           ||__ DDRSC0 */
/*                                           |___ DDRSC1 */

/*  SCI 2 */
/*  reg_SCI2BD      SCI2 Baud Rate Register */
#define init_SCI2BD (0x0004)

/*  reg_SCI2CR1     SCI2 Control Register 1 */
#define init_SCI2CR1 (0x00) /*      (0b00000000) */
/*                                     ||||||||__ PT */
/*                                     |||||||___ PE */
/*                                     ||||||____ ILT */
/*                                     |||||_____ WAKE */
/*                                     ||||______ M */
/*                                     |||_______ RSRC */
/*                                     ||________ WOMS */
/*                                     |_________ LOOPS */

/*  reg_SCI2CR2     SCI2 Control Register 2 */
#define init_SCI2CR2 (0x00) /*      (0b00000000) */
/*                                     ||||||||__ SBK */
/*                                     |||||||___ RWU */
/*                                     ||||||____ RE */
/*                                     |||||_____ TE */
/*                                     ||||______ ILIE */
/*                                     |||_______ RIE */
/*                                     ||________ TCIE */
/*                                     |_________ TIE */

/*  reg_SCI2SR1     SCI2 Status Register 1 */
#define mmc_testSCI2_TxD_Empty      (reg_SCI2SR1.bit.TDRE == 1)
#define mmc_testSCI2_TxD_Complete   (reg_SCI2SR1.bit.TC == 1)
#define mmc_testSCI2_RxD_Full       (reg_SCI2SR1.bit.RDRF == 1)
#define mmc_testSCI2_RxD_Idle       (reg_SCI2SR1.bit.IDLE == 1)
#define mmc_testSCI2_Overrun        (reg_SCI2SR1.bit.OR == 1)
#define mmc_testSCI2_Noise          (reg_SCI2SR1.bit.NF == 1)
#define mmc_testSCI2_FramingError   (reg_SCI2SR1.bit.FE == 1)
#define mmc_testSCI2_ParityError    (reg_SCI2SR1.bit.PF == 1)

/*  reg_SCI2SR2     SCI2 Status Register 2 */
#define mmc_testSCI2_RxD_Active     (reg_SCI2SR2.bit.RAF == 1)

/*  reg_SCI2DRH     SCI2 Data Register High */
#define mmc_SCI2_RxD8               (reg_SCI2DRH.bit.R8)
#define mmc_SCI2_TxD8               (reg_SCI2DRH.bit.T8)

/*  reg_SCI2DRL     SCI2 Data Register Low */
#define mmc_SCI2_Data               (reg_SCI2DRL)

/*  reg_SCI2PURD    SCI2 Pullup and Reduced Drive Register */
#define init_SCI2PURD (0x00) /*     (0b00000000) */
/*                                     |  |   |__ PUPSCI */
/*                                     |  |______ RDPSCI */
/*                                     |_________ SCISDOZ */

/*  reg_SCI2PORT    SCI2 Port Data Register */
#define init_SCI2PORT (0x00) /*     (0b00000000) */
/*                                           ||__ PORTSC0 */
/*                                           |___ PORTSC1 */

/*  reg_SCI2DDR     SCI2 Data Direction Register */
//#define init_SCI2DDR (0x00) /*      (0b00000000) */
#define init_SCI2DDR (0x02) /*      (0b00000000) */
/*                                           ||__ DDRSC0 */
/*                                           |___ DDRSC1 */

/*  Timer Modules  ************************************************************/
/*  Timer 1 */
/*  reg_TIM1IOS     TIM1 Input Capture / Output Compare Select Register */
#define init_TIM1IOS (0x00) /*      (0b00000000) */
/*                                         ||||__ IOS0 */
/*                                         |||___ IOS1 */
/*                                         ||____ IOS2 */
/*                                         |_____ IOS3 */

/*  reg_TIM1CFORC   TIM1 Compare Force Register */
#define init_TIM1CFORC (0x00) /*    (0b00000000) */
/*                                         ||||__ FOC0 */
/*                                         |||___ FOC1 */
/*                                         ||____ FOC2 */
/*                                         |_____ FOC3 */

/*  reg_TIM1OC3M    TIM1 Output Compare 3 Mask Register */
#define init_TIM1OC3M (0x00) /*     (0b00000000) */
/*                                         ||||__ OC3M0 */
/*                                         |||___ OC3M1 */
/*                                         ||____ OC3M2 */
/*                                         |_____ OC3M3 */

/*  reg_TIM1OC3D    TIM1 Output Compare 3 Data Register */
#define init_TIM1OC3D (0x00) /*     (0b00000000) */
/*                                         ||||__ OC3D0 */
/*                                         |||___ OC3D1 */
/*                                         ||____ OC3D2 */
/*                                         |_____ OC3D3 */

#define init_TIM1FOC ((init_TIM1IOS<<24)+(init_TIM1CFORC<<16)+                 \
                         (init_TIM1OC3M<<8)+(init_TIM1OC3D))

/*  reg_TIM1SCR1    TIM1 System Control Register 1 */
#define init_TIM1SCR1 (0x00) /*     (0b00000000) */
/*                                     |  |______ TFFCA */
/*                                     |_________ TIMEN */
#define mmc_enTimer1                (reg_TIM1SCR1.bit.TIMEN = 1)
#define mmc_enTimer1_FastFlagClear  (reg_TIM1SCR1.bit.TFFCA = 1)

/*  reg_TIM1TOV     TIM1 Toggle on Overflow Register */
#define init_TIM1TOV (0x00) /*      (0b00000000) */
/*                                         ||||__ TOV0 */
/*                                         |||___ TOV1 */
/*                                         ||____ TOV2 */
/*                                         |_____ TOV3 */

/*  reg_TIM1CTL1    TIM1 Control Register 1 */
#define init_TIM1CTL1 (0x00) /*     (0b00000000) */
/*                                     ||||||||__ OM0_OL0 */
/*                                     ||||||____ OM1_OL1 */
/*                                     ||||______ OM2_OL2 */
/*                                     ||________ OM3_OL3 */

/*  reg_TIM1CTL2    TIM1 Control Register 2 */
#define init_TIM1CTL2 (0x00) /*     (0b00000000) */
/*                                     ||||||||__ EDG0 */
/*                                     ||||||____ EDG1 */
/*                                     ||||______ EDG2 */
/*                                     ||________ EDG3 */
#define init_TIM1TOVCTL ((init_TIM1TOV<<24)+(init_TIM1CTL1<<16)+(init_TIM1CTL2))

/*  reg_TIM1IE      TIM1 Interrupt Enable Register */
#define init_TIM1IE (0x00) /*       (0b00000000) */
/*                                         ||||__ C0I */
/*                                         |||___ C1I */
/*                                         ||____ C2I */
/*                                         |_____ C3I */

/*  reg_TIM1SCR2    TIM1 System Control Register 2 */
#define init_TIM1SCR2 (0x00) /*     (0b00000000) */
/*                                     | ||||||__ PR */
/*                                     | |||_____ TCRE */
/*                                     | ||______ RDPT */
/*                                     | |_______ PUPT */
/*                                     |_________ TOI */

/*  reg_TIM1FLG1    TIM1 Flag Register 1 */
#define init_TIM1FLG1 (0x00) /*     (0b00000000) */
/*                                         ||||__ C0F */
/*                                         |||___ C1F */
/*                                         ||____ C2F */
/*                                         |_____ C3F */
#define mmc_testTIM1_Channel3Active (reg_TIM1FLG1.bit.C3F == 1)
#define mmc_testTIM1_Channel2Active (reg_TIM1FLG1.bit.C2F == 1)
#define mmc_testTIM1_Channel1Active (reg_TIM1FLG1.bit.C1F == 1)
#define mmc_testTIM1_Channel0Active (reg_TIM1FLG1.bit.C0F == 1)

/*  reg_TIM1FLG2    TIM1 Flag Register 2 */
#define init_TIM1FLG2 (0x00) /*     (0b00000000) */
/*                                     |________ TOF */
#define mmc_testTIM1_TimerOverflow  (reg_TIM1FLG2.bit.TOF == 1)

/*  reg_TIM1C0    TIM1 Channel 0 Register */
#define init_TIM1C0 (0x0000) 

/*  reg_TIM1C1    TIM1 Channel 1 Register */
#define init_TIM1C1 (0x0000) 

/*  reg_TIM1C2    TIM1 Channel 2 Register */
#define init_TIM1C2 (0x0000) 

/*  reg_TIM1C3    TIM1 Channel 3 Register */
#define init_TIM1C3 (0x0000) 

/*  reg_TIM1PACTL   TIM1 Pulse Accumulator Control Register */
#define init_TIM1PACTL (0x00) /*    (0b00000000) */
/*                                      |||||||__ PAI */
/*                                      ||||||___ PAOVI */
/*                                      |||||____ CLK */
/*                                      |||______ PEDGE */
/*                                      ||_______ PAMOD */
/*                                      |________ PAE */

/*  reg_TIM1PAFLG   TIM1 Pulse Accumulator Flag Register */
#define init_TIM1PAFLG (0x00) /*    (0b00000000) */
/*                                           ||__ PAIF */
/*                                            |__ PAOVF */
#define mmc_testTIM1_PulseAccOverflow   (reg_TIM1PAFLG.bit.PAOVF == 1)
#define mmc_testTIM1_PulseAccInput      (reg_TIM1PAFLG.bit.PAIF == 1)

/*  reg_TIM1PACNT   TIM1 Pulse Accumulator Counter Register */
#define init_TIM1PACNT (0x0000)

/*  reg_TIM1PORT    TIM1 Port Data Register */
#define init_TIM1PORT (0x00) /*     (0b00000000) */
/*                                         ||||__ PORTT0 */
/*                                         |||___ PORTT1 */
/*                                         ||____ PORTT2 */
/*                                         |_____ PORTT3 */

/*  reg_TIM1DDR     TIM1 Port Data Direction Register */
#define init_TIM1DDR (0x03) /*      (0b00000000) */
/*                                         ||||__ DDRT0 */
/*                                         |||___ DDRT1 */
/*                                         ||____ DDRT2 */
/*                                         |_____ DDRT3 */

/*  Timer 2 */
/*  reg_TIM2IOS     TIM2 Input Capture / Output Compare Select Register */
#define init_TIM2IOS (0x00) /*      (0b00000000) */
/*                                         ||||__ IOS0 */
/*                                         |||___ IOS1 */
/*                                         ||____ IOS2 */
/*                                         |_____ IOS3 */

/*  reg_TIM2CFORC   TIM2 Compare Force Register */
#define init_TIM2CFORC (0x00) /*    (0b00000000) */
/*                                         ||||__ FOC0 */
/*                                         |||___ FOC1 */
/*                                         ||____ FOC2 */
/*                                         |_____ FOC3 */

/*  reg_TIM2OC3M    TIM2 Output Compare 3 Mask Register */
#define init_TIM2OC3M (0x00) /*     (0b00000000) */
/*                                         ||||__ OC3M0 */
/*                                         |||___ OC3M1 */
/*                                         ||____ OC3M2 */
/*                                         |_____ OC3M3 */

/*  reg_TIM2OC3D    TIM2 Output Compare 3 Data Register */
#define init_TIM2OC3D (0x00) /*     (0b00000000) */
/*                                         ||||__ OC3D0 */
/*                                         |||___ OC3D1 */
/*                                         ||____ OC3D2 */
/*                                         |_____ OC3D3 */

#define init_TIM2FOC ((init_TIM2IOS<<24)+(init_TIM2CFORC<<16)+                 \
                         (init_TIM2OC3M<<8)+(init_TIM2OC3D))

/*  reg_TIM2SCR1    TIM2 System Control Register 1 */
#define init_TIM2SCR1 (0x00) /*     (0b00000000) */
/*                                     |  |______ TFFCA */
/*                                     |_________ TIMEN */

#define mmc_enTimer2                (reg_TIM2SCR1.bit.TIMEN = 1)
#define mmc_enTimer2_FastFlagClear  (reg_TIM2SCR1.bit.TFFCA = 1)

/*  reg_TIM2TOV     TIM2 Toggle on Overflow Register */
#define init_TIM2TOV (0x00) /*      (0b00000000) */
/*                                         ||||__ TOV0 */
/*                                         |||___ TOV1 */
/*                                         ||____ TOV2 */
/*                                         |_____ TOV3 */

/*  reg_TIM2CTL1    TIM2 Control Register 1 */
#define init_TIM2CTL1 (0x00) /*     (0b00000000) */
/*                                     ||||||||__ OM0_OL0 */
/*                                     ||||||____ OM1_OL1 */
/*                                     ||||______ OM2_OL2 */
/*                                     ||________ OM3_OL3 */

/*  reg_TIM2CTL2    TIM2 Control Register 2 */
#define init_TIM2CTL2 (0x00) /*     (0b00000000) */
/*                                     ||||||||__ EDG0 */
/*                                     ||||||____ EDG1 */
/*                                     ||||______ EDG2 */
/*                                     ||________ EDG3 */

#define init_TIM2TOVCTL ((init_TIM2TOV<<24)+(init_TIM2CTL1<<16)+(init_TIM2CTL2))
/*  reg_TIM2IE      TIM2 Interrupt Enable Register */
#define init_TIM2IE (0x00) /*       (0b00000000) */
/*                                         ||||__ C0I */
/*                                         |||___ C1I */
/*                                         ||____ C2I */
/*                                         |_____ C3I */

/*  reg_TIM2SCR2    TIM2 System Control Register 2 */
#define init_TIM2SCR2 (0x00) /*     (0b00000000) */
/*                                     | ||||||__ PR */
/*                                     | |||_____ TCRE */
/*                                     | ||______ RDPT */
/*                                     | |_______ PUPT */
/*                                     |_________ TOI */

/*  reg_TIM2FLG1    TIM2 Flag Register 1 */
#define init_TIM2FLG1 (0x00) /*     (0b00000000) */
/*                                         ||||__ C0F */
/*                                         |||___ C1F */
/*                                         ||____ C2F */
/*                                         |_____ C3F */
#define mmc_testTIM2_Channel3Active (reg_TIM2FLG1.bit.C3F == 1)
#define mmc_testTIM2_Channel2Active (reg_TIM2FLG1.bit.C2F == 1)
#define mmc_testTIM2_Channel1Active (reg_TIM2FLG1.bit.C1F == 1)
#define mmc_testTIM2_Channel0Active (reg_TIM2FLG1.bit.C0F == 1)

/*  reg_TIM2FLG2    TIM2 Flag Register 2 */
#define init_TIM2FLG2 (0x00) /*     (0b00000000) */
/*                                     |________ TOF */
#define mmc_testTIM2_TimerOverflow  (reg_TIM2FLG2.bit.TOF == 1)

/*  reg_TIM2C0    TIM2 Channel 0 Register */
#define init_TIM2C0 (0x0000) 

/*  reg_TIM2C1    TIM2 Channel 1 Register */
#define init_TIM2C1 (0x0000) 

/*  reg_TIM2C2    TIM2 Channel 2 Register */
#define init_TIM2C2 (0x0000) 

/*  reg_TIM2C3    TIM2 Channel 3 Register */
#define init_TIM2C3 (0x0000) 

/*  reg_TIM2PACTL   TIM2 Pulse Accumulator Control Register */
#define init_TIM2PACTL (0x00) /*    (0b00000000) */
/*                                      |||||||__ PAI */
/*                                      ||||||___ PAOVI */
/*                                      |||||____ CLK */
/*                                      |||______ PEDGE */
/*                                      ||_______ PAMOD */
/*                                      |________ PAE */

/*  reg_TIM2PAFLG   TIM2 Pulse Accumulator Flag Register */
#define init_TIM2PAFLG (0x00) /*    (0b00000000) */
/*                                           ||__ PAIF */
/*                                            |__ PAOVF */
#define mmc_testTIM2_PulseAccOverflow   (reg_TIM2PAFLG.bit.PAOVF == 1)
#define mmc_testTIM2_PulseAccInput      (reg_TIM2PAFLG.bit.PAIF == 1)

/*  reg_TIM2PACNT   TIM2 Pulse Accumulator Counter Register */
#define init_TIM2PACNT (0x0000)

/*  reg_TIM2PORT    TIM2 Port Data Register */
#define init_TIM2PORT (0x00) /*     (0b00000000) */
/*                                         ||||__ PORTT0 */
/*                                         |||___ PORTT1 */
/*                                         ||____ PORTT2 */
/*                                         |_____ PORTT3 */

/*  reg_TIM2DDR     TIM2 Port Data Direction Register */
#define init_TIM2DDR (0x04) /*      (0b00000000) */
/*                                         ||||__ DDRT0 */
/*                                         |||___ DDRT1 */
/*                                         ||____ DDRT2 */
/*                                         |_____ DDRT3 */

/*  FLASH Memory Control Module  **********************************************/

/*  reg_SGFMCR      SGFM  Configuration Register */
#define init_SGFMCR (0x0000)      /* (0b0000000000000000) */
/*                                       | | |  |||   ||__ BKSEL */
/*                                       | | |  |||_______ KEYACC */
/*                                       | | |  ||________ CCIE  */
/*                                       | | |  |_________ CBEIE */
/*                                       | | |____________ LOCK  */
/*                                       | |______________ EME   */
/*                                       |________________ FRZ   */

/*  reg_SGFMCLCKD   SGFM  Clock Divider Register */
#define init_SGFMCLKD (0x0000) /*   (0b0000000000000000) */
/*                                     ||||||||__________ DIV   */
/*                                     ||________________ PRDIV */
/*                                     |_________________ DIVLD */
#define mmc_testSGFM_DividerLoaded (reg_SGFMCLKD.bit.DIVLD == 1)

/*  reg_SGFMSEC     SGFM Security Register (Read Only) */
#define mmc_testSGFM_BackDoorEnabled      (reg_SGFMSEC.bit.KEYEN == 1)
#define mmc_testSGFM_FlashSecurityEnabled (reg_SGFMSEC.bit.SECSTAT == 1)
#define mmc_SGFM_SecurityField            (reg_SGFMSEC.bit.SEC)

/*  reg_SGFMPROT    SGFM Protection Register */
#define mmc_SGFM_Protection    (reg_SGFMPROT.bit.PROT)

/*  reg_SGFMSACC    SGFM Supervisor Access Register */
#define mmc_SGFM_SupervisorAccess    (reg_SGFMSACC.bit.SUPV)

/*  reg_SGFMDACC    SGFM Data Access Register */
#define mmc_SGFM_DataAccess    (reg_SGFMDACC.bit.DATA)

/*  reg_SGFMUSTAT   SGFM User Status Register */
#define init_SGFMUSTAT (0xC000) /*  (0b1100000000000000) */
/*                                     |||| |____________ BLANK   */
/*                                     ||||______________ ACCERR  */
/*                                     |||_______________ PVIOL   */
/*                                     ||________________ CCIF    */
/*                                     |_________________ CBEIF   */
#define mmc_testSGFM_CommandBufferEmpty  (reg_SGFMUSTAT.bit.CBEIF == 1)
#define mmc_testSGFM_CommandComplete     (reg_SGFMUSTAT.bit.CCIF == 1)
#define mmc_testSGFM_ProtectionViolation (reg_SGFMUSTAT.bit.PVIOL == 1)
#define mmc_testSGFM_AccessError         (reg_SGFMUSTAT.bit.ACCERR == 1)
#define mmc_testSGFM_EraseVerified       (reg_SGFMUSTAT.bit.BLANK == 1)

/*  reg_SGFMCMD     SGFM Command Register */
#define mmc_SGFM_Command    (reg_SGFMCMD.bit.CMD)

#endif // _MMC2114_INITVALS_H_
#endif
