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

extern volatile uint_fast8_t RXData[];
volatile uint_fast8_t mode;
volatile uint_fast8_t TXSize;

volatile bool ACKSTAT;

volatile uint_fast8_t enabledIRQ;
volatile uint_fast8_t enabledEPIRQ;
volatile uint_fast8_t peripheralConnected;
volatile uint_fast8_t lastTransferResult;

volatile void (*handlePtr)( uint_fast8_t );

/* PROTOTYPES FOR PRIVATE FUNCTIONS */

uint_fast8_t _getCommandByte( uint_fast8_t, uint_fast8_t );

/* PUBLIC FUNCTIONS */

void MAX_start( uint_fast8_t startAsMaster ) {
    mode = startAsMaster;

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

        /* Enable HOST, DMPULLDN and DPPULLDN */
        MAX_enableOptions(27, BIT0 | BIT6 | BIT7);

    } else {
        /* We're starting as a peripheral */
        //MAX_enableOptions(rUSBCTL, BIT3);
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
    if ( mode ) {
        MAX_writeRegister(rHIEN, 0);
        MAX_writeRegister(rHIRQ, 0xFF);
    } else {
        MAX_writeRegister(rEPIEN, 0);
        MAX_writeRegister(rEPIRQ, 0xFF);
        MAX_writeRegister(rUSBIEN, 0);
        MAX_writeRegister(rUSBIRQ, 0xFF);
    }
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
    if ( mode )
        MAX_enableOptions(25, flags);
    else
        MAX_enableOptions(13, flags);
}

void MAX_clearEPInterruptStatus( uint_fast8_t flags ) {
    /* Clear the specified interrupts */
    MAX_writeRegister(rEPIRQ, flags);
}

void MAX_enableInterruptsMaster( void ) {
    /* Set IE to 1 */
    MAX_writeRegister(16, BIT0);
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
    return MAX_readRegister(rEPIRQ);
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

uint_fast8_t MAX_readRegisterAS( uint_fast8_t address ) {
    ACKSTAT = true;
    return MAX_readRegister(address);
}

void MAX_multiReadRegister( uint_fast8_t address, uint_fast8_t * buffer,
        uint_fast8_t length ) {

    /* Start the transaction by pulling the CS low */
    SET_CS_LOW

    /* Transmit the command byte */
    SIMSPI_transmitByte(_getCommandByte(address, DIR_READ));

    /* Transmit 0s, as we don't actually care about what's written but we do about the response */
    SIMSPI_readBytes(buffer, length);

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

void MAX_setStateChangeIRQ( void (*handle)( uint_fast8_t ) ) {
    /* Assign the given handler to the local pointer */
    handlePtr = (volatile void (*)( uint_fast8_t )) handle;
}

uint_fast8_t MAX_scanBus( void ) {
    /* Enable SAMPLEBUS */
    MAX_enableOptions(rHCTL, BIT2);
    while ( !(MAX_readRegister(rHCTL) & BIT2 ) )
        SysCtlDelay(200);
    /* Return the J/K state bits */
    return (MAX_readRegister(rHRSL) & 0xC0) >> 6;
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

    /* Peripheral: we got a setup package */
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
    }

    /* Peripheral: the buffer is available again */
    if ( USBEPStatus & MAX_IRQ_IN2BAV ) {
        MAX_disableEPInterrupts(MAX_IRQ_IN2BAV);
    }

    /* Peripheral: a bus reset was commanded */
    if ( !mode && USBStatus & MAX_IRQ_URESDN ) {
        MAX_writeRegister(rUSBIRQ, MAX_IRQ_URESDN);

        /* Reconfigure the interrupts after a reset */
        MAX_enableEPInterrupts(MAX_IRQ_SUDAV);
        MAX_clearEPInterruptStatus(MAX_IRQ_SUDAV);
        MAX_enableInterrupts(MAX_IRQ_URESDN);
        MAX_clearInterruptStatus(MAX_IRQ_URESDN);

        /* Re-enable EP2 */
        MAX_writeRegister(rEP2INBC, 64);
    }

    /* Host: a peripheral connected or disconnected */
    if ( mode && USBStatus & MAX_IRQ_CONDET ) {

        regval = MAX_readRegister(31);
        if ( regval & 0xC0 ) {
            peripheralConnected = 1;

            /* Turn on the blue LED */
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);

            /* Enable the SOF generator */
            MAX_enableOptions(27, BIT3);
            while ( !(MAX_readRegister(rHIRQ) & MAX_IRQ_FRAME ) )
                ;

            USB_busReset( );

            SysCtlDelay(4000000);

            if ( USB_doEnumeration( ) )
                printf("Enumeration Failed...\n");
            else
                printf("Done with enumeration!\n");

            /* Add a delay to stabilise the bus */
            SysCtlDelay(8000000);
        } else {
            peripheralConnected = 0;
            /* Disable the SOF generator */
            MAX_disableOptions(27, BIT3);

            /* Turn off the blue LED */
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }

        if ( handlePtr != 0 )
            handlePtr((uint_fast8_t) peripheralConnected);

        MAX_writeRegister(rHIRQ, BIT5);

    }
    MAP_GPIO_clearInterruptFlag(USBINT_PORT, USBINT_PIN);
}
