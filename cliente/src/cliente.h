/*
 * cliente.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_CLIENTE_H_
#define SRC_CLIENTE_H_

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

#define PUERTO "8000"
#define IP "127.0.0.1"

//estructuras
typedef struct {
  int id;
  int largo;
} __attribute__((packed)) ContentHeader;

//prototipos
int socketCliente(char*, char*);
int enviarInformacion(int, char*, int*);


#endif /* SRC_CLIENTE_H_ */
