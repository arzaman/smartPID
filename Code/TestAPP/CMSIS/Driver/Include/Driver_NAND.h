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
 * Project:      NAND Flash Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.11
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */ 

#ifndef __DRIVER_NAND_H
#define __DRIVER_NAND_H

#include "Driver_Common.h"

#define ARM_NAND_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,11)  /* API version */


/**
\brief NAND Page Layout configuration
*/
typedef struct _ARM_NAND_PAGE_LAYOUT {
  struct {
    uint8_t  ofs_lsn;                     ///< LSN position, where logical sector number (LSN) is placed. Usually, this is the first byte of Spare and has the value 0. LSN is a 32-bit value.
    uint8_t  ofs_dcm;                     ///< Page Data corrupted marker. Usually, this byte is the 5-th byte of Spare and has the value 4.
    uint8_t  ofs_bbm;                     ///< Bad Block marker position. Is usually placed as the 6-th byte of Spare and has the value 5.
    uint8_t  ofs_ecc;                     ///< Position of the first byte of Error Correction Code (ECC). Is usually the 7-th byte of Spare and has the value 6. This value is used by flash translation layer only if ECC is encoded and decoded in software.
  } spare;                                ///< Structure spare
  uint16_t spare_ofs;                     ///< Spare area offset from beginning of the page.
  uint16_t spare_inc;                     ///< Column increment till next spare. If page contains multiple sectors, then the first byte of the first spare area is determined by reading spare_ofs value. Location of the first byte of the second spare is (spare_inc + spare_ofs).
  uint16_t sector_inc;                    ///< Column increment till next sector. If page contains multiple sectors, then the first sector always starts at the beginning of the page (byte zero). Second sector starts at sect_inc, third sector at (sect_inc + sect_inc) and so on.
} ARM_NAND_PAGE_LAYOUT;

/**
\brief NAND Flash Device information
*/
typedef struct _ARM_NAND_DEVICE {
  ARM_NAND_PAGE_LAYOUT *page_layout;      ///< Page Layout configuration   
  uint8_t               type;             ///< \ref ARM_NAND_TYPE 
  uint8_t               device_number;    ///< Device number (chip select)
  uint16_t              page_size;        ///< Page Size in bytes
  uint32_t              block_count;      ///< Number of Blocks in Device
  uint16_t              page_count;       ///< Number of Pages per Block
  uint16_t              block_sectors;    ///< Number of Sectors per Block
  uint8_t               page_sectors;     ///< Number of Sectors per Page  
  uint8_t               row_cycles;       ///< Number of Row address cycles  
  uint8_t               col_cycles;       ///< Number of Column address cycles  
  uint8_t               sw_ecc;           ///< value > 0: error correction code (ECC) encoding/decoding enabled in software: \n value = 0: Software ECC disabled \n value = 1: Hamming ECC algorithm enabled in software.
} const ARM_NAND_DEVICE;

/**
\brief Mandatory NAND Flash Commands (ONFI V1.0 or higher)
*/
typedef enum _ARM_NAND_COMMAND {
  ARM_NAND_CMD_READ_1ST          = 0x00,  ///< Read 1st Cycle
  ARM_NAND_CMD_CHANGE_RD_COL_1ST = 0x05,  ///< Change Read Column 1st Cycle
  ARM_NAND_CMD_PROGRAM_2ND       = 0x10,  ///< Page Program 2nd Cycle
  ARM_NAND_CMD_READ_2ND          = 0x30,  ///< Read 2nd Cycle
  ARM_NAND_CMD_ERASE_1ST         = 0x60,  ///< Block Erase 1st Cycle
  ARM_NAND_CMD_STATUS            = 0x70,  ///< Read Status
  ARM_NAND_CMD_PROGRAM_1ST       = 0x80,  ///< Page Program 1st Cycle
  ARM_NAND_CMD_CHANGE_WR_COL     = 0x85,  ///< Change Write Column
  ARM_NAND_CMD_READ_ID           = 0x90,  ///< Read ID
  ARM_NAND_CMD_ERASE_2ND         = 0xD0,  ///< Block Erase 2nd cycle
  ARM_NAND_CMD_CHANGE_RD_COL_2ND = 0xE0,  ///< Change Read Column 2nd Cycle
  ARM_NAND_CMD_READ_PARAM_PAGE   = 0xEC,  ///< Read Parameter Page
  ARM_NAND_CMD_RESET             = 0xFF   ///< Reset Command
} ARM_NAND_COMMAND;

