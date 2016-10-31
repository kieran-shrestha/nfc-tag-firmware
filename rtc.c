/*
 * rtc.c
 *
 *  Created on: Oct 31, 2016
 *      Author: kiran
 */

#include "rtc.h"
#include "msp430.h"
#include "myuart.h"
#include "stdint.h"

void RTC_init(){

	RTCCTL01 |= RTCHOLD | RTCTEV_0 | RTCBCD | RTCCTL0 | RTCTEVIE;//| RTCAIE;

	RTCHOUR = 0x20;
	RTCMIN = 0x50;
	RTCSEC = 0x00;
	RTCYEAR = 0x2016;
	RTCMON = 0x10;
	RTCDAY = 0x31;

	RTCAMIN = 0x00;
	RTCAHOUR = 0x00;
	RTCADAY = 0x00;
	RTCADOW = 0x00;

	RTCCTL01 &= ~RTCHOLD;

}

void getTimeStamp(){
	rtc_Type rtcData;
	uint8_t x;

	x = RTCHOUR;
	//myuart_tx_byte(x);
	rtcData.hour[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.hour[1] = (x & 0x0F ) + 48;

	x = RTCMIN;
	rtcData.minute[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.minute[1] = (x & 0x0F ) + 48;

	x = (char)RTCYEAR;
	rtcData.year[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.year[1] = (x & 0x0F ) + 48;

	x = RTCMON;
	rtcData.month[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.month[1] = (x & 0x0F ) + 48;

	x = RTCDAY;
	rtcData.day[0] = (x & 0x0F ) + 48;
	x >>= 4;
	rtcData.day[1] = (x & 0x0F ) + 48;

	myuart_tx_byte(rtcData.hour[1]);
	myuart_tx_byte(rtcData.hour[0]);

	myuart_tx_byte(':');

	myuart_tx_byte(rtcData.minute[1]);
	myuart_tx_byte(rtcData.minute[0]);

	myuart_tx_byte(0x20);

	myuart_tx_byte(rtcData.year[1]);
	myuart_tx_byte(rtcData.year[0]);
	myuart_tx_byte('/');
	myuart_tx_byte(rtcData.month[1]);
	myuart_tx_byte(rtcData.month[0]);
	myuart_tx_byte('/');
	myuart_tx_byte(rtcData.day[1]);
	myuart_tx_byte(rtcData.day[0]);

}


#pragma vector=RTC_VECTOR
__interrupt void RTCISR(void)
{
  switch (__even_in_range(RTCIV, RTCIV_RTCOFIFG)){
    case RTCIV_NONE: break;
    case RTCIV_RTCRDYIFG: break;
    case RTCIV_RTCTEVIFG:		// Should fire and be here once ever minute
    	__no_operation();
    	LPM4_EXIT;

      break;
    case RTCIV_RTCAIFG:		// Alarm Flag
    	break;
    case RTCIV_RT0PSIFG: break;
    case RTCIV_RT1PSIFG: break;
    case RTCIV_RTCOFIFG: break;
  }
}

