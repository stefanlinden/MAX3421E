/*
 * usb.c
 *
 *  Created on: 7 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "usb.h"
#include "max3421e.h"


/* Host functions */

void USB_setNewPeripheralAddress( uint_fast8_t peraddress ) {

    ControlPacket addrPacket = { 0, /* perAddress */
    0x10, /* type */
    0, /* endPoint */
    0, /*bmRequestType*/
    0x05, /* bRequest */
    peraddress, /* wValue */
    0, /* wIndex */
    0, /* wLength */
    DIR_IN /* direction */
    };
    MAX_sendControlPacket(&addrPacket);
}

void USB_requestStatus( uint_fast8_t * resultBuffer ) {
    ControlPacket packet = { PERIPHERAL_ADDRESS, /* perAddress */
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


/* Peripheral functions */

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
    //default:
        //USB_stallEndpoint(0);
        // don’t understand the request
    }
}
