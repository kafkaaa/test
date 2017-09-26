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
#ifndef _SPI_GPIO_H_
#define _SPI_GPIO_H_

#include <stdint.h>
#include "board.h"
#include "wiringPi.h"
//~ /* SX1276 */
//~ /* Define port and pins */
//~ #define NSS		8
//~ #define SCK		11
//~ #define MOSI   	10
//~ #define MISO   	9
//~ #define RESET   22
//~ #define DI0    	27

/* RF96 */
/* Define port and pins */
#define NSS		8
#define SCK		11
#define MOSI   	10
#define MISO   	9
#define RESET   7
#define DI0    	25

/* Define state for pins */
#define NSS0	digitalWrite(NSS,LOW)
#define NSS1	digitalWrite(NSS,HIGH)
#define SCK0	digitalWrite(SCK,LOW)
#define SCK1	digitalWrite(SCK,HIGH)
#define MOSI0	digitalWrite(MOSI,LOW)
#define MOSI1	digitalWrite(MOSI,HIGH)
#define MISO0	digitalWrite(MISO,LOW)
#define MISO1	digitalWrite(MISO,HIGH)
#define RESET0  digitalWrite(RESET,LOW)
#define RESET1  digitalWrite(RESET,HIGH)

/* SPI functions */
void SPI_Init(void);
uint8_t SPI_RW(uint8_t Buff);
uint8_t SPI_Read(uint8_t reg);
uint8_t SPI_Write(uint8_t reg, uint8_t value);
void SPI_Read_Buf(uint8_t reg, uint8_t* pBuf, uint8_t uchars);
void SPI_Write_Buf(uint8_t reg, uint8_t* pBuf, char uchars);

#endif
/** End Of File **/
