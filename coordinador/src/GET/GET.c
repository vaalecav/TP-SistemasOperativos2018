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

	Clave* clave;
	clave = malloc(sizeof(Clave));
	clave->bloqueado = 1;
	clave->nombre = malloc(strlen(key) + 1);
	strcpy(clave->nombre, key);
	clave->nombre[strlen(key)] = '\0';

	configuracion = config_create("./configuraciones/configuracion.txt");
	algoritmo_distribucion = config_get_string_value(configuracion, "ALG_DISTR");

	pthread_mutex_lock(&mutexListaInstancias);
	if (strcmp(algoritmo_distribucion, "EL") == 0) {
		instancia = algoritmoDistribucionEL(listaInstancias);
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

	free(algoritmo_distribucion);
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
				if (clave->bloqueado) {
					respuestaGET = COORDINADOR_ESI_BLOQUEADO;
				} else {
					respuestaGET = COORDINADOR_ESI_BLOQUEAR;
					clave->bloqueado = 1;
				}
				pthread_mutex_unlock(&mutexListaInstancias);
			} else {
				enviarHeader(socketPlanificador, key, COORDINADOR_INSTANCIA_CAIDA);
				enviarMensaje(socketPlanificador, key);
				return;
			}
		} else {
			asignarClaveAInstancia(key);
			respuestaGET = COORDINADOR_ESI_CREADO;
		}

		enviarHeader(socketEsi, key, respuestaGET);
		enviarMensaje(socketEsi, key);

}
