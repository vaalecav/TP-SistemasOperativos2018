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

 int socketCliente(int puerto, char* ip) {
	 int socketDelCliente;
	 struct sockaddr_in direccionDestino;

	 // Creo el socket
	 if ((socketDelCliente = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		 perror("socket");
		 exit(1);
	 }

	 // Definiendo el destino
	 direccionDestino.sin_family = AF_INET;
	 direccionDestino.sin_port = htons(puerto);
	 direccionDestino.sin_addr = *((struct in_addr *)ip);
	 memset(&(direccionDestino.sin_zero), 0, sizeof(direccionDestino.sin_zero));

	 if (connect(socketDelCliente, (struct sockaddr *)&direccionDestino, sizeof(struct sockaddr)) == -1) {
		 perror("connect");
		 exit(1);
	 }

	 return socketDelCliente;
}

int enviarInformacion(int socket, char *texto, int *bytesAMandar) {
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
