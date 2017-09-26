/*******************************************************************************
 *
 * Copyright (c) 2017
 * All Rights Reserved
 *
 *
 * Description: Include file for application
 *
 * Author: Nguyen Canh Thang
 *
 * Last Changed By:  $Author: Nguyen Canh Thang $
 * Revision:         $Revision: 1.0.0.1 $
 * Last Changed:     $Date:  $
 *
 ******************************************************************************/
#ifndef __MAIN_H__
#define __MAIN_H__
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "board.h"
#include "buffer.h"
#include "database.h"
#include "mqtt.h"
#include "sx1276.h"
#include "spi_gpio.h"
#include "appconfig.h"
#include <string.h>
#include <math.h>
#include <wiringPi.h>

int a,b,c;
PI_THREAD (listen_toServer){
	while(1)
	{
		read_socket();
	}
}

PI_THREAD (send_controlRequest){
	while(1)
	{
		export_fromServerBuffer();
		delayMicroseconds(100);
	}
}

PI_THREAD (send_toServer){
	while(1)
	{
		export_fromPiBuffer();
		delayMicroseconds(100);
	}
}

void createThread(){
	a = piThreadCreate (listen_toServer);
	if(a != 0) printf("Thread listen_toServer did not start!");
	
	b = piThreadCreate (send_controlRequest);
	if(b != 0) printf("Thread send_controlRequest did not start!");
	
	c = piThreadCreate (send_toServer);
	if(b != 0) printf("Thread send_toServer did not start!");
}

#endif
/** End Of File **/
