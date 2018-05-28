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
	t_list *claves;
} __attribute__((packed)) Instancia;

typedef struct {
	char *valores;
	t_list *entradas;
	int *entradasUsadas;
	int cantidadEntradas;
	int tamanioEntrada;
} __attribute__((packed)) EstructuraAdministrativa;

typedef struct {
	char* clave;
	char* valor;
	int primerEntrada;
	int cantidadEntradas;
} __attribute__((packed)) Entrada;

typedef struct {
  char* nombre;
  int bloqueado;
} __attribute__((packed)) Clave;

typedef struct {
  int socketComponente;
  int socketPlanificador;
} __attribute__((packed)) SocketHilos;

#endif /* COMUNICACION_COMUNICACION_H_ */
