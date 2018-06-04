/*
 ============================================================================
 Name        : esi.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso ESI
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <socket/sockets.h>
#include <configuracion/configuracion.h>
#include <configuracion/configuracion.h>
#include <parsi/parser.h>

void parsearScript();

#define SIZE 1024

int filasArchivo() {
    const char filename[] = "parsi/ejemplo/script.esi";
    FILE *in_file;
    char buffer[SIZE + 1], lastchar = '\n';
    size_t bytes;
    int lines = 0;

    if (NULL == (in_file = fopen(filename, "r"))) {
        perror(filename);
        return EXIT_FAILURE;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer) - 1, in_file))) {
        lastchar = buffer[bytes - 1];
        for (char *c = buffer; (c = memchr(c, '\n', bytes - (c - buffer))); c++) {
            lines++;
        }
    }
    if (lastchar != '\n') {
        lines++;  /* Count the last line even if it lacks a newline */
    }
    if (ferror(in_file)) {
        perror(filename);
        fclose(in_file);
        return EXIT_FAILURE;
    }

    fclose(in_file);
    printf("Number of lines in the file is %i\n", lines);
    return(lines);
}

int main() {
	puts("Iniciando ESI.");
	int socketPlanificador;
	char ipPlanificador[16];
	int puertoPlanificador;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO_PLANIFICADOR:%d", &puertoPlanificador);
	leerConfiguracion("IP_PLANIFICADOR:%s", &ipPlanificador);
	socketPlanificador = clienteConectarComponente("ESI", "planificador",
			puertoPlanificador, ipPlanificador);

	int maxFilas = filasArchivo();

	parsearScript(socketPlanificador, maxFilas);

	close(socketPlanificador);

	puts("El ESI se ha finalizado correctamente.");
	return 0;
}

void parsearScript(int socketPlanificador, int maxFilas) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	ContentHeader *header;
	char* mensaje;
	int filasLeidas = 0;

	fp = fopen("parsi/ejemplo/script.esi", "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}

	while (filasLeidas < maxFilas) {

		// TODO esperar el mensaje del planificador ordenando ejecutar una linea
		header = recibirHeader(socketPlanificador);
		mensaje = malloc((header->largo) + 1);

		recibirMensaje(socketPlanificador, header->largo, &mensaje);
		if (header->id == PLANIFICADOR) {
			if ((read = getline(&line, &len, fp)) != -1) {
				t_esi_operacion parsed = parse(line);

				if (parsed.valido) {
					switch (parsed.keyword) {
					case GET:
						printf("GET\tclave: <%s>\n",
								parsed.argumentos.GET.clave);
						break;
					case SET:
						printf("SET\tclave: <%s>\tvalor: <%s>\n",
								parsed.argumentos.SET.clave,
								parsed.argumentos.SET.valor);
						break;
					case STORE:
						printf("STORE\tclave: <%s>\n",
								parsed.argumentos.STORE.clave);
						break;
					default:
						fprintf(stderr, "No pude interpretar <%s>\n", line);
						exit(EXIT_FAILURE);
					}

					filasLeidas++;
					destruir_operacion(parsed);
				} else {
					fprintf(stderr, "La linea <%s> no es valida\n", line);
					exit(EXIT_FAILURE);
				}
			} else {
				break;
			}
		}

	}
	fclose(fp);
	if (line)
	free(line);
}
