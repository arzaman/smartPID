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
 * $Revision:    V2.00
 *
 * Project:      USB Host Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.00
 *    Replaced function ARM_USBH_PortPowerOnOff with ARM_USBH_PortVbusOnOff
 *    Changed function ARM_USBH_EndpointCreate parameters
 *    Replaced function ARM_USBH_EndpointConfigure with ARM_USBH_EndpointModify
 *    Replaced function ARM_USBH_EndpointClearHalt with ARM_USBH_EndpointReset
 *    Replaced function ARM_USBH_URB_Submit with ARM_USBH_EndpointTransfer
 *    Replaced function ARM_USBH_URB_Abort with ARM_USBH_EndpointTransferAbort
 *    Added function ARM_USBD_EndpointTransferGetResult
 *    Added function ARM_USBH_GetFrameNumber
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.20
 *    Added API for OHCI/EHCI Host Controller Interface (HCI)
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */ 

#ifndef __DRIVER_USBH_H
#define __DRIVER_USBH_H

#include "Driver_USB.h"

#define ARM_USBH_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,0)  /* API version */


/**
\brief USB Host Port State
*/
typedef struct _ARM_USBH_PORT_STATE {
  uint32_t connected   : 1;             ///< USB Host Port connected flag
  uint32_t overcurrent : 1;             ///< USB Host Port overcurrent flag
  uint32_t speed       : 2;             ///< USB Host Port speed setting
} ARM_USBH_PORT_STATE;

/**
\brief USB Host Endpoint Handle
*/
typedef uint32_t ARM_USBH_EP_HANDLE;


/****** USB Host Packet *****/
#define ARM_USBH_PACKET_TOKEN_Pos         0
#define ARM_USBH_PACKET_TOKEN_Msk        (0x0FUL << ARM_USBH_PACKET_TOKEN_Pos)
#define ARM_USBH_PACKET_SETUP            (0x01UL << ARM_USBH_PACKET_TOKEN_Pos)
#define ARM_USBH_PACKET_OUT              (0x02UL << ARM_USBH_PACKET_TOKEN_Pos)
#define ARM_USBH_PACKET_IN               (0x03UL << ARM_USBH_PACKET_TOKEN_Pos)
#define ARM_USBH_PACKET_PING             (0x04UL << ARM_USBH_PACKET_TOKEN_Pos)

#define ARM_USBH_PACKET_DATA_Pos          4
#define ARM_USBH_PACKET_DATA_Msk         (0x0FUL << ARM_USBH_PACKET_DATA_Pos)
#define ARM_USBH_PACKET_DATA0            (0x01UL << ARM_USBH_PACKET_DATA_Pos)
#define ARM_USBH_PACKET_DATA1            (0x02UL << ARM_USBH_PACKET_DATA_Pos)

#define ARM_USBH_PACKET_SPLIT_Pos         8
#define ARM_USBH_PACKET_SPLIT_Msk        (0x0FUL << ARM_USBH_PACKET_SPLIT_Pos)
#define ARM_USBH_PACKET_SSPLIT           (0x08UL << ARM_USBH_PACKET_SPLIT_Pos)
#define ARM_USBH_PACKET_SSPLIT_S         (0x09UL << ARM_USBH_PACKET_SPLIT_Pos)
#define ARM_USBH_PACKET_SSPLIT_E         (0x0AUL << ARM_USBH_PACKET_SPLIT_Pos)
#define ARM_USBH_PACKET_SSPLIT_S_E       (0x0BUL << ARM_USBH_PACKET_SPLIT_Pos)
#define ARM_USBH_PACKET_CSPLIT           (0x0CUL << ARM_USBH_PACKET_SPLIT_Pos)         

#define ARM_USBH_PACKET_PRE              (1UL << 12)


