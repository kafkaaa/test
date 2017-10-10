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
#include "sx1276.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */
/*!
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF                              -164
#define RSSI_OFFSET_HF                              -157

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
int SX_SEND_LOCK;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */
/*!
 * Radio hardware and global parameters
 */
SX1276_t SX1276;

/*!
 * Reception buffer
 */
static uint8_t RxTxBuffer[RX_BUFFER_SIZE];

/*!
 * Performs the Rx chain calibration for LF and HF bands
 * \remark Must be called just after the reset so all registers are at their
 *         default values
 */
static void RxChainCalibration( void );

/*!
 * \brief Resets the SX1276
 */
void SX1276Reset( void );

/*!
 * \brief Sets the SX1276 in transmission mode for the given time
 * \param [IN] timeout Transmission timeout [ms] [0: continuous, others timeout]
 */
void SX1276SetTx( void );

/*!
 * \brief Writes the buffer contents to the SX1276 FIFO
 *
 * \param [IN] buffer Buffer containing data to be put on the FIFO.
 * \param [IN] size Number of bytes to be written to the FIFO
 */
void SX1276WriteFifo( uint8_t *buffer, uint8_t size );

/*!
 * \brief Reads the contents of the SX1276 FIFO
 *
 * \param [OUT] buffer Buffer where to copy the FIFO read data.
 * \param [IN] size Number of bytes to be read from the FIFO
 */
void SX1276ReadFifo( uint8_t *buffer, uint8_t size );

/*!
 * \brief Sets the SX1276 operating mode
 *
 * \param [IN] opMode New operating mode
 */
void SX1276SetOpMode( uint8_t opMode );

/*****************************************************************************/

void SX1276Init( void ) {
    SX1276Reset( );

    RxChainCalibration();

    SX1276SetOpMode( RF_OPMODE_SLEEP );

    //~ SX1276IoIrqInit( );
}

void SX1276SetChannel( uint32_t freq ) {
    SX1276.Settings.Channel = freq;
    freq = ( uint32_t )( ( double )freq / ( double )FREQ_STEP );
    SX1276Write( REG_FRFMSB, ( uint8_t )( ( freq >> 16 ) & 0xFF ) );
    SX1276Write( REG_FRFMID, ( uint8_t )( ( freq >> 8 ) & 0xFF ) );
    SX1276Write( REG_FRFLSB, ( uint8_t )( freq & 0xFF ) );
}

bool SX1276IsChannelFree( uint32_t freq, int16_t rssiThresh ) {
    int16_t rssi = 0;

    SX1276SetModem( );

    SX1276SetChannel( freq );

    SX1276SetOpMode( RF_OPMODE_RECEIVER );

    delay(1);

    rssi = SX1276ReadRssi( );

    SX1276SetSleep( );

    if( rssi > rssiThresh )
    {
        return false;
    }
    return true;
}

uint32_t SX1276Random( void ) {
    uint8_t i;
    uint32_t rnd = 0;

    /*
     * Radio setup for random number generation
     */
    SX1276SetModem();

    // Disable LoRa modem interrupts
    SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
                  RFLR_IRQFLAGS_RXDONE |
                  RFLR_IRQFLAGS_PAYLOADCRCERROR |
                  RFLR_IRQFLAGS_VALIDHEADER |
                  RFLR_IRQFLAGS_TXDONE |
                  RFLR_IRQFLAGS_CADDONE |
                  RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                  RFLR_IRQFLAGS_CADDETECTED );

    // Set radio in continuous reception
    SX1276SetOpMode( RF_OPMODE_RECEIVER );

    for( i = 0; i < 32; i++ )
    {
        delay(1);
        // Unfiltered RSSI value reading. Only takes the LSB value
        rnd |= ( ( uint32_t )SX1276Read( REG_LR_RSSIWIDEBAND ) & 0x01 ) << i;
    }

    SX1276SetSleep( );

    return rnd;
}

