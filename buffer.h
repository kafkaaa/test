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
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "sx1276.h"
#include "board.h"
#include "mqtt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
/* Thread Lock for sending packet */
extern int SX_SEND_LOCK;

/* Pi Buffer - For Future */
void insert_toPiBuffer(uint8_t* data);
void import_toPiBuffer(uint8_t* packet);
void export_fromPiBuffer();
/* Server Buffer */
void insert_toServerBuffer(uint8_t* data);
void import_toServerBuffer(uint8_t* packet);
void export_fromServerBuffer();
/* Gateway Functions */
void gw_Interupt(void);

#endif
/** End Of File **/
