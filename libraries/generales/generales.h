/*
 * generales.h
 *
 *  Created on: 28 may. 2018
 *      Author: utnso
 */

#ifndef GENERALES_GENERALES_H_
#define GENERALES_GENERALES_H_

#include <commons/collections/list.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>

/*
Eclipse
*/
#define ARCHIVO_CONFIGURACION "./configuraciones/configuracion.txt"
#define ARCHIVO_LOG "../log.txt"
/*
Consola
#define ARCHIVO_CONFIGURACION "../configuraciones/configuracion.txt"
#define ARCHIVO_LOG "../../log.txt"
*/


int min(int, int);
int max(int, int);
int divCeil(int, int);
t_link_element* list_find_element_with_param(t_list*, void*, int(*condition)(void*, void*), int*);
void* list_find_with_param(t_list*, void*, int(*condition)(void*, void*));
int strcmpVoid(void*, void*);
void* list_remove_by_condition_with_param(t_list*, void*, int(*condition)(void*, void*));

// Estructuras necesarias

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

#endif /* GENERALES_GENERALES_H_ */
