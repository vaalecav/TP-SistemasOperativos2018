/*
 * sockets.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <arpa/inet.h>


#define MAX_CONEX 10
//estructuras
typedef struct {
  int id;
  int largo;
} __attribute__((packed)) ContentHeader;

//prototipos
int conectarClienteA(int, char*);
int enviarInformacion(int, void*, int*);
int socketServidor(int, char*);
int enviarHeader(int, char*);
int enviarMensaje(int, char*);
void recibirMensaje(int, int, char**);
int recibirHeader(int);
int servidorConectarComponente(int*, char*, char*);
int clienteConectarComponente(char*, char*, int, char*);

#endif /* SOCKETS_H_ */
