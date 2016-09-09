//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include <stdio.h>
#include <driverlib.h>

#include "max3421e.h"
#include "usb.h"
#include "delay.h"

#define ISMASTER

volatile uint8_t isMaster;
volatile uint_fast8_t bulkData[64] = { 1, 2, 3, 4, 5, 6, 7, 8, 9,
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        1 };
volatile uint_fast8_t i = 0;

void main( void ) {

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

#ifdef ISMASTER
    isMaster = true;

    /* Use the blue led to indicate we have contact */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    MAX_start(true);

    /* Enable interrupts */
    MAX_enableInterrupts(MAX_IRQ_CONDET);
    MAX_clearInterruptStatus(MAX_IRQ_CONDET);
    MAX_enableInterruptsMaster();

    uint_fast8_t regval = MAX_readRegister(rREVISION);
    printf("Revision: 0x%x\n", regval);

    regval = MAX_readRegister(25);
    printf("Value of register 25: 0x%x\n", regval);

#else
    isMaster = false;

    MAX_start(false);

    MAX_enableEPInterrupts(MAX_IRQ_SUDAV);
    MAX_clearEPInterruptStatus(MAX_IRQ_SUDAV);
    MAX_enableInterrupts(MAX_IRQ_URESDN);
    MAX_clearInterruptStatus(MAX_IRQ_URESDN);
    //printf("EPIEN: 0x%x\n", MAX_readRegister(rEPIEN));
    //printf("EPIRQ: 0x%x\n", MAX_readRegister(rEPIRQ));
    //printf("CLRTOGS: 0x%x\n", MAX_readRegister(rCLRTOGS));
    MAX_enableInterruptsMaster( );
    //SysCtlDelay(1000);


#endif

    while ( 1 ) {
        if ( isMaster ) {
            /* The host can sleep until needed */
            MAP_PCM_gotoLPM0InterruptSafe( );
        } else {
            /* The peripheral/slave needs to keep the buffer filled */
            if ( MAX_readRegister(rEPIRQ) & MAX_IRQ_IN2BAV ) {
                bulkData[0] = i;
                bulkData[1] = i+1;
                i++;
                if(i == 9)
                    i = 0;

                //printf("Writing: %d - %d\n", bulkData[0], bulkData[1]);
                MAX_multiWriteRegister(rEP2INFIFO, (uint_fast8_t *) bulkData,
                        64);
                MAX_writeRegister(rEP2INBC, 64);
                MAX_enableOptions(rUSBCTL, BIT3);
            } else {
                MAX_enableEPInterrupts(MAX_IRQ_IN2BAV);
                MAP_PCM_gotoLPM0InterruptSafe( );
            }
        }
    }
}
