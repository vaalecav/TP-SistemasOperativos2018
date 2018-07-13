/*
 ============================================================================
 Name        : planificador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Planificador
 ============================================================================
 */

#include "planificador.h"

int main() {
	// Declaraciones Iniciales //
	puts("Iniciando Planificador.");
	pthread_t hiloConexiones;
	pthread_t hiloAlgoritmos;
	char* ipCoordinador;
	int puertoCoordinador;

	// Leo el Archivo de Configuracion //
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	ipCoordinador = config_get_string_value(configuracion, "IP_COORDINADOR");
	puertoCoordinador = config_get_int_value(configuracion,
			"PUERTO_COORDINADOR");

	// Me conecto con el Coordinador //
	socketCoordinador = clienteConectarComponente("planificador", "coordinador",
			puertoCoordinador, ipCoordinador);

	// Inicializo las listas enlazadas //
	colaReady = list_create();
	colaBloqueados = list_create();
	colaTerminados = list_create();
	colaAbortados = list_create();
	listaClaves = list_create();

	// Inicio el hilo que maneja las Conexiones //
	if (pthread_create(&hiloConexiones, NULL, (void *) tratarConexiones,
	NULL)) {
		fprintf(stderr, "Error creando el thread.\n");
		return 1;
	}

	// Inicio el hilo que maneja la Ejecucion de ESIs //
	if (pthread_create(&hiloAlgoritmos, NULL, (void *) manejoAlgoritmos,
	NULL)) {
		fprintf(stderr, "Error creando el thread.\n");
		return 1;
	}

	// Inicio la Consola del Planificador //
	iniciarConsola();

	// Espero a que finalize el hilo que maneja las Conexiones //
	if (pthread_join(hiloConexiones, NULL)) {
		fprintf(stderr, "Error joineando thread\n");
		return 2;
	}

	// Espero a que finalize el hilo que maneja la Ejecucion de ESIs //
	if (pthread_join(hiloAlgoritmos, NULL)) {
		fprintf(stderr, "Error joineando thread\n");
		return 2;
	}

	// Finalizo correctamente al Planificador //
	cerrarPlanificador();
	puts("El Planificador se ha finalizado correctamente.");

	return 0;
}

