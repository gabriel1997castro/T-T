/*******************************************************************************
* \file serialmsg-arm7.c
* \brief  Implementa modulo basico de comunicação cliente-servidor usando porta serial.
// Observaçoes:
//    - Inspirado no projeto Carcarah
*******************************************************************************/

// Cabecalhos des biblioteca padrao C:
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cabecalhos especificos do modulo:
#include "AT91SAM7S64.h"
//#include "lib_AT91SAM7S64.h"

#include "definicoes.h"
#include "serialmsg-arm7.h"

// Definicoes internas:

// Prototipos internos:

// Variaveis do modulo:

/*****************************************************************************
******************************************************************************
** Funcoes de inicializacao e encerramento
******************************************************************************
*****************************************************************************/
int serialmsg_server_init(int comportnumber, unsigned long int comportBPS, int flagverbose, serialmsgcontrol_t *pserialmsgcontrol)
{
	int i;
	
	pserialmsgcontrol->mode = SERIALMSG_MODE_SERVER;
	pserialmsgcontrol->flagverbose = flagverbose;
	for(i=0;i<LPROCOTOL_MAX_FUNCTIONS;++i){
		pserialmsgcontrol->pmsgfunctionhandler[i] = NULL;
	}
	
	lprotocol_init();
	lprotocol_datagram_decoder_reset(&pserialmsgcontrol->protocoldatagram);

	
	return serial_init(comportnumber, comportBPS, MCK, &pserialmsgcontrol->serialportconfig);
}

int serialmsg_client_init(int comportnumber, unsigned long int comportBPS, int flagverbose, serialmsgcontrol_t *pserialmsgcontrol)
{
	pserialmsgcontrol->mode = SERIALMSG_MODE_CLIENT;
	pserialmsgcontrol->flagverbose = flagverbose;

	lprotocol_init();

	return serial_init(comportnumber, comportBPS, MCK, &pserialmsgcontrol->serialportconfig);
}

int serialmsg_close(serialmsgcontrol_t *pserialmsgcontrol)
{
//	serial_close(&pserialmsgcontrol->serialportconfig);

	lprotocol_close();

	return 0;
}

/*****************************************************************************
******************************************************************************
** Funcoes de interface
******************************************************************************
*****************************************************************************/
int serialmsg_server_update(float time_ms, serialmsgcontrol_t *pserialmsgcontrol)
{
	static int flagfirstcall = 1;
	static float time_ms_previous_byte = 0.0;
	static float time_ms_current_byte = 0.0;
	volatile int serialdata_in,i;
	int msghandlerstatus;
	
	if(!serial_kbhit(&pserialmsgcontrol->serialportconfig)){
		return 1;
	}

	time_ms_previous_byte = time_ms_current_byte;
	serialdata_in = serial_getc(&pserialmsgcontrol->serialportconfig);
	time_ms_current_byte = time_ms;

    // pequeno atraso antes de responder:
//    for(i=0;i<100;++i);

    // decomente a linha abaixo para operar em modo de eco:
//	serial_putc(serialdata_in, &pserialmsgcontrol->serialportconfig);	return(1);
	
	switch(lprotocol_datagram_decoder_process_received_byte(serialdata_in, &pserialmsgcontrol->protocoldatagram)){
	case LPROTOCOL_DATAGRAM_UNDER_DECODING:
//		serial_putc(pserialmsgcontrol->protocoldatagram.state, &pserialmsgcontrol->serialportconfig);
		if ((pserialmsgcontrol->protocoldatagram.state != LPROTOCOL_DATAGRAM_STATE_HEADER_RECEIVED) && (flagfirstcall==0) ){
			if ((time_ms_current_byte-time_ms_previous_byte) > 10.0){
				lprotocol_datagram_decoder_reset(&pserialmsgcontrol->protocoldatagram);
			}
		}
		break; 
	case LPROTOCOL_ERROR_RECEIVED_MORE_BYTES_THAN_EXPECTED:
		// tratar erro:
		// reiniciar decodificação:
		lprotocol_datagram_decoder_reset(&pserialmsgcontrol->protocoldatagram);
		break; 
	case LPROTOCOL_ERROR_INVALID_DATAGRAM_CHECKSUM:
		// tratar erro:
		// reiniciar decodificação:
		lprotocol_datagram_decoder_reset(&pserialmsgcontrol->protocoldatagram);
		break; 
	case LPROTOCOL_DATAGRAM_RECEIVED:
		// tratar mensagem do protocolo:
//			serial_putc(protocoldatagram.state, &uart0config);
//			serial_puts("\nmensagem recebida com sucesso: ", &uart0config);
		lprotocol_datagram_decoder_retrieve_data(&pserialmsgcontrol->protocoldata, &pserialmsgcontrol->protocoldatagram);
//		for(i=0;i<200;i+=2){ --i; }
		if(pserialmsgcontrol->pmsgfunctionhandler[pserialmsgcontrol->protocoldata.function_code] != NULL){
			msghandlerstatus = (*pserialmsgcontrol->pmsgfunctionhandler[pserialmsgcontrol->protocoldata.function_code])(&pserialmsgcontrol->protocoldata, &pserialmsgcontrol->protocoldatagram);
			switch(msghandlerstatus){
			case SERIALMSG_MSGHANDLERSTATUS_ACK: 
				// ok. se houver resposta, foi preenchida no gerenciador da mensagem.
				for(i=0;i<pserialmsgcontrol->protocoldatagram.datagram_size;++i){
					serial_putc(pserialmsgcontrol->protocoldatagram.datagram[i], &pserialmsgcontrol->serialportconfig);
				}
				break;
			case SERIALMSG_MSGHANDLERSTATUS_ERROR_WRONG_NUMBER_OF_ARGUMENTS: 
				// erro. responde a mensagem sinalizando.
				lprotocol_datagram_encoder_init(LPROCOTOL_FUNCTION_ERROR_WRONG_NUMBER_OF_ARGUMENTS, &pserialmsgcontrol->protocoldata, &pserialmsgcontrol->protocoldatagram);
				lprotocol_datagram_encoder_end(&pserialmsgcontrol->protocoldatagram);
				for(i=0;i<pserialmsgcontrol->protocoldatagram.datagram_size;++i){
					serial_putc(pserialmsgcontrol->protocoldatagram.datagram[i], &pserialmsgcontrol->serialportconfig);
				}
				break;
			}
		} else {
				// erro. responde a mensagem sinalizando.
				lprotocol_datagram_encoder_init(LPROCOTOL_FUNCTION_ERROR_WRONG_NUMBER_OF_ARGUMENTS, &pserialmsgcontrol->protocoldata, &pserialmsgcontrol->protocoldatagram);
				lprotocol_datagram_encoder_end(&pserialmsgcontrol->protocoldatagram);
				for(i=0;i<pserialmsgcontrol->protocoldatagram.datagram_size;++i){
					serial_putc(pserialmsgcontrol->protocoldatagram.datagram[i], &pserialmsgcontrol->serialportconfig);
				}
		}
		// reiniciar decodificação:
		lprotocol_datagram_decoder_reset(&pserialmsgcontrol->protocoldatagram);
		break; 
	}

	if(flagfirstcall){
		flagfirstcall = 0;
	}
	
	return 0;
}

