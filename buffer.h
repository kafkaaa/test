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
#ifndef _BUFFER_H_
#define _BUFFER_H_

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "sx1276.h"
#include "board.h"
#include "mqtt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
int SX_SEND_LOCK;
/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */

/* --- Pi Buffer --- */

/* @brief insert data function */
void bf_insertToPiBuffer(uint8_t* data);

/* @brief import packet to Pi Buffer */
void bf_importToPiBuffer(uint8_t* packet);

/* @brief export packet from Pi Buffer */
void bf_exportFromPiBuffer();

/* --- Server Buffer --- */
/* @brief */
void bf_insertToServerBuffer(uint8_t* data);
/* @brief */
void bf_importToServerBuffer(uint8_t* packet);
/* @brief export packet from Server Buffer */
void bf_exportFromServerBuffer();

/* --- Gateway Functions --- */
/* @brief interupt function for receiving packet */
void lgw_interupt(void);

#endif
/** End Of File **/
