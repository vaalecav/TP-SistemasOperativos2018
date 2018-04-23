/*
 * servidor.c
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */
#include "servidor.h"

int main(void){
	size_t tamanioMensaje;
	int miSocket;
	miSocket = socketServidor(PUERTO,IP);
	tamanioMensaje = recibirHeader(miSocket);
	recibirMensaje(miSocket, tamanioMensaje);

	close(miSocket);
	return 0;
}

