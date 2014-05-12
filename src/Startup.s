/*******************************************************************************
*  Nomis Seismograph, Inc.
*  Copyright 2002-2005, All Rights Reserved
*
*  $RCSfile: Startup.s,v $
*
*  $Author: lking $
*  $Date: 2011/07/30 17:30:08 $
*  $Source: /Nomis_NS8100/ns7100_Port/source/Startup.s,v $
*
*  $Revision: 1.1 $
*******************************************************************************/

#if 0

/*******************************************************************************
*   Includes, Exports and External References                                  *
*******************************************************************************/
        /* Following are defined by MW Linker. */
  .extern     __bss_begin, __bss_end, __stack_begin, __stack_end
  .extern			__vector_table, __call_static_initializers, __init_user
  .extern     main

				.text
  .export     __start
  .export			__reset
  .export     _program_end
  .export     __reset                 /*   0  0x000  Reset */
  .export     __misaligned_access     /*   1  0x004  Misaligned Access */
  .export     __access_error          /*   2  0x008  Access Error */
  .export     __divide_by_zero        /*   3  0x00C  Divide by Zero */
  .export     __illegal_instruction   /*   4  0x010  Illegal Instruction */
  .export     __privilege_violation   /*   5  0x014  Privilege Violation */
  .export     __trace_exception       /*   6  0x018  Trace Exception */
  .export     __breakpoint_exception  /*   7  0x01C  Breakpoint Exception */
  .export     __unrecoverable_error   /*   8  0x020  Unrecoverable Error */
  .export     __soft_reset            /*   9  0x024  Soft Reset */
  .export     __normal_autovector_interrupt
                                      /*  10  0x028  Normal Autovector Int */
  .export     __fast_autovector_interrupt
                                      /*  11  0x02C  Fast Autov Int (AF used) */
  .export     __hardware_accelerator  /*  12  0x030  Hardware Accelerator */
  .export     __trap0                 /*  16  0x040  Trap #0 */
  .export     __trap1                 /*  17  0x044  Trap #1 */
  .export     __trap2                 /*  18  0x048  Trap #2 */
  .export     __trap3                /*  19  0x04C  Trap #3 */
  .export     __vectored_normal_p0    /*  32  0x080  Priority 0 Normal Int */
  .export     __vectored_normal_p1    /*  33  0x084  Priority 1 Normal Int */
  .export     __vectored_normal_p2    /*  34  0x088  Priority 2 Normal Int */
  .export     __vectored_normal_p3    /*  35  0x08C  Priority 3 Normal Int */
  .export     __vectored_normal_p4    /*  36  0x090  Priority 4 Normal Int */
  .export     __vectored_normal_p5    /*  37  0x094  Priority 5 Normal Int */
  .export     __vectored_normal_p6    /*  38  0x098  Priority 6 Normal Int */
  .export     __vectored_normal_p7    /*  39  0x09C  Priority 7 Normal Int */
  .export     __vectored_normal_p8    /*  40  0x0A0  Priority 8 Normal Int */
  .export     __vectored_normal_p9    /*  41  0x0A4  Priority 9 Normal Int */
  .export     __vectored_normal_p10   /*  42  0x0A8  Priority 10 Normal Int */
  .export     __vectored_normal_p11   /*  43  0x0AC  Priority 11 Normal Int */
  .export     __vectored_normal_p12   /*  44  0x0B0  Priority 12 Normal Int */
  .export     __vectored_normal_p13   /*  45  0x0B4  Priority 13 Normal Int */
  .export     __vectored_normal_p14   /*  46  0x0B8  Priority 14 Normal Int */
  .export     __vectored_normal_p15   /*  47  0x0BC  Priority 15 Normal Int */
  .export     __vectored_normal_p16   /*  48  0x0C0  Priority 16 Normal Int */
  .export     __vectored_normal_p17   /*  49  0x0C4  Priority 17 Normal Int */
  .export     __vectored_normal_p18   /*  50  0x0C8  Priority 18 Normal Int */
  .export     __vectored_normal_p19   /*  51  0x0CC  Priority 19 Normal Int */
  .export     __vectored_normal_p20   /*  52  0x0D0  Priority 20 Normal Int */
  .export     __vectored_normal_p21   /*  53  0x0D4  Priority 21 Normal Int */
  .export     __vectored_normal_p22   /*  54  0x0D8  Priority 22 Normal Int */
  .export     __vectored_normal_p23   /*  55  0x0DC  Priority 23 Normal Int */
  .export     __vectored_normal_p24   /*  56  0x0E0  Priority 24 Normal Int */
  .export     __vectored_normal_p25   /*  57  0x0E4  Priority 25 Normal Int */
  .export     __vectored_normal_p26   /*  58  0x0E8  Priority 26 Normal Int */
  .export     __vectored_normal_p27   /*  59  0x0EC  Priority 27 Normal Int */
  .export     __vectored_normal_p28   /*  60  0x0F0  Priority 28 Normal Int */
  .export     __vectored_normal_p29   /*  61  0x0F4  Priority 29 Normal Int */
  .export     __vectored_normal_p30   /*  62  0x0F8  Priority 30 Normal Int */
  .export     __vectored_normal_p31   /*  63  0x0FC  Priority 31 Normal Int */
  .export     __vectored_fast_p0      /*  64  0x100  Priority 0 Fast Int */
  .export     __vectored_fast_p1      /*  65  0x104  Priority 1 Fast Int */
  .export     __vectored_fast_p2      /*  66  0x108  Priority 2 Fast Int */
  .export     __vectored_fast_p3      /*  67  0x10C  Priority 3 Fast Int */
  .export     __vectored_fast_p4      /*  68  0x110  Priority 4 Fast Int */
  .export     __vectored_fast_p5      /*  69  0x114  Priority 5 Fast Int */
  .export     __vectored_fast_p6      /*  70  0x118  Priority 6 Fast Int */
  .export     __vectored_fast_p7      /*  71  0x11C  Priority 7 Fast Int */
  .export     __vectored_fast_p8      /*  72  0x120  Priority 8 Fast Int */
  .export     __vectored_fast_p9      /*  73  0x124  Priority 9 Fast Int */
  .export     __vectored_fast_p10     /*  74  0x128  Priority 10 Fast Int */
  .export     __vectored_fast_p11     /*  75  0x12C  Priority 11 Fast Int */
  .export     __vectored_fast_p12     /*  76  0x130  Priority 12 Fast Int */
  .export     __vectored_fast_p13     /*  77  0x134  Priority 13 Fast Int */
  .export     __vectored_fast_p14     /*  78  0x138  Priority 14 Fast Int */
  .export     __vectored_fast_p15     /*  79  0x13C  Priority 15 Fast Int */
  .export     __vectored_fast_p16     /*  80  0x140  Priority 16 Fast Int */
  .export     __vectored_fast_p17     /*  81  0x144  Priority 17 Fast Int */
  .export     __vectored_fast_p18     /*  82  0x148  Priority 18 Fast Int */
  .export     __vectored_fast_p19     /*  83  0x14C  Priority 19 Fast Int */
  .export     __vectored_fast_p20     /*  84  0x150  Priority 20 Fast Int */
  .export     __vectored_fast_p21     /*  85  0x154  Priority 21 Fast Int */
  .export     __vectored_fast_p22     /*  86  0x158  Priority 22 Fast Int */
  .export     __vectored_fast_p23     /*  87  0x15C  Priority 23 Fast Int */
  .export     __vectored_fast_p24     /*  88  0x160  Priority 24 Fast Int */
  .export     __vectored_fast_p25     /*  89  0x164  Priority 25 Fast Int */
  .export     __vectored_fast_p26     /*  90  0x168  Priority 26 Fast Int */
  .export     __vectored_fast_p27     /*  91  0x16C  Priority 27 Fast Int */
  .export     __vectored_fast_p28     /*  92  0x170  Priority 28 Fast Int */
  .export     __vectored_fast_p29     /*  93  0x174  Priority 29 Fast Int */
  .export     __vectored_fast_p30     /*  94  0x178  Priority 30 Fast Int */
  .export     __vectored_fast_p31     /*  95  0x17C  Priority 31 Fast Int */


