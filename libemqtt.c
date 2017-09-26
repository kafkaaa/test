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
#include "libemqtt.h"

void mqtt_init(mqtt_broker_handle_t* broker, const char* clientid) {
	// Connection options
	broker->alive = 300;
	broker->seq = 1; // Sequency for message indetifiers
	// Client options
	memset(broker->clientid, 0, sizeof(broker->clientid));
	memset(broker->username, 0, sizeof(broker->username));
	memset(broker->password, 0, sizeof(broker->password));
	if(clientid) {
		strncpy(broker->clientid, clientid, sizeof(broker->clientid));
	} else {
		strcpy(broker->clientid, "loragateway");
	}
	// Will topic
	broker->clean_session = 1;
}

void mqtt_init_auth(mqtt_broker_handle_t* broker, const char* username, const char* password) {
	if(username && username[0] != '\0')
		strncpy(broker->username, username, sizeof(broker->username)-1);
	if(password && password[0] != '\0')
		strncpy(broker->password, password, sizeof(broker->password)-1);
}

void mqtt_set_alive(mqtt_broker_handle_t* broker, uint16_t alive) {
	broker->alive = alive;
}

int mqtt_connect(mqtt_broker_handle_t* broker)
{
	uint16_t flags = 0x00;

	uint16_t clientidlen = strlen(broker->clientid);
	uint16_t usernamelen = strlen(broker->username);
	uint16_t passwordlen = strlen(broker->password);
	uint16_t payload_len = clientidlen + 2;

	// Preparing the flags
	if(usernamelen) {
		payload_len += usernamelen + 2;
		flags |= MQTT_USERNAME_FLAG;
	}
	if(!passwordlen) {
		payload_len += passwordlen + 2;
		flags |= MQTT_PASSWORD_FLAG;
	}
	if(broker->clean_session) {
		flags |= MQTT_CLEAN_SESSION;
	}
	
	// Variable header
	char var_header[] = {
		0x00,0x04,0x4d,0x51,0x54,0x54, // Protocol name: MQTT
		0x04, // Protocol version
		flags, // Connect flags
		broker->alive>>8, broker->alive&0xFF, // Keep alive
	};
	
   	// Fixed header
    uint8_t fixedHeaderSize = 2;    // Default size = one byte Message Type + one byte Remaining Length
    uint8_t remainLen = sizeof(var_header)+payload_len;
    //~ printf("remainLen: %d\n",remainLen);
    if (remainLen > 127) {
        fixedHeaderSize++;          // add an additional byte for Remaining Length
    }
    uint8_t fixed_header[fixedHeaderSize];
    // Message Type
    fixed_header[0] = MQTT_MSG_CONNECT;
		
    // Remaining Length
    if (remainLen <= 127) {
        fixed_header[1] = remainLen;
    } else {
        // first byte is remainder (mod) of 128, then set the MSB to indicate more bytes
        fixed_header[1] = remainLen % 128;
        fixed_header[1] = fixed_header[1] | 0x80;
        // second byte is number of 128s
        fixed_header[2] = remainLen / 128;
    }

	uint16_t offset = 0;
	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+payload_len];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	offset += sizeof(fixed_header);
	memcpy(packet+offset, var_header, sizeof(var_header));
	offset += sizeof(var_header);
	// Client ID - UTF encoded
	packet[offset++] = clientidlen>>8;
	packet[offset++] = clientidlen&0xFF;
	memcpy(packet+offset, broker->clientid, clientidlen);
	offset += clientidlen;
	
	if(usernamelen) {
		// Username - UTF encoded
		packet[offset++] = usernamelen>>8;
		packet[offset++] = usernamelen&0xFF;
		memcpy(packet+offset, broker->username, usernamelen);
		offset += usernamelen;
	}
	
	if(!passwordlen) {
		// Password - UTF encoded
		packet[offset++] = passwordlen>>8;
		packet[offset++] = passwordlen&0xFF;
		memcpy(packet+offset, broker->password, passwordlen);
	}

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}
	
	return 1;
}

