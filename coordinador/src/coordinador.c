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
#include <pthread.h>

int main() {
	puts("Iniciando Coordinador.");
	int socketEscucha, socketEsi, socketInstancia, socketPlanificador, socketComponente;

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

	while(socketComponente = servidorConectarComponente(&socketEscucha,"","")){//preguntar si hace falta mandar msjes de ok x cada hilo
		correrEnHilo(socketComponente, manejadorConexion());
	}

	close(socketEscucha);
	close(socketEsi);
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}

//recibe funcion manejadora
int correrEnHilo(int socketConectado, void* (*manejadorConexion)(void *)){
	pthread_t idHilo;
	int *nuevoSocket;
	nuevoSocket = malloc(sizeof(int));
	*nuevoSocket = socketConectado;

	printf("Antes del Hilo\n");
	if(pthread_create(&idHilo, NULL, manejadorConexion , (void*)nuevoSocket)){
		perror("No se pudo crear el Hilo");
		exit(1);
	}
	puts("Manejador asignado");
	pthread_join(idHilo, NULL);
	printf("Despues del Hilo\n");


	return 1;
}

void manejadorConexion (void* nuevoSocket){
	int socket = *(int*)nuevoSocket;
	ContentHeader * header;

	header = recibirHeader(socket);

	switch(header->id){
		case INSTANCIA: manejadorInstancia(socket);
			break;
		case ESI: manejadorEsi(socket);
			break;
	}
	//fijarse donde tiene que terminar el thread
}

void manejadorInstancia(int socketInstancia){

	free(socketInstancia);
	//terminar thread?
}

void manejadorEsi(int socketEsi){

	free(socketEsi);
	//terminar thread?
}
