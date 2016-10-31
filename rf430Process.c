#include "myuart.h"
#include "rf430nfc.h"
#include "rf430Process.h"
#include "stdint.h"

#define DEBUG

extern uint16_t SelectedFile;

void rf430Interrupt(uint16_t flags) {

	uint16_t interrupt_serviced = 0;//which interrupt is just served of nfc ic

	if (flags & FIELD_REMOVED_INT_ENABLE) {
#ifdef DEBUG
		myuart_tx_string("Removed from the RF field");
#endif
		interrupt_serviced |= FIELD_REMOVED_INT_FLAG; // clear this flag later
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
			file_id = Read_Register(NDEF_FILE_ID);//determine the file like e103 or e104
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

			interrupt_serviced |= DATA_TRANSACTION_INT_FLAG; // clear this flag later
			buffer_start = Read_Register(NDEF_BUFFER_START); // where to start in the RF430 buffer to read the file data (0-2999)
			file_offset = Read_Register(NDEF_FILE_OFFSET); // the file offset that the data begins at
			file_length = Read_Register(NDEF_FILE_LENGTH); // how much of the file is in the RF430 buffer

			//can have bounds check for the requested length
			ReadDataOnFile(SelectedFile, buffer_start, file_offset,
					file_length);
			Write_Register(INT_FLAG_REG, interrupt_serviced); // ACK the flags to clear
			Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD); // the interrupt has been serviced

			break;
		}

		} //end of switch

	}

}

