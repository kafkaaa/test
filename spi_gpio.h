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
#ifndef _SPI_GPIO_H_
#define _SPI_GPIO_H_

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include <stdint.h>
#include "board.h"
#include "wiringPi.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */
#define NSS		23
#define SCK		7
#define MOSI   	24
#define MISO   	8
#define DI0    	25
#define RESET	10

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

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */
/* @brief */
void SPI_Init(void);

/* @brief */
uint8_t SPI_RW(uint8_t Buff);

/* @brief */
uint8_t SPI_Read(uint8_t reg);

/* @brief */
uint8_t SPI_Write(uint8_t reg, uint8_t value);

/* @brief */
void SPI_Read_Buf(uint8_t reg, uint8_t* pBuf, uint8_t uchars);

/* @brief */
void SPI_Write_Buf(uint8_t reg, uint8_t* pBuf, char uchars);

#endif
/** End Of File **/
