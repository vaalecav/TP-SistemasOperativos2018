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

#include <commonsNuestras/config.h>
#include <commonsNuestras/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <generales/generales.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <socket/sockets.h>
#include <configuracion/configuracion.h>
#include <semaphore.h>

//===================DEFINES====================================================

//=======================COMANDOS DE CONSOLA====================================

int cmdQuit(), cmdHelp(), cmdPause(), cmdContinue(), cmdColaReady(),
		cmdColaBloqueados(), cmdColaTerminados(), cmdListaClaves(),
		cmdDesbloquear(char*), cmdBloquear(char*, int), cmdKill(int), cmdStatus(
				char*); // Son las funciones que ejecutan los comandos ingresados por consola.

//==========================ESTRUCTURAS=========================================

typedef struct CLAVE {
	char* clave;
	t_list *listaEsi;
} CLAVE;

typedef struct ESI {
	int id;
	int socket;
	int lineas;
} DATA;

typedef struct COMANDO {
	char* cmd;
	int (*funcion)();
	char* info;
	int parametros;
} COMANDO;

COMANDO comandos[] = {
		{"pausar", cmdPause, "Pausa la ejecucion de ESIs.", 0},

		{"continuar", cmdContinue, "Reanuda la ejecucion de ESIs.", 0},

		{"colaTerminados", cmdColaTerminados, "Imprime en pantalla la cola de Terminados.", 0},

		{"colaBloqueados", cmdColaBloqueados,"Imprime en pantalla la cola de Bloqueados.", 0},

		{"colaReady", cmdColaReady,"Imprime en pantalla la cola de Ready.", 0},

		{"listaClaves", cmdListaClaves, "Imprime la lista de Claves.", 0},

		{"bloquear", cmdDesbloquear, "Bloquea una clave.", 2},

		{"desbloquear",cmdBloquear, "Desbloquea una clave.", 2},

		// { "listar","Este comando aun no se ha desarrollado.", 1},

		{"kill", cmdKill, "Finaliza el proceso.", 1},

		{"status", cmdStatus,"Conocer el estado de una clave.", 1},

		//{ "deadlock","Este comando aun no se ha desarrollado.", 0},

		{"help", cmdHelp, "Imprime los comandos disponibles.", 0},

		{"quit", cmdQuit, "Finaliza al Planificador.", 0},

		{(char *) NULL, (Function *) NULL, (char *) NULL, (int *) NULL}
		};


//======================VARIABLES GLOBALES======================================

int done = 0; // Es 0 por default. La pasamos a 1 para finalizar al Planificador.
int ejecutar = 0; 		// Es 0 por default. La pasamos a 1 para ejecutar ESI.
t_config* configuracion; 	// Configuracion del socket servidor.
t_list* colaReady;			// Lista enlazada de Ready.
t_list* colaBloqueados;		// Lista enlazada de Bloqueados.
t_list* colaTerminados;		// Lista enlazada de Terminados.
t_list* colaAbortados;		// Lista enlazada de Abortados.
t_list* listaClaves;		// Lista enladaza de Claves y sus Bloqueados.
int socketCoordinador;		// Socket del Coordinador.

//=====================FUNCIONES DE PLANIFICADOR=====================================

void cerrarPlanificador(); 	// Finaliza correctamente al Planificador.
void remove_element(int *array, int index, int array_length); // Quita un elemento del array.
int dameMaximo(int *tabla, int n); // Devuelve el mas alto del array.
void tratarConexiones(); // Hilo que maneja conexiones con select().
void borrarDeColas(int socket); // Borra de las listas enlazadas el ESI con el socket indicado.
int compararSocket(void* esiVoid, void* indexVoid); // Funcion que compara para funcion de listas.
void imprimirEnPantalla(void* esiVoid); // Imprime un ESI en pantalla.
void manejoAlgoritmos(); // Hilo que maneja la ejecucion de ESIs.
int chequearRespuesta(int id); // Devuelve para hacer switch con la respuesta del Coordinador.
void imprimirEnPantallaClaves(void* claveVoid); // Imprime la lista de Claves.
void imprimirEnPantallaClavesAux(void* idVoid); // Imprime sublista de Claves.
int chequearClave(void* claveVoid, void* nombreVoid); // Compara string de clave con ESI.
int buscarEnBloqueados(void* esiVoid, void* idVoid); // Busca un ESI en bloqueados.
bool menorCantidadDeLineas(void* esi1Void, void* esi2Void); // Para hacer un sort del SJF
int desbloquearClave(char* clave); // Para desbloquear clave desde el STORE
int desbloquearClaveESI(char* clave, char* valor);

//=====================FUNCIONES DE CONSOLA=====================================

void iniciarConsola(); 									// Ejecuta la consola.
void ejecutarComando(char *linea);				// Manda a ejecutar un comando.
void obtenerParametros(char **parametros, char *linea);	// Separa los parametros de la linea original.
char *leerComando(char *linea);		// Separa el comando de la linea original.
char *recortarLinea(char *string);// Quita los espacios al principio y al final de la linea.
int existeComando(char* comando);// Chequea que el comando exista en el array.
int ejecutarSinParametros(COMANDO *comando);// Llama a la funcion de un comando sin parametros.
int ejecutarConParametros(char *parametros, COMANDO *comando);// Llama a la funcion de un comando con parametros.
int verificarParametros(char *linea, int posicion);	// Chequea que la cantidad de parametros ingresada sea correcta.
COMANDO *punteroComando(int posicion);// Devuelve el puntero al comando del array.

#endif /* PLANIFICADOR_H_ */
