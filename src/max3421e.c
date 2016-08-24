/*
 * MAX3421E.c
 *
 *  Created on: 24 Aug 2016
 *      Author: Stefan van der Linden
 */

#include <max3421e.h>

#define BUFFER_SIZE 14

volatile uint_fast8_t RXData;
volatile uint_fast8_t mode;
volatile uint_fast8_t TXData[BUFFER_SIZE];
volatile uint_fast8_t TXSize;

/* PROTOTYPES FOR PRIVATE FUNCTIONS */

uint_fast8_t _getCommandByte(uint_fast8_t, uint_fast8_t);
uint_fast8_t _getCommandByteAckstat(uint_fast8_t, uint_fast8_t, uint_fast8_t);


/* PUBLIC FUNCTIONS */

void MAX_start() {
    SIMSPI_startSPI();
}

void MAX_reset() {
    MAX_enableOptions(15, BIT5);
    DELAY_WITH_TIMEOUT(1);
}

uint_fast8_t MAX_writeRegister(uint_fast8_t address, uint_fast8_t value) {
    uint_fast8_t result;

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW;

    /* Build and transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_WRITE));
    /* Transmit the data */
    result = SIMSPI_transmitByte(value);

    /* End the transaction by pulling the CS back to high */
    SET_CS_HIGH;

    return result;
}

uint_fast8_t MAX_readRegister(uint_fast8_t address) {
    uint_fast8_t result;

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW;

    /* Transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_READ));
    /* Transmit a 0, as we don't actually care about what's written but we do about the response */
    result = SIMSPI_transmitByte(0);

    /* End the transaction by pulling the CS back to high */
    SET_CS_HIGH;

    return result;
}

void MAX_enableOptions(uint_fast8_t address, uint_fast8_t flags) {
    /* Read the current state of the register */
    uint_fast8_t regVal = MAX_readRegister(address);

    /* Enable the given bits */
    regVal |= flags;

    /* Write the new register value back to the module */
    MAX_writeRegister(address, regVal);
}

void MAX_disableOptions(uint_fast8_t address, uint_fast8_t flags) {
    /* Read the current state of the register */
    uint_fast8_t regVal = MAX_readRegister(address);

    /* Disable the given bits */
    regVal &= ~flags;

    /* Write the new register value back to the module */
    MAX_writeRegister(address, regVal);
}

/* PRIVATE FUNCTIONS */

uint_fast8_t _getCommandByteAckstat(uint_fast8_t address, uint_fast8_t direction, uint_fast8_t ackstat) {
    uint_fast8_t result = 0;
    result |= address << 3;
    result |= direction << 1;
    result |= ackstat;
    return result;
}

uint_fast8_t _getCommandByte(uint_fast8_t address, uint_fast8_t direction) {
    return _getCommandByteAckstat(address, direction, 0);
}


/* INTERRUPT HANDLERS */

void GPIOP2_ISR(void) {

}