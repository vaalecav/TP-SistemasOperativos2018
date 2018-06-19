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
	InformacionEntradas * entradasInstancia = (InformacionEntradas*) malloc(tamanioInformacionEntradas);
	t_config* configuracion;
	char* nombreInstancia;

	nombreInstancia = malloc(largoMensaje + 1);
	configuracion = config_create(ARCHIVO_CONFIGURACION);

	//recibimos el nombre de la instancia que se conecto
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);

	// Logueo la informacion recibida
	log_trace(logCoordinador, "Se conectó la instancia de nombre: %s",
			nombreInstancia);

	//leo cantidad entradas y su respectivo tamanio del archivo de configuracion
	entradasInstancia->cantidad = config_get_int_value(configuracion, "CANTIDAD_ENTRADAS");
	entradasInstancia->tamanio = config_get_int_value(configuracion, "TAMANIO_ENTRADA");

	//enviamos cantidad de entradas y su respectivo tamanio a la instancia
	enviarInformacion(socketInstancia, entradasInstancia, &tamanioInformacionEntradas);

	//Logueamos el envio de informacion
	log_trace(logCoordinador, "Enviamos a la Instancia: Cant. de Entradas = %d; Tam. de Entrada = %d", entradasInstancia->cantidad, entradasInstancia->tamanio);

	// Guardo la instancia en la lista
	instanciaConectada->nombre = nombreInstancia;
	instanciaConectada->socket = socketInstancia;
	instanciaConectada->caida = 0;
	instanciaConectada->claves = list_create();
	instanciaConectada->entradasLibres = entradasInstancia->cantidad;

	pthread_mutex_lock(&mutexListaInstancias);
	list_add(listaInstancias, (void*) instanciaConectada);
	pthread_mutex_unlock(&mutexListaInstancias);

	// Libero memoria
	free(entradasInstancia);
	free(nombreInstancia);
	config_destroy(configuracion);
}

void closeInstancia(void* instanciaVoid) {
	Instancia* instancia = (Instancia*) instanciaVoid;
	close(instancia->socket);
}

void cerrarInstancias() {
	list_destroy_and_destroy_elements(listaInstancias, (void*) closeInstancia);
}

void loguearOperacion(char* nombre, char* mensaje) {
	pthread_mutex_lock(&mutexLog);
	FILE *f = fopen("log.txt", "a");

	if (f == NULL) {
		pthread_mutex_unlock(&mutexLog);
		// Logueamos que no se pudo loguear la operacion
		log_error(logCoordinador, "No se pudo loguear la operación");
		return;
	}
	fprintf(f, "%s || %s\n", nombre, mensaje);

	fclose(f);
	pthread_mutex_unlock(&mutexLog);

	// Logueo que loguié
	log_trace(logCoordinador, "Se pudo loguear: %s || %s", nombre, mensaje);
}

int tiempoRetardoFicticio() {
	t_config* configuracion;
	int retardo;

	configuracion = config_create(ARCHIVO_CONFIGURACION);
	retardo = config_get_int_value(configuracion, "RETARDO");
	config_destroy(configuracion);

	// La funcion usleep usa microsegundos (1 miliseg = 1000 microseg)
	return retardo * 1000;
}

void manejarEsi(int socketEsi, int socketPlanificador, int largoMensaje) {
	char* nombre;
	char* mensaje;
	char** mensajeSplitted;
	ContentHeader * header;

	// Recibo nombre esi
	nombre = malloc(largoMensaje + 1);
	recibirMensaje(socketEsi, largoMensaje, &nombre);

	// Recibo mensaje del esi
	header = (ContentHeader*) malloc(sizeof(ContentHeader));
	header = recibirHeader(socketEsi);
	mensaje = malloc(header->largo + 1);
	recibirMensaje(socketEsi, header->largo, &mensaje);

	// Logueo la operación
	loguearOperacion(nombre, mensaje);

	// Logueo el mensaje
	log_trace(logCoordinador, "Recibimos el mensaje: Nombre = %s; Mensaje = %s",
			nombre, mensaje);

	// Retraso ficticio de la ejecucion
	int retardo = tiempoRetardoFicticio();
	log_trace(logCoordinador, "Hago un retardo de %d microsegundos", retardo);
	usleep(retardo);

	// Ejecuto
	mensajeSplitted = string_split(mensaje, " ");
	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		getClave(mensajeSplitted[1], socketPlanificador, socketEsi);
		log_trace(logCoordinador, "Se ejecuto un GET");
	} else if (strcmp(mensajeSplitted[0], "SET") == 0
			|| strcmp(mensajeSplitted[0], "STORE") == 0) {
		ejecutarSentencia(socketEsi, socketPlanificador, mensaje, nombre);
		log_trace(logCoordinador, "Se ejecuto un %s", mensajeSplitted[0]);

		if (strcmp(mensajeSplitted[0], "SET") == 0)
			free(mensajeSplitted[2]);
	} else {
		log_error(logCoordinador, "Error en el mensaje enviado por el ESI");
	}

	// Libero memoria
	free(header);
	free(nombre);
	free(mensaje);
	free(mensajeSplitted[0]);
	free(mensajeSplitted[1]);
	free(mensajeSplitted);
	close(socketEsi);
}