static void RxChainCalibration( void ) {
    uint8_t regPaConfigInitVal;
    uint32_t initialFreq;

    // Save context
    regPaConfigInitVal = SX1276Read( REG_PACONFIG );
    initialFreq = ( double )( ( ( uint32_t )SX1276Read( REG_FRFMSB ) << 16 ) |
                              ( ( uint32_t )SX1276Read( REG_FRFMID ) << 8 ) |
                              ( ( uint32_t )SX1276Read( REG_FRFLSB ) ) ) * ( double )FREQ_STEP;

    // Cut the PA just in case, RFO output, power = -1 dBm
    SX1276Write( REG_PACONFIG, 0x00 );

    // Launch Rx chain calibration for LF band
    SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Sets a Frequency in HF band
    SX1276SetChannel( 868000000 );

    // Launch Rx chain calibration for HF band
    SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Restore context
    SX1276Write( REG_PACONFIG, regPaConfigInitVal );
    SX1276SetChannel( initialFreq );
}

void SX1276SetRxConfig(  uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint32_t bandwidthAfc, uint16_t preambleLen,
                         uint16_t symbTimeout, bool fixLen,
                         uint8_t payloadLen,
                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                         bool iqInverted, bool rxContinuous ) {
    SX1276SetModem();

		if( bandwidth > 2 )
		{
				// Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
				while( 1 );
		}
		bandwidth += 7;
		SX1276.Settings.LoRa.Bandwidth = bandwidth;
		SX1276.Settings.LoRa.Datarate = datarate;
		SX1276.Settings.LoRa.Coderate = coderate;
		SX1276.Settings.LoRa.PreambleLen = preambleLen;
		SX1276.Settings.LoRa.FixLen = fixLen;
		SX1276.Settings.LoRa.PayloadLen = payloadLen;
		SX1276.Settings.LoRa.CrcOn = crcOn;
		SX1276.Settings.LoRa.FreqHopOn = freqHopOn;
		SX1276.Settings.LoRa.HopPeriod = hopPeriod;
		SX1276.Settings.LoRa.IqInverted = iqInverted;
		SX1276.Settings.LoRa.RxContinuous = rxContinuous;

		if( datarate > 12 )
		{
				datarate = 12;
		}
		else if( datarate < 6 )
		{
				datarate = 6;
		}

		if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
				( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
		{
				SX1276.Settings.LoRa.LowDatarateOptimize = 0x01;
		}
		else
		{
				SX1276.Settings.LoRa.LowDatarateOptimize = 0x00;
		}

		SX1276Write( REG_LR_MODEMCONFIG1,
								 ( SX1276Read( REG_LR_MODEMCONFIG1 ) &
									 RFLR_MODEMCONFIG1_BW_MASK &
									 RFLR_MODEMCONFIG1_CODINGRATE_MASK &
									 RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
									 ( bandwidth << 4 ) | ( coderate << 1 ) |
									 fixLen );

		SX1276Write( REG_LR_MODEMCONFIG2,
								 ( SX1276Read( REG_LR_MODEMCONFIG2 ) &
									 RFLR_MODEMCONFIG2_SF_MASK &
									 RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK &
									 RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) |
									 ( datarate << 4 ) | ( crcOn << 2 ) |
									 ( ( symbTimeout >> 8 ) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) );

		SX1276Write( REG_LR_MODEMCONFIG3,
								 ( SX1276Read( REG_LR_MODEMCONFIG3 ) &
									 RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) |
									 ( SX1276.Settings.LoRa.LowDatarateOptimize << 3 ) );

		SX1276Write( REG_LR_SYMBTIMEOUTLSB, ( uint8_t )( symbTimeout & 0xFF ) );

		SX1276Write( REG_LR_PREAMBLEMSB, ( uint8_t )( ( preambleLen >> 8 ) & 0xFF ) );
		SX1276Write( REG_LR_PREAMBLELSB, ( uint8_t )( preambleLen & 0xFF ) );

		if( fixLen == 1 )
		{
				SX1276Write( REG_LR_PAYLOADLENGTH, payloadLen );
		}

		if( SX1276.Settings.LoRa.FreqHopOn == true )
		{
				SX1276Write( REG_LR_PLLHOP, ( SX1276Read( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
				SX1276Write( REG_LR_HOPPERIOD, SX1276.Settings.LoRa.HopPeriod );
		}

		if( ( bandwidth == 9 ) && ( SX1276.Settings.Channel > RF_MID_BAND_THRESH ) )
		{
				// ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
				SX1276Write( REG_LR_TEST36, 0x02 );
				SX1276Write( REG_LR_TEST3A, 0x64 );
		}
		else if( bandwidth == 9 )
		{
				// ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
				SX1276Write( REG_LR_TEST36, 0x02 );
				SX1276Write( REG_LR_TEST3A, 0x7F );
		}
		else
		{
				// ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
				SX1276Write( REG_LR_TEST36, 0x03 );
		}

		if( datarate == 6 )
		{
				SX1276Write( REG_LR_DETECTOPTIMIZE,
										 ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
											 RFLR_DETECTIONOPTIMIZE_MASK ) |
											 RFLR_DETECTIONOPTIMIZE_SF6 );
				SX1276Write( REG_LR_DETECTIONTHRESHOLD,
										 RFLR_DETECTIONTHRESH_SF6 );
		}
		else
		{
				SX1276Write( REG_LR_DETECTOPTIMIZE,
										 ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
										 RFLR_DETECTIONOPTIMIZE_MASK ) |
										 RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
				SX1276Write( REG_LR_DETECTIONTHRESHOLD,
										 RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
		}
}

void SX1276SetTxConfig( int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout ) {
    uint8_t paConfig = 0;
    uint8_t paDac = 0;
	
    SX1276SetModem();

    paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );
    paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power < -1 )
        {
            power = -1;
        }
        if( power > 14 )
        {
            power = 14;
        }
        paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
    }
    SX1276Write( REG_PACONFIG, paConfig );
    SX1276Write( REG_PADAC, paDac );

		SX1276.Settings.LoRa.Power = power;
		if( bandwidth > 2 )
		{
				// Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
				while( 1 );
		}
		bandwidth += 7;
		SX1276.Settings.LoRa.Bandwidth = bandwidth;
		SX1276.Settings.LoRa.Datarate = datarate;
		SX1276.Settings.LoRa.Coderate = coderate;
		SX1276.Settings.LoRa.PreambleLen = preambleLen;
		SX1276.Settings.LoRa.FixLen = fixLen;
		SX1276.Settings.LoRa.FreqHopOn = freqHopOn;
		SX1276.Settings.LoRa.HopPeriod = hopPeriod;
		SX1276.Settings.LoRa.CrcOn = crcOn;
		SX1276.Settings.LoRa.IqInverted = iqInverted;
		SX1276.Settings.LoRa.TxTimeout = timeout;

		if( datarate > 12 )
		{
				datarate = 12;
		}
		else if( datarate < 6 )
		{
				datarate = 6;
		}
		if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
				( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
		{
				SX1276.Settings.LoRa.LowDatarateOptimize = 0x01;
		}
		else
		{
				SX1276.Settings.LoRa.LowDatarateOptimize = 0x00;
		}

		if( SX1276.Settings.LoRa.FreqHopOn == true )
		{
				SX1276Write( REG_LR_PLLHOP, ( SX1276Read( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
				SX1276Write( REG_LR_HOPPERIOD, SX1276.Settings.LoRa.HopPeriod );
		}

		SX1276Write( REG_LR_MODEMCONFIG1,
								 ( SX1276Read( REG_LR_MODEMCONFIG1 ) &
									 RFLR_MODEMCONFIG1_BW_MASK &
									 RFLR_MODEMCONFIG1_CODINGRATE_MASK &
									 RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
									 ( bandwidth << 4 ) | ( coderate << 1 ) |
									 fixLen );

		SX1276Write( REG_LR_MODEMCONFIG2,
								 ( SX1276Read( REG_LR_MODEMCONFIG2 ) &
									 RFLR_MODEMCONFIG2_SF_MASK &
									 RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK ) |
									 ( datarate << 4 ) | ( crcOn << 2 ) );

		SX1276Write( REG_LR_MODEMCONFIG3,
								 ( SX1276Read( REG_LR_MODEMCONFIG3 ) &
									 RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) |
									 ( SX1276.Settings.LoRa.LowDatarateOptimize << 3 ) );

		SX1276Write( REG_LR_PREAMBLEMSB, ( preambleLen >> 8 ) & 0x00FF );
		SX1276Write( REG_LR_PREAMBLELSB, preambleLen & 0xFF );

		if( datarate == 6 )
		{
				SX1276Write( REG_LR_DETECTOPTIMIZE,
										 ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
											 RFLR_DETECTIONOPTIMIZE_MASK ) |
											 RFLR_DETECTIONOPTIMIZE_SF6 );
				SX1276Write( REG_LR_DETECTIONTHRESHOLD,
										 RFLR_DETECTIONTHRESH_SF6 );
		}
		else
		{
				SX1276Write( REG_LR_DETECTOPTIMIZE,
										 ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
										 RFLR_DETECTIONOPTIMIZE_MASK ) |
										 RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
				SX1276Write( REG_LR_DETECTIONTHRESHOLD,
										 RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
		}
}

void SX1276Send( uint8_t *buffer, uint8_t size ) {
		if( SX1276.Settings.LoRa.IqInverted == true )
		{
				SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_ON ) );
				SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
		}
		else
		{
				SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
				SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
		}

		SX1276.Settings.LoRaPacketHandler.Size = size;

		// Initializes the payload size
		SX1276Write( REG_LR_PAYLOADLENGTH, size );

		// Full buffer used for Tx
		SX1276Write( REG_LR_FIFOTXBASEADDR, 0 );
		SX1276Write( REG_LR_FIFOADDRPTR, 0 );

		// FIFO operations can not take place in Sleep mode
		if( ( SX1276Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
		{
				SX1276SetStby( );
				delay(1);
		}
		// Write payload buffer
		SX1276WriteFifo( buffer, size );

		SX1276SetTx();
}

void SX1276SetSleep( void ) {
    SX1276SetOpMode( RF_OPMODE_SLEEP );
    SX1276.Settings.State = RF_IDLE;
}

void SX1276SetStby( void ) {
    SX1276SetOpMode( RF_OPMODE_STANDBY );
    SX1276.Settings.State = RF_IDLE;
}

void SX1276SetRx( void ) {
    bool rxContinuous = false;

		if( SX1276.Settings.LoRa.IqInverted == true )
		{
				SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_ON | RFLR_INVERTIQ_TX_OFF ) );
				SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
		}
		else
		{
				SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
				SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
		}

		// ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
		if( SX1276.Settings.LoRa.Bandwidth < 9 )
		{
				SX1276Write( REG_LR_DETECTOPTIMIZE, SX1276Read( REG_LR_DETECTOPTIMIZE ) & 0x7F );
				SX1276Write( REG_LR_TEST30, 0x00 );
				switch( SX1276.Settings.LoRa.Bandwidth )
				{
				case 0: // 7.8 kHz
						SX1276Write( REG_LR_TEST2F, 0x48 );
						SX1276SetChannel(SX1276.Settings.Channel + 7.81e3 );
						break;
				case 1: // 10.4 kHz
						SX1276Write( REG_LR_TEST2F, 0x44 );
						SX1276SetChannel(SX1276.Settings.Channel + 10.42e3 );
						break;
				case 2: // 15.6 kHz
						SX1276Write( REG_LR_TEST2F, 0x44 );
						SX1276SetChannel(SX1276.Settings.Channel + 15.62e3 );
						break;
				case 3: // 20.8 kHz
						SX1276Write( REG_LR_TEST2F, 0x44 );
						SX1276SetChannel(SX1276.Settings.Channel + 20.83e3 );
						break;
				case 4: // 31.2 kHz
						SX1276Write( REG_LR_TEST2F, 0x44 );
						SX1276SetChannel(SX1276.Settings.Channel + 31.25e3 );
						break;
				case 5: // 41.4 kHz
						SX1276Write( REG_LR_TEST2F, 0x44 );
						SX1276SetChannel(SX1276.Settings.Channel + 41.67e3 );
						break;
				case 6: // 62.5 kHz
						SX1276Write( REG_LR_TEST2F, 0x40 );
						break;
				case 7: // 125 kHz
						SX1276Write( REG_LR_TEST2F, 0x40 );
						break;
				case 8: // 250 kHz
						SX1276Write( REG_LR_TEST2F, 0x40 );
						break;
				}
		}
		else
		{
				SX1276Write( REG_LR_DETECTOPTIMIZE, SX1276Read( REG_LR_DETECTOPTIMIZE ) | 0x80 );
		}

		rxContinuous = SX1276.Settings.LoRa.RxContinuous;

		if( SX1276.Settings.LoRa.FreqHopOn == true )
		{
				SX1276Write( REG_LR_IRQFLAGSMASK, //RFLR_IRQFLAGS_RXTIMEOUT |
																					//RFLR_IRQFLAGS_RXDONE |
																					//RFLR_IRQFLAGS_PAYLOADCRCERROR |
																					RFLR_IRQFLAGS_VALIDHEADER |
																					RFLR_IRQFLAGS_TXDONE |
																					RFLR_IRQFLAGS_CADDONE |
																					//RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
																					RFLR_IRQFLAGS_CADDETECTED );

				// DIO0=RxDone, DIO2=FhssChangeChannel
				SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK  ) | RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO2_00 );
		}
		else
		{
				SX1276Write( REG_LR_IRQFLAGSMASK, //RFLR_IRQFLAGS_RXTIMEOUT |
																					//RFLR_IRQFLAGS_RXDONE |
																					//RFLR_IRQFLAGS_PAYLOADCRCERROR |
																					RFLR_IRQFLAGS_VALIDHEADER |
																					RFLR_IRQFLAGS_TXDONE |
																					RFLR_IRQFLAGS_CADDONE |
																					RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
																					RFLR_IRQFLAGS_CADDETECTED );

				// DIO0=RxDone
				SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK ) | RFLR_DIOMAPPING1_DIO0_00 );
		}
		SX1276Write( REG_LR_FIFORXBASEADDR, 0 );
		SX1276Write( REG_LR_FIFOADDRPTR, 0 );

    memset( RxTxBuffer, 0, ( size_t )RX_BUFFER_SIZE );

    SX1276.Settings.State = RF_RX_RUNNING;
		
		if( rxContinuous == true )
		{
				SX1276SetOpMode( RFLR_OPMODE_RECEIVER );
		}
		else
		{
				SX1276SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
		}
}

void SX1276SetTx( void ) {
		if( SX1276.Settings.LoRa.FreqHopOn == true )
		{
				SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
																					RFLR_IRQFLAGS_RXDONE |
																					RFLR_IRQFLAGS_PAYLOADCRCERROR |
																					RFLR_IRQFLAGS_VALIDHEADER |
																					//RFLR_IRQFLAGS_TXDONE |
																					RFLR_IRQFLAGS_CADDONE |
																					//RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
																					RFLR_IRQFLAGS_CADDETECTED );

				// DIO0=TxDone, DIO2=FhssChangeChannel
				SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK ) | RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO2_00 );
		}
		else
		{
				SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
																					RFLR_IRQFLAGS_RXDONE |
																					RFLR_IRQFLAGS_PAYLOADCRCERROR |
																					RFLR_IRQFLAGS_VALIDHEADER |
																					//RFLR_IRQFLAGS_TXDONE |
																					RFLR_IRQFLAGS_CADDONE |
																					RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
																					RFLR_IRQFLAGS_CADDETECTED );

				// DIO0=TxDone
				SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK ) | RFLR_DIOMAPPING1_DIO0_01 );
		}

    SX1276.Settings.State = RF_TX_RUNNING;
		
    SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
}

