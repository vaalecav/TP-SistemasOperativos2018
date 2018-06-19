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

#include <commonsNuestras/config.h>
#include <commonsNuestras/log.h> // Con esta no deberia andar el t_log????
#include <commonsNuestras/string.h>
#include <socket/sockets.h>
#include <generales/generales.h>
#include <pthread.h>

#include "algoritmosDistribucion/algoritmosDistribucion.h"

t_list *listaInstancias;
pthread_mutex_t mutexListaInstancias;
pthread_mutex_t mutexLog;
t_log* logCoordinador;

enum CLAVE {
	NO_EN_INSTANCIA = 0,
	EN_INSTANCIA_BLOQUEADA = 1,
	EN_INSTANCIA_NO_BLOQUEADA = 2
};

int buscarClaveEnListaDeClaves(void*, void*);
int buscarInstanciaConClave(void*, void*);
void manejarInstancia(int, int);
void closeInstancia(void*);
void cerrarInstancias();
void asignarInstancia(char*);
void manejarEsi(int, int, int);
void manejarConexion(void*);
int correrEnHilo(SocketHilos);
int claveEstaEnInstancia(char*);
int compararClave(void*, char*);

// GET
int sePuedeComunicarConLaInstancia(Instancia*);
void getClave(char*, int, int);

// SET y STORE
void ejecutarSentencia(int, int, char*, char*);
void avisarA(int, char*, int);
int esSET(char*);
int esSTORE(char*);

#endif /* COORDINADOR_H_ */
