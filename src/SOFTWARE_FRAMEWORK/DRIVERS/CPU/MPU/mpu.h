/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3A-1.4.0 Release */

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

#ifndef _MPU_H_
#define _MPU_H_

#include "compiler.h"

//! MPU Access Permission Register A & B identifiers.
//! @{
#define  MPU_APRA_ID   0
#define  MPU_APRB_ID   1
//! @}


//! MPU Access Permissions
//! @{

#define  MPU_PRIVR_UNPRIVNONE      0   //! Privileged mode: Read, Unprivileged mode: None
#define  MPU_PRIVRX_UNPRIVNONE     1   //! Privileged mode: Read/Execute, Unprivileged mode: None
#define  MPU_PRIVRW_UNPRIVNONE     2   //! Privileged mode: Read/Write, Unprivileged mode: None
#define  MPU_PRIVRWX_UNPRIVNONE    3   //! Privileged mode: Read/Write/Execute, Unprivileged mode: None
#define  MPU_PRIVR_UNPRIVR         4   //! Privileged mode: Read, Unprivileged mode: Read
#define  MPU_PRIVRX_UNPRIVRX       5   //! Privileged mode: Read/Execute, Unprivileged mode: Read/Execute
#define  MPU_PRIVRW_UNPRIVRW       6   //! Privileged mode: Read/Write, Unprivileged mode: Read/Write
#define  MPU_PRIVRWX_UNPRIVRWX     7   //! Privileged mode: Read/Write/Execute, Unprivileged mode: Read/Write/Execute
#define  MPU_PRIVRW_UNPRIVR        8   //! Privileged mode: Read/Write, Unprivileged mode: Read
#define  MPU_PRIVRW_UNPRIVRX       9   //! Privileged mode: Read/Write, Unprivileged mode: Read/Execute
#define  MPU_PRIVNONE_UNPRIVNONE   10  //! Privileged mode: None, Unprivileged mode: None
//! @}


//! MPU entry set up function (set_mpu_entry) return codes.
//! @{
#define  MPU_SETUP_ENTRY_OK                0
#define  MPU_SETUP_ENTRY_INVALIDBASEADDR   1
#define  MPU_SETUP_ENTRY_INVALIDSIZE       2
//! @}

/*!
 * Possible protected region sizes. Based on Table 5.1 in doc32002 "AVR32UC Technical Reference Manual Complete".
 */
typedef enum
{
   MPU_REGION_SIZE_LOWLIMIT_FORBIDDEN= 10,
   MPU_REGION_SIZE_4KB,
   MPU_REGION_SIZE_8KB,
   MPU_REGION_SIZE_16KB,
   MPU_REGION_SIZE_32KB,
   MPU_REGION_SIZE_64KB,
   MPU_REGION_SIZE_128KB,
   MPU_REGION_SIZE_256KB,
   MPU_REGION_SIZE_512KB,
   MPU_REGION_SIZE_1MB,
   MPU_REGION_SIZE_2MB,
   MPU_REGION_SIZE_4MB,
   MPU_REGION_SIZE_8MB,
   MPU_REGION_SIZE_16MB,
   MPU_REGION_SIZE_32MB,
   MPU_REGION_SIZE_64MB,
   MPU_REGION_SIZE_128MB,
   MPU_REGION_SIZE_256MB,
   MPU_REGION_SIZE_512MB,
   MPU_REGION_SIZE_1GB,
   MPU_REGION_SIZE_2GB,
   MPU_REGION_SIZE_4GB,
   MPU_REGION_SIZE_HIGHLIMIT_FORBIDDEN
} eRegionSize;


/*! Structure for MPU entry
*/
typedef struct {
  unsigned int addr; /*! Base address of the protection region. Must be aligned on a 4kB boundary. */
  eRegionSize size;  /*! Size of the protection region */
  Bool valid;        /*! Protection validity. One of SET(the protection region is valid) or CLR(the region is not considered in the protection testing). */
} mpu_entry_t;

#define CAST(to, from)                   \
  ({                                     \
    typeof(from) __temp__ = from;        \
    to = *(typeof(to) *)&__temp__;       \
  })

/*!
 * \brief Enable the memory protection unit. MPU address checking is enabled
 */
extern void enable_mpu(void);

/*!
 * \brief Disable the memory protection unit. MPU address checking is disabled, no exceptions will be generated by the MPU.
 */
extern void disable_mpu(void);


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
extern Bool mpu_convert_kbsize_to_eregionsize(eRegionSize *peRegionSizeValue, U32 kBSizeValue);

/*!
 * Setup a MPU entry
 * \param mpu_entry: pointer to mpu_entry_t with the MPU settings (base address, size, validity, access permission) .
 * \param region_number: MPU entry region number (0..7).
 * \return int       MPU_SETUP_ENTRY_OK              if the setup succeeded
 *                   MPU_SETUP_ENTRY_INVALIDBASEADDR if the setup failed because the input base address is not aligned on a 4kB boundary.
 *                   MPU_SETUP_ENTRY_INVALIDSIZE     if the setup failed because the input size is not a power of 2 or is less than 4kB or is bigger than 4GB.
 */
extern char set_mpu_entry(const mpu_entry_t *mpu_entry, unsigned int region_number);

/*!
 * Disable a MPU entry
 * \param region_number: MPU entry region number (0..7).
 * \param register_select:  register A : '0' -- B : '1'
 */
extern void disable_mpu_entry(unsigned int region_number, unsigned int register_select);

/*!
 * Setup a register A and B
 * \param region_number: MPU entry region number (0..7).
 * \param register_select:  register A : '0' -- B : '1'
 * \param right_access: R/W/X see doc32002.pdf (Table 5-3. Access permissions implied by the APn bits)
 */
extern void set_access_permissions(unsigned int region_number, unsigned int register_select, unsigned int right_access);

/*!
 * Setup a Subregion
 * \param region_number: MPU entry region number (0..7).
 * \param pattern_select:  register A : '0' -- B : '1'
 */
extern void select_subregion(unsigned int region_number, unsigned int pattern_select);
#endif // _MPU_H_