int mqtt_disconnect(mqtt_broker_handle_t* broker) {
	uint8_t packet[] = {
		MQTT_MSG_DISCONNECT, // Message Type, DUP flag, QoS level, Retain
		0x00 // Remaining length
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_ping(mqtt_broker_handle_t* broker) {
	uint8_t packet[] = {
		MQTT_MSG_PINGREQ, 	// Message Type, DUP flag, QoS level, Retain
		0x00 				// Remaining length
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t qos,uint8_t retain) {
	uint16_t topiclen = strlen(topic);
	uint16_t msglen = strlen(msg);

	uint8_t qos_flag = MQTT_QOS0_FLAG;
	uint8_t qos_size = 0; // No QoS included
	if(qos == 1) {
		qos_size = 2; // 2 bytes for QoS
		qos_flag = MQTT_QOS1_FLAG;
	}
	else if(qos == 2) {
		qos_size = 2; // 2 bytes for QoS
		qos_flag = MQTT_QOS2_FLAG;
	}

	// Variable header
	uint8_t var_header[topiclen+2+qos_size]; // Topic size (2 bytes), utf-encoded topic
	memset(var_header, 0, sizeof(var_header));
	var_header[0] = topiclen>>8;
	var_header[1] = topiclen&0xFF;
	memcpy(var_header+2, topic, topiclen);
	if(qos_size) {
		var_header[topiclen+2] = 0;
		var_header[topiclen+3] = 10;
		//~ if(message_id) { // Returning message id
			//~ *message_id = broker->seq;
		//~ }
		//~ broker->seq++;
	}

	// Fixed header
	// the remaining length is one byte for messages up to 127 bytes, then two bytes after that
	// actually, it can be up to 4 bytes but I'm making the assumption the embedded device will only
	// need up to two bytes of length (handles up to 16,383 (almost 16k) sized message)
	uint8_t fixedHeaderSize = 2;    // Default size = one byte Message Type + one byte Remaining Length
	uint16_t remainLen = sizeof(var_header)+msglen;
	if (remainLen > 127) {
		fixedHeaderSize++;          // add an additional byte for Remaining Length
	}
	uint8_t fixed_header[fixedHeaderSize];
    
    // Message Type, DUP flag, QoS level, Retain
    fixed_header[0] = MQTT_MSG_PUBLISH | qos_flag;
	if(retain) {
		fixed_header[0] |= MQTT_RETAIN_FLAG;
    }
    // Remaining Length
    if (remainLen <= 127) {
        fixed_header[1] = remainLen;
    } else {
       // first byte is remainder (mod) of 128, then set the MSB to indicate more bytes
       fixed_header[1] = remainLen % 128;
       fixed_header[1] = fixed_header[1] | 0x80;
       // second byte is number of 128s
       fixed_header[2] = remainLen / 128;
    }

	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+msglen];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), msg, msglen);

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	//~ int i;
	//~ printf("Publish packet:");
		//~ for(i=0;i<sizeof(packet);i++){
			//~ printf("%d",packet[i]);
		//~ }
		//~ printf("\n");
	return 1;
}

int mqtt_subscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id) {
	uint16_t topiclen = strlen(topic);

	// Variable header
	uint8_t var_header[2]; // Message ID
	//~ var_header[0] = broker->seq>>8;
	//~ var_header[1] = broker->seq&0xFF;
	var_header[0] = 0x00;
	var_header[1] = 0x0A;
	if(message_id) { // Returning message id
		*message_id = broker->seq;
	}
	broker->seq++;

	// utf topic
	uint8_t utf_topic[topiclen+3]; // Topic size (2 bytes), utf-encoded topic, QoS byte
	memset(utf_topic, 0, sizeof(utf_topic));
	utf_topic[0] = topiclen>>8;
	utf_topic[1] = topiclen&0xFF;
	memcpy(utf_topic+2, topic, topiclen);
	utf_topic[topiclen+2] = 0x01;
	
	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_SUBSCRIBE | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+sizeof(utf_topic)
	};

	//~ printf("fixed_header packet:");
		//~ for(i=0;i<sizeof(fixed_header);i++){
			//~ printf("%d",fixed_header[i]);
		//~ }
		//~ printf("\n");
		
	uint8_t packet[sizeof(var_header)+sizeof(fixed_header)+sizeof(utf_topic)];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, sizeof(utf_topic));

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_unsubscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id) {
	uint16_t topiclen = strlen(topic);

	// Variable header
	uint8_t var_header[2]; // Message ID
	var_header[0] = broker->seq>>8;
	var_header[1] = broker->seq&0xFF;
	if(message_id) { // Returning message id
		*message_id = broker->seq;
	}
	broker->seq++;

	// utf topic
	uint8_t utf_topic[topiclen+2]; // Topic size (2 bytes), utf-encoded topic
	memset(utf_topic, 0, sizeof(utf_topic));
	utf_topic[0] = topiclen>>8;
	utf_topic[1] = topiclen&0xFF;
	memcpy(utf_topic+2, topic, topiclen);

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_UNSUBSCRIBE | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+sizeof(utf_topic)
	};

	uint8_t packet[sizeof(var_header)+sizeof(fixed_header)+sizeof(utf_topic)];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, sizeof(utf_topic));

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}
/** End Of File **/
