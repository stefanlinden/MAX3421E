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

    MAX_start( );
    MAX_writeRegister(17, 0x18);

    uint_fast8_t regval = MAX_readRegister(17);
    printf("Value of register 17: 0x%x", regval);

    //while ( 1 ) {
    //    MAP_PCM_gotoLPM0InterruptSafe( );
    //}
}
