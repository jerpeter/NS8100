/*******************************************************************************
*  Nomis Seismograph, Inc.
*  Copyright 2002-2005, All Rights Reserved 
*
*  $RCSfile: Vector.s,v $
*
*  $Author: lking $
*  $Date: 2011/07/30 17:30:09 $
*  $Source: /Nomis_NS8100/ns7100_Port/source/Vector.s,v $
*
*  $Revision: 1.1 $
*******************************************************************************/

/*******************************************************************************
*   Reference ISR Names                                  *
*   Following is a list of ISR names for each source of  *
*   interrupt on the MMC2114.  If the user wishes to use these ISR names in    *
*   his application, then all he needs to do is copy the name and paste it     *
*   into the vector table which follows, at the interrupt slot that he wishes  *
*   to handle the interrupt.  If he wants to use a different name for his ISR, *
*   he can just add the name to this list and plug it into the vector table.   *
*******************************************************************************/
  .extern  isr_reset                /* Reset */
  .extern  isr_misaligned_access    /* Misaligned access */
  .extern  isr_access_error         /* Access Error */
  .extern  isr_divide_by_zero       /* Divide by zero */
  .extern  isr_illegal_instruction  /* Illegal instruction */
  .extern  isr_privilege_violation  /* Privilege violation */
  .extern  isr_trace_exception      /* Trace exception */
  .extern  isr_breakpoint_exception /* Breakpoint exception */
  .extern  isr_unrecoverable_error  /* Unrecoverable error */
  .extern  isr_soft_reset           /* Soft reset */
  .extern  isr_normal_autovector_interrupt  /* INT autovector */
  .extern  isr_fast_autovector_interrupt    /* FINT autovector */
  .extern  isr_hardware_accelerator /* Hardware accelerator */
  .extern  isr_trap0                /* Trap 0-3 instr. vectors */
  .extern  isr_trap1
  .extern  isr_trap2
  .extern  isr_trap3

  .extern  isr_Queue1_Pause         /* Queue 1 Pause ISR*/
  .extern  isr_Queue1_Completion    /* Queue 1 Completion ISR */
  .extern  isr_Queue2_Pause         /* Queue 2 Pause ISR*/
  .extern  isr_Queue2_Completion    /* Queue 2 Completion ISR */

  .extern  isr_SPI_Mode_Fault       /* SPI Mode Fault ISR */
  .extern  isr_SPI_Transfer_Complete /* SPI Transfer Complete ISR */
  
  .extern  isr_SCI1_TDRE            /* SCI1 Transmit Data Register Empty ISR */
  .extern  isr_SCI1_TC              /* SCI1 Transmit Complete ISR */
  .extern  isr_SCI1_RDRF            /* SCI1 Receive Data Register Full ISR */
  .extern  isr_SCI1_OR              /* SCI1 Receiver Overrun ISR */
  .extern  isr_SCI1_IDLE            /* SCI1 Receiver Line Idle ISR */
  
  .extern  isr_SCI2_TDRE            /* SCI2 Transmit Data Register Empty ISR */
  .extern  isr_SCI2_TC              /* SCI2 Transmit Complete ISR */
  .extern  isr_SCI2_RDRF            /* SCI2 Receive Data Register Full ISR */
  .extern  isr_SCI2_OR              /* SCI2 Receiver Overrun ISR */
  .extern  isr_SCI2_IDLE            /* SCI2 Receiver Line Idle ISR */
  
  .extern  isr_TIM1_C0F             /* Timer 1 Channel 0 ISR */
  .extern  isr_TIM1_C1F             /* Timer 1 Channel 1 ISR */
  .extern  isr_TIM1_C2F             /* Timer 1 Channel 2 ISR */
  .extern  isr_TIM1_C3F             /* Timer 1 Channel 3 ISR */
  .extern  isr_TIM1_TOF             /* Timer 1 Overflow ISR */
  .extern  isr_TIM1_PAIF            /* Timer 1 Pulse Accumulator Input ISR */
  .extern  isr_TIM1_PAOVF           /* Timer 1 Pulse Accumulator Overflow ISR */

  .extern  isr_TIM2_C0F             /* Timer 2 Channel 0 ISR */
  .extern  isr_TIM2_C1F             /* Timer 2 Channel 1 ISR */
  .extern  isr_TIM2_C2F             /* Timer 2 Channel 2 ISR */
  .extern  isr_TIM2_C3F             /* Timer 2 Channel 3 ISR */
  .extern  isr_TIM2_TOF             /* Timer 2 Overflow ISR */
  .extern  isr_TIM2_PAIF            /* Timer 2 Pulse Accumulator Input ISR */
  .extern  isr_TIM2_PAOVF           /* Timer 2 Pulse Accumulator Overflow ISR */
  
  .extern  isr_PIT1_PIF_Normal      /* Programmable Interrupt Timer 1 ISR */
  .extern  isr_PIT2_PIF             /* Programmable Interrupt Timer 2 ISR */
   
  .extern  isr_EPF0                 /* Edge Port Flag 0 / LVD ISR */
  .extern  isr_EPF1                 /* Edge Port Flag 1 / SGFM Buffer Empty / SGFM Command Complete ISR */
  .extern  isr_EPF2                 /* Edge Port Flag 2 ISR */
  .extern  isr_EPF3                 /* Edge Port Flag 3 ISR */
  .extern  isr_EPF4                 /* Edge Port Flag 4 ISR */
  .extern  isr_EPF5                 /* Edge Port Flag 5 ISR */
  .extern  isr_EPF6                 /* Edge Port Flag 6 ISR */
  .extern  isr_EPF7                 /* Edge Port Flag 7 ISR */

  .extern  __reset                  /*   0  0x000  Reset */
  .extern  __misaligned_access      /*   1  0x004  Misaligned Access */
  .extern  __access_error           /*   2  0x008  Access Error */
  .extern  __divide_by_zero         /*   3  0x00C  Divide by Zero */
  .extern  __illegal_instruction    /*   4  0x010  Illegal Instruction */
  .extern  __privilege_violation    /*   5  0x014  Privilege Violation */
  .extern  __trace_exception        /*   6  0x018  Trace Exception */
  .extern  __breakpoint_exception   /*   7  0x01C  Breakpoint Exception */
  .extern  __unrecoverable_error    /*   8  0x020  Unrecoverable Error */
  .extern  __soft_reset             /*   9  0x024  Soft Reset */
  .extern  __normal_autovector_interrupt /* 10   0x028  Normal Autovector Int */
  .extern  __fast_autovector_interrupt   /* 11   0x02C  Fast Autovector Int */
  .extern  __hardware_accelerator   /*  12  0x030  Hardware Accelerator */
  .extern  __trap0                  /*  16  0x040  Trap #0 */
  .extern  __trap1                  /*  17  0x044  Trap #1 */
  .extern  __trap2                  /*  18  0x048  Trap #2 */
  .extern  __trap3                  /*  19  0x04C  Trap #3 */
  .extern  isr_Keypad			    /*  32  0x080  Priority 0 Normal Int */
  .extern  isr_SCI2			        /*  33  0x084  Priority 1 Normal Int */
  .extern  __vectored_normal_p2		/*  34  0x088  Priority 2 Normal Int */
  .extern  isr_RTC					/*  35  0x08C  Priority 3 Normal Int */
  .extern  isr_PowerOnKey			/*  36  0x090  Priority 4 Normal Int */
  .extern  isr_PowerOffKey			/*  37  0x094  Priority 5 Normal Int */
  .extern  __vectored_normal_p6     /*  38  0x098  Priority 6 Normal Int */
  .extern  __vectored_normal_p7     /*  39  0x09C  Priority 7 Normal Int */
  .extern  __vectored_normal_p8     /*  40  0x0A0  Priority 8 Normal Int */
  .extern  __vectored_normal_p9     /*  41  0x0A4  Priority 9 Normal Int */
  .extern  __vectored_normal_p10    /*  42  0x0A8  Priority 10 Normal Int */
  .extern  __vectored_normal_p11    /*  43  0x0AC  Priority 11 Normal Int */
  .extern  __vectored_normal_p12    /*  44  0x0B0  Priority 12 Normal Int */
  .extern  __vectored_normal_p13    /*  45  0x0B4  Priority 13 Normal Int */
  .extern  __vectored_normal_p14    /*  46  0x0B8  Priority 14 Normal Int */
  .extern  __vectored_normal_p15    /*  47  0x0BC  Priority 15 Normal Int */
  .extern  __vectored_normal_p16    /*  48  0x0C0  Priority 16 Normal Int */
  .extern  __vectored_normal_p17    /*  49  0x0C4  Priority 17 Normal Int */
  .extern  __vectored_normal_p18    /*  50  0x0C8  Priority 18 Normal Int */
  .extern  __vectored_normal_p19    /*  51  0x0CC  Priority 19 Normal Int */
  .extern  __vectored_normal_p20    /*  52  0x0D0  Priority 20 Normal Int */
  .extern  isr_SCI1			        /*  53  0x0D4  Priority 21 Normal Int */
  .extern  __vectored_normal_p22    /*  54  0x0D8  Priority 22 Normal Int */
  .extern  __vectored_normal_p23    /*  55  0x0DC  Priority 23 Normal Int */
  .extern  __vectored_normal_p24    /*  56  0x0E0  Priority 24 Normal Int */
  .extern  __vectored_normal_p25    /*  57  0x0E4  Priority 25 Normal Int */
  .extern  __vectored_normal_p26    /*  58  0x0E8  Priority 26 Normal Int */
  .extern  __vectored_normal_p27    /*  59  0x0EC  Priority 27 Normal Int */
  .extern  __vectored_normal_p28    /*  60  0x0F0  Priority 28 Normal Int */
  .extern  isr_PIT1				    /*  61  0x0F4  Priority 29 Normal Int */
  .extern  isr_Trig					/*  62  0x0F8  Priority 30 Normal Int */
  .extern  __vectored_normal_p31    /*  63  0x0FC  Priority 31 Normal Int */
  .extern  __vectored_fast_p0       /*  64  0x100  Priority 0 Fast Int */
  .extern  __vectored_fast_p1       /*  65  0x104  Priority 1 Fast Int */
  .extern  __vectored_fast_p2       /*  66  0x108  Priority 2 Fast Int */
  .extern  __vectored_fast_p3       /*  67  0x10C  Priority 3 Fast Int */
  .extern  __vectored_fast_p4       /*  68  0x110  Priority 4 Fast Int */
  .extern  __vectored_fast_p5       /*  69  0x114  Priority 5 Fast Int */
  .extern  __vectored_fast_p6       /*  70  0x118  Priority 6 Fast Int */
  .extern  __vectored_fast_p7       /*  71  0x11C  Priority 7 Fast Int */
  .extern  __vectored_fast_p8       /*  72  0x120  Priority 8 Fast Int */
  .extern  __vectored_fast_p9       /*  73  0x124  Priority 9 Fast Int */
  .extern  __vectored_fast_p10      /*  74  0x128  Priority 10 Fast Int */
  .extern  __vectored_fast_p11      /*  75  0x12C  Priority 11 Fast Int */
  .extern  __vectored_fast_p12      /*  76  0x130  Priority 12 Fast Int */
  .extern  __vectored_fast_p13      /*  77  0x134  Priority 13 Fast Int */
  .extern  __vectored_fast_p14      /*  78  0x138  Priority 14 Fast Int */
  .extern  __vectored_fast_p15      /*  79  0x13C  Priority 15 Fast Int */
  .extern  __vectored_fast_p16      /*  80  0x140  Priority 16 Fast Int */
  .extern  __vectored_fast_p17      /*  81  0x144  Priority 17 Fast Int */
  .extern  __vectored_fast_p18      /*  82  0x148  Priority 18 Fast Int */
  .extern  __vectored_fast_p19      /*  83  0x14C  Priority 19 Fast Int */
  .extern  __vectored_fast_p20      /*  84  0x150  Priority 20 Fast Int */
  .extern  __vectored_fast_p21      /*  85  0x154  Priority 21 Fast Int */
  .extern  __vectored_fast_p22      /*  86  0x158  Priority 22 Fast Int */
  .extern  __vectored_fast_p23      /*  87  0x15C  Priority 23 Fast Int */
  .extern  __vectored_fast_p24      /*  88  0x160  Priority 24 Fast Int */
  .extern  __vectored_fast_p25      /*  89  0x164  Priority 25 Fast Int */
  .extern  __vectored_fast_p26      /*  90  0x168  Priority 26 Fast Int */
  .extern  __vectored_fast_p27      /*  91  0x16C  Priority 27 Fast Int */
  .extern  __vectored_fast_p28      /*  92  0x170  Priority 28 Fast Int */
  .extern  __vectored_fast_p29      /*  93  0x174  Priority 29 Fast Int */
  .extern  __vectored_fast_p30      /*  94  0x178  Priority 30 Fast Int */
  .extern  __vectored_fast_p31      /*  95  0x17C  Priority 31 Fast Int */


