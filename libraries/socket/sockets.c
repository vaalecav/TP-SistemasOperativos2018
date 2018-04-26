/*
 ============================================================================
 Name        : sockets.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "sockets.h"

//Cliente
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

//Servidor
//Crear socket, devuelvo un socket que se conecto
int socketServidor(int puerto, char* ip){
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

int enviarHeader(int socketDestino, char* mensaje){
	int tamanioMensaje = strlen(mensaje);
	int tamanioHeader;
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));

	header->largo = tamanioMensaje;
	header->id = 69;
	tamanioHeader = sizeof(ContentHeader);

	if(enviarInformacion(socketDestino, header, &tamanioHeader) < 0){
		puts("Error en enviar header");
		exit(1);
	}
	puts("Header enviado");

	return 1;
}

int enviarMensaje(int miSocket, char* mensaje){
	int tamanioMensaje = strlen(mensaje);
	if(enviarInformacion(miSocket, mensaje, &tamanioMensaje) < 0){
		puts("Error en enviar mensaje");
		exit(1);
	}
	puts("Mensaje enviado");

	return 1;
}

int recibirHeader(int socketEmisor){
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	int recibido;
	int largo;

	recibido = recv(socketEmisor, header, sizeof(ContentHeader), 0);
	if (recibido < 0) {
		puts("Error en recibir mensaje");
		exit(1);
	} else if (recibido == 0) {
		puts("Socket emisor desconectado");
		close(socketEmisor);
		free(header);
		exit(1);
	}

	if (write(socketEmisor, "Mensaje recibido", 16) < 0){
		puts("Error write socket");
		exit(1);
	}

	// Tambien tiene que ver que hacer con el ID (todavia no esta hecho) chequeo si es ese id
	largo = header->largo;
	printf("Recibi el header que tiene el largo: %d\n", header->largo);

	free(header);
	return largo;
}

void recibirMensaje(int socketEmisor, int tamanioMensaje){
	char *buffer;
	int recibido;

	buffer = (char*) malloc(tamanioMensaje + 1);

	recibido = recv(socketEmisor, buffer, tamanioMensaje, 0);
	if(recibido < 0){
		puts("Error en recibir mensaje");
		exit(1);
	} else if (recibido == 0){
		puts("Socket Emisor desconectado");
		close(socketEmisor);
		free(buffer);
		exit(1);
	}

	buffer[tamanioMensaje] = '\0';
	printf("El mensaje recibido es: %s\n", buffer);

	free(buffer);

	if (write(socketEmisor, "Mensaje recibido", 16) < 0) {
		puts("Error write socket");
		exit(1);
	}
}
