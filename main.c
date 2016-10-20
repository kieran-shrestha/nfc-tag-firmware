#include "myclocks.h"
#include <driverlib.h>
#include "tmp112.h"
#include "rf430nfc.h"
float temp;
int main(void) {

	unsigned int pTempData;
	unsigned char ui8RxData[2];


    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    initClocks();				// set aclk 32, smclk 1M, mclk 1M

    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6 );                             // Red LED (LED1)
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6 );

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6 );                             // Red LED (LED1)
    GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN6 );

    P1SEL1 |= (1<<6);
	P1SEL1 |= (1<<7);

    PMM_unlockLPM5();
    TMP_I2C_Init();
   // RF430_I2C_Init();

    while(1){


    			UCB0CTL1  |= UCSWRST;
    			UCB0CTLW1 = UCASTP_2;  // generate STOP condition.
    			UCB0TBCNT = 0x0001;
    			UCB0CTL1  &= ~UCSWRST;

    			UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
    			while(!(UCB0IFG & UCTXIFG0));
    			UCB0TXBUF = 0x00 & 0xFF;
    			while(!(UCB0IFG & UCBCNTIFG));

    			UCB0CTL1 &= ~UCTR;
    			UCB0CTLW1 = UCASTP_2;  			// generate STOP condition.
    			UCB0TBCNT = 0x0002;
    			UCB0CTL1 |= UCTXSTT; 			// Repeated start

    			while(!(UCB0IFG & UCRXIFG0));
    			ui8RxData[1] = UCB0RXBUF;
    			while(!(UCB0IFG & UCRXIFG0));
    			ui8RxData[0] = UCB0RXBUF;
    			while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
    			UCB0CTL1  |= UCSWRST;

    			pTempData = (0x0FF0 & (ui8RxData[1] << 4)) | (0x0F & (ui8RxData[0] >> 4 ));

    			pTempData = 0x07FF & pTempData;
    			temp = pTempData * 0.0625;

    			_delay_cycles(10000);

    }

	return 0;
}
