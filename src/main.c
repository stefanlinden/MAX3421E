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
    MAX_reset();

    uint_fast8_t regval = MAX_readRegister(17);
    printf("Value of register 17: 0x%x\n", regval);

    regval = MAX_readRegister(27);
    printf("Value of register 27 before edit: 0x%x\n", regval);

    MAX_enableOptions(27, BIT0 | BIT6 | BIT7);

    regval = MAX_readRegister(27);
    printf("Value of register 27 after edit: 0x%x\n", regval);

    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
}
