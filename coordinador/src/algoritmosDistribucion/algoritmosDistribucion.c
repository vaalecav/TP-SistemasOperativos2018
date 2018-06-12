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

Instancia* maxEspacio(void* primerInstanciaVoid, void* segundaInstanciaVoid){
	Instancia* primerInstancia = (Instancia*)primerInstanciaVoid;
	Instancia* segundaInstancia = (Instancia*)segundaInstanciaVoid;
	int entradasLibresPrimera = primerInstancia->entradasLibres;
	int entradasLibresSegunda = segundaInstancia->entradasLibres;

	if(max(entradasLibresPrimera, entradasLibresSegunda) == entradasLibresPrimera){
		return primerInstancia;
	}
	return segundaInstancia;
}

Instancia* algoritmoDistribucionLSU(t_list* listaInstancias){
	void* instanciaVoid;

	instanciaVoid = list_compare_elements_get(listaInstancias, maxEspacio);

	Instancia* instancia = (Instancia*)instanciaVoid;
	return instancia;
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
	int posicionPrimerLetraClave = ultimaLetraAbecedario - primerLetraClave;

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
