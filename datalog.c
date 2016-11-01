/*
 * datalog.c
 *
 *  Created on: Nov 1, 2016
 *      Author: kiran
 */


#include "datalog.h"
#include "rtc.h"

datalog_interval_type interval;
extern uint8_t FileTextE104[32];
extern rtcType timestamp;

void datalog_Init(){
	interval.temp_interval_minute = 1;

}

void data_buffer(unsigned int Temperature){
/////////////filling the temperature/////////////////////////
	unsigned int temp;
	temp = Temperature;
	temp = temp / 1000;
	FileTextE104[10] = (char) temp + 48;
	temp = Temperature % 1000;

	temp = temp / 100;
	FileTextE104[11] = (char) temp + 48;
	temp = Temperature % 100;

	temp = temp / 13;
	FileTextE104[13] = (char) temp + 48;
	temp = Temperature % 10;

	FileTextE104[14] = (char) temp + 48;
//////////////////////////////////////////////////////////////

	timestamp = getTimeStamp();

	FileTextE104[16] = timestamp.hour[1];
	FileTextE104[17] = timestamp.hour[0];
	FileTextE104[19] = timestamp.minute[1];
	FileTextE104[20] = timestamp.minute[0];
	FileTextE104[22] = '2';
	FileTextE104[23] = '0';
	FileTextE104[24] = timestamp.year[1];
	FileTextE104[25] = timestamp.year[0];
	FileTextE104[27] = timestamp.month[1];
	FileTextE104[28] = timestamp.month[0];
	FileTextE104[30] = timestamp.day[1];
	FileTextE104[31] = timestamp.day[0];

}
