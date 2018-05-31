/*
 * algoritmoDeDistribucion.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef ALGORITMOSDISTRIBUCION_ALGORITMOSDISTRIBUCION_H_
#define ALGORITMOSDISTRIBUCION_ALGORITMOSDISTRIBUCION_H_

/*
 ============================================================================
 Name        : algoritmosDistribucion.h
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Algoritmos de distribucion para el coordinador
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <comunicacion/comunicacion.h>

int indexInstanciaEL;

Instancia* algoritmoDistribucionEL(t_list*);

#endif /* ALGORITMOSDISTRIBUCION_ALGORITMOSDISTRIBUCION_H_ */
