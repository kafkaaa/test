/*******************************************************************************
 *
 * Copyright (c) 2016
 * Lumi, JSC.
 * All Rights Reserved
 *
 *
 * Description:
 *
 * Author: Quang Huy
 *
 * Last Changed By:  $Author: Quang Huy
 * Revision:         $Revision: 1.0.0.1 $
 * Last Changed:     $Date:  $
 *
 ******************************************************************************/
#ifndef __APPCONFIG_H__
#define __APPCONFIG_H__
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
#define USE_DEBUG
#ifdef USE_DEBUG

#define DEBUG_I2C_CMD
#define DEDUG_I2C_DRV
#endif /* DEBUG */

/*Define parameters for Txconfig*/
#define LORA_RF_FREQUENCY							434175000 // Hz
#define LORA_TX_OUTPUT_POWER						20				// dBm
#define LORA_FDEV									0
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_DATARATE					            8         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_CRC_ON									true
#define LORA_FREQ_HOP_ON							0
#define LORA_HOP_PERIOUS							0
#define LORA_IQ_INVERSION_ON                        false
#define LORA_TIMEOUT								3000

/*Define parameters for Rxconfig*/
#define LORA_BANDWIDTH_AFC							0 // for lora_modem, bandwidth_afc = 0
#define LORA_PAYLOAD_LENGTH							10
#define LORA_SYS_TIMEOUT							3000
#define LORA_RX_CONTINUOUS							true

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/

/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

#endif
/** End Of File **/
