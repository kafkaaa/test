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
#include "database.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */
/* LOCALHOST */
#define    	LOCALHOST	"localhost"
/* MYSQL_USERNAME */
#define		MYSQL_USERNAME	"root"
/* MYSQL_PASSWORD */
#define		MYSQL_PASSWORD 	"canhthang"
/* DATABASE_NAME */
#define    	DATABASE_NAME	"gateway"
/* TABLE_NAME */
#define    	TABLE_NAME		"device"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
char query[200];
char local_id_up[2];
char local_id_down[2];

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */
/* @brief */
void finish_with_error() {
	fprintf(stderr, "%s\n", mysql_error(connection));
	mysql_close(connection);
	//~ exit(1);        
}

/* @brief */
void db_init(){
	/* Database */
	connection = mysql_init(NULL);
	if (connection == NULL)
	{
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  
	  
	if (mysql_real_connect(connection, LOCALHOST, MYSQL_USERNAME, MYSQL_PASSWORD, DATABASE_NAME, 0, NULL, 0) == NULL) 
	{
		finish_with_error(connection);
	}
	db_checkJoinedDevice();
}

/* @brief */
void db_query(char* query){
	if (mysql_query(connection, query)) //0 is success
	{
		finish_with_error(connection);
	}
}

/* @brief */
void db_showResponse(){
	MYSQL_RES *result = mysql_store_result(connection);
	int i;
	if (result == NULL) 
	{
	finish_with_error(connection);
	}

	int num_fields = mysql_num_fields(result);
	
	MYSQL_ROW row;
	  
	while ((row = mysql_fetch_row(result))) 
	{ 
		for(i = 0; i < num_fields; i++) 
		{
		printf("%s ", row[i] ? row[i] : "NULL"); 
		} 
		printf("\n"); 
	}
	  
	mysql_free_result(result);
}

/* @brief */
char* db_getResponse(){
	char *response;
	MYSQL_RES *result = mysql_store_result(connection);
	  
	if (result == NULL) 
	{
	finish_with_error(connection);
	}

	int num_fields = mysql_num_fields(result);
	if(num_fields == 1){
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		response = row[0];
	}
	return response;
	mysql_free_result(result);
}

/* @brief */
void db_checkDevAddr(uint8_t *packet){
	char* response;
	char b1[1],b2[1],b3[1];
	sprintf(b1,"%d",packet[2]);
	sprintf(b2,"%d",packet[3]);
	sprintf(b3,"%d",packet[4]);

	strcpy(query,"select count(*) from device where (b1 = ");
	strcat(query,b1);
	strcat(query,") and (b2 = ");
	strcat(query,b2);
	strcat(query,") and (b3 = ");
	strcat(query,b3);
	strcat(query,");");
	
	//~ printf("%s\n",query);
	
	/* Run Query */
	db_query(query);
	/* Get Response */
	response = db_getResponse();
	
	/* If device exist */
	if(strcmp(response,"1") == 0){
		strcpy(query,"select id from device where (b1 = ");
		strcat(query,b1);
		strcat(query,") and (b2 = ");
		strcat(query,b2);
		strcat(query,") and (b3 = ");
		strcat(query,b3);
		strcat(query,");");
		//~ printf("%s\n",query);	
		
		/* Run Query */
		db_query(query);
		/* Get Response */
		response = db_getResponse();
		sprintf(local_id_up,"%s",response);
		printf("Device is available with id = %s\n",local_id_up);
		printf("-----------\n");
		db_updateStatus(local_id_up,1);

		switch(packet[5]){							
			case 1:
				break;
			case 2: 
				if(WIFI_STATUS == true){
					/* Subcribe to control device */
					mqtt_subscribeToDevice(local_id_up);
					break;
				}
		}
	}	
	/* If device does not exist */					
	else {
		//Assign new id for device
		int newid;
		response = db_getHighestID();
		newid = response[0] - '0';
		sprintf(local_id_up,"%d",newid + 1);

		/* Insert DevAddr into database */
		strcpy(query,"insert into device (id, b1, b2, b3, b4,type,status) values (");
		strcat(query,local_id_up);
		strcat(query,",");
		strcat(query,b1);
		strcat(query,",");
		strcat(query,b2);
		strcat(query,",");
		strcat(query,b3);
		strcat(query,",1,");

		switch(packet[5]){							
			case 1:
				strcat(query,"1,1);");
				//~ printf("%s\n",query);
				db_query(query);
				break;
			case 2: 
				strcat(query,"2,1);");
				//~ printf("%s\n",query);
				db_query(query);
				if(WIFI_STATUS == true){
					/* Make device and variables on Server */
					mqtt_makeNewDevice(local_id_up);
					delay(1);
					/* Subcribe to new control device */
					mqtt_subscribeToDevice(local_id_up);
				}
				break;
		}
	}
}

/* @brief */
int db_checkDevExist(uint8_t *packet){
	char* response;
	char b1[1],b2[1],b3[1];
	sprintf(b1,"%d",packet[1]);
	sprintf(b2,"%d",packet[2]);
	sprintf(b3,"%d",packet[3]);

	strcpy(query,"select count(*) from device where (b1 = ");
	strcat(query,b1);
	strcat(query,") and (b2 = ");
	strcat(query,b2);
	strcat(query,") and (b3 = ");
	strcat(query,b3);
	strcat(query,");");
	//~ printf("%s\n",query);
	
	/* Run Query */
	db_query(query);
	/* Get Response */
	response = db_getResponse();
	
	if(strcmp(response,"1") == 0){
		return 1;
	}						
	else{
		return 0;
	}
}

/* @brief */
char* db_getHighestID(){
	char *response;
	strcpy(query,"select id from device order by id desc limit 1;");
	/* Run Query */
	db_query(query);
	/* Get Response */
	response = db_getResponse();
	return response;
}

/* @brief */
void db_getDevAddr(uint8_t *buff,char *received){
	char *response;
	char b[1];
	int i,j;
	//~ printf("26 %d", received[26]);
	/* This code is suitable for 0 - 99 devices */
	/* If more than 9 devices */
	if(received[25] != 47){
		local_id_down[0] = received[28];
		local_id_down[1] = received[29];
		printf("Device %s",local_id_down);
		switch(received[30]){
		case 76: 
			printf(" Led");
			buff[4] = 2;
			break;
		case 80: 
			printf(" Pump"); 
			buff[4] = 3;		
			break;
		}	
	}
	/* If less than 9 devices */
	else{
		local_id_down[0] = received[27];
		printf("Device %s",local_id_down);
		switch(received[28]){
		case 76: 
			printf(" Led");
			buff[4] = 2;
			break;
		case 80: 
			printf(" Pump"); 
			buff[4] = 3;		
			break;
		}	
	}
	for(i = 1; i < 5; i++){
		sprintf(b,"%d",i);
		strcpy(query,"select b");
		strcat(query,b);
		strcat(query," from device where id = ");
		strcat(query,local_id_down);
		//~ printf("%s\n",query);
		
		/* Run Query */
		db_query(query);
		/* Get Response */
		response = db_getResponse(connection);
		sscanf(response,"%d",&j);
		buff[i-1] = j;
	}
}

/* @brief */
char* db_getDevID(uint8_t *packet){
	//~ int id;
	char *response;
	char b1[1],b2[1],b3[1];
	sprintf(b1,"%d",packet[1]);
	sprintf(b2,"%d",packet[2]);
	sprintf(b3,"%d",packet[3]);

	strcpy(query,"select id from device where (b1 = ");
	strcat(query,b1);
	strcat(query,") and (b2 = ");
	strcat(query,b2);
	strcat(query,") and (b3 = ");
	strcat(query,b3);
	strcat(query,");");
	//~ printf("%s\n",query);
	
	/* Run Query */
	db_query(query);
	/* Get Response */
	response = db_getResponse(connection);
	return response;
	//~ sscanf(response,"%d",&id);
	//~ return id;
}

/* @brief */
void db_checkJoinedDevice(){
	char *response;
	int i,nb_device;
	
	/* Get number of device in database */
	strcpy(query,"select count(*) from device;");
	/* Run Query */
	db_query(query);
	/* Get Response */
	response = db_getResponse(connection);
	sscanf(response,"%d",&nb_device);
	
	/* Subcribe to control device if status = 1 */
	for(i = 1; i<= nb_device; i++){
		sprintf(local_id_up,"%d",i);
		strcpy(query,"select count(*) from device where id = ");
		strcat(query,local_id_up);
		strcat(query," and type = 2 and status = 1;");
		/* Run Query */
		db_query(query);
		/* Get Response */
		response = db_getResponse(connection);
		if(strcmp(response,"1") == 0){
			printf("Device %s joined network!\n",local_id_up);
			if(WIFI_STATUS == true){
				mqtt_subscribeToDevice(local_id_up);
				delay(5);
			}
		}
	}
}

/* @brief */
int db_getDevStatus(char* id){
	int status;
	char *response;
	
	strcpy(query,"select status from device where id =");
	strcat(query, id);
	/* Run Query */
	db_query(query);
	/* Get Response */
	response = db_getResponse();
	
	status = response[0] - '0';
	return status;
}

/* @brief */
void db_updateStatus(char* id,int status){
	char state[1];
	sprintf(state,"%d",status);
	
	/* Get number of device in database */
	strcpy(query,"update device set status = ");
	strcat(query, state);
	strcat(query," where id = ");
	strcat(query, id);
	/* Run Query */
	db_query(query);
}
/** End Of File **/
