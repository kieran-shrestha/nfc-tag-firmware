#include "myclocks.h"
#include <driverlib.h>
#include "tmp112.h"
#include "rf430nfc.h"
#include "myuart.h"
#include "myTimers.h"



int main(void) {
//	unsigned int tempBin;
	unsigned char NDEF_Application_Data[] = RF430_DEFAULT_DATA;
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    initClocks();				// set aclk 32, smclk 1M, mclk 1M

    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6 );
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6 );

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6 );           // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6 );

    P1SEL1 |= (1<<6);
	P1SEL1 |= (1<<7);

    PMM_unlockLPM5();

  // initTimers();
  //  myuart_init();


    TMP_I2C_Init();
    _delay_cycles(10000);
    //TMP_Config_Init();
    _delay_cycles(10000);
    RF430_I2C_Init();

    Write_Continuous(0, NDEF_Application_Data, 48);
    //Configure INTO pin for active low and enable RF

    Write_Register(INT_ENABLE_REG, GT4_REQ_INT_ENABLE);

    Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE + INTO_HIGH);

    P2DIR &= ~(1<<2);	//setting as input

   // P2OUT |= (1<<2);
  //  P2REN |= (1<<2);
   // P2IFG &= ~(1<<2);
 //   P2IE |= (1<<2);
 //   __delay_cycles(4000000);
 //   __bis_SR_register( GIE );
 //   myuart_tx_byte('s');
  //  P2IFG &= ~(1<<2);
  //      P2IE |= (1<<2);
    while(1){
    	//__bis_SR_register(LPM3_bits + GIE); //go to low power mode and enable interrupts. We are waiting for an NFC read or write of/to the RF430
    //	__no_operation();
    	if(P2IN & BIT2 )
    		{
    		//	Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE);
    		//	__delay_cycles(750000);
    		//	Write_Register(INT_ENABLE_REG, 0);

    		//    Write_Register(INT_ENABLE_REG, GT4_REQ_INT_ENABLE);
    		//    Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE + INTO_HIGH);


    		}
    }

}


//#pragma vector=PORT2_VECTOR
//__interrupt void PORT2_ISR(void)
//{
//	//INTO interrupt fired
//	if(P2IFG & BIT2 )
//	{
//	    P2IE &= ~(1<<2); //disable INTO
//	    P2IFG &= ~(1<<2); //clear interrupt flag
//	    GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN6 );
//	    //Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE); //clear control reg to disable RF
//	   // __delay_cycles(750000);
//	 //   GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6 );
//	    P2IE |= (1<<2);
//	}
//}