/****** USB Host Port Event *****/
#define ARM_USBH_EVENT_CONNECT           (1UL << 0)     ///< USB Device Connected to Port
#define ARM_USBH_EVENT_DISCONNECT        (1UL << 1)     ///< USB Device Disconnected from Port
#define ARM_USBH_EVENT_OVERCURRENT       (1UL << 2)     ///< USB Device caused Overcurrent
#define ARM_USBH_EVENT_RESET             (1UL << 3)     ///< USB Reset completed
#define ARM_USBH_EVENT_SUSPEND           (1UL << 4)     ///< USB Suspend occurred
#define ARM_USBH_EVENT_RESUME            (1UL << 5)     ///< USB Resume occurred
#define ARM_USBH_EVENT_REMOTE_WAKEUP     (1UL << 6)     ///< USB Device activated Remote Wakeup

/****** USB Host Endpoint Event *****/
#define ARM_USBH_EVENT_TRANSFER_COMPLETE (1UL << 0)     ///< Transfer completed
#define ARM_USBH_EVENT_HANDSHAKE_NAK     (1UL << 1)     ///< NAK Handshake received
#define ARM_USBH_EVENT_HANDSHAKE_NYET    (1UL << 2)     ///< NYET Handshake received
#define ARM_USBH_EVENT_HANDSHAKE_MDATA   (1UL << 3)     ///< MDATA Handshake received
#define ARM_USBH_EVENT_HANDSHAKE_STALL   (1UL << 4)     ///< STALL Handshake received
#define ARM_USBH_EVENT_HANDSHAKE_ERR     (1UL << 5)     ///< ERR Handshake received
#define ARM_USBH_EVENT_BUS_ERROR         (1UL << 6)     ///< Bus Error detected


