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
	Clave* structClave = (Clave*) structClaveVoid;
	return strcmp(structClave->nombre, (char*) claveVoid) == 0;
}

int buscarInstanciaConClave(void* instanciaVoid, void* claveVoid) {
	Instancia* instancia = (Instancia*) instanciaVoid;
	return list_find_with_param(instancia->claves, claveVoid,
			buscarClaveEnListaDeClaves) != NULL;
}

void manejarInstancia(int socketInstancia, int largoMensaje) {
	int tamanioInformacionEntradas = sizeof(InformacionEntradas);
	Instancia *instanciaConectada = (Instancia*) malloc(sizeof(Instancia));
	InformacionEntradas * entradasInstancia = (InformacionEntradas*) malloc(
			tamanioInformacionEntradas);
	t_config* configuracion;
	char* nombreInstancia;

	nombreInstancia = malloc(largoMensaje + 1);
	configuracion = config_create("./configuraciones/configuracion.txt");

	//recibimos el nombre de la instancia que se conecto
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);

	// Logueo la informacion recibida
	log_trace(logCoordinador, "Recibimos la instancia: Nombre = %d",
			nombreInstancia);

	//leo cantidad entradas y su respectivo tamanio del archivo de configuracion
	entradasInstancia->cantidad = config_get_int_value(configuracion,
			"CANTIDAD_ENTRADAS");
	entradasInstancia->tamanio = config_get_int_value(configuracion,
			"TAMANIO_ENTRADA");

	//enviamos cantidad de entradas y su respectivo tamanio a la instancia
	enviarInformacion(socketInstancia, entradasInstancia,
			&tamanioInformacionEntradas);

	instanciaConectada->nombre = nombreInstancia;
	instanciaConectada->socket = socketInstancia;
	instanciaConectada->caida = 0;
	instanciaConectada->claves = list_create();

	//Logueamos el envio de informacion
	log_trace(logCoordinador,
			"Enviamos a la Instancia: Cant. de Entradas = %d; Tam. de Entrada = %d",
			entradasInstancia->cantidad, entradasInstancia->tamanio);

	//agregamos a la lista de instancias
	pthread_mutex_lock(&mutexListaInstancias);
	list_add(listaInstancias, (void*) instanciaConectada);
	pthread_mutex_unlock(&mutexListaInstancias);

	// loguear algo aca?

	free(entradasInstancia);
	free(nombreInstancia);
	config_destroy(configuracion);
}

void closeInstancia(void* instancia) {
	close(*(int*) instancia);
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
		//Logueamos que no se pudo loguear la operacion
		log_trace(logCoordinador, "No se pudo loguear la operación");
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
	// Logueo el mensaje
	log_trace(logCoordinador, "Recibimos el mensaje: Nombre = %d; Mensaje = %d",
			nombre, mensaje);

	// Retraso ficticio de la ejecucion
	usleep(tiempoRetardoFicticio());

	// Ejecuto
	mensajeSplitted = string_split(mensaje, " ");
	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		getClave(mensajeSplitted[1], socketPlanificador, socketEsi);
		log_trace(logCoordinador, "Se ejecuto un GET");
	} else if (strcmp(mensajeSplitted[0], "SET") == 0) {
		ejecutarSentencia(socketEsi, mensaje);
		log_trace(logCoordinador, "Se ejecuto un SET");
	} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
		ejecutarSentencia(socketEsi, mensaje);
		log_trace(logCoordinador, "Se ejecuto un STORE");
	} else {
		puts("Error en el mensaje enviado al coordinador por le ESI");
		log_trace(logCoordinador,
				"Error en el mensaje enviado al coordinador por el ESI");
	}

	// Libero memoria
	free(header);
	free(nombre);
	free(mensaje);
	free(mensajeSplitted);
	close(socketEsi);
}

void manejarDesconexion(int socketInstancia, int largoMensaje) {
	char* nombreInstancia = malloc(largoMensaje + 1);
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);
	Instancia* instancia = malloc(sizeof(Instancia));

	//busco instancia
	pthread_mutex_lock(&mutexListaInstancias);
	instancia = list_find_with_param(listaInstancias, nombreInstancia, strcmp);

	if (instancia == NULL) {
		puts("Error en encontrar instancia desconectada");
		log_trace(logCoordinador,
				"Error en encontrar instancia desconectada: Instrancia: %d",
				instancia);
	}
	instancia->caida = 1;

	pthread_mutex_unlock(&mutexListaInstancias);

	return;
}

void manejarConexion(void* socketsNecesarios) {
	SocketHilos socketsConectados = *(SocketHilos*) socketsNecesarios;
	ContentHeader * header;

	header = recibirHeader(socketsConectados.socketComponente);

	switch (header->id) {
	case INSTANCIA:
		manejarInstancia(socketsConectados.socketComponente, header->largo);
		log_trace(logCoordinador, "Se maneja una instancia");
		break;

	case ESI:
		manejarEsi(socketsConectados.socketComponente,
				socketsConectados.socketPlanificador, header->largo);
		log_trace(logCoordinador, "Se maneja un ESI");
		break;

	case INSTANCIA_COORDINADOR_DESCONECTADA:
		manejarDesconexion(socketsConectados.socketComponente, header->largo);
		log_trace(logCoordinador, "Se maneja una instancia desconectada");
		break;

	case INSTANCIA_SENTENCIA_OK:
		log_trace(logCoordinador, "Se maneja una instancia conectada");
		break;

	case INSTANCIA_ERROR:
		// Son casos no contemplados
		break;

	case INSTANCIA_CLAVE_NO_IDENTIFICADA:
		// TODO ¿a quién hay que avisarle?
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
	if (pthread_create(&idHilo, NULL, (void*) manejarConexion,
			(void*) socketsNecesarios)) {
		perror("No se pudo crear el Hilo");
		log_trace(logCoordinador, "No se pudo crear el Hilo");
		exit(1);
	}

	puts("Manejador asignado");
	log_trace(logCoordinador, "Manejador asignado");
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

	// Inicio el log
	logCoordinador = log_create(ARCHIVO_LOG, "Coordinador", true,
			LOG_LEVEL_TRACE);

	//Leo puertos e ips de archivo de configuracion
	configuracion = config_create("./configuraciones/configuracion.txt");
	puerto = config_get_int_value(configuracion, "PUERTO");
	ipPlanificador = config_get_string_value(configuracion, "IP");
	maxConexiones = config_get_int_value(configuracion, "MAX_CONEX");

	//Comienza a escuchar conexiones
	socketEscucha = socketServidor(puerto, ipPlanificador, maxConexiones);

	// Se conecta
	socketConectadoPlanificador = servidorConectarComponente(&socketEscucha,
			"coordinador", "planificador");

	// Logueo la conexion
	log_trace(logCoordinador,
			"Se conectó el planificador: Puerto=%d; Ip Planificador=%d; Maximas Conexiones",
			puerto, ipPlanificador, maxConexiones);

	listaInstancias = list_create();

	while ((socketComponente = servidorConectarComponente(&socketEscucha, "",
			""))) { //preguntar si hace falta mandar msjes de ok x cada hilo
		socketsNecesarios.socketComponente = socketComponente;
		socketsNecesarios.socketPlanificador = socketConectadoPlanificador;
		correrEnHilo(socketsNecesarios);
	} // loguear aca???

	close(socketEscucha);
	close(socketConectadoPlanificador);
	free(ipPlanificador);
	config_destroy(configuracion);
	cerrarInstancias();
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