void manejoAlgoritmos() {
	DATA * esi;
	void * esiVoid;
	char * algoritmo = config_get_string_value(configuracion, "ALGORITMO");
	if (strcmp(algoritmo, "HRRN") == 0) {
		alphaHRRN = config_get_int_value(configuracion, "ALPHA");
	}
	ContentHeader * header;
	int paraSwitchear;
	char* clave;
	int flagFin;

	// CLAVES BLOQUEADAS DESDE EL PRINCIPIO //
	char* clavesBloqueadas = config_get_string_value(configuracion,
			"CLAVES_BLOQUEADAS");
	enviarHeader(socketCoordinador, clavesBloqueadas, BLOQUEAR_CLAVE_MANUAL);
	enviarMensaje(socketCoordinador, clavesBloqueadas);

	while (done == 0) {
		flagFin = 0;
		if (ejecutar == 1) {

			// Si es SJF, antes de ejecutar tiene que ordenar la cola de ready
			if (strcmp(algoritmo, "SJF-CD") == 0
					|| strcmp(algoritmo, "SJF-SD") == 0) {
				realizarEstimaciones();
				list_sort(colaReady, menorCantidadDeLineas);
			}

			if (strcmp(algoritmo, "HRRN") == 0) {
				realizarRatios();
				list_sort(colaReady, mayorRatio);
			}

			if ((esiVoid = list_remove(colaReady, 0)) == NULL) {
				// No hay nadie para ejecutar //
			} else {
				while (flagFin != 1) {
					// Le ordeno al ESI que ejecute //
					esi = (DATA*) esiVoid;
					enviarHeader(esi->socket, "", PLANIFICADOR);
					CLAVE * claveAux;

					// Le quito la espera y la rafaga anterior //
					esi->espera = 0;
					esi->rafaga = 0;

					// Espero la respuesta //
					header = recibirHeader(socketCoordinador);

					// Veo que tengo que hacer //
					paraSwitchear = chequearRespuesta(header->id);

					switch (paraSwitchear) {
					case 0: // ERROR //
						// Lo agrego a la cola de Abortados //
						list_add(colaAbortados, (void*) esi);
						flagFin = 1;
						break;

					case 1: // BLOQUEO //
						// Lo agrego a la cola de Bloqueados //

						clave = malloc(header->largo + 1);
						recibirMensaje(socketCoordinador, header->largo,
								&clave);
						clave[header->largo] = '\0';

						claveAux = list_find_with_param(listaClaves,
								(void*) clave, chequearClave);
						list_add(claveAux->listaEsi, (void*) esi->id);

						list_add(colaBloqueados, (void*) esi);
						free(clave);
						flagFin = 1;
						break;

					case 2: // EXITO //
						esi->lineas--;
						esi->rafaga++;
						switch (esi->lineas) {
						case 0: // TERMINO EL ESI //
							// Lo agrego a la cola de Terminados //
							list_add(colaTerminados, (void*) esi);
							flagFin = 1;
							break;
						}
						break;

					case 3: // STORE EXITO //
						clave = malloc(header->largo + 1);
						recibirMensaje(socketCoordinador, header->largo,
								&clave);
						clave[header->largo] = '\0';

						// Es un store, por lo que desbloqueo la clave
						desbloquearClave(clave);

						esi->lineas--;
						esi->rafaga++;
						switch (esi->lineas) {
						case 0: // TERMINO EL ESI //
							// Lo agrego a la cola de Terminados //
							list_add(colaTerminados, (void*) esi);
							flagFin = 1;
							break;
						}
						free(clave);
						break;

					case 4:
						clave = malloc(header->largo + 1);
						recibirMensaje(socketCoordinador, header->largo,
								&clave);
						clave[header->largo] = '\0';
						claveAux = (CLAVE*) malloc(sizeof(CLAVE));
						claveAux->clave = malloc(header->largo + 1);
						strcpy(claveAux->clave, clave);
						claveAux->clave[header->largo] = '\0';
						claveAux->listaEsi = list_create();
						list_add(listaClaves, (void*) claveAux);
						esi->lineas--;
						esi->rafaga++;
						switch (esi->lineas) {
						case 0: // TERMINO EL ESI //
							// Lo agrego a la cola de Terminados //
							list_add(colaTerminados, (void*) esi);
							flagFin = 1;
							break;
						}
						free(clave);
						break;
					}

					free(header);

					aumentarEsperaDeEsi();

					if (strcmp(algoritmo, "SJF-CD") == 0) {
						if (flagFin == 0) {
							esi->necesitaCalcular = 1;
							list_add(colaReady, (void*) esi);
							flagFin = 1;
						}
					}
				}
			}
		}
	}
}

void aumentarEsperaDeEsi() {
	colaReady = list_map(colaReady, aumentarEspera);
}

void realizarRatios() {
	realizarEstimaciones();
	colaReady = list_map(colaReady, calcularRatio);
}

void realizarEstimaciones() {
	colaReady = list_map(colaReady, calcularEstimacion);
}

void* calcularRatio(void* esiCalcularRatio) {
	DATA* esiCalcular = (DATA*) esiCalcularRatio;
	esiCalcular->ratio = (esiCalcular->espera + esiCalcular->estimacion)
			/ esiCalcular->estimacion;
	return (void*) esiCalcular;
}

