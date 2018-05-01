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
#include <configuracion/configuracion.h>

int main() {
	puts("Iniciando Coordinador.");
	int socketEscucha, socketEsi, socketInstancia, socketPlanificador;

	char ip[16];
	int puerto;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO:%d", &puerto);
	leerConfiguracion("IP:%s", &ip);

	socketEscucha = socketServidor(puerto, ip);
	socketInstancia = servidorConectarComponente(&socketEscucha, "coordinador", "instancia");
	socketEsi = servidorConectarComponente(&socketEscucha, "coordinador", "esi");
	socketPlanificador = servidorConectarComponente(&socketEscucha, "coordinador", "planificador");

	close(socketEscucha);
	close(socketEsi);
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
