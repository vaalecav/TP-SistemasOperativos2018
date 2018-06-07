#include "SET_STORE.h"
#include "../coordinador.h"

void avisarA(int socketAvisar, char* mensaje, int error) {
	enviarHeader(socketAvisar, mensaje, error);

	if (strlen(mensaje) > 1) enviarMensaje(socketAvisar, mensaje);
}

void ejecutarSentencia(int socketEsi, int socketPlanificador, char* mensaje, char* nombreESI) {
	void* instanciaVoid;
	Instancia* instancia;
	ContentHeader * header;

	void* claveVoid;
	Clave* clave;
	char** mensajeSplitted;
	mensajeSplitted = string_split(mensaje, " ");

	// Valido que la clave no exceda el máximo
		if (esSET(mensajeSplitted[0]) && strlen(mensajeSplitted[1]) > 40) {
			// Logueo el error
			log_error(logCoordinador, "Error, la clave excede el tamaño máximo de 40 caracteres");

			// Le aviso al planificador por el error
			avisarA(socketPlanificador, nombreESI, COORDINADOR_ESI_ERROR_TAMANIO_CLAVE);

			// Libero memoria
			free(mensajeSplitted[0]);
			free(mensajeSplitted[1]);
			free(mensajeSplitted[2]);
			free(mensajeSplitted);
			return;
		}

	// Busco la clave en la lista de claves
		pthread_mutex_lock(&mutexListaInstancias);
		instanciaVoid = list_find_with_param(listaInstancias, (void*)mensajeSplitted[1], buscarInstanciaConClave);

		if (instanciaVoid == NULL) {
			pthread_mutex_unlock(&mutexListaInstancias);

			// Logueo el error
			log_error(logCoordinador, "Clave no identificada");

			// Le aviso al planificador
			avisarA(socketPlanificador, nombreESI, COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);

			// Libero memoria
			free(mensajeSplitted[0]);
			free(mensajeSplitted[1]);
			free(mensajeSplitted[2]);
			free(mensajeSplitted);
			return;
		}

		// Si encontré una instancia, busco su clave
		claveVoid = list_find_with_param(((Instancia*)instanciaVoid)->claves, (void*)mensajeSplitted[1], buscarClaveEnListaDeClaves);


	// Valido que la clave esté bloqueada
		instancia = (Instancia*)instanciaVoid;
		clave = (Clave*)claveVoid;

		if (!clave->bloqueado) {
			pthread_mutex_unlock(&mutexListaInstancias);

			// Logueo el error
			log_error(logCoordinador, "La clave que intenta acceder no se encuentra tomada");

			// Le aviso al planificador
			avisarA(socketPlanificador, nombreESI, COORDINADOR_ESI_ERROR_CLAVE_DESBLOQUEADA);

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

	// Espero la respuesta de la instancia
		header = recibirHeader(instancia->socket);

		switch(header->id) {
			case INSTANCIA_SENTENCIA_OK:
				// Si está OK, le aviso al ESI
				log_trace(logCoordinador, "La sentencia se ejecutó correctamente");
				avisarA(socketEsi, "", INSTANCIA_SENTENCIA_OK);
				break;

			case INSTANCIA_CLAVE_NO_IDENTIFICADA:
				// Cuando hay un error, le aviso al planificador
				log_error(logCoordinador, "Clave no identificada");
				avisarA(socketPlanificador, nombreESI, COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA);
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
		free(header);
}

int esSET(char* sentencia) {
	return strcmp(sentencia, "SET") == 0;
}

int esSTORE(char* sentencia) {
	return strcmp(sentencia, "STORE") == 0;
}
