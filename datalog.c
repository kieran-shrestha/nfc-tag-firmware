/*
 * datalog.c
 *
 *  Created on: Nov 1, 2016
 *      Author: kiran
 */


#include "datalog.h"
#include "rtc.h"
#include "myuart.h"

#define DATA_WIDTH 23
#define SENDTFRAMTHRSHLD 2


#include <stdio.h>

#define DEBUG 1


//unsigned char* gp_EndofLogMem =  0xFF79;	//ONE LESS THAN LAST MEMORY
//unsigned char* gp_StartofLogMem = (void *) 0xD869;	//10k records

unsigned char log_StartOfData[DATA_WIDTH][SENDTFRAMTHRSHLD];

#pragma PERSISTENT (FRAM_LOG)
unsigned char FRAM_LOG[DATA_WIDTH][150]={0};

#pragma PERSISTENT (numOfLogsInFram)
unsigned int numOfLogsInFram = 0;

datalog_interval_type interval;
int counter;
extern uint8_t FileTextE104[];
extern rtcType timestamp;

void datalog_Init(){
	interval.temp_interval_minute = 1;
	counter = 0;
}


void data_buffer(unsigned int Temperature){
/////////////filling the temperature/////////////////////////
	unsigned int temp;
	temp = Temperature;
	unsigned char bufferHold[DATA_WIDTH];
	unsigned char x,y;

#ifdef DEBUG
			char str[30];
			sprintf(str, "\n\rTemperature: %d ", Temperature);
			myuart_tx_string(str);
			myuart_tx_byte(0xB0);
			myuart_tx_byte('C');
			myuart_tx_byte(0x0D);
#endif

	temp = temp / 1000;
	FileTextE104[10] = (char) temp + 48;
	bufferHold[1] = FileTextE104[10];
	temp = Temperature % 1000;

	temp = temp / 100;
	FileTextE104[11] = (char) temp + 48;
	bufferHold[2] = FileTextE104[11];
	temp = Temperature % 100;

	temp = temp / 13;
	FileTextE104[13] = (char) temp + 48;
	bufferHold[4] = FileTextE104[13];
	temp = Temperature % 10;

	FileTextE104[14] = (char) temp + 48;
	bufferHold[5] = FileTextE104[14];
//////////////////////////////////////////////////////////////

	timestamp = getTimeStamp();

	FileTextE104[16] = timestamp.hour[1];
	bufferHold[7] = FileTextE104[16];
	FileTextE104[17] = timestamp.hour[0];
	bufferHold[8] = FileTextE104[17];

	FileTextE104[19] = timestamp.minute[1];
	bufferHold[10] = FileTextE104[19];
	FileTextE104[20] = timestamp.minute[0];
	bufferHold[11] = FileTextE104[20];

	FileTextE104[22] = '2';
	bufferHold[13] = FileTextE104[22];
	FileTextE104[23] = '0';
	bufferHold[14] = FileTextE104[23];
	FileTextE104[24] = timestamp.year[1];
	bufferHold[15] = FileTextE104[24];
	FileTextE104[25] = timestamp.year[0];
	bufferHold[16] = FileTextE104[25];

	FileTextE104[27] = timestamp.month[1];
	bufferHold[18] = FileTextE104[27];
	FileTextE104[28] = timestamp.month[0];
	bufferHold[19] = FileTextE104[28];

	FileTextE104[30] = timestamp.day[1];
	bufferHold[21] = FileTextE104[30];
	FileTextE104[31] = timestamp.day[0];
	bufferHold[22] = FileTextE104[31];


#ifdef DEBUG
	myuart_tx_byte(timestamp.hour[1]);
	myuart_tx_byte(timestamp.hour[0]);

	myuart_tx_byte(':');

	myuart_tx_byte(timestamp.minute[1]);
	myuart_tx_byte(timestamp.minute[0]);

	myuart_tx_byte(0x20);

	myuart_tx_byte(timestamp.year[1]);
	myuart_tx_byte(timestamp.year[0]);
	myuart_tx_byte('/');
	myuart_tx_byte(timestamp.month[1]);
	myuart_tx_byte(timestamp.month[0]);
	myuart_tx_byte('/');
	myuart_tx_byte(timestamp.day[1]);
	myuart_tx_byte(timestamp.day[0]);
	myuart_tx_byte(0x0D);
#endif

	for(x = 0 ; x < DATA_WIDTH ; x++){
		if(x == 0 || x == 3 || x == 6 || x == 9 || x == 12 || x == 17 || x == 20){
			if(x == 0)
				log_StartOfData[x][counter] = 'T';
			if(x == 3)
				log_StartOfData[x][counter] = '.';
			if(x == 6)
				log_StartOfData[x][counter] = 'C';
			if(x == 9)
				log_StartOfData[x][counter] = ':';
			if(x == 12)
				log_StartOfData[x][counter] = 'D';
			if(x == 17)
				log_StartOfData[x][counter] = '/';
			if(x == 20)
				log_StartOfData[x][counter] = '/';
			continue;
		}
		log_StartOfData[x][counter] = bufferHold[x];

	}

	counter++;
//////////send the the data logs to the fram when threshold is crossed//////////////////
	if(counter > SENDTFRAMTHRSHLD-1){
		temp = numOfLogsInFram;
		for(y = 0; y < SENDTFRAMTHRSHLD; y++)
			for (x = 0; x <DATA_WIDTH ;x++)
				FRAM_LOG[x][y + temp] = log_StartOfData[x][y];

		numOfLogsInFram+=counter;
		counter = 0;

#ifdef DEBUG
		sprintf(str,"\n\rTL=%d dumping all\n\r",numOfLogsInFram);
		myuart_tx_string(str);
		for(temp = 0 ;temp < numOfLogsInFram;temp++){
			for(y = 0 ; y< DATA_WIDTH; y++){
				myuart_tx_byte(FRAM_LOG[y][temp]);
			}
			myuart_tx_byte(0x0D);
		}
#endif

	}


}

