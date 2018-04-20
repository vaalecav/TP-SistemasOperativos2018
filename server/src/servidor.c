/*
 * servidor.c
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */
#include "servidor.h"

//Crear socket
void crearSocket(){
	//Funcion que crea el socket.
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	//comprobacion de errores del socket
	if(sockfd == -1){
		perror("Socket error");
		exit(1);
	}
	puts("Socket creado");

	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(MYPORT);
	server.sin_addr.s_addr = inet_addr(IP);
}

//Bind
void bindSocket(){
	//Realizo bind, compruebo error
	if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1){
		perror("Bind error");
		exit(1);
	}
	puts("Bind realizado");
}

//Listen
void listenSocket(){
	//Escucho, y comprobacion errores
	if(listen(sockfd , MAX_CONEX) == -1){
		perror("Listen error");
		exit(1);
	}
	puts("Escuchando nuevas conexiones...");
}

//Aceptar conexion
void acceptSocket(){
	//Ciclo de accept, bloquea el proceso hasta que cliente se concete
	int c = sizeof(struct sockaddr_in);
	client_socket = accept(sockfd, (struct sockaddr *)&client,(socklen_t *) &c);

	if(client_socket == -1){
		perror("Accept error");
		exit(1);
	}
	puts("Conexion aceptada");
}

void enviarHeaderCliente(size_t tamanioMensaje){
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	header->largo = tamanioMensaje;
	header->id = 69;
	if(send(sockfd, header, sizeof(ContentHeader), 0) < 0){
		puts("Error en enviar header");
		exit(1);
	}
	puts("Header enviado");
}

void enviarMensajeCliente(char* mensaje){
	if(send(sockfd, mensaje, sizeof(mensaje), 0) < 0){
		puts("Error en enviar mensaje");
		exit(1);
	}
	puts("Mensaje enviado");
}

int recibirHeader(){
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	int recibido;
	int largo;

	recibido = recv(client_socket, header, sizeof(ContentHeader), 0);
	if (recibido < 0) {
		puts("Error en recibir mensaje");
		exit(1);
	} else if (recibido == 0) {
		puts("Cliente desconectado");
		close(client_socket);
		free(header);
		exit(1);
	}

	if (write(client_socket, "Mensaje recibido", 16) < 0){
		puts("Error write socket");
		exit(1);
	}

	// Tambien tiene que ver que hacer con el ID (todavia no esta hecho)
	largo = header->largo;
	printf("Recibi el header que tiene el largo: %d\n", header->largo);

	free(header);
	return largo;
}

void recibirMensaje(int tamanioMensaje){
	char *buffer;
	int recibido;

	buffer = (char*) malloc(tamanioMensaje + 1);

	recibido = recv(client_socket, buffer, tamanioMensaje, 0);
	if(recibido < 0){
		puts("Error en recibir mensaje");
		exit(1);
	} else if (recibido == 0){
		puts("Cliente desconectado");
		close(client_socket);
		free(buffer);
		exit(1);
	}

	buffer[tamanioMensaje] = '\0';
	printf("El mensaje recibido es: %s\n", buffer);

	free(buffer);

	if (write(client_socket, "Mensaje recibido", 16) < 0) {
		puts("Error write socket");
		exit(1);
	}
}

int main(void){
	size_t tamanioMensaje;

	crearSocket();
	bindSocket();
	listenSocket();
	acceptSocket();
	tamanioMensaje = recibirHeader();
	recibirMensaje(tamanioMensaje);

	close(sockfd);
	return 0;
}

