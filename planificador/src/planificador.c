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

#define true 1;
#define false 0;

typedef int Bool;
//typedef int Function;

typedef struct {
	char* cmd;
	//Function func;
	char* info;
} COMANDO;

COMANDO comandos[] = {
		{ "pausar", "Este comando aun no se ha desarrollado."},
		{ "continuar", "Este comando aun no se ha desarrollado."},
		{ "bloquear", "Este comando aun no se ha desarrollado."},
		{ "desbloquear", "Este comando aun no se ha desarrollado."},
		{ "listar","Este comando aun no se ha desarrollado."},
		{ "kill", "Este comando aun no se ha desarrollado."},
		{ "status","Este comando aun no se ha desarrollado."},
		{ "deadlock","Este comando aun no se ha desarrollado."}
};

int done;

Bool existeComando(char* comando){
	register int i;
	for(i = 0; comandos[i].cmd; i++) {
		if (strcmp(comando, comandos[i].cmd) == 0){
			return true;
		}
	}
	return false;
}

void leerComando(char *linea, char **comando) {
	int i, j;
	int largocmd = 0;
	for (i = 0; i < strlen(linea); i++) {
		if (linea[i] == ' ')
			break;
		largocmd++;
	}
	(*comando) = malloc(largocmd);
	for (j = 0; j < largocmd; j++) {
		(*comando)[j] = linea[j];
	}
	(*comando)[j++] = '\0';
}

void ejecutarComando(char *linea) {
	char *comando;
	leerComando(linea, &comando);
	if(!existeComando(comando)){
		puts("El comando ingresado no existe.");
	} else {
		puts("El comando existe! :)");
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
