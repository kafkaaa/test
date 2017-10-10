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
#include "board.h"
#include "buffer.h"
#include "database.h"
#include "mqtt.h"
#include "sx1276.h"
#include "spi_gpio.h"
#include <string.h>
#include <math.h>
#include <wiringPi.h>

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */
#define ARRAY_SIZE(a)   (sizeof(a) / sizeof((a)[0]))
#define STRINGIFY(x)    #x
#define STR(x)          STRINGIFY(x)

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define USE_DEBUG
#ifdef USE_DEBUG
#define DEBUG_I2C_CMD
#define DEDUG_I2C_DRV
#endif /* DEBUG */

/* Define parameters for Txconfig */
#define LORA_RF_FREQUENCY							434175000   // Hz
#define LORA_TX_OUTPUT_POWER						20		    // dBm
#define LORA_FDEV									0
#define LORA_BANDWIDTH                              0           // [0: 125 kHz,
                                                                //  1: 250 kHz,
                                                                //  2: 500 kHz,
                                                                //  3: Reserved]
#define LORA_DATARATE					            8           // [SF7..SF12]
#define LORA_CODINGRATE                             1           // [1: 4/5,
                                                                //  2: 4/6,
                                                                //  3: 4/7,
                                                                //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8           // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_CRC_ON									true
#define LORA_FREQ_HOP_ON							0
#define LORA_HOP_PERIOUS							0
#define LORA_IQ_INVERSION_ON                        false
#define LORA_TIMEOUT								3000

/* Define parameters for Rxconfig */
#define LORA_BANDWIDTH_AFC							0           // for lora_modem, bandwidth_afc = 0
#define LORA_PAYLOAD_LENGTH							10
#define LORA_SYS_TIMEOUT							3000
#define LORA_RX_CONTINUOUS							true
/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES (GLOBAL) ------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
void createThread();

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */
void createThread(){
    int i;
    i = piThreadCreate (thread_sendToServer);
    if(i != 0) printf("thread_sendToServer did not start!");

    i = piThreadCreate (thread_listenToServer);
    if(i != 0) printf("thread_listenToServer did not start!");

    i = piThreadCreate (thread_sendToNode);
    if(i != 0) printf("thread_sendToNode did not start!");
}

/* -------------------------------------------------------------------------- */
/* --- MAIN FUNCTION -------------------------------------------------------- */
int main( void )
{
	/* Init GPIO */
	BoardInit();
	
	/* Init SX1276 */
	LoRaMacInitialization();
	
	/* Set Rx Continuous */
	RxWindowSetup(LORA_RX_CONTINUOUS);
	
	/* Init MQTT */
	mqtt_init_all();
	
	/* Init Database */
	dtb_init();
	
	/* Init Thread */
	createThread();

    /* THREAD 1 - RECEIVING PACKETS AND IMPORTING THEM TO PI BUFFER  */
	wiringPiISR(DI0,INT_EDGE_RISING,&gw_Interupt);
	
	while(1)
	{

	}
}

/* ------------------------------------------------------------------------------- */
/* --- THREAD 2: CHECKING PACKETS TO BE SENT FROM PI BUFFER AND SEND TO SERVER --- */
PI_THREAD (thread_sendToServer){
    while(1)
    {
        export_fromPiBuffer();
        delayMicroseconds(100);
    }
}

/* ------------------------------------------------------------------------------- */
/* --- THREAD 3: LISTENING TO SERVER AND IMPORTING COMMANDS TO SERVER BUFFER --- */
PI_THREAD (thread_listenToServer){
    while(1)
    {
        read_socket();
    }
}

/* ------------------------------------------------------------------------------- */
/* --- THREAD 4: CHECKING PACKETS TO BE SENT FROM SERVER BUFFER AND SEND TO NODE --- */
PI_THREAD (thread_sendToNode){
    while(1)
    {
        export_fromServerBuffer();
        delayMicroseconds(100);
    }
}

/** End Of File **/