/**
\brief Optional NAND Flash Commands (ONFI V1.0 or higher)
*/
typedef enum _ARM_NAND_COMMAND_OPTIONAL {
  ARM_NAND_CMD_COPYBACK_READ_1ST      = 0x00, ///< Copyback Read 1st Cycle
  ARM_NAND_CMD_COPYBACK_READ_2ND      = 0x35, ///< Copyback Read 2nd Cycle
  ARM_NAND_CMD_READ_CACHE_SEQUENTIAL  = 0x31, ///< Read Cache Sequential
  ARM_NAND_CMD_READ_CACHE_END         = 0x3F, ///< Read Cache End
  ARM_NAND_CMD_READ_STATUS_ENHANCED   = 0x78, ///< Read Status Enhanced
  ARM_NAND_CMD_PAGE_CACHE_PROGRAM_1ST = 0x80, ///< Page Cache Program 1st Cycle
  ARM_NAND_CMD_PAGE_CACHE_PROGRAM_2ND = 0x15, ///< Page Cache Program 2nd Cycle
  ARM_NAND_CMD_COPYBACK_PROGRAM_1ST   = 0x85, ///< Copyback Program 1st Cycle
  ARM_NAND_CMD_COPYBACK_PROGRAM_2ND   = 0x10, ///< Copyback Program 2nd Cycle
  ARM_NAND_CMD_READ_UNIQUE_ID         = 0xED, ///< Read Unique ID
  ARM_NAND_CMD_GET_FEATURES           = 0xEE, ///< Get Features
  ARM_NAND_CMD_SET_FEATURES           = 0xEF  ///< Set Features
} ARM_NAND_COMMAND_OPTIONAL;

// NAND Status Flags Masks (ONFI V1.0 or higher)
#define ARM_NAND_STAT_FAIL        0x01    ///< Last command failed
#define ARM_NAND_STAT_FAILC       0x02    ///< Command prior last failed
#define ARM_NAND_STAT_CSP         0x08    ///< Command specific
#define ARM_NAND_STAT_VSP         0x10    ///< Vendor specific
#define ARM_NAND_STAT_ARDY        0x20    ///< Array operation in progress
#define ARM_NAND_STAT_RDY         0x40    ///< LUN ready for another command
#define ARM_NAND_STAT_WP          0x80    ///< Write protected

// NAND ID addresses (ONFI V1.0 or higher)
#define ARM_NAND_ID_ADDR_ONFI     0x20    ///< ONFI signature address for Read ID command
#define ARM_NAND_ID_ADDR_JEDEC    0x00    ///< JEDEC signature address for the Read ID command


/**
\brief NAND Flash Device Type
*/
typedef enum _ARM_NAND_TYPE {
  ARM_NAND_TYPE_RAW_NAND = 0,             ///< Raw NAND Flash device
  ARM_NAND_TYPE_EZ_NAND  = 1,             ///< NAND Flash device with integrated ECC
  ARM_NAND_TYPE_ONENAND  = 2,             ///< OneNAND Flash device
} ARM_NAND_TYPE;


/**
\brief NAND Flash Driver Status
*/
typedef enum _ARM_NAND_STATUS {
  ARM_NAND_OK             =  0,           ///< No error
  ARM_NAND_ECC_CORRECTED  =  1,           ///< ECC corrected the data
  ARM_NAND_ECC_FAILED     =  2,           ///< ECC could not correct the data
  ARM_NAND_PROGRAM_FAILED =  3,           ///< Programming failed
  ARM_NAND_ERASE_FAILED   =  4,           ///< Erase verify failed
  ARM_NAND_TIMEOUT        =  5,           ///< NAND hardware timeout
  ARM_NAND_UNSUPPORTED    =  6,           ///< Functionality not supported
  ARM_NAND_ERROR          =  7            ///< Unspecified error
} ARM_NAND_STATUS;


// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_NAND_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
/**
  \fn          ARM_NAND_CAPABILITIES ARM_NAND_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_NAND_CAPABILITIES
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_Initialize (ARM_NAND_SignalEvent_t  cb_event,
                                                    ARM_NAND_DEVICE        *ptr_device,
                                                    uint32_t                num_devices)
  \brief       Initialize NAND Interface.
  \param[in]   cb_event    Pointer to \ref ARM_NAND_SignalEvent
  \param[in]   ptr_device  Pointer to device information
  \param[in]   num_devices Number of devices
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_Uninitialize (void)
  \brief       De-initialize NAND Interface.
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_PowerControl (ARM_POWER_STATE state)
  \brief       Control the NAND interface power.
  \param[in]   state    Power state
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_ResetDevice (uint32_t dev_num)
  \brief       Reset a NAND device.
  \param[in]   dev_num   Device number
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_ReadID (uint32_t  dev_num,
                                                uint8_t   addr,
                                                uint8_t  *buf,
                                                uint32_t  len)
  \brief       Read NAND device ID.
  \param[in]   dev_num   Device number
  \param[in]   addr      ID address
  \param[out]  buf       Buffer for data read from NAND
  \param[in]   len       Number of bytes to read (buffer length)
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_ReadParamPage (uint32_t  dev_num,
                                                       uint32_t  col,
                                                       uint8_t  *buf,
                                                       uint32_t  len)
  \brief       Read NAND parameter page.
  \param[in]   dev_num   Device number
  \param[in]   col       Column address
  \param[out]  buf       Buffer for data read from NAND
  \param[in]   len       Number of bytes to read (buffer length)
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_ReadPage (uint32_t  dev_num,
                                                  uint32_t  row,
                                                  uint32_t  col,
                                                  uint8_t  *buf,
                                                  uint32_t  len)
  \brief       Read data from NAND page.
  \param[in]   dev_num   Device number
  \param[in]   row       Row address
  \param[in]   col       Column address
  \param[out]  buf       Buffer for data read from NAND
  \param[in]   len       Number of bytes to read (buffer length)
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_WritePage (      uint32_t  dev_num,
                                                         uint32_t  row,
                                                         uint32_t  col,
                                                   const uint8_t  *buf,
                                                         uint32_t  len)
  \brief       Write data to NAND page.
  \param[in]   dev_num   Device number
  \param[in]   row       Row address
  \param[in]   col       Column address
  \param[out]  buf       Buffer with data to write to NAND
  \param[in]   len       Number of bytes to write (buffer length)
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_CopyPage (uint32_t dev_num,
                                                  uint32_t row_src,
                                                  uint32_t row_dst,
                                                  uint32_t row_cnt)
  \brief       Copy pages within NAND device.
  \param[in]   dev_num   Device number
  \param[in]   row_src   Source row address
  \param[in]   row_dst   Destination row address
  \param[in]   row_cnt   Number of pages (rows) to copy
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_EraseBlock (uint32_t  dev_num,
                                                    uint32_t  row)
  \brief       Erase blocks in NAND device.
  \param[in]   dev_num   Device number
  \param[in]   row       Block start row address
  \return      execution status \ref ARM_NAND_STATUS
*/
/**
  \fn          ARM_NAND_STATUS ARM_NAND_ReadStatus (uint32_t dev_num, uint8_t *stat)
  \brief       Read NAND device status.
  \param[in]   dev_num   Device number
  \param[out]  stat      Pointer to store status read from NAND
  \return      execution status \ref ARM_NAND_STATUS
*/

