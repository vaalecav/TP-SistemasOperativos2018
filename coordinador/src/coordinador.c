/*
 ============================================================================
 Name        : coordinador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Coordinador
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>

//constantes
#define PUERTO 8000
#define IP "127.0.0.1"


int main() {
	puts("Iniciando Coordinador.");
	int miSocket;
	miSocket = servidorConectarComponente("coordinador", "esi", PUERTO, IP);

	close(miSocket);
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
