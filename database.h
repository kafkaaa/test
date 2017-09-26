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
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "mqtt.h"
#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdint.h>

/* Database */
extern MYSQL *connection;

/* Database functions */
void finish_with_error();
void dtb_init();
void dtb_query(char* query);
void dtb_showResponse();
char* dtb_getResponse();
void dtb_checkDevAddr(uint8_t *packet);
int dtb_checkDevExist(uint8_t *packet);
char* dtb_getHighestID();
void dtb_getDevAddr(uint8_t *buff,char *received);
char* dtb_getDevID(uint8_t *packet);
void dtb_checkDevStatus();
void dtb_checkJoinedDevice();
void dtb_updateStatus(char* id,int status);
#endif
/** End Of File **/
