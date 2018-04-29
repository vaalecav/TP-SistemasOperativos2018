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

	puts("Iniciando ESI.");
	int socketCoordinador, socketPlanificador;
	char ipCoordinador[16];
	int puertoCoordinador;
	char ipPlanificador[16];
	int puertoPlanificador;

	leerConfiguracion("PUERTO_COORDINADOR:%d", &puertoCoordinador);
	leerConfiguracion("IP_COORDINADOR:%s", &ipCoordinador);
	leerConfiguracion("PUERTO_PLANIFICADOR:%d", &puertoPlanificador);
	leerConfiguracion("IP_PLANIFICADOR%s", &ipPlanificador);
/*
	printf("PUERTO_COORDINADOR: %d\n", puertoCoordinador);
	printf("IP_COORDINADOR: %s\n", ipCoordinador);
	printf("PUERTO_PLANIFICADOR: %d\n", puertoPlanificador);
	printf("IP_PLANIFICADOR: %s\n", ipPlanificador);

	socketCoordinador = clienteConectarComponente("ESI", "coordinador", puertoCoordinador, ipCoordinador);
	socketPlanificador = clienteConectarComponente("ESI", "planificador", puertoPlanificador, ipPlanificador);*/

	socketCoordinador = clienteConectarComponente("ESI", "coordinador", PUERTO_COORDINADOR, IP_COORDINADOR);
	socketPlanificador = clienteConectarComponente("ESI", "planificador", PUERTO_PLANIFICADOR, IP_PLANIFICADOR);

	close(socketCoordinador);
	close(socketPlanificador);
	puts("El ESI se ha finalizado correctamente.");
	return 0;
}


