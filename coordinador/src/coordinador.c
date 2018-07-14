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
	return list_find_with_param(instancia->claves, claveVoid, buscarClaveEnListaDeClaves) != NULL;
}

int buscarClaveDeEsi(void* structClaveVoid, void* nombreEsiVoid) {
	Clave* structClave = (Clave*) structClaveVoid;
	return strcmp(structClave->nombreEsi, (char*) nombreEsiVoid) == 0;
}

int buscarInstanciaConEsi(void* instanciaVoid, void* nombreEsiVoid) {
	Instancia* instancia = (Instancia*) instanciaVoid;
	return list_find_with_param(instancia->claves, nombreEsiVoid, buscarClaveDeEsi) != NULL;
}

int buscarNombreDeLaInstancia(void* instancia, void* nombre) {
	return strcmp(((Instancia*)instancia)->nombre, (char*)nombre) == 0;
}

void manejarInstancia(int socketInstancia, int largoMensaje) {
	int tamanioInformacionEntradas = sizeof(InformacionEntradas);
	InformacionEntradas * entradasInstancia = (InformacionEntradas*) malloc(tamanioInformacionEntradas);
	t_config* configuracion;
	char* nombreInstancia;

	ContentHeader* headerEntradasLibres;
	int entradasLibres;

	Instancia* instanciaConectada;
	void* instanciaVoid;

	nombreInstancia = malloc(largoMensaje + 1);
	configuracion = config_create(ARCHIVO_CONFIGURACION);

	//recibimos el nombre de la instancia que se conecto
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);

	// Logueo la informacion recibida
	log_trace(logCoordinador, "Se conectó la instancia de nombre: %s", nombreInstancia);

	//leo cantidad entradas y su respectivo tamanio del archivo de configuracion
	entradasInstancia->cantidad = config_get_int_value(configuracion, "CANTIDAD_ENTRADAS");
	entradasInstancia->tamanio = config_get_int_value(configuracion, "TAMANIO_ENTRADA");

	//enviamos cantidad de entradas y su respectivo tamanio a la instancia
	enviarInformacion(socketInstancia, entradasInstancia, &tamanioInformacionEntradas);

	//Logueamos el envio de informacion
	log_trace(logCoordinador, "Enviamos a la Instancia: Cant. de Entradas = %d; Tam. de Entrada = %d", entradasInstancia->cantidad, entradasInstancia->tamanio);

	// Verifico si tengo que agregar la instancia o si solo se reincorporó
	pthread_mutex_lock(&mutexListaInstancias);

	// Como se reincorporó, capaz ya tiene entradas usadas, por lo que recibo las libres siempre
	headerEntradasLibres = recibirHeader(socketInstancia);
	entradasLibres = headerEntradasLibres->id;
	free(headerEntradasLibres);

	instanciaVoid = list_find_with_param(listaInstancias, (void*)nombreInstancia, buscarNombreDeLaInstancia);

	if (instanciaVoid == NULL) {
		instanciaConectada = (Instancia*) malloc(sizeof(Instancia));

		instanciaConectada->nombre = malloc(strlen(nombreInstancia) + 1);
		strcpy(instanciaConectada->nombre, nombreInstancia);
		instanciaConectada->nombre[strlen(nombreInstancia)] = '\0';

		instanciaConectada->socket = socketInstancia;
		instanciaConectada->claves = list_create();
		instanciaConectada->entradasLibres = entradasLibres;

		list_add(listaInstancias, (void*) instanciaConectada);

		// Logueo que llegó una instancia nueva
		log_trace(logCoordinador, "Se ingresó la instancia nueva <%s> con %d entradas libres", nombreInstancia, instanciaConectada->entradasLibres);
	} else {

		// Guardo el nuevo socket
		instanciaConectada = (Instancia*)instanciaVoid;
		close(instanciaConectada->socket);
		instanciaConectada->socket = socketInstancia;

		// Guardo las entradas libres
		instanciaConectada->entradasLibres = entradasLibres;

		// Logueo que se reincorporó una instancia
		log_trace(logCoordinador, "Se reincorporó la instancia <%s> con %d entradas libres", nombreInstancia, instanciaConectada->entradasLibres);
	}

	pthread_mutex_unlock(&mutexListaInstancias);

	// Libero memoria
	free(entradasInstancia);
	free(nombreInstancia);
	config_destroy(configuracion);
}

