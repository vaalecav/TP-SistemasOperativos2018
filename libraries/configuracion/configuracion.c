#include "configuracion.h"

int leerConfiguracion(char* clave, void* valor) {

   FILE *entrada;
   char linea[MAX_CHARS];
   int finalizo;
   char ch;

   if ((entrada = fopen("configuraciones/configuracion.txt", "rt")) == NULL) {
      perror("/configuraciones/configuracion.txt");
      exit(1);
   }

   /* Procesamos cada linea del archivo */
   finalizo = 0;
   while (fgets(linea, MAX_CHARS, entrada) != NULL && !finalizo) {
	   if (sscanf(linea, "%*[\n#]%c", &ch) == 1) {
		   /* Se ignoran las lineas en blanco y comentarios */
   	   } else if (sscanf(linea, clave, valor) == 1){
   		   /* Se encontro la clave y se copia en valor */
   		   finalizo = 1;
   	   }
   }

   fclose(entrada);
   return 1;
}