void* calcularEstimacion(void* esiCalcularVoid) {
	DATA* esiCalcular = (DATA*) esiCalcularVoid;
	if (esiCalcular->necesitaCalcular == 1) {
		esiCalcular->estimacion = (((float) alphaHRRN / 100)
				* esiCalcular->rafaga)
				+ ((1 - ((float) alphaHRRN / 100)) * esiCalcular->estimacion);
		esiCalcular->necesitaCalcular = 0;
		return (void*) esiCalcular;
	} else
		return (void*) esiCalcular;
}

void* aumentarEspera(void* esiAumentarVoid) {
	DATA* esiAumentar = (DATA*) esiAumentarVoid;
	esiAumentar->espera++;
	return (void*) esiAumentar;
}

int desbloquearClave(char* clave) {
	CLAVE * claveParaDesbloquear;
	void * claveParaDesbloquearVoid;
	void * esiEjecutar;
	DATA * esiBloqueada;

	// Busco la clave en la lista
	if ((claveParaDesbloquearVoid = list_find_with_param(listaClaves,
			(void*) clave, chequearClave)) != NULL) {

		// Agarro el primer ESI que exista para ejecutar
		claveParaDesbloquear = (CLAVE*) claveParaDesbloquearVoid;
		esiEjecutar = list_remove(claveParaDesbloquear->listaEsi, 0);

		// Si hay un esi bloqueado por la clave, lo saco de la lista de bloqueados y lo paso a ready
		if (esiEjecutar != NULL) {
			esiBloqueada = list_remove_by_condition_with_param(colaBloqueados,
					esiEjecutar, buscarEnBloqueados);
			esiBloqueada->necesitaCalcular = 1;
			list_add(colaReady, (void*) esiBloqueada);
		} else {
			// Si no hay esis, libero la clave.
			list_remove_by_condition_with_param(listaClaves, (void*) clave,
					chequearClave);
		}

		return 1;
	} else {
		printf("La clave <%s> no existe.\n", clave);
		return 0;
	}
}

int bloquearClaveESI(char* parametros, int nada) {
	CLAVE * claveParaBloquear;
	void * claveParaBloquearVoid;
	void * esiBloquearVoid;
	DATA * esiBloquear;
	CLAVE * claveAux;

	// SEPARO PARAMETROS //
	char* search = " ";
	char* clave = strtok(parametros, search);
	int esi = atoi(strtok(NULL, search));

	// Me fijo si existe el ESI en ready //
	if ((esiBloquearVoid = list_find_with_param(colaReady, (void*) esi,
			buscarEnBloqueados)) != NULL) {

		// Existe el ESI en ready, procedo //
		esiBloquearVoid = list_remove_by_condition_with_param(colaReady,
				(void*) esi, buscarEnBloqueados);

		esiBloquear = (DATA *) esiBloquearVoid;

		// Busco la clave, si no existe la creo //
		if ((claveParaBloquearVoid = list_find_with_param(listaClaves,
				(void*) clave, chequearClave)) != NULL) {
			claveParaBloquear = (CLAVE *) claveParaBloquearVoid;
			// Existe la clave, procedo //
			list_add(claveParaBloquear->listaEsi, (void*) esiBloquear->id);
			list_add(colaBloqueados, (void*) esiBloquear);
			printf("Se bloqueó la clave %s del ESI con ID %d\n", clave, esi);
			return 1;
		} else {
			// No existe la clave, procedo //
			claveParaBloquear = (CLAVE*) malloc(sizeof(CLAVE));
			claveParaBloquear->clave = malloc(sizeof(clave));
			strcpy(claveParaBloquear->clave, clave);
			claveParaBloquear->listaEsi = list_create();
			list_add(listaClaves, (void*) claveParaBloquear);

			claveAux = list_find_with_param(listaClaves, (void*) clave,
					chequearClave);
			list_add(claveAux->listaEsi, (void*) esi);

			list_add(colaBloqueados, (void*) esiBloquear);
			printf("Se creo la clave: %s\n", clave);
			printf("Se bloqueó la clave %s del ESI con ID %d\n", clave, esi);
			return 1;
		}
	} else {
		// El ESI no esta en ready, procedo //
		puts("El ESI ingresado no existe. No se realizaron acciones.");
		return 0;
	}
}

