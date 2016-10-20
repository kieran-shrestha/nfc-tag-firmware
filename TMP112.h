#ifndef TMP103_H_
#define TMP103_H_

#include "msp430.h"

void TMP_I2C_Init(void);
unsigned int TMP_I2C_Read_Register(unsigned char reg_addr);
void TMP_Get_Temp(unsigned int* ui16TempReturn, unsigned char* uc8NegFlagReturn );


#define TMP112_I2C_ADDR 	0x0048

//TMP112 Register Addresses
#define TMP112_TEMP_REG		0x00
#define TMP112_CONF_REG 	0x01
#define TMP112_TLOW_REG		0x02
#define TMP112_THIGH_REG 	0x03


#endif /* TMP103_H_ */

