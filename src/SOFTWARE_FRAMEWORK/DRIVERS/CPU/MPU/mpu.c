/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3A-1.4.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief MPU driver.
 *
 * MPU (Memory Protection Unit) driver module for AVR32 devices.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *****************************************************************************/

/* Copyright (C) 2006-2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <avr32/io.h>
#include "compiler.h"
#include "mpu.h"


#define MIN_REGION_SIZE_KB   4        // 4kB
#define MAX_REGION_SIZE_KB   4000000  // 4GB


/*!
 * Control of the MPU control register MPUCR : enable MPUCR
 */
void enable_mpu(void)
{
  Set_system_register(AVR32_MPUCR, AVR32_MPUCR_E_MASK);
}


/*!
 * Control of the MPU control register MPUCR : disable MPUCR
 */
void disable_mpu(void)
{
  Set_system_register(AVR32_MPUCR, 0);
}


/*!
 * \brief Converts an input region size expressed in kBytes to the corresponding
 *  eRegionSize type value.
 *
 * \param kBSizeValue : input region size expressed in kBytes
 * \param peRegionSizeValue : output region size in the eRegionSize type
 *
 * \return Bool OK if the conversion succeeded
 *              KO if the conversion failed (the input is not a possible protected region size).
 */
Bool mpu_convert_kbsize_to_eregionsize(eRegionSize *peRegionSizeValue, U32 kBSizeValue)
{
  U32 i;
  eRegionSize RegVal;
  Bool Status = KO;


  if( ( kBSizeValue >= MIN_REGION_SIZE_KB ) && ( kBSizeValue <= MAX_REGION_SIZE_KB ) )
  {
    for( i = MIN_REGION_SIZE_KB, RegVal = MPU_REGION_SIZE_4KB;
      RegVal <= MAX_REGION_SIZE_KB; i *= 2, RegVal++ )
    {
      if(i == kBSizeValue)
      {
        *peRegionSizeValue = RegVal;
        Status = OK;
        break;
      }
    }
  }
  return(Status);
}


/*!
 * Setup a MPU entry
 * \param mpu_entry: pointer to mpu_entry_t with the MPU settings (base address, size, validity).
 * \param region_number: MPU entry region number (0..7).
 * \return int       MPU_SETUP_ENTRY_OK              if the setup succeeded
 *                   MPU_SETUP_ENTRY_INVALIDBASEADDR if the setup failed because the input base address is not aligned on a 4kB boundary.
 *                   MPU_SETUP_ENTRY_INVALIDSIZE     if the setup failed because the input size is not one of the type eRegionSize.
 */
char set_mpu_entry(const mpu_entry_t *mpu_entry, unsigned int region_number)
{
  /* Set Address Register. */
  avr32_mpuar0_t ar;


  // Check the base address : it must be aligned on a 4kB boundary.
  if(mpu_entry->addr & 0x00000FFF)
    // ERROR: the input base address is not aligned on a 4kB boundary.
    return(MPU_SETUP_ENTRY_INVALIDBASEADDR);

  // Check the size : it must (be a power of 2) and (greater or equal to 4kB) and (less or equal to 4GB).
  // Based on Table 5.1 in doc32002 "AVR32UC Technical Reference Manual Complete".
  if( ( mpu_entry->size <= MPU_REGION_SIZE_LOWLIMIT_FORBIDDEN )
    ||( mpu_entry->size >= MPU_REGION_SIZE_HIGHLIMIT_FORBIDDEN ) )
    // ERROR: the input size is not of the type eRegionSize.
    return(MPU_SETUP_ENTRY_INVALIDSIZE);

  // Specify the 20 most significant bits of the start address.
  ar.base = mpu_entry->addr >> 12;
  // Size of this protected region; based on Table 5.1 in doc32000 (AVR32
  // Architecture Manual Complete).
  ar.size = mpu_entry->size;
  ar.v = mpu_entry->valid;

  /* Instruction entry */
  switch ( region_number & 0x7 )
  {
  default:
        case 0: 
          Set_system_register(AVR32_MPUAR0, *((unsigned int *)&ar));
          break;
        case 1: 
          Set_system_register(AVR32_MPUAR1, *((unsigned int *)&ar)); 
          break;
        case 2: 
          Set_system_register(AVR32_MPUAR2, *((unsigned int *)&ar)); 
          break;
        case 3: 
          Set_system_register(AVR32_MPUAR3, *((unsigned int *)&ar)); 
          break;
        case 4: 
          Set_system_register(AVR32_MPUAR4, *((unsigned int *)&ar)); 
          break;
        case 5: 
          Set_system_register(AVR32_MPUAR5, *((unsigned int *)&ar)); 
          break;
        case 6: 
          Set_system_register(AVR32_MPUAR6, *((unsigned int *)&ar)); 
          break;
        case 7: 
          Set_system_register(AVR32_MPUAR7, *((unsigned int *)&ar));
          break;
  }
  return(MPU_SETUP_ENTRY_OK);
}

