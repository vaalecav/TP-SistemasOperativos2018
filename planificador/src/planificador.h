/*
 * planificador.h
 *
 *  Created on: 25 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

/*
 ============================================================================
 Name        : planificador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Planificador
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <socket/sockets.h>

//===================DEFINES====================================================
#define PUERTO_COORDINADOR 8000
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO 8001
#define IP "127.0.0.2"

//=======================COMANDOS DE CONSOLA====================================

int cmdQuit(), cmdHelp(); // Son las funciones que ejecutan los comandos ingresados por consola.

//==========================ESTRUCTURAS=========================================

typedef struct COMANDO{
	char* cmd;
	int (*funcion)();
	char* info;
	int parametros;
} COMANDO;

COMANDO comandos[] = {
/*		{ "pausar","Este comando aun no se ha desarrollado.", 0},
		{ "continuar","Este comando aun no se ha desarrollado.", 0},
		{ "bloquear","Este comando aun no se ha desarrollado.", 2},
		{ "desbloquear","Este comando aun no se ha desarrollado.", 1},
		{ "listar","Este comando aun no se ha desarrollado.", 1},
		{ "kill","Este comando aun no se ha desarrollado.", 1},
		{ "status","Este comando aun no se ha desarrollado.", 1},
		{ "deadlock","Este comando aun no se ha desarrollado.", 0},*/
		{ "help", cmdHelp, "Imprime los comandos disponibles.", 0},
		{ "quit", cmdQuit, "Finaliza al Planificador.", 0}
};

//======================VARIABLES GLOBALES======================================

int done; // Es 0 por default. La pasamos a 1 para finalizar al Planificador.

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
