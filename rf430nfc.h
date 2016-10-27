#ifndef RF430NFC_H_
#define RF430NFC_H_

#include "msp430.h"

void RF430_I2C_Init(void);
void Write_Register(unsigned int reg_addr, unsigned int value);
unsigned int Read_Register(unsigned int reg_addr);
void Write_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length);
void RF430_Init(void);

#define RF430_I2C_ADDR 	0x0018

//define the values for Granite's registers we want to access
#define CONTROL_REG 		0xFFFE
#define STATUS_REG			0xFFFC
#define INT_ENABLE_REG		0xFFFA
#define INT_FLAG_REG		0xFFF8
#define CRC_RESULT_REG		0xFFF6
#define CRC_LENGTH_REG		0xFFF4
#define CRC_START_ADDR_REG	0xFFF2
#define COMM_WD_CTRL_REG	0xFFF0
#define VERSION_REG			0xFFEE //contains the software version of the ROM
#define TEST_FUNCTION_REG   0xFFE2
#define TEST_MODE_REG		0xFFE0

//define the different virtual register bits
//CONTROL_REG bits
#define SW_RESET		BIT0
#define RF_ENABLE		BIT1
#define INT_ENABLE		BIT2
#define INTO_HIGH		BIT3
#define INTO_DRIVE		BIT4
#define BIP8_ENABLE		BIT5
#define STANDBY_ENABLE	BIT6
#define TEST430_ENABLE	BIT7
//STATUS_REG bits
#define READY			BIT0
#define CRC_ACTIVE		BIT1
#define RF_BUSY			BIT2
//INT_ENABLE_REG bits
#define EOR_INT_ENABLE		BIT1
#define EOW_INT_ENABLE		BIT2
#define CRC_INT_ENABLE		BIT3
#define BIP8_ERROR_INT_ENABLE		BIT4
#define GT4_REQ_INT_ENABLE	BIT5
#define GENERIC_ERROR_INT_ENABLE	BIT7
//INT_FLAG_REG bits
#define EOR_INT_FLAG	BIT1
#define EOW_INT_FLAG	BIT2
#define CRC_INT_FLAG	BIT3
#define BIP8_ERROR_INT_FLAG	BIT4
#define NDEF_ERROR_INT_FLAG	BIT5
#define GENERIC_ERROR_INT_FLAG	BIT7
//COMM_WD_CTRL_REG bits
#define WD_ENABLE	BIT0
#define TIMEOUT_PERIOD_2_SEC	0
#define TIMEOUT_PERIOD_32_SEC	BIT1
#define TIMEOUT_PERIOD_8_5_MIN	BIT2
#define TIMEOUT_PERIOD_MASK		BIT1 + BIT2 + BIT3

#define TEST_MODE_KEY 0x004E

#define RF430_DEFAULT_DATA		{  														\
/*NDEF Tag Application Name*/ 															\
0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 												\
																						\
/*Capability Container ID*/ 															\
0xE1, 0x03, 																			\
0x00, 0x0F,	/* CCLEN */																	\
0x20,		/* Mapping version 2.0 */													\
0x00, 0xF9,	/* MLe (49 bytes); Maximum R-APDU data size */								\
0x00, 0xF6, /* MLc (52 bytes); Maximum C-APDU data size */								\
0x04, 		/* Tag, File Control TLV (4 = NDEF file) */									\
0x06, 		/* Length, File Control TLV (6 = 6 bytes of data for this tag) */			\
0xE1, 0x04,	/* File Identifier */														\
0x0B, 0xDF, /* Max NDEF size (3037 bytes of useable memory) */							\
0x00, 		/* NDEF file read access condition, read access without any security */		\
0x00, 		/* NDEF file write access condition; write access without any security */	\
																						\
/* NDEF File ID */ 																		\
0xE1, 0x04, 																			\
																						\
/* NDEF File for Hello World */                             				            \
0x00, 0x14, /* NLEN: NDEF length (20 byte long message, max. length for RF430CL) */     \
                                                                                        \
/* NDEF Record (refer to NFC Data Exchange Format specifications)*/                     \
0xD1,       /*MB(Message Begin), SR(Short Record) flags set, ME(Message End), IL(ID length field present) flags cleared; TNF(3bits) = 1; */ \
0x01, 0x10, /*Type Length = 0x01; Payload Length = 0x10 */                              \
0x54,       /* Type = T (text) */                                                       \
0x02,       /* 1st payload byte: "Start of Text", as specified in ASCII Tables */       \
0x65, 0x6E, /* 'e', 'n', (2nd, 3rd payload bytes*/                                      \
                                                                                        \
/* 'Hello, world!' NDEF data*/                                                          \
0x48, 0x65, 0x6C, 0x6C, 0x6f, 0x2c, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21            \
	} /* End of data */


#endif
