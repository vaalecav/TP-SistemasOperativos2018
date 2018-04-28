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
#include <configuracion/configuracion.h>

#define PUERTO_COORDINADOR 8000
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_PLANIFICADOR 8001
#define IP_PLANIFICADOR "127.0.0.2"

int main() {
/*
	char* valor;
	valor = malloc(255);
	int respuesta = leerConfiguracion("IP_COORDINADOR", &valor);
	printf("%s", valor);
	free(valor);
	*/
	puts("Iniciando ESI.");
	int socketCoordinador, socketPlanificador;

	socketCoordinador = clienteConectarComponente("ESI", "coordinador", PUERTO_COORDINADOR, IP_COORDINADOR);
	socketPlanificador = clienteConectarComponente("ESI", "planificador", PUERTO_PLANIFICADOR, IP_PLANIFICADOR);

	close(socketCoordinador);
	close(socketPlanificador);
	puts("El ESI se ha finalizado correctamente.");
	return 0;
}


