/*
 ============================================================================
 Name        : coordinador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Coordinador
 ============================================================================
 */
#include "coordinador.h"

void manejarInstancia(int socketInstancia) {
	pthread_mutex_lock(&mutexListaInstancias);
	list_add(listaInstancias, (void*) socketInstancia);
	pthread_mutex_unlock(&mutexListaInstancias);
}

void closeInstancia(void* instancia) {
	close(*(int*) instancia);
}

void cerrarInstancias() {
	list_destroy_and_destroy_elements(listaInstancias, (void*) closeInstancia);
}

void asignarInstancia(char* mensaje) {
	char algoritmo_distribucion[4];
	void* socketInstancia;

	leerConfiguracion("ALG_DISTR:%s", &algoritmo_distribucion);

	pthread_mutex_lock(&mutexListaInstancias);
	if (strcmp(algoritmo_distribucion, "EL") == 0) {
		socketInstancia = algoritmoDistribucionEL(listaInstancias);
	} else if (strcmp(algoritmo_distribucion, "LSU") == 0) {
		// TODO Implementar algoritmo Least Space Used
	} else if (strcmp(algoritmo_distribucion, "KE") == 0) {
		// TODO Implementar Key Explicit
	} else {
		puts("Error al asignar instancia. Algoritmo de distribucion invalido.");
		pthread_mutex_unlock(&mutexListaInstancias);
		return;
	}
	pthread_mutex_unlock(&mutexListaInstancias);

	if (socketInstancia != NULL) {
		enviarHeader(*(int*) socketInstancia, mensaje, COORDINADOR);
		enviarMensaje(*(int*) socketInstancia, mensaje);
	} else {
		puts("No hay instancias creadas a la cual asignar el mensaje.");
	}
}

void manejarEsi(int socketEsi, int largoMensaje) {
	char* mensaje;
	char** mensajeSplitted;

	mensaje = malloc(largoMensaje + 1);
	recibirMensaje(socketEsi, largoMensaje, &mensaje);

	mensajeSplitted = string_split(mensaje, " ");

	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		puts("GET");
	} else if (strcmp(mensajeSplitted[0], "SET") == 0) {
		puts("SET");
		asignarInstancia(mensaje);
		// TODO Acá, de la lista de instancias, hay que elegir dependiendo del tipo que se obtiene por configuracion
	} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
		puts("STORE");
	} else {
		puts("Error en el mensaje enviado al coordinador por le ESI");
	}

	free(mensaje);
}

void manejarConexion(void* nuevoSocket) {
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
	int socketEscucha, socketComponente;

	char ip[16];
	int puerto;
	int maxConexiones;

	indexInstanciaEL = 0;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO:%d", &puerto);
	leerConfiguracion("IP:%s", &ip);
	leerConfiguracion("MAX_CONEX:%d", &maxConexiones);

	socketEscucha = socketServidor(puerto, ip, maxConexiones);
	//close(servidorConectarComponente(&socketEscucha, "coordinador", "instancia"));
	//close(servidorConectarComponente(&socketEscucha, "coordinador", "planificador"));
	//close(servidorConectarComponente(&socketEscucha, "coordinador", "esi"));

	listaInstancias = list_create();
	while((socketComponente = servidorConectarComponente(&socketEscucha,"",""))) {//preguntar si hace falta mandar msjes de ok x cada hilo
		correrEnHilo(socketComponente);
	}

	close(socketEscucha);
	cerrarInstancias();
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