/**
  \fn          void ARM_NAND_SignalEvent (uint32_t dev_num)
  \brief       Signal NAND event. Callback function.
  \param[in]   dev_num   Device number
               Occurs when Program/Erase (WritePage, CopyPage, EraseBlock) completes.
  \return      none
*/

typedef void (*ARM_NAND_SignalEvent_t) (uint32_t dev_num);    ///< Pointer to \ref ARM_NAND_SignalEvent : Signal NAND Event.


/**
\brief NAND Driver Capabilities.
*/
typedef struct _ARM_NAND_CAPABILITIES {
  uint32_t events   :  1;               ///< Signals events
  uint32_t raw_nand :  1;               ///< Supports NAND Interface
  uint32_t one_nand :  1;               ///< Supports OneNAND Interface
  uint32_t ecc_slc  :  1;               ///< Supports ECC for SLC NAND
  uint32_t ecc_mlc  :  1;               ///< Supports ECC for MLC NAND
  uint32_t reserved : 27;               ///< reserved for future extension
} ARM_NAND_CAPABILITIES;


/**
\brief Access structure of the NAND Driver.
*/
typedef struct _ARM_DRIVER_NAND {
  ARM_DRIVER_VERSION    (*GetVersion)     (void);                                                                               ///< Pointer to \ref ARM_NAND_GetVersion : Get driver version.
  ARM_NAND_CAPABILITIES (*GetCapabilities)(void);                                                                               ///< Pointer to \ref ARM_NAND_GetCapabilities : Get driver capabilities.
  ARM_NAND_STATUS       (*Initialize)     (ARM_NAND_SignalEvent_t cb_event, ARM_NAND_DEVICE *ptr_device, uint32_t num_devices); ///< Pointer to \ref ARM_NAND_Initialize : Initialize NAND Interface.
  ARM_NAND_STATUS       (*Uninitialize)   (void);                                                                               ///< Pointer to \ref ARM_NAND_Uninitialize : De-initialize NAND Interface.
  ARM_NAND_STATUS       (*PowerControl)   (ARM_POWER_STATE state);                                                              ///< Pointer to \ref ARM_NAND_PowerControl : Control NAND Interface Power.
  ARM_NAND_STATUS       (*ResetDevice)    (uint32_t dev_num);                                                                   ///< Pointer to \ref ARM_NAND_ResetDevice : Reset NAND Device.
  ARM_NAND_STATUS       (*ReadID)         (uint32_t dev_num, uint8_t addr, uint8_t *buf, uint32_t len);                         ///< Pointer to \ref ARM_NAND_ReadID : Read Device ID from NAND.
  ARM_NAND_STATUS       (*ReadParamPage)  (uint32_t dev_num, uint32_t col, uint8_t *buf, uint32_t len);                         ///< Pointer to \ref ARM_NAND_ReadParamPage : Read parameter page from NAND.
  ARM_NAND_STATUS       (*ReadPage)       (uint32_t dev_num, uint32_t row, uint32_t col, uint8_t *buf, uint32_t len);           ///< Pointer to \ref ARM_NAND_ReadPage : Read data from NAND Page.
  ARM_NAND_STATUS       (*WritePage)      (uint32_t dev_num, uint32_t row, uint32_t col, const uint8_t *buf, uint32_t len);     ///< Pointer to \ref ARM_NAND_WritePage : Write data to NAND Page.
  ARM_NAND_STATUS       (*CopyPage)       (uint32_t dev_num, uint32_t row_src, uint32_t row_dst, uint32_t row_cnt);             ///< Pointer to \ref ARM_NAND_CopyPage : Copy pages in NAND Device.
  ARM_NAND_STATUS       (*EraseBlock)     (uint32_t dev_num, uint32_t row);                                                     ///< Pointer to \ref ARM_NAND_EraseBlock : Erase block in NAND Device.
  ARM_NAND_STATUS       (*ReadStatus)     (uint32_t dev_num, uint8_t *stat);                                                    ///< Pointer to \ref ARM_NAND_ReadStatus : Read Device status from NAND.
} const ARM_DRIVER_NAND;

#endif /* __DRIVER_NAND_H */
