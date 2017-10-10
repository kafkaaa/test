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

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "LoRaMAC.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */
/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD   	255
#define NB_PKT_MAX      			8 	/* max number of packets per fetch/send cycle */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

uint8_t LoRaMacFrameType = FRAME_TYPE_PROPRIETARY;

const uint8_t netID = 0x01;

struct lgw_pkt_tx_s txpkt; 				/* array containing 1 outbound packet + metadata */

/* allocate memory for packet fetching and processing */
struct lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; /* array containing inbound packets + metadata */
int nb_pkt;
int meas_dw;
/*!
 * Device Address
 */
static uint32_t LoRaMacDevAddr;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */
void print_tx_status(uint8_t tx_status) {
    switch (tx_status) {
        case TX_OFF:
            MSG("INFO: [jit] lgw_status returned TX_OFF\n");
            break;
        case TX_FREE:
            MSG("INFO: [jit] lgw_status returned TX_FREE\n");
            break;
        case TX_EMITTING:
            MSG("INFO: [jit] lgw_status returned TX_EMITTING\n");
            break;
        case TX_SCHEDULED:
            MSG("INFO: [jit] lgw_status returned TX_SCHEDULED\n");
            break;
        default:
            MSG("INFO: [jit] lgw_status returned UNKNOWN (%d)\n", tx_status);
            break;
    }
}

