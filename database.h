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
#include "board.h"
#include "mqtt.h"
#include <wiringPi.h>
#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- Database --- */
MYSQL *connection;
int DB_LOCK;
/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
/* @brief */
void finish_with_error();

/* @brief */
void db_init();

/* @brief */
void db_query(char* query);

/* @brief */
void db_showResponse();

/* @brief */
char* db_getResponse();

/* @brief */
void db_checkDevAddr(uint8_t *packet);

/* @brief */
int db_checkDevExist(uint8_t *packet);

/* @brief */
char* db_getHighestID();

/* @brief */
void db_getDevAddr(uint8_t *buff,char *received);

/* @brief */
char* db_getDevID(uint8_t *packet);

/* @brief */
void db_checkJoinedDevice();

/* @brief */
int db_getDevStatus(char* id);

/* @brief */
void db_updateStatus(char* id,int status);

#endif
/** End Of File **/
