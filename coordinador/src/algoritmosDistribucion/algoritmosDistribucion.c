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

bool mayorEspacioLibre(void* instancia1Void, void* instancia2Void){
	Instancia* instancia1 = (Instancia*) instancia1Void;
	Instancia* instancia2 = (Instancia*) instancia2Void;

	return instancia1->entradasLibres > instancia2->entradasLibres;
}

Instancia* algoritmoDistribucionLSU(t_list* listaInstancias){
	void* instanciaVoid;

	//ordeno lista de instancias por mayor cantidad de entradas libres
	list_sort(listaInstancias, mayorEspacioLibre);
	//obtengo la instancia que tenga mas entradas libres
	instanciaVoid = list_get(listaInstancias, 0);

	return (Instancia*)instanciaVoid;
}

Instancia* algoritmoDistribucionKE(t_list* listaInstancias, char* nombreClave){
	void* instanciaVoid;
	int cantidadInstancias;
	int letrasPorInstancia;
	int numeroInstancia = 0;
	int cantidadLetrasAbecedario = 25;
	int ultimaLetraAbecedario = 122; //z

	//obtengo ASCII de la primer letra de la clave
	int primerLetraClave = nombreClave[0];
	int posicionPrimerLetraClave = cantidadLetrasAbecedario - (ultimaLetraAbecedario - primerLetraClave);

	//obtengo la cantidad de letras que habra por instancia
	cantidadInstancias = list_size(listaInstancias);
	letrasPorInstancia = divCeil(cantidadLetrasAbecedario, cantidadInstancias);

	while(posicionPrimerLetraClave > letrasPorInstancia){
		posicionPrimerLetraClave -= letrasPorInstancia;
		numeroInstancia ++;
	}

	instanciaVoid = list_get(listaInstancias, numeroInstancia);

	return (Instancia*)instanciaVoid;
}
