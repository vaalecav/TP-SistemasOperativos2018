/*
 ============================================================================
 Name        : instancia.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Instancia
 ============================================================================
 */
#include "instancia.h"

void freeEntrada(void* ent) {
	Entrada* entrada = (Entrada*)ent;
	free(entrada->clave);
	free(entrada->valor);
	free(ent);
}

void loguearEntrada(void* entr) {
	Entrada entrada = *(Entrada*)entr;
	logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
	log_trace(logInstancia, "CLAVE %s\nEntrada: %d - %d\nValor: %s\n---------------------\n", entrada.clave, entrada.primerEntrada, entrada.cantidadEntradas, entrada.valor);
	log_destroy(logInstancia);
}

void cerrarInstancia(int sig) {
    terminar = 1;
    char* nombre;

	// Logueo el cierre de la instancia
		logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
		log_trace(logInstancia, "Se cerró la instancia, la tabla de entradas quedó:");
		log_destroy(logInstancia);
		list_iterate(estructuraAdministrativa.entradas, loguearEntrada);

		nombre = config_get_string_value(configuracion, "NOMBRE");
		//Se lo comunico al coordinador
		enviarHeader(socketCoordinador, nombre, INSTANCIA_COORDINADOR_DESCONECTADA);
		enviarMensaje(socketCoordinador, nombre);

	// Libero memoria
		free(info);
		config_destroy(configuracion);
		close(socketCoordinador);
		list_destroy_and_destroy_elements(estructuraAdministrativa.entradas, freeEntrada);

	exit(1);
}

int recibirInformacionEntradas(int socketEmisor, InformacionEntradas** info) {
	int recibido;

	recibido = recv(socketEmisor, *info, sizeof(InformacionEntradas), 0);
	if (recibido < 0) {
		return -1;
	} else if (recibido == 0) {
		close(socketEmisor);
		free(info);
		return 0;
	}

	return 1;
}

int buscarEspacioEnTabla(int entradasNecesarias) {
	int libre = -1;
	int cantidadSeguidos = 0;

	for (int i = 0; i < estructuraAdministrativa.cantidadEntradas; i++) {
		if (!estructuraAdministrativa.entradasUsadas[i]) {
			if (!cantidadSeguidos) {
				libre = i;
			}

			cantidadSeguidos++;

			if (cantidadSeguidos == entradasNecesarias) {
				break;
			}
		} else {
			libre = -1;
			cantidadSeguidos = 0;
		}
	}

	return libre;
}

int entradaEsIgualAClave(void* entrada, void* clave) {
	Entrada *ent = (Entrada*) entrada;
	return strcmp(ent->clave, (char*)clave)	== 0;
}

void mostrarEntrada(void* entr) {
	Entrada entrada = *(Entrada*)entr;
	printf("CLAVE %s\nEntrada: %d - %d\nValor: %s\n---------------------\n", entrada.clave, entrada.primerEntrada, entrada.cantidadEntradas, entrada.valor);
}

void setearValor(char* clave, char* valor, int entradasNecesarias) {
	void* entradaVoid;
	Entrada *entrada;
	int posicionParaSetear;

	posicionParaSetear = buscarEspacioEnTabla(entradasNecesarias);

	if (posicionParaSetear >= 0) {
		entrada = malloc(sizeof(Entrada));
		// Marco las entradas como ocupadas
		for (int i = posicionParaSetear; i < posicionParaSetear + entradasNecesarias; i++) {
			estructuraAdministrativa.entradasUsadas[i] = 1;
		}

		if ((entradaVoid = list_find_with_param(estructuraAdministrativa.entradas, (void*)clave, entradaEsIgualAClave)) != NULL) {
			entrada = (Entrada*)entradaVoid;
			free(entrada->valor);
			free(entrada->clave);
		}

		entrada->clave = malloc(strlen(clave) + 1);
		strcpy(entrada->clave, clave);
		entrada->clave[strlen(clave)] = '\0';
		entrada->valor = malloc(strlen(valor) + 1);
		strcpy(entrada->valor, valor);
		entrada->valor[strlen(valor)] = '\0';
		entrada->primerEntrada = posicionParaSetear;
		entrada->cantidadEntradas = entradasNecesarias;

		if (entradaVoid == NULL) {
			list_add(estructuraAdministrativa.entradas, (void*)entrada);
		}

		// Logueo que setteo el valor
		logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
		log_trace(logInstancia, "Setteo %s: %s, en la entrada %d con un largo de entradas %d", entrada->clave, entrada->valor, entrada->primerEntrada, entrada->cantidadEntradas);
		log_destroy(logInstancia);

		/*
		PARA DEBUGGEAR

		list_iterate(estructuraAdministrativa.entradas, mostrarEntrada);
		puts("");
		for(int i = 0; i < estructuraAdministrativa.cantidadEntradas; i++) {
			printf("Entrada %d: %s\n", i, (estructuraAdministrativa.entradasUsadas[i] ? "usada" : "libre"));
		}
		puts("");
		puts("=====================");
		puts("");
		*/
	}
}

