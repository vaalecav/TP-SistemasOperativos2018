/*
 * esi.h
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_


#include <commonsNuestras/config.h>
#include <commonsNuestras/log.h>
#include <commonsNuestras/parsi/parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>
#include <configuracion/configuracion.h>
#include <generales/generales.h>
#include <math.h>

#define SIZE 1024

t_config* configuracion;
t_log* logESI;
int socketPlanificador;

void liberarMemoria();
void parsearScript(char*, int, int);
int filasArchivo();

#endif /* ESI_H_ */
