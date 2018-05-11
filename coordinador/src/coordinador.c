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
#include <commons/string.h>
#include <configuracion/configuracion.h>
#include <pthread.h>

void manejarInstancia(int socketInstancia){
	// TODO Acá se tiene que agregar a la lista de instancias el socket
}

void manejarEsi(int socketEsi, int largoMensaje) {
	// TODO Agarrar del archivo de configuracion el algoritmo de distribución
	char* mensaje;
	char** mensajeSplitted;

	mensaje = malloc(largoMensaje + 1);
	recibirMensaje(socketEsi, largoMensaje, &mensaje);

	mensajeSplitted = string_split(mensaje, " ");

	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		puts("GET");
	} else if (strcmp(mensajeSplitted[0], "SET") == 0) {
		puts("SET");
		// TODO Acá, de la lista de instancias, hay que elegir dependiendo del tipo que se obtiene por configuracion
	} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
		puts("STORE");
	} else {
		puts("Error en el mensaje enviado al coordinador por le ESI");
	}

	free(mensaje);
}

void *manejarConexion(void* nuevoSocket) {

	int socketConectado = *(int*)nuevoSocket;
	ContentHeader * header;

	header = recibirHeader(socketConectado);

	switch(header->id){
		case INSTANCIA:
			manejarInstancia(socketConectado);
			break;

		case ESI:
			manejarEsi(socketConectado, header->largo);
			break;
	}

	free(header);
	close(socketConectado);
	puts("hola");

	return manejarConexion;
}

int correrEnHilo(int socketConectado) {
	pthread_t idHilo;
	int *nuevoSocket;
	nuevoSocket = malloc(sizeof(int));
	*nuevoSocket = socketConectado;

	printf("Antes del Hilo\n");
	if (pthread_create(&idHilo, NULL, (void*) manejarConexion, (void*)nuevoSocket)) {
		perror("No se pudo crear el Hilo");
		exit(1);
	}

	puts("Manejador asignado");
	pthread_join(idHilo, NULL);
	printf("Despues del Hilo\n");

	return 1;
}

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
	close(servidorConectarComponente(&socketEscucha, "coordinador", "instancia"));
	close(servidorConectarComponente(&socketEscucha, "coordinador", "planificador"));
	close(servidorConectarComponente(&socketEscucha, "coordinador", "esi"));

	while((socketComponente = servidorConectarComponente(&socketEscucha,"",""))) {//preguntar si hace falta mandar msjes de ok x cada hilo
		puts("inicia \n");correrEnHilo(socketComponente);
	}

	close(socketEscucha);
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
