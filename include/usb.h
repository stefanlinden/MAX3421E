/*
 * usb.h
 *
 *  Created on: 5 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_USB_H_
#define INCLUDE_USB_H_

#include <stdint.h>
#include <stdbool.h>

/* It is important that for the the standard peripheral address it is assumed only
 * a single peripheral is connected
 */
#define PERIPHERAL_ADDRESS 5

#define DIR_IN      0
#define DIR_OUT      1

/* Transfer Tokens */
#define xfrSETUP    0x10
#define xfrIN       0x00
#define xfrOUT      0x20
#define xfrINHS     0x80
#define xfrOUTHS    0xA0
#define xfrISOIN    0x40
#define xfrISOOUT   0x60

/* Result Codes */
#define rslSUCCES   0x00
#define rslBUSY     0x01
#define rslBADREQ   0x02
#define rslUNDEF    0x03
#define rslNAK      0x04
#define rslSTALL    0x05
#define rslTOGERR   0x06
#define rslWRONGPID 0x07
#define rslBADBC    0x08
#define rslPIDERR   0x09
#define rslPKTERR   0x0A
#define rslCRCERR   0x0B
#define rslKERR     0x0C
#define rslJERR     0x0D
#define rslTIMEOUT  0x0E
#define rslBABBLE   0x0F

/* Standard Requests */
#define reqGET_STATUS           0x00
#define reqCLEAR_FEATURE        0x01
#define reqSET_FEATURE          0x03
#define reqSET_ADDRESS          0x05
#define reqGET_DESCRIPTOR       0x06
#define reqSET_DESCRIPTOR       0x07
#define reqGET_CONFIGURATION    0x08
#define reqSET_CONFIGURATION    0x09

typedef struct {
    uint_fast8_t perAddress;
    uint_fast8_t type;
    uint_fast8_t endPoint;
    uint_fast8_t bmRequestType;
    uint_fast8_t bRequest;
    uint_fast16_t wValue;
    uint_fast16_t wIndex;
    uint_fast16_t wLength;
    uint_fast8_t direction;
} ControlPacket;

/* Host Prototypes */

/**
 * Perform a GET_STATUS request
 *
 * Parmeters:
 * uint_fast8_t * resultBuffer: a two-byte array for storing the result
 */
void USB_requestStatus(uint_fast8_t * );

/**
 * Send a Set_Address Control packet
 *
 * Parameters:
 * uint_fast8_t peraddress: the new peripheral address
 */
void USB_setNewPeripheralAddress(uint_fast8_t);


/* Peripheral Prototypes */

/**
 * Stalls the given endpoint
 *
 * Parameters:
 * uint_fast8_t ep: the endpoint to stall
 */
void USB_stallEndpoint( uint_fast8_t );

/** Takes a GET_STATUS requests and responds to it
 *
 * Parameters:
 * uint_fast8_t * request: a pointer to the received request
 * */
void USB_respondStatus( uint_fast8_t * );

#endif /* INCLUDE_USB_H_ */
