/*
 * esi.h
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_


#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>
#include <configuracion/configuracion.h>
#include <commons/config.h>
#include <commons/parsi/parser.h>
#include <commons/log.h>
#include <generales/generales.h>

#define SIZE 1024

t_config* configuracion;

void parsearScript(int, int, char*);
int filasArchivo();




#endif /* ESI_H_ */