__start:

/*******************************************************************************
*  Default Exception Handlers                                                  *
*******************************************************************************/
/*  Reset Exception Handler  **************************************************/
__reset:

/* Load PSR register */
  PSR_REG_VALUE = 0x80000100
  lrw     r3,PSR_REG_VALUE
  mtcr    r3,psr

/*  Initialize the Vector Base Register (VBR) */
  lrw     r3,__vector_table
  mtcr    r3,vbr

/* From Codewarrior's startup.s */
	lrw    	r7, __stack_end
	lrw    	r10, __stack_begin
  mov    	r0, r10

/*  Call our chip select setup function */
  jbsr    board_chip_select_setup

	lrw     r5, __bss_begin
	lrw     r6, __bss_end
	cmphs   r5, r6

	bt      _end_zero_bss
	xor     r4, r4

_zero_bss:
	subi	  r6, 1
	st.b    r4, (r6,0)
	cmplt   r5, r6
	bt		_zero_bss

_end_zero_bss:
	jbsr		startup_copy_rom_section
/*	jbsr		__init_user  */

	jbsr		main
	jmpi	  _ExitProcess

_ExitProcess:
_break:
	bkpt
	bkpt
	br _break


/*******************************************************************************
*  The following exception handlers initially just sit and loop to serve as    *
*  traps.  Specific action, or calls to the real external ISR (your C code)    *
*  can be coded here as appropriate.                                           *
*******************************************************************************/

