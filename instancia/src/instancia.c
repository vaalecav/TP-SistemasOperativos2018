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
	t_config* configuracion;
	char* algoritmo;
	void* entradaVoid;
	Entrada* entrada;

	configuracion = config_create("./configuraciones/configuracion.txt");
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

	config_destroy(configuracion);
}

void setearClave(char* clave, char* valor) {
	Entrada* entrada;
	void* entradaVoid;
	int entradasNecesarias;

	// Verifico si alcanzan las entradas
		entradasNecesarias = divCeil(strlen(valor), estructuraAdministrativa.tamanioEntrada);

		if (entradasNecesarias > estructuraAdministrativa.cantidadEntradas) {
			puts("La cantidad de entradas no son suficientes para el tamanio del valor pasado.");
			return;
		}

	// Verifico si ya está seteada la clave
		if ((entradaVoid = list_find_with_param(estructuraAdministrativa.entradas, (void*)clave, entradaEsIgualAClave)) != NULL) {
			entrada = (Entrada*)entradaVoid;

			// Si ya está bloqueada, pongo todas sus entradas como posibles para ingresar
			for (int i = entrada->primerEntrada; i < entrada->cantidadEntradas; i++) {
				estructuraAdministrativa.entradasUsadas[i] = 0;
			}
		}

	// Verifico si hay espacio continuo disponible
		while (cantidadEntradasPosiblesContinuas() < entradasNecesarias) {
			ejecutarAlgoritmoDeRemplazo();
		}

	// Setteo porque hay lugar
		setearValor(clave, valor, entradasNecesarias);
}


void recibirSentencia(int socketCoordinador) {
	char* mensaje;
	char** mensajeSplitted;
	ContentHeader *header;

	header = recibirHeader(socketCoordinador);

	if (header->id == COORDINADOR) {
		mensaje = malloc(header->largo + 1);
		recibirMensaje(socketCoordinador, header->largo, &mensaje);

		mensajeSplitted = string_split(mensaje, " ");

		if (strcmp(mensajeSplitted[0], "SET") == 0) {
			setearClave(mensajeSplitted[1], mensajeSplitted[2]);
		} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
			puts("STORE");
		} else {
			puts("Error en el mensaje enviado al coordinador por el ESI");
		}

		free(mensaje);
		free(mensajeSplitted);
	}

	free(header);
}

int main() {
	puts("Iniciando Instancia.");
	int socketCoordinador;
	char* ipCoordinador;
	int puertoCoordinador;
	char* nombre;

	InformacionEntradas * info;
	t_config* configuracion;

	// Archivo de configuracion
		configuracion = config_create("./configuraciones/configuracion.txt");
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

	// Espero las sentencias
		while(1) {
			recibirSentencia(socketCoordinador);
		}

	// Libero memoria
		free(info);
		free(ipCoordinador);
		config_destroy(configuracion);
		close(socketCoordinador);
		list_destroy_and_destroy_elements(estructuraAdministrativa.entradas, freeEntrada);

	puts("La Instancia se ha finalizado correctamente.");
	return 0;
}