/*******************************************************************************
*   Exception Vector Table                                                     *
*   By default, in single chip mode, the MMC2114 will fetch the power-on reset *
*   vector from 0x0000_0000.  For convenience, the entire exception vector     *
*   table is located here and the Vector Base Register (VBR) points here.  If  *
*   the user does not replace the reference address in the vector table or the *
*   exception handler calls at the end of this file with his own ISR reference *
*   from the list above, then the interrupt will automatically have a trap to  *
*   the short routines which follow.  Note that some initialization for reset	 *
*		operation is included in its 'exception' routine below.                    *
*******************************************************************************/
  .rodata
  .export    __vector_table
   .align    4                       /* align on 2-byte range */
__vector_table:
  .long    __reset                  /*   0  0x000  Reset */
#if 1 // Use custom defined exception handlers
  .long    isr_misaligned_access      /*   1  0x004  Misaligned Access */
  .long    isr_access_error           /*   2  0x008  Access Error */
  .long    isr_divide_by_zero         /*   3  0x00C  Divide by Zero */
  .long    isr_illegal_instruction    /*   4  0x010  Illegal Instruction */
  .long    isr_privilege_violation    /*   5  0x014  Privilege Violation */
  .long    isr_trace_exception        /*   6  0x018  Trace Exception */
  .long    isr_breakpoint_exception   /*   7  0x01C  Breakpoint Exception */
  .long    isr_unrecoverable_error    /*   8  0x020  Unrecoverable Error */