void SX1276StartCad( void ) {
		SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
																RFLR_IRQFLAGS_RXDONE |
																RFLR_IRQFLAGS_PAYLOADCRCERROR |
																RFLR_IRQFLAGS_VALIDHEADER |
																RFLR_IRQFLAGS_TXDONE |
																//RFLR_IRQFLAGS_CADDONE |
																RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL // |
																//RFLR_IRQFLAGS_CADDETECTED
																);

		// DIO3=CADDone
		SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK ) | RFLR_DIOMAPPING1_DIO0_00 );

		SX1276.Settings.State = RF_CAD;
		SX1276SetOpMode( RFLR_OPMODE_CAD );
}

int16_t SX1276ReadRssi( void ) {
    int16_t rssi = 0;
	
		if( SX1276.Settings.Channel > RF_MID_BAND_THRESH )
		{
				rssi = RSSI_OFFSET_HF + SX1276Read( REG_LR_RSSIVALUE );
		}
		else
		{
				rssi = RSSI_OFFSET_LF + SX1276Read( REG_LR_RSSIVALUE );
		}
		
    return rssi;
}

void SX1276Reset( void ) {
    RESET0;
    delay(1);
    RESET1;
    delay(6);
}

void SX1276SetOpMode( uint8_t opMode ) {
    SX1276Write( REG_OPMODE, ( SX1276Read( REG_OPMODE ) & RF_OPMODE_MASK ) | opMode );
}

