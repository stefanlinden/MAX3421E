/*
 * usb.c
 *
 *  Created on: 5 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "usb.h"
#include "max3421e.h"

bool ep0stall;
bool ep1stall;
bool ep2stall;
bool ep3stall;

/** Prototypes for private functions */
void _sendControlPacket( SetupPacket * );

/* Host Functions */
void USB_requestStatus( uint_fast8_t * resultBuffer ) {
    SetupPacket packet = { PERIPHERAL_ADDRESS, /* perAddress */
    0x10, /* type */
    0, /* endPoint */
    0x80, /*bmRequestType*/
    reqGET_STATUS, /* bRequest */
    0, /* wValue */
    0, /* wIndex */
    2, /* wLength */
    DIR_OUT };
    MAX_sendControlPacket(&packet);
}

void USB_setNewPeripheralAddress( uint_fast8_t peraddress ) {
    SetupPacket addrPacket = { 0, /* perAddress */
    0x10, /* type */
    0, /* endPoint */
    0, /*bmRequestType*/
    0x05, /* bRequest */
    peraddress, /* wValue */
    0, /* wIndex */
    0, /* wLength */
    DIR_IN };
    _sendControlPacket(&addrPacket);
}

/** Peripheral Functions */

void USB_stallEndpoint( uint_fast8_t ep ) {
    switch ( ep ) {
    case 0:
        ep0stall = true;
        MAX_enableOptions(rEPSTALLS, 0x03);
        break;
    case 1:
        ep2stall = true;
        MAX_enableOptions(rEPSTALLS, 0x04);
        break;
    case 2:
        ep2stall = true;
        MAX_enableOptions(rEPSTALLS, 0x08);
        break;
    case 3:
        ep3stall = true;
        MAX_enableOptions(rEPSTALLS, 0x10);
        break;
    }
}

void USB_respondStatus( uint_fast8_t * request ) {
    /* Based on example in http://pdfserv.maximintegrated.com/en/an/AN3690.pdf */
    uint_fast8_t testbyte;
    testbyte = request[0];
    switch ( testbyte ) {
    case 0x80: // directed to DEVICE
        MAX_writeRegister(rEP0FIFO, 0x03); // first byte is 000000rs
        // where r=enabled for RWU and s=self-powered.
        MAX_writeRegister(rEP0FIFO, 0x00); // second byte is always 0
        MAX_writeRegisterAS(rEP0BC, 2);
        break; // load byte count, arm the IN transfer,
        // ACK the status stage of the CTL transfer
    case 0x81: // directed to INTERFACE
        MAX_writeRegister(rEP0FIFO, 0x00); // this one is easy--two zero bytes
        MAX_writeRegister(rEP0FIFO, 0x00);
        MAX_writeRegisterAS(rEP0BC, 2);
        break; // load byte count, arm the IN transfer,
        // ACK the status stage of the CTL transfer
    case 0x82: // directed to ENDPOINT
        if ( request[4] == 0x80 ) {
            MAX_writeRegister(rEP0FIFO, ep0stall); // first byte is 0000000h where h is the halt bit
        } else if ( request[4] == 0x81 ) {
            MAX_writeRegister(rEP0FIFO, ep1stall); // first byte is 0000000h where h is the halt bit
        } else if ( request[4] == 0x82 ) {
            MAX_writeRegister(rEP0FIFO, ep2stall); // first byte is 0000000h where h is the halt bit
        } else if ( request[4] == 0x83 ) {
            MAX_writeRegister(rEP0FIFO, ep3stall); // first byte is 0000000h where h is the halt bit
        } else {
            USB_stallEndpoint(0);
            break;
        }

        MAX_writeRegister(rEP0FIFO, 0x00); // second byte is always 0
        MAX_writeRegisterAS(rEP0BC, 2);
        break;
    default:
        USB_stallEndpoint(0);
        // don’t understand the request
    }
}