#else
  .long    __misaligned_access      /*   1  0x004  Misaligned Access */
  .long    __access_error           /*   2  0x008  Access Error */
  .long    __divide_by_zero         /*   3  0x00C  Divide by Zero */
  .long    __illegal_instruction    /*   4  0x010  Illegal Instruction */
  .long    __privilege_violation    /*   5  0x014  Privilege Violation */
  .long    __trace_exception        /*   6  0x018  Trace Exception */
  .long    __breakpoint_exception   /*   7  0x01C  Breakpoint Exception */
  .long    __unrecoverable_error    /*   8  0x020  Unrecoverable Error */
#endif
  .long    __soft_reset             /*   9  0x024  Soft Reset */
  .long    __normal_autovector_interrupt
                                    /*  10  0x028  Normal Autovector Int */
  .long    __fast_autovector_interrupt+1 
                                    /*  11  0x02C  Fast Autovec Int (AF used) */
  .long    __hardware_accelerator   /*  12  0x030  Hardware Accelerator */
  .space   0x0C                     /*  13-15   Unused exception space */
  .long    __trap0                  /*  16  0x040  Trap #0 */
  .long    __trap1                  /*  17  0x044  Trap #1 */
  .long    __trap2                  /*  18  0x048  Trap #2 */
  .long    __trap3                  /*  19  0x04C  Trap #3 */
  .space   0x30                     /*  20-31   Unused exception space */
  .long    isr_Keypad			     /*  32  0x080  Priority 0 Normal Int */
  .long    isr_SCI2			       	 /*  33  0x084  Priority 1 Normal Int */
  .long    __vectored_normal_p2		 /*  34  0x088  Priority 2 Normal Int */
  .long    isr_RTC					 /*  35  0x08C  Priority 3 Normal Int */
  .long    isr_PowerOnKey			 /*  36  0x090  Priority 4 Normal Int */
  .long    isr_PowerOffKey			 /*  37  0x094  Priority 5 Normal Int */
  .long    isr_Lan			         /*  38  0x098  Priority 6 Normal Int */
  .long    isr_UsbHost		         /*  39  0x09C  Priority 7 Normal Int */
  .long    isr_UsbDevice	         /*  40  0x0A0  Priority 8 Normal Int */
  .long    __vectored_normal_p9      /*  41  0x0A4  Priority 9 Normal Int */
  .long    __vectored_normal_p10     /*  42  0x0A8  Priority 10 Normal Int */
  .long    __vectored_normal_p11     /*  43  0x0AC  Priority 11 Normal Int */
  .long    __vectored_normal_p12     /*  44  0x0B0  Priority 12 Normal Int */
  .long    isr_SPI_Transfer_Complete /*  45  0x0B4  Priority 13 Normal Int */
  .long    __vectored_normal_p14     /*  46  0x0B8  Priority 14 Normal Int */
  .long    __vectored_normal_p15     /*  47  0x0BC  Priority 15 Normal Int */
  .long    __vectored_normal_p16     /*  48  0x0C0  Priority 16 Normal Int */
  .long    __vectored_normal_p17     /*  49  0x0C4  Priority 17 Normal Int */
  .long    __vectored_normal_p18     /*  50  0x0C8  Priority 18 Normal Int */
  .long    __vectored_normal_p19     /*  51  0x0CC  Priority 19 Normal Int */
  .long    __vectored_normal_p20     /*  52  0x0D0  Priority 20 Normal Int */
  .long    isr_SCI1			         /*  53  0x0D4  Priority 21 Normal Int */
  .long    __vectored_normal_p22     /*  54  0x0D8  Priority 22 Normal Int */
  .long    __vectored_normal_p23     /*  55  0x0DC  Priority 23 Normal Int */
  .long    __vectored_normal_p24     /*  56  0x0E0  Priority 24 Normal Int */
  .long    __vectored_normal_p25     /*  57  0x0E4  Priority 25 Normal Int */
  .long    __vectored_normal_p26     /*  58  0x0E8  Priority 26 Normal Int */
  .long    __vectored_normal_p27     /*  59  0x0EC  Priority 27 Normal Int */
  .long    __vectored_normal_p28     /*  60  0x0F0  Priority 28 Normal Int */
  .long    isr_PIT1				     /*  61  0x0F4  Priority 29 Normal Int */
  .long    isr_Trig			         /*  62  0x0F8  Priority 30 Normal Int */
  .long    __vectored_normal_p31     /*  63  0x0FC  Priority 31 Normal Int */
  .long    __vectored_fast_p0       /*  64  0x100  Priority 0 Fast Int */
  .long    __vectored_fast_p1       /*  65  0x104  Priority 1 Fast Int */
  .long    __vectored_fast_p2       /*  66  0x108  Priority 2 Fast Int */
  .long    __vectored_fast_p3       /*  67  0x10C  Priority 3 Fast Int */
  .long    __vectored_fast_p4       /*  68  0x110  Priority 4 Fast Int */
  .long    __vectored_fast_p5       /*  69  0x114  Priority 5 Fast Int */
  .long    __vectored_fast_p6       /*  70  0x118  Priority 6 Fast Int */
  .long    __vectored_fast_p7       /*  71  0x11C  Priority 7 Fast Int */
  .long    __vectored_fast_p8       /*  72  0x120  Priority 8 Fast Int */
  .long    __vectored_fast_p9       /*  73  0x124  Priority 9 Fast Int */
  .long    __vectored_fast_p10      /*  74  0x128  Priority 10 Fast Int */
  .long    __vectored_fast_p11      /*  75  0x12C  Priority 11 Fast Int */
  .long    __vectored_fast_p12      /*  76  0x130  Priority 12 Fast Int */
  .long    __vectored_fast_p13      /*  77  0x134  Priority 13 Fast Int */
  .long    __vectored_fast_p14      /*  78  0x138  Priority 14 Fast Int */
  .long    __vectored_fast_p15      /*  79  0x13C  Priority 15 Fast Int */
  .long    __vectored_fast_p16      /*  80  0x140  Priority 16 Fast Int */
  .long    __vectored_fast_p17      /*  81  0x144  Priority 17 Fast Int */
  .long    __vectored_fast_p18      /*  82  0x148  Priority 18 Fast Int */
  .long    __vectored_fast_p19      /*  83  0x14C  Priority 19 Fast Int */
  .long    __vectored_fast_p20      /*  84  0x150  Priority 20 Fast Int */
  .long    __vectored_fast_p21      /*  85  0x154  Priority 21 Fast Int */
  .long    __vectored_fast_p22      /*  86  0x158  Priority 22 Fast Int */
  .long    __vectored_fast_p23      /*  87  0x15C  Priority 23 Fast Int */
  .long    __vectored_fast_p24      /*  88  0x160  Priority 24 Fast Int */
  .long    __vectored_fast_p25      /*  89  0x164  Priority 25 Fast Int */
  .long    __vectored_fast_p26      /*  90  0x168  Priority 26 Fast Int */
  .long    __vectored_fast_p27      /*  91  0x16C  Priority 27 Fast Int */
  .long    __vectored_fast_p28      /*  92  0x170  Priority 28 Fast Int */
  .long    __vectored_fast_p29      /*  93  0x174  Priority 29 Fast Int */
  .long    __vectored_fast_p30      /*  94  0x178  Priority 30 Fast Int */
  .long    __vectored_fast_p31      /*  95  0x17C  Priority 31 Fast Int */
  .space   0x78                     /*  96-125  Unused exception space */
  .long    0xA55A0FF0               /*  126 0x1F8  Valid Image ID */
  .space   0x04                     /*  127 0x1FC  Lenght word */

