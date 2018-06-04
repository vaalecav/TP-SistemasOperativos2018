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

sem_t mutex;

int main() {
	puts("Iniciando Planificador.");
	sem_init(&mutex, 0, 1);
	pthread_t hiloConexiones;
	pthread_t algoritmo;
	if (pthread_create(&hiloConexiones, NULL, (void *) tratarConexiones,
	NULL)) {

		fprintf(stderr, "Error creando el thread\n");
		return 1;
	}
	if (pthread_create(&algoritmo, NULL, (void *) algoritmoFifo,
	NULL)) {

		fprintf(stderr, "Error creando el thread\n");
		return 1;
	}
	iniciarConsola();
	if (pthread_cancel(hiloConexiones)) {

		fprintf(stderr, "Error cerrando el thread\n");
		return 2;

	}
	if (pthread_cancel(algoritmo)) {

			fprintf(stderr, "Error cerrando el thread\n");
			return 2;

		}
	cerrarConexiones();
	sem_destroy(&mutex);
	puts("El Planificador se ha finalizado correctamente.");
	return 0;
}

//=====================ALGORITMO FIFO ================================================

int tomarPrimero(int *array, int array_length) {
	int i;
	int retorno = array[0];
	for (i = 0; i < array_length - 1; i++)
		array[i] = array[i + 1];
	return retorno;
}

void quitarReady(int socket){
	int i = 0;
	while (colaReady[i] != socket){
		i++;
	}
	remove_element(colaReady, i, numeroClientes);
}

void algoritmoFifo() {
	int esiToRun;
	char* mensaje;
	while (1) {
		if (ejecutar == 1) {
			sem_wait(&mutex);
			esiToRun = tomarPrimero(colaReady, numeroClientes);
			numeroEnReady--;
			//TODO enviar mensaje al ESI para indicarle que ejecute;

			mensaje="ejecutar";
			enviarHeader(esiToRun, mensaje, PLANIFICADOR);
			enviarMensaje(esiToRun, mensaje);

			sleep(10);
			//TODO recibir respuesta del ESI, vuelvo a encolarlo;



			colaReady[numeroClientes-1] = esiToRun;
			numeroEnReady++;
			sem_post(&mutex);
		} else {
		}
	}
}

//=====================FUNCIONES DE MANEJO DE ESI=====================================

void remove_element(int *array, int index, int array_length) {
	int i;
	for (i = index; i < array_length - 1; i++)
		array[i] = array[i + 1];
}

void tratarConexiones() {
	fd_set descriptoresLectura;
	int i;
	struct timeval timeout;
	int resultado;

	char ip[16];
	int puerto;
	int maxConexiones;

	//Leo puertos e ips de archivo de configuracion
	leerConfiguracion("PUERTO:%d", &puerto);
	leerConfiguracion("IP:%s", &ip);
	leerConfiguracion("MAX_CONEX:%d", &maxConexiones);

	socketServer = socketServidor(puerto, ip, maxConexiones); //Levanto el servidor.
	/*socketCliente[numeroClientes] = servidorConectarComponente(&socketServer, "planificador",
	 "esi"); //Espero al primer ESI.
	 numeroClientes++;
	 printf("El cliente %d acaba de ingresar a nuestro servidor\n", socketCliente[0]);*/

	while (1) {
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&descriptoresLectura);
		FD_SET(socketServer, &descriptoresLectura);
		for (i = 0; i < numeroClientes; i++) {
			FD_SET(socketCliente[i], &descriptoresLectura);
		}

		resultado = select(100, &descriptoresLectura, NULL, NULL, &timeout);

		if (resultado != 0) {
			/* Se tratan los clientes */
			sem_wait(&mutex);
			for (i = 0; i < numeroClientes; i++) {
				if (FD_ISSET(socketCliente[i], &descriptoresLectura)) {
					//printf("El cliente %d se fue de nuestro servidor\n",socketCliente[i]);
					close(socketCliente[i]);
					quitarReady(socketCliente[i]);
					remove_element(socketCliente, i, numeroClientes);
					numeroClientes--;
					numeroEnReady--;
					//printf("CANTIDAD DE CLIENTES: %d\n", numeroClientes);
					/* Hay un error en la lectura. Posiblemente el cliente ha cerrado la conexión. Hacer aquí el tratamiento. En el ejemplo, se cierra el socket y se elimina del array de socketCliente[] */
				}
			}
			/* Se trata el socket servidor */
			if (FD_ISSET(socketServer, &descriptoresLectura)) {
				socketCliente[numeroClientes] = servidorConectarComponente(
						&socketServer, "planificador", "esi");
				colaReady[numeroClientes] = socketCliente[numeroClientes];
				//printf("El cliente %d acaba de ingresar a nuestro servidor\n",socketCliente[numeroClientes]);
				numeroClientes++;
				numeroEnReady++;
				//printf("CANTIDAD DE CLIENTES: %d\n", numeroClientes);
				/* Un nuevo cliente solicita conexión. Aceptarla aquí. En el ejemplo, se acepta la conexión, se mete el descriptor en socketCliente[] y se envía al cliente su posición en el array como número de cliente. */
			}
			sem_post(&mutex);
		}
	}
}

void cerrarConexiones() {
	while (numeroClientes != 0) {
		printf("El cliente %d fue finalizado por comando (quit).\n",
				socketCliente[numeroClientes - 1]);
		close(socketCliente[numeroClientes - 1]);
		numeroClientes--;
	}

	close(socketServer);
}

//=======================COMANDOS DE CONSOLA====================================

int cmdListaEsi() {
	register int i;
	printf("CANTIDAD DE ESI CONECTADOS: %d\n", numeroClientes);
	for (i = 0; i < numeroClientes; i++) {
		printf("ESI %d en posicion %d\n", socketCliente[i], i);
	}
	return 0;
}

int cmdColaReady() {
	register int i;
	printf("CANTIDAD DE ESI EN COLA DE READY: %d\n", numeroEnReady);
	for (i = 0; i < numeroEnReady; i++) {
		printf("ESI %d en posicion %d\n", colaReady[i], i);
	}
	return 0;
}


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
		if (strlen(comandos[i].cmd) < 5)
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