void manejarDesconexion(int socketInstancia, int largoMensaje) {
	char* nombreInstancia = malloc(largoMensaje + 1);
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);
	Instancia* instancia = malloc(sizeof(Instancia));

	//busco instancia
	pthread_mutex_lock(&mutexListaInstancias);
	instancia = list_find_with_param(listaInstancias, nombreInstancia,
			strcmpVoid);

	if (instancia == NULL) {
		log_error(logCoordinador, "Error en encontrar instancia desconectada: Instrancia: %d", instancia);
		pthread_mutex_unlock(&mutexListaInstancias);
		return;
	}

	// La marco como caída
	instancia->caida = 1;
	pthread_mutex_unlock(&mutexListaInstancias);

	// Logueo la desconexión
	log_trace(logCoordinador, "Se desconectó una instancia");
}

void manejarConexion(void* socketsNecesarios) {
	SocketHilos socketsConectados = *(SocketHilos*) socketsNecesarios;
	ContentHeader * header;

	header = recibirHeader(socketsConectados.socketComponente);

	switch (header->id) {
	case INSTANCIA:
		log_trace(logCoordinador, "Se conectó una instancia");
		manejarInstancia(socketsConectados.socketComponente, header->largo);
		break;

	case ESI:
		log_trace(logCoordinador, "Se conectó un ESI");
		manejarEsi(socketsConectados.socketComponente, socketsConectados.socketPlanificador, header->largo);
		break;

	case INSTANCIA_COORDINADOR_DESCONECTADA:
		log_trace(logCoordinador, "Se desconectó una instancia");
		manejarDesconexion(socketsConectados.socketComponente, header->largo);
		break;
	}

	free(header);
}

int correrEnHilo(SocketHilos socketsConectados) {
	pthread_t idHilo;
	SocketHilos* socketsNecesarios;
	socketsNecesarios = (SocketHilos*) malloc(sizeof(SocketHilos));
	*socketsNecesarios = socketsConectados;

	if (pthread_create(&idHilo, NULL, (void*) manejarConexion,
			(void*) socketsNecesarios)) {
		log_error(logCoordinador, "No se pudo crear el hilo");
		free(socketsNecesarios);
		return 0;
	}

	log_trace(logCoordinador, "Hilo asignado");
	pthread_join(idHilo, NULL);

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
	logCoordinador = log_create(ARCHIVO_LOG, "Coordinador", LOG_PRINT, LOG_LEVEL_TRACE);

	// Leo puertos e ips de archivo de configuracion
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	puerto = config_get_int_value(configuracion, "PUERTO");
	ipPlanificador = config_get_string_value(configuracion, "IP");
	maxConexiones = config_get_int_value(configuracion, "MAX_CONEX");

	// Comienzo a escuchar conexiones
	socketEscucha = socketServidor(puerto, ipPlanificador, maxConexiones);

	// Se conecta el planificador
	socketConectadoPlanificador = servidorConectarComponente(&socketEscucha, "coordinador", "planificador");

	// Logueo la conexion
	log_trace(logCoordinador, "Se conectó el planificador: Puerto=%d; Ip Planificador=%d; Máximas conexiones=%d", puerto, ipPlanificador, maxConexiones);

	// Instancio la lista de instancias
	listaInstancias = list_create();

	// Espero conexiones de ESIs e instancias
	while ((socketComponente = servidorConectarComponente(&socketEscucha, "",
			""))) {
		log_trace(logCoordinador, "Se conectó un componente");

		socketsNecesarios.socketComponente = socketComponente;
		socketsNecesarios.socketPlanificador = socketConectadoPlanificador;

		if (!correrEnHilo(socketsNecesarios)) {
			close(socketComponente);
		}
	}

	// Libero memoria
	close(socketEscucha);
	close(socketConectadoPlanificador);
	free(ipPlanificador);
	config_destroy(configuracion);
	cerrarInstancias();

	return 0;
}

