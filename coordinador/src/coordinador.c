/*
 ============================================================================
 Name        : coordinador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Coordinador
 ============================================================================
 */
#include "coordinador.h"

void manejarInstancia(int socketInstancia, int largoMensaje) {
	int tamanioInformacionEntradas = sizeof(InformacionEntradas);
	Instancia *instanciaConectada = (Instancia*) malloc(sizeof(Instancia));
	InformacionEntradas * entradasInstancia = (InformacionEntradas*) malloc(tamanioInformacionEntradas);
	t_config* configuracion;
	char* nombreInstancia;

	nombreInstancia = malloc(largoMensaje + 1);
	configuracion = config_create("./configuraciones/configuracion.txt");

	//recibimos el nombre de la instancia que se conecto
	recibirMensaje(socketInstancia, largoMensaje, &nombreInstancia);

	//leo cantidad entradas y su respectivo tamanio del archivo de configuracion
	entradasInstancia->cantidad = config_get_int_value(configuracion, "CANTIDAD_ENTRADAS");
	entradasInstancia->tamanio = config_get_int_value(configuracion, "TAMANIO_ENTRADA");

	//enviamos cantidad de entradas y su respectivo tamanio a la instancia
	enviarInformacion(socketInstancia, entradasInstancia, &tamanioInformacionEntradas);

	instanciaConectada->nombre = nombreInstancia;
	instanciaConectada->socket = socketInstancia;
	instanciaConectada->claves = list_create();

	//agregamos a la lista de instancias
	pthread_mutex_lock(&mutexListaInstancias);
	list_add(listaInstancias, (void*) instanciaConectada);
	pthread_mutex_unlock(&mutexListaInstancias);

	free(entradasInstancia);
	free(nombreInstancia);
	config_destroy(configuracion);
}

void closeInstancia(void* instancia) {
	close(*(int*) instancia);
}

void cerrarInstancias() {
	list_destroy_and_destroy_elements(listaInstancias, (void*) closeInstancia);
}

void asignarInstancia(char* mensaje) {
	char algoritmo_distribucion[4];
	void* socketInstancia;

	leerConfiguracion("ALG_DISTR:%s", &algoritmo_distribucion);

	pthread_mutex_lock(&mutexListaInstancias);
	if (strcmp(algoritmo_distribucion, "EL") == 0) {
		socketInstancia = algoritmoDistribucionEL(listaInstancias);
	} else if (strcmp(algoritmo_distribucion, "LSU") == 0) {
		// TODO Implementar algoritmo Least Space Used
	} else if (strcmp(algoritmo_distribucion, "KE") == 0) {
		// TODO Implementar Key Explicit
	} else {
		puts("Error al asignar instancia. Algoritmo de distribucion invalido.");
		pthread_mutex_unlock(&mutexListaInstancias);
		return;
	}
	pthread_mutex_unlock(&mutexListaInstancias);

	if (socketInstancia != NULL) {
		enviarHeader(*(int*) socketInstancia, mensaje, COORDINADOR);
		enviarMensaje(*(int*) socketInstancia, mensaje);
	} else {
		puts("No hay instancias creadas a la cual asignar el mensaje.");
	}
}

int compararClave(void* data, char*clave){
	Instancia *instancia = (Instancia*)data;
	void* resultadoBusqueda = list_find_with_param(instancia->claves, (void*) clave, strcmpVoid);

	if(resultadoBusqueda != NULL){
		Clave *claveInstancia = (Clave*)resultadoBusqueda;
		if(claveInstancia->bloqueado){
			return EN_INSTANCIA_BLOQUEADA; //hay una instancia que ya tiene la clave y esta bloqueada
		}else{
			claveInstancia->bloqueado = 1; //la bloqueo
			return EN_INSTANCIA_NO_BLOQUEADA; // hay una instancia que ya tiene la clave pero no esta bloqueada
		}
	}
	return NO_EN_INSTANCIA;
}

int claveEstaEnInstancia(char* clave){
	//recorro lista de instancias -> recorro cada lista de claves
	t_link_element *element = listaInstancias->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		int resultado = compararClave(element->data, clave);
		if(resultado == EN_INSTANCIA_BLOQUEADA){
			return EN_INSTANCIA_BLOQUEADA; //hay una instancia que ya tiene la clave y esta bloqueada
		}else if(resultado == EN_INSTANCIA_NO_BLOQUEADA){
			return EN_INSTANCIA_NO_BLOQUEADA; //hay una instancia que ya tiene la clave y no esta bloqueada
		}
		element = aux;
	}
	return NO_EN_INSTANCIA;
}