bool mayorRatio(void* esi1Void, void* esi2Void) {
	DATA* esi1 = (DATA*) esi1Void;
	DATA* esi2 = (DATA*) esi2Void;

	return esi1->ratio > esi2->ratio;
}

bool menorCantidadDeLineas(void* esi1Void, void* esi2Void) {
	DATA* esi1 = (DATA*) esi1Void;
	DATA* esi2 = (DATA*) esi2Void;

	return esi1->estimacion < esi2->estimacion;
}

int buscarEnBloqueados(void* esiVoid, void* idVoid) {
	DATA * esi = (DATA*) esiVoid;
	int id = (int*) idVoid;
// 													ANTES COMO: int id = (int*) idVoid;
	return esi->id == id;
}

int buscarPorSocket(void* esiVoid, void* idVoid) {
	DATA * esi = (DATA*) esiVoid;
	int id = (int*) idVoid;
// 													ANTES COMO: int id = (int*) idVoid;
	return esi->socket == id;
}

int chequearClave(void* claveVoid, void* nombreVoid) {
	CLAVE * clave = (CLAVE*) claveVoid;
	char * nombre = (char*) nombreVoid;
	return strcmp(clave->clave, nombre) == 0;
}

int chequearRespuesta(int id) {
	int a;
	switch (id) {
	case COORDINADOR_ESI_ERROR_TAMANIO_CLAVE:
		a = 0;
		break;
	case COORDINADOR_ESI_ERROR_CLAVE_NO_IDENTIFICADA:
		a = 0;
		break;
	case COORDINADOR_ESI_ERROR_CLAVE_NO_TOMADA:
		a = 0;
		break;
	case COORDINADOR_INSTANCIA_CAIDA:
		a = 0;
		break;
	case NO_HAY_INSTANCIAS:
		a = 0;
		break;
	case INSTANCIA_ERROR:
		a = 0;
		break;
	case COORDINADOR_ESI_BLOQUEADO:
		a = 1;
		break;
	case COORDINADOR_ESI_BLOQUEAR:
		a = 2;
		break;
	case COORDINADOR_ESI_CREADO:
		a = 4;
		break;
	case INSTANCIA_SENTENCIA_OK_SET:
		a = 2;
		break;
	case INSTANCIA_SENTENCIA_OK_STORE:
		a = 3;
		break;
	}
	return a;
}

