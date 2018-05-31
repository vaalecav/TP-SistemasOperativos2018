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
		entrada->clave[strlen(clave) + 1] = '\0';
		entrada->valor = malloc(strlen(valor) + 1);
		strcpy(entrada->valor, valor);
		entrada->valor[strlen(valor) + 1] = '\0';
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
		*/
	}
}

int cantidadentradasUsadasContinuas() {
	int cantidadContinuas = 0;
	for (int i = 0; i < estructuraAdministrativa.cantidadEntradas; i++) {
		if (estructuraAdministrativa.entradasUsadas[i]) {
			cantidadContinuas = 0;
		} else {
			cantidadContinuas++;
		}
	}

	return cantidadContinuas;
}

void setearClave(char* clave, char* valor) {
	Entrada* entrada;
	void* entradaVoid;
	int sePuede;
	int entradasNecesarias;

	// Verifico si alcanzan las entradas
		entradasNecesarias = (int)(ceil((double)strlen(valor) / (double)estructuraAdministrativa.tamanioEntrada));

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
		sePuede = cantidadentradasUsadasContinuas() >= entradasNecesarias;

	// Seteo
		if (sePuede) {
			setearValor(clave, valor, entradasNecesarias);
		} else {
			puts("ejecutar algoritmo de remplazo");
		}
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

void freeEntrada(void* ent) {
	Entrada* entrada = (Entrada*)ent;
	free(entrada->clave);
	free(entrada->valor);
	free(ent);
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


	// Cierro todas las cosas que no necesite para recibir sentencias
		free(info);
		free(ipCoordinador);
		config_destroy(configuracion);


	// Espero las sentencias
		while(1) {
			recibirSentencia(socketCoordinador);
		}

	// Cierro lo que falta
		close(socketCoordinador);
		list_destroy_and_destroy_elements(estructuraAdministrativa.entradas, freeEntrada);

	puts("La Instancia se ha finalizado correctamente.");
	return 0;
}