int serialmsg_server_define_transaction_handler(unsigned char function_code, pserialmsgfunctionhandler_t pmsgfunctionhandler, serialmsgcontrol_t *pserialmsgcontrol)
{
	pserialmsgcontrol->pmsgfunctionhandler[function_code] = pmsgfunctionhandler;
	
	return 0;
}

int serialmsg_client_transaction_init(unsigned char function_code, serialmsgcontrol_t *pserialmsgcontrol)
{
	lprotocol_datagram_encoder_init(function_code, &pserialmsgcontrol->protocoldata, &pserialmsgcontrol->protocoldatagram);

	return 1; 
}

int serialmsg_client_transaction_insert_data(unsigned char *pdata, unsigned int data_size_in_bytes, serialmsgcontrol_t *pserialmsgcontrol)
{
	lprotocol_datagram_encoder_insert_data(pdata, data_size_in_bytes, &pserialmsgcontrol->protocoldatagram);
	
	return 1; 
}

int serialmsg_client_transaction_retrieve_data(int slot, unsigned char **ppdata, unsigned int *pdata_size_in_bytes, serialmsgcontrol_t *pserialmsgcontrol)
{
	if((slot>=0) && (slot<pserialmsgcontrol->protocoldata.number_of_data)){
		*ppdata = pserialmsgcontrol->protocoldata.pdata[slot];
		*pdata_size_in_bytes = pserialmsgcontrol->protocoldata.data_size[slot];
	
		return 1; 
	}
	
	return 0;
}

int serialmsg_client_transaction_execute(serialmsgcontrol_t *pserialmsgcontrol)
{
	int n;
	unsigned char serial_data;
	
//	if(pserialmsgcontrol->flagverbose) printf("\n serialmsg_client_transaction_execute: Mensagem enviada:  ");
	lprotocol_datagram_encoder_end(&pserialmsgcontrol->protocoldatagram);
	for(n=0;n<pserialmsgcontrol->protocoldatagram.datagram_size;++n){
		serial_data = pserialmsgcontrol->protocoldatagram.datagram[n];
		serial_putc(serial_data, &pserialmsgcontrol->serialportconfig);
//		if(pserialmsgcontrol->flagverbose) printf(" %2X",serial_data);
	}

//	if(pserialmsgcontrol->flagverbose) printf("\n serialmsg_client_transaction_execute: Mensagem recebida: ");
	lprotocol_datagram_decoder_reset(&pserialmsgcontrol->protocoldatagram);		
	do{
		serial_data = serial_getc(&pserialmsgcontrol->serialportconfig);
//		if(pserialmsgcontrol->flagverbose) printf(" %2X",serial_data);
		lprotocol_datagram_decoder_process_received_byte(serial_data, &pserialmsgcontrol->protocoldatagram);
	} while(lprotocol_datagram_decoder_process_received_byte(serial_data, &pserialmsgcontrol->protocoldatagram)==LPROTOCOL_DATAGRAM_UNDER_DECODING);

//		lprotocol_datagram_print(&protocoldatagram);
	lprotocol_datagram_decoder_retrieve_data(&pserialmsgcontrol->protocoldata, &pserialmsgcontrol->protocoldatagram);
	if(pserialmsgcontrol->protocoldata.function_code==LPROCOTOL_FUNCTION_ACK){
//		if(pserialmsgcontrol->flagverbose) printf("\n Transação reconhecida pelo servidor.");
	}
	else{
//		if(pserialmsgcontrol->flagverbose) printf("\n Transação não reconhecida pelo servidor.");
		return 0;
	}
	return 1;
}

/*****************************************************************************
******************************************************************************
** Funcoes internas
******************************************************************************
*****************************************************************************/
