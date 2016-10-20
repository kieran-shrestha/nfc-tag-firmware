#include <driverlib.h>
#include "tmp112.h"
#include "rf430nfc.h"

void TMP_I2C_Init(void){
		UCB0CTL1 |= UCSWRST;	            			//Software reset enabled
		UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR;	//I2C mode, Master mode, sync, transmitter
		UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 8MHz

		UCB0BRW = 20; 									// Baudrate = SMLK/40 = 200kHz

		UCB0I2CSA  = TMP112_I2C_ADDR;					// Set Slave Address
		UCB0IE = 0;
		UCB0CTL1 &= ~UCSWRST;
}

