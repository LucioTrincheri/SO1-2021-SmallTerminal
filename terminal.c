#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<wait.h>
#define PROMPT "$ "

typedef struct _Datos{
	char* args[512];
	int amps;
} Datos;

Datos parsear(char buffer[]){
	// Inicializamos la estructura y apuntamos los argumentos
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


void tracking(Datos datos, pid_t pidHijo){
	int status;
	waitpid(pidHijo,&status,0);
	
	if(WIFEXITED(status)){
		if(WEXITSTATUS(status) != 0){
			printf("Comando %s con pid %d fallo\n",datos.args[0], pidHijo);
		} else {
			printf("Comando %s con pid %d ha finalizado satisfactoriamente\n",datos.args[0], pidHijo);
		}
	} else {
		printf("No se conoce es estado actual del hijo\n");
	}
}

int main(int argc, char *argv[]){
	while(1){
		printf(PROMPT);
		//fflush(stdout);
		char buffer[1024] = {};
		// Checkear que -1 no sea \0 y arreglar el \n
		scanf("%1023[^\n]",buffer);
		int c;
		while ((c = getchar()) != '\n' && c != EOF){}
		Datos datos = parsear(buffer);
		if(strcmp(datos.args[0], "exit") == 0){
			return 0;
		}
		fflush(stdout);
		fflush(stdin);
		if(datos.amps == 0){
			pid_t p;
			p = fork();
			
			if (p < 0) {
				perror("Fallo al realizar el fork");
			}
			if (p == 0){ // Hijo 
				execv(datos.args[0],datos.args);
				_exit(-1);
			} else { // Padre
				tracking(datos, p); // El padre espera que finalize el hijo con su pid.
			}	
		} else {
			pid_t p;
			p = fork();
			if (p < 0) {
				perror("Fallo al realizar el fork");
			}
			if (p == 0){ // Hijo en este caso trackea a su hijo (nieto padre)
				pid_t k;
				k = fork();
				if(k == 0){ // Nieto ejecuta el comando pedido
					execv(datos.args[0],datos.args);
					_exit(-1);
				} else { // Hijo
					tracking(datos, k); // Trackea el estado del nieto
					printf(PROMPT);
					fflush(stdout);
					_exit(0);
				}
			} // El padre en este caso no hace nada, loopea
		}
	}
	return 0;
}

// Ver por que cuando llamo a mi nieto mi hijo llega a decir que ampersand bien
// Limpiar y funciones
