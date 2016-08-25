/*
 * simple_spi.h
 *
 *  Created on: 27 Jun 2016
 *      Author: Stefan van der Linden
 */

#ifndef SIMPLE_SPI_H_
#define SIMPLE_SPI_H_

/*** Defines ***/
#define MODULE EUSCI_B0_BASE
#define CS_PORT GPIO_PORT_P3
#define CS_PIN GPIO_PIN7
#define SPI_PORT GPIO_PORT_P1
#define SPI_PIN GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7

#define SPI_TIMEOUT         500

/*** MACROs ***/

#define DELAY_WITH_TIMEOUT(STATEMENT)   uint_fast8_t __it__ = 0; \
                                        while(__it__ < SPI_TIMEOUT && STATEMENT) { __it__++; }

#define SET_CS_LOW  MAP_GPIO_setOutputLowOnPin(CS_PORT, CS_PIN);
#define SET_CS_HIGH MAP_GPIO_setOutputHighOnPin(CS_PORT, CS_PIN);

/*** PROTOTYPES ***/

/**
 * Start the SPI module
 */
void SIMSPI_startSPI(void);

/**
 * Transmit and receive a single byte
 *
 * Parameters:
 * uint_fast8_t byte: the single byte to send
 *
 * Returns:
 * uint_fast8_t: the single byte received during the transmission
 */
uint_fast8_t SIMSPI_transmitByte( uint_fast8_t );

/**
 * Transmit and receive an array of bytes
 *
 * Parameters:
 * uint_fast8_t bytes: the byte array to transmit
 * uint_fast8_t length: the amount of bytes in the given array
 *
 * Returns:
 * uint_fast8_t: the single byte received during the transmission
 */
uint_fast8_t SIMSPI_transmitBytes( uint_fast8_t *, uint_fast8_t );

/**
 * Transmit the given bytes and return all the received bytes
 *
 * Parameters:
 * uint_fast8_t * rxbuffer: a buffer able to hold the received bytes
 * uint_fast8_t * bytes: the byte array to transmit
 * uint_fast8_t length: the amount of bytes in the given array
 *
 * Returns:
 * uint_fast8_t *: a (dynamically allocated) pointer to the received bytes
 */
uint_fast8_t SIMSPI_transmitBytesReadAll(uint_fast8_t *, uint_fast8_t *, uint_fast8_t);

#endif /* SIMPLE_SPI_H_ */
