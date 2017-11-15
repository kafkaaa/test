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
#include "mqtt.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */
/* MQTT client id */
#define		CLIENT_ID			"LoRaGateway"
/* MQTT server's hostname */
#define		SERVER_HOSTNAME		"things.ubidots.com"
/* MQTT broker address */
#define		SERVER_ADDRESS 		"50.23.124.68"
/* MQTT broker port */
#define    	SERVER_PORT			1883
/* MQTT client name */
#define 	USER_NAME			"vjioaDOjYPv78qNu9LvMn6nAUzr1xq"
/* MQTT password */
#define 	PASSWORD			""
/* KEEP_ALIVE Time */
#define 	KEEP_ALIVE			30
/* MAX_PACKET_SIZE */
#define 	MAX_PACKET_SIZE		500

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* Connection to Database */
char recvbuff[MAX_PACKET_SIZE];
char SUBCRIBE_TOPIC[MAX_PACKET_SIZE];// "/v1.6/devices/Device/D1Led/lv"
char PUBLISH_TOPIC[MAX_PACKET_SIZE]; // "/v1.6/devices/Device1"
char sendbuff[MAX_PACKET_SIZE];
mqtt_broker_handle_t broker;
int socket_id;
char node_id[1], temp[1], humid[2], light[2], battery[1];
uint8_t buffer[6];
int SERVER_LOCK;
/* Measurement of TX and MQTT Parameters */
uint32_t meas_nb_mqtt_tx_ok = 0;
uint32_t meas_nb_mqtt_ping = 0;
uint32_t meas_nb_mqtt_connect = 0;
int meas_nb_timeout = 0;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */
/* @brief send PINGREQ to server */
int catch_signal(int sig,void (*handler) (int)) {
	struct sigaction action;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	return sigaction(sig, &action, NULL);
}

/* @brief */
void alive(int sig) {
	int i, devStatus, highestID;
	char local_id_alive[2];
	char* response;
	
	++meas_nb_timeout;
	if(meas_nb_timeout == 10){ // after 5 minutes
		//~ piLock(DB_LOCK);
		response = db_getHighestID();
		highestID = response[0] - '0';
		
		for(i = 1; i <= highestID; i++){
			sprintf(local_id_alive,"%d",i);
			devStatus = db_getDevStatus(local_id_alive);
			if(devStatus == 1){ // Since joined, there is no packet received => delete from network.
				db_updateStatus(local_id_alive, 0);
			}
			else if (devStatus > 1){ // Reset status to the one after joined.
				db_updateStatus(local_id_alive, 1);
			}
		}			
		
		meas_nb_timeout = 0;
		//~ piUnlock(DB_LOCK);
	}
	
	if(WIFI_STATUS == true){
		printf("Timeout! Sending ping...\n");
		printf("-----------\n");
		printf("nb_mqtt_ping: %u\n",++meas_nb_mqtt_ping);
		printf("-----------\n");		
		mqtt_ping(&broker);
	
		if(meas_nb_mqtt_ping == 10){ // after 5 minutes
			//~ piLock(SERVER_LOCK);
								
			printf("Disconnected from Server!\n");
			printf("-----------\n");
			/* DISCONNECT */
			mqtt_disconnect(&broker);
			close_socket(&broker);
			
			printf("Reconnecting to Server...\n");
			printf("-----------\n");
			
			/* Init MQTT */
			mqtt_init(&broker, CLIENT_ID);
			mqtt_init_auth(&broker, USER_NAME, PASSWORD);
			init_socket(&broker, SERVER_ADDRESS, SERVER_PORT, KEEP_ALIVE);

			/* RECONNECT */
			if((mqtt_connect(&broker)) < 0){
				error("Cannot connect to server!");
			}
			
			/* Resubscribe */
			db_checkJoinedDevice();
			
			//~ piUnlock(SERVER_LOCK);
			meas_nb_mqtt_connect++;
			meas_nb_mqtt_ping = 0;
		}
	}
	
	alarm(KEEP_ALIVE);
	
}

/* @brief */
void term(int sig) {
	//~ /* Delete device from database */
	//~ int i, highestID;
	//~ char local_id_delete[2];
	//~ char* response;
	//~ response = db_getHighestID();
	//~ highestID = response[0] - '0';
	
	//~ for(i = 1; i <= highestID; i++){
		//~ sprintf(local_id_delete,"%d",i);
		//~ db_updateStatus(local_id_delete,0);
	//~ }
	
	/* DISCONNECT */
	mysql_close(connection);
	mqtt_disconnect(&broker);
	close_socket(&broker);
	printf("\n");
	printf("Disconnected from Server!\n");
	printf("INFO: nb_tx_ok: %u, nb_rx_ok: %u \n", meas_nb_tx_ok, meas_nb_rx_ok);
	printf("INFO: nb_mqtt_tx_ok: %u, nb_mqtt_ping: %u, nb_mqtt_connect: %u\n", meas_nb_mqtt_tx_ok, meas_nb_mqtt_ping, meas_nb_mqtt_connect);
	exit(0);
}

