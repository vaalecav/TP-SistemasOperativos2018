#include "SET.h"
#include "../coordinador.h"

void avisarAEsi(int socketEsi, char* key, int error) {
	enviarHeader(socketEsi, key, error);
	enviarMensaje(socketEsi, key);
}

void setClave(int socketEsi, char* mensaje) {
	void* instanciaVoid;
	Instancia* instancia;

	void* claveVoid;
	Clave* clave;
	char** mensajeSplitted;
	mensajeSplitted = string_split(mensaje, " ");

	// Valido que el mensajeSplitted[2] no exceda el máximo
		if (strlen(mensajeSplitted[1]) > 40) {
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

		pthread_mutex_unlock(&mutexListaInstancias);

	// Valido que la clave esté bloqueada
		instancia = (Instancia*)instanciaVoid;
		clave = (Clave*)claveVoid;

		if (!clave->bloqueado) {
			avisarAEsi(socketEsi, mensajeSplitted[1], COORDINADOR_ESI_ERROR_CLAVE_DESBLOQUEADA);
			free(mensajeSplitted);
			return;
		}

	// Si llega hasta acá es porque es válido, le mando el mensaje a la instancia
		enviarHeader(instancia->socket, mensaje, COORDINADOR);
		enviarMensaje(instancia->socket, mensaje);
		free(mensajeSplitted);
}
