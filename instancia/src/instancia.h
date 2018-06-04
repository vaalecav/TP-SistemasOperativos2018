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
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <generales/generales.h>
#include <signal.h>
#include <stdbool.h>

EstructuraAdministrativa estructuraAdministrativa;
int indexCirc;
int terminar;

// Los  mantengo globales para poder liberarles la memoria
InformacionEntradas * info;
t_config* configuracion;
int socketCoordinador;
t_log* logInstancia;

#endif /* INSTANCIA_H_ */