#ifndef __DOXYGEN_MW__                  // exclude from middleware documentation

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_USBH_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
/**
  \fn          ARM_USBH_CAPABILITIES ARM_USBH_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_USBH_CAPABILITIES
*/
/**
  \fn          int32_t ARM_USBH_Initialize (ARM_USBH_SignalPortEvent_t     cb_port_event,
                                            ARM_USBH_SignalEndpointEvent_t cb_endpoint_event)
  \brief       Initialize USB Host Interface.
  \param[in]   cb_port_event      Pointer to \ref ARM_USBH_SignalPortEvent
  \param[in]   cb_endpoint_event  Pointer to \ref ARM_USBH_SignalEndpointEvent
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_Uninitialize (void)
  \brief       De-initialize USB Host Interface.
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_PowerControl (ARM_POWER_STATE state)
  \brief       Control USB Host Interface Power.
  \param[in]   state Power state
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_PortVbusOnOff (uint8_t port, bool vbus)
  \brief       Root HUB Port VBUS on/off.
  \param[in]   port  Root HUB Port Number
  \param[in]   vbus
                - \b false VBUS off
                - \b true  VBUS on
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_PortReset (uint8_t port)
  \brief       Do Root HUB Port Reset.
  \param[in]   port  Root HUB Port Number
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_PortSuspend (uint8_t port)
  \brief       Suspend Root HUB Port (stop generating SOFs).
  \param[in]   port  Root HUB Port Number
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_PortResume (uint8_t port)
  \brief       Resume Root HUB Port (start generating SOFs).
  \param[in]   port  Root HUB Port Number
  \return      \ref execution_status
*/
/**
  \fn          ARM_USBH_PORT_STATE ARM_USBH_PortGetState (uint8_t port)
  \brief       Get current Root HUB Port State.
  \param[in]   port  Root HUB Port Number
  \return      Port State \ref ARM_USBH_PORT_STATE
*/
/**
  \fn          ARM_USBH_EP_HANDLE ARM_USBH_EndpointCreate (uint8_t  dev_addr,
                                                           uint8_t  dev_speed,
                                                           uint8_t  hub_addr,
                                                           uint8_t  hub_port,
                                                           uint8_t  ep_addr,
                                                           uint8_t  ep_type,
                                                           uint16_t ep_max_packet_size,
                                                           uint8_t  ep_interval)
  \brief       Create Endpoint in System.
  \param[in]   dev_addr   Device Address
  \param[in]   dev_speed  Device Speed
  \param[in]   hub_addr   Hub Address
  \param[in]   hub_port   Hub Port
  \param[in]   ep_addr    Endpoint Address
                - ep_addr.0..3: Address
                - ep_addr.7:    Direction
  \param[in]   ep_type    Endpoint Type
  \param[in]   ep_max_packet_size Endpoint Maximum Packet Size
  \param[in]   ep_interval        Endpoint Polling Interval
  \return      Endpoint Handle \ref ARM_USBH_EP_HANDLE
*/
/**
  \fn          int32_t            ARM_USBH_EndpointModify (ARM_USBH_EP_HANDLE ep_hndl,
                                                           uint8_t            dev_addr,
                                                           uint8_t            dev_speed,
                                                           uint8_t            hub_addr,
                                                           uint8_t            hub_port,
                                                           uint16_t           ep_max_packet_size)
  \brief       Modify Endpoint in System.
  \param[in]   ep_hndl    Endpoint Handle
  \param[in]   dev_addr   Device Address
  \param[in]   dev_speed  Device Speed
  \param[in]   hub_addr   Hub Address
  \param[in]   hub_port   Hub Port
  \param[in]   ep_max_packet_size Endpoint Maximum Packet Size
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_EndpointDelete (ARM_USBH_EP_HANDLE ep_hndl)
  \brief       Delete Endpoint from System.
  \param[in]   ep_hndl  Endpoint Handle
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_EndpointReset (ARM_USBH_EP_HANDLE ep_hndl)
  \brief       Reset Endpoint.
  \param[in]   ep_hndl  Endpoint Handle
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_EndpointTransfer (ARM_USBH_EP_HANDLE ep_hndl,
                                                  uint32_t packet,
                                                  uint8_t *data,
                                                  uint32_t num)
  \brief       Transfer packets through USB Endpoint.
  \param[in]   ep_hndl  Endpoint Handle
  \param[in]   packet   Packet information
  \param[in]   data     Pointer to buffer with data to send or for data to receive
  \param[in]   num      Number of data bytes to transfer
  \return      \ref execution_status
*/
/**
  \fn          uint32_t ARM_USBH_EndpointTransferGetResult (ARM_USBH_EP_HANDLE ep_hndl)
  \brief       Get result of USB Endpoint transfer.
  \param[in]   ep_hndl  Endpoint Handle
  \return      number of successfully transfered data bytes
*/
/**
  \fn          int32_t ARM_USBH_EndpointTransferAbort (ARM_USBH_EP_HANDLE ep_hndl)
  \brief       Abort current USB Endpoint transfer.
  \param[in]   ep_hndl  Endpoint Handle
  \return      \ref execution_status
*/
/**
  \fn          uint16_t ARM_USBH_GetFrameNumber (void)
  \brief       Get current USB Frame Number.
  \return      Frame Number
*/

/**
  \fn          void ARM_USBH_SignalPortEvent (uint8_t port, uint32_t event)
  \brief       Signal Root HUB Port Event.
  \param[in]   port  Root HUB Port Number
  \param[in]   event \ref USBH_port_events
  \return      none
*/
/**
  \fn          void ARM_USBH_SignalEndpointEvent (ARM_USBH_EP_HANDLE ep_hndl, uint32_t event)
  \brief       Signal Endpoint Event.
  \param[in]   ep_hndl  Endpoint Handle
  \param[in]   event    \ref USBH_ep_events
  \return      none
*/

typedef void (*ARM_USBH_SignalPortEvent_t)     (uint8_t port, uint32_t event);                ///< Pointer to \ref ARM_USBH_SignalPortEvent : Signal Root HUB Port Event.
typedef void (*ARM_USBH_SignalEndpointEvent_t) (ARM_USBH_EP_HANDLE ep_hndl, uint32_t event);  ///< Pointer to \ref ARM_USBH_SignalEndpointEvent : Signal Endpoint Event.


