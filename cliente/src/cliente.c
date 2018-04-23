/*
 ** Socket de cliente
 */
#include "cliente.h"

int main() {
	int socketCli;
	char* texto;
	int cantidadDeDatos;

	socketCli = socketCliente(PUERTO, IP);
	texto = readline("Se conecto, escribir el mensaje: ");
	
	cantidadDeDatos = strlen(texto);
	enviarHeader(socketCli, cantidadDeDatos);
	enviarInformacion(socketCli, texto, &cantidadDeDatos);
	
	printf("Se mandaron %d bytes\n", cantidadDeDatos);
	close(socketCli);
	free(texto);
	return 0;
}
