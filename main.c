#include "myclocks.h"
#include <driverlib.h>
#include "tmp112.h"
#include "rf430nfc.h"
#include "myTimers.h"



int main(void) {
//	unsigned int tempBin;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    initClocks();				// set aclk 32, smclk 1M, mclk 1M

    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6 );
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6 );

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6 );                             // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6 );

    P1SEL1 |= (1<<6);
	P1SEL1 |= (1<<7);

    PMM_unlockLPM5();

    initTimers();

    __bis_SR_register( GIE );

    TMP_I2C_Init();
    _delay_cycles(10000);
    TMP_Config_Init();
    _delay_cycles(10000);
   // RF430_I2C_Init();

    while(1){
//    	tempBin = getTemperature();
//    	temp = tempBin*0.0625;
//    	printf("%f",temp);
    }

}
