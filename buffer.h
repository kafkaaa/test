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
/* --- Thread Lock for sending packet --- */
extern int SX_SEND_LOCK;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
/* --- Pi Buffer - For Future --- */

/*
 * @brief insert data function
 */
void insert_toPiBuffer(uint8_t* data);
/*
 * @brief import packet to Pi Buffer
 */
void import_toPiBuffer(uint8_t* packet);
/*
 * @brief export packet from Pi Buffer
 */
void export_fromPiBuffer();

/* --- Server Buffer --- */
/*
 * @brief
 */
void insert_toServerBuffer(uint8_t* data);
/*
 * @brief
 */
void import_toServerBuffer(uint8_t* packet);
/*
 * @brief export packet from Server Buffer
 */
void export_fromServerBuffer();

/* --- Gateway Functions --- */
/*
 * @brief interupt function for receiving packet
 */
void gw_Interupt(void);

#endif
/** End Of File **/
