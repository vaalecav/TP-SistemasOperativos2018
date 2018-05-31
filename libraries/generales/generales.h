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
#include <string.h>

int min(int, int);
int divCeil(int, int);
t_link_element* list_find_element_with_param(t_list*, void*, int(*condition)(void*, void*), int*);
void* list_find_with_param(t_list*, void*, int(*condition)(void*, void*));
int strcmpVoid(void*, void*);

#endif /* GENERALES_GENERALES_H_ */