void SX1276SetModem(void) {
    assert_param( ( SX1276.Spi.Spi.Instance != NULL ) );
	
		if( SX1276.Settings.Modem == MODEM_LORA )
    {
        return;
    }
		
		SX1276.Settings.Modem = MODEM_LORA;

		SX1276SetOpMode( RF_OPMODE_SLEEP );
		SX1276Write( REG_OPMODE, ( SX1276Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON );

		SX1276Write( REG_DIOMAPPING1, 0x00 );
		SX1276Write( REG_DIOMAPPING2, 0x00 );
}

void SX1276Write( uint8_t addr, uint8_t data ) {
    SX1276WriteBuffer( addr, &data, 1 );
}

uint8_t SX1276Read( uint8_t addr ) {
    uint8_t data;
    SX1276ReadBuffer( addr, &data, 1 );
    return data;
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size ) {
	SPI_Write_Buf( addr, buffer, size );
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size ) {
	SPI_Read_Buf( addr, buffer, size );
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size ) {
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size ) {
    SX1276ReadBuffer( 0, buffer, size );
}

void SX1276SetMaxPayloadLength( uint8_t max ) {
    SX1276SetModem();

    SX1276Write( REG_LR_PAYLOADMAXLENGTH, max );
}

void SX1276OnDio0Irq( void ) {
	//~ int i;
	int16_t rssi;
    volatile uint8_t irqFlags = 0;

    switch( SX1276.Settings.State )
    {
        case RF_RX_RUNNING:
					{
						int8_t snr = 0;
						// Clear Irq
						SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE );

						irqFlags = SX1276Read( REG_LR_IRQFLAGS );
						if( ( irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK ) == RFLR_IRQFLAGS_PAYLOADCRCERROR )
						{
								// Clear Irq
								SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR );

								if( SX1276.Settings.LoRa.RxContinuous == false )
								{
										SX1276.Settings.State = RF_IDLE;
								}
								break;
						}

						SX1276.Settings.LoRaPacketHandler.SnrValue = SX1276Read( REG_LR_PKTSNRVALUE );
						if( SX1276.Settings.LoRaPacketHandler.SnrValue & 0x80 ) // The SNR sign bit is 1
						{
								// Invert and divide by 4
								snr = ( ( ~SX1276.Settings.LoRaPacketHandler.SnrValue + 1 ) & 0xFF ) >> 2;
								snr = -snr;
						}
						else
						{
								// Divide by 4
								snr = ( SX1276.Settings.LoRaPacketHandler.SnrValue & 0xFF ) >> 2;
						}

						rssi = SX1276Read( REG_LR_PKTRSSIVALUE );
						if( snr < 0 )
						{
								if( SX1276.Settings.Channel > RF_MID_BAND_THRESH )
								{
										SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_HF + rssi + ( rssi >> 4 ) + snr;
								}
								else
								{
										SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_LF + rssi + ( rssi >> 4 ) + snr;
								}
						}
						else
						{
								if( SX1276.Settings.Channel > RF_MID_BAND_THRESH )
								{
										SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_HF + rssi + ( rssi >> 4 );
								}
								else
								{
										SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_LF + rssi + ( rssi >> 4 );
								}
						}

						SX1276.Settings.LoRaPacketHandler.Size = SX1276Read( REG_LR_RXNBBYTES );
						SX1276ReadFifo( RxTxBuffer, SX1276.Settings.LoRaPacketHandler.Size );
						
						//Parse Packet Here
						OnRadioRxDone(RxTxBuffer, SX1276.Settings.LoRaPacketHandler.Size, rssi, snr);
						
						if( SX1276.Settings.LoRa.RxContinuous == false )
						{
								SX1276.Settings.State = RF_IDLE;
						}
					}
            break;
        case RF_TX_RUNNING:
                SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE );
                SX1276.Settings.State = RF_IDLE;
								//Dua SX ve che do nhan lien tuc
								OnRadioTxDone();
						break;
        default:
            break;
    }
}

