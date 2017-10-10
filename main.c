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

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
void createThread();

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
	wiringPiISR(DI0,INT_EDGE_RISING,&lgw_interupt);
	
	while(1)
	{

	}
}

/* ------------------------------------------------------------------------------- */
/* --- THREAD 2: CHECKING PACKETS TO BE SENT FROM PI BUFFER AND SEND TO SERVER --- */
PI_THREAD (thread_sendToServer){
    while(1)
    {
        bf_exportFromPiBuffer();
        delayMicroseconds(100);
    }
}

/* ----------------------------------------------------------------------------- */
/* --- THREAD 3: LISTENING TO SERVER AND IMPORTING COMMANDS TO SERVER BUFFER --- */
PI_THREAD (thread_listenToServer){
    while(1)
    {
        read_socket();
    }
}

/* --------------------------------------------------------------------------------- */
/* --- THREAD 4: CHECKING PACKETS TO BE SENT FROM SERVER BUFFER AND SEND TO NODE --- */
PI_THREAD (thread_sendToNode){
    while(1)
    {
        bf_exportFromServerBuffer();
        delayMicroseconds(100);
    }
}

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

/** End Of File **/
