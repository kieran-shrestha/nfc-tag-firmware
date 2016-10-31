#include <driverlib.h>
#include <stdio.h>

#include "myclocks.h"
#include "tmp112.h"
#include "rf430nfc.h"
#include "myuart.h"
#include "myTimers.h"

#define DEBUG 1

extern uint16_t SelectedFile;
extern uint8_t FileTextE104[30];

//Temp_Modes_t g_ui8TemperatureModeFlag;
unsigned char ui8TemperatureNegFlag;

unsigned int g_TempDataFahr;
unsigned int g_TempDataCel;

char g_TempNegFlagCel = 0;
char g_TempNegFlagFahr = 0;

typedef enum {
	Fahrenheit = 0, Celcius
} Temp_Modes_t;

Temp_Modes_t g_ui8TemperatureModeFlag = Celcius;

unsigned char timerFired = 0;

//***** Prototypes ************************************************************
void initGPIO(void);

int main(void) {

	unsigned int Temperature;		//to hold the temperature
	unsigned int flags = 0;	// to hold the interuupt flag of nfc ic
#ifdef DEBUG
	char str[30];			//for sprintf
#endif
	uint16_t interrupt_serviced = 0;	//which interrupt is just served of nfc ic

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	/*************************initialize the hardware**************************/
	initGPIO();
	initClocks();				// set aclk 10K, smclk 4M, mclk 4M
#ifdef DEBUG
	myuart_init();				// at 9600 baud
#endif
	initTimers();				// for 6.5sec temperature reads
	TMP_Config_Init();			// configure to be in shutdown one shot mode
	RF430_Init();				// resets the nfc ic

	__bis_SR_register( GIE);	//enable global interrupt
#ifdef DEBUG
	myuart_tx_string("Program started...\r\n");
#endif
	while (1) {
		//__bis_SR_register(LPM0_bits + GIE); //go to low power mode and enable interrupts. We are waiting for an NFC read or write of/to the RF430
		//__no_operation();
		if (!(P2IN & BIT2)) {	//if int from the nfc ic

			flags = Read_Register(INT_FLAG_REG); //read the flag register to check if a read or write
			GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN5);
			GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN5);

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (flags & FIELD_REMOVED_INT_ENABLE) {
#ifdef DEBUG
				myuart_tx_string("Removed from the RF field");
#endif
				interrupt_serviced |= FIELD_REMOVED_INT_FLAG;// clear this flag later
				Write_Register(INT_FLAG_REG, interrupt_serviced); //ACK the flags to clear
			}

			/////////////////////////////////////////////////////////////////////////////////////////////
			if (flags & DATA_TRANSACTION_INT_FLAG) {
				uint16_t status;
				uint16_t ret;
				status = Read_Register(STATUS_REG);	//read status register to determine the nature of interrupt

				switch (status & APP_STATUS_REGS) {

				// NDEF File Select Request is coming from the mobile/reader - response to the request is determined here
				// based on whether the file exists in our file database

				case FILE_SELECT_STATUS: {
					uint16_t file_id;
					file_id = Read_Register(NDEF_FILE_ID);	//determine the file like e103 or e104
					ret = SearchForFile((uint8_t *) &file_id);//check if the file exist
					interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;// clear this flag later

					if (ret == FileFound) {
						Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
						Write_Register(HOST_RESPONSE,
								INT_SERVICED_FIELD + FILE_EXISTS_FIELD); // indicate to the RF430 that the file exist

					} else {
						Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
						Write_Register(HOST_RESPONSE,
								INT_SERVICED_FIELD + FILE_DOES_NOT_EXIST_FIELD); // the file does not exist
					}

					break;
				}

				// NDEF ReadBinary request has been sent by the mobile / reader

				case FILE_REQUEST_STATUS: {
					uint16_t buffer_start;
					uint16_t file_offset;
					uint16_t file_length;
					buffer_start = Read_Register(NDEF_BUFFER_START);
					// where to start writing the file info in the RF430 buffer (0-2999)
					file_offset = Read_Register(NDEF_FILE_OFFSET);
					// what part of the file to start sending
					file_length = Read_Register(NDEF_FILE_LENGTH);
					// how much of the file starting at offset to send
					// we can send more than requested, called caching
					// as long as we write back into the length register how
					// much we sent it
					interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;// clear this flag later
					// can have bounds check for the requested length

					file_length = SendDataOnFile(SelectedFile, buffer_start,
							file_offset, file_length);

					Write_Register(NDEF_FILE_LENGTH, file_length); // how much was actually written
					Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
					Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD); // indicate that we have serviced the request

					break;
				}

				// NDEF UpdateBinary request
				case FILE_AVAILABLE_STATUS: {
					uint16_t buffer_start;
					uint16_t file_offset;
					uint16_t file_length;

					interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;// clear this flag later
					buffer_start = Read_Register(NDEF_BUFFER_START);// where to start in the RF430 buffer to read the file data (0-2999)
					file_offset = Read_Register(NDEF_FILE_OFFSET);// the file offset that the data begins at
					file_length = Read_Register(NDEF_FILE_LENGTH);// how much of the file is in the RF430 buffer

					//can have bounds check for the requested length
					ReadDataOnFile(SelectedFile, buffer_start, file_offset,
							file_length);
					Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
					Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD);// the interrup has been serviced

					break;
				}

				}//end of switch


			}

			GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);

			flags = 0;

			//renable the interrupt
			Write_Register(INT_ENABLE_REG, DATA_TRANSACTION_INT_ENABLE);

			//	P2IFG &= ~BIT2;	//clear the interrupt again
			//	P2IE |= BIT2;	//enable the interrupt

		}

		if (timerFired) {
			unsigned int temp;
			timerFired = 0;
			GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN6);

			TMP_Get_Temp(&Temperature, &ui8TemperatureNegFlag,
					g_ui8TemperatureModeFlag);
			if (ui8TemperatureNegFlag) {
				Temperature = (-1.0) * Temperature;
			}
