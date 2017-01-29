/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        2. Jan 2014
 * $Revision:    V1.11
 *
 * Project:      NOR Flash Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.11
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */ 

#ifndef __DRIVER_NOR_H
#define __DRIVER_NOR_H

#include "Driver_Common.h"

#define  ARM_NOR_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,11)  /* API version */


#define  ARM_NOR_SECTOR_INFO(addr,size) { addr, addr+size-4 }

#define _ARM_NOR_Sector_(n)          NOR_Sector##_##n
#define  ARM_NOR_Sector_(n)     _ARM_NOR_Sector_(n)
#define _ARM_NOR_Device_(n)          NOR_Device##_##n
#define  ARM_NOR_Device_(n)     _ARM_NOR_Device_(n)

#define _ARM_NOR_SPI_Slave_(n)       NOR_SPI_Slave##_##n
#define  ARM_NOR_SPI_Slave_(n)  _ARM_NOR_SPI_Slave_(n)
#define _ARM_NOR_SPI_Driver_(n)      NOR_SPI_Driver##_##n
#define  ARM_NOR_SPI_Driver_(n) _ARM_NOR_SPI_Driver_(n)

#define _ARM_Driver_NOR_(n)          Driver_NOR##_##n
#define  ARM_Driver_NOR_(n)     _ARM_Driver_NOR_(n)


/**
\brief NOR Flash Sector information
*/
typedef struct _ARM_NOR_SECTOR {
  uint32_t  start;                      ///< Sector Start address
  uint32_t  end;                        ///< Sector End address (start+size-4)
} const ARM_NOR_SECTOR;

/**
\brief NOR Flash Device information
*/
typedef struct _ARM_NOR_DEVICE {
  uint32_t        size;                 ///< Device size in bytes, typically hole size of the flash device. It is allowed to specify only a part of the device to be used for storage. The rest of the device can be used for the application code. In this case, replace \b EraseChip in the driver control block \ref ARM_DRIVER_NOR with NULL. 
  uint32_t        sector_count;         ///< Number of sectors.
  ARM_NOR_SECTOR *sector_info;          ///< Sector layout. Copy flash sector layout information from Flash device datasheet. \n - Flash sectors must not be aligned continuously. Gaps are allowed in the device memory space. \n - Flash sectors can be reserved for application code. Do not include such sectors in the description table.
  uint8_t         erased_value;         ///< Contents of erased memory. In most cases, this value is set to \b 0xFF. Accepted values are \b 0xFF or \b 0x00. 
} const ARM_NOR_DEVICE;


/**
\brief NOR Flash Status of executed operation
*/
typedef enum _ARM_NOR_STATUS {
  ARM_NOR_OK            =  0,           ///< Operation succeeded
  ARM_NOR_ERROR         =  1            ///< Unspecified error
} ARM_NOR_STATUS;


// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_NOR_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_Initialize (uint32_t base_addr)
  \brief       Initialize NOR Flash interface.
  \param[in]   base_addr   Device base address
  \return      execution status \ref ARM_NOR_STATUS
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_Uninitialize (void)
  \brief       Deinitialize NOR Flash interface.
  \return      execution status \ref ARM_NOR_STATUS
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_PowerControl (ARM_POWER_STATE state)
  \brief       Control NOR Flash interface power.
  \param[in]   state    Power state
  \return      execution status \ref ARM_NOR_STATUS
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_ReadData (uint32_t addr, uint8_t *data, uint32_t size)
  \brief       Read data from NOR Flash. Optional function. Used for non-memory-mapped devices.
  \param[in]   addr  Starting data address. Must be 4-byte aligned. 
  \param[out]  data  Pointer to buffer storing the data read from NOR Flash.
  \param[in]   size  Number of bytes to read. Must be a multiple of 4.
  \return      execution status \ref ARM_NOR_STATUS
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_WriteData (uint32_t addr, const uint8_t *data, uint32_t size)
  \brief       Write data to NOR Flash device.
  \param[in]   addr  Data address.
  \param[in]   data  Pointer to a buffer containing the data to be written to NOR Flash.   Must be a multiple of 4.
  \param[in]   size  Number of bytes to be written. Must be 4-byte-aligned.
  \return      execution status \ref ARM_NOR_STATUS
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_EraseSector (uint32_t addr)
  \brief       Erase NOR Flash Sector.
  \param[in]   addr  Sector address
  \return      execution status \ref ARM_NOR_STATUS
*/
/**
  \fn          ARM_NOR_STATUS ARM_NOR_EraseChip (void)
  \brief       Erase complete NOR Flash.
               Optional function for faster full chip erase.
  \return      execution status \ref ARM_NOR_STATUS
*/


/**
\brief Access structure of the NOR Flash Driver
*/
typedef struct _ARM_DRIVER_NOR {
  ARM_DRIVER_VERSION (*GetVersion)   (void);                                               ///< Pointer to \ref ARM_NOR_GetVersion : Get driver version.
  ARM_NOR_STATUS     (*Initialize)   (uint32_t base_addr);                                 ///< Pointer to \ref ARM_NOR_Initialize : Initialize NOR Flash Interface.
  ARM_NOR_STATUS     (*Uninitialize) (void);                                               ///< Pointer to \ref ARM_NOR_Uninitialize : De-initialize NOR Flash Interface.
  ARM_NOR_STATUS     (*PowerControl) (ARM_POWER_STATE state);                              ///< Pointer to \ref ARM_NOR_PowerControl : Control NOR Flash Interface Power.
  ARM_NOR_STATUS     (*ReadData)     (uint32_t addr,       uint8_t *data, uint32_t size);  ///< Pointer to \ref ARM_NOR_ReadData : Read data from NOR Flash. Optional, NULL for parallel memory-mapped devices.
  ARM_NOR_STATUS     (*WriteData)    (uint32_t addr, const uint8_t *data, uint32_t size);  ///< Pointer to \ref ARM_NOR_WriteData : Write data to NOR Flash.
  ARM_NOR_STATUS     (*EraseSector)  (uint32_t addr);                                      ///< Pointer to \ref ARM_NOR_EraseSector : Erase NOR Flash Sector.
  ARM_NOR_STATUS     (*EraseChip)    (void);                                               ///< Pointer to \ref ARM_NOR_EraseChip : Erase complete NOR Flash. Optional function.
} const ARM_DRIVER_NOR;

#endif /* __DRIVER_NOR_H */
