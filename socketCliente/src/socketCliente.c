/*
 ============================================================================
 Name        : socketCliente.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>

int main(void) {
	int socketCli;
	char* texto;
	int cantidadDeDatos;

	socketCli = socketCliente("8000", "127.0.0.1");
	*texto = malloc(4 * sizeof(char));
	strcpy(texto, "hola");

	cantidadDeDatos = strlen(texto);
	enviarHeader(socketCli, cantidadDeDatos);
	enviarInformacion(socketCli, texto, &cantidadDeDatos);

	printf("Se mandaron %d bytes\n", cantidadDeDatos);
	close(socketCli);
	free(texto);
	return EXIT_SUCCESS;
}
