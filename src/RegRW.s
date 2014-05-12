/*******************************************************************************
*  Nomis Seismograph, Inc.
*  Copyright 2002-2005, All Rights Reserved
*
*  $RCSfile: RegRW.s,v $
*
*  $Author: lking $
*  $Date: 2011/07/30 17:30:08 $
*  $Source: /Nomis_NS8100/ns7100_Port/source/RegRW.s,v $
*
*  $Revision: 1.1 $
*******************************************************************************/

#if 0

/*******************************************************************************
*   Includes, Exports and External References                                  *
*******************************************************************************/
    .file   "RegRW.s"
    .text
    .export ReadPSR,ReadVBR,ReadSS0,ReadSS1,ReadSS2,ReadSS3
    .export ReadSS4,ReadGCR,ReadGSR
    .export WritePSR,WriteVBR,WriteSS0,WriteSS1,WriteSS2,WriteSS3
    .export WriteSS4,WriteGCR,WriteGSR
    .export WriteR0,InitAltSP,Wait,Doze,Stop
/*******************************************************************************
*   Control register read routines                                             *
*******************************************************************************/
ReadPSR:
    mfcr    r2,cr0
    rts
ReadVBR:
    mfcr    r2,cr1
    rts
ReadSS0:
    mfcr    r2,cr6
    rts
ReadSS1:
    mfcr    r2,cr7
    rts
ReadSS2:
    mfcr    r2,cr8
    rts
ReadSS3:
    mfcr    r2,cr9
    rts
ReadSS4:
    mfcr    r2,cr10
    rts
ReadGCR:
    mfcr    r2,cr11
    rts
ReadGSR:
    mfcr    r2,cr12
    rts

/*******************************************************************************
*   Control register write routines                                            *
*******************************************************************************/
WritePSR:
    mtcr    r2,cr0
    br      WritePSR1
WritePSR1:
    rts

WriteVBR:
    mtcr    r2,cr1
    br      WriteVBR1
WriteVBR1:
    rts

WriteSS0:
    mtcr    r2,cr6
    br      WriteSS01
WriteSS01:
    rts

WriteSS1:
    mtcr    r2,cr7
    br      WriteSS11
WriteSS11:
    rts

WriteSS2:
    mtcr    r2,cr8
    br      WriteSS21
WriteSS21:
    rts

WriteSS3:
    mtcr    r2,cr9
    br      WriteSS31
WriteSS31:
    rts

WriteSS4:
    mtcr    r2,cr10
    br      WriteSS41
WriteSS41:
    rts

WriteGCR:
    mtcr    r2,cr11
    br      WriteGCR1
WriteGCR1:
    rts

WriteGSR:
    mtcr    r2,cr12
    br      WriteGSR1
WriteGSR1:
    rts


WriteR0:
    mov 		r0,r2
    rts

InitAltSP:
    mtcr    r2,cr10

    mfcr    r2,cr0
    bseti 	r2,1
    mtcr    r2,cr0

		mfcr    r0,cr10

    mfcr    r2,cr0
    bclri		r2,1
    mtcr    r2,cr0
    rts

Wait:
    wait
    rts

Doze:
    doze
    rts

Stop:
    stop
    rts

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
