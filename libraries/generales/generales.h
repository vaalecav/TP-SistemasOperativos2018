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

int min(int, int);
int max(int, int);
int divCeil(int, int);
t_link_element* list_find_element_with_param(t_list*, void*, int(*condition)(void*, void*), int*);
void* list_find_with_param(t_list*, void*, int(*condition)(void*, void*));
int strcmpVoid(void*, void*);
void* list_remove_by_condition_with_param(t_list*, void*, int(*condition)(void*, void*));

#endif /* GENERALES_GENERALES_H_ */
