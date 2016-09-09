/*
 * usb.c
 *
 *  Created on: 7 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "usb.h"
#include "packets.h"
#include "max3421e.h"
#include "delay.h"

/* Host functions */

uint_fast8_t USB_setNewPeripheralAddress( uint_fast8_t peraddress ) {

    ControlPacket addrPacket = { 0, /* perAddress */
    0x10, /* type */
    0, /* endPoint */
    0, /*bmRequestType*/
    0x05, /* bRequest */
    peraddress, /* wValue */
    0, /* wIndex */
    0, /* wLength */
    DIR_OUT /* direction */
    };
    return sendControl(&addrPacket);
}

uint_fast8_t USB_requestStatus( uint_fast8_t * resultBuffer ) {
    ControlPacket packet = { PERIPHERAL_ADDRESS, /* perAddress */
    0x10, /* type */
    0, /* endPoint */
    0x80, /*bmRequestType*/
    reqGET_STATUS, /* bRequest */
    0, /* wValue */
    0, /* wIndex */
    2, /* wLength */
    DIR_IN };
    return sendControl(&packet);
}

uint_fast8_t USB_doEnumeration( void ) {
    uint16_t tries = 0;
    MAX_enableOptions(rHCTL, BIT7);
    MAX_disableOptions(rHCTL, BIT6);
    MAX_enableOptions(rHCTL, BIT5);
    MAX_disableOptions(rHCTL, BIT4);

    while ( tries < 20 ) {
        if ( tries ) {
            printf("Enumeration failed. Retrying...\n");
            USB_busReset( );
            SysCtlDelay(4000000);
        }
        tries++;
        MAX_writeRegister(rPERADDR, 0);
        if ( !USB_setNewPeripheralAddress(PERIPHERAL_ADDRESS) ) {
            MAX_writeRegister(rPERADDR, PERIPHERAL_ADDRESS);
            SysCtlDelay(500000);
        } else {
            continue;
        }
        if ( !USB_requestStatus(0) )
            break;
    }
    if ( tries < 20 ) {
        MAX_enableOptions(rHCTL, BIT6);
        MAX_disableOptions(rHCTL, BIT7);
        MAX_enableOptions(rHCTL, BIT4);
        MAX_disableOptions(rHCTL, BIT5);
        return 0;
    } else {
        return 1;
    }
}

void USB_busReset( void ) {
    /* First disable the SOF generator */
    MAX_disableOptions(rMODE, BIT3);

    /* Perform the reset */
    MAX_enableOptions(rHCTL, BIT0);
    while ( !MAX_readRegister(rHCTL) & BIT0 ) {
        SysCtlDelay(10000);
    }

    /* Restart the SOF generator */
    MAX_enableOptions(rMODE, BIT3);

    /* Wait until the first SOF is transmitted */
    while ( !MAX_readRegister(rHIRQ) & BIT6 ) {
        SysCtlDelay(100);
    }
}

/* Peripheral functions */

void USB_respondStatus( uint_fast8_t * request ) {
    /* Based on example in http://pdfserv.maximintegrated.com/en/an/AN3690.pdf */

    uint_fast8_t testbyte;
    uint_fast8_t bytes[2];
    testbyte = request[0];
    switch ( testbyte ) {
    case 0x80: // directed to DEVICE
        bytes[0] = 0x03;
        bytes[1] = 0x00;
        MAX_readRegisterAS(rPERADDR);
        MAX_multiWriteRegister(rEP0FIFO, bytes, 2);
        //MAX_writeRegister(rEP0FIFO, 0x03); // first byte is 000000rs
        // where r=enabled for RWU and s=self-powered.
        //MAX_writeRegister(rEP0FIFO, 0x00); // second byte is always 0
        MAX_writeRegister(rEP0BC, 2);
        break; // load byte count, arm the IN transfer,
        // ACK the status stage of the CTL transfer
    case 0x81: // directed to INTERFACE
        MAX_writeRegister(rEP0FIFO, 0x00); // this one is easy--two zero bytes
        MAX_writeRegister(rEP0FIFO, 0x00);
        MAX_writeRegisterAS(rEP0BC, 2);
        break; // load byte count, arm the IN transfer,
        // ACK the status stage of the CTL transfer
        //default:
        //USB_stallEndpoint(0);
        // don’t understand the request
    }
}
