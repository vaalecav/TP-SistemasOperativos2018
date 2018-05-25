/*
 * comunicacion.h
 *
 *  Created on: 25 may. 2018
 *      Author: utnso
 */

#ifndef COMUNICACION_COMUNICACION_H_
#define COMUNICACION_COMUNICACION_H_

#include <commons/collections/list.h>

//estructuras
typedef struct {
	int cantidad;
	int tamanio;
} __attribute__((packed)) InformacionEntradas;

typedef struct {
	int socket;
	char* nombre;
	char** claves;
} __attribute__((packed)) Instancia;

typedef struct {
	char *valores;
	t_list *claves;
	int *entradas;
	int cantidadEntradas;
	int tamanioEntrada;
} __attribute__((packed)) EstructuraAdministrativa;

typedef struct {
	char* clave;
	int inicioClave;
	int largoClave;
	int primerEntrada;
	int cantidadEntradas;
} __attribute__((packed)) Entrada;

#endif /* COMUNICACION_COMUNICACION_H_ */
