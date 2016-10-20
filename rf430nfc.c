#include <driverlib.h>
#include "rf430nfc.h"

void RF430_I2C_Init(void){
	unsigned char ui8RxData[2];
	unsigned int ui16ReturnData;

	UCB0CTL1 |= UCSWRST;	            			// Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR;					// I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 4MHz
	UCB0BRW = 30; 									// Baudrate = SMLK/40 = 100kHz
	UCB0I2CSA  = RF430_I2C_ADDR;					// Set Slave Address
	UCB0CTL1 &= ~UCSWRST;
	UCB0IE = 0;	// disable interrupts



	UCB0CTL1  |= UCSWRST;	// following two lines can be only written when this bit is 1
	UCB0CTLW1 |= UCASTP_2;  // generate STOP condition.
	UCB0TBCNT = 0x0002;		// on every single byte
	UCB0CTL1  &= ~UCSWRST;


	UCB0CTLW0 |= UCTXSTT + UCTR;		// Start i2c write operation
	while(!(UCB0IFG & UCTXIFG0));	// checks for tx flag clears

	UCB0TXBUF = 0xFF & 0xFF;	// register to read from
	while(!(UCB0IFG & UCTXIFG0));	// if 1byte is transmitted

	UCB0TXBUF = 0xEE & 0xFF;	// register to read from
	while(!(UCB0IFG & UCBCNTIFG));	// if 1byte is transmitted

	UCB0CTL1  |= UCSWRST;
	UCB0CTL1 &= ~UCTR;				// in receiver mode
	UCB0CTLW1 |= UCASTP_2;  		// generate STOP condition.
	UCB0TBCNT = 0x0002;				// after two bytes
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT; 			// Repeated start is required
	while(!(UCB0IFG & UCRXIFG0));
	ui8RxData[1] = UCB0RXBUF;
	while(!(UCB0IFG & UCRXIFG0));
	ui8RxData[0] = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	ui16ReturnData = (0x0FF0 & (ui8RxData[1] << 4)) | (0x0F & (ui8RxData[0] >> 4 ));

}


