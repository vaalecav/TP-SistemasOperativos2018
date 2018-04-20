/*
 ** Socket de cliente
 */
#include "cliente.h"

int socketCliente(char* puerto, char* ip) {
	int socketDelServidor;
	struct addrinfo direccionDestino;
	struct addrinfo *informacionServidor;

	// Definiendo el destino
	memset(&direccionDestino, 0, sizeof(direccionDestino));
	direccionDestino.ai_family = AF_INET;    // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	direccionDestino.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(ip, puerto, &direccionDestino, &informacionServidor);  // Carga en server_info los datos de la conexion

	 // Creo el socket
	 if ((socketDelServidor = socket(informacionServidor->ai_family, informacionServidor->ai_socktype, informacionServidor->ai_protocol)) == -1) {
		 perror("socket");
		 exit(1);
	 }

	 if (connect(socketDelServidor, informacionServidor->ai_addr, informacionServidor->ai_addrlen) == -1) {
		 perror("connect");
		 exit(1);
	 }


	 freeaddrinfo(informacionServidor);  // No lo necesitamos mas

	 return socketDelServidor;
}

int enviarInformacion(int socket, void *texto, int *bytesAMandar) {
	int totalEnviados = 0; // cuántos bytes se mandan ahora
	int bytesRestantes = *bytesAMandar; // cuántos se han quedado pendientes de antes, lo asigno a una variable local

	int bytesEnviados;
	while (totalEnviados < *bytesAMandar) {
		bytesEnviados = send(socket, texto + totalEnviados, bytesRestantes, 0);
		if (bytesEnviados == -1) { break; }
		totalEnviados += bytesEnviados;
		bytesRestantes -= bytesEnviados;
	}

	*bytesAMandar = totalEnviados; // devuelve aquí la cantidad que se termino por mandar, se deberían haber mandado todos
	return bytesEnviados == -1 ? -1 : 0; // devuelve -1 si hay fallo, 0 en otro caso
}

void enviarHeader(int socketCliente, int tamanioMensaje){
	int cantidadDeDatos;

	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	header->largo = tamanioMensaje;
	header->id = 69;

	cantidadDeDatos = sizeof(ContentHeader);
	enviarInformacion(socketCliente, header, &cantidadDeDatos);
	puts("Header enviado");
}

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
