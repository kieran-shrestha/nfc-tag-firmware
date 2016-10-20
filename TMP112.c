#include <driverlib.h>
#include "tmp112.h"

void TMP_I2C_Init(void){
		UCB0CTL1 |= UCSWRST;	            			//Software reset enabled
		UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR;	//I2C mode, Master mode, sync, transmitter
		UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 4MHz

		UCB0BRW = 10; 									// Baudrate = SMLK/40 = 400kHz

		UCB0I2CSA  = TMP112_I2C_ADDR;					// Set Slave Address
		UCB0IE = 0;
		UCB0CTL1 &= ~UCSWRST;
}

unsigned int getTemperature(){
	unsigned int pTempData;
	unsigned char ui8RxData[2];

				UCB0CTL1  |= UCSWRST;
	    		UCB0CTLW1 = UCASTP_2;  // generate STOP condition.
	    		UCB0TBCNT = 0x0001;
	    		UCB0CTL1  &= ~UCSWRST;
//	_delay_cycles(1000);

	    		UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
	    		while(!(UCB0IFG & UCTXIFG0));
	    		UCB0TXBUF = 0x00 & 0xFF;
	    		while(!(UCB0IFG & UCBCNTIFG));

	    		UCB0CTL1 &= ~UCTR;
	    		UCB0CTLW1 = UCASTP_2;  			// generate STOP condition.
	    		UCB0TBCNT = 0x0002;
	    		UCB0CTL1 |= UCTXSTT; 			// Repeated start
	//_delay_cycles(1000);

	    		while(!(UCB0IFG & UCRXIFG0));
	    		ui8RxData[1] = UCB0RXBUF;
	    		while(!(UCB0IFG & UCRXIFG0));
	    		ui8RxData[0] = UCB0RXBUF;
	    		while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	    		UCB0CTL1  |= UCSWRST;

	    		pTempData = (0x0FF0 & (ui8RxData[1] << 4)) | (0x0F & (ui8RxData[0] >> 4 ));

	    		pTempData = 0x07FF & pTempData;



	return pTempData;

}
