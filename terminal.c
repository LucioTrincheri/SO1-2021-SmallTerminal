#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<wait.h>
#define PROMPT "$ "
#define LARGO 1024

// Estructura que contiene los datos del comando pasado por la terminal
typedef struct _Datos{
	char* args[LARGO];
	int amps;
} Datos;

// Función encargada de reemplazar los espacios entre comandos y argumentos
// con caracteres NULL y apunta al principio de cada uno de estos con un puntero
// para no tener que copiar partes del comando innecesariamente.
Datos parsear(char buffer[]){
	Datos datos;
	datos.args[0] = buffer;
	datos.amps = 0;
	int cant = 1;
	for(int i = 0; buffer[i] != '\0'; i++){
		if(buffer[i] == '&'){
			datos.args[cant - 1] = NULL;
			datos.amps = 1;
			return datos;
		}
		if(buffer[i] == ' '){
			buffer[i] = '\0';
			datos.args[cant++] = &(buffer[i+1]);
		}
	}
	datos.args[cant] = NULL;
	return datos;
}

// Funcion encargada de trackear el estado de la ejecucion del comando
// pedido. A su vez muestra por la consola un mensaje de error sea necesario.
void tracking(Datos datos, pid_t pidHijo){
	
	// Con el Pid del proceso hijo a trackear, espero su ejecucion
	// y segun el estado de las banderas, muestro mensajes de error.
	int status;
	waitpid(pidHijo,&status,0);
	
	if(WIFEXITED(status)){
		if(WEXITSTATUS(status) != 0){
			printf("Comando %s con pid %d fallo\n",datos.args[0], pidHijo);
		}
	} else {
		printf("No se conoce el estado actual del hijo\n");
	}
}

int main(int argc, char *argv[]){
	// Loop de terminal, la cual sigue esperando comandos hasta
	// que se finalize el proceso o se mande un mensaje de exit
	while(1){
		printf(PROMPT);
		char buffer[LARGO] = {};
		// Checkear que -1 no sea \0 y arreglar el \n
		scanf("%1023[^\n]", buffer);
		// En caso que se exceda el largo maximo, limpiamos el buffer
		// Esto se podria mejorar notificando del error y no ejecutando
		// el comando ingresado.
		int c;
		while ((c = getchar()) != '\n' && c != EOF){}
		
		// Parseamos el comando leido por consola
		Datos datos = parsear(buffer);
		
		// Si el comando leido es exit, salimos del programa
		if(strcmp(datos.args[0], "exit") == 0){
			return 0;
		}
		
		// Segun si la ejecucion tiene que esperar al resultado
		// del comando (si posee & o no), realizamos la logica
		// de ejecucion de dos maneras distintas
		if(datos.amps == 0){
			// En el caso que no haya un &, el padre se encargada
			// de trackear el estado del hijo (que se encarga de
			// ejecutar el comando).
			pid_t pid_p_h;
			pid_p_h = fork();
			
			if (pid_p_h < 0) {
				perror("Fallo al realizar el fork");
			}
			if (pid_p_h == 0){ // Hijo 
				execv(datos.args[0],datos.args);
				_exit(-1);
			} else { // Padre
				tracking(datos, pid_p_h); // El padre espera que finalize el hijo con su pid.
			}	
		} else {
			// En el caso que si haya un &, la lógica es la siguiente:
			// Como el padre debe seguir esperando entrada por parte
			// del usuario, en este caso decidimos forkear por primera
			// vez, liberando al padre de tener que trackear al hijo y
			// encargarse de esperar la entrada del usuario. Debido a esto,
			// el hijo se encarga de forkear otra vez y tracker el estado 
			// de su nuevo hijo (comando). En este caso, el hijo 
			// actua como el padre de la lógica anterior.
			pid_t pid_p_h;
			pid_p_h = fork();
			if (pid_p_h < 0) {
				perror("Fallo al realizar el fork");
			}
			if (pid_p_h == 0){ 
				// El hijo en este caso forkea para trackear a su hijo (nieto padre)
				pid_t pid_h_n;
				pid_h_n = fork();
				if(pid_h_n == 0){ // Nieto ejecuta el comando pedido
					execv(datos.args[0],datos.args);
					_exit(-1);
				} else { // Hijo real
					tracking(datos, pid_h_n); // Trackea el estado del nieto
					printf("\n");
					printf(PROMPT); // Vuelvo a imprimir el PROMPT al finalizar
					fflush(stdout);
					_exit(0);
				}
			} // El padre termina el loop y vuelve a esperar entrada.
		}
	}
	return 0;
}
