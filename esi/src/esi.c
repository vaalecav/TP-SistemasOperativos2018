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

int main() {

	puts("Iniciando ESI.");
	int socketCoordinador, socketPlanificador;
	char ipCoordinador[16];
	int puertoCoordinador;
	char ipPlanificador[16];
	int puertoPlanificador;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO_COORDINADOR:%d", &puertoCoordinador);
	leerConfiguracion("IP_COORDINADOR:%s", &ipCoordinador);
	leerConfiguracion("PUERTO_PLANIFICADOR:%d", &puertoPlanificador);
	leerConfiguracion("IP_PLANIFICADOR%s", &ipPlanificador);

	socketCoordinador = clienteConectarComponente("ESI", "coordinador", puertoCoordinador, ipCoordinador);
	socketPlanificador = clienteConectarComponente("ESI", "planificador", puertoPlanificador, ipPlanificador);

	close(socketCoordinador);
	close(socketPlanificador);
	puts("El ESI se ha finalizado correctamente.");
	return 0;
}


