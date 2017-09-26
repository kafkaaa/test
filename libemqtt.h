/*
 * This file is part of libemqtt.
 *
 * libemqtt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libemqtt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libemqtt.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *
 * Created by Filipe Varela on 09/10/16.
 * Copyright 2009 Caixa Mágica Software. All rights reserved.
 *
 * Fork developed by Vicente Ruiz Rodríguez
 * Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */
 
#ifndef __LIBEMQTT_H__
#define __LIBEMQTT_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MQTT_DUP_FLAG     1<<3
#define MQTT_QOS0_FLAG    0<<1
#define MQTT_QOS1_FLAG    1<<1
#define MQTT_QOS2_FLAG    2<<1

#define MQTT_RETAIN_FLAG  1

#define MQTT_CLEAN_SESSION  1<<1
#define MQTT_WILL_FLAG      1<<2
#define MQTT_WILL_RETAIN    1<<5
#define MQTT_USERNAME_FLAG  1<<7
#define MQTT_PASSWORD_FLAG  1<<6

#define MQTT_MSG_CONNECT       1<<4
#define MQTT_MSG_CONNACK       2<<4
#define MQTT_MSG_PUBLISH       3<<4
#define MQTT_MSG_PUBACK        4<<4
#define MQTT_MSG_PUBREC        5<<4
#define MQTT_MSG_PUBREL        6<<4
#define MQTT_MSG_PUBCOMP       7<<4
#define MQTT_MSG_SUBSCRIBE     8<<4
#define MQTT_MSG_SUBACK        9<<4
#define MQTT_MSG_UNSUBSCRIBE  10<<4
#define MQTT_MSG_UNSUBACK     11<<4
#define MQTT_MSG_PINGREQ      12<<4
#define MQTT_MSG_PINGRESP     13<<4
#define MQTT_MSG_DISCONNECT   14<<4
#define MQTT_MSG_SUBACK_VALUE  	 50

typedef struct {
	void* socket_info;
	int (*send)(void* socket_info, const void* buf, unsigned int count);
	// Connection info
	char clientid[50];
	// Auth fields
	char username[31];
	char password[2];
	// Will topic
	uint8_t will_retain;
	uint8_t will_qos;
	uint8_t clean_session;
	// Management fields
	uint16_t seq;
	uint16_t alive;
} mqtt_broker_handle_t;

/** Initialize the information to connect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 * @param clientid A string that identifies the client id.
 *
 * @note Only has effect before to call mqtt_connect
 */
void mqtt_init(mqtt_broker_handle_t* broker, const char* clientid);

/** Enable the authentication to connect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 * @param username A string that contains the username.
 * @param password A string that contains the password.
 *
 * @note Only has effect before to call mqtt_connect
 */
void mqtt_init_auth(mqtt_broker_handle_t* broker, const char* username, const char* password);

/** Set the keep alive timer.
 * @param broker Data structure that contains the connection information with the broker.
 * @param alive Keep aliver timer value (in seconds).
 *
 * @note Only has effect before to call mqtt_connect
 */
void mqtt_set_alive(mqtt_broker_handle_t* broker, uint16_t alive);

/** Connect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_connect(mqtt_broker_handle_t* broker);

/** Disconnect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 *
 * @note The socket must also be closed.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_disconnect(mqtt_broker_handle_t* broker);

/** Publish a message on a topic.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 * @param msg The message.
 * @param retain Enable or disable the Retain flag (values: 0 or 1).
 * @param qos Quality of Service (values: 0, 1 or 2)
 * @param message_id Variable that will store the Message ID, if the pointer is not NULL.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t qos,uint8_t retain);

/** Subscribe to a topic.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 * @param message_id Variable that will store the Message ID, if the pointer is not NULL.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_subscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id);

/** Unsubscribe from a topic.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 * @param message_id Variable that will store the Message ID, if the pointer is not NULL.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_unsubscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id);

/** Make a ping.
 * @param broker Data structure that contains the connection information with the broker.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_ping(mqtt_broker_handle_t* broker);

#endif // __LIBEMQTT_H__
/** End Of File **/
