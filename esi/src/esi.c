/*
 ============================================================================
 Name        : esi.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso ESI
 ============================================================================
 */

#include "esi.h"

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

int main(int argc, char **argv){
	puts("Iniciando ESI.");
	int socketPlanificador;
	char* ipPlanificador;
	int puertoPlanificador;

	//Leo puertos e ips de archivo de configuracion
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	ipPlanificador = config_get_string_value(configuracion, "IP_PLANIFICADOR");
	puertoPlanificador = config_get_int_value(configuracion, "PUERTO_PLANIFICADOR");

	socketPlanificador = clienteConectarComponente("ESI", "planificador", puertoPlanificador, ipPlanificador);

	int maxFilas = filasArchivo();

	parsearScript(socketPlanificador, maxFilas, argv[1]);

	close(socketPlanificador);
	config_destroy(configuracion);

	puts("El ESI se ha finalizado correctamente.");
	return 0;
}

void parsearScript(int socketPlanificador, int maxFilas, char* path) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	ContentHeader *headerPlanificador, *headerCoordinador;
	char* mensaje;
	int filasLeidas = 0;
	char* ipCoordinador;
	int puertoCoordinador;

	char* mensajeCoordinador;
	int socketCoordinador;
	char* respuestaCoordinador;

	//leo puertos e ip del coordinador
	ipCoordinador = config_get_string_value(configuracion, "IP_COORDINADOR");
	puertoCoordinador = config_get_int_value(configuracion, "PUERTO_COORDINADOR");


	fp = fopen(path, "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}

	while (filasLeidas < maxFilas) {
		headerPlanificador = recibirHeader(socketPlanificador);
		mensaje = malloc((headerPlanificador->largo) + 1);
		recibirMensaje(socketPlanificador, headerPlanificador->largo, &mensaje);

		if (headerPlanificador->id == PLANIFICADOR) {
			if ((read = getline(&line, &len, fp)) != -1) {
				t_esi_operacion parsed = parse(line);

				if (parsed.valido) {
					switch (parsed.keyword) {
					case GET:
						mensajeCoordinador = malloc(strlen("GET ") + strlen(parsed.argumentos.GET.clave) + 1);
						strcpy(mensajeCoordinador, "GET ");
						strcat(mensajeCoordinador, parsed.argumentos.GET.clave);
						mensajeCoordinador[strlen("GET ") + strlen(parsed.argumentos.GET.clave)] = '\0';
						break;
					case SET:
						mensajeCoordinador = malloc(strlen("SET ") + strlen(parsed.argumentos.SET.clave + strlen(parsed.argumentos.SET.valor)) +1);
						strcpy(mensajeCoordinador, "SET ");
						strcat(mensajeCoordinador, parsed.argumentos.SET.clave);
						strcat(mensajeCoordinador, " ");
						strcat(mensajeCoordinador, parsed.argumentos.SET.valor);
						mensajeCoordinador[strlen("GET ") + strlen(parsed.argumentos.SET.clave) + strlen(parsed.argumentos.SET.valor)] = '\0';
						break;
					case STORE:
						mensajeCoordinador = malloc(strlen("STORE ") + strlen(parsed.argumentos.STORE.clave) +1);
						strcpy(mensajeCoordinador, "STORE ");
						strcat(mensajeCoordinador, parsed.argumentos.STORE.clave);
						mensajeCoordinador[strlen("STORE ") + strlen(parsed.argumentos.STORE.clave)] = '0';
						break;
					default:
						fprintf(stderr, "No pude interpretar <%s>\n", line);
						exit(EXIT_FAILURE);
					}

					socketCoordinador = clienteConectarComponente("ESI", "coordinador", puertoCoordinador, ipCoordinador);
					enviarHeader(socketCoordinador, mensajeCoordinador, ESI);
					enviarMensaje(socketCoordinador, mensajeCoordinador);

					headerCoordinador = recibirHeader(socketCoordinador);
					respuestaCoordinador = malloc(headerCoordinador->largo + 1);
					recibirMensaje(socketCoordinador, headerCoordinador->largo, &respuestaCoordinador);


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