void closeInstancia(void* instanciaVoid) {
	Instancia* instancia = (Instancia*)instanciaVoid;

	free(instancia->nombre);
	close(instancia->socket);
	free(instancia);
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
	log_trace(logCoordinador, "Recibimos el mensaje: Nombre = %s; Mensaje = %s", nombre, mensaje);

	// Retraso ficticio de la ejecucion
	int retardo = tiempoRetardoFicticio();
	log_trace(logCoordinador, "Hago un retardo de %d microsegundos", retardo);
	usleep(retardo);

	// Ejecuto
	mensajeSplitted = string_split(mensaje, " ");
	if (strcmp(mensajeSplitted[0], "GET") == 0) {

		// Se ejecuta un GET
		getClave(mensajeSplitted[1], socketPlanificador, socketEsi, nombre);
		log_trace(logCoordinador, "Se ejecuto un GET");

	} else if (strcmp(mensajeSplitted[0], "SET") == 0 || strcmp(mensajeSplitted[0], "STORE") == 0) {

		// EL SET y el STORE se manejan a través del ejecutarSentencia
		ejecutarSentencia(socketEsi, socketPlanificador, mensaje, nombre);
		log_trace(logCoordinador, "Se ejecuto un %s", mensajeSplitted[0]);

		if (strcmp(mensajeSplitted[0], "SET") == 0) {
			free(mensajeSplitted[2]);
		}
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

void manejarConexion(void* socketsNecesarios) {
	SocketHilos socketsConectados = *(SocketHilos*) socketsNecesarios;
	ContentHeader * header;

	header = recibirHeader(socketsConectados.socketComponente);

	switch (header->id) {
	case INSTANCIA:
		log_trace(logCoordinador, "Se conectó una instancia");
		manejarInstancia(socketsConectados.socketComponente, header->largo);
		llegoUnaInstancia = 1;
		break;

	case ESI:
		log_trace(logCoordinador, "Se conectó un ESI");
		manejarEsi(socketsConectados.socketComponente, socketsConectados.socketPlanificador, header->largo);
		break;
	}

	free(header);
}

int correrEnHilo(SocketHilos socketsConectados) {
	pthread_t idHilo;
	SocketHilos* socketsNecesarios;
	socketsNecesarios = (SocketHilos*) malloc(sizeof(SocketHilos));
	*socketsNecesarios = socketsConectados;

	if (pthread_create(&idHilo, NULL, (void*) manejarConexion, (void*) socketsNecesarios)) {
		log_error(logCoordinador, "No se pudo crear el hilo");
		free(socketsNecesarios);
		return 0;
	}

	log_trace(logCoordinador, "Hilo asignado");
	pthread_join(idHilo, NULL);

	return 1;
}

char* obtenerValorClaveInstancia(int socketInstancia, char* nombreClave){
	ContentHeader* header;
	char* valorClave;
	// Envio la clave de la cual quiero su contenido
	enviarHeader(socketInstancia, nombreClave, COMANDO_STATUS);
	enviarMensaje(socketInstancia, nombreClave);
	// Recibo contenido de la instancia
	header = recibirHeader(socketInstancia);

	switch(header->id){
		case COMANDO_STATUS_VALOR_CLAVE_OK: // Si hay un valor en esa clave
			valorClave = malloc(header->largo + 1);
			recibirMensaje(socketInstancia, header->largo, &valorClave);
			valorClave[header->largo] = '\0';
			break;
		case COMANDO_STATUS_VALOR_CLAVE_NULL: // Si no hay un valor en esa clave
			valorClave = NULL;
			break;
	}

	free(header);
	return valorClave;
}

void manejarComandoStatus(int socketPlanificador, int largoMensaje){
	char* nombreClave = malloc(largoMensaje +1);
	char* valorClave;
	char* nombreInstancia;
	Instancia* instanciaNueva = malloc(sizeof(Instancia));
	char* nombreInstanciaNueva;
	Instancia* instancia;
	void* instanciaVoid;
	Clave* clave;
	void* claveVoid;

	// Recibo nombre de la clave
	recibirMensaje(socketPlanificador, largoMensaje, &nombreClave);
	//TODO Busco clave y paso sus datos (valor, instancia en la que se guardaria)

	// Busco la clave en la lista de instancias
	pthread_mutex_lock(&mutexListaInstancias);
	instanciaVoid = list_find_with_param(listaInstancias, (void*) nombreClave, buscarInstanciaConClave);
	// Si encontré una instancia, busco la clave
	claveVoid = (instanciaVoid != NULL ? list_find_with_param(((Instancia*) instanciaVoid)->claves, (void*) nombreClave, buscarClaveEnListaDeClaves) : NULL);
	pthread_mutex_unlock(&mutexListaInstancias);

	if(instanciaVoid != NULL){
		instancia = (Instancia*) instanciaVoid;
		clave = (Clave*) claveVoid;

		// Obtengo nombre de la instancia que contiene la clave
		nombreInstancia = malloc(strlen(instancia->nombre) + 1);
		strcpy(nombreInstancia, instancia->nombre);
		nombreInstancia[strlen(instancia->nombre)] = '\0';

		// Obtengo contenido de la clave, necesito recibirlo de la instancia
		valorClave = obtenerValorClaveInstancia(instancia->socket, clave->nombre);

	} else {
		// En caso de que la clave no se encuentre en una instancia, por ende no tiene valor
		valorClave = NULL;
		nombreInstancia = NULL;
	}

	// Vuelvo a calcular en que instancia ubicaria ahora a la clave
	instanciaNueva = seleccionarInstanciaAlgoritmoDistribucion(clave);
	// Obtengo nombre de la instancia que contendria a la clave
	nombreInstanciaNueva = malloc(strlen(instanciaNueva->nombre) + 1);
	strcpy(nombreInstanciaNueva, instanciaNueva->nombre);
	nombreInstanciaNueva[strlen(instanciaNueva->nombre)] = '\0';

	//Le envio uno por uno la informacion obtenida al planificador
	// Envio valor de la clave
	enviarHeader(socketPlanificador, valorClave, COMANDO_STATUS);
	enviarMensaje(socketPlanificador, valorClave);
	// Envio instancia en la que se encuentra la clave
	enviarHeader(socketPlanificador, nombreInstancia, COMANDO_STATUS);
	enviarMensaje(socketPlanificador, nombreInstancia);
	// Envio instancia en la que se guardaria actualmente la clave
	enviarHeader(socketPlanificador, nombreInstanciaNueva, COMANDO_STATUS);
	enviarMensaje(socketPlanificador, nombreInstanciaNueva);

	// Libero memoria
	if(valorClave != NULL){
		free(valorClave);
	}
	free(nombreInstancia);
	free(nombreInstanciaNueva);
	free(nombreClave);
	free(instanciaNueva);
}

void manejarComandoKill(int socketPlanificador, int largoMensaje){
	void* instanciaVoid;
	void* claveVoid;
	Clave* clave;
	char* nombreEsi = malloc(largoMensaje +1);
	// Recibo nombre del esi
	recibirMensaje(socketPlanificador, largoMensaje, &nombreEsi);
	// Busco instancias con claves del esi
	pthread_mutex_lock(&mutexListaInstancias);
	if ((instanciaVoid = list_find_with_param(listaInstancias, (void*) nombreEsi, buscarInstanciaConEsi)) != NULL){
		// Busco claves del esi en la instancia
		if ((claveVoid = list_find_with_param(((Instancia*) instanciaVoid)->claves, (void*) nombreEsi, buscarClaveDeEsi)) != NULL){
			clave = (Clave*) claveVoid;
			// Desbloqueo clave del esi
			clave->bloqueado = 0;
			free(clave->nombreEsi);
		}
	}
	pthread_mutex_unlock(&mutexListaInstancias);

	free(nombreEsi);
}

void manejarBloquearClaveManual(int socketPlanificador, int largoMensaje) {
	char** claves;
	char* todasLasClaves = malloc(largoMensaje +1);
	
	// Recibo las claves que tengo que bloquear
	recibirMensaje(socketPlanificador, largoMensaje, &todasLasClaves);
	
	// Logueo la recepción
	log_trace(logCoordinador, "Se recibieron las claves <%s> para bloquear", todasLasClaves);
	
	// Espero hasta que llegue una instancia
	while(llegoUnaInstancia == 0);

	if (guardarClavesBloqueadasAlIniciar) {
		// Recorro todas las claves y bloqueo una por una
		claves = string_split(todasLasClaves, ",");
		for (int i = 0; claves[i] != NULL; i++) {
			asignarClaveAInstancia(claves[i], "");
			free(claves[i]);
		}
		
		free(claves);
	}
}

void manejarDesbloquearClaveManual(int socketPlanificador, int largoMensaje){
	void* instanciaVoid;
	void* claveVoid;
	Clave* clave;
	char* claveDesbloquear = malloc(largoMensaje +1);
	
	// Recibo la clave que tengo que desbloquear
	recibirMensaje(socketPlanificador, largoMensaje, &claveDesbloquear);
	
	// Busco instancias con claves del esi
	pthread_mutex_lock(&mutexListaInstancias);
	if ((instanciaVoid = list_find_with_param(listaInstancias, (void*) claveDesbloquear, buscarInstanciaConClave)) != NULL){
		
		// Busco claves del esi en la instancia
		if ((claveVoid = list_find_with_param(((Instancia*) instanciaVoid)->claves, (void*) claveDesbloquear, buscarClaveEnListaDeClaves)) != NULL){
			// Desbloqueo clave del esi
			clave = (Clave*) claveVoid;
			clave->bloqueado = 0;
			free(clave->nombreEsi);
		}
	}
	pthread_mutex_unlock(&mutexListaInstancias);

	free(claveDesbloquear);
}

void consolaPlanificador(void* socketPlanificadorVoid){
	int socketPlanificador = *(int*) socketPlanificadorVoid;
	ContentHeader * header;

	while(true){
		header = recibirHeader(socketPlanificador);

		switch (header->id) {
			case COMANDO_KILL:
				log_trace(logCoordinador, "Se recibio kill de esi desde el planificador");
				manejarComandoKill(socketPlanificador, header->largo);
				break;

			case COMANDO_STATUS:
				log_trace(logCoordinador, "Se recibio status de clave desde el planificador");
				manejarComandoStatus(socketPlanificador, header->largo);
				break;

			case BLOQUEAR_CLAVE_MANUAL:
				manejarBloquearClaveManual(socketPlanificador, header->largo);
				break;

			case DESBLOQUEAR_CLAVE_MANUAL:
				manejarDesbloquearClaveManual(socketPlanificador, header->largo);
				break;
		}
		free(header);
	}
}

void cerrarCoordinador(int sig) {
	guardarClavesBloqueadasAlIniciar = 0;
	llegoUnaInstancia = 1;
	close(socketEscucha);
	exit(1);
}

int main() {
	puts("Iniciando Coordinador.");
	int socketComponente, socketConectadoPlanificador;
	SocketHilos socketsNecesarios;
	t_config* configuracion;
	int puerto;
	int maxConexiones;
	char* ipPlanificador;

	indexInstanciaEL = 0;

	// Claves bloqueadas por defecto
	guardarClavesBloqueadasAlIniciar = 1;
	llegoUnaInstancia = 0;

	signal(SIGTSTP, &cerrarCoordinador);

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

	// Creo hilo para esperar mensajes de consola del planificador
	pthread_t idHilo;
	if (pthread_create(&idHilo, NULL, (void*) consolaPlanificador, (void*) &socketConectadoPlanificador)) {
		log_error(logCoordinador, "No se pudo crear el hilo para recibir mensajes del Planificador");
		//Libero memoria
		close(socketEscucha);
		close(socketConectadoPlanificador);
		free(ipPlanificador);
		config_destroy(configuracion);
		cerrarInstancias();
		return 0;
	}
	log_trace(logCoordinador, "Hilo asignado para recibir mensajes del Planificador");

	// Instancio la lista de instancias
	listaInstancias = list_create();

	// Espero conexiones de ESIs e instancias
	while ((socketComponente = servidorConectarComponente(&socketEscucha, "", ""))) {
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

int sePuedeComunicarConLaInstancia(Instancia* instancia) {
	// Si se puede comunicar devuelve 1, si no -1
	return enviarHeader(instancia->socket, "", 0);
}

bool instanciasNoCaidas(void* instanciaVoid) {
	return sePuedeComunicarConLaInstancia((Instancia*) instanciaVoid) != -1;
}

Instancia* seleccionarInstanciaAlgoritmoDistribucion(Clave* clave){
	char* algoritmo_distribucion;
	t_config* configuracion;
	Instancia* instancia;
	t_list* listaInstanciasNoCaidas;

	// Leo del archivo de configuracion
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	algoritmo_distribucion = config_get_string_value(configuracion, "ALG_DISTR");

	pthread_mutex_lock(&mutexListaInstancias);
	// Filtro instancias que no se encuentren caidas
	listaInstanciasNoCaidas = list_filter(listaInstancias, instanciasNoCaidas);
	pthread_mutex_unlock(&mutexListaInstancias);

	if (list_is_empty(listaInstanciasNoCaidas)) {
		log_error(logCoordinador, "No se puede seleccionar algoritmo de distribucion, todas las instancias se encuentran caidas");
		return NULL;
	}

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
	}

	// Libero memoria
	config_destroy(configuracion);
	list_destroy(listaInstanciasNoCaidas);

	return instancia;
}

int asignarClaveAInstancia(char* key, char* nombreEsi) {
	Instancia* instancia;
	// Creo la clave
	Clave* clave;
	clave = malloc(sizeof(Clave));
	clave->bloqueado = 1;

	clave->nombre = malloc(strlen(key) + 1);
	strcpy(clave->nombre, key);
	clave->nombre[strlen(key)] = '\0';

	clave->nombreEsi = malloc(strlen(nombreEsi) + 1);
	strcpy(clave->nombreEsi, nombreEsi);
	clave->nombreEsi[strlen(nombreEsi)] = '\0';

	instancia = seleccionarInstanciaAlgoritmoDistribucion(clave);

	if(instancia == NULL){
		// Libero memoria, ya loguie antes
		free(clave->nombre);
		free(clave->nombreEsi);
		free(clave);
		return 0;
	}

	list_add(instancia->claves, clave);

	return 1;
}

void getClave(char* key, int socketPlanificador, int socketEsi, char* nombreEsi) {
	Clave* clave;
	void* claveVoid;
	Instancia* instancia;
	void* instanciaVoid;
	int respuestaGET;

	// Busco la clave en la lista de claves
	pthread_mutex_lock(&mutexListaInstancias);
	instanciaVoid = list_find_with_param(listaInstancias, (void*) key, buscarInstanciaConClave);

	// Si encontré una instancia, busco su clave
	claveVoid = (instanciaVoid != NULL ? list_find_with_param(((Instancia*) instanciaVoid)->claves, (void*) key, buscarClaveEnListaDeClaves) : NULL);

	pthread_mutex_unlock(&mutexListaInstancias);
	clave = (Clave*) claveVoid;

	if (instanciaVoid != NULL) {
		instancia = (Instancia*) instanciaVoid;
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

				// Asigno que esi la bloqueo
				free(clave->nombreEsi);
				clave->nombreEsi = malloc(strlen(nombreEsi) + 1);
				strcpy(clave->nombreEsi, nombreEsi);
				clave->nombreEsi[strlen(nombreEsi)] = '\0';

				// Aviso
				avisarA(socketEsi, "", respuestaGET);
				avisarA(socketPlanificador, "", respuestaGET);
			}
			pthread_mutex_unlock(&mutexListaInstancias);
		} else {
			// Instancia está caída
			respuestaGET = COORDINADOR_INSTANCIA_CAIDA;
			log_error(logCoordinador, "La clave que intenta acceder existe en el sistema pero se encuentra en una instancia que esta desconectada");

			//Le aviso al planificador y esi del error
			avisarA(socketEsi, "", respuestaGET);
			avisarA(socketPlanificador, "", respuestaGET);
		}
	} else {
		// Le asigno la nueva clave a la instancia
		if (asignarClaveAInstancia(key, nombreEsi)) {
			respuestaGET = COORDINADOR_ESI_CREADO;
			log_trace(logCoordinador, "Se asigna la nueva clave a la instancia");
		} else {
			respuestaGET = NO_HAY_INSTANCIAS;
			log_trace(logCoordinador, "No hay instancias disponibles");
		}

		avisarA(socketEsi, "", respuestaGET);
		avisarA(socketPlanificador, key, respuestaGET);
	}
}

// Cosas para el SET y para el STORE
void avisarA(int socketAvisar, char* mensaje, int error) {
	enviarHeader(socketAvisar, mensaje, error);
	if (strlen(mensaje) >= 1) {
		enviarMensaje(socketAvisar, mensaje);
	}
}

void ejecutarSentencia(int socketEsi, int socketPlanificador, char* mensaje, char* nombreESI) {
	void* instanciaVoid;
	Instancia* instancia;
	ContentHeader *headerEstado, *headerEntradas, *headerCompactacion;

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

	if(sePuedeComunicarConLaInstancia(instancia) == -1) {
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

	if (!clave->bloqueado || strcmp(clave->nombreEsi, nombreESI) != 0) {
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
		// Espero saber si necesita compactar
		headerCompactacion = recibirHeader(instancia->socket);
		if (headerCompactacion->id == COMPACTAR) compactar();
		free(headerCompactacion);
	}

	// Espero la respuesta de la instancia
	headerEstado = recibirHeader(instancia->socket);

	switch (headerEstado->id) {
		case INSTANCIA_SENTENCIA_OK_SET:

			// Espero cantidad de entradas libres de la instancia
			headerEntradas = recibirHeader(instancia->socket);

			pthread_mutex_lock(&mutexListaInstancias);
			instancia->entradasLibres = headerEntradas->id;
			pthread_mutex_unlock(&mutexListaInstancias);

			free(headerEntradas);

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

			// Le aviso al planificador
			avisarA(socketPlanificador, "", COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);

			//Tambien le aviso al esi para que no se quede esperando
			avisarA(socketEsi, "", COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);
			break;

		case INSTANCIA_ERROR:
			log_error(logCoordinador, "Error no contemplado");

			// Le aviso al planificador
			avisarA(socketPlanificador, "", INSTANCIA_ERROR);

			// Tambien le aviso al esi para que no se quede esperando
			avisarA(socketEsi, "", INSTANCIA_ERROR);
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

// Compactacion
void compactar() {
	pthread_mutex_lock(&mutexListaInstancias);
	list_iterate(listaInstancias, compactarInstancia);
	pthread_mutex_unlock(&mutexListaInstancias);
}

void compactarInstancia(void* instancia) {
	enviarHeader(((Instancia*)instancia)->socket, "", COMPACTAR);
}
