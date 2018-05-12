/*
 ============================================================================
 Name        : algoritmosDistribucion.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Algoritmos de distribucion para el coordinador
 ============================================================================
 */
#include "algoritmosDistribucion.h"

void* algoritmoDistribucionEL(t_list* listaInstancias) {
	void* socketInstancia;

	// Agarro el socket correspondiente de la lista y sumo uno al contador
	socketInstancia = list_get(listaInstancias, indexInstanciaEL);
	if (socketInstancia == NULL) {
		indexInstanciaEL = 0;
		socketInstancia = list_get(listaInstancias, indexInstanciaEL);

		if (socketInstancia == NULL) {
			return NULL;
		}
	}
	indexInstanciaEL++;

	// Convierto a int el socket y devuelvo
	return socketInstancia;

}
