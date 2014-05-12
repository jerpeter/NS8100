/*******************************************************************************
*  Nomis Seismograph, Inc.
*  Copyright 2002-2005, All Rights Reserved 
*
*  $RCSfile: SGFMConfig.s,v $
*
*  $Author: lking $
*  $Date: 2011/07/30 17:30:08 $
*  $Source: /Nomis_NS8100/ns7100_Port/source/SGFMConfig.s,v $
*
*  $Revision: 1.1 $
*******************************************************************************/

/*******************************************************************************
*   Description:  Sets protection in the SGFM's Supervisor Access              *
*                 Register, the Data Access Register, and the Protection       *
*                 Register for both blocks of the MMC2114's FLASH.  This file  *
*                 also allows entry of the FLASH security word at 0x0000_0228  *
*                 and a back door access key for security override.            *  
*                 Initially provided as a skeleton, the project designer is    *
*                 expected to edit this file appropriately.                    *
*******************************************************************************/

/*******************************************************************************
*   SGFM Configuration Field                                                   *
*   The SGFM configuration field for the MMC2114 is located at 0x0000_0200 -   *
*   0x0000_022B.  The following code provides placeholders for the user to set *
*   protection for each of the two blocks of the 2114's FLASH.  There are also *
*   placeholders for other FLASH block protection in accordance with the       *
*   SGFM's configuration field in the FLASH.  Protection for blocks 2 and 3    *
*   are not used in the 2114 and can be left unprotected.  At the end of       *
*   this code is a 4-byte location for the FLASH security word.  This          *
*   security word specifies whether the back door to FLASH is enabled and      *
*   whether FLASH security is enabled.  Security is enabled by programming the *
*   lower half-word of this 4-byte field to $000B. All other values will       *
*   result in FLASH security being disabled.  If the back door (security       *
*   bypass) is enabled in the FLASH security word, then the 8-byte key         *
*   programmed at 0x0000_0200-0x0000_0207 must be written to this address (in  *
*   two consecutive word writes) for security to be bypassed.                  *
*******************************************************************************/
  .rodata
  .export    __SGFM_configuration_field
  .align    4                  /* align on 2-byte range */
__SGFM_configuration_field:
  .long    0xffffffff           /* 0x0000_0200 Backdoor Word #1 */
  .long    0xffffffff           /* 0x0000_0204 Backdoor Word #2 */
  .short   0xffff               /* 0x0000_0208 Block 0  Program/Erase Space Restrictions */
  .short   0xffff               /* 0x0000_020A Reserved */
  .short   0xffff               /* 0x0000_020C Block 0  Supervisor/User Space Restrictions */
  .short   0x0000               /* 0x0000_020E Block 0  Program/Data Space Restrictions, Sector ALL defined as program and data access */
  .short   0xffff               /* 0x0000_0210 Block 1  Program/Erase Space Restrictions */
  .short   0xffff               /* 0x0000_0212 Reserved */
  .short   0xffff               /* 0x0000_0214 Block 1  Supervisor/User Space Restrictions */
  .short   0x0000               /* 0x0000_0216 Block 1  Program/Data Space Restrictions  was 0xffff DRB*/ 
  .short   0xffff               /* 0x0000_0218 Block 2  Program/Erase Space Restrictions */
  .short   0xffff               /* 0x0000_021A Reserved */
  .short   0xffff               /* 0x0000_021C Block 2  Supervisor/User Space Restrictions */
  .short   0xffff               /* 0x0000_021E Block 2  Program/Data Space Restrictions */
  .short   0xffff               /* 0x0000_0220 Block 3  Program/Erase Space Restrictions */
  .short   0xffff               /* 0x0000_0222 Reserved */
  .short   0xffff               /* 0x0000_0224 Block 3  Supervisor/User Space Restrictions */
  .short   0xffff               /* 0x0000_0226 Block 3  Program/Data Space Restrictions */
  .long    0xffffffff           /* 0x0000_0228 Security Word */
  .space   0xd4                 /* This should put the next block at 0x300 */
  