#ifdef DEBUG
			sprintf(str, "Temperature: %d ", Temperature);
			myuart_tx_string(str);
			myuart_tx_byte(0xB0);
			myuart_tx_byte('C');
			myuart_tx_byte(0x0D);
#endif
			temp = Temperature;

			temp = temp/1000;
			FileTextE104[10] = (char)temp+48;
			temp = Temperature%1000;

			temp = temp/100;
			FileTextE104[11] = (char)temp+48;
			temp = Temperature%100;

			temp = temp/13;
			FileTextE104[13] = (char)temp+48;
			temp = Temperature%10;

			FileTextE104[14] = (char)temp+48;

			GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);
		}

	}

}

void initGPIO(void) {
	GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6);		//for powering the IIC
	GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6);

	GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN6);         // d1 led
	GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN6);

	GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN5);         // d2 led
	GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN5);

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
__interrupt void timer1_ISR(void) {

	//**************************************************************************
	// 4. Timer ISR and vector
	//**************************************************************************
	switch (__even_in_range(TA1IV, TA1IV_TAIFG)) {
	case TA1IV_NONE:
		break;                 // (0x00) None
	case TA1IV_TACCR1:                      // (0x02) CCR1 IFG
		_no_operation();
		break;
	case TA1IV_TACCR2:                      // (0x04) CCR2 IFG
		_no_operation();
		break;
	case TA1IV_3:
		break;                    // (0x06) Reserved
	case TA1IV_4:
		break;                    // (0x08) Reserved
	case TA1IV_5:
		break;                    // (0x0A) Reserved
	case TA1IV_6:
		break;                    // (0x0C) Reserved
	case TA1IV_TAIFG:                       // (0x0E) TA1IFG - TAR overflow
		timerFired = 1;
		break;
	default:
		_never_executed();
	}
}
