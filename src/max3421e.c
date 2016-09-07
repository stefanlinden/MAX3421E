/*
 * MAX3421E.c
 *
 *  Created on: 24 Aug 2016
 *      Author: Stefan van der Linden
 */

#include <driverlib.h>
#include <stdbool.h>

#include "usb.h"
#include "packets.h"
#include "max3421e.h"
#include "delay.h"

volatile uint_fast8_t RXData[BUFFER_SIZE];
volatile uint_fast8_t mode;
volatile uint_fast8_t TXSize;

volatile bool ACKSTAT;

volatile uint_fast8_t enabledIRQ;
volatile uint_fast8_t enabledEPIRQ;
volatile uint_fast8_t peripheralConnected;
volatile uint_fast8_t lastTransferResult;

/* PROTOTYPES FOR PRIVATE FUNCTIONS */

uint_fast8_t _getCommandByte( uint_fast8_t, uint_fast8_t );
void sendControl( ControlPacket * );

/* PUBLIC FUNCTIONS */

void MAX_start( uint_fast8_t startAsMaster ) {
    /* Start SPI */
    SIMSPI_startSPI( );

    /* Set the SPI configuration to 4-wire and IRQ mode to pulldown */
    MAX_writeRegister(17, 0x18);

    /* Make sure everything is reset (note: this does NOT reset the SPI config) */
    MAX_reset( );

    /* Enable the dedicated INT pin (active low) */
    MAP_GPIO_setAsInputPin(USBINT_PORT, USBINT_PIN);
    MAP_GPIO_interruptEdgeSelect(USBINT_PORT, USBINT_PIN,
    GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_clearInterruptFlag(USBINT_PORT, USBINT_PIN);
    MAP_GPIO_enableInterrupt(USBINT_PORT, USBINT_PIN);
    MAP_Interrupt_enableInterrupt(INT_PORT2);

    /* Enabling MASTER interrupts */
    MAP_Interrupt_enableMaster( );

    if ( startAsMaster ) {
        /* We're starting as a USB host/master */
        mode = MODE_HOST;

        /* Enable HOST, DMPULLDN and DPPULLDN */
        MAX_enableOptions(27, BIT0 | BIT6 | BIT7);

    } else {
        /* We're starting as a peripheral */
        mode = MODE_PERIPH;
        MAX_enableOptions(15, BIT3);
    }
}

void MAX_reset( void ) {
    /* Enable the reset */
    MAX_writeRegister(15, BIT5);

    /* Immediately clear the reset */
    MAX_writeRegister(15, 0);

    /* Wait a short while until the oscillator is stable */
    DELAY_WITH_TIMEOUT(!(MAX_readRegister(13) & BIT0));

    /* Reset the interrupt state */
    MAX_disableInterruptsMaster( );
    MAX_writeRegister(26, 0);
    MAX_writeRegister(25, 0xFF);
    enabledIRQ = 0;
    enabledEPIRQ = 0;
    ACKSTAT = false;
}

void MAX_enableInterrupts( uint_fast8_t flags ) {
    /* Enable the interrupts */
    if ( mode )
        MAX_enableOptions(26, flags);
    else
        MAX_enableOptions(14, flags);

    enabledIRQ |= flags;
}

void MAX_disableInterrupts( uint_fast8_t flags ) {
    /* Disable the interrupts */
    if ( mode )
        MAX_disableOptions(26, flags);
    else
        MAX_disableOptions(14, flags);

    enabledIRQ &= ~flags;
}

void MAX_enableEPInterrupts( uint_fast8_t flags ) {
    /* Enable the interrupts */
    if ( mode )
        return;
    else
        MAX_enableOptions(12, flags);

    enabledEPIRQ |= flags;
}

void MAX_disableEPInterrupts( uint_fast8_t flags ) {
    /* Disable the interrupts */
    if ( mode )
        return;
    else
        MAX_disableOptions(12, flags);

    enabledEPIRQ &= ~flags;
}

void MAX_clearInterruptStatus( uint_fast8_t flags ) {
    /* Clear the specified interrupts */
    MAX_enableOptions(25, flags);
}

void MAX_clearEPInterruptStatus( uint_fast8_t flags ) {
    /* Clear the specified interrupts */
    MAX_enableOptions(11, flags);
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
    if ( mode )
        return MAX_readRegister(rHIRQ);
    else
        return MAX_readRegister(rUSBIRQ);
}

uint_fast8_t MAX_getEnabledInterruptStatus( void ) {
    uint_fast8_t result = MAX_getInterruptStatus( );
    return result & enabledIRQ;
}

uint_fast8_t MAX_getEPInterruptStatus( void ) {
    return MAX_readRegister(11);
}

uint_fast8_t MAX_getEnabledEPInterruptStatus( void ) {
    uint_fast8_t result = MAX_getEPInterruptStatus( );
    return result & enabledEPIRQ;
}

uint_fast8_t MAX_writeRegister( uint_fast8_t address, uint_fast8_t value ) {
    uint_fast8_t result;

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW

    /* Build and transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_WRITE));
    /* Transmit the data */
    result = SIMSPI_transmitByte(value);

    /* End the transaction by pulling the CS back to high */
    SET_CS_HIGH

    return result;
}

uint_fast8_t MAX_writeRegisterAS( uint_fast8_t address, uint_fast8_t value ) {
    ACKSTAT = true;
    return MAX_writeRegister(address, value);
}

uint_fast8_t MAX_multiWriteRegister( uint_fast8_t address,
        uint_fast8_t * values, uint_fast8_t length ) {
    uint_fast8_t result;

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW

    /* Build and transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_WRITE));
    /* Transmit the data */
    result = SIMSPI_transmitBytes(values, length);

    /* End the transaction by pulling the CS back to high */
    SET_CS_HIGH

    return result;
}

uint_fast8_t MAX_readRegister( uint_fast8_t address ) {
    uint_fast8_t result;

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW

    /* Transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_READ));
    /* Transmit a 0, as we don't actually care about what's written but we do about the response */
    result = SIMSPI_transmitByte(0);

    /* End the transaction by pulling the CS back to high */
    SET_CS_HIGH

    return result;
}

void MAX_multiReadRegister( uint_fast8_t address, uint_fast8_t * buffer,
        uint_fast8_t length ) {

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW

    /* Transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_READ));
    /* Transmit a 0, as we don't actually care about what's written but we do about the response */
    SIMSPI_transmitBytesReadAll(buffer, (uint_fast8_t *) RXData, length);

    /* End the transaction by pulling the CS back to high */
    SET_CS_HIGH
}

void MAX_enableOptions( uint_fast8_t address, uint_fast8_t flags ) {
    /* Read the current state of the register */
    uint_fast8_t regVal = MAX_readRegister(address);

    /* Enable the given bits */
    regVal |= flags;

    /* Write the new register value back to the module */
    MAX_writeRegister(address, regVal);
}

void MAX_disableOptions( uint_fast8_t address, uint_fast8_t flags ) {
    /* Read the current state of the register */
    uint_fast8_t regVal = MAX_readRegister(address);

    /* Disable the given bits */
    regVal &= ~flags;

    /* Write the new register value back to the module */
    MAX_writeRegister(address, regVal);
}

/* PRIVATE FUNCTIONS */

uint_fast8_t _getCommandByte( uint_fast8_t address, uint_fast8_t direction ) {
    uint_fast8_t result = 0;
    result |= address << 3;
    result |= direction << 1;
    result |= ACKSTAT;
    ACKSTAT = false;
    return result;
}

/* INTERRUPT HANDLERS */

void GPIOP2_ISR( void ) {

    uint_fast8_t regval, USBStatus, USBEPStatus;

    /* Get the IQR status */
    USBStatus = MAX_getEnabledInterruptStatus( );
    USBEPStatus = MAX_getEnabledEPInterruptStatus( );

    //printf("Status: 0x%x, 0x%x\n", USBStatus, USBEPStatus);

    if ( USBEPStatus & MAX_IRQ_SUDAV ) {
        MAX_writeRegister(rEPIRQ, BIT5);
        MAX_multiReadRegister(4, (uint_fast8_t *) RXData, 8);
        switch ( RXData[1] ) {
        case reqSET_ADDRESS:
            ACKSTAT = true;
            MAX_readRegister(19);
            break;
        case reqGET_STATUS:
            USB_respondStatus((uint_fast8_t *) RXData);
            break;
        }
        //SysCtlDelay(5000);
        //regval = MAX_readRegister(19);
        printf("Address: 0x%x.\n", regval);
    }

    if ( USBStatus & MAX_IRQ_RCVDAV ) {
        MAX_writeRegister(rHIRQ, BIT2);
        printf("Received %d bytes!\n", MAX_readRegister(rRCVBC));
    }

    if ( USBStatus & MAX_IRQ_CONDET ) {
        /* Triggered on connect/disconnect of a peripheral */

        regval = MAX_readRegister(31);
        if ( regval & 0xC0 ) {
            peripheralConnected = 1;

            /* Turn on the blue LED */
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
            /* Enable the SOF generator */
            MAX_enableOptions(27, BIT3);

            MAX_enableOptions(rHCTL, BIT7);
            MAX_enableOptions(rHCTL, BIT5);

            USB_setNewPeripheralAddress(PERIPHERAL_ADDRESS);
            SysCtlDelay(2000);
            uint_fast8_t res;
            USB_requestStatus(&res);
            //printf("Done with enumeration!\n");

        } else {
            peripheralConnected = 0;
            /* Disable the SOF generator */
            MAX_disableOptions(27, BIT3);

            /* Turn off the blue LED */
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }
        MAX_writeRegister(rHIRQ, BIT5);
    }
}
