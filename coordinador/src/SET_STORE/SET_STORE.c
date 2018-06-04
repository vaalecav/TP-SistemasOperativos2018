#include "SET_STORE.h"
#include "../coordinador.h"

void avisarAEsi(int socketEsi, char* key, int error) {
	enviarHeader(socketEsi, key, error);
	enviarMensaje(socketEsi, key);
}

void ejecutarSentencia(int socketEsi, char* mensaje) {
	void* instanciaVoid;
	Instancia* instancia;

	void* claveVoid;
	Clave* clave;
	char** mensajeSplitted;
	mensajeSplitted = string_split(mensaje, " ");

	// Valido que la clave no exceda el máximo
		if (esSET(mensajeSplitted[0]) && strlen(mensajeSplitted[1]) > 40) {
			avisarAEsi(socketEsi, mensajeSplitted[1], COORDINADOR_ESI_ERROR_TAMANIO_CLAVE);
			free(mensajeSplitted);
			return;
		}

	// Busco la clave en la lista de claves
		pthread_mutex_lock(&mutexListaInstancias);
		instanciaVoid = list_find_with_param(listaInstancias, (void*)mensajeSplitted[1], buscarInstanciaConClave);

		if (instanciaVoid == NULL) {
			pthread_mutex_unlock(&mutexListaInstancias);
			avisarAEsi(socketEsi, mensajeSplitted[1], COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);
			free(mensajeSplitted);
			return;
		}

		// Si encontré una instancia, busco su clave
		claveVoid = list_find_with_param(((Instancia*)instanciaVoid)->claves, (void*)mensajeSplitted[1], buscarClaveEnListaDeClaves);


	// Valido que la clave esté bloqueada
		instancia = (Instancia*)instanciaVoid;
		clave = (Clave*)claveVoid;

		if (!clave->bloqueado) {
			avisarAEsi(socketEsi, mensajeSplitted[1], COORDINADOR_ESI_ERROR_CLAVE_DESBLOQUEADA);
			pthread_mutex_unlock(&mutexListaInstancias);
			free(mensajeSplitted);
			return;
		}

		if (esSTORE(mensajeSplitted[0])) {
			// Si es STORE, tengo que desbloquear la clave
			clave->bloqueado = 0;
		}

		pthread_mutex_unlock(&mutexListaInstancias);

	// Si llega hasta acá es porque es válido, le mando el mensaje a la instancia
		enviarHeader(instancia->socket, mensaje, COORDINADOR);
		enviarMensaje(instancia->socket, mensaje);
		free(mensajeSplitted);
}

int esSET(char* sentencia) {
	return strcmp(sentencia, "SET") == 0;
}

int esSTORE(char* sentencia) {
	return strcmp(sentencia, "STORE") == 0;
}
