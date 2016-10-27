#include <driverlib.h>
#include "rf430nfc.h"


unsigned char RxData[2] = {0,0};
unsigned char TxData[2] = {0,0};
unsigned char TxAddr[2] = {0,0};

//BUG FIXES: LOOPING, INIT AND NO WAIT TIME EXTENSION REQUEST
uint8_t errata_fixes[] =	 {  0xB2, 0xF0, 0xFF, 0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF, 0xFD, 0x00, 0x07, 0xA2, 0xC3, 0x00, 0x07,
								0x08, 0x3C, 0xB2, 0xF0, 0xFF, 0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF, 0xFD, 0x00, 0x07, 0xA2, 0xC3,
								0x00, 0x07, 0xB2, 0xB0, 0x04, 0x04, 0x00, 0x07, 0xF4, 0x23, 0x30, 0x41, 0x0F, 0x12, 0x0E, 0x12,
								0xB2, 0xB0, 0x00, 0x01, 0x00, 0x07, 0x08, 0x24, 0x3E, 0x40, 0x1D, 0x00, 0x1F, 0x42, 0xB2, 0x2A,
								0x6E, 0x9F, 0x02, 0x20, 0x92, 0x43, 0x2E, 0x2A, 0x3E, 0x41, 0x3F, 0x41, 0x30, 0x41};

unsigned int Read_Register(unsigned int reg_addr)
{
	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address

	RF430_I2C_Init();

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

}

void RF430_Init(void){
	unsigned char NDEF_Application_Data[] = RF430_DEFAULT_DATA;

	RF430_I2C_Init();
	//reset the rf430
	GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN4 );           // reset pin
	GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN4 );
	GPIO_setOutputLowOnPin(GPIO_PORT_P4,GPIO_PIN4);
	_delay_cycles(1000);
	GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN4 );


	P2SEL0 &= ~BIT2;	//set as gpio
	P2SEL1 &= ~BIT2;
	P2DIR &= ~BIT2;	//setting as input ///by default as gpio
	P2OUT |= BIT2;	//pull up is selected
	P2REN |= BIT2;	//enable pullup
//	P2IFG &= ~BIT2;	//clears the interrupt
//	P2IES |= BIT2;	// int at htol transition

	_delay_cycles(4000000); 	// Leave time for the RF430CL33H to get itself initialized;
	while(!(Read_Register(STATUS_REG) & READY));

		/****************************************************************************/
	    /* Errata Fixes: "Looping", unresponsive RF, and no SWXT                    */
	    /****************************************************************************/
	    {
			//Please implement this fix as given in this block.  It is important that
			//no line be removed or changed.

	    	//Enter test mode to set the maximum data rate that RF430 reports to mobile/reader
			//Currently sets to 106kbps with a 0 setting
	    	//Also loads some bug fixes.

			Write_Register(0xFFE0, 0x004E);
			Write_Register(0xFFFE, 0x0080);
			Write_Continuous (0x2AD0, (uint8_t *) &errata_fixes, sizeof(errata_fixes));		//write the patch to memory
			Write_Register(0x2A90, 0x2AFC); 	            // looping fix
			Write_Register(0x2AAE, 0x2AD0); 	            // wait time extension fix
			Write_Register(0x2A66, 0x0000); 		        // fix by init

			//RF Data rate settings
			// C4 = 847kbps , A2 = 424kbps, 91 = 212kbps, 00 = 106kbps (DEFAULT)
			//Write_Register(0x2ABA, 0x00C4);  				// C4 = 847kbps
			//Write_Register(0x2ABA, 0x00A2);  				// A2 = 424kbps
			//Write_Register(0x2ABA, 0x0091);  				// 91 = 212kbps
			Write_Register(0x2ABA, 0x0000);  				// 00 = 106kbps (DEFAULT SETTING) so this line is optional

			Write_Register(0x27B8, 0);
			Write_Register(0xFFE0, 0);
			//Upon exit of this block, the control register is set to 0x0
	    }

	Write_Continuous(0, NDEF_Application_Data, 48);

	//only interrupt for general type 4 request
	Write_Register(INT_ENABLE_REG, GT4_REQ_INT_ENABLE);

	//Configure INTO pin for active low and enable RF
	Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE);

//	P2IFG &= ~BIT2;	//clear the interrupt again
//	P2IE |= BIT2;	//enable the interrupt

}


void Write_Register(unsigned int reg_addr, unsigned int value)
{
	RF430_I2C_Init();

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

	RF430_I2C_Init();

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
