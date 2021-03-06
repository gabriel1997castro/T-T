/*****************************************************************************
*** Projeto CARCARAH (UnB-Expansion)
*** Conteudo: modulo socketmsg.
*** Autor: G. A. Borges.
*** Atualizacoes: 
	- 01/05/2009: criacao
*****************************************************************************/
/*! \file socketmsg.cpp
* \brief Arquivo exemplo de modulo. */

// Cabecalhos des biblioteca padrao C:
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for libc5 */
#include <sys/io.h> /* for glibc */
#include <pthread.h>
#include <errno.h>

// Cabecalhos especificos do modulo:
#include "socketmsg-x86.h"

// Definicoes internas:
inline void socketmsg_mutexinit(socketmsg_t *pSocketMsgStruct);

// Prototipos internos:

// Variaveis do modulo:

/*****************************************************************************
******************************************************************************
** Funes de inicializacao e encerramento
******************************************************************************
*****************************************************************************/

/*! \fn int socketmsg_server_init(int PortNumber, socketmsg_t *pSocketMsgStruct, int BufferLength, int flagverbose)
* Funcao de inicializacao.
* \param 
* \return
*/
int socketmsg_server_init(int PortNumber, socketmsg_t *pSocketMsgStruct, int BufferLength, int flagverbose)
{
	printf("\n\n*** Inicializacao do modulo socketmsg em modo servidor");

	// Inicializa variaveis globais, aloca memoria, etc.
	socketmsg_mutexinit(pSocketMsgStruct);
	pSocketMsgStruct->mode = SOCKETMSG_MODE_SERVER;
	pSocketMsgStruct->buffer = (unsigned char*) malloc(BufferLength);
	if (pSocketMsgStruct->buffer==NULL){
		printf("\n*** Erro: falha na alocao do buffer.");
		pSocketMsgStruct->bufferlen = 0;
		return 0;
	}
	pSocketMsgStruct->bufferlen = BufferLength;
	pSocketMsgStruct->flagverbose = flagverbose;

	// Inicializa socket.
	pSocketMsgStruct->serversocketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pSocketMsgStruct->serversocketfd == -1) {
		printf("\n*** Erro: falha na chamada de socket(): %s", strerror(errno));
		return 0;
	}

	// Modo servidor:
	// Recover sever structure:
	memset((struct cockaddr_in *)&(pSocketMsgStruct->serveraddress),0,sizeof(pSocketMsgStruct->serveraddress));
	pSocketMsgStruct->serveraddress.sin_family = AF_INET;
	pSocketMsgStruct->serveraddress.sin_port   = htons(PortNumber);
	pSocketMsgStruct->serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(pSocketMsgStruct->serveraddress.sin_zero), '\0', 8); // zero the rest of the struct 

	// Associar o socket ao endereo
	if (bind(pSocketMsgStruct->serversocketfd, (struct sockaddr *)(&pSocketMsgStruct->serveraddress),sizeof(pSocketMsgStruct->serveraddress))!=0) {
		printf("\n*** Erro: erro na chamada a bind(): %s", strerror(errno));
		close(pSocketMsgStruct->serversocketfd);
		return 0;
	}
	printf("\n*** Servidor associado ao IP %s - porta %i.",
		inet_ntoa(pSocketMsgStruct->serveraddress.sin_addr), 
		ntohs(pSocketMsgStruct->serveraddress.sin_port));

	// Entrar em modo de escuta
	if (listen(pSocketMsgStruct->serversocketfd, SOMAXCONN)!=0) {
		printf("\n*** Erro: erro na chamada a listen(): %s",strerror(errno));
		close(pSocketMsgStruct->serversocketfd);
		return 0;
	}
	printf("\n*** Servidor escutando no IP %s - porta %i.",
		inet_ntoa(pSocketMsgStruct->serveraddress.sin_addr), 
		ntohs(pSocketMsgStruct->serveraddress.sin_port));

	// Inicializa threads.

	// Retorna 

    return 1; 
}                       

