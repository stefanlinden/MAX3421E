//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include <stdio.h>
#include <driverlib.h>

#include "max3421e.h"

#define ISMASTER

void main( void ) {

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

#ifdef ISMASTER
    /* Use the blue led to indicate we have contact */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    MAX_start(true);

    /* Enable interrupts */
    MAX_clearInterruptStatus(MAX_IRQ_CONDET | MAX_IRQ_RCVDAV | MAX_IRQ_HXFRDN);
    MAX_enableInterrupts(MAX_IRQ_CONDET | MAX_IRQ_RCVDAV | MAX_IRQ_HXFRDN);
    MAX_enableInterruptsMaster();

    uint_fast8_t regval = MAX_readRegister(13);
    printf("Value of register 13: 0x%x\n", regval);

    regval = MAX_readRegister(25);
    printf("Value of register 25: 0x%x\n", regval);

#else
    MAX_start(false);
    MAX_enableEPInterrupts(MAX_IRQ_SUDAV);
    MAX_clearEPInterruptStatus(MAX_IRQ_SUDAV);
    MAX_enableInterruptsMaster();
#endif

    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
}
