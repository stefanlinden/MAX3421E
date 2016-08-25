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

#define MAX_IRQ_BUSEVENT    BIT0
#define MAX_IRQ_RWU         BIT1
#define MAX_IRQ_RCVDAV      BIT2
#define MAX_IRQ_SNDBAV      BIT3
#define MAX_IRQ_SUSDN       BIT4
#define MAX_IRQ_CONDET      BIT5
#define MAX_IRQ_FRAME       BIT6
#define MAX_IRQ_HXFRDN      BIT7

/* Change this GPIO pin to change the interrupt pin */
#define USBINT_PORT GPIO_PORT_P2
#define USBINT_PIN  GPIO_PIN3

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
void MAX_reset( void );

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

/**
 * Enable interrupts
 *
 * Parameters:
 * uint_fast8_t flags: the interrupts to enable
 */
void MAX_enableInterrupts(uint_fast8_t);

/**
 * Disable interrupts
 *
 * Parameters:
 * uint_fast8_t flags: the interrupts to disable
 */
void MAX_disableInterrupts(uint_fast8_t);

/**
 * Clear the interrupt statuses
 *
 * Parameters:
 * uint_fast8_t flags: the interrupts to clear
 */
void MAX_clearInterruptStatus(uint_fast8_t);

/**
 * Enable the interrupt pin
 */
void MAX_enableInterruptsMaster( void );

/**
 * Disable the interrupt pin
 */
void MAX_disableInterruptsMaster( void );

/**
 * Get the status of the interrupts
 *
 * Returns:
 * uint_fast8_t: a byte containing the status of the interrupts
 */
uint_fast8_t MAX_getInterruptStatus( void );

/**
 * Get the status of enabled interrupts
 *
 * Returns:
 * uint_fast8_t: a byte containing the status of the enabled interrupts
 */
uint_fast8_t MAX_getEnabledInterruptStatus( void );

#endif /* INCLUDE_MAX3421E_H_ */
