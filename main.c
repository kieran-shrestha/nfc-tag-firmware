#include <driverlib.h>
#include <stdio.h>

#include "myclocks.h"
#include "tmp112.h"
#include "rf430nfc.h"
#include "myuart.h"
#include "myTimers.h"

//Temp_Modes_t g_ui8TemperatureModeFlag;
unsigned char ui8TemperatureNegFlag;

unsigned int g_TempDataFahr;
unsigned int g_TempDataCel;

char g_TempNegFlagCel = 0;
char g_TempNegFlagFahr = 0;

typedef enum {
	Fahrenheit = 0,
	Celcius
} Temp_Modes_t;

Temp_Modes_t g_ui8TemperatureModeFlag = Celcius;

unsigned char timerFired = 0;

//***** Prototypes ************************************************************
void initGPIO(void);

int main(void) {

	float Temperature;
	unsigned int flags = 0;
	char str[30];

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	initGPIO();

	initClocks();				// set aclk 10K, smclk 4M, mclk 4M

	myuart_init();

    initTimers();

	TMP_Config_Init();


	RF430_Init();


	__bis_SR_register( GIE );

	myuart_tx_string("Program started...\n");

    while(1){
    	//__bis_SR_register(LPM0_bits + GIE); //go to low power mode and enable interrupts. We are waiting for an NFC read or write of/to the RF430
    	//__no_operation();
    	if(!(P2IN & BIT2 ))
    	{
    		myuart_tx_byte('a');
    		flags = Read_Register(INT_FLAG_REG); //read the flag register to check if a read or write

    		Write_Register(INT_FLAG_REG, GT4_REQ_INT_ENABLE); //ACK the flags to clear
    		//setting to 1 clears the interrupt

    		Write_Register(INT_ENABLE_REG, 0);

    		GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6 );

    		myuart_tx_byte((char)flags);

    		if(flags & GT4_REQ_INT_ENABLE ){

    			GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN6 );
    			myuart_tx_string("Card was read or written");
    			GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);

    		}
    		flags = 0;
    		//renable the interrupt
    		Write_Register(INT_ENABLE_REG, GT4_REQ_INT_ENABLE);
    		//Configure INTO pin for active low and re-enable RF
    		Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE);

    	//	P2IFG &= ~BIT2;	//clear the interrupt again
    	//	P2IE |= BIT2;	//enable the interrupt



    	}

    	if(timerFired){
    		timerFired = 0;
    		GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN6 );

    		TMP_Get_Temp(&Temperature, &ui8TemperatureNegFlag, g_ui8TemperatureModeFlag);
    		if(ui8TemperatureNegFlag){
    		    Temperature = (-1.0)*Temperature;
    		}
    		sprintf(str, "Temperature: %.2f ", Temperature);
    		myuart_tx_string(str); myuart_tx_byte(0xB0); myuart_tx_byte('C'); myuart_tx_byte(0x0D);

    		GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6 );
    	}

    }

}



void initGPIO(void){
    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6 );			//for powering the IIC
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6 );

    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6 );         // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6 );

    P1SEL1 |= BIT6;		//setting p1.6 as sda
	P1SEL1 |= BIT7;		//setting p1.7 as scl

    PMM_unlockLPM5();

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

//*****************************************************************************
// Interrupt Service Routine
//*****************************************************************************
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer1_ISR (void)
{


    //**************************************************************************
    // 4. Timer ISR and vector
    //**************************************************************************
    switch( __even_in_range( TA1IV, TA1IV_TAIFG )) {
     case TA1IV_NONE: break;                 // (0x00) None
     case TA1IV_TACCR1:                      // (0x02) CCR1 IFG
          _no_operation();
           break;
     case TA1IV_TACCR2:                      // (0x04) CCR2 IFG
          _no_operation();
           break;
     case TA1IV_3: break;                    // (0x06) Reserved
     case TA1IV_4: break;                    // (0x08) Reserved
     case TA1IV_5: break;                    // (0x0A) Reserved
     case TA1IV_6: break;                    // (0x0C) Reserved
     case TA1IV_TAIFG:                       // (0x0E) TA1IFG - TAR overflow
          timerFired = 1;
        break;
     default: _never_executed();
    }
}
