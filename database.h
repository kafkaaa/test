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
#ifndef __DATABASE_H__
#define __DATABASE_H__

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "mqtt.h"
#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- Database --- */
extern MYSQL *connection;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
/* @brief */
void finish_with_error();

/* @brief */
void dtb_init();

/* @brief */
void dtb_query(char* query);

/* @brief */
void dtb_showResponse();

/* @brief */
char* dtb_getResponse();

/* @brief */
void dtb_checkDevAddr(uint8_t *packet);

/* @brief */
int dtb_checkDevExist(uint8_t *packet);

/* @brief */
char* dtb_getHighestID();

/* @brief */
void dtb_getDevAddr(uint8_t *buff,char *received);

/* @brief */
char* dtb_getDevID(uint8_t *packet);

/* @brief */
void dtb_checkDevStatus();

/* @brief */
void dtb_checkJoinedDevice();

/* @brief */
void dtb_updateStatus(char* id,int status);

#endif
/** End Of File **/
