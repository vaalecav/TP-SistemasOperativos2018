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

int buscarClaveEnListaDeClaves(void* structClaveVoid, void* claveVoid) {
	Clave* structClave = (Clave*)structClaveVoid;
	return strcmp(structClave->nombre, (char*)claveVoid) == 0;
}

int buscarInstanciaConClave(void* instanciaVoid, void* claveVoid) {
	Instancia* instancia = (Instancia*)instanciaVoid;
	return list_find_with_param(instancia->claves, claveVoid, buscarClaveEnListaDeClaves) != NULL;
}

void manejarInstancia(int socketInstancia, int largoMensaje) {
	int tamanioInformacionEntradas = sizeof(InformacionEntradas);
	Instancia *instanciaConectada = (Instancia*) malloc(sizeof(Instancia));
	InformacionEntradas * entradasInstancia = (InformacionEntradas*) malloc(tamanioInformacionEntradas);
	t_config* configuracion;
	char* nombreInstancia;

	nombreInstancia = malloc(largoMensaje + 1);
	configuracion = config_create("./configuraciones/configuracion.txt");

	//recibimos el nombre de la instancia que se conecto
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);

	//leo cantidad entradas y su respectivo tamanio del archivo de configuracion
	entradasInstancia->cantidad = config_get_int_value(configuracion, "CANTIDAD_ENTRADAS");
	entradasInstancia->tamanio = config_get_int_value(configuracion, "TAMANIO_ENTRADA");

	//enviamos cantidad de entradas y su respectivo tamanio a la instancia
	enviarInformacion(socketInstancia, entradasInstancia, &tamanioInformacionEntradas);

	instanciaConectada->nombre = nombreInstancia;
	instanciaConectada->socket = socketInstancia;
	instanciaConectada->claves = list_create();

	//agregamos a la lista de instancias
	pthread_mutex_lock(&mutexListaInstancias);
	list_add(listaInstancias, (void*) instanciaConectada);
	pthread_mutex_unlock(&mutexListaInstancias);

	free(entradasInstancia);
	free(nombreInstancia);
	config_destroy(configuracion);
}

void closeInstancia(void* instancia) {
	close(*(int*)instancia);
}

void cerrarInstancias() {
	list_destroy_and_destroy_elements(listaInstancias, (void*) closeInstancia);
}

void loguearOperacion(char* nombre, char* mensaje) {
	pthread_mutex_lock(&mutexLog);
	FILE *f = fopen("log.txt", "a");
	if (f == NULL) {
		puts("No se pudo loguear la operación");
		pthread_mutex_unlock(&mutexLog);
		return;
	}

	fprintf(f, "%s || %s\n", nombre, mensaje);
	pthread_mutex_unlock(&mutexLog);
	fclose(f);
}

int tiempoRetardoFicticio() {
	t_config* configuracion;
	int retardo;

	configuracion = config_create("./configuraciones/configuracion.txt");
	retardo = config_get_int_value(configuracion, "RETARDO");
	config_destroy(configuracion);

	// La funcion usleep usa microsegundos (1 seg = 1000 microseg)
	return retardo * 1000;
}

void manejarEsi(int socketEsi, int socketPlanificador, int largoMensaje) {
	char* nombre;
	char* mensaje;
	char** mensajeSplitted;
	ContentHeader * header;

	// Recibo mensajes
	nombre = malloc(largoMensaje + 1);
	recibirMensaje(socketEsi, largoMensaje, &nombre);

	header = recibirHeader(socketEsi);
	mensaje = malloc(header->largo + 1);
	recibirMensaje(socketEsi, header->largo, &mensaje);

	// Logueo la operación
	loguearOperacion(nombre, mensaje);

	// Retraso ficticio de la ejecucion
	usleep(tiempoRetardoFicticio());

	// Ejecuto
	mensajeSplitted = string_split(mensaje, " ");
	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		getClave(mensajeSplitted[1], socketPlanificador);
	} else if (strcmp(mensajeSplitted[0], "SET") == 0) {
		setClave(socketEsi, mensaje);
	} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
		puts("STORE");
	} else {
		puts("Error en el mensaje enviado al coordinador por le ESI");
	}

	// Libero memoria
	free(header);
	free(nombre);
	free(mensaje);
	free(mensajeSplitted);
	close(socketEsi);
}

void manejarConexion(void* socketsNecesarios) {
	SocketHilos socketsConectados = *(SocketHilos*)socketsNecesarios;
	ContentHeader * header;

	header = recibirHeader(socketsConectados.socketComponente);

	switch(header->id){
		case INSTANCIA:
			manejarInstancia(socketsConectados.socketComponente, header->largo);
			break;

		case ESI:
			manejarEsi(socketsConectados.socketComponente, socketsConectados.socketPlanificador, header->largo);
			break;
	}

	free(header);
}

int correrEnHilo(SocketHilos socketsConectados) {
	pthread_t idHilo;
	SocketHilos* socketsNecesarios;
	socketsNecesarios = malloc(sizeof(SocketHilos));
	*socketsNecesarios = socketsConectados;

	printf("Antes del Hilo\n");
	if (pthread_create(&idHilo, NULL, (void*) manejarConexion, (void*)socketsNecesarios)) {
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
	int socketEscucha, socketComponente, socketConectadoPlanificador;
	SocketHilos socketsNecesarios;
	t_config* configuracion;
	int puerto;
	int maxConexiones;
	char* ipPlanificador;

	indexInstanciaEL = 0;

	//Leo puertos e ips de archivo de configuracion
	configuracion = config_create("./configuraciones/configuracion.txt");
	puerto = config_get_int_value(configuracion, "PUERTO");
	ipPlanificador = config_get_string_value(configuracion, "IP");
	maxConexiones = config_get_int_value(configuracion, "MAX_CONEX");

	socketEscucha = socketServidor(puerto, ipPlanificador, maxConexiones);

	socketConectadoPlanificador = servidorConectarComponente(&socketEscucha, "coordinador", "planificador");

	listaInstancias = list_create();
	while((socketComponente = servidorConectarComponente(&socketEscucha,"",""))) {//preguntar si hace falta mandar msjes de ok x cada hilo
		socketsNecesarios.socketComponente = socketComponente;
		socketsNecesarios.socketPlanificador = socketConectadoPlanificador;
		correrEnHilo(socketsNecesarios);
	}

	close(socketEscucha);
	close(socketConectadoPlanificador);
	free(ipPlanificador);
	config_destroy(configuracion);
	cerrarInstancias();
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
