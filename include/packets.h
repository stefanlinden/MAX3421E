/*
 * controlpacket.h
 *
 *  Created on: 25 Aug 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_PACKETS_H_
#define INCLUDE_PACKETS_H_

#include <stdint.h>

typedef struct {
    uint_fast8_t perAddress;
    uint_fast8_t type;
    uint_fast8_t endPoint;
    uint_fast8_t bmRequestType;
    uint_fast8_t bRequest;
    uint_fast16_t wValue;
    uint_fast16_t wIndex;
    uint_fast16_t wLength;
} ControlPacket;

#endif /* INCLUDE_PACKETS_H_ */
