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

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "spi_gpio.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
unsigned char TxBuf[32]="";

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */
/* @brief */
void SPI_Init() {
	wiringPiSetupGpio();
	pinMode(NSS,OUTPUT);
	pinMode(SCK,OUTPUT);
	pinMode(MOSI,OUTPUT);
	pinMode(MISO,INPUT);
	pullUpDnControl (MISO,PUD_UP);
	pinMode(RESET,OUTPUT);
	pinMode(DI0,INPUT);
	pullUpDnControl (DI0,PUD_UP);
}

/* @brief */
uint8_t SPI_RW(uint8_t Buff) {
	uint8_t bit_ctr;
   	for(bit_ctr=0;bit_ctr<8;bit_ctr++) // output 8-bit
   	{
		if((Buff&0x80)==0x80) MOSI1;
		else MOSI0;
        delayMicroseconds(5);
		Buff = (Buff << 1);           // shift next bit into MSB..
		SCK1;                      // Set SCK high..
        delayMicroseconds(5);
		if(digitalRead(MISO)) Buff |= 0x01;
		SCK0;            		  // ..then set SCK low again
   	}
    return(Buff);           		  // return read uchar
}

/* @brief */
uint8_t SPI_Read(uint8_t reg) {
	uint8_t reg_val;

	NSS0;   	            // NSS low, initialize SPI communication...
	SPI_RW(reg&0x7F);            // Select register to read from..
	reg_val = SPI_RW(0);    // ..then read registervalue
	NSS1;	                // NSS high, terminate SPI communication

	return(reg_val);        // return register value
}

/* @brief */
uint8_t SPI_Write(uint8_t reg, uint8_t value) {
	uint8_t status;

	NSS0;                   // NSS low, init SPI transaction
	status = SPI_RW(reg|0x80);      // select register
	SPI_RW(value);             // ..and write value to it..
	NSS1;                   // NSS high again

	return(status);            // return nRF24L01 status uchar
}

/* @brief */
void SPI_Read_Buf(uint8_t reg, uint8_t* pBuf, uint8_t uchars) {
	uint8_t uchar_ctr;
	
	for(uchar_ctr=0; uchar_ctr<uchars; uchar_ctr++)
	{
		pBuf[uchar_ctr] = SPI_Read(reg); 
	}
	return;
}

/* @brief */
void SPI_Write_Buf(uint8_t reg, uint8_t* pBuf, char uchars) {
	uint8_t uchar_ctr;
	
	for(uchar_ctr=0; uchar_ctr<uchars; uchar_ctr++)
	{
		SPI_Write(reg, pBuf[uchar_ctr]);
	}
	return;
}
/** End Of File **/
