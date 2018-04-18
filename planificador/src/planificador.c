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

int cmdHelp();

#define true 1;
#define false 0;

typedef int Bool;

int done;

typedef struct COMANDO{
	char* cmd;
	int (*funcion)();
	char* info;
	int parametros;
} COMANDO;

int cmdQuit(){
	done = 1;
	return 0;
}

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

int cmdHelp(){
	register int i;
	puts("Comando:					Descripcion:");
	for(i=0; comandos[i].cmd; i++){
		printf("%s						%s\n", comandos[i].cmd, comandos[i].info);
	}
	return 0;
}

int existeComando(char* comando) {
	register int i;
	for (i = 0; comandos[i].cmd; i++) {
		if (strcmp(comando, comandos[i].cmd) == 0) {
			return i;
		}
	}
	return -1;
}

char *leerComando(char *linea) {
	char *comando;
	int i, j;
	int largocmd = 0;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
		largocmd++;
	}
	comando = malloc(largocmd + 1);
	for (j = 0; j < largocmd; j++) {
		comando[j] = linea[j];
	}
	comando[j++] = '\0';
	return comando;
}

void obtenerParametros(char **parametros, char *linea) {
	int i, j;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
	}
	(*parametros) = malloc(strlen(linea) - i);
	i++;
	for (j = 0; i < strlen(linea); j++) {
		if (linea[i] == '\0')
			break;
		(*parametros)[j] = linea[i];
		i++;
	}
	(*parametros)[j++] = '\0';
}

COMANDO *punteroComando(int posicion){
	return (&comandos[posicion]);
}

int ejecutarSinParametros(COMANDO *comando){
	return ((*(comando->funcion)) ());
}

int ejecutarConParametros(char *parametros, COMANDO *comando){
	return ((*(comando->funcion)) (parametros));
}

int verificarParametros(char *linea, int posicion) {
	int i;
	int espacios = 0;
	char *parametros;
	COMANDO *comando;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			espacios++;
	}
	if (comandos[posicion].parametros == espacios) {
		if (espacios == 0) {
			comando = punteroComando(posicion);
			ejecutarSinParametros(comando);
		} else {
			obtenerParametros(&parametros, linea);
			comando = punteroComando(posicion);
			ejecutarConParametros(parametros,comando);
			free(parametros);
		}
	} else {
		printf("%s: La cantidad de parametros ingresados es incorrecta.\n", comandos[posicion].cmd);
	}
	return 0;
}

void ejecutarComando(char *linea) {
	char *comando = leerComando(linea);
	int posicion = existeComando(comando);
	if (posicion == -1) {
		printf("%s: El comando ingresado no existe.\n", comando);
	} else {
		verificarParametros(linea, posicion);
	}
	free(comando);
}

char *recortarLinea(char *string) {
	register char *s, *t;
	for (s = string; whitespace(*s); s++)
		;
	if (*s == 0)
		return s;
	t = s + strlen(s) - 1;
	while (t > s && whitespace(*t))
		t--;
	*++t = '\0';
	return s;
}

void iniciarConsola() {
	char *linea, *aux;
	done = 0;
	while (done == 0) {
		linea = readline("user@planificador: ");
		if (!linea)
			break;
		aux = recortarLinea(linea);
		if (*aux)
			ejecutarComando(aux);
		free(linea);
	}
}

int main() {
	puts("Iniciando Planificador.");
	iniciarConsola();
	puts("El Planificador se ha finalizado correctamente.");
	return 0;
}
