//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include <driverlib.h>

#include "max3421e.h"
#include "usb.h"
#include "packets.h"
#include "delay.h"

#define ISMASTER

volatile bool isMaster, peripheralAvailable;
volatile uint_fast8_t RXData[BUFFER_SIZE];

volatile uint_fast8_t bulkData[64] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2,
        3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8,
        9, 1 };
volatile uint_fast8_t i = 0;

void busStateChanged( uint_fast8_t );

void main( void ) {

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

#ifdef ISMASTER
    isMaster = true;

    /* Use the blue led to indicate we have contact */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    MAX_start(true);

    MAX_setStateChangeIRQ(&busStateChanged);

    /* Enable interrupts */
    MAX_enableInterrupts(MAX_IRQ_CONDET);
    MAX_clearInterruptStatus(MAX_IRQ_CONDET);
    MAX_enableInterruptsMaster( );

    uint_fast8_t regval = MAX_readRegister(rREVISION);
    printf("Revision: 0x%x\n", regval);

    /* Perform a bus reset to reconnect after a power down */
    if ( !peripheralAvailable )
        USB_busReset( );

    while ( 1 ) {
        if ( peripheralAvailable ) {
            /* Make sure the RX buffer is free */
            MAX_writeRegister(rHIRQ, BIT2);

            uint_fast8_t i, result;
            uint_fast32_t totalRcvd = 0;

            printf("Requesting data...\n");
            printf("(Address: %d)\n", MAX_readRegister(rPERADDR));
            MAX_writeRegister(rHIRQ, BIT2);
            for ( i = 0; i < 1000; i++ ) {
                result = requestData((uint_fast8_t *) RXData, 64);
                if ( result != 0 ) {
                    printf("Result error: 0x%x (%d)\n", result, i);
                } else {
                    totalRcvd += 64;
                }
            }
            printf("Received %d bytes!\n", totalRcvd);
        }
        /* The host can sleep until needed */
        MAP_PCM_gotoLPM0InterruptSafe( );

    }

#else
    isMaster = false;

    MAX_start(false);

    /* Enable the interrupts */
    MAX_enableEPInterrupts(MAX_IRQ_SUDAV);
    MAX_clearEPInterruptStatus(MAX_IRQ_SUDAV);
    MAX_enableInterrupts(MAX_IRQ_URESDN);
    MAX_clearInterruptStatus(MAX_IRQ_URESDN);
    MAX_enableInterruptsMaster( );

    while ( 1 ) {
        if ( MAX_readRegister(rEPIRQ) & MAX_IRQ_IN2BAV ) {
            bulkData[0] = i;
            bulkData[1] = i + 1;
            i++;
            if ( i == 9 )
            i = 0;

            //printf("Writing: %d - %d\n", bulkData[0], bulkData[1]);
            MAX_multiWriteRegister(rEP2INFIFO, (uint_fast8_t *) bulkData, 64);
            MAX_writeRegister(rEP2INBC, 64);
            MAX_enableOptions(rUSBCTL, BIT3);
        } else {
            MAX_enableEPInterrupts(MAX_IRQ_IN2BAV);
            MAP_PCM_gotoLPM0InterruptSafe( );
        }
    }

#endif
}

void busStateChanged( uint_fast8_t newState ) {
    uint_fast8_t result = MAX_scanBus( );
    if ( result == 0x01 || result == 0x02 )
        peripheralAvailable = true;
    else
        peripheralAvailable = false;
}