/**
\brief USB Host Driver Capabilities.
*/
typedef struct _ARM_USBH_CAPABILITIES {
  uint32_t port_mask           : 15;    ///< Root HUB available Ports Mask
  uint32_t auto_split          :  1;    ///< Automatic SPLIT packet handling
  uint32_t event_connect       :  1;    ///< Signal Connect event
  uint32_t event_disconnect    :  1;    ///< Signal Disconnect event
  uint32_t event_overcurrent   :  1;    ///< Signal Overcurrent event
} ARM_USBH_CAPABILITIES;


/**
\brief Access structure of USB Host Driver.
*/
typedef struct _ARM_DRIVER_USBH {
  ARM_DRIVER_VERSION    (*GetVersion)                (void);                                             ///< Pointer to \ref ARM_USBH_GetVersion : Get driver version.
  ARM_USBH_CAPABILITIES (*GetCapabilities)           (void);                                             ///< Pointer to \ref ARM_USBH_GetCapabilities : Get driver capabilities.
  int32_t               (*Initialize)                (ARM_USBH_SignalPortEvent_t     cb_port_event,            
                                                      ARM_USBH_SignalEndpointEvent_t cb_endpoint_event); ///< Pointer to \ref ARM_USBH_Initialize : Initialize USB Host Interface.
  int32_t               (*Uninitialize)              (void);                                             ///< Pointer to \ref ARM_USBH_Uninitialize : De-initialize USB Host Interface.
  int32_t               (*PowerControl)              (ARM_POWER_STATE state);                            ///< Pointer to \ref ARM_USBH_PowerControl : Control USB Host Interface Power.
  int32_t               (*PortVbusOnOff)             (uint8_t port, bool vbus);                          ///< Pointer to \ref ARM_USBH_PortVbusOnOff : Root HUB Port VBUS on/off.
  int32_t               (*PortReset)                 (uint8_t port);                                     ///< Pointer to \ref ARM_USBH_PortReset : Do Root HUB Port Reset.
  int32_t               (*PortSuspend)               (uint8_t port);                                     ///< Pointer to \ref ARM_USBH_PortSuspend : Suspend Root HUB Port (stop generating SOFs).
  int32_t               (*PortResume)                (uint8_t port);                                     ///< Pointer to \ref ARM_USBH_PortResume : Resume Root HUB Port (start generating SOFs).
  ARM_USBH_PORT_STATE   (*PortGetState)              (uint8_t port);                                     ///< Pointer to \ref ARM_USBH_PortGetState : Get current Root HUB Port State.
  ARM_USBH_EP_HANDLE    (*EndpointCreate)            (uint8_t dev_addr,
                                                      uint8_t dev_speed,
                                                      uint8_t hub_addr,
                                                      uint8_t hub_port,
                                                      uint8_t ep_addr,
                                                      uint8_t ep_type,
                                                      uint16_t ep_max_packet_size,
                                                      uint8_t ep_interval);                              ///< Pointer to \ref ARM_USBH_EndpointCreate : Create Endpoint in System.
  int32_t               (*EndpointModify)            (ARM_USBH_EP_HANDLE ep_hndl,
                                                      uint8_t dev_addr,
                                                      uint8_t dev_speed,
                                                      uint8_t hub_addr,
                                                      uint8_t hub_port,
                                                      uint16_t ep_max_packet_size);                      ///< Pointer to \ref ARM_USBH_EndpointModify : Modify Endpoint in System.
  int32_t               (*EndpointDelete)            (ARM_USBH_EP_HANDLE ep_hndl);                       ///< Pointer to \ref ARM_USBH_EndpointDelete : Delete Endpoint from System.
  int32_t               (*EndpointReset)             (ARM_USBH_EP_HANDLE ep_hndl);                       ///< Pointer to \ref ARM_USBH_EndpointReset : Reset Endpoint.
  int32_t               (*EndpointTransfer)          (ARM_USBH_EP_HANDLE ep_hndl, 
                                                      uint32_t packet,
                                                      uint8_t *data,
                                                      uint32_t num);                                     ///< Pointer to \ref ARM_USBH_EndpointTransfer : Transfer packets through USB Endpoint.
  uint32_t              (*EndpointTransferGetResult) (ARM_USBH_EP_HANDLE ep_hndl);                       ///< Pointer to \ref ARM_USBD_EndpointTransferGetResult : Get result of USB Endpoint transfer.
  int32_t               (*EndpointTransferAbort)     (ARM_USBH_EP_HANDLE ep_hndl);                       ///< Pointer to \ref ARM_USBH_EndpointTransferAbort : Abort current USB Endpoint transfer.
  uint16_t              (*GetFrameNumber)            (void);                                             ///< Pointer to \ref ARM_USBH_GetFrameNumber : Get current USB Frame Number.                    
} const ARM_DRIVER_USBH;


