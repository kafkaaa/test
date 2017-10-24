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
#ifndef __MQTT_H__
#define __MQTT_H__

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "database.h"
#include "board.h"
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

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
uint32_t meas_nb_rx_ok;
/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
/* @brief */
int catch_signal(int sig,void (*handler) (int));

/* @brief */
void alive(int sig);

/* @brief */
void term(int sig);

/* @brief */
void error(char* msg);

/* @brief */
int send_packet(void* socket_info, const void* buf, unsigned int count);

/* @brief */
void init_socket(mqtt_broker_handle_t* broker, const char* hostname, short port, int keepalive);

/* @brief */
int close_socket(mqtt_broker_handle_t* broker);

/* @brief */
void read_socket();

/* @brief */
void mqtt_publish(uint8_t* node_data);

/* @brief */
void mqtt_subscribe_all();

/* @brief */
void mqtt_init_all();

/* @brief */
void mqtt_makeNewDevice(char *id);

/* @brief */
void mqtt_subscribeToDevice(char *id);
#endif
/** End Of File **/
