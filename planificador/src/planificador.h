/*
 ============================================================================
 Name        : planificador.h
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Planificador
 ============================================================================
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <socket/sockets.h>
#include <configuracion/configuracion.h>
#include <semaphore.h>

//===================DEFINES====================================================


//=======================COMANDOS DE CONSOLA====================================

int cmdQuit(), cmdHelp(), cmdListaEsi(), cmdPause(), cmdContinue(), cmdColaReady(); // Son las funciones que ejecutan los comandos ingresados por consola.

//==========================ESTRUCTURAS=========================================

typedef struct COMANDO{
	char* cmd;
	int (*funcion)();
	char* info;
	int parametros;
} COMANDO;

COMANDO comandos[] = {
		{ "pausar", cmdPause,"Pausa la ejecucion de ESIs.", 0},
		{ "continuar", cmdContinue,"Reanuda la ejecucion de ESIs.", 0},
/*		{ "bloquear","Este comando aun no se ha desarrollado.", 2},
		{ "desbloquear","Este comando aun no se ha desarrollado.", 1},
		{ "listar","Este comando aun no se ha desarrollado.", 1},
		{ "kill","Este comando aun no se ha desarrollado.", 1},
		{ "status","Este comando aun no se ha desarrollado.", 1},
		{ "deadlock","Este comando aun no se ha desarrollado.", 0},*/
		{ "help", cmdHelp, "Imprime los comandos disponibles.", 0},
		{ "quit", cmdQuit, "Finaliza al Planificador.", 0},
		{ "listaEsi", cmdListaEsi, "Muestra la lista de ESI actual.", 0},
		{ "colaReady", cmdColaReady, "Muestra la cola de Ready actual.", 0},
		{ (char *)NULL, (Function *)NULL, (char *)NULL, (int *) NULL}
};

//======================VARIABLES GLOBALES======================================

int done = 0; // Es 0 por default. La pasamos a 1 para finalizar al Planificador.
int ejecutar = 0;
int socketServer;
int socketCliente[100];
int colaReady[100];
int numeroClientes = 0;
int sem_clientes = 1;

//=====================ALGORITMO FIFO===========================================

int tomarPrimero(int *array, int array_length);
void algoritmoFifo();

//=====================FUNCIONES DE MANEJO DE ESI===============================

void remove_element(int *array, int index, int array_length);
void tratarConexiones();
void cerrarConexiones();

//=====================FUNCIONES DE CONSOLA=====================================

void iniciarConsola(); 												// Ejecuta la consola.
void ejecutarComando(char *linea);									// Manda a ejecutar un comando.
void obtenerParametros(char **parametros, char *linea);				// Separa los parametros de la linea original.
char *leerComando(char *linea);										// Separa el comando de la linea original.
char *recortarLinea(char *string);									// Quita los espacios al principio y al final de la linea.
int existeComando(char* comando);									// Chequea que el comando exista en el array.
int ejecutarSinParametros(COMANDO *comando);						// Llama a la funcion de un comando sin parametros.
int ejecutarConParametros(char *parametros, COMANDO *comando);		// Llama a la funcion de un comando con parametros.
int verificarParametros(char *linea, int posicion);					// Chequea que la cantidad de parametros ingresada sea correcta.
COMANDO *punteroComando(int posicion);								// Devuelve el puntero al comando del array.


#endif /* PLANIFICADOR_H_ */