// HCI (OHCI/EHCI)

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_USBH_HCI_GetVersion (void)
  \brief       Get USB Host HCI (OHCI/EHCI) driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
/**
  \fn          ARM_USBH_HCI_CAPABILITIES ARM_USBH_HCI_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_USBH_HCI_CAPABILITIES
*/
/**
  \fn          int32_t ARM_USBH_HCI_Initialize (ARM_USBH_HCI_Interrupt_t *cb_interrupt)
  \brief       Initialize USB Host HCI (OHCI/EHCI) Interface.
  \param[in]   cb_interrupt Pointer to Interrupt Handler Routine
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_HCI_Uninitialize (void)
  \brief       De-initialize USB Host HCI (OHCI/EHCI) Interface.
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_HCI_PowerControl (ARM_POWER_STATE state)
  \brief       Control USB Host HCI (OHCI/EHCI) Interface Power.
  \param[in]   state Power state
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_USBH_HCI_PortVbusOnOff (uint8_t port, bool vbus)
  \brief       USB Host HCI (OHCI/EHCI) Root HUB Port VBUS on/off.
  \param[in]   port  Root HUB Port Number
  \param[in]   vbus
                - \b false VBUS off
                - \b true  VBUS on
  \return      \ref execution_status
*/

/**
  \fn          void ARM_USBH_HCI_Interrupt (void)
  \brief       USB Host HCI Interrupt Handler.
  \return      none
*/

typedef void (*ARM_USBH_HCI_Interrupt_t) (void);  ///< Pointer to Interrupt Handler Routine.


/**
\brief USB Host HCI (OHCI/EHCI) Driver Capabilities.
*/
typedef struct _ARM_USBH_HCI_CAPABILITIES {
  uint32_t port_mask : 15;              ///< Root HUB available Ports Mask
} ARM_USBH_HCI_CAPABILITIES;


/**
  \brief Access structure of USB Host HCI (OHCI/EHCI) Driver.
*/
typedef struct _ARM_DRIVER_USBH_HCI {
  ARM_DRIVER_VERSION        (*GetVersion)      (void);                                  ///< Pointer to \ref ARM_USBH_HCI_GetVersion : Get USB Host HCI (OHCI/EHCI) driver version.
  ARM_USBH_HCI_CAPABILITIES (*GetCapabilities) (void);                                  ///< Pointer to \ref ARM_USBH_HCI_GetCapabilities : Get driver capabilities.
  int32_t                   (*Initialize)      (ARM_USBH_HCI_Interrupt_t cb_interrupt); ///< Pointer to \ref ARM_USBH_HCI_Initialize : Initialize USB Host HCI (OHCI/EHCI) Interface.
  int32_t                   (*Uninitialize)    (void);                                  ///< Pointer to \ref ARM_USBH_HCI_Uninitialize : De-initialize USB Host HCI (OHCI/EHCI) Interface.
  int32_t                   (*PowerControl)    (ARM_POWER_STATE state);                 ///< Pointer to \ref ARM_USBH_HCI_PowerControl : Control USB Host HCI (OHCI/EHCI) Interface Power.
  int32_t                   (*PortVbusOnOff)   (uint8_t port, bool vbus);               ///< Pointer to \ref ARM_USBH_HCI_PortVbusOnOff : USB Host HCI (OHCI/EHCI) Root HUB Port VBUS on/off.
} const ARM_DRIVER_USBH_HCI;

#endif /* __DOXYGEN_MW__ */

#endif /* __DRIVER_USBH_H */
