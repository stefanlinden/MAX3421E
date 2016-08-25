/*
 * MAX3421E.h
 *
 *  Created on: 24 Aug 2016
 *      Author: Stefan van der Linden
 */

#include <driverlib.h>
#include <stdio.h>

#include "simple_spi.h"

#ifndef INCLUDE_MAX3421E_H_
#define INCLUDE_MAX3421E_H_


/* DEFINES */

#define DIR_WRITE 1
#define DIR_READ  0


/* PROTOTYPES */

/**
 * Start the module and initialise
 *
 * Parameters:
 * uint_fast8_t startAsMaster: set to true if the module should be initialised as a USB master
 */
void MAX_start(uint_fast8_t);

/**
 * Reset the module by enabling CHIPRES
 */
void MAX_reset();

/**
 * Write to the specified register
 *
 * Parameters:
 * uint_fast8_t address: register address to write to
 * uint_fast8_t value: the value to write to the specified register
 *
 * Returns:
 * uint_fast8_t: the byte read concurrently on MISO while writing on MOSI
 */
uint_fast8_t MAX_writeRegister(uint_fast8_t, uint_fast8_t);

/**
 * Read from the specified register
 *
 * Parameters:
 * uint_fast8_t address: register address to read from
 *
 * Returns:
 * uint_fast8_t: the byte read
 */
uint_fast8_t MAX_readRegister(uint_fast8_t);

/**
 * Enable the specified bits
 *
 * Parameters:
 * uint_fast8_t address: register address to enable the bits in
 * uint_fast8_t flags: the bits to enable
 */
void MAX_enableOptions(uint_fast8_t, uint_fast8_t);

/**
 * Disable the specified bits
 *
 * Parameters:
 * uint_fast8_t address: register address to disable the bits in
 * uint_fast8_t flags: the bits to disable
 */
void MAX_disableOptions(uint_fast8_t, uint_fast8_t);


#endif /* INCLUDE_MAX3421E_H_ */
