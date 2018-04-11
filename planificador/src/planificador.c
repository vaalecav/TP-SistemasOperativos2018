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

#include <readline/readline.h>
#include <readline/history.h>

int main() {
	char* line;
	puts("Inicializando Planificador...");
	while (1) {
		line = readline("user@planificador: ");
		if (strcmp(line, "exit") == 0) {
			free(line);
			puts("Finalizando Planificador...");
			exit(0);
		}
		printf("El comando %s no es un comando valido.\n", line);
		free(line);
	}

	return 0;
}