/*  Misaligned Access Exception Handler  **************************************/
__misaligned_access:
        br      __misaligned_access

/*  Access Error Exception Handler  *******************************************/
__access_error:
        br      __access_error

/*  Divide by Zero Exception Handler  *****************************************/
__divide_by_zero:
        br      __divide_by_zero

/*  Illegal Instruction Exception Handler  ************************************/
__illegal_instruction:
        br      __illegal_instruction

/*  Privilege Violation Exception Handler  ************************************/
__privilege_violation:
        br      __privilege_violation

/*  Trace Exception Handler  **************************************************/
__trace_exception:
        br      __trace_exception

/*  Breakpoint Exception Handler  *********************************************/
__breakpoint_exception:
        br      __breakpoint_exception

/*  Unrecoverable Error Exception Handler  ************************************/
__unrecoverable_error:
        br      __unrecoverable_error

/*  Soft Reset Exception Handler  *********************************************/
__soft_reset:
        br      __soft_reset

/*  Normal Interrupt Autovector Exception Handler  ****************************/
__normal_autovector_interrupt:
        br      __normal_autovector_interrupt  /* jump into ISR */

/*  Fast Interrupt Autovector Exception Handler  ******************************/
/*  The Fast Interrupt Autovector has bit 0 set, thereby enabling the         */
/*  alternate register file.                                                  */
__fast_autovector_interrupt:
        br      __fast_autovector_interrupt

/*  Hardware Accelerator Exception Handler  ***********************************/
__hardware_accelerator:
        br      __hardware_accelerator

/*  Trap #0 Exception Handler  ************************************************/
/*  The System Initialization routine is run as the Trap #0 exception         */
/*  handler in order to force execution in supervisor mode.                   */
__trap0:
      br        __trap0

/*  Trap #1 Exception Handler  ************************************************/
__trap1:
        br      __trap1

/*  Trap #2 Exception Handler  ************************************************/
__trap2:
        br      __trap2

/*  Trap #3 Exception Handler  ************************************************/
__trap3:
        br      __trap3

/*  Vectored (Fast and Normal) Interrupt Exception Handler ********************/
__vectored_normal_p0:
        br      __vectored_normal_p0
__vectored_normal_p1:
        br      __vectored_normal_p1
__vectored_normal_p2:
        br      __vectored_normal_p2
