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

Instancia* algoritmoDistribucionEL(t_list* listaInstancias) {
	void* instanciaVoid;

	// Agarro la instancia correspondiente de la lista y sumo uno al contador
	instanciaVoid = list_get(listaInstancias, indexInstanciaEL);
	if (instanciaVoid == NULL) {
		indexInstanciaEL = 0;
		instanciaVoid = list_get(listaInstancias, indexInstanciaEL);

		if (instanciaVoid == NULL) {
			return NULL;
		}
	}
	indexInstanciaEL++;

	// Convierto a instancia el void* y devuelvo
	return (Instancia*)instanciaVoid;

}
