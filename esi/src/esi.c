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
	int socketPlanificador;

	// Leo el Archivo de Configuracion //
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	puertoPlanificador = config_get_int_value(configuracion,
			"PUERTO_PLANIFICADOR");
	ipPlanificador = config_get_string_value(configuracion, "IP_PLANIFICADOR");

	// Levanto la conexion con el Planificador //
	socketPlanificador = clienteConectarComponente("ESI", "planificador",
			puertoPlanificador, ipPlanificador);

	// Finalizo correctamente al ESI //
	cerrarEsi(socketPlanificador);
	puts("El ESI se ha finalizado correctamente.");

	return 0;
}

void cerrarEsi(int socketPlanificador) {
	close(socketPlanificador);
	config_destroy(configuracion);
}