int cantidadEntradasPosiblesContinuas() {
	int cantidadContinuasMax = 0;
	int cantidadContinuas = 0;
	for (int i = 0; i < estructuraAdministrativa.cantidadEntradas; i++) {
		if (estructuraAdministrativa.entradasUsadas[i]) {
			cantidadContinuasMax = max(cantidadContinuasMax, cantidadContinuas);
			cantidadContinuas = 0;
		} else {
			cantidadContinuas++;
		}
	}

	cantidadContinuasMax = max(cantidadContinuasMax, cantidadContinuas);
	return cantidadContinuasMax;
}

int tieneElIndexYEsAtomico(void* entradaVoid, void* indexVoid) {
	Entrada* entrada = (Entrada*)entradaVoid;
	int index = *(int*)indexVoid;

	return entrada->primerEntrada == index && entrada->cantidadEntradas == 1;
}

void ejecutarAlgoritmoDeRemplazo() {
	char* algoritmo;
	void* entradaVoid;
	Entrada* entrada;

	algoritmo = config_get_string_value(configuracion, "ALG_REMP");

	if (strcmp(algoritmo, "CIRC") == 0) {
		// Elimino el item de la lista que tenga el index y sea atómico
		entradaVoid = list_remove_by_condition_with_param(estructuraAdministrativa.entradas, (void*)(&indexCirc), tieneElIndexYEsAtomico);

		// Aumento el index
		indexCirc++;
	} else if (strcmp(algoritmo, "LRU") == 0) {
		// TODO Algoritmo de remplazo LRU
	} else if (strcmp(algoritmo, "BSU") == 0) {
		// TODO Algoritmo de remplazo BSU
	}

	if (entradaVoid != NULL) {
		entrada = (Entrada*)entradaVoid;

		// Pongo en la tabla de entradas que el espacio está libre
		estructuraAdministrativa.entradasUsadas[entrada->primerEntrada] = 0;

		// Libero el espacio que la entrada ocupaba
		freeEntrada(entradaVoid);
	}
}

void setearClave(char* clave, char* valor) {
	Entrada* entrada;
	void* entradaVoid;
	int entradasNecesarias;

	// Verifico si alcanzan las entradas
		entradasNecesarias = divCeil(strlen(valor), estructuraAdministrativa.tamanioEntrada);

		if (entradasNecesarias > estructuraAdministrativa.cantidadEntradas) {
			// Logueo el error
			logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
			log_error(logInstancia, "El valor no entra en la tabla de entradas");
			log_destroy(logInstancia);

			puts("La cantidad de entradas no son suficientes para el tamanio del valor pasado.");
			return;
		}

	// Verifico si ya está seteada la clave
		if ((entradaVoid = list_find_with_param(estructuraAdministrativa.entradas, (void*)clave, entradaEsIgualAClave)) != NULL) {
			entrada = (Entrada*)entradaVoid;

			// Logueo que la entrada ya existe
			logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
			log_trace(logInstancia, "La clave ya está seteada en la instancia");
			log_destroy(logInstancia);

			// Si ya está bloqueada, pongo todas sus entradas como posibles para ingresar
			for (int i = entrada->primerEntrada; i < entrada->cantidadEntradas; i++) {
				estructuraAdministrativa.entradasUsadas[i] = 0;
			}
		}

	// Verifico si hay espacio continuo disponible
		while (cantidadEntradasPosiblesContinuas() < entradasNecesarias) {

			// Logueo que ejecuto el algoritmo de remplazo
			logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
			log_trace(logInstancia, "Ejecuto algoritmo de remplazo");
			log_destroy(logInstancia);

			ejecutarAlgoritmoDeRemplazo();
		}

	// Setteo porque hay lugar
		setearValor(clave, valor, entradasNecesarias);
}

