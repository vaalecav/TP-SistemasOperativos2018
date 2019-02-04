# TRABAJO PRÁCTICO SISTEMAS OPERATIVOS 2018 

## Breve descripción del proyecto

El trabajo consta de cuatro procesos base que se coordinan en conjunto para lograr la ejecución de los scripts otorgados por la cátedra.
Los Procesos ESI son el punto de entrada al Sistema. Estos correrán un script con instrucciones a ser ejecutadas. Sú función es la de interpretar de a una las líneas de un programa a medida que el Planificador se lo indique. Para resolver las distintas sentencias del programa a ser interpretado; deberá colaborar con el Coordinador para bloquear/liberar recursos o datos. El Coordinador es el proceso encargado de distribuir los pedidos de información del Sistema y gestionar las Instancias que son los procesos encargados del almacenamiento de los datos.

### Orden para ejecutar:

1) Coordinador
2) Planificador
3) Instancia
4) Esi

### Para usar los scripts .sh
Usar: 

$chmod +x path/yourscript.sh

si estas parado en el directorio, solamente: ./yourscript.sh
 
----

Ejemplo:

$chmod +x ./planificador.sh

Si los tenes en una carpeta entonces:

$ cd planificador
$ chmod ./planificador.sh

o

$chmod +x planificador/planificador.sh

### Comandos del Planificador:
- pausar: Pausa la ejecucion de ESIs.
- continuar: Reanuda la ejecucion de ESIs.
- colaTerminados:  Imprime en pantalla la cola de Terminados
- colaBloqueados: Imprime en pantalla la cola de Bloqueados.
- colaReady: Imprime en pantalla la cola de Ready
- listaClaves: Imprime la lista de Claves
		/*		{ "bloquear","Este comando aun no se ha desarrollado.", 2},
		 { "desbloquear","Este comando aun no se ha desarrollado.", 1},
		 { "listar","Este comando aun no se ha desarrollado.", 1},
		 { "kill","Este comando aun no se ha desarrollado.", 1},
		 { "status","Este comando aun no se ha desarrollado.", 1},
		 { "deadlock","Este comando aun no se ha desarrollado.", 0},*/
- help: Imprime los comandos disponibles.
- quit: Finaliza al Planificador.