// Cosas para el GET
//si se puede comunicar devuelve 1, si no -1
int sePuedeComunicarConLaInstancia(Instancia* instancia) {
	return enviarHeader(instancia->socket, "", COORDINADOR);
}

bool instanciasNoCaidas(void* instanciaVoid){
	Instancia* instancia = (Instancia*) instanciaVoid;
	if(instancia->caida == 1){
		//instancia se encuentra caida
		return 0;
	}
	//instancia no se encuentra caida
	return 1;
}

void asignarClaveAInstancia(char* key) {
	char* algoritmo_distribucion;
	t_config* configuracion;
	Instancia* instancia;
	t_list* listaInstanciasNoCaidas;

	//Creo la clave
	Clave* clave;
	clave = malloc(sizeof(Clave));
	clave->bloqueado = 1;
	clave->nombre = malloc(strlen(key) + 1);
	strcpy(clave->nombre, key);
	clave->nombre[strlen(key)] = '\0';

	// Leo del archivo de configuracion
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	algoritmo_distribucion = config_get_string_value(configuracion, "ALG_DISTR");

	pthread_mutex_lock(&mutexListaInstancias);
	//filtro instancias que no se encuentren caidas
	listaInstanciasNoCaidas = list_filter(listaInstancias, instanciasNoCaidas);
	pthread_mutex_unlock(&mutexListaInstancias);

	// Selecciono algoritmo de distribucion de instancias
	if (strcmp(algoritmo_distribucion, "EL") == 0) {
		log_trace(logCoordinador, "Utilizo algoritmo de distribucion de instancias EL");
		instancia = algoritmoDistribucionEL(listaInstanciasNoCaidas);
	} else if (strcmp(algoritmo_distribucion, "LSU") == 0) {
		log_trace(logCoordinador, "Utilizo algoritmo de distribucion de instancias LSU");
		instancia = algoritmoDistribucionLSU(listaInstanciasNoCaidas);
	} else if (strcmp(algoritmo_distribucion, "KE") == 0) {
		log_trace(logCoordinador, "Utilizo algoritmo de distribucion de instancias KE");
		instancia = algoritmoDistribucionKE(listaInstanciasNoCaidas, clave->nombre);
	} else {
		log_error(logCoordinador, "Algoritmo de distribucion invalido.");
		return;
	}

	// Libero memoria
	config_destroy(configuracion);
	list_add(instancia->claves, clave);
	list_destroy(listaInstanciasNoCaidas);
}

