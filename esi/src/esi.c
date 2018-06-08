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
#include <commons/parsi/parser.h>
#include <commons/config.h>
#include <generales/generales.h>

t_config* configuracion;
void cerrarEsi(int socketPlanificador);

int main() {
	// Declaraciones Iniciales //
	puts("Iniciando ESI.");
	char* ipPlanificador;
	int puertoPlanificador;

	char* path = argv[1];

	// Leo el Archivo de Configuracion
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	puertoPlanificador = config_get_int_value(configuracion, "PUERTO_PLANIFICADOR");
	ipPlanificador = config_get_string_value(configuracion, "IP_PLANIFICADOR");

	// Levanto la conexion con el Planificador //
	socketPlanificador = clienteConectarComponente("ESI", "planificador",
			puertoPlanificador, ipPlanificador);

	// Valido que haya ingresado el nombre del archivo
	if (argc < 2) {
		log_error(logESI, "No ingreso el nombre del archivo");
		log_destroy(logESI);
		config_destroy(configuracion);
		exit(EXIT_FAILURE);
	}

	// Me conecto con el planificador
	socketPlanificador = clienteConectarComponente("ESI", "planificador", puertoPlanificador, ipPlanificador);

	int maxFilas = filasArchivo(path);
	parsearScript(path, maxFilas);

	// Libero memoria
	liberarMemoria();
	config_destroy(configuracion);

	return 0;
}

void parsearScript(char* path, int maxFilas) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	ContentHeader *headerPlanificador, *headerCoordinador;
	int filasLeidas = 0;
	int abortarEsi = 0;

	char* ipCoordinador;
	int puertoCoordinador;
	char* mensajeCoordinador;
	int socketCoordinador;
	char* nombre;

	//leo puertos e ip del coordinador
	ipCoordinador = config_get_string_value(configuracion, "IP_COORDINADOR");
	puertoCoordinador = config_get_int_value(configuracion, "PUERTO_COORDINADOR");
	nombre = config_get_string_value(configuracion, "NOMBRE");

	// Valido que el archivo exista
	if ((fp = fopen(path, "r")) == NULL) {
		log_error(logESI, "El archivo '%s' no existe o es inaccesible", path);
		liberarMemoria();
		exit(EXIT_FAILURE);
	}

	while (filasLeidas < maxFilas || !abortarEsi) {
		// Espero un mensaje del planificador
		headerPlanificador = recibirHeader(socketPlanificador);

		// Cuando el planificador me habla avanzo
		if (headerPlanificador->id == PLANIFICADOR) {
			// Loggeo que me habló el planificador
			log_trace(logESI, "Recibí una señal de avanzar del coordinador");

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
							mensajeCoordinador = malloc(strlen("SET ") + strlen(parsed.argumentos.SET.clave) + 1 + strlen(parsed.argumentos.SET.valor) + 1);
							strcpy(mensajeCoordinador, "SET ");
							strcat(mensajeCoordinador, parsed.argumentos.SET.clave);
							strcat(mensajeCoordinador, " ");
							strcat(mensajeCoordinador, parsed.argumentos.SET.valor);
							mensajeCoordinador[strlen("GET ") + strlen(parsed.argumentos.SET.clave) + 1 + strlen(parsed.argumentos.SET.valor)] = '\0';
							break;

						case STORE:
							mensajeCoordinador = malloc(strlen("STORE ") + strlen(parsed.argumentos.STORE.clave) +1);
							strcpy(mensajeCoordinador, "STORE ");
							strcat(mensajeCoordinador, parsed.argumentos.STORE.clave);
							mensajeCoordinador[strlen("STORE ") + strlen(parsed.argumentos.STORE.clave)] = '\0';
							break;

						default:
							log_error(logESI, "No pude interpretar <%s>\n", line);
							liberarMemoria();
							exit(EXIT_FAILURE);
					}

					// Logueo el mensaje que le voy a mandar al coordinador
					log_trace(logESI, "Le comunico al coordinador la sentencia <%s>", mensajeCoordinador);

					// Me conecto con el coordinador
					socketCoordinador = clienteConectarComponente("ESI", "coordinador", puertoCoordinador, ipCoordinador);

					// Le mando el nombre
					enviarHeader(socketCoordinador, nombre, ESI);
					enviarMensaje(socketCoordinador, nombre);

					// Le mando la sentencia
					enviarHeader(socketCoordinador, mensajeCoordinador, ESI);
					enviarMensaje(socketCoordinador, mensajeCoordinador);
					free(mensajeCoordinador);

					// Espero una respuesta del coordinador
					headerCoordinador = recibirHeader(socketCoordinador);

					// Logueo la respuesta del coordinador
					switch (headerCoordinador->id) {
						case INSTANCIA_SENTENCIA_OK:
						case COORDINADOR_ESI_BLOQUEAR:
						case COORDINADOR_ESI_CREADO:

							log_trace(logESI, "El coordinador me respondió que salió todo OK");
							break;

						case COORDINADOR_ESI_BLOQUEADO:
							//Como se bloquea en la linea actual, no sigo avanzando y vuelvo a intentar esta linea
							fseek (fp, -1*strlen(line), SEEK_CUR );
							break;

						default:
							log_error(logESI, "El coordinador me respondió que hubo un error: %s", PROTOCOLO_MENSAJE[headerCoordinador->id]);
							abortarEsi = 1;
							// TODO ¿acá debería abortar o no le importa y espera que lo aborte el planificador?
							break;
					}

					filasLeidas++;
					free(headerCoordinador);
					destruir_operacion(parsed);
				} else {
					log_error(logESI, "La linea <%s> no es valida\n", line);
					liberarMemoria();
					exit(EXIT_FAILURE);
				}
			} else {
				log_error(logESI, "No se pudo leer la linea del script");
				break;
			}
		}
	}
	fclose(fp);
	liberarMemoria();
	if (line) free(line);
}
