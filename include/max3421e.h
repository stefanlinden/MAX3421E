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
 * Start the module
 */
void MAX_start();

/**
 * Write to the specified register
 *
 * Parameters:
 * uint_fast8_t address: register address to write to
 * uint_fast8_t value: the value to write to the specified register
 */
uint_fast8_t MAX_writeRegister(uint_fast8_t, uint_fast8_t);

/**
 * Read from the specified register
 *
 * Parameters:
 * uint_fast8_t address: register address to read from
 */
uint_fast8_t MAX_readRegister(uint_fast8_t);


#endif /* INCLUDE_MAX3421E_H_ */
