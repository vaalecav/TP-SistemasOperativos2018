/*
 ** Socket de cliente
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <string.h>
 #include <netdb.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>

 int main(int puerto, char *ip) {
	 int socket;
	 struct sockaddr_in direccionDestino;

	 // Creo el socket
	 if ((socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		 perror("socket");
		 exit(1);
	 }

	 // Definiendo el destino
	 direccionDestino.sin_family = AF_INET;
	 direccionDestino.sin_port = htons(puerto);
	 direccionDestino.sin_addr = *((struct in_addr *)ip);
	 memset(&(direccionDestino.sin_zero), 8);

	 if (connect(socket, (struct sockaddr *)&direccionDestino, sizeof(struct direccionDestino)) == -1) {
		 perror("connect");
		 exit(1);
	 }

	 return socket;
 }
