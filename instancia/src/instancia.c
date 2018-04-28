/*
 ============================================================================
 Name        : instancia.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Instancia
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>

#define PUERTO_COORDINADOR 8000
#define IP_COORDINADOR "127.0.0.1"

int main() {
	puts("Iniciando Instancia.");
	int socketCoordinador;

	socketCoordinador = clienteConectarComponente("instancia", "coordinador", PUERTO_COORDINADOR, IP_COORDINADOR);

	close(socketCoordinador);
	puts("La Instancia se ha finalizado correctamente.");
	return 0;
}
