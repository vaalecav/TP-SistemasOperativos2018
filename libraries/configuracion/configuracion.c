#include "configuracion.h"

int leerConfiguracion(char* clave, char** valor) {

   FILE *entrada;
   char linea[MAX_CHARS];
   int finalizo;
   char ch;
   puts("antes del fopen");

   if ((entrada = fopen("configuraciones/configuracion.txt", "rt")) == NULL) {
      perror("/configuraciones/configuracion.txt");
      exit(1);
   }

   /* Procesamos cada linea del archivo */
   finalizo = 0;
   while (fgets(linea, MAX_CHARS, entrada) != NULL && !finalizo) {


	   if (sscanf(linea, "%*[^\n#]%c", &ch) == 1) {
		   /* Se ignoran las lineas en blanco y comentarios */
   	   } else if (sscanf(linea, "IP_COORDINADOR:%s", (*valor)) == 1){
   		   /* Se encontro la clave y se copia en valor */
   		   finalizo = 1;
   	   }

	   puts("concha de tu madre");
   }

   fclose(entrada);
   return 1;
}
