/* ----------------------------------------------------------------------------
 *
 * Copyright (c) 2017
 * All Rights Reserved
 *
 * Description:
 *
 * Author: Nguyen Canh Thang
 *
 * Last Changed By:  $Author: Nguyen Canh Thang
 * Revision:         $Revision: 1.0.0.1 $
 * Last Changed:     $Date:  $
 *
 ---------------------------------------------------------------------------- */
#ifndef __LORAMAC_H__
#define __LORAMAC_H__

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "loragw_hal.h"
#include "buffer.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */
#define MSG(args...)    fprintf(stderr, args) /* message that is destined to the user */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */
#define TX_RF_CHAIN                 0 /* TX only supported on radio A */
#define DEFAULT_RSSI_OFFSET         0.0
#define DEFAULT_MODULATION          "LORA"
#define DEFAULT_BR_KBPS             50
#define DEFAULT_FDEV_KHZ            25
#define DEFAULT_NOTCH_FREQ          129000U /* 129 kHz */
#define DEFAULT_SX127X_RSSI_OFFSET  -4 /* dB */

/* Measurement of TX Parameters */
extern uint32_t meas_nb_tx_fail; 	/* count packets were TX failed for other reasons */
extern uint32_t meas_nb_tx_pkt;	/* count packets emitted */
extern uint32_t meas_nb_tx_command; 	/* count command packets emitted successfully */
extern uint32_t meas_nb_tx_response; /* count packets responded*/

/* THREAD LOCK */
extern int gw_lock; /* control access to the concentrator */
extern int meas_dw; /* control access to the downstream measurements */
/*!
 * LoRaMAC Frame Types
 */
typedef enum eLoRaMacFrameType
{
		FRAME_TYPE_JOIN_REQ					= 0x00,
		FRAME_TYPE_JOIN_ACCEPT				= 0x01,
		FRAME_TYPE_DATA_UNCONFIRMED_UP		= 0x02,
		FRAME_TYPE_DATA_UNCONFIRMED_DOWN	= 0x03,
		FRAME_TYPE_DATA_CONFIRMED_UP		= 0x04,
		FRAME_TYPE_DATA_CONFIRMED_DOWN		= 0x05,
		FRAME_TYPE_RFU						= 0x06,
		FRAME_TYPE_PROPRIETARY				= 0x07,	
}LoRaMacFrameType_t;

/*!
 * LoRaMAC Status
 */
typedef enum eLoRaMacStatus
{
    /*!
     * Service started successfully
     */
    LORAMAC_STATUS_OK,
    /*!
     * Service not started - LoRaMAC is busy
     */
    LORAMAC_STATUS_BUSY,
    /*!
     * Service unknown
     */
    LORAMAC_STATUS_SERVICE_UNKNOWN,
    /*!
     * Service not started - invalid parameter
     */
    LORAMAC_STATUS_PARAMETER_INVALID,
    /*!
     * Service not started - invalid frequency
     */
    LORAMAC_STATUS_FREQUENCY_INVALID,
    /*!
     * Service not started - invalid datarate
     */
    LORAMAC_STATUS_DATARATE_INVALID,
    /*!
     * Service not started - invalid frequency and datarate
     */
    LORAMAC_STATUS_FREQ_AND_DR_INVALID,
    /*!
     * Service not started - the device is not in a LoRaWAN
     */
    LORAMAC_STATUS_NO_NETWORK_JOINED,
    /*!
     * Service not started - playload lenght error
     */
    LORAMAC_STATUS_LENGTH_ERROR,
    /*!
     * Service not started - playload lenght error
     */
    LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR,
    /*!
     * Service not started - the device is switched off
     */
    LORAMAC_STATUS_DEVICE_OFF,
}LoRaMacStatus_t;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
/**
@brief print the tx status of gateway
*/
void print_tx_status(uint8_t tx_status);
/**
@brief send packet with settable parameters
*/
LoRaMacStatus_t Send( LoRaMacFrameType_t LoRaMacFrameType, uint32_t p_freq, int p_sf, int p_cr, int p_bw, uint8_t *p_payload, uint16_t p_payloadSize);
/**
@brief receive packet from device
*/
void Receive( void );
/**
@brief check packet and do some work
*/
void OnRadioRxDone(struct lgw_pkt_rx_s *packet);
#endif
/** End Of File **/
