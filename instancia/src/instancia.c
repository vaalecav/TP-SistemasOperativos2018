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
#include <configuracion/configuracion.h>

int main() {
	puts("Iniciando Instancia.");
	int socketCoordinador;
	char ipCoordinador[16];
	int puertoCoordinador;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO_COORDINADOR:%d", &puertoCoordinador);
	leerConfiguracion("IP_COORDINADOR:%s", &ipCoordinador);

	socketCoordinador = clienteConectarComponente("instancia", "coordinador", puertoCoordinador, ipCoordinador);

	close(socketCoordinador);
	puts("La Instancia se ha finalizado correctamente.");
	return 0;
}
