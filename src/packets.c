/*
 * packets.c
 *
 *  Created on: 7 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "max3421e.h"
#include "usb.h"
#include "packets.h"
//#include "simple_spi.h"

volatile uint_fast8_t TXData[BUFFER_SIZE];

uint_fast8_t transmitPacket( uint_fast8_t token, uint_fast8_t ep ) {
    uint_fast8_t regval;
    uint16_t timeout;

    /* Instruct the module to send the data as the specified type */
    MAX_writeRegister(rHXFR, token | ep);

    timeout = 0xFFFF;
    while ( timeout ) {
        timeout--;
        regval = MAX_readRegister(rHRSL) & 0x0F;
        if ( regval == rslNAK )
            MAX_writeRegister(rHXFR, token | ep);
        else if ( regval != rslBUSY )
            break;
    }

    if ( regval )
        printf("Error or timeout: 0x%x.\n", regval);

    return regval;
}

void sendControl( ControlPacket * packet ) {
    uint_fast8_t rescode;

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
    printf("Sending bRequest: 0x%x.\n", packet->bRequest);

    /* Start the transaction */
    rescode = transmitPacket(0x10, 0);

    /* Check whether we need a data stage (request only at the moment) */
    if ( packet->wLength > 0 && packet->direction == DIR_IN ) {
        rescode = transmitPacket(xfrIN, 0);
        printf("Result: 0x%x\n", rescode);
    }

    /* Send an HS-IN or HS-OUT. */
    if ( packet->direction == DIR_OUT ) {
        rescode = transmitPacket(0x80, 0);
    } else {
        rescode = transmitPacket(0xA0, 0);
    }
    //printf("HCTL: 0x%x\n", MAX_readRegister(rHCTL));
}
