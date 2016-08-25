//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include <stdio.h>
#include <driverlib.h>

#include "max3421e.h"

void main( void ) {

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    MAX_start(true);

    /* Enable interrupts */
    MAX_enableInterrupts(MAX_IRQ_CONDET);
    MAX_clearInterruptStatus(MAX_IRQ_CONDET);
    MAX_enableInterruptsMaster();

    uint_fast8_t regval = MAX_readRegister(13);
    printf("Value of register 13: 0x%x\n", regval);

    regval = MAX_readRegister(25);
    printf("Value of register 25: 0x%x\n", regval);
    MAX_writeRegister(25, 0xFF);


    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
}