/*******************************************************************************/
/********************************* LORAMAC.c ***********************************/
/*******************************************************************************/
/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

uint8_t typeOfData = dataType_DataUnknow;

const uint8_t netID = 0x01;

/*!
 * Device Address
 */
static uint32_t LoRaMacDevAddr;

void LoRaMacInitialization( void ) {
		SX1276Init();
		SX1276SetChannel(LORA_RF_FREQUENCY);
}

void RxWindowSetup( bool rxContinuous ) {
		SX1276SetRxConfig( LORA_BANDWIDTH, LORA_DATARATE,
													LORA_CODINGRATE, LORA_BANDWIDTH_AFC,
													LORA_PREAMBLE_LENGTH, LORA_SYS_TIMEOUT,
													LORA_FIX_LENGTH_PAYLOAD_ON, LORA_PAYLOAD_LENGTH,
													LORA_CRC_ON, LORA_FREQ_HOP_ON,
													LORA_HOP_PERIOUS, LORA_IQ_INVERSION_ON, rxContinuous);
	
		SX1276SetRx();
}

LoRaMacStatus_t PrepareFrame( typeOfData_t typeOfData, void *fBuffer, uint16_t fBufferSize ) {
		uint8_t* payload = fBuffer;
		//~ int i;
	
		LoRaMacBufferPktLen = 0;
		
		memset(LoRaMacBuffer, 0, 11);
		
		LoRaMacBuffer[LoRaMacBufferPktLen++] = typeOfData;

		switch(typeOfData)
		{
			case dataTypeJoin_Accept:
				{
						LoRaMacBuffer[LoRaMacBufferPktLen++] =	LoRaMacDevAddr & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = netID;
						
						printf("Device %d %d %d %d 's Join Request is accepted!\n",LoRaMacBuffer[1],LoRaMacBuffer[2],LoRaMacBuffer[3],LoRaMacBuffer[4]);
						printf("-----------\n");
						LoRaMacBufferPktLen = 11;
				}
				break;
			case dataType_DataUnconFirmDown:
				{
						LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevAddr & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
						LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;
					
						LoRaMacBufferPktLen = 11;
				}
				break;
			case dataType_DataConfirmDown:
				{
						LoRaMacBuffer[LoRaMacBufferPktLen++] = payload[0];
						LoRaMacBuffer[LoRaMacBufferPktLen++] = payload[1];
						LoRaMacBuffer[LoRaMacBufferPktLen++] = payload[2];
						LoRaMacBuffer[LoRaMacBufferPktLen++] = payload[3];
						LoRaMacBuffer[LoRaMacBufferPktLen++] = payload[4];
						LoRaMacBuffer[LoRaMacBufferPktLen++] = payload[5];

						LoRaMacBufferPktLen = 11;
						//~ printf("DataConfirmDown LoRaMacBuffer:");
							//~ for(i=0;i<6;i++){
								//~ printf("%d ",LoRaMacBuffer[i]);
							//~ }
							//~ printf("\n");
				}
				break;
			default:
				return LORAMAC_STATUS_SERVICE_UNKNOWN;
		}
	
		return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t Send( typeOfData_t typeOfData, void *fBuffer, uint16_t fBufferSize ) {
	LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;
	
	// Prepare the frame
    status = PrepareFrame( typeOfData, fBuffer, fBufferSize );

	// Validate status
    if( status != LORAMAC_STATUS_OK )
    {
        return status;
    }

	status = SendFrameOnChannel();

	return status;
}

LoRaMacStatus_t SendFrameOnChannel( ) {
		SX1276SetMaxPayloadLength(LoRaMacBufferPktLen);
		SX1276SetTxConfig( LORA_TX_OUTPUT_POWER, LORA_FDEV, 
													LORA_BANDWIDTH, LORA_DATARATE, LORA_CODINGRATE, 
													LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, 
													LORA_CRC_ON, LORA_FREQ_HOP_ON, LORA_HOP_PERIOUS,
													LORA_IQ_INVERSION_ON, LORA_TIMEOUT );
		
		SX1276Send(LoRaMacBuffer, LoRaMacBufferPktLen);
		
		return LORAMAC_STATUS_OK;
}

void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {
		//~ int i;
		//~ char *response;
		uint8_t pktLen = 0;
		uint8_t netIDFromGW = 0;
		uint8_t typeOfDataFromeGW = payload[pktLen++];

		switch(typeOfDataFromeGW)
		{
			case dataTypeJoin_Req:
				{
						LoRaMacDevAddr = payload[pktLen++];
						LoRaMacDevAddr |= ( (uint32_t)payload[pktLen++] << 8 );
						LoRaMacDevAddr |= ( (uint32_t)payload[pktLen++] << 16 );
						LoRaMacDevAddr |= ( (uint32_t)payload[pktLen++] << 24 );
						printf("Device %d %d %d %d send Join Request!\n",payload[1],payload[2],payload[3],payload[4]);
						printf("-----------\n");
						
						dtb_checkDevAddr(payload);
					
						piLock(SX_SEND_LOCK);
						Send(dataTypeJoin_Accept, 0, 0);
						piUnlock(SX_SEND_LOCK);
				}
				break;
			case dataType_DataConfirmUp:
				{
						netIDFromGW = payload[4];
						if(netIDFromGW==netID)
						{
							LoRaMacDevAddr = payload[pktLen++];
							LoRaMacDevAddr |= ( (uint32_t)payload[pktLen++] << 8 );
							LoRaMacDevAddr |= ( (uint32_t)payload[pktLen++] << 16 );
							LoRaMacDevAddr |= ( (uint32_t)payload[pktLen++] << 24 );	
							
							if(dtb_checkDevExist(payload) == 1){
								/* Import to Pi Buffer */
								bf_importToPiBuffer(payload);
							}
							piLock(SX_SEND_LOCK);
							Send(dataType_DataUnconFirmDown, 0, 0);
							piUnlock(SX_SEND_LOCK);							
						}
				}
				break;
			case dataType_DataUnconFirmUp:
				{
						
				}
				break;
			default:
				break;
		} 
}

void OnRadioTxDone( void ) {
		RxWindowSetup(true);
}

uint8_t getTypeOfData(void) {
	return typeOfData;
}

void setTypeOfData(uint8_t typeOfData_set) {
	typeOfData = typeOfData_set;
}
/** End Of File **/
