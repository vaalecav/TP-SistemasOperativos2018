#include "GET.h"
#include "../coordinador.h"

int sePuedeComunicarConLaInstancia() {
	// TODO ver qué onda esto
	return 1;
}

void asignarClaveAInstancia(char* key) {
	char* algoritmo_distribucion;
	t_config* configuracion;
	Instancia* instancia;

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

	// Selecciono algoritmo de distribucion de instancias
	pthread_mutex_lock(&mutexListaInstancias);
	if (strcmp(algoritmo_distribucion, "EL") == 0) {
		log_trace(logCoordinador, "Utilizo algoritmo de distribucion de instancias EL");
		instancia = algoritmoDistribucionEL(listaInstancias);
	} else if (strcmp(algoritmo_distribucion, "LSU") == 0) {
		log_trace(logCoordinador, "Utilizo algoritmo de distribucion de instancias LSU");
		// TODO Implementar algoritmo Least Space Used
	} else if (strcmp(algoritmo_distribucion, "KE") == 0) {
		log_trace(logCoordinador, "Utilizo algoritmo de distribucion de instancias KE");
		// TODO Implementar Key Explicit
	} else {
		log_error(logCoordinador, "Algoritmo de distribucion invalido.");
		pthread_mutex_unlock(&mutexListaInstancias);
		return;
	}
	pthread_mutex_unlock(&mutexListaInstancias);

	// Libero memoria
	config_destroy(configuracion);
	list_add(instancia->claves, clave);
}

void getClave(char* key, int socketPlanificador, int socketEsi) {
	Clave* clave;
	void* claveVoid;
	Instancia* instancia;
	void* instanciaVoid;
	int respuestaGET;

	// Busco la clave en la lista de claves
		pthread_mutex_lock(&mutexListaInstancias);
		instanciaVoid = list_find_with_param(listaInstancias, (void*)key, buscarInstanciaConClave);

		// Si encontré una instancia, busco su clave
		claveVoid = (instanciaVoid != NULL ? list_find_with_param(((Instancia*)instanciaVoid)->claves, (void*)key, buscarClaveEnListaDeClaves) : NULL);

		pthread_mutex_unlock(&mutexListaInstancias);

	// Le aviso al planificador de su estado
		if (instanciaVoid != NULL) {
			instancia = (Instancia*)instanciaVoid;

			// Se tiene que verificar si la instancia no está caída
			if (sePuedeComunicarConLaInstancia(instancia)) {
				clave = (Clave*)claveVoid;
				pthread_mutex_lock(&mutexListaInstancias);

				// Se fija si la clave se encuentra bloqueada
				if (clave->bloqueado) {
					log_trace(logCoordinador, "La clave se encuentra bloqueada");
					respuestaGET = COORDINADOR_ESI_BLOQUEADO;
				} else {
					// No esta bloqueada, entonces la bloqueo
					log_trace(logCoordinador, "La clave no se encuentra bloqueada, se bloquea");
					respuestaGET = COORDINADOR_ESI_BLOQUEAR;
					clave->bloqueado = 1;
				}
				pthread_mutex_unlock(&mutexListaInstancias);
			} else {
				// Instancia está caída
				respuestaGET = COORDINADOR_INSTANCIA_CAIDA;
				log_error(logCoordinador, "La clave que intenta acceder existe en el sistema pero se encuentra en una instancia que esta desconectada");

				//Le aviso al planificador
				enviarHeader(socketPlanificador, key, COORDINADOR_INSTANCIA_CAIDA);
				enviarMensaje(socketPlanificador, key);

				//Tambien le aviso al esi para que no se quede esperando
				enviarHeader(socketEsi, key, respuestaGET);
				enviarMensaje(socketEsi, key);

				return;
			}
		} else {
			// Le asigno la nueva clave a la instancia
			asignarClaveAInstancia(key);
			respuestaGET = COORDINADOR_ESI_CREADO;

			log_trace(logCoordinador, "Se asigna la nueva clave a la instancia");
		}

		// Le aviso al ESI
		enviarHeader(socketEsi, key, respuestaGET);
		enviarMensaje(socketEsi, key);

}
