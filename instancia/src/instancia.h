/*
 * instancia.h
 *
 *  Created on: 28 may. 2018
 *      Author: utnso
 */

#ifndef INSTANCIA_H_
#define INSTANCIA_H_

/*
 ============================================================================
 Name        : instancia.h
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Coordinador
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <socket/sockets.h>
#include <commonsNuestras/config.h>
#include <commonsNuestras/string.h>
#include <commonsNuestras/log.h>
#include <generales/generales.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/stat.h>

EstructuraAdministrativa estructuraAdministrativa;
int indexCirc;
int terminar;

// Los  mantengo globales para poder liberarles la memoria
InformacionEntradas * info;
t_config* configuracion;
int socketCoordinador;
t_log* logInstancia;

// Manejo de Entradas
void freeEntrada(void*);
void loguearEntrada(void*);
void mostrarEntrada(void*);
int recibirInformacionEntradas(int, InformacionEntradas*);
int cantidadEntradasPosiblesContinuas();
int entradaEsIgualAClave(void*, void*);

// Manejo de Claves
int setearClave(char*, char*);
int storeClave(char*);

void avisarAlCoordinador(int);
void cerrarInstancia(int);
int buscarEspacioEnTabla(int);
int setearValor(char*, char*, int);
int tieneElIndexYEsAtomico(void*, void*);
void ejecutarAlgoritmoDeRemplazo();
void recibirSentencia();

#endif /* INSTANCIA_H_ */
