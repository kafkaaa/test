/*******************************************************************************
 *
 * Copyright (c) 2016
 * All Rights Reserved
 *
 *
 * Description:
 *
 * Author: Nguyen Canh Thang
 *
 * Last Changed By:  $Author: Nguyen Canh Thang
 * Revision:         $Revision: 1.0.0.1 $
 * Last Changed:     $Date:  $
 *
 ******************************************************************************/
 
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "main.h"

/**
 * Main application entry point.
 */

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

	/* When data receive (DIO edge rising), go into GW_Interupt */
	wiringPiISR(DI0,INT_EDGE_RISING,&gw_Interupt);
	
	while(1)
	{

	}
	
}
/** End Of File **/
