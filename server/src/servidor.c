/*
 * servidor.c
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */
#include "servidor.h"

//Crear socket, devuelvo cliente
int socketServidor(char* puerto,char* ip){
	struct sockaddr_in server;
	struct sockaddr_in client;
	int socketServidor, socketCliente;
	//Funcion que crea el socket.
	socketServidor = socket(AF_INET,SOCK_STREAM,0);
	//comprobacion de errores del socket
	if(socketServidor == -1){
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
	if(bind(socketServidor, (struct sockaddr *) &server, sizeof(server)) == -1){
		perror("Bind error");
		exit(1);
	}
	puts("Bind realizado");


	//Listen
	//Escucho, y comprobacion errores
	if(listen(socketServidor , MAX_CONEX) == -1){
		perror("Listen error");
		exit(1);
	}
	puts("Escuchando nuevas conexiones...");


	//Aceptar conexion
	//Ciclo de accept, bloquea el proceso hasta que cliente se concete
	int c = sizeof(struct sockaddr_in);
	socketCliente = accept(socketServidor, (struct sockaddr *)&client,(socklen_t *) &c);

	if(socketCliente == -1){
		perror("Accept error");
		exit(1);
	}
	puts("Conexion aceptada");

	return socketCliente;
}

void enviarHeaderCliente(size_t tamanioMensaje){
	ContentHeader * header = (ContentHeader*) malloc(sizeof(ContentHeader));
	header->largo = tamanioMensaje;
	header->id = 69;
	if(send(socketServidor, header, sizeof(ContentHeader), 0) < 0){
		puts("Error en enviar header");
		exit(1);
	}
	puts("Header enviado");
}

void enviarMensajeCliente(char* mensaje){
	if(send(socketServidor, mensaje, sizeof(mensaje), 0) < 0){
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
	int socketCliente;

	socketCliente = socketServidor(PUERTO,IP);
	tamanioMensaje = recibirHeader(socketCliente);
	recibirMensaje(socketCliente, tamanioMensaje);

	close(socketServidor);
	return 0;
}