/*!
 * Disable a MPU entry
 * \param region_number: MPU entry region number (0..7).
 * \param register_select:  register A : '0' -- B : '1'
 */
void disable_mpu_entry(unsigned int region_number, unsigned int register_select)
{
  /* Set Address Register. */
  avr32_mpuar0_t ar;

	register_select += 0; // Avoid unused variable warning

  ar.base = 0;
  ar.size = 0;
  ar.v = 0;
  
  /* Instruction entry */
  switch ( region_number & 0x7 )
  {
  default:
        case 0: 
           Set_system_register(AVR32_MPUAR0, *((unsigned int *)&ar)); 
           break;
        case 1: 
          Set_system_register(AVR32_MPUAR1, *((unsigned int *)&ar)); 
          break;
        case 2: 
          Set_system_register(AVR32_MPUAR2, *((unsigned int *)&ar)); 
          break;
        case 3: 
          Set_system_register(AVR32_MPUAR3, *((unsigned int *)&ar)); 
          break;
        case 4: 
          Set_system_register(AVR32_MPUAR4, *((unsigned int *)&ar)); 
          break;
        case 5: 
          Set_system_register(AVR32_MPUAR5, *((unsigned int *)&ar)); 
          break;
        case 6: 
          Set_system_register(AVR32_MPUAR6, *((unsigned int *)&ar)); 
          break;
        case 7: 
          Set_system_register(AVR32_MPUAR7, *((unsigned int *)&ar));
          break;
  }

}

/*!
 * Setup a register A and B
 * \param region_number: MPU entry region number (0..7).
 * \param register_select:  register A : '0' -- B : '1'
 * \param right_access: R/W/X see doc32002.pdf (Table 5-3. Access permissions implied by the APn bits)
 */
void set_access_permissions(unsigned int region_number, unsigned int register_select, unsigned int right_access)
{
  avr32_mpuapra_t mpu_regA;
  avr32_mpuaprb_t mpu_regB; 
  
  *(U32 *)&mpu_regA = (U32) Get_system_register(AVR32_MPUAPRA);
  *(U32 *)&mpu_regB = (U32) Get_system_register(AVR32_MPUAPRB);

  /* Instruction entry */ 
  if (register_select==0) //Register A
  {
      switch ( region_number & 0x7 )
      {
        default:
        case 0: 
           mpu_regA.ap0 = (right_access);
           break;
        case 1: 
          mpu_regA.ap1  = (right_access);
          break;
        case 2:         
          mpu_regA.ap2  = (right_access);
          break;
        case 3: 
          mpu_regA.ap3  = (right_access);
          break;
        case 4: 
          mpu_regA.ap4  = (right_access);
          break;
        case 5: 
          mpu_regA.ap5  = (right_access);
          break;
        case 6: 
          mpu_regA.ap6  = (right_access);
          break;
        case 7: 
          mpu_regA.ap7  = (right_access);
          break;
      }
      /* Set permissions */
      Set_system_register(AVR32_MPUAPRA, *((unsigned int *)&mpu_regA));
  }
  else //Register B
  {
      switch ( region_number & 0x7 )
      {
            default:
        case 0: 
           mpu_regB.ap0 = (right_access);
           break;
        case 1: 
          mpu_regB.ap1  = (right_access);
          break;
        case 2:         
          mpu_regB.ap2  = (right_access);
          break;
        case 3: 
          mpu_regB.ap3  = (right_access);
          break;
        case 4: 
          mpu_regB.ap4  = (right_access);
          break;
        case 5: 
          mpu_regB.ap5  = (right_access);
          break;
        case 6: 
          mpu_regB.ap6  = (right_access);
          break;
        case 7: 
          mpu_regB.ap7  = (right_access);
          break;
      }    
        /* Set permissions */
        Set_system_register(AVR32_MPUAPRB, *((unsigned int *)&mpu_regB));
  }
}

/*!
 * Setup a Subregion
 * \param region_number: MPU entry region number (0..7).
 * \param pattern_select:  register A : '0' -- B : '1'
 */
void select_subregion(unsigned int region_number, unsigned int pattern_select)
{
  /* Instruction entry */
  switch ( region_number & 0x7 )
  {
  default:
  case 0: Set_system_register(AVR32_MPUPSR0, pattern_select );break;
  case 1: Set_system_register(AVR32_MPUPSR1, pattern_select );break;
  case 2: Set_system_register(AVR32_MPUPSR2, pattern_select );break;
  case 3: Set_system_register(AVR32_MPUPSR3, pattern_select );break;
  case 4: Set_system_register(AVR32_MPUPSR4, pattern_select );break;
  case 5: Set_system_register(AVR32_MPUPSR5, pattern_select );break;
  case 6: Set_system_register(AVR32_MPUPSR6, pattern_select );break;
  case 7: Set_system_register(AVR32_MPUPSR7, pattern_select );break;
  }
}
