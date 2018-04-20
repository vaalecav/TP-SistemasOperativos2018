/*
 * servidor.h
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */

#ifndef SERVIDOR_H_
#define SERVIDOR_H_

//bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <readline/readline.h>

//constantes
#define PUERTO 8000
#define IP "127.0.0.1"
#define MAX_CONEX 10

//estructuras
typedef struct {
  int id;
  int largo;
} __attribute__((packed)) ContentHeader;


//prototipos funciones
void crearSocket();
void bindSocket();
void listenSocket();
void acceptSocket();
void enviarHeaderCliente();
void enviarMensajeCliente();
void recibirMensajeCliente();


#endif /* SERVIDOR_H_ */
