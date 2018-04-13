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

int done;

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
	printf("%s\n", comando);
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
