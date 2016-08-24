/*
 * simple_spi.c
 *
 *  Created on: 27 Jun 2016
 *      Author: Stefan van der Linden
 */

#include <driverlib.h>
#include "simple_spi.h"

/* SPI Master Configuration Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig = {
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
        24000000, // SMCLK = DCO = 24MHz
        4000000, // SPICLK = 1MHz
        EUSCI_B_SPI_MSB_FIRST, // MSB First
        EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT, // Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW, // Low polarity
        EUSCI_B_SPI_3PIN // 3Wire SPI Mode + GPIO for SS
        };

void SIMSPI_startSPI(void) {
    /* Initialise the pins */
    MAP_GPIO_setAsOutputPin(CS_PORT, CS_PIN);
    MAP_GPIO_setOutputHighOnPin(CS_PORT, CS_PIN);

    /* Selecting P1.5 P1.6 and P1.7 in SPI mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(SPI_PORT,
    SPI_PIN, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring SPI in 3wire master mode */
    MAP_SPI_initMaster(MODULE, &spiMasterConfig);

    /* Enable SPI module */
    MAP_SPI_enableModule(MODULE);
}

uint_fast8_t SIMSPI_transmitByte(uint_fast8_t byte) {
    // Make sure the receive interrupt is cleared
    MAP_SPI_clearInterruptFlag(MODULE, EUSCI_B_SPI_RECEIVE_INTERRUPT);

    /* Transmit the byte */
    MAP_SPI_transmitData(MODULE, byte);

    /* Wait until the RX buffer is ready */
    DELAY_WITH_TIMEOUT(
            !MAP_SPI_getInterruptStatus(MODULE, EUSCI_B_SPI_RECEIVE_INTERRUPT));

    /* Return the RX buffer's contents */
    return MAP_SPI_receiveData(MODULE);
}

uint_fast8_t SIMSPI_transmitBytes(uint_fast8_t * bytes, uint_fast8_t length) {
    uint_fast8_t it;

    // Make sure the receive interrupt is cleared
    MAP_SPI_clearInterruptFlag(MODULE, EUSCI_B_SPI_RECEIVE_INTERRUPT);

    for (it = 0; it < length - 1; it++) {
        /* Transmit the current byte */
        SIMSPI_transmitByte(bytes[it]);
    }
    return SIMSPI_transmitByte(bytes[it++]);
}

uint_fast8_t SIMSPI_transmitBytesReadAll(uint_fast8_t * rxbuffer, uint_fast8_t * bytes, uint_fast8_t length) {
    uint_fast8_t it;

    /* Make sure the receive interrupt is cleared */
    MAP_SPI_clearInterruptFlag(MODULE, EUSCI_B_SPI_RECEIVE_INTERRUPT);

    for (it = 0; it < length; it++) {
        /* Transmit the current byte */
        rxbuffer[it] = SIMSPI_transmitByte(bytes[it]);
    }
    return 0;
}
