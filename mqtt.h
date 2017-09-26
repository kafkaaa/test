/*******************************************************************************
 *
 * Copyright (c) 2017
 * All Rights Reserved
 *
 * Description:
 *
 * Author: Nguyen Canh Thang
 *
 * Last Changed By:  $Author: Nguyen Canh Thang $
 * Revision:         $Revision: 1.0.0.0 $
 * Last Changed:     $Date:  $
 *
 ******************************************************************************/
 
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#ifndef __MQTT_H__
#define __MQTT_H__

#include "database.h"
#include "libemqtt.h"
#include "buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/tcp.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <wiringPi.h>
#include <errno.h>
#include <sys/socket.h>
#include <time.h>

int catch_signal(int sig,void (*handler) (int));
void alive(int sig);
void term(int sig);
void error(char* msg);
int send_packet(void* socket_info, const void* buf, unsigned int count);
void init_socket(mqtt_broker_handle_t* broker, const char* hostname, short port, int keepalive);
int close_socket(mqtt_broker_handle_t* broker);
void read_socket();
void mqtt_publish(uint8_t* node_data);
void mqtt_subscribe_all();
void mqtt_init_all();
void mqtt_makeNewDevice(char *id);
void mqtt_subscribeToDevice(char *id);
#endif
/** End Of File **/
