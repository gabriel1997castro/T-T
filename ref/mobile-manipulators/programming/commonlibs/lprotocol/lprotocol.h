/*******************************************************************************
* \file lprotocol.h
* \brief  Implementa modulo basico do protocolo de comunicação de dados.
// Observaçoes:
*******************************************************************************/

#ifndef LPROTOCOL_H
#define LPROTOCOL_H

#include "lprotocol-user-defines.h"

#ifdef __cplusplus
 extern "C" {
#endif 

// Definicoes de uso externo quer dependem da aplicação do usuario:
#define LPROTOCOL_MAXDATAGRAMSIZE	500
#define LPROTOCOL_MAXDATAFIELDS		10

// Definicoes de uso externo:
#define LPROTOCOL_DATAGRAM_HEADER								'#'
#define LPROTOCOL_DATAGRAM_STATE_EMPTY							0
#define LPROTOCOL_DATAGRAM_STATE_HEADER_RECEIVED				1
#define LPROTOCOL_DATAGRAM_STATE_FUNCTION_RECEIVED				2
#define LPROTOCOL_DATAGRAM_STATE_DATAGRAMSIZE_RECEIVED			3
#define LPROTOCOL_DATAGRAM_STATE_DATAFIELD_RECEIVED				4
#define LPROTOCOL_DATAGRAM_STATE_READY							5
#define LPROTOCOL_DATAGRAM_STATE_CHECKSUMERROR					6

// Datagram encoding
#define LPROTOCOL_DATAGRAM_ENCODING_SHORT_EQUALDATAFIELDSSIZE			2
// Designed to encode sequences of the same data type, which can be char, int, float as well as structs.
// Format: HEADER + FUNCTION + DATAGRAMSIZE + DATASIZE + DATA_1 + ... + DATA_N + CHECKSUM
// HEADER: unsigned char (byte), given by LPROTOCOL_DATAGRAM_HEADER
// FUNCTION: unsigned char (byte), representing the procedure to be applied to data
// DATAGRAMSIZE: unsigned char (byte), representing the total datagram size in bytes. If DATAGRAMSIZE == 4, means no data.
// DATASIZE: unsigned char (byte), representing the number of bytes of each data. 
// DATA_1: sequence of unsigned char (1 byte), representing the data indexed 1
// DATA_N: sequence of unsigned char (1 byte), representing the data indexed N
// CHECKSUM: unsigned char (byte), given by the sum of all bytes of the datagram
// N is obtained from N = (DATAGRAMSIZE - 4)/DATASIZE
// If each data has always the same number of bytes given by EACHDATASIZE, to be known at sender as well as at the receiver, 
// Designed to encode sequences of the same data type, which can be char, int, float as well as structs.

#define LPROTOCOL_DATAGRAM_ENCODING_SHORT_DIFFERENTDATAFIELDSSIZE		3
// Designed to encode sequences of different data types, which can be char, int, float as well as structs.
// Format: HEADER + FUNCTION + DATAGRAMSIZE + DATASIZE_1 + DATA_1 + ... + DATASIZE_N + DATA_N + CHECKSUM
// HEADER: unsigned char (byte), given by LPROTOCOL_DATAGRAM_HEADER
// FUNCTION: unsigned char (byte), representing the procedure to be applied to data
// DATAGRAMSIZE: unsigned char (byte), representing the total datagram size in bytes. If DATAGRAMSIZE == 4, means no data.
// DATASIZE_1: unsigned char (byte), representing the number of bytes of DATA_1. 
// DATA_1: sequence of unsigned char (1 byte), representing the data indexed 1
// DATASIZE_N: unsigned char (byte), representing the number of bytes of DATA_N. 
// DATA_N: sequence of unsigned char (1 byte), representing the data indexed N
// CHECKSUM: unsigned char (byte), given by the sum of all bytes of the datagram
// N is obtained processing the message sequentialy
// If each data has always the same number of bytes given by EACHDATASIZE, to be known at sender as well as at the receiver, 
// Designed to encode sequences of the same data type, which can be char, int, float as well as structs.

#define LPROTOCOL_DATAGRAM_ENCODING_LONG								4
// Designed to encode very long sequences of one data, usualy a struct.
// Format: HEADER + FUNCTION + DATAGRAMSIZE_BYTE_4 + DATAGRAMSIZE_BYTE_3 + DATAGRAMSIZE_BYTE_2 + DATAGRAMSIZE_BYTE_1  + DATA + CHECKSUM
// HEADER: unsigned char (byte), given by LPROTOCOL_DATAGRAM_HEADER
// FUNCTION: unsigned char (byte), representing the procedure to be applied to data
// DATAGRAMSIZE_BYTE_4: unsigned char (byte), to obtain the datagram size.
// DATAGRAMSIZE_BYTE_3: unsigned char (byte), to obtain the datagram size.
// DATAGRAMSIZE_BYTE_2: unsigned char (byte), to obtain the datagram size.
// DATAGRAMSIZE_BYTE_1: unsigned char (byte), to obtain the datagram size. 
//    DATAGRAMSIZE is given by DATAGRAMSIZE_BYTE_3*(1 << 24) + DATAGRAMSIZE_BYTE_3*(1 << 16) + DATAGRAMSIZE_BYTE_2*(1 << 8) + DATAGRAMSIZE_BYTE_1
//    DATAGRAMSIZE == 7, means no data.
// DATASIZE_1: unsigned char (byte), representing the number of bytes of DATA_1. 
// DATA_1: sequence of unsigned char (1 byte), representing the data indexed 1
// DATASIZE_N: unsigned char (byte), representing the number of bytes of DATA_N. 
// DATA_N: sequence of unsigned char (1 byte), representing the data indexed N
// CHECKSUM: unsigned char (byte), given by the sum of all bytes of the datagram

// Choose one encoding 
#define LPROTOCOL_DATAGRAM_ENCODING		LPROTOCOL_DATAGRAM_ENCODING_SHORT_EQUALDATAFIELDSSIZE
//#define LPROTOCOL_DATAGRAM_ENCODING		LPROTOCOL_DATAGRAM_ENCODING_SHORT_DIFFERENTDATAFIELDSSIZE	
//#define LPROTOCOL_DATAGRAM_ENCODING		LPROTOCOL_DATAGRAM_ENCODING_LONG	

/**********************************************************************************************
***** Defines will be used in the project. All of them can be defined at compilation 
***** time according to compiler options.
**********************************************************************************************/
#if !defined(LPROTOCOL_DEFINE_ALL)
	#define LPROTOCOL_DEFINE_ALL 								(0) 
#endif
#if !defined(LPROTOCOL_DEFINE_DATAGRAM_PRINT)
	#define LPROTOCOL_DEFINE_DATAGRAM_PRINT						(0 + LPROTOCOL_DEFINE_ALL) 
#endif

typedef int lprotocolreturnstatus_t;

#define LPROTOCOL_DATAGRAM_RECEIVED							 2
#define LPROTOCOL_DATAGRAM_UNDER_DECODING					 1
#define LPROTOCOL_SUCCESS						 			 0
#define LPROTOCOL_ERROR_INVALID_DATA_ENCODING				-1
#define LPROTOCOL_ERROR_INVALID_DATAGRAM_CHECKSUM			-2
#define LPROTOCOL_ERROR_RECEIVED_MORE_BYTES_THAN_EXPECTED	-3

typedef struct{
	unsigned char   *pdata[LPROTOCOL_MAXDATAFIELDS];
	unsigned int 	data_size[LPROTOCOL_MAXDATAFIELDS]; // number of bytes of each data
	unsigned int 	number_of_data;
	unsigned char	function_code;
} lprotocoldata_t;

typedef struct{
  // data
  unsigned char datagram[LPROTOCOL_MAXDATAGRAMSIZE];
  unsigned int  datagram_size;
  // for internal control
  unsigned char state;
  unsigned char checksum;
} lprotocoldatagram_t;

// Prototipos de uso externo:
lprotocolreturnstatus_t lprotocol_init(void);
void lprotocol_close(void);

void lprotocol_datagram_print(lprotocoldatagram_t * pdatagram);
void lprotocol_datagram_encoder_init(char function_code, lprotocoldata_t *pprotocoldata, lprotocoldatagram_t * pdatagram);
void lprotocol_datagram_encoder_end(lprotocoldatagram_t * pdatagram);
void lprotocol_datagram_encoder_insert_data(unsigned char *pdata, unsigned int data_size_in_bytes, lprotocoldatagram_t *pdatagram);

void lprotocol_datagram_decoder_reset(lprotocoldatagram_t *pdatagram);
lprotocolreturnstatus_t lprotocol_datagram_decoder_process_received_byte(unsigned char received_byte, lprotocoldatagram_t *pdatagram);
void lprotocol_datagram_decoder_retrieve_data(lprotocoldata_t *pprotocoldata, lprotocoldatagram_t *pdatagram);

#ifdef __cplusplus
}
#endif 

#endif // LPROTOCOL_H