void tratarConexiones() {
// Declaraciones Iniciales //
	char* ip;
	int puerto;
	int maxConex;
	int socketServer;
	int estimacionOriginal;

// Leo el Archivo de Configuracion //
	puerto = config_get_int_value(configuracion, "PUERTO");
	ip = config_get_string_value(configuracion, "IP");
	maxConex = config_get_int_value(configuracion, "MAX_CONEX");
	estimacionOriginal = config_get_int_value(configuracion, "ESTIMACION");

// Nuevas Declaraciones //
	fd_set descriptoresLectura;
	int socketCliente[maxConex];
	int numeroClientes = 0;
	int i;
	struct timeval timeout;
	int nextIdEsi = 1;
	ContentHeader *header;

// Levanto el Servidor //
	socketServer = socketServidor(puerto, ip, maxConex);

// Bucle del select() //
	while (done == 0) {
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* Se inicializa descriptoresLectura */
		FD_ZERO(&descriptoresLectura);

		/* Se añade para select() el socket servidor */
		FD_SET(socketServer, &descriptoresLectura);

		/* Se añaden para select() los sockets con los clientes ya conectados */
		for (i = 0; i < numeroClientes; i++)
			FD_SET(socketCliente[i], &descriptoresLectura);

		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 que decir: un nuevo cliente o un cliente ya conectado que envía un
		 mensaje */

		select(100, &descriptoresLectura, NULL, NULL, &timeout);

		// Se comprueba si algún cliente ya conectado ha enviado algo
		for (i = 0; i < numeroClientes; i++) {
			if (FD_ISSET(socketCliente[i], &descriptoresLectura)) {
				// Se indica que el cliente ha cerrado la conexión
				moveToAbortados(socketCliente[i]);
				close(socketCliente[i]);
				remove_element(socketCliente, i, numeroClientes);
				numeroClientes--;
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 admite */

		if (FD_ISSET(socketServer, &descriptoresLectura)) {
			/* Acepta la conexión con el cliente, guardándola en el array */
			socketCliente[numeroClientes] = servidorConectarComponente(
					&socketServer, "planificador", "esi");
			numeroClientes++;

			/* Si se ha superado el maximo de clientes, se cierra la conexión,
			 se deja como estaba y se vuelve. */
			if (numeroClientes > maxConex) {
				enviarHeader(socketCliente[numeroClientes - 1], "", 0);
				close(socketCliente[numeroClientes - 1]);
				numeroClientes--;
			} else {
				/* Envía su número de id al cliente */
				enviarHeader(socketCliente[numeroClientes - 1], "", nextIdEsi);

				// Recibo la cantidad de lineas del ESI //
				header = recibirHeader(socketCliente[numeroClientes - 1]);
				int cantLineas = header->id;
				free(header);

				/* Agrego al ESI a la cola de Ready */
				DATA *nuevoEsi = (DATA*) malloc(sizeof(DATA));
				nuevoEsi->id = nextIdEsi;
				nuevoEsi->lineas = cantLineas;
				nuevoEsi->socket = socketCliente[numeroClientes - 1];
				nuevoEsi->espera = 0;
				nuevoEsi->estimacion = estimacionOriginal;
				nuevoEsi->rafaga = 0;
				nuevoEsi->necesitaCalcular = 0;
				nuevoEsi->ratio = 0;
				list_add(colaReady, (void*) nuevoEsi);

				/* Aumento el ID para el proximo ESI */
				nextIdEsi++;
			}
		}
	}

// Finalizo cualquier conexion restante //
	while (numeroClientes > 0) {
		printf("El cliente %d fue finalizado por comando (quit).\n",
				socketCliente[numeroClientes - 1]);
		close(socketCliente[numeroClientes - 1]);
		numeroClientes--;
	}

	close(socketServer);
}

void borrarDeColas(int socket) {
	list_remove_by_condition_with_param(colaReady, (void*) (&socket),
			compararSocket);
}

int compararSocket(void* esiVoid, void* indexVoid) {
	DATA* esi = (DATA*) esiVoid;
	int index = *(int*) indexVoid;

	return esi->socket == index;
}

void freeClave(void* clave) {
	CLAVE* claveLiberar = (CLAVE*) clave;
	free(claveLiberar->clave);
	list_destroy_and_destroy_elements(claveLiberar->listaEsi, free);
	free(claveLiberar);

}

void cerrarPlanificador() {
	config_destroy(configuracion);
	list_destroy_and_destroy_elements(colaReady, free);
	list_destroy_and_destroy_elements(colaBloqueados, free);
	list_destroy_and_destroy_elements(colaTerminados, free);
	list_destroy_and_destroy_elements(colaAbortados, free);
	list_destroy_and_destroy_elements(listaClaves, freeClave);
}

int dameMaximo(int *tabla, int n) {
	int i, max = 0;
	for (i = 0; i < n; i++) {
		if (tabla[i] > max)
			max = tabla[i];
	}
	return max;
}

void remove_element(int *array, int index, int array_length) {
	int i;
	for (i = index; i < array_length - 1; i++)
		array[i] = array[i + 1];
}

void imprimirEnPantalla(void* esiVoid) {
	DATA* esi = (DATA*) esiVoid;
	printf(
			"ID: %d, SOCKET: %d, LARGO: %d, TIEMPO EN READY: %d, ESTIMACION: %d, RAFAGA ANTERIOR: %d\n",
			esi->id, esi->socket, esi->lineas, esi->espera, esi->estimacion,
			esi->rafaga);
}

void imprimirEnPantallaClaves(void* claveVoid) {
	CLAVE* clave = (CLAVE*) claveVoid;
	printf("CLAVE: %s\n", clave->clave);
	list_iterate(clave->listaEsi, imprimirEnPantallaClavesAux);
}

void imprimirEnPantallaClavesAux(void* idVoid) {
	int id = (int*) idVoid;
	printf("ESI ID: %d\n", id);
}

void moveToAbortados(int socketId) {
	void* esiVoid;
	DATA* esiAMatar;
	int largoId;
	char* nombre;
	if ((esiVoid = list_find_with_param(colaReady, (void*) socketId,
			buscarPorSocket)) != NULL) {
		esiVoid = list_remove_by_condition_with_param(colaReady,
				(void*) socketId, buscarPorSocket);
		esiAMatar = (DATA*) esiVoid;
		list_add(colaAbortados, esiVoid);
	} else {
		if ((esiVoid = list_find_with_param(colaBloqueados, (void*) socketId,
				buscarPorSocket)) != NULL) {
			esiVoid = list_remove_by_condition_with_param(colaBloqueados,
					(void*) socketId, buscarPorSocket);
			esiAMatar = (DATA*) esiVoid;
			list_add(colaAbortados, esiVoid);
		} else {
			esiVoid = list_remove_by_condition_with_param(colaTerminados,
					(void*) socketId, buscarPorSocket);
			esiAMatar = (DATA*) esiVoid;
			esiAMatar->socket = 0;
			list_add(colaTerminados, (void*) esiAMatar);
		}
	}
	largoId = floor(log10(abs(esiAMatar->id))) + 1;
	nombre = malloc(4 + largoId + 1);
	sprintf(nombre, "ESI %d", esiAMatar->id);
	nombre[4 + largoId] = '\0';

	enviarHeader(socketCoordinador, nombre, COMANDO_KILL);
	enviarMensaje(socketCoordinador, nombre);

	free(nombre);
}

void matarEsi(int esi) {
	void* esiVoid;
	DATA* esiMatar;
	int largoId;
	char* nombre;
	if ((esiVoid = list_find_with_param(colaReady, (void*) esi,
			buscarEnBloqueados)) != NULL) {

		// Existe el ESI en ready, procedo //
		//esiVoid = list_remove_by_condition_with_param(colaReady, (void*) esi,
		//		buscarEnBloqueados);
		esiMatar = (DATA*) esiVoid;

		enviarHeader(esiMatar->socket, "", COMANDO_KILL);

		//close(esiMatar->socket);
		//list_add(colaAbortados, esiVoid);

		// Informo al coordinador
		largoId = floor(log10(abs(esi))) + 1;
		nombre = malloc(4 + largoId + 1);
		sprintf(nombre, "ESI %d", esi);
		nombre[4 + largoId] = '\0';

		enviarHeader(socketCoordinador, nombre, COMANDO_KILL);
		enviarMensaje(socketCoordinador, nombre);

		printf("Se mato al ESI %d.\n", esi);
		free(nombre);
	} else {
		if ((esiVoid = list_find_with_param(colaBloqueados, (void*) esi,
				buscarEnBloqueados)) != NULL) {

			// Existe el ESI en ready, procedo //
			//esiVoid = list_remove_by_condition_with_param(colaBloqueados,
			//		(void*) esi, buscarEnBloqueados);
			esiMatar = (DATA*) esiVoid;

			enviarHeader(esiMatar->socket, "", COMANDO_KILL);

			//close(esiMatar->socket);
			//list_add(colaAbortados, esiVoid);

			// Informo al coordinador
			largoId = floor(log10(abs(esi))) + 1;
			nombre = malloc(4 + largoId + 1);
			sprintf(nombre, "ESI %d", esi);
			nombre[4 + largoId] = '\0';

			enviarHeader(socketCoordinador, nombre, COMANDO_KILL);
			enviarMensaje(socketCoordinador, nombre);

			printf("Se mato al ESI %d.\n", esi);
			free(nombre);
		} else {
			printf("El ESI %d no se pudo matar.\n", esi);
		}
	}
}

void hacerStatus(char *clave) {
	void* claveStatusVoid;
	CLAVE* claveStatus;
	ContentHeader* header;
	char* valorClave;
	char* nombreInstancia;
	char* nombreInstanciaNueva;

	if ((claveStatusVoid = list_find_with_param(listaClaves, (void*) clave,
			chequearClave)) != NULL) {
		// Existe la clave, procedo:
		enviarHeader(socketCoordinador, clave, COMANDO_STATUS);
		enviarMensaje(socketCoordinador, clave);

		//Recibo valorClave del coordinador:
		header = recibirHeader(socketCoordinador);
		valorClave = malloc(header->largo + 1);
		recibirMensaje(socketCoordinador, header->largo, &valorClave);

		//Recibo nombreInstancia del coordinador:
		header = recibirHeader(socketCoordinador);
		nombreInstancia = malloc(header->largo + 1);
		recibirMensaje(socketCoordinador, header->largo, &nombreInstancia);

		//Recibo nombreInstanciaNueva del coordinador:
		header = recibirHeader(socketCoordinador);
		nombreInstanciaNueva = malloc(header->largo + 1);
		recibirMensaje(socketCoordinador, header->largo, &nombreInstanciaNueva);

		//Imprimo los valores:
		printf("Status de la clave: %s\n", clave);
		printf("Valor de la clave: %s\n", valorClave);
		printf("Instancia en la que se encuentra la clave: %s\n",
				nombreInstancia);
		printf("Instancia en la que se guardaria actualmente la clave: %s\n",
				nombreInstanciaNueva);

		//Imprimo los esi bloqueados por esa clave:
		claveStatus = (CLAVE *) claveStatusVoid;
		list_iterate(claveStatus->listaEsi, imprimirEnPantallaClavesAux);

		//Libero memoria
		free(valorClave);
		free(nombreInstancia);
		free(nombreInstanciaNueva);
	} else {
		puts("La clave solicitada no existe.");
	}
}

/*void deadlock() {

 //Conseguir lista de claves asociadas a esis
 //listaClavesEsis
 //Buscar esis bloqueados
 list_iterate(colaBloqueados, buscarEnClavesEsis);
 //Iterar con un for, por cada esi bloqueado, buscar en la listaClavesEsis si alguna de esas claves es la que necesita
 //Si la necesita, buscar si el esi a la que esta asignada esa clave, esta bloqueado, y devuelta lo mismo
 //Fijarse si llega al esi inicial y meterlo en esisEnDeadlock

 else {
 puts("Ningun ESI se encuentra en deadlock.")
 }

 } */

//=======================COMANDOS DE CONSOLA====================================
/*int cmdDeadlock() {
 deadlock()
 b return 0;
 }*/

int cmdDesbloquear(char* clave) {
	if (desbloquearClave(clave))
		printf("Se desbloqueó la clave %s\n", clave);
	return 0;
}

int cmdBloquear(char* clave, int esi) {
	bloquearClaveESI(clave, esi);
	return 0;
}

int cmdKill(char* esi) {
	matarEsi(atoi(esi));
	return 0;
}

int cmdStatus(char* clave) {
	hacerStatus(clave);
	return 0;
}

int cmdListaClaves() {
	list_iterate(listaClaves, imprimirEnPantallaClaves);
	return 0;
}

int cmdColaReady() {
	list_iterate(colaReady, imprimirEnPantalla);
	return 0;
}

int cmdColaAbortados() {
	list_iterate(colaAbortados, imprimirEnPantalla);
	return 0;
}

int cmdColaTerminados() {
	list_iterate(colaTerminados, imprimirEnPantalla);
	return 0;
}

int cmdColaBloqueados() {
	list_iterate(colaBloqueados, imprimirEnPantalla);
	return 0;
}

int cmdQuit() {
	done = 1;
	return 0;
}

int cmdPause() {
	ejecutar = 0;
	puts("Se ha pausado la ejecucion de ESIs.");
	return 0;
}

int cmdContinue() {
	ejecutar = 1;
	puts("Se ha reanudado la ejecucion de ESIs.");
	return 0;
}

int cmdHelp() {
	register int i;
	puts("Comando:\t\t\tDescripcion:");
	for (i = 0; comandos[i].cmd; i++) {
		if (strlen(comandos[i].cmd) < 7)
			printf("%s\t\t\t\t%s\n", comandos[i].cmd, comandos[i].info);
		else
			printf("%s\t\t\t%s\n", comandos[i].cmd, comandos[i].info);
	}
	return 0;
}

//=====================FUNCIONES DE CONSOLA=====================================

int existeComando(char* comando) {
	register int i;
	for (i = 0; comandos[i].cmd; i++) {
		if (strcmp(comando, comandos[i].cmd) == 0) {
			return i;
		}
	}
	return -1;
}

char *leerComando(char *linea) {
	char *comando;
	int i, j;
	int largocmd = 0;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
		largocmd++;
	}
	comando = malloc(largocmd + 1);
	for (j = 0; j < largocmd; j++) {
		comando[j] = linea[j];
	}
	comando[j++] = '\0';
	return comando;
}

void obtenerParametros(char **parametros, char *linea) {
	int i, j;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
	}
	(*parametros) = malloc(strlen(linea) - i);
	i++;
	for (j = 0; i < strlen(linea); j++) {
		if (linea[i] == '\0')
			break;
		(*parametros)[j] = linea[i];
		i++;
	}
	(*parametros)[j++] = '\0';
}

