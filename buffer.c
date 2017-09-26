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
#include "buffer.h"

//Define for Pi Buffer
#define MAX_PI_BUFFER_SIZE 55
#define MAX_PI_PACKET_SIZE 11
#define isFullP packetCountP == MAX_PI_BUFFER_SIZE/MAX_PI_PACKET_SIZE
uint8_t piBuffer[MAX_PI_BUFFER_SIZE];
uint8_t dataExportedP[MAX_PI_PACKET_SIZE];
int frontP = 0, rearP = -1, packetCountP = 0, newPacketP = 0;

//Define for Server Buffer
#define MAX_SERVER_BUFFER_SIZE 30
#define MAX_SERVER_PACKET_SIZE 6
#define isFullS packetCountS == MAX_SERVER_BUFFER_SIZE/MAX_SERVER_PACKET_SIZE
uint8_t serverBuffer[MAX_SERVER_BUFFER_SIZE];
uint8_t dataExportedS[MAX_SERVER_PACKET_SIZE];
int frontS = 0,rearS = -1, packetCountS = 0, newPacketS = 0;

//General Variables
int i;
int PiBuffer_LOCK, ServerBuffer_LOCK;

/**********************************************************
**Name:     insert_toPiBuffer
**Function: 
**Input:    None
**Output:   None
**********************************************************/
void insert_toPiBuffer(uint8_t* data){
	for(i=0;i<MAX_PI_PACKET_SIZE;i++){
		piBuffer[++rearP] = data[i];
	}
}

/**********************************************************
**Name:     import_toPiBuffer
**Function: 
**Input:    None
**Output:   None
**********************************************************/
void import_toPiBuffer(uint8_t* packet){
	if(isFullP){
		rearP = -1;
		packetCountP = 0;
	}
	insert_toPiBuffer(packet);
	
	piLock(PiBuffer_LOCK);
	packetCountP++;
	newPacketP++;
	piUnlock(PiBuffer_LOCK);
	//~ printf("packetCount: %d,newPacket: %d,rear: %d,front: %d\n",packetCount,newPacket,rear,front);
	//~ printf("PiBuffer: %s \n",piBuffer);
}

/**********************************************************
**Name:     export_fromPiBuffer
**Function: 
**Input:    None
**Output:   None
**********************************************************/
void export_fromPiBuffer(){
	if(newPacketP > 0){
		for(i=0;i<MAX_PI_PACKET_SIZE;i++){
		dataExportedP[i] = piBuffer[frontP++];
		}
		if(isFullP){
			frontP = 0;
		}
		//~ printf("Pi Data Exported: %s\n",dataExportedP);
		mqtt_publish(dataExportedP);
		
		piLock(PiBuffer_LOCK);
		newPacketP--;
		piUnlock(PiBuffer_LOCK);
	}
}

/**********************************************************
**Name:     insert_toServerBuffer
**Function: 
**Input:    None
**Output:   None
**********************************************************/
void insert_toServerBuffer(uint8_t* data){
	for(i=0;i<MAX_SERVER_PACKET_SIZE;i++){
		serverBuffer[++rearS] = data[i];
	}
}
/**********************************************************
**Name:     import_toServerBuffer
**Function: 
**Input:    None
**Output:   None
**********************************************************/
void import_toServerBuffer(uint8_t* packet){
	if(isFullS){
		rearS = -1;
		packetCountS = 0;
	}
	insert_toServerBuffer(packet);
	
	piLock(ServerBuffer_LOCK);
	packetCountS++;
	newPacketS++;
	piUnlock(ServerBuffer_LOCK);
	//~ printf("packetCountS: %d,newPacketS: %d,rearS: %d,frontS: %d\n",packetCountS,newPacketS,rearS,frontS);
	//~ printf("ServerBuffer:%s\n",serverBuffer);
}

/**********************************************************
**Name:     export_fromServerBuffer
**Function: 
**Input:    None
**Output:   None
**********************************************************/
void export_fromServerBuffer(){
	if(newPacketS > 0){
		for(i=0;i<MAX_SERVER_PACKET_SIZE;i++){
		dataExportedS[i] = serverBuffer[frontS++];
		}
		if(isFullS){
			frontS = 0;
		}
		
		//~ printf("Server Data Exported:");
		//~ for(i = 0;i<MAX_SERVER_PACKET_SIZE;i++){
			//~ printf(" %d",dataExportedS[i]);
		//~ }
		//~ printf("\n");
		
		piLock(SX_SEND_LOCK);
		Send(dataType_DataConfirmDown, dataExportedS, 6);
		piUnlock(SX_SEND_LOCK);
		
		piLock(ServerBuffer_LOCK);
		newPacketS--;
		piUnlock(ServerBuffer_LOCK);
	}
}

/**********************************************************
**Name:     gw_Interupt
**Function: Do interupt actions when receive packet
**Input:    None
**Output:   None
**********************************************************/
void gw_Interupt(void)
{
	if(digitalRead(DI0))
	{
		SX1276OnDio0Irq();
	}
}
/** End Of File **/
