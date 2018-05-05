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
#include "../../libraries/socket/sockets.h"
#include <configuracion/configuracion.h>

int main() {
	puts("Iniciando Coordinador.");
	int socketEscucha, socketEsi, socketInstancia, socketPlanificador;

	char ip[16];
	int puerto;
	int maxConexiones;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO:%d", &puerto);
	leerConfiguracion("IP:%s", &ip);
	leerConfiguracion("MAX_CONEX:%d", &maxConexiones);

	socketEscucha = socketServidor(puerto, ip, maxConexiones);
	socketInstancia = servidorConectarComponente(&socketEscucha, "coordinador", "instancia");
	socketPlanificador = servidorConectarComponente(&socketEscucha, "coordinador", "planificador");
	socketEsi = servidorConectarComponente(&socketEscucha, "coordinador", "esi");

	close(socketEscucha);
	close(socketEsi);
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