/*! \fn int socketmsg_client_init(char *pServer, int ServerAddressMode, int PortNumber, socketmsg_t *pSocketMsgStruct, int BufferLength, int flagverbose)
* Funcao de inicializacao.
* \param 
* \return
*/
int socketmsg_client_init(char *pServer, int ServerAddressMode, int PortNumber, socketmsg_t *pSocketMsgStruct, int BufferLength, int flagverbose)
{
	struct hostent *host;
	char pServerString[100];

	printf("\n\n*** Inicializacao do modulo socketmsg em modo cliente");

	if(pServer!=NULL){
		strcpy(pServerString,pServer);
	} else{
		pServerString[0] = '\0';
	};

	// Inicializa variaveis globais, aloca memoria, etc.
	socketmsg_mutexinit(pSocketMsgStruct);
	pSocketMsgStruct->mode = SOCKETMSG_MODE_CLIENT;
	pSocketMsgStruct->buffer = (unsigned char*) malloc(BufferLength);
	if (pSocketMsgStruct->buffer==NULL){
		printf("\n*** Erro: falha na alocao do buffer.");
		pSocketMsgStruct->bufferlen = 0;
		return 0;
	}
	pSocketMsgStruct->bufferlen = BufferLength;
	pSocketMsgStruct->flagverbose = flagverbose;
	
	// Inicializa socket.
	pSocketMsgStruct->clientsocketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pSocketMsgStruct->clientsocketfd == -1) {
		printf("\n*** Erro: falha na chamada de socket(): %s", strerror(errno));
		return 0;
	}

	// Modo cliente:
	// Recover sever structure:
	memset((char *)&pSocketMsgStruct->serveraddress,0,sizeof(pSocketMsgStruct->serveraddress));
	pSocketMsgStruct->serveraddress.sin_family = AF_INET;
	pSocketMsgStruct->serveraddress.sin_port   = htons(PortNumber);

	// Recover sever address:
	switch(ServerAddressMode){
		case SOCKETMSG_SERVERADDRESSMODE_NAME:
			host = gethostbyname(pServerString);
			// Get the server details
			memcpy((char *)&pSocketMsgStruct->serveraddress.sin_addr.s_addr, (char *)host->h_addr, host->h_length);
			break;
		case SOCKETMSG_SERVERADDRESSMODE_IP:
/*			unsigned char ipaddress[4];
			int i,j,ipoint;
			for(i=0,j=0,ipoint=-1;i < (int)strlen(pServerString);++i){
							printf("\n pServerString[%i] = %c ---  %i",i,pServerString[i],(int)strlen(pServerString));
							printf("\n i = %i, j = %i, ipoint = %i",i,j,ipoint);
				if(pServerString[i]=='.'){
					if(j>=4) {return 0;}
					pServerString[i] = '\0';
					ipaddress[j++] = atoi((const char*)&pServerString[ipoint+1]);
					pServerString[i] = '.';
					ipoint = i;
				}
			}
			ipaddress[j++] = atoi((const char*)&pServerString[ipoint+1]);
			if(j!=4){ return 0;}
			printf("\n %i %i %i %i",(int)ipaddress[0],(int)ipaddress[1],(int)ipaddress[2],(int)ipaddress[3]);
			host = gethostbyaddr(ipaddress,4,AF_INET);
			if(host==NULL){
				printf("\n*** Erro: falha em gethostbyaddr(): h_errno = %i", h_errno);
				return 0;
			} */
			pSocketMsgStruct->serveraddress.sin_addr.s_addr = inet_addr(pServerString);
			break;
		default:
			printf("\n*** Erro: argumento ServerAddressMode invalido.");
			return 0;
	}

	printf("\n*** Endereco IP do servidor : %s:%i.",
		inet_ntoa(pSocketMsgStruct->serveraddress.sin_addr), 
		ntohs(pSocketMsgStruct->serveraddress.sin_port));

	if (pSocketMsgStruct->serveraddress.sin_addr.s_addr == INADDR_NONE ) {
		printf("\n*** Erro: falha na atribuicao do nome ou endereco %s do servidor.",pServerString);
		close(pSocketMsgStruct->clientsocketfd);
		return 0;
	}
	// Establish connection with the server
	if (connect(pSocketMsgStruct->clientsocketfd, (struct sockaddr *)(&pSocketMsgStruct->serveraddress),sizeof(pSocketMsgStruct->serveraddress))!=0) {
		printf("\n*** Erro: servidor nao responde a tentativa de conexao.");
		close(pSocketMsgStruct->clientsocketfd);
		return 0;
	} else{
		printf("\n*** Servidor conectado com sucesso.");
	}

	// Inicializa threads.

	// Retorna 

    return 1; 
}                       

/*! \fn int socketmsg_close(socketmsg_t *pSocketMsgStruct)
* Funcao de encerramento
* \param 
* \return
*/

int socketmsg_close(socketmsg_t *pSocketMsgStruct)
{
	// Fecha os sockets.
	//close(pSocketMsgStruct->serversocketfd);
	//close(pSocketMsgStruct->clientsocketfd);
	shutdown(pSocketMsgStruct->serversocketfd, 2);
	shutdown(pSocketMsgStruct->clientsocketfd, 2);
	
	// Desaloca memoria.

	// Retorna 
    return 1; 
}                      


