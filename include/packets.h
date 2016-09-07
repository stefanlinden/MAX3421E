/*
 * packets.h
 *
 *  Created on: 7 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_PACKETS_H_
#define INCLUDE_PACKETS_H_

#include "max3421e.h"

uint_fast8_t transmitPacket( uint_fast8_t, uint_fast8_t );

/**
 * Send the given Setup Packet
 *
 * Parameters:
 * SetupPacket * packet: a reference to the packet to transmit
 */
void sendControl( ControlPacket * );


#endif /* INCLUDE_PACKETS_H_ */
