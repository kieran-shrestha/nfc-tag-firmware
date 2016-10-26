#include <driverlib.h>
#include "rf430nfc.h"


unsigned char RxData[2] = {0,0};
unsigned char TxData[2] = {0,0};
unsigned char TxAddr[2] = {0,0};

unsigned int Read_Register(unsigned int reg_addr)
{
	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address


		UCB0CTL1  |= UCSWRST;
		UCB0CTLW1 = UCASTP_0;  // generate STOP condition.
		UCB0CTL1  &= ~UCSWRST;

		UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
		while(!(UCB0IFG & UCTXIFG0));

		UCB0TXBUF = TxAddr[0] & 0xFF;
		while(!(UCB0IFG & UCTXIFG0));

		UCB0TXBUF = TxAddr[1] & 0xFF;
		while(!(UCB0IFG & UCTXIFG0));

	UCB0CTL1 &= ~UCTR; 				//i2c read operation
	UCB0CTL1 |= UCTXSTT; 			//repeated start

	while(!(UCB0IFG & UCRXIFG0));
	RxData[0] = UCB0RXBUF;
	UCB0CTL1 |= UCTXSTP; 			//send stop after next RX

	while(!(UCB0IFG & UCRXIFG0));
	RxData[1] = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG));    // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	return RxData[1] << 8 | RxData[0];
}

void RF430_I2C_Init(void){

	UCB0CTL1 |= UCSWRST;	            			//Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR;	//I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 4MHz

	UCB0BRW = 10; 									// Baudrate = SMLK/40 = 400kHz

	UCB0I2CSA  = RF430_I2C_ADDR;					// Set Slave Address
	UCB0IE = 0;
	UCB0CTL1 &= ~UCSWRST;
	// Software reset released

	//reset the rf430
	GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN4 );           // reset pin
	GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN4 );
	GPIO_setOutputLowOnPin(GPIO_PORT_P4,GPIO_PIN4);
	_delay_cycles(1000);
	GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN4 );

	P2DIR |= 0x04;	//setting as input ///by default as gpio
	P2OUT |= 0x04;	//pull up is selected
	P2REN |= 0x04;	//enable pullup
	P2IFG |= 0x04;	//clears the interrupt
	P2IES |= 0x04;	// int at htol transition

	_delay_cycles(4000000); 	// Leave time for the RF430CL33H to get itself initialized;

	while(!(Read_Register(STATUS_REG) & READY));



}


void Write_Register(unsigned int reg_addr, unsigned int value)
{
	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address
	TxData[0] = value >> 8;
	TxData[1] = value & 0xFF;

	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		//start i2c write operation
	//write the address
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	//write the data
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxData[1];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxData[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0CTL1 |= UCTXSTP;
	while((UCB0STAT & UCBBUSY));     // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

}

void Write_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length)
{
	unsigned int i;

	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address

	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		//start i2c write operation
	//write the address
	while(!(UCB0IFG & UCTXIFG0));

	UCB0TXBUF = TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];

	for(i = 0; i < data_length; i++)
	{
		while(!(UCB0IFG & UCTXIFG0));
		UCB0TXBUF = write_data[i];
	}

	while(!(UCB0IFG & UCTXIFG0));
	UCB0CTL1 |= UCTXSTP;
	while((UCB0STAT & UCBBUSY));    // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

}