void getClave(char* key, int socketPlanificador, int socketEsi) {
	Clave* clave;
	void* claveVoid;
	Instancia* instancia;
	void* instanciaVoid;
	int respuestaGET;

	// Busco la clave en la lista de claves
	pthread_mutex_lock(&mutexListaInstancias);
	instanciaVoid = list_find_with_param(listaInstancias, (void*) key,
			buscarInstanciaConClave);

	// Si encontré una instancia, busco su clave
	claveVoid = (instanciaVoid != NULL ? list_find_with_param(((Instancia*) instanciaVoid)->claves, (void*) key, buscarClaveEnListaDeClaves) : NULL);

	pthread_mutex_unlock(&mutexListaInstancias);
	clave = (Clave*) claveVoid;
	// Le aviso al planificador de su estado
	if (instanciaVoid != NULL) {
		instancia = (Instancia*) instanciaVoid;

		pthread_mutex_lock(&mutexListaInstancias);

		// Se tiene que verificar si la instancia no está caída
		if (sePuedeComunicarConLaInstancia(instancia) != -1) {
			// Se fija si la clave se encuentra bloqueada
			if (clave->bloqueado) {
				log_trace(logCoordinador, "La clave se encuentra bloqueada");
				respuestaGET = COORDINADOR_ESI_BLOQUEADO;
				avisarA(socketEsi, "", respuestaGET);
				avisarA(socketPlanificador, key, respuestaGET);
			} else {
				// No esta bloqueada, entonces la bloqueo
				log_trace(logCoordinador, "La clave no se encuentra bloqueada, se bloquea");
				respuestaGET = COORDINADOR_ESI_BLOQUEAR;
				clave->bloqueado = 1;
				avisarA(socketEsi, "", respuestaGET);
				avisarA(socketPlanificador, "", respuestaGET);
			}
			pthread_mutex_unlock(&mutexListaInstancias);
		} else {
			// Instancia está caída
			instancia->caida = 1;
			pthread_mutex_unlock(&mutexListaInstancias);

			respuestaGET = COORDINADOR_INSTANCIA_CAIDA;
			log_error(logCoordinador, "La clave que intenta acceder existe en el sistema pero se encuentra en una instancia que esta desconectada");
			//Le aviso al planificador y esi del error
			avisarA(socketEsi, "", respuestaGET);
			avisarA(socketPlanificador, "", respuestaGET);
		}
	} else {
		// Le asigno la nueva clave a la instancia
		asignarClaveAInstancia(key);
		respuestaGET = COORDINADOR_ESI_CREADO;
		avisarA(socketEsi, "", respuestaGET);
		avisarA(socketPlanificador, key, respuestaGET);
		log_trace(logCoordinador, "Se asigna la nueva clave a la instancia");
	}
}

// Cosas para el SET y para el STORE
void avisarA(int socketAvisar, char* mensaje, int error) {
	enviarHeader(socketAvisar, mensaje, error);
	if (strlen(mensaje) > 1) {
		enviarMensaje(socketAvisar, mensaje);
	}
}

