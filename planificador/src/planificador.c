/*
 ============================================================================
 Name        : planificador.c
 Author      : Los Simuladores
 Version     : Alpha
 Copyright   : Todos los derechos reservados
 Description : Proceso Planificador
 ============================================================================
 */

#include "planificador.h"
// TODO LIST:
// 1- Crear Listas Enlazadas de Ready, Ejecucion (?), Bloqueados y Terminados //
// 2- Cuando se conecta un cliente, meterlo a cola de Ready. //
// 3- Cuando se desconecta un cliente, buscarlo en Ready, Ejecucion (?) o Bloqueados //
// 4- Cuando termina de ejecutar un cliente, desconectarlo nosotros y pasarlo a Terminados //
// 5- Crear un hilo de ejecucion que ejecute segun el algoritmo indicado //
// IMPORTANTE: El tipo de dato de las listas sera DATA, que es un struct de ESI //
// IMPORTANTE: Todavia falta hacer la funcion para recibir el largo del ESI cuando el mismo se conecta //

int main() {
	// Declaraciones Iniciales //
	puts("Iniciando Planificador.");
	pthread_t hiloConexiones;

	// Inicio el hilo que maneja las Conexiones //
	if (pthread_create(&hiloConexiones, NULL, (void *) tratarConexiones,
	NULL)) {
		fprintf(stderr, "Error creando el thread.\n");
		return 1;
	}

	// Inicio la Consola del Planificador //
	iniciarConsola();

	// Espero a que finalize el hilo que maneja las Conexiones //
	if (pthread_join(hiloConexiones, NULL)) {
		fprintf(stderr, "Error joining thread\n");
		return 2;

	}

	// Finalizo correctamente al Planificador //
	cerrarPlanificador();
	puts("El Planificador se ha finalizado correctamente.");

	return 0;
}

void tratarConexiones() {
	// Declaraciones Iniciales //
	char* ip;
	int puerto;
	int maxConex;
	int socketServer;

	// Leo el Archivo de Configuracion //
	configuracion = config_create(ARCHIVO_CONFIGURACION);
	puerto = config_get_int_value(configuracion, "PUERTO");
	ip = config_get_string_value(configuracion, "IP");
	maxConex = config_get_int_value(configuracion, "MAX_CONEX");

	// Nuevas Declaraciones //
	fd_set descriptoresLectura;
	int socketCliente[maxConex];
	int numeroClientes = 0;
	int i;
	struct timeval timeout;
	int nextIdEsi = 1;

	// Levanto el Servidor //
	socketServer = socketServidor(puerto, ip, maxConex);

	// Bucle del select() //
	while (done == 0) {
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* Se inicializa descriptoresLectura */
		FD_ZERO(&descriptoresLectura);

		/* Se añade para select() el socket servidor */
		FD_SET(socketServer, &descriptoresLectura);

		/* Se añaden para select() los sockets con los clientes ya conectados */
		for (i = 0; i < numeroClientes; i++)
			FD_SET(socketCliente[i], &descriptoresLectura);

		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 que decir: un nuevo cliente o un cliente ya conectado que envía un
		 mensaje */
		select(100, &descriptoresLectura, NULL, NULL, &timeout);

		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		for (i = 0; i < numeroClientes; i++) {
			if (FD_ISSET(socketCliente[i], &descriptoresLectura)) {
				/* Se indica que el cliente ha cerrado la conexión */
				close(socketCliente[i]);
				remove_element(socketCliente, i, numeroClientes);
				numeroClientes--;
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 admite */
		if (FD_ISSET(socketServer, &descriptoresLectura)) {
			/* Acepta la conexión con el cliente, guardándola en el array */
			socketCliente[numeroClientes] = servidorConectarComponente(
					&socketServer, "planificador", "esi");
			numeroClientes++;

			/* Si se ha superado el maximo de clientes, se cierra la conexión,
			 se deja como estaba y se vuelve. */
			if (numeroClientes > maxConex) {
				enviarHeader(socketCliente[numeroClientes - 1], "", 0);
				close(socketCliente[numeroClientes - 1]);
				numeroClientes--;
			} else {
				/* Envía su número de id al cliente */
				enviarHeader(socketCliente[numeroClientes - 1], "", nextIdEsi);

				// TODO RECIBIR DEL ESI SU CANTIDAD DE LINEAS;

				/* Aumento el ID para el proximo ESI */
				nextIdEsi++;
			}
		}
	}

	// Finalizo cualquier conexion restante //
	while (numeroClientes > 0) {
		printf("El cliente %d fue finalizado por comando (quit).\n",
				socketCliente[numeroClientes - 1]);
		close(socketCliente[numeroClientes - 1]);
		numeroClientes--;
	}

	close(socketServer);
}

void cerrarPlanificador() {
	config_destroy(configuracion);
}

int dameMaximo(int *tabla, int n) {
	int i, max = 0;
	for (i = 0; i < n; i++) {
		if (tabla[i] > max)
			max = tabla[i];
	}
	return max;
}

void remove_element(int *array, int index, int array_length) {
	int i;
	for (i = index; i < array_length - 1; i++)
		array[i] = array[i + 1];
}

//=======================COMANDOS DE CONSOLA====================================

int cmdQuit() {
	done = 1;
	return 0;
}

int cmdPause() {
	ejecutar = 0;
	puts("Se ha pausado la ejecucion de ESIs.");
	return 0;
}

int cmdContinue() {
	ejecutar = 1;
	puts("Se ha reanudado la ejecucion de ESIs.");
	return 0;
}

int cmdHelp() {
	register int i;
	puts("Comando:\t\t\tDescripcion:");
	for (i = 0; comandos[i].cmd; i++) {
		if (strlen(comandos[i].cmd) < 7)
			printf("%s\t\t\t\t%s\n", comandos[i].cmd, comandos[i].info);
		else
			printf("%s\t\t\t%s\n", comandos[i].cmd, comandos[i].info);
	}
	return 0;
}

//=====================FUNCIONES DE CONSOLA=====================================

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

COMANDO *punteroComando(int posicion) {
	return (&comandos[posicion]);
}

int ejecutarSinParametros(COMANDO *comando) {
	return ((*(comando->funcion))());
}

int ejecutarConParametros(char *parametros, COMANDO *comando) {
	return ((*(comando->funcion))(parametros));
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
			ejecutarConParametros(parametros, comando);
			free(parametros);
		}
	} else {
		printf("%s: La cantidad de parametros ingresados es incorrecta.\n",
				comandos[posicion].cmd);
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