void manejarEsi(int socketEsi, int socketPlanificador, int largoMensaje) {
	char* mensaje;
	char** mensajeSplitted;

	mensaje = malloc(largoMensaje + 1);
	recibirMensaje(socketEsi, largoMensaje, &mensaje);

	mensajeSplitted = string_split(mensaje, " ");

	if (strcmp(mensajeSplitted[0], "GET") == 0) {
		puts("GET");

		int estadoClave = claveEstaEnInstancia(mensajeSplitted[1]);
		if(estadoClave == EN_INSTANCIA_BLOQUEADA){ //Notifico situacion al planificador
			enviarHeader(socketPlanificador, mensajeSplitted[1], COORDINADOR_ESI_BLOQUEADO); //LE ENVIO LA CLAVE
			enviarMensaje(socketPlanificador, mensajeSplitted[1]);
		} else if(estadoClave == EN_INSTANCIA_NO_BLOQUEADA){
			//clave ya se bloqueo en lista instancias
			enviarHeader(socketPlanificador, mensajeSplitted[1], COORDINADOR_ESI_BLOQUEAR);
			enviarMensaje(socketPlanificador, mensajeSplitted[1]);
		} else if(estadoClave == NO_EN_INSTANCIA){
			enviarHeader(socketPlanificador, mensajeSplitted[1], COORDINADOR_ESI_CREADO);
			enviarMensaje(socketPlanificador, mensajeSplitted[1]);
		}

	} else if (strcmp(mensajeSplitted[0], "SET") == 0) {
		puts("SET");
		asignarInstancia(mensaje);
		// TODO Acá, de la lista de instancias, hay que elegir dependiendo del tipo que se obtiene por configuracion
	} else if (strcmp(mensajeSplitted[0], "STORE") == 0) {
		puts("STORE");
	} else {
		puts("Error en el mensaje enviado al coordinador por le ESI");
	}

	free(mensaje);
	free(mensajeSplitted);
	close(socketEsi);
}

void manejarConexion(void* socketsNecesarios) {
	SocketHilos socketsConectados = *(SocketHilos*)socketsNecesarios;
	ContentHeader * header;

	header = recibirHeader(socketsConectados.socketComponente);

	switch(header->id){
		case INSTANCIA:
			manejarInstancia(socketsConectados.socketComponente, header->largo);
			break;

		case ESI:
			manejarEsi(socketsConectados.socketComponente, socketsConectados.socketPlanificador, header->largo);
			break;
	}

	free(header);
}

int correrEnHilo(SocketHilos socketsConectados) {
	pthread_t idHilo;
	SocketHilos* socketsNecesarios;
	socketsNecesarios = malloc(sizeof(SocketHilos));
	*socketsNecesarios = socketsConectados;

	printf("Antes del Hilo\n");
	if (pthread_create(&idHilo, NULL, (void*) manejarConexion, (void*)socketsNecesarios)) {
		perror("No se pudo crear el Hilo");
		exit(1);
	}

	puts("Manejador asignado");
	pthread_join(idHilo, NULL);
	printf("Despues del Hilo\n");

	return 1;
}

int main() {
	puts("Iniciando Coordinador.");
	int socketEscucha, socketComponente, socketConectadoPlanificador;
	SocketHilos socketsNecesarios;
	t_config* configuracion;
	char ip[16];
	int puerto;
	int maxConexiones;

	indexInstanciaEL = 0;

	//Leo puertos e ips de archivo de configuracion
	configuracion = config_create("./configuraciones/configuracion.txt");
	puerto = config_get_int_value(configuracion, "PUERTO");
	memcpy(ip, config_get_string_value(configuracion, "IP"), sizeof(ip));
	maxConexiones = config_get_int_value(configuracion, "MAX_CONEX");

	socketEscucha = socketServidor(puerto, ip, maxConexiones);

	socketConectadoPlanificador = servidorConectarComponente(&socketEscucha, "coordinador", "planificador");

	listaInstancias = list_create();
	while((socketComponente = servidorConectarComponente(&socketEscucha,"",""))) {//preguntar si hace falta mandar msjes de ok x cada hilo
		socketsNecesarios.socketComponente = socketComponente;
		socketsNecesarios.socketPlanificador = socketConectadoPlanificador;
		correrEnHilo(socketsNecesarios);
	}

	close(socketEscucha);
	close(socketConectadoPlanificador);
	config_destroy(configuracion);
	cerrarInstancias();
	puts("El Coordinador se ha finalizado correctamente.");
	return 0;
}
