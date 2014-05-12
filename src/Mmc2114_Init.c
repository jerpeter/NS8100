///----------------------------------------------------------------------------
///	Nomis Seismograph, Inc.
///	Copyright 2002-2007, All Rights Reserved 
///
///	$RCSfile: Mmc2114_Init.c,v $
///	$Author: jgetz $
///	$Date: 2012/04/26 01:09:53 $
///
///	$Source: /Nomis_NS8100/ns7100_Port/src/Mmc2114_Init.c,v $
///	$Revision: 1.2 $
///----------------------------------------------------------------------------

#if 0
///----------------------------------------------------------------------------
///	Includes
///----------------------------------------------------------------------------
#include "MMC2114.h"           /* target microcontroller register definitions */
#include "MMC2114_initvals.h"  /* project hardware translations */
#include "Common.h"

///----------------------------------------------------------------------------
///	Defines
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Externs
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
///	Globals
///----------------------------------------------------------------------------
typ_RSR rsr_BOOT_status;

///----------------------------------------------------------------------------
///	Prototypes
///----------------------------------------------------------------------------
void initProcessor(void);

/*******************************************************************************
* Function:   void initProcessor(void)                                         *
* Purpose:    This subroutine performs the basic power-on reset initialization.*
*******************************************************************************/
void initProcessor(void)
{ 
	/* Reset Status module */
	rsr_BOOT_status.reg = reg_RSR.reg;	/* store the rest status reg */

	/* Initialize Clock Module */
	reg_SYNCR.reg = init_SYNCR;			/* Synthesizer Control */

	/* Initialize Chip Configuration Module */
	reg_CCR.reg = init_CCR;				/* Chip configuration */
	reg_CTR = init_CTR;					/* Chip test */

	/* Initialize Chip Select Module */
	reg_CSCR0.reg = init_CSCR0;			/* Chip Select 0 */
	reg_CSCR1.reg = init_CSCR1;			/* Chip Select 1 */
	reg_CSCR2.reg = init_CSCR2;			/* Chip Select 2 */
	reg_CSCR3.reg = init_CSCR3;			/* Chip Select 3 */

	/* Initialize Watchdog Timer Module */
	mmc_ResetWatchdog;					/* Reset Watchdog */
	reg_WCR.reg = init_WCR;				/* Watchdog control */
	reg_WMR = init_WMR;				    /* Watchdog modulus */

	/* Initialize Ports */
	reg_PCDPAR.reg = init_PCDPAR;      /* Port C/D pin assignment */
	reg_PORT_A_D = init_PORT_A_D;      /* Port A - D output data */
	reg_PEPAR.reg = init_PEPAR;        /* Port E pin assignment */
	reg_PORT_E_H = init_PORT_E_H;      /* Port E - H output data */
	reg_PORTI.reg = init_PORTI;        /* Port I output data */
	reg_DDR_A_D = init_DDR_A_D;        /* Port A - D data direction */
	reg_DDR_E_H = (unsigned int)init_DDR_E_H;        /* Port E - H data direction */
	reg_DDRI.reg = init_DDRI;          /* Port I data direction */

	/* Initialize Interrupt Controller Module */
	reg_ICR.reg = init_ICR;            /* Interrupt Control */
	reg_PLSR_0_3 = init_PLSR_0_3;      /* Priority Level Selects  0 -  3 */
	reg_PLSR_4_7 = init_PLSR_4_7;      /* Priority Level Selects  4 -  7 */
	reg_PLSR_8_11 = init_PLSR_8_11;    /* Priority Level Selects  8 - 11 */
	reg_PLSR_12_15 = init_PLSR_12_15;  /* Priority Level Selects 12 - 15 */
	reg_PLSR_16_19 = init_PLSR_16_19;  /* Priority Level Selects 16 - 19 */
	reg_PLSR_20_23 = init_PLSR_20_23;  /* Priority Level Selects 20 - 23 */
	reg_PLSR_24_27 = init_PLSR_24_27;  /* Priority Level Selects 24 - 27 */
	reg_PLSR_28_31 = init_PLSR_28_31;  /* Priority Level Selects 28 - 31 */
	reg_PLSR_32_35 = init_PLSR_32_35;  /* Priority Level Selects 32 - 35 */
	reg_PLSR_36_39 = init_PLSR_36_39;  /* Priority Level Selects 36 - 39 */

	/* Initialize Edge Port Module */
	reg_EPPAR.reg = init_EPPAR;        /* EPORT pin assignment */
	reg_EPDR.reg = init_EPDR;          /* EPORT port data */
	reg_EPDDR.reg = init_EPDDR;        /* EPORT data direction */
	reg_EPIER.reg = init_EPIER;        /* EPORT interrupt enable */

	/* Initialize Programmable Interrupt Timer Modules */
	reg_PCSR1.reg = init_PCSR1;        /* PIT1 control and status */
	reg_PMR1 = init_PMR1;              /* PIT1 modulus */
	reg_PCSR2.reg = init_PCSR2;        /* PIT2 control and status */
	reg_PMR2 = init_PMR2;              /* PIT2 modulus */

	/* Initialize Serial Peripheral Interface Module */
	reg_SPIPURD.reg = init_SPIPURD;    /* SPI pullup and reduced drive */
	reg_SPIPORT.reg = init_SPIPORT;    /* SPI Port data */
	reg_SPIDDR.reg = init_SPIDDR;      /* SPI Port data direction */
	reg_SPIBR.reg = init_SPIBR;        /* SPI baud rate */
	reg_SPICR1.reg = init_SPICR1;      /* SPI control register 1 */
	reg_SPICR2.reg = init_SPICR2;      /* SPI control register 2 */

	/* Initialize Serial Communications Interface Modules */
	reg_SCI1PURD.reg = init_SCI1PURD;  /* SCI1 pullup and reduced drive */
	reg_SCI1PORT.reg = init_SCI1PORT;  /* SCI1 Port data */
	reg_SCI1DDR.reg = init_SCI1DDR;    /* SCI1 Port data direction */
	reg_SCI1BD = init_SCI1BD;          /* SCI1 baud rate */
	reg_SCI1CR1.reg = init_SCI1CR1;    /* SCI1 control register 1 */
	reg_SCI1CR2.reg = init_SCI1CR2;    /* SCI1 control register 2 */
	reg_SCI2PURD.reg = init_SCI2PURD;  /* SCI2 pullup and reduced drive */
	reg_SCI2PORT.reg = init_SCI2PORT;  /* SCI2 Port data */
	reg_SCI2DDR.reg = init_SCI2DDR;    /* SCI2 Port data direction */
	reg_SCI2BD = init_SCI2BD;          /* SCI2 baud rate */
	reg_SCI2CR1.reg = init_SCI2CR1;    /* SCI2 control register 1 */
	reg_SCI2CR2.reg = init_SCI2CR2;    /* SCI2 control register 2 */

	/* Initialize Timer Modules */
	reg_TIM1PORT.reg = init_TIM1PORT;  /* TIM1 Port data */
	reg_TIM1DDR.reg = init_TIM1DDR;    /* TIM1 Port data direction */
	reg_TIM1SCR2.reg = init_TIM1SCR2;  /* TIM1 system control register 2 */
	reg_TIM1SCR1.reg = init_TIM1SCR1;  /* TIM1 system control register 1 */
	reg_TIM1IOS.reg = init_TIM1IOS;    /* TIM1 IC/OC select */
	reg_TIM1OC3M.reg = init_TIM1OC3M;  /* TIM1 OC 3 mask register */
	reg_TIM1OC3D.reg = init_TIM1OC3D;  /* TIM1 OC 3 data register */
	reg_TIM1CTL1.reg = init_TIM1CTL1;  /* TIM1 control register 1 */
	reg_TIM1CTL2.reg = init_TIM1CTL2;  /* TIM1 control register 2 */
	reg_TIM1TOV.reg = init_TIM1TOV;    /* TIM1 toggle on overflow and control */
	reg_TIM1CFORC.reg = init_TIM1CFORC;/* TIM1 force / output compare 3 */
	reg_TIM1PACTL.reg = init_TIM1PACTL;/* TIM1 pulse accumulator control */
	reg_TIM1C0 = init_TIM1C0;          /* TIM1 channel 0 register */
	reg_TIM1C1 = init_TIM1C1;          /* TIM1 channel 1 register */
	reg_TIM1C2 = init_TIM1C2;          /* TIM1 channel 2 register */
	reg_TIM1C3 = init_TIM1C3;          /* TIM1 channel 3 register */ 
	reg_TIM1PACNT = init_TIM1PACNT;    /* TIM1 PA count register */
	reg_TIM1FLG1.reg = init_TIM1FLG1;  /* TIM1 flag register 1 */
	reg_TIM1FLG2.reg = init_TIM1FLG2;  /* TIM1 flag register 2 */
	reg_TIM1PAFLG.reg = init_TIM1PAFLG;/* TIM1 PA flag register */
	reg_TIM1IE.reg = init_TIM1IE;      /* TIM1 interrupt enable */

	reg_TIM2PORT.reg = init_TIM2PORT;  /* TIM2 Port data */
	reg_TIM2DDR.reg = init_TIM2DDR;    /* TIM2 Port data direction */
	reg_TIM2SCR2.reg = init_TIM2SCR2;  /* TIM2 system control register 2 */
	reg_TIM2SCR1.reg = init_TIM2SCR1;  /* TIM2 system control register 1 */
	reg_TIM2IOS.reg = init_TIM2IOS;    /* TIM2 IC/OC select */
	reg_TIM2OC3M.reg = init_TIM2OC3M;  /* TIM2 OC 3 mask register */
	reg_TIM2OC3D.reg = init_TIM2OC3D;  /* TIM2 OC 3 data register */
	reg_TIM2CTL1.reg = init_TIM2CTL1;  /* TIM2 control register 1 */
	reg_TIM2CTL2.reg = init_TIM2CTL2;  /* TIM2 control register 2 */
	reg_TIM2TOV.reg = init_TIM2TOV;    /* TIM2 toggle on overflow and control */
	reg_TIM2CFORC.reg = init_TIM2CFORC;/* TIM2 force / output compare 3 */
	reg_TIM2PACTL.reg = init_TIM2PACTL;/* TIM2 pulse accumulator control */
	reg_TIM2C0 = init_TIM2C0;          /* TIM2 channel 0 register */
	reg_TIM2C1 = init_TIM2C1;          /* TIM2 channel 1 register */
	reg_TIM2C2 = init_TIM2C2;          /* TIM2 channel 2 register */
	reg_TIM2C3 = init_TIM2C3;          /* TIM2 channel 3 register */ 
	reg_TIM2PACNT = init_TIM2PACNT;    /* TIM2 PA count register */
	reg_TIM2FLG1.reg = init_TIM2FLG1;  /* TIM2 flag register 1 */
	reg_TIM2FLG2.reg = init_TIM2FLG2;  /* TIM2 flag register 2 */
	reg_TIM2PAFLG.reg = init_TIM2PAFLG;/* TIM2 PA flag register */
	reg_TIM2IE.reg = init_TIM2IE;      /* TIM2 interrupt enable */

	/* Initialize Queued Analog-to-Digital Converter Module */
	reg_CCW0.reg = init_CCW0;          /* QADC Conversion Command Word */
	reg_CCW1.reg = init_CCW1;          /* QADC Conversion Command Word */
	reg_CCW2.reg = init_CCW2;          /* QADC Conversion Command Word */
	reg_CCW3.reg = init_CCW3;          /* QADC Conversion Command Word */
	reg_CCW4.reg = init_CCW4;          /* QADC Conversion Command Word */
	reg_CCW5.reg = init_CCW5;          /* QADC Conversion Command Word */
	reg_CCW6.reg = init_CCW6;          /* QADC Conversion Command Word */
	reg_CCW7.reg = init_CCW7;          /* QADC Conversion Command Word */
	reg_CCW8.reg = init_CCW8;          /* QADC Conversion Command Word */
	reg_CCW9.reg = init_CCW9;          /* QADC Conversion Command Word */
	reg_CCW10.reg = init_CCW10;        /* QADC Conversion Command Word */
	reg_CCW11.reg = init_CCW11;        /* QADC Conversion Command Word */
	reg_CCW12.reg = init_CCW12;        /* QADC Conversion Command Word */
	reg_CCW13.reg = init_CCW13;        /* QADC Conversion Command Word */
	reg_CCW14.reg = init_CCW14;        /* QADC Conversion Command Word */
	reg_CCW15.reg = init_CCW15;        /* QADC Conversion Command Word */
	reg_CCW16.reg = init_CCW16;        /* QADC Conversion Command Word */
	reg_CCW17.reg = init_CCW17;        /* QADC Conversion Command Word */
	reg_CCW18.reg = init_CCW18;        /* QADC Conversion Command Word */
	reg_CCW19.reg = init_CCW19;        /* QADC Conversion Command Word */
	reg_CCW20.reg = init_CCW20;        /* QADC Conversion Command Word */
	reg_CCW21.reg = init_CCW21;        /* QADC Conversion Command Word */
	reg_CCW22.reg = init_CCW22;        /* QADC Conversion Command Word */
	reg_CCW23.reg = init_CCW23;        /* QADC Conversion Command Word */
	reg_CCW24.reg = init_CCW24;        /* QADC Conversion Command Word */
	reg_CCW25.reg = init_CCW25;        /* QADC Conversion Command Word */
	reg_CCW26.reg = init_CCW26;        /* QADC Conversion Command Word */
	reg_CCW27.reg = init_CCW27;        /* QADC Conversion Command Word */
	reg_CCW28.reg = init_CCW28;        /* QADC Conversion Command Word */
	reg_CCW29.reg = init_CCW29;        /* QADC Conversion Command Word */
	reg_CCW30.reg = init_CCW30;        /* QADC Conversion Command Word */
	reg_CCW31.reg = init_CCW31;        /* QADC Conversion Command Word */
	reg_CCW32.reg = init_CCW32;        /* QADC Conversion Command Word */
	reg_CCW33.reg = init_CCW33;        /* QADC Conversion Command Word */
	reg_CCW34.reg = init_CCW34;        /* QADC Conversion Command Word */
	reg_CCW35.reg = init_CCW35;        /* QADC Conversion Command Word */
	reg_CCW36.reg = init_CCW36;        /* QADC Conversion Command Word */
	reg_CCW37.reg = init_CCW37;        /* QADC Conversion Command Word */
	reg_CCW38.reg = init_CCW38;        /* QADC Conversion Command Word */
	reg_CCW39.reg = init_CCW39;        /* QADC Conversion Command Word */
	reg_CCW40.reg = init_CCW40;        /* QADC Conversion Command Word */
	reg_CCW41.reg = init_CCW41;        /* QADC Conversion Command Word */
	reg_CCW42.reg = init_CCW42;        /* QADC Conversion Command Word */
	reg_CCW43.reg = init_CCW43;        /* QADC Conversion Command Word */
	reg_CCW44.reg = init_CCW44;        /* QADC Conversion Command Word */
	reg_CCW45.reg = init_CCW45;        /* QADC Conversion Command Word */
	reg_CCW46.reg = init_CCW46;        /* QADC Conversion Command Word */
	reg_CCW47.reg = init_CCW47;        /* QADC Conversion Command Word */
	reg_CCW48.reg = init_CCW48;        /* QADC Conversion Command Word */
	reg_CCW49.reg = init_CCW49;        /* QADC Conversion Command Word */
	reg_CCW50.reg = init_CCW50;        /* QADC Conversion Command Word */
	reg_CCW51.reg = init_CCW51;        /* QADC Conversion Command Word */
	reg_CCW52.reg = init_CCW52;        /* QADC Conversion Command Word */
	reg_CCW53.reg = init_CCW53;        /* QADC Conversion Command Word */
	reg_CCW54.reg = init_CCW54;        /* QADC Conversion Command Word */
	reg_CCW55.reg = init_CCW55;        /* QADC Conversion Command Word */
	reg_CCW56.reg = init_CCW56;        /* QADC Conversion Command Word */
	reg_CCW57.reg = init_CCW57;        /* QADC Conversion Command Word */
	reg_CCW58.reg = init_CCW58;        /* QADC Conversion Command Word */
	reg_CCW59.reg = init_CCW59;        /* QADC Conversion Command Word */
	reg_CCW60.reg = init_CCW60;        /* QADC Conversion Command Word */
	reg_CCW61.reg = init_CCW61;        /* QADC Conversion Command Word */
	reg_CCW62.reg = init_CCW62;        /* QADC Conversion Command Word */
	reg_CCW63.reg = init_CCW63;        /* QADC Conversion Command Word */

	soft_usecWait(1000);

	reg_QADCMCR.reg = init_QADCMCR;    /* QADC module configuration */
	reg_PORTQA.reg = init_PORTQA;      /* QADC Port A data */
	reg_PORTQB.reg = init_PORTQB;      /* QADC Port B data */
	reg_DDRQA.reg = init_DDRQA;        /* QADC Port A data direction */
	reg_DDRQB.reg = init_DDRQB;        /* QADC Port B data direction */
	reg_QACR0.reg = init_QACR0;        /* QADC control register 0 */
	reg_QACR1.reg = init_QACR1;        /* QADC control register 1 */
	reg_QACR2.reg = init_QACR2;        /* QADC control register 2 */

	soft_usecWait(1000);
}

#endif
