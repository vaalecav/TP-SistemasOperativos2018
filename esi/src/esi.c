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

int main() {
	puts("Iniciando ESI.");
	int socketCli;
	char* texto;

	//conectar con Planificador y Coordinador
	socketCli = socketCliente("8000", "127.0.0.1");
	texto = malloc(4 * sizeof(char));
	strcpy(texto, "hola");

	enviarHeader(socketCli, texto);
	enviarMensaje(socketCli, texto);

	printf("Se mandaron %d bytes\n", strlen(texto));
	close(socketCli);
	free(texto);
	puts("El ESI se ha finalizado correctamente.");
	return 0;
}