void storeClave(char* clave) {
	void* entradaVoid;
	Entrada* entrada;
	char* pto_montaje;
	char* nombreArchivo;
	FILE* archivo;

	pto_montaje = config_get_string_value(configuracion, "PUNTO_MONTAJE");
	nombreArchivo = malloc(strlen(pto_montaje) + strlen(clave) + 1);
	strcpy(nombreArchivo, pto_montaje);
	strcat(nombreArchivo, clave);
	nombreArchivo[strlen(pto_montaje) + strlen(clave)] = '\0';
	if ((archivo = fopen(nombreArchivo, "w"))) {
		if ((entradaVoid = list_find_with_param(estructuraAdministrativa.entradas, (void*)clave, entradaEsIgualAClave)) != NULL) {
			entrada = (Entrada*)entradaVoid;

			fprintf(archivo, "%s", entrada->valor);
		} else {
			logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_ERROR);
			log_error(logInstancia, "No existe la entrada setteada en la instancia");
			log_destroy(logInstancia);
		}

		fclose(archivo);
	} else {
		logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_ERROR);
		log_error(logInstancia, "No se puede acceder al archivo de la clave para hacer store del valor");
		log_destroy(logInstancia);
	}
}

void recibirSentencia() {
	char* mensaje;
	char** mensajeSplitted;
	ContentHeader *header;

	header = recibirHeader(socketCoordinador);

	if (header->id == COORDINADOR) {
		mensaje = malloc(header->largo + 1);
		recibirMensaje(socketCoordinador, header->largo, &mensaje);

		// Logueo que se recibio una sentencia
		logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
		log_trace(logInstancia, "Se recibió una sentencia: %s", mensaje);
		log_destroy(logInstancia);

		mensajeSplitted = string_split(mensaje, " ");

		if (strcmp(mensajeSplitted[0], "SET") == 0) {
			setearClave(mensajeSplitted[1], mensajeSplitted[2]);
		} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
			storeClave(mensajeSplitted[1]);
		} else {
			puts("Error en el mensaje enviado al coordinador por el ESI");
		}

		free(mensaje);
		free(mensajeSplitted[0]);
		free(mensajeSplitted[1]);
		free(mensajeSplitted[2]);
		free(mensajeSplitted);
	}

	free(header);
}

int main() {
	puts("Iniciando Instancia.");
	char* ipCoordinador;
	int puertoCoordinador;
	char* nombre;

	// Instancio las cosas necesarias para saber cuando se cierra la entrada
		terminar = 0;
		signal(SIGTSTP, &cerrarInstancia);

	// Archivo de configuracion
		configuracion = config_create(ARCHIVO_CONFIGURACION);

		puertoCoordinador = config_get_int_value(configuracion, "PUERTO_COORDINADOR");
		ipCoordinador = config_get_string_value(configuracion, "IP_COORDINADOR");
		nombre = config_get_string_value(configuracion, "NOMBRE");

	// Instancio las cosas necesarias para los algoritmos de remplazo
		indexCirc = 0;

	// Conexion con el coordinador
		socketCoordinador = clienteConectarComponente("instancia", "coordinador", puertoCoordinador, ipCoordinador);
		enviarHeader(socketCoordinador, nombre, INSTANCIA);
		int largo = strlen(nombre);
		enviarInformacion(socketCoordinador, nombre, &largo);

	// Instancio todas las estructuras administrativas
		// Espera hasta que recive la informacion de lo que será la tabla de entradas
		info = (InformacionEntradas*) malloc(sizeof(InformacionEntradas));

		recibirInformacionEntradas(socketCoordinador, &info);

		estructuraAdministrativa.cantidadEntradas = info->cantidad;
		estructuraAdministrativa.tamanioEntrada = info->tamanio;

		// A todas las entradas las inicio como vacías (0)
		estructuraAdministrativa.entradasUsadas = malloc(sizeof(int) * info->cantidad);
		for (int i = 0; i < info->cantidad; i++) {
			estructuraAdministrativa.entradasUsadas[i] = 0;
		}

		// Creo la lista de claves por primera vez
		estructuraAdministrativa.entradas = list_create();

		// Logueo la operacion de la estructura administrativa
		logInstancia = log_create(ARCHIVO_LOG, "Instancia", true, LOG_LEVEL_TRACE);
		log_trace(logInstancia, "Se conectó el coordinador e instanció la estructura administrativa: Cant. entradas=%d; Tam. entrada=%d;", estructuraAdministrativa.cantidadEntradas, estructuraAdministrativa.tamanioEntrada);
		log_destroy(logInstancia);

	// Espero las sentencias
		while(!terminar) {
			recibirSentencia();
		}


	puts("La Instancia se ha finalizado correctamente.");
	return 0;
}
