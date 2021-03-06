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
 
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "board.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 SIM->UIDMH
#define         ID2                                 SIM->UIDML
#define         ID3                                 SIM->UIDL
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/

/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

void BoardInit()
{
	/*Giao tiep SPI*/
	SPI_Init();
}

/*SX1276 on board*/
uint8_t SX1276GetPaSelect( uint32_t channel )
{
    if( channel < RF_MID_BAND_THRESH )
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
    else
    {
        return RF_PACONFIG_PASELECT_RFO;
    }
}
/** End Of File **/