/*****************************************************************************
******************************************************************************
** Funcoes de interface
******************************************************************************
*****************************************************************************/
/*! \fn int socketmsg_server_acceptconnection()
* Funcao de inicializacao.
* \param 
* \return
*/
int socketmsg_server_acceptconnection(socketmsg_t *pSocketMsgStruct)
{
	socklen_t lenclient;

	if(pSocketMsgStruct->mode != SOCKETMSG_MODE_SERVER){
			printf("\n*** Aviso: funcao socketmsg_server_acceptconnection() deve ser chamada em modo servidor.");
	}

	lenclient = sizeof(pSocketMsgStruct->clientaddress);
	do{
		pSocketMsgStruct->clientsocketfd = accept(pSocketMsgStruct->serversocketfd, (struct sockaddr *)(&pSocketMsgStruct->clientaddress),&lenclient);
		if(pSocketMsgStruct->clientsocketfd<0){
			printf("\n*** Erro de conexao: %s", strerror(errno));
			return 0;
		}
		usleep(10000);
	}
	while(pSocketMsgStruct->clientsocketfd < 0);

//	printf("\n*** Cliente conectado a %s:%i a partir de %s.", inet_ntoa(pSocketMsgStruct->serveraddress.sin_addr),ntohs(pSocketMsgStruct->serveraddress.sin_port), inet_ntoa(pSocketMsgStruct->clientaddress.sin_addr));

	return 1;
}

/*! \fn int socketmsg_wait_message(socketmsg_t *pSocketMsgStruct, unsigned char *pHeader, unsigned char **pData, int *pDataSizes, int *pNData)
* Funcao de inicializacao.
* \param 
* \return
*/
int socketmsg_wait_message(socketmsg_t *pSocketMsgStruct, messsage_t *pMessage)
{
	int nBytesRecv,i,j;
	int nTotalBytesRecv;
	int nTotalMessageSize;
	unsigned long nBytesInBuffer;
/*
	if(pSocketMsgStruct->mode != SOCKETMSG_MODE_SERVER){
		remotesockedfd = pSocketMsgStruct->clientsocketfd
	} else{
		remotesockedfd = pSocketMsgStruct->socketfd
	}
*/
	// Entire input buffer
	nTotalBytesRecv = 0;
	nTotalMessageSize = -1;
	do{
		nBytesRecv = recv(pSocketMsgStruct->clientsocketfd, &pSocketMsgStruct->buffer[nTotalBytesRecv], (pSocketMsgStruct->bufferlen-nTotalBytesRecv), 0);
		if (nBytesRecv == -1){
			printf("\n Erro ocorrido quando da recepcao de dados pelo socket : %s.", strerror(errno));
			return 0;
		}
		if(nBytesRecv==0){
			//printf("\n Cliente desconectado.");
			return 0;
		}

		nTotalBytesRecv += nBytesRecv;

		if( (nTotalMessageSize==-1) && (nTotalBytesRecv >= ((int)(1+sizeof(int)))) ){
			nTotalMessageSize = *((int *)(&pSocketMsgStruct->buffer[1]));
		}
		//ioctlsocket(pSocketMsgStruct->clientsocketfd,, FIONREAD, &nBytesInBuffer);

		if( (nTotalMessageSize!=-1) ){
			nBytesInBuffer = nTotalMessageSize - nTotalBytesRecv;
		}
		else{
			nBytesInBuffer = 1;
		}

		//printf("\n nBytesRecv = %i ; BytesInBuffer = %li ; TotalMessageSize = %i",nBytesRecv,nBytesInBuffer,nTotalMessageSize);
	} while(nBytesInBuffer!=0);

	if(pSocketMsgStruct->flagverbose==1){
		printf("\n Received %i bytes:",nTotalBytesRecv);
		for(i=0;i<10;++i){printf(" %X",pSocketMsgStruct->buffer[i] & 0xFF);}
	}

	// format message
	if(nTotalBytesRecv < 1){
		return 0;
	}
	pMessage->Header = pSocketMsgStruct->buffer[0];

	if(pSocketMsgStruct->flagverbose==1){
		printf("\n*** Message received:");
		printf("\n       Header (h): %X", pMessage->Header);
	}

	for(pMessage->NData=0, i = (1+sizeof(int));(pMessage->NData <= SOCKETMSG_MAX_MESSAGE_DATA_FIELDS) && (i<nTotalBytesRecv);++pMessage->NData){
		pMessage->DataSizes[pMessage->NData] = *((int *)(&pSocketMsgStruct->buffer[i]));
		i += sizeof(int);
		pMessage->pData[pMessage->NData] = &pSocketMsgStruct->buffer[i];
		i += pMessage->DataSizes[pMessage->NData];
		if(pSocketMsgStruct->flagverbose){
			printf("\n       DataSizes[%i] : %i", pMessage->NData, pMessage->DataSizes[pMessage->NData]);
			printf("\n       Data (h):");
			for(j=0;(j<10) && (j<pMessage->DataSizes[pMessage->NData]);++j){printf(" %X",(*(pMessage->pData[pMessage->NData] + j)) & 0xFF);}
		}
	}

	if(pSocketMsgStruct->flagverbose==1){
		printf("\n       NData     : %i", pMessage->NData);
	}

	return 1;
}

