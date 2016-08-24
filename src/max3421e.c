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

uint_fast8_t MAX_writeRegister(uint_fast8_t address, uint_fast8_t value) {
    uint_fast8_t result;

    SET_CS_LOW;

    SIMSPI_transmitByte(_getCommandByte(address, DIR_WRITE));
    result = SIMSPI_transmitByte(value);

    SET_CS_HIGH;

    return result;
}

uint_fast8_t MAX_readRegister(uint_fast8_t address) {
    uint_fast8_t result;

    SET_CS_LOW;

    SIMSPI_transmitByte(_getCommandByte(address, DIR_READ));
    result = SIMSPI_transmitByte(0);

    SET_CS_HIGH;

    return result;
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
