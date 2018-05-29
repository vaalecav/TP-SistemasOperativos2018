/*
 * coordinador.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

/*
 ============================================================================
 Name        : coordinador.h
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Coordinador
 ============================================================================
 */

#include <socket/sockets.h>
#include <commons/string.h>
#include <commons/config.h>
#include <comunicacion/comunicacion.h>
#include <generales/generales.h>
#include <configuracion/configuracion.h>
#include <pthread.h>
#include "algoritmosDistribucion/algoritmosDistribucion.h"

t_list *listaInstancias;
pthread_mutex_t mutexListaInstancias;

enum CLAVE{
	NO_EN_INSTANCIA = 0,
	EN_INSTANCIA_BLOQUEADA = 1,
	EN_INSTANCIA_NO_BLOQUEADA = 2
};

void manejarInstancia(int, int);
void closeInstancia(void*);
void cerrarInstancias();
void asignarInstancia(char*);
void manejarEsi(int, int, int);
void manejarConexion(void*);
int correrEnHilo(SocketHilos);
int claveEstaEnInstancia(char*);
int compararClave(void*, char*);

#endif /* COORDINADOR_H_ */
