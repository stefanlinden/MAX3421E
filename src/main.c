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

    MAX_start(1);

    uint_fast8_t regval = MAX_readRegister(13);
    printf("Value of register 13: 0x%x\n", regval);

    regval = MAX_readRegister(27);
    printf("Value of register 27: 0x%x\n", regval);


    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
}
