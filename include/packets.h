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
 *
 * \param packet: a reference to the packet to transmit
 * \return result code
 */
uint_fast8_t sendControl( ControlPacket * );

/**
 * Request data and checks whether it is the correct amount
 *
 * Parameters:
 * uint_fast8_t * rxbuffer: a buffer to hold the data
 * uint_fast8_t nbytes: the amount of bytes
 */
uint_fast8_t requestData( uint_fast8_t *, uint_fast8_t);


#endif /* INCLUDE_PACKETS_H_ */