void ejecutarSentencia(int socketEsi, int socketPlanificador, char* mensaje, char* nombreESI) {
	void* instanciaVoid;
	Instancia* instancia;
	ContentHeader *headerEstado, *headerEntradas;

	void* claveVoid;
	Clave* clave;
	char** mensajeSplitted;
	mensajeSplitted = string_split(mensaje, " ");

	// Valido que la clave no exceda el máximo
	if (esSET(mensajeSplitted[0]) && strlen(mensajeSplitted[1]) > 40) {
		// Logueo el error
		log_error(logCoordinador, "Error, la clave excede el tamaño máximo de 40 caracteres");

		// Le aviso al planificador por el error
		avisarA(socketPlanificador, "", COORDINADOR_ESI_ERROR_TAMANIO_CLAVE);

		//Tambien le aviso al esi para que no se quede esperando
		avisarA(socketEsi, "", COORDINADOR_ESI_ERROR_TAMANIO_CLAVE);

		// Libero memoria
		free(mensajeSplitted[0]);
		free(mensajeSplitted[1]);
		free(mensajeSplitted[2]);
		free(mensajeSplitted);
		return;
	}

	// Busco la clave en la lista de claves, devuelvo la instancia que la tenga
	pthread_mutex_lock(&mutexListaInstancias);
	instanciaVoid = list_find_with_param(listaInstancias, (void*) mensajeSplitted[1], buscarInstanciaConClave);

	if (instanciaVoid == NULL) {
		pthread_mutex_unlock(&mutexListaInstancias);

		// Logueo el error
		log_error(logCoordinador, "Clave no identificada");

		// Le aviso al planificador
		avisarA(socketPlanificador, "", COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);

		//Tambien le aviso al esi para que no se quede esperando
		avisarA(socketEsi, "", COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);

		// Libero memoria
		free(mensajeSplitted[0]);
		free(mensajeSplitted[1]);
		free(mensajeSplitted[2]);
		free(mensajeSplitted);
		return;
	}

	instancia = (Instancia*) instanciaVoid;

	if(sePuedeComunicarConLaInstancia(instancia)== -1) {
		instancia->caida = 1;
		pthread_mutex_unlock(&mutexListaInstancias);

		log_error(logCoordinador, "La instancia que se intenta acceder se encuentra caida.");

		// Le aviso al planificador
		avisarA(socketPlanificador, "", INSTANCIA_COORDINADOR_DESCONECTADA);

		//Tambien le aviso al esi para que no se quede esperando
		avisarA(socketEsi, "", INSTANCIA_COORDINADOR_DESCONECTADA);

		// Libero memoria
		free(mensajeSplitted[0]);
		free(mensajeSplitted[1]);
		free(mensajeSplitted[2]);
		free(mensajeSplitted);
		return;
	}

	// Si encontré una instancia, busco su clave
	claveVoid = list_find_with_param(((Instancia*) instanciaVoid)->claves, (void*) mensajeSplitted[1], buscarClaveEnListaDeClaves);

	// Valido que la clave esté bloqueada
	clave = (Clave*) claveVoid;

	if (!clave->bloqueado) {
		pthread_mutex_unlock(&mutexListaInstancias);

		// Logueo el error
		log_error(logCoordinador, "La clave que intenta acceder no se encuentra tomada");

		// Le aviso al planificador
		avisarA(socketPlanificador, "", COORDINADOR_ESI_ERROR_CLAVE_NO_TOMADA);

		//Tambien le aviso al esi para que no se quede esperando
		avisarA(socketEsi, "", COORDINADOR_ESI_ERROR_CLAVE_NO_TOMADA);

		// Libero memoria
		free(mensajeSplitted[0]);
		free(mensajeSplitted[1]);
		free(mensajeSplitted[2]);
		free(mensajeSplitted);
		return;
	}

	if (esSTORE(mensajeSplitted[0])) {
		// Si es STORE, tengo que desbloquear la clave
		clave->bloqueado = 0;

		// Logueo -\_('.')_/-
		log_trace(logCoordinador, "Desbloqueo la clave %s", clave->nombre);
	}

	pthread_mutex_unlock(&mutexListaInstancias);

	// Si llega hasta acá es porque es válido, le mando el mensaje a la instancia
	enviarHeader(instancia->socket, mensaje, COORDINADOR);
	enviarMensaje(instancia->socket, mensaje);

	if (esSET(mensajeSplitted[0])) {
		// Espero cantidad de entradas libres de la instancia
		headerEntradas = recibirHeader(instancia->socket);

		//le asigno la cantidad de entradas libres a la instancia
		pthread_mutex_lock(&mutexListaInstancias);
		instancia->entradasLibres = headerEntradas->id;
		pthread_mutex_unlock(&mutexListaInstancias);
	}

	// Espero la respuesta de la instancia
	headerEstado = recibirHeader(instancia->socket);

	switch (headerEstado->id) {
	case INSTANCIA_SENTENCIA_OK_SET:
		// Si está OK, le aviso al ESI y al planificador
		log_trace(logCoordinador, "La sentencia se ejecutó correctamente");
		avisarA(socketEsi, "", INSTANCIA_SENTENCIA_OK_SET);
		avisarA(socketPlanificador, "", INSTANCIA_SENTENCIA_OK_SET);
		break;
	case INSTANCIA_SENTENCIA_OK_STORE:
		// Si está OK, le aviso al ESI y al planificador
		log_trace(logCoordinador, "La sentencia se ejecutó correctamente");
		avisarA(socketEsi, "", INSTANCIA_SENTENCIA_OK_STORE);
		avisarA(socketPlanificador, mensajeSplitted[1],	INSTANCIA_SENTENCIA_OK_STORE);
		break;

	case INSTANCIA_CLAVE_NO_IDENTIFICADA:
		// Cuando hay un error, le aviso al planificador
		log_error(logCoordinador, "Clave no identificada");

		//Le aviso al planificador
		avisarA(socketPlanificador, "", COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);
		//Tambien le aviso al esi para que no se quede esperando
		avisarA(socketEsi, "", COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);
		break;

	case INSTANCIA_ERROR:
		log_error(logCoordinador, "Error no contemplado");
		break;
	}

	// Libero memoria
	free(mensajeSplitted[0]);
	free(mensajeSplitted[1]);
	free(mensajeSplitted[2]);
	free(mensajeSplitted);
	free(headerEstado);
}

int esSET(char* sentencia) {
	return strcmp(sentencia, "SET") == 0;
}

int esSTORE(char* sentencia) {
	return strcmp(sentencia, "STORE") == 0;
}