LoRaMacStatus_t Send( LoRaMacFrameType_t LoRaMacFrameType, uint32_t p_freq, int p_sf, int p_cr, int p_bw, uint8_t *p_payload, uint16_t p_payloadSize)
{
	LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

	int i;
	uint8_t status_var;
	char* name_ofPacket;
	
	/* Application Parameters */
    char mod[64] = DEFAULT_MODULATION;
    uint32_t f_target = p_freq; /* target frequency - invalid default value, has to be specified by user */
    int sf = p_sf; /* SF10 by default */
    int cr = p_cr; /* CR1 aka 4/5 by default */
    int bw = p_bw; /* 125kHz bandwidth by default */
    //~ printf("%u %d %d %d",p_freq, p_sf,  p_cr,p_bw );
    int pow = 14; /* 14 dBm by default */
    int preamb = 8; /* 8 symbol preamble by default */
    int pl_size = p_payloadSize; /* 16 bytes payload by default */
    bool invert = false;
    /* For FSK */
    float br_kbps = DEFAULT_BR_KBPS;
    uint8_t fdev_khz = DEFAULT_FDEV_KHZ;
    /* For LBT */
    bool lbt_enable = false;
    uint32_t sx1301_count_us;
    
    /* fill-up payload and parameters */
    memset(&txpkt, 0, sizeof(txpkt));
    txpkt.freq_hz = f_target;
    if (lbt_enable == true) {
        txpkt.tx_mode = TIMESTAMPED;
    } else {
        txpkt.tx_mode = IMMEDIATE;
    }
    txpkt.rf_chain = TX_RF_CHAIN;
    txpkt.rf_power = pow;
    if( strcmp( mod, "FSK" ) == 0 ) {
        txpkt.modulation = MOD_FSK;
        txpkt.datarate = br_kbps * 1e3;
        txpkt.f_dev = fdev_khz;
    } else {
        txpkt.modulation = MOD_LORA;
        switch (bw) {
            case 3: txpkt.bandwidth = BW_125KHZ; break;
            case 2: txpkt.bandwidth = BW_250KHZ; break;
            case 1: txpkt.bandwidth = BW_500KHZ; break;
            default:
                MSG("ERROR: invalid 'bw' variable\n");
                return EXIT_FAILURE;
        }
        switch (sf) {
            case  2: txpkt.datarate = DR_LORA_SF7;  break;
            case  4: txpkt.datarate = DR_LORA_SF8;  break;
            case  8: txpkt.datarate = DR_LORA_SF9;  break;
            case 16: txpkt.datarate = DR_LORA_SF10; break;
            case 32: txpkt.datarate = DR_LORA_SF11; break;
            case 64: txpkt.datarate = DR_LORA_SF12; break;
            default:
                MSG("ERROR: invalid 'sf' variable\n");
                return EXIT_FAILURE;
        }
        switch (cr) {
            case 1: txpkt.coderate = CR_LORA_4_5; break;
            case 2: txpkt.coderate = CR_LORA_4_6; break;
            case 3: txpkt.coderate = CR_LORA_4_7; break;
            case 4: txpkt.coderate = CR_LORA_4_8; break;
            default:
                MSG("ERROR: invalid 'cr' variable\n");
                return EXIT_FAILURE;
        }
    }
    txpkt.invert_pol = invert;
    txpkt.preamble = preamb;
    txpkt.size = pl_size;
    
    /* Prepare the packet payload */
    LoRaMacBufferPktLen = 0;
	txpkt.payload[LoRaMacBufferPktLen++] = LoRaMacFrameType;

	switch(LoRaMacFrameType)
	{
		case FRAME_TYPE_JOIN_ACCEPT:
			{
				txpkt.payload[LoRaMacBufferPktLen++] =	LoRaMacDevAddr & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = netID;
				
				printf("Device %d %d %d %d 's Join Request is accepted!\n",txpkt.payload[1],txpkt.payload[2],txpkt.payload[3],txpkt.payload[4]);
				printf("-----------\n");
				name_ofPacket = "Join Accept";
			}
			break;
		case FRAME_TYPE_DATA_UNCONFIRMED_DOWN: /* Response for Data packet from Node */
			{
				txpkt.payload[LoRaMacBufferPktLen++] = LoRaMacDevAddr & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
				txpkt.payload[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;
				name_ofPacket = "Unconfirmed Data Down";
			}
			break;
		case FRAME_TYPE_DATA_CONFIRMED_DOWN: /* Control packet for Node */
			{
				txpkt.payload[LoRaMacBufferPktLen++] = p_payload[0];
				txpkt.payload[LoRaMacBufferPktLen++] = p_payload[1];
				txpkt.payload[LoRaMacBufferPktLen++] = p_payload[2];
				txpkt.payload[LoRaMacBufferPktLen++] = p_payload[3];
				txpkt.payload[LoRaMacBufferPktLen++] = p_payload[4];
				txpkt.payload[LoRaMacBufferPktLen++] = p_payload[5];

				//~ printf("txpkt.payload:");
					//~ for(i=0;i<7;i++){
						//~ printf("%d ",txpkt.payload[i]);
					//~ }
					//~ printf("\n");
					
				name_ofPacket = "Confirmed Data Down";
			}
			break;
		default:
			return LORAMAC_STATUS_SERVICE_UNKNOWN; break;
	}
	
    //~ strcpy((char *)txpkt.payload, "123456" ); /* copy prepared payload to packet */
	
    /* Send to Radio */
	++meas_nb_tx_pkt;

	/* When LBT is enabled, immediate send is not allowed, so we need
		to set a timestamp to the packet */
	if (lbt_enable == true) {
		/* Get the current SX1301 time */
		lgw_reg_w(LGW_GPS_EN, 0);
		lgw_get_trigcnt(&sx1301_count_us);
		lgw_reg_w(LGW_GPS_EN, 1);

		/* Set packet timestamp to current time + few milliseconds */
		txpkt.count_us = sx1301_count_us + 50E3;
	}

	/* check if concentrator is free for sending new packet */
	uint8_t tx_status;
	piLock(gw_lock); /* may have to wait for a fetch to finish */
	i = lgw_status(TX_STATUS, &tx_status);
	piUnlock(gw_lock); /* free concentrator ASAP */
	if (i == LGW_HAL_ERROR) {
		MSG("WARNING: [Send] lgw_status failed\n");
	}
	else {
		if (tx_status == TX_EMITTING) {
			MSG("ERROR: concentrator is currently emitting\n");
			print_tx_status(tx_status);
		}
		else if (tx_status == TX_SCHEDULED) {
			MSG("WARNING: a downlink was already scheduled, overwritting it...\n");
				print_tx_status(tx_status);
		}
		else {
				/* Nothing to do */
		}
	}

	/* send packet */
	printf("Sending packet number %u - %s\n", meas_nb_tx_pkt, name_ofPacket);
	piLock(gw_lock); 		/* may have to wait for a fetch to finish */
	i = lgw_send(txpkt);	/* non-blocking scheduling of TX packet */
	printf("-----------\n");
	piUnlock(gw_lock); 		/* free concentrator ASAP */
	if (i == LGW_HAL_ERROR) {
		/* Number of failure packet */
		piLock(meas_dw); 
		meas_nb_tx_fail += 1;
		piUnlock(meas_dw); 	
		MSG("WARNING: [jit] lgw_send failed\n");
	} else if (i == LGW_LBT_ISSUE ) {
		printf("Failed: Not allowed (LBT)\n");
	} else {
		/* wait for packet to finish sending */
		do {
			wait_ms(5);
			lgw_status(TX_STATUS, &status_var); /* get TX status */
		} while (status_var != TX_FREE);
		status = LORAMAC_STATUS_OK;	
		
		if(LoRaMacFrameType == FRAME_TYPE_DATA_CONFIRMED_DOWN){
			meas_nb_tx_command += 1;
		}
	}
	return status;
}

void Receive()
{
	int i; /* loop variables */

	/* fetch packets */
	piLock(gw_lock);
	nb_pkt = lgw_receive(NB_PKT_MAX, rxpkt);
	piUnlock(gw_lock);
	if (nb_pkt == LGW_HAL_ERROR) {
		MSG("ERROR: failed packet fetch, exiting\n");
	} else if (nb_pkt == 0) {
		delayMicroseconds(1);
	} else {
		printf("Received Packet: ");
		for(i = 0; i < rxpkt->size; i++){
			printf("%d ", rxpkt->payload[i]);
		}
		printf("\n");
		printf("-----------\n");
		OnRadioRxDone(rxpkt);
	}
}

void OnRadioRxDone(struct lgw_pkt_rx_s *packet){
	uint8_t pktLen = 0;
	uint8_t netIDFromGW = 0;
	uint8_t LoRaMacFrameType = packet->payload[pktLen++];

	switch(LoRaMacFrameType)
	{
		case FRAME_TYPE_JOIN_REQ:
			{
				LoRaMacDevAddr = packet->payload[pktLen++];
				LoRaMacDevAddr |= ( (uint32_t)packet->payload[pktLen++] << 8 );
				LoRaMacDevAddr |= ( (uint32_t)packet->payload[pktLen++] << 16 );
				LoRaMacDevAddr |= ( (uint32_t)packet->payload[pktLen++] << 24 );
				printf("Device %d %d %d %d send Join Request!\n",packet->payload[1],packet->payload[2],packet->payload[3],packet->payload[4]);
				printf("-----------\n");
				
				//~ printf("freq_hz: %d, sf: %d, cr: %d, bw: %d \n", packet->freq_hz, packet->datarate, packet->coderate, packet->bandwidth);
				dtb_checkDevAddr(packet->freq_hz, packet->datarate, packet->coderate, packet->bandwidth, packet->payload);
			
				//~ piLock(gw_lock);
				Send(FRAME_TYPE_JOIN_ACCEPT, packet->freq_hz, packet->datarate, packet->coderate, packet->bandwidth, 0, 6);
				//~ piUnlock(gw_lock);
				
			}
			break;
		case FRAME_TYPE_DATA_CONFIRMED_UP: /* Data packet from Node */
			{
				netIDFromGW = packet->payload[4];
				if(netIDFromGW==netID)
				{
					LoRaMacDevAddr = packet->payload[pktLen++];
					LoRaMacDevAddr |= ( (uint32_t)packet->payload[pktLen++] << 8 );
					LoRaMacDevAddr |= ( (uint32_t)packet->payload[pktLen++] << 16 );
					LoRaMacDevAddr |= ( (uint32_t)packet->payload[pktLen++] << 24 );	
					
					if(dtb_checkDevExist(packet->payload) == 1){
						/* Import to Pi Buffer */
						import_toPiBuffer(packet->payload);
					}
					
					//~ piLock(gw_lock);
					Send(FRAME_TYPE_DATA_UNCONFIRMED_DOWN, packet->freq_hz, packet->datarate, packet->coderate, packet->bandwidth, 0, 5);
					//~ piUnlock(gw_lock);
					
				}
			}
			break;
		case FRAME_TYPE_DATA_UNCONFIRMED_UP: /* Response packet from Node for control command */
			{
				meas_nb_tx_response += 1;
			}
			break;
		default:
			break;
	} 
}

/** End Of File **/
