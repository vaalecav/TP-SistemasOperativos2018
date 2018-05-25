/*
 * comunicacion.h
 *
 *  Created on: 25 may. 2018
 *      Author: utnso
 */

#ifndef COMUNICACION_COMUNICACION_H_
#define COMUNICACION_COMUNICACION_H_
#include <collections/list.h>

//estructuras
typedef struct {
  int socket;
  char* nombre;
  t_list* claves;
} __attribute__((packed)) Instancia;

typedef struct {
  int cantidad;
  int tamanio;
} __attribute__((packed)) InformacionEntradas;

typedef struct {
  char* nombre;
  int bloqueado;
} __attribute__((packed)) Clave;

typedef struct {
  int socketComponente;
  int socketPlanificador;
} __attribute__((packed)) SocketHilos;




#endif /* COMUNICACION_COMUNICACION_H_ */
