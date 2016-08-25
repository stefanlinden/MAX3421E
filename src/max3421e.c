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

volatile uint_fast8_t enabledIRQ;

/* PROTOTYPES FOR PRIVATE FUNCTIONS */

uint_fast8_t _getCommandByte(uint_fast8_t, uint_fast8_t);
uint_fast8_t _getCommandByteAckstat(uint_fast8_t, uint_fast8_t, uint_fast8_t);


/* PUBLIC FUNCTIONS */

void MAX_start(uint_fast8_t startAsMaster) {
    /* Start SPI */
    SIMSPI_startSPI();

    /* Set the SPI configuration to 4-wire and IRQ mode to pulldown */
    MAX_writeRegister(17, 0x18);

    /* Make sure everything is reset (note: this does NOT reset the SPI config) */
    MAX_reset();

    /* Enable the dedicated INT pin (active low) */
    MAP_GPIO_setAsInputPinWithPullUpResistor(USBINT_PORT, USBINT_PIN);
    MAP_GPIO_interruptEdgeSelect(USBINT_PORT, USBINT_PIN,
    GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_clearInterruptFlag(USBINT_PORT, USBINT_PIN);
    MAP_GPIO_enableInterrupt(USBINT_PORT, USBINT_PIN);
    MAP_Interrupt_enableInterrupt(INT_PORT2);

    if(startAsMaster) {
        /* We're starting as a USB host/master */
        /* Enable HOST, DMPULLDN and DPPULLDN */
        MAX_enableOptions(27, BIT0 | BIT6 | BIT7);
    } else {
        /* We're starting as a peripheral */

    }
}

void MAX_reset( void ) {
    /* Enable the reset */
    MAX_writeRegister(15, BIT5);

    /* Immediately Stop the reset */
    MAX_writeRegister(15, 0);

    /* Wait a short while until the oscillator is stable */
    DELAY_WITH_TIMEOUT(!(MAX_readRegister(13) & BIT0));

    /* Reset the interrupt state */
    MAX_disableInterruptsMaster();
    MAX_writeRegister(26, 0);
    MAX_writeRegister(25, 0xFF);
    enabledIRQ = 0;
}

void MAX_enableInterrupts(uint_fast8_t flags) {
    /* Enable the interrupts */
    MAX_enableOptions(26, flags);
    enabledIRQ |= flags;
}

void MAX_disableInterrupts(uint_fast8_t flags) {
    /* Disable the interrupts */
    MAX_disableOptions(26, flags);
    enabledIRQ &= ~flags;
}

void MAX_clearInterruptStatus(uint_fast8_t flags) {
    /* Clear the specified interrupts */
    MAX_enableOptions(25, flags);
}

void MAX_enableInterruptsMaster( void ) {
    /* Set IE to 1 */
    MAX_enableOptions(16, BIT0);
}

void MAX_disableInterruptsMaster( void ) {
    /* Set IE to 0 */
    MAX_disableOptions(16, BIT0);
}

uint_fast8_t MAX_getInterruptStatus( void ) {
    return MAX_readRegister(25);
}

uint_fast8_t MAX_getEnabledInterruptStatus( void ) {
    uint_fast8_t result = MAX_getInterruptStatus();
    return result  & enabledIRQ;
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
    //MAX_enableOptions(29, BIT2);
    uint_fast8_t regval = MAX_readRegister(26);
    printf("Value of register 26: 0x%x\n", regval);
    uint_fast8_t status = MAX_getInterruptStatus();
    printf("Got an interrupt: 0x%x\n", status);
    MAX_clearInterruptStatus(status);
}