/* @brief */
void error(char* msg) {
	printf("%s: %s\n",msg,strerror(errno));
	exit(1);
}

/* @brief */
int send_packet(void* socket_info, const void* buf, unsigned int count) {
	int fd = *((int*)socket_info);
	return send(fd, buf, count, 0);
}

/* @brief */
void init_socket(mqtt_broker_handle_t* broker, const char* hostname, short port, int keepalive) {
	int flag = 1;

	/* Create the socket */
	if((socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("Cannot create socket!");
		
	/* Disable Nagle Algorithm */
	if (setsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
		error("Cannot disable nagle algorithm!");
	
	/* Change default timeout */
	struct timeval timeout;
	timeout.tv_sec = 33;
	timeout.tv_usec = 0;

	/* Set received timeout in socket */
	if (setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
		error("Cannot setsockopt1!");
	/* Set send timeout in socket */
	if (setsockopt(socket_id, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
		error("Cannot setsockopt2!");

	struct sockaddr_in socket_address;
	memset(&socket_address, 0, sizeof(socket_address));
	/* Create the stuff we need to connect */
	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons(port);
	socket_address.sin_addr.s_addr = inet_addr(hostname);
 
	/* Connect the socket */
	if((connect(socket_id, (struct sockaddr*)&socket_address, sizeof(socket_address))) < 0)
		error("Cannot connect the socket!");

	/* MQTT stuffs */
	mqtt_set_alive(broker, keepalive);
	broker->socket_info = (void*)&socket_id;
	broker->send = send_packet;
	
	/* Set memory */
	memset(recvbuff, 0, MAX_PACKET_SIZE);
}

/* @brief */
int close_socket(mqtt_broker_handle_t* broker) {
	int fd = *((int*)broker->socket_info);
	return close(fd);
}

/* @brief */
void read_socket() {
	int length = 0;
	/* listen to Server */
	length = read(socket_id,recvbuff, MAX_PACKET_SIZE);
    if (length < 0){
         error("\nERROR");
	}
    else
	{	
		//~ /* Print Response from Server */
		//~ int i;
		//~ printf("Response:");
		//~ for(i=0;i<length;i++){
			//~ printf("%d ",recvbuff[i]);
		//~ }
		//~ printf("\n");
		
		/* Analyse Type Of Msg */
		switch(recvbuff[0]){
			case MQTT_MSG_CONNACK:
				if (recvbuff[length-1] == 0x00){
					printf("Connected to Server!\n");
					printf("-----------\n");	
				}
				else {printf("Connection Failure!\n");}
				break;
							
			case MQTT_MSG_SUBACK:
				if ((recvbuff[length-1] == 0x00)||(recvbuff[length-1] == 0x01)||(recvbuff[length-1] == 0x02)){
					printf("Subcription Success!\n");
					printf("-----------\n");	
				}
				else{
					printf("Subcribed Failure!\n");
					printf("-----------\n");
				}
				break;
				
			case MQTT_MSG_SUBACK_VALUE:
				db_getDevAddr(buffer,recvbuff);
				switch(recvbuff[length-1]){
					case 48: 
						printf(": TURN OFF\n");
						printf("-----------\n");
						buffer[5] = recvbuff[length-1] - '0';
						bf_importToServerBuffer(buffer);
						break;
					case 49: 
						printf(": TURN ON\n"); 
						printf("-----------\n");
						buffer[5] = recvbuff[length-1] - '0';		
						bf_importToServerBuffer(buffer);			
						break;
				}
				break;
				
			case MQTT_MSG_PINGRESP:
				printf("Server Accepted Ping!\n");printf("-----------\n");break;
					
			case MQTT_MSG_PUBACK:
				printf("Publication Success!\n"); printf("-----------\n");break;	
		}
		
		/* Clear memory */
		memset(recvbuff, 0, MAX_PACKET_SIZE);
	}
}

/* @brief */
void mqtt_publish(uint8_t* node_data){
	int i;
	char* response;
	response = db_getDevID(node_data);
							
	printf("Device %s Sent To Server: ",response);
	for(i=0;i<11;i++){
	printf("%d ",node_data[i]);
	}
	printf("\n");
	printf("-----------\n");
	meas_nb_mqtt_tx_ok++;
	
	//NodeID
	sprintf(node_id,"%s",response);
	
	/* Temperature data */
	if(node_data[5] <= 80){
		sprintf(temp,"%d",node_data[5]);
	}
	
	/* Humidity data */
	if(node_data[6] == 0x00)
	{
		int hum = node_data[7];
		if(hum <= 100){
			sprintf(humid,"%d",node_data[7]);
		}
	}
	else{
		int hum = (node_data[6]) * 10 + node_data[7];
		if(hum <= 100){
			sprintf(humid,"%d%d",node_data[6],node_data[7]);
		}
		
	}
	
	/* Light Intensity data */
	if(node_data[8] == 0x00){
		int lig = node_data[9];
		if(lig <= 1000){
			sprintf(light,"%d",node_data[9]);
		}
	}
	else{
		int lig = (node_data[8]) * 10 + node_data[9];
		if(lig <= 1000){
			sprintf(light,"%d%d",node_data[8],node_data[9]);
		}
	}

	/* Battery */
	if(node_data[10] <= 1){
		sprintf(battery,"%d",node_data[10]);
	}

	/* Make PUBLISH_TOPIC */
	strcpy(PUBLISH_TOPIC,"/v1.6/devices/Device");
	strcat(PUBLISH_TOPIC,node_id);
	
	/* Make Message */
	strcpy(sendbuff,"{\"Temperature\": ");
	strcat(sendbuff,temp);
	strcat(sendbuff,", \"Humidity\": ");
	strcat(sendbuff,humid);
	strcat(sendbuff,", \"Light\": ");
	strcat(sendbuff,light);
	strcat(sendbuff,", \"Battery\": ");
	strcat(sendbuff,battery);
	strcat(sendbuff,"}");
	//~ printf("%s\n %s\n",PUBLISH_TOPIC,sendbuff);
	
	if(WIFI_STATUS == true){
		//~ piLock(SERVER_LOCK);
		/* >>>>> PUBLISH */
		mqtt_publish_with_qos(&broker, PUBLISH_TOPIC, sendbuff, 1, 0);
		//~ piUnlock(SERVER_LOCK);
	}
	
}

/* @brief */
void mqtt_init_all(){
	/* Init */
	mqtt_init(&broker, CLIENT_ID);
	mqtt_init_auth(&broker, USER_NAME, PASSWORD);
	init_socket(&broker, SERVER_ADDRESS, SERVER_PORT, KEEP_ALIVE);

	/* >>>>> CONNECT */
	if((mqtt_connect(&broker)) < 0){
		error("Cannot connect to server!");
	}
	/* <<<<< CONNACK */
	read_socket(recvbuff);
	
	/* Signals after connect MQTT */
	catch_signal(SIGALRM, alive);  	//Set SIGALRM to alive()
	alarm(KEEP_ALIVE);				//Start the clock
	catch_signal(SIGINT, term);		//Set SIGTERM to term()
}

/* @brief */
void mqtt_makeNewDevice(char *id){
	/* Make PUBLISH_TOPIC */
	strcpy(PUBLISH_TOPIC,"/v1.6/devices/Device");
	strcat(PUBLISH_TOPIC,id);
	
	/* Make Message */
	strcpy(sendbuff,"{\"D");
	strcat(sendbuff,id);
	strcat(sendbuff,"Led\": 0}");
	//~ printf("%s\n %s\n",PUBLISH_TOPIC,sendbuff);
	
	/* >>>>> PUBLISH */
	printf("Making Control Device %s on Server!\n",id);
	printf("-----------\n");
	//~ piLock(SERVER_LOCK);
	mqtt_publish_with_qos(&broker, PUBLISH_TOPIC, sendbuff, 1, 0);
	//~ piUnlock(SERVER_LOCK);
}

/* @brief */
void mqtt_subscribeToDevice(char *id){
	//~ piLock(SERVER_LOCK);
	
	printf("Subcribing to Device %s on Server!\n",id);
	printf("-----------\n");
	/* Led */
	strcpy(SUBCRIBE_TOPIC,"/v1.6/devices/device");
	strcat(SUBCRIBE_TOPIC,id);
	strcat(SUBCRIBE_TOPIC,"/d");
	strcat(SUBCRIBE_TOPIC,id);
	strcat(SUBCRIBE_TOPIC,"led/lv");
	//~ printf("su:%s\n",SUBCRIBE_TOPIC);
	mqtt_subscribe(&broker, SUBCRIBE_TOPIC, 0);
	
	//~ piUnlock(SERVER_LOCK);
}
/** End Of File **/
