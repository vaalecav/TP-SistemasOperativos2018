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

int main()
{
	puts("Iniciando ESI.");
	int socketPlanificador;
	char ipPlanificador[16];
	int puertoPlanificador;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO_PLANIFICADOR:%d", &puertoPlanificador);
	leerConfiguracion("IP_PLANIFICADOR:%s", &ipPlanificador);
	socketPlanificador = clienteConectarComponente("ESI", "planificador",
			puertoPlanificador, ipPlanificador);

	parsearScript();

	sleep(20);

	close(socketPlanificador);

	puts("El ESI se ha finalizado correctamente.");
	return 0;
}

void parsearScript()
{
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("parsi/ejemplo/script.esi", "r");
	if (fp == NULL)
	{
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		t_esi_operacion parsed = parse(line);

		if (parsed.valido)
		{
			switch (parsed.keyword)
			{
			case GET:
				printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
				break;
			case SET:
				printf("SET\tclave: <%s>\tvalor: <%s>\n",
						parsed.argumentos.SET.clave,
						parsed.argumentos.SET.valor);
				break;
			case STORE:
				printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
				break;
			default:
				fprintf(stderr, "No pude interpretar <%s>\n", line);
				exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
		}
		else
		{
			fprintf(stderr, "La linea <%s> no es valida\n", line);
			exit(EXIT_FAILURE);
		}
	}

	fclose(fp);
	if (line)
		free(line);
}