__vectored_normal_p3:
        br      __vectored_normal_p3
__vectored_normal_p4:
        br      __vectored_normal_p4
__vectored_normal_p5:
        br      __vectored_normal_p5
__vectored_normal_p6:
        br      __vectored_normal_p6
__vectored_normal_p7:
        br      __vectored_normal_p7
__vectored_normal_p8:
        br      __vectored_normal_p8
__vectored_normal_p9:
        br      __vectored_normal_p9
__vectored_normal_p10:
        br      __vectored_normal_p10
__vectored_normal_p11:
        br      __vectored_normal_p11
__vectored_normal_p12
        br      __vectored_normal_p12
__vectored_normal_p13:
        br      __vectored_normal_p13
__vectored_normal_p14:
        br      __vectored_normal_p14
__vectored_normal_p15:
        br      __vectored_normal_p15
__vectored_normal_p16:
        br      __vectored_normal_p16
__vectored_normal_p17:
        br      __vectored_normal_p17
__vectored_normal_p18:
        br      __vectored_normal_p18
__vectored_normal_p19:
        br      __vectored_normal_p19
__vectored_normal_p20:
        br      __vectored_normal_p20
__vectored_normal_p21:
        br      __vectored_normal_p21
__vectored_normal_p22:
        br      __vectored_normal_p22
__vectored_normal_p23:
        br      __vectored_normal_p23
__vectored_normal_p24:
        br      __vectored_normal_p24
__vectored_normal_p25:
        br      __vectored_normal_p25
__vectored_normal_p26:
        br      __vectored_normal_p26
__vectored_normal_p27:
        br      __vectored_normal_p27
__vectored_normal_p28:
        br      __vectored_normal_p28
__vectored_normal_p29:
        br      __vectored_normal_p29
__vectored_normal_p30:
        br      __vectored_normal_p30
__vectored_normal_p31:
        br      __vectored_normal_p31
__vectored_fast_p0:
        br      __vectored_fast_p0
__vectored_fast_p1:
        br      __vectored_fast_p1
__vectored_fast_p2:
        br      __vectored_fast_p2
__vectored_fast_p3:
        br      __vectored_fast_p3
__vectored_fast_p4:
        br      __vectored_fast_p4
__vectored_fast_p5:
        br      __vectored_fast_p5
__vectored_fast_p6:
        br      __vectored_fast_p6
__vectored_fast_p7:
        br      __vectored_fast_p7
__vectored_fast_p8:
        br      __vectored_fast_p8
__vectored_fast_p9:
        br      __vectored_fast_p9
__vectored_fast_p10:
        br      __vectored_fast_p10
__vectored_fast_p11:
        br      __vectored_fast_p11
__vectored_fast_p12:
        br      __vectored_fast_p12
__vectored_fast_p13:
        br      __vectored_fast_p13
__vectored_fast_p14:
        br      __vectored_fast_p14
__vectored_fast_p15:
        br      __vectored_fast_p15
__vectored_fast_p16:
        br      __vectored_fast_p16
__vectored_fast_p17:
        br      __vectored_fast_p17
__vectored_fast_p18:
        br      __vectored_fast_p18
__vectored_fast_p19:
        br      __vectored_fast_p19
__vectored_fast_p20:
        br      __vectored_fast_p20
__vectored_fast_p21:
        br      __vectored_fast_p21
__vectored_fast_p22:
        br      __vectored_fast_p22
__vectored_fast_p23:
        br      __vectored_fast_p23
__vectored_fast_p24:
        br      __vectored_fast_p24
__vectored_fast_p25:
        br      __vectored_fast_p25
__vectored_fast_p26:
        br      __vectored_fast_p26
__vectored_fast_p27:
        br      __vectored_fast_p27
__vectored_fast_p28:
        br      __vectored_fast_p28
__vectored_fast_p29:
        br      __vectored_fast_p29
__vectored_fast_p30:
        br      __vectored_fast_p30
__vectored_fast_p31:
        br      __vectored_fast_p31

_end_start:
  			.literals

#endif

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