COMANDO *punteroComando(int posicion) {
	return (&comandos[posicion]);
}

int ejecutarSinParametros(COMANDO *comando) {
	return ((*(comando->funcion))());
}

int ejecutarConParametros(char *parametros, COMANDO *comando) {
	return ((*(comando->funcion))(parametros));
}

int verificarParametros(char *linea, int posicion) {
	int i;
	int espacios = 0;
	char *parametros;
	COMANDO *comando;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			espacios++;
	}
	if (comandos[posicion].parametros == espacios) {
		if (espacios == 0) {
			comando = punteroComando(posicion);
			ejecutarSinParametros(comando);
		} else {
			obtenerParametros(&parametros, linea);
			comando = punteroComando(posicion);
			ejecutarConParametros(parametros, comando);
			free(parametros);
		}
	} else {
		printf("%s: La cantidad de parametros ingresados es incorrecta.\n",
				comandos[posicion].cmd);
	}
	return 0;
}

void ejecutarComando(char *linea) {
	char *comando = leerComando(linea);
	int posicion = existeComando(comando);
	if (posicion == -1) {
		printf("%s: El comando ingresado no existe.\n", comando);
	} else {
		verificarParametros(linea, posicion);
	}
	free(comando);
}

char *recortarLinea(char *string) {
	register char *s, *t;
	for (s = string; whitespace(*s); s++)
		;
	if (*s == 0)
		return s;
	t = s + strlen(s) - 1;
	while (t > s && whitespace(*t))
		t--;
	*++t = '\0';
	return s;
}

void iniciarConsola() {
	char *linea, *aux;
	done = 0;
	while (done == 0) {
		linea = readline("user@planificador: ");
		if (!linea)
			break;
		aux = recortarLinea(linea);
		if (*aux)
			ejecutarComando(aux);
		free(linea);
	}
}