/*******************************************************************************
*    Templates                                                                 *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
*******************************************************************************/
                                                                                      
/*******************************************************************************
* Function:                                                                    *
* Purpose:                                                                     *
* ProtoType:                                                                   *
* Usage:                                                                       *
* Input:                                                                       *
* Output:                                                                      *
*******************************************************************************/

/*******************************************************************************
*    Disclaimer                                                                *
********************************************************************************
*   Motorola reserves the right to make changes without further notice to any  *
*   product herein to improve reliability, function, or design.  Motorola does *
*   not assume any liability arising out of the application or use of any      *
*   product, circuit, or software described herein; neither does it convey     *
*   any license under its patent rights nor the rights of others.  Motorola    *
*   products are not designed, intended, or authorized for use as components   *
*   in systems intended for surgical implant into the body, or other           *
*   applications intended to support life, or for any other application in     *
*   which the failure of the Motorola product could create a situation where   *
*   personal injury or death may occur.  Should Buyer purchase or use Motorola *
*   products for any such intended or unauthorized application, Buyer shall    *
*   indemnify and hold Motorola and its officers, employees, subsidiaries,     *
*   affiliates, and distributors harmless against all claims, costs, damages,  *
*   and expenses, and reasonable attorney fees arising out of, directly or     *
*   indirectly, any claim of personal injury or death associated with such     *
*   unintended or unauthorized use, even if such claim alleges that Motorola   *
*   was negligent regarding the design or manufacture of the part.             *
*                                                                              *
*   Motorola and the Motorola logo are registered trademarks of Motorola Ltd.  *
*******************************************************************************/
