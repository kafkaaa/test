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
#include "database.h"

/** LOCALHOST */
#define    	LOCALHOST	"localhost"
/** MYSQL_USERNAME */
#define		MYSQL_USERNAME	"root"
/** MYSQL_PASSWORD */
#define		MYSQL_PASSWORD 	"canhthang"
/** DATABASE_NAME */
#define    	DATABASE_NAME	"gateway"
/** TABLE_NAME */
#define    	TABLE_NAME		"device"

char query[200];
char local_id_up[2];
char local_id_down[2];

/**********************************************************
**Name:     finish_with_error
**Function: 
**Input:    
**Output:   
**********************************************************/
void finish_with_error()
{
	fprintf(stderr, "%s\n", mysql_error(connection));
	mysql_close(connection);
	//~ exit(1);        
}

/**********************************************************
**Name:     dtb_init
**Function: 
**Input:    
**Output:   
**********************************************************/
void dtb_init(){
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
	dtb_checkJoinedDevice();
}

/**********************************************************
**Name:     dtb_query
**Function: 
**Input:    
**Output:   
**********************************************************/
void dtb_query(char* query){
	if (mysql_query(connection, query)) //0 is success
	{
		finish_with_error(connection);
	}
}

/**********************************************************
**Name:     dtb_showResponse
**Function: 
**Input:    
**Output:   
**********************************************************/
void dtb_showResponse(){
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

/**********************************************************
**Name:     dtb_getResponse
**Function: 
**Input:    
**Output:   
**********************************************************/
char* dtb_getResponse(){
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

/**********************************************************
**Name:     dtb_checkDevAddr
**Function: 
**Input:    
**Output:   
**********************************************************/
void dtb_checkDevAddr(uint8_t *packet){
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
	dtb_query(query);
	/* Get Response */
	response = dtb_getResponse();
	
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
		dtb_query(query);
		/* Get Response */
		response = dtb_getResponse();
		sprintf(local_id_up,"%s",response);
		printf("Device is available with id = %s\n",local_id_up);
		printf("-----------\n");
		dtb_updateStatus(local_id_up,1);

		switch(packet[5]){							
			case 1:
				break;
			case 2: 
				/* Subcribe to control device */
				mqtt_subscribeToDevice(local_id_up);
				break;
		}
	}	
	/* If device does not exist */					
	else {
		//Assign new id for device
		int newid;
		response = dtb_getHighestID();
		newid = response[0] - '0';
		sprintf(local_id_up,"%d",newid + 1);

		/* Insert DevAddr into database */
		strcpy(query,"insert into device (id, b1, b2, b3, b4,type,status) values (");
		strcat(query,local_id_up);
		strcat(query,", ");
		strcat(query,b1);
		strcat(query,", ");
		strcat(query,b2);
		strcat(query,", ");
		strcat(query,b3);
		strcat(query,", 1,");

		switch(packet[5]){							
			case 1:
				strcat(query," 1,1);");
				//~ printf("%s\n",query);
				dtb_query(query);
				break;
			case 2: 
				strcat(query,", 2,1);");
				//~ printf("%s\n",query);
				dtb_query(query);
				/* Make device and variables on Server */
				mqtt_makeNewDevice(local_id_up);
				/* Subcribe to new control device */
				mqtt_subscribeToDevice(local_id_up);
				break;
		}
	}
}
/**********************************************************
**Name:     dtb_checkDevExist
**Function: 
**Input:    
**Output:   
**********************************************************/
int dtb_checkDevExist(uint8_t *packet){
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
	dtb_query(query);
	/* Get Response */
	response = dtb_getResponse();
	
	if(strcmp(response,"1") == 0){
		return 1;
	}						
	else{
		return 0;
	}
}

/**********************************************************
**Name:     dtb_getHighestID
**Function: 
**Input:    
**Output:   
**********************************************************/
char* dtb_getHighestID(){
	char *response;
	strcpy(query,"select id from device order by id desc limit 1;");
	/* Run Query */
	dtb_query(query);
	/* Get Response */
	response = dtb_getResponse();
	return response;
}

/**********************************************************
**Name:     dtb_getDevAddr
**Function: 
**Input:    
**Output:   
**********************************************************/
void dtb_getDevAddr(uint8_t *buff,char *received){
	char *response;
	char b[1];
	int i,j;
	//~ printf("26 %d", received[26]);
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
		dtb_query(query);
		/* Get Response */
		response = dtb_getResponse(connection);
		sscanf(response,"%d",&j);
		buff[i-1] = j;
	}
}

/**********************************************************
**Name:     dtb_getDevID
**Function: 
**Input:    
**Output:   
**********************************************************/
char* dtb_getDevID(uint8_t *packet){
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
	dtb_query(query);
	/* Get Response */
	response = dtb_getResponse(connection);
	return response;
	//~ sscanf(response,"%d",&id);
	//~ return id;
}

/**********************************************************
**Name:     dtb_checkDevStatus
**Function: check device's status to subcribe to control device on server
**Input:    
**Output:   
**********************************************************/
void dtb_checkDevStatus(){
	
}

/**********************************************************
**Name:     dtb_checkJoinedDevice
**Function: check device's status to subcribe to control device on server
**Input:    
**Output:   
**********************************************************/
void dtb_checkJoinedDevice(){
	char *response;
	int i,nb_device;
	
	/* Get number of device in database */
	strcpy(query,"select count(*) from device;");
	/* Run Query */
	dtb_query(query);
	/* Get Response */
	response = dtb_getResponse(connection);
	sscanf(response,"%d",&nb_device);
	
	/* Subcribe to control device if status = 1 */
	for(i = 1; i<= nb_device; i++){
		sprintf(local_id_up,"%d",i);
		strcpy(query,"select count(*) from device where id = ");
		strcat(query,local_id_up);
		strcat(query," and type = 2 and status = 1;");
		/* Run Query */
		dtb_query(query);
		/* Get Response */
		response = dtb_getResponse(connection);
		if(strcmp(response,"1") == 0){
			printf("Device %s joined network!\n",local_id_up);
			mqtt_subscribeToDevice(local_id_up);
			delay(5);
		}
	}
}

/**********************************************************
**Name:     dtb_updateStatus
**Function: change the status of device in LoRaWAN
**Input:    
**Output:   
**********************************************************/
void dtb_updateStatus(char* id,int status){
	char state[1];
	sprintf(state,"%d",status);
	
	/* Get number of device in database */
	strcpy(query,"update device set status = ");
	strcat(query, state);
	strcat(query," where id = ");
	strcat(query, id);
	/* Run Query */
	dtb_query(query);
}
/** End Of File **/
