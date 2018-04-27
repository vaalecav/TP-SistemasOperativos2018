/*
 ============================================================================
 Name        : esi.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso ESI
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>

#define PUERTO_COORDINADOR 8000
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_PLANIFICADOR 8001
#define IP_PLANIFICADOR "127.0.0.2"

int conectarCoordinador(){
	int socketCli;
	socketCli = socketCliente(PUERTO_COORDINADOR, IP_COORDINADOR);
	enviarMensaje(socketCli, "esi");
	return 1;
}

int main() {
	puts("Iniciando ESI.");
	int socketCli;
	char* texto;

	socketCli = socketCliente(PUERTO_COORDINADOR, IP_COORDINADOR);
	texto = malloc(2 * sizeof(char));
	strcpy(texto, "OK");

	enviarMensaje(socketCli, texto);

	printf("Se mandaron %d bytes\n", strlen(texto));
	close(socketCli);
	free(texto);
	puts("El ESI se ha finalizado correctamente.");
	return 0;
}


