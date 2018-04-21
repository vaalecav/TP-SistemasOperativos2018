/*
 * servidor.c
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */
#include "servidor.h"

//Crear socket, devuelvo un socket que se conecto
int socketServidor(int puerto,char* ip){
	struct sockaddr_in server;
	struct sockaddr_in conexion;
	int miSocket, socketConectado;
	//Funcion que crea el socket.
	miSocket = socket(AF_INET,SOCK_STREAM,0);
	//comprobacion de errores del socket
	if (miSocket == -1) {
		perror("Socket error");
		exit(1);
	}
	puts("Socket creado");

	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(puerto);
	server.sin_addr.s_addr = inet_addr(ip);

	//Bind
	//Realizo bind, compruebo error
	if(bind(miSocket, (struct sockaddr *) &server, sizeof(server)) == -1){
		perror("Bind error");
		exit(1);
	}
	puts("Bind realizado");


	//Listen
	//Escucho, y comprobacion errores
	if(listen(miSocket , MAX_CONEX) == -1){
		perror("Listen error");
		exit(1);
	}
	puts("Escuchando nuevas conexiones...");


	//Aceptar conexion
	//Ciclo de accept, bloquea el proceso hasta que cliente se concete
	int c = sizeof(struct sockaddr_in);
	socketConectado = accept(miSocket, (struct sockaddr *)&conexion,(socklen_t *) &c);

	if (socketConectado == -1) {
		perror("Accept error");
		exit(1);
	}
	puts("Conexion aceptada");

	return socketConectado;
}

void enviarHeaderCliente(int miSocket, size_t tamanioMensaje){
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	header->largo = tamanioMensaje;
	header->id = 69;
	if(send(miSocket, header, sizeof(ContentHeader), 0) < 0){
		puts("Error en enviar header");
		exit(1);
	}
	puts("Header enviado");
}

void enviarMensajeCliente(int miSocket, char* mensaje){
	if(send(miSocket, mensaje, sizeof(mensaje), 0) < 0){
		puts("Error en enviar mensaje");
		exit(1);
	}
	puts("Mensaje enviado");
}

int recibirHeader(int socketCliente){
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	int recibido;
	int largo;

	recibido = recv(socketCliente, header, sizeof(ContentHeader), 0);
	if (recibido < 0) {
		puts("Error en recibir mensaje");
		exit(1);
	} else if (recibido == 0) {
		puts("Cliente desconectado");
		close(socketCliente);
		free(header);
		exit(1);
	}

	if (write(socketCliente, "Mensaje recibido", 16) < 0){
		puts("Error write socket");
		exit(1);
	}

	// Tambien tiene que ver que hacer con el ID (todavia no esta hecho)
	largo = header->largo;
	printf("Recibi el header que tiene el largo: %d\n", header->largo);

	free(header);
	return largo;
}

void recibirMensaje(int socketCliente, int tamanioMensaje){
	char *buffer;
	int recibido;

	buffer = (char*) malloc(tamanioMensaje + 1);

	recibido = recv(socketCliente, buffer, tamanioMensaje, 0);
	if(recibido < 0){
		puts("Error en recibir mensaje");
		exit(1);
	} else if (recibido == 0){
		puts("Cliente desconectado");
		close(socketCliente);
		free(buffer);
		exit(1);
	}

	buffer[tamanioMensaje] = '\0';
	printf("El mensaje recibido es: %s\n", buffer);

	free(buffer);

	if (write(socketCliente, "Mensaje recibido", 16) < 0) {
		puts("Error write socket");
		exit(1);
	}
}


int main(void){
	size_t tamanioMensaje;
	int miSocket;

	miSocket = socketServidor(PUERTO,IP);
	tamanioMensaje = recibirHeader(miSocket);
	recibirMensaje(miSocket, tamanioMensaje);

	close(miSocket);
	return 0;
}