/*! \fn int socketmsg_send_message(socketmsg_t *pSocketMsgStruct, messsage_t *pMessage)
* Funcao de inicializacao.
* \param 
* \return
*/
int socketmsg_send_message(socketmsg_t *pSocketMsgStruct, messsage_t *pMessage)
{
	int nBytesSent,i,j;
	int TotalDataSize = 0;

	pSocketMsgStruct->buffer[0] = pMessage->Header; // header.
	TotalDataSize = 1 + sizeof(int);  // reserva espao para colocar o numero de bytes a serem enviados.

	for(i=0;i<pMessage->NData;++i){
		if((TotalDataSize + (int)sizeof(int)) > pSocketMsgStruct->bufferlen){
			printf("\n*** Erro: dados a serem enviados por socketmsg_send_message() nao cabem no buffer. Abortando envio.");
			return 0;
		}
		memcpy(&pSocketMsgStruct->buffer[TotalDataSize], (unsigned char *)(&pMessage->DataSizes[i]), sizeof(int));
		TotalDataSize += sizeof(int);
		if((TotalDataSize + pMessage->DataSizes[i]) > pSocketMsgStruct->bufferlen){
			printf("\n*** Erro: dados a serem enviados por socketmsg_send_message() nao cabem no buffer. Abortando envio.");
			return 0;
		}
		memcpy(&pSocketMsgStruct->buffer[TotalDataSize], pMessage->pData[i], pMessage->DataSizes[i]);
		TotalDataSize += pMessage->DataSizes[i];
	}
	*((int *)(&pSocketMsgStruct->buffer[1])) = TotalDataSize;

	nBytesSent = send(pSocketMsgStruct->clientsocketfd,  pSocketMsgStruct->buffer, TotalDataSize, 0);

	if (nBytesSent == -1){
			printf("\n*** Error occurred while receiving from socket : %s.", strerror(errno));
			return 0;
	}
	if(nBytesSent!=TotalDataSize){
		printf("\n*** Erro: trasmitidos %i bytes, mas deveria transmitir %i bytes.",nBytesSent,TotalDataSize);
		return 0;
	}

	if(pSocketMsgStruct->flagverbose==1){
		printf("\n*** Message sent:");
		printf("\n       Header (h): %X", pMessage->Header);
		printf("\n       TotalMessageSize : %i", *((int *)(&pSocketMsgStruct->buffer[1])));
		for(i = 0;i<pMessage->NData;++i){
			printf("\n       DataSizes[%i] : %i", i, pMessage->DataSizes[i]);
			printf("\n       Data (h):");
			for(j=0;(j<10) && (j<pMessage->DataSizes[i]);++j){printf(" %X",(*(pMessage->pData[i] + j)) & 0xFF);}
		}
		printf("\n       NData     : %i", pMessage->NData);
	}

	if(pSocketMsgStruct->flagverbose==1){
		printf("\n Sent %i bytes: ",nBytesSent);
		for(i=0;i<10;++i){printf(" %X",pSocketMsgStruct->buffer[i] & 0xFF);}
	}

	return 1;
}

/*****************************************************************************
******************************************************************************
** Funcoes internas
******************************************************************************
*****************************************************************************/
inline void socketmsg_mutexinit(socketmsg_t *pSocketMsgStruct)
{
	pthread_mutex_init(&pSocketMsgStruct->mutex, NULL);
}

inline int socketmsg_mutexwait(socketmsg_t *pSocketMsgStruct)
{
	if(pthread_mutex_lock(&pSocketMsgStruct->mutex)==0){
		return 1;
	}
	return 0;
}

inline int socketmsg_mutexrelease(socketmsg_t *pSocketMsgStruct)
{
	if(pthread_mutex_unlock(&pSocketMsgStruct->mutex)==0){
		return 1;
	}
	return 0;
}


