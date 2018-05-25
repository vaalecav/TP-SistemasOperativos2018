/*
 ============================================================================
 Name        : instancia.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Instancia
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>
#include <comunicacion/comunicacion.h>
#include <commons/config.h>
#include <commons/string.h>

int recibirInformacionEntradas(int socketEmisor, InformacionEntradas** info) {
	int recibido;

	recibido = recv(socketEmisor, info, sizeof(InformacionEntradas), 0);
	if (recibido < 0) {
		return -1;
	} else if (recibido == 0) {
		close(socketEmisor);
		free(info);
		return 0;
	}

	return 1;
}

void setearValor(char* clave, char* valor, int necesarios, EstructuraAdministrativa *estructuraAdministrativa) {
	int libre = -1;
	int cantidadSeguidos = 0;
	void* entradaVoid;
	int index= -1;
	Entrada entrada;

	for (int i = 0; i < estructuraAdministrativa->cantidadEntradas; i++) {
		if (estructuraAdministrativa->entradas[i]) {
			if (!cantidadSeguidos) {
				libre = i;
			}

			cantidadSeguidos++;

			if (cantidadSeguidos == necesarios) {
				break;
			}
		} else {
			libre = -1;
			cantidadSeguidos = 0;
		}
	}

	if (libre >= 0) {
		for (int i = libre; i < libre+necesarios; i++) {
			estructuraAdministrativa->entradas[i] = 1;
		}

		int finValores = strlen(estructuraAdministrativa->valores);
		int h = 0;
		for (int i = finValores-1; i < finValores-1+strlen(valor); i++) {
			estructuraAdministrativa->valores[i] = valor[h];
			h++;
		}

		if ((entradaVoid = list_find_by_condition(estructuraAdministrativa->claves, entradaEsIgualAClave, clave)) != NULL) {
			entrada = (Entrada*)entradaVoid;
		}

		entrada->clave = malloc(strlen(clave));
		strcpy(&entrada->clave, clave);
		entrada->primerEntrada = libre;
		entrada->cantidadEntradas = necesarios;
		entrada->inicioClave = finValores-1;
		entrada->largoClave = strlen(valor);

		if (entradaVoid == NULL) {
			list_add(estructuraAdministrativa->claves, (void*)entrada);
		}

	}
}

int cantidadEntradasDisponiblesContinuas(int* entradas, int cantidadEntradas) {
	int cantidadContinuas = 0;
	for (int i = 0; i < cantidadEntradas; i++) {
		cantidadContinuas += (entradas[i] == 1);
	}

	return cantidadContinuas;
}

int min(int n1, int n2) { return n1 < n2 ? n1 : n2; }

int entradaEsIgualAClave(void* entrada, void* clave) {
	Entrada *ent = (Entrada*) entrada;
	return strcmp(ent->clave, (char*)clave)	== 0;
}

void bajarValoresDeLista(t_list* entradas, int desde, int cantidadRestar) {
	Entrada entrada;
	t_link_element *element = entradas->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;

		entrada = (Entrada*) element->data;
		if (entrada->inicioClave > desde) {
			entrada->inicioClave -= cantidadRestar;
		}

		element = aux;
	}
}

void setearClave(char* clave, char* valor, EstructuraAdministrativa *estructuraAdministrativa) {
	Entrada* entrada;
	void* entradaVoid;
	char* aux;
	int sePuede;
	int entradasNecesarias;
	int posLetra;

	// Verifico si alcanzan las entradas
		entradasNecesarias = round(sizeof(valor) / estructuraAdministrativa->tamanioEntrada);
		if (entradasNecesarias > estructuraAdministrativa->cantidadEntradas) {
			puts("La cantidad de entradas no son suficientes para el tamanio del valor pasado.");
			return;
		}

	// Verifico si ya está seteada la clave
		if ((entradaVoid = list_find_by_condition(estructuraAdministrativa->claves, entradaEsIgualAClave, clave)) != NULL) {
			entrada = (Entrada*)entradaVoid;

			// Si ya está bloqueada, pongo todas sus entradas como posibles para ingresar
			for (int i = entrada->primerEntrada; i < entrada->cantidadEntradas; i++) {
				estructuraAdministrativa->entradas[i] = 0;
			}

			// Borro la palabra de los valores
			for (posLetra = entrada->inicioClave; posLetra < min(entrada->inicioClave + entrada->largoClave, strlen(estructuraAdministrativa->valores)); posLetra++) {
				estructuraAdministrativa->valores[posLetra] = estructuraAdministrativa->valores[posLetra + entrada->largoClave];
			}
			estructuraAdministrativa->valores[posLetra] = '\0';
			bajarValoresDeLista(estructuraAdministrativa->entradas, entrada->inicioClave, entrada->largoClave);

		}

	// Verifico si hay espacio continuo disponible
		sePuede = cantidadEntradasDisponiblesContinuas(estructuraAdministrativa->entradas) >= entradasNecesarias;

	// Seteo
		if (sePuede) {
			setearValor(clave, valor, entradasNecesarias, &estructuraAdministrativa);
		} else {
			puts("ejecutar algoritmo de remplazo");
		}
}


void recibirSentencia(int socketCoordinador, EstructuraAdministrativa *estructuraAdministrativa) {
	char* mensaje;
	char** mensajeSplitted;
	ContentHeader *header;

	header = recibirHeader(socketCoordinador);

	if (header->id == COORDINADOR) {
		mensaje = malloc(header->largo + 1);
		recibirMensaje(socketCoordinador, header->largo, &mensaje);

		mensajeSplitted = string_split(mensaje, " ");
		if (strcmp(mensajeSplitted[0], "SET") == 0) {
			puts("SET");
			setearClave(mensajeSplitted[1], mensajeSplitted[2], &estructuraAdministrativa);
		} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
			puts("STORE");
		} else {
			puts("Error en el mensaje enviado al coordinador por le ESI");
		}

		free(mensaje);
		free(mensajeSplitted);
	}

	free(header);
}

void limpiarEstructuraAdministrativa(EstructuraAdministrativa *estructura) {
	list_destroy_and_destroy_elements(estructura->claves);
	free(estructura->valores);
	free(estructura->entradas);
	free(estructura);
}

int main() {
	puts("Iniciando Instancia.");
	int socketCoordinador;
	char* ipCoordinador;
	int puertoCoordinador;
	char* nombre;

	InformacionEntradas * info;
	EstructuraAdministrativa *estructuraAdministrativa;
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
		while (recibirInformacionEntradas(socketCoordinador, &info) != 1);

		estructuraAdministrativa = (EstructuraAdministrativa*) malloc(sizeof(EstructuraAdministrativa));
		estructuraAdministrativa->cantidadEntradas = info->cantidad;
		estructuraAdministrativa->tamanioEntrada = info->tamanio;

		// El string de valores tendrá, a lo sumo, la cantidad de entradas * el tamanio que ocupa cada una
		estructuraAdministrativa->valores = malloc(info->tamanio * info->cantidad + sizeof(char));

		// A todas las entradas las inicio como vacías (0)
		estructuraAdministrativa->entradas = malloc(sizeof(int) * info->cantidad);
		for (int i = 0; i < info->cantidad; i++) {
			estructuraAdministrativa->entradas[i] = 0;
		}

		// Creo la lista de claves por primera vez
		estructuraAdministrativa->claves = list_create();
		estructuraAdministrativa->claves[0] = '\0';

	// Espero las sentencias
		while(1) {
			recibirSentencia(socketCoordinador, &estructuraAdministrativa);
		}

	// Cierro todas las cosas
		close(socketCoordinador);
		free(info);
		limpiarEstructuraAdministrativa(&estructuraAdministrativa);
		free(ipCoordinador);
		config_destroy(configuracion);

	puts("La Instancia se ha finalizado correctamente.");
	return 0;
}
