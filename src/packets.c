/*
 * packets.c
 *
 *  Created on: 7 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "max3421e.h"
#include "usb.h"
#include "packets.h"
#include "delay.h"

volatile uint_fast8_t TXData[BUFFER_SIZE];

uint_fast8_t transmitPacket( uint_fast8_t token, uint_fast8_t ep ) {
    uint_fast8_t regval;
    uint16_t timeout;

    /* Instruct the module to send the data as the specified type */
    MAX_writeRegister(rHXFR, token | ep);

    timeout = 0xFFFF;
    while ( timeout ) {
        regval = MAX_readRegister(rHRSL) & 0x0F;
        if ( regval == rslBUSY ) {
            SysCtlDelay(100);
        } else if ( regval == rslNAK ) {
            timeout--;
            MAX_writeRegister(rHXFR, token | ep);
            SysCtlDelay(200);
        } else {
            break;
        }
    }
    //regval = MAX_readRegister(rHRSL) & 0x0F;
    if ( regval )
        printf("Error or timeout: 0x%x.\n", regval);

    return regval;
}

uint_fast8_t sendControl( ControlPacket * packet ) {
    uint_fast8_t rescode;
    uint_fast8_t timeout;

    /* Make sure the peripheral address is correct */
    MAX_writeRegister(rPERADDR, packet->perAddress);

    /* Load the contents from the given packet and send this as a Control packet */
    TXData[0] = packet->bmRequestType;
    TXData[1] = packet->bRequest;
    TXData[2] = (uint_fast8_t) (packet->wValue);
    TXData[3] = (uint_fast8_t) (packet->wValue >> 8);
    TXData[4] = (uint_fast8_t) (packet->wIndex);
    TXData[5] = (uint_fast8_t) (packet->wIndex >> 8);
    TXData[6] = (uint_fast8_t) (packet->wLength);
    TXData[7] = (uint_fast8_t) (packet->wLength >> 8);

    /* Write the data into the DUPFIFO */
    MAX_multiWriteRegister(4, (uint_fast8_t *) TXData, 8);
    printf("Sending bRequest: 0x%x. (addr %d)\n", packet->bRequest,
            packet->perAddress);

    /* Start the transaction */
    rescode = transmitPacket(0x10, 0);
    if ( rescode )
        return rescode;

    /* Check whether we need a data stage (request only at the moment) and perform */
    if ( packet->wLength > 0 && packet->direction == DIR_IN ) {
        rescode = transmitPacket(xfrIN, 0);
        if ( rescode )
            return rescode;
        timeout = 0xFFFF;
        while ( (MAX_readRegister(rHIRQ) & MAX_IRQ_RCVDAV ) && timeout ) {
            timeout--;
            SysCtlDelay(5);
        }

        /* Check if we got data and read if available */
        if ( MAX_readRegister(rHIRQ) & MAX_IRQ_RCVDAV ) {
            uint8_t readlength = MAX_readRegister(rRCVBC);
            printf("Got control data: %d bytes\n", readlength);
            MAX_writeRegister(rHIRQ, MAX_IRQ_RCVDAV);
        }
    }

    /* Send an HS-IN or HS-OUT. */
    if ( packet->direction == DIR_OUT ) {
        rescode = transmitPacket(0x80, 0);
    } else {
        rescode = transmitPacket(0xA0, 0);
    }
    return rescode;
}

uint_fast8_t requestData( uint_fast8_t * rxbuffer, uint_fast8_t nbytes) {
    uint_fast8_t it, timeout, readlength, result;

    /* Send a BULK-IN request packet */
    transmitPacket(xfrIN, 2);

    /* Wait until we have a reply, or timeout */
    timeout = 0xFF;
    while ( (!MAX_readRegister(rHIRQ) & MAX_IRQ_RCVDAV ) && timeout ) {
        SysCtlDelay(300);
        timeout--;
    }

    /* Quit if we got a timeout */
    if(!timeout)
        return 0xFF;

    /* Get the length of the received data (should be the same as nbytes) */
    readlength = MAX_readRegister(rRCVBC);
    if(readlength != nbytes)
        return 0x0F;
    //totalRcvd += readlength;

    /* This delay is apparently necessary, as the RX FIFO isn't directly ready (first byte will randomly corrupt) */
    SysCtlDelay(500);

    /* Check the transfer result code: if it's an error, then quit */
    result = MAX_readRegister(rHRSL) & 0x0F;
    if ( result && result != rslBUSY ) {
        return result;
    }

    /* No error, so read the actual data */
    MAX_multiReadRegister(rRCVFIFO, rxbuffer, readlength);

    /* A simple test for now */
    /* TODO remove! */
    for ( it = 0; it < 64; it++ ) {
        if ( rxbuffer[it] > 9 )
            printf("Byte error: %d (%d)...\n", rxbuffer[it], it);
    }

    /* Clear the interrupt */
    MAX_writeRegister(rHIRQ, MAX_IRQ_RCVDAV);

    return 0;
}
