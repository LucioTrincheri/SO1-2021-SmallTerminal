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

int main(int argc, char *argv[]){
	while(1){
		printf(PROMPT);
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
				perror("Ah no manches wey la wea fome");
		  }
		  if (p == 0){ 	// Hijo 
				execv(datos.args[0],datos.args);
				_exit(-1);
		  } else { 			// Padre
				int status;
				wait(&status);
				
				if(WIFEXITED(status)){
					if(WEXITSTATUS(status) != 0){
						printf("Mi hijo se murio mal\n");
					} else {
						printf("No ampersand termino bien\n");
					}
				} else {
					printf("I cry everyday\n");
				}
				fflush(stdout);
				fflush(stdin);
		  }	
		} else {
			pid_t p;
			p = fork();
			
			if (p < 0) {
				perror("Ah no manches wey la wea fome");
		  }
		  if (p == 0){ 	// Hijo 
				pid_t k;
				k = fork(); // k en hijo = nieto
				if(k == 0){ // Nieto
					execv(datos.args[0],datos.args);
					fflush(stdout);
					fflush(stdin);
					_exit(-1);
				} else { // Hijo
					int statusHijo;
					waitpid(k, &statusHijo, 0);
					if(WIFEXITED(statusHijo)){
						if(WEXITSTATUS(statusHijo) != 0){
							printf("El comando %s con pid: %d esta muerto\n", datos.args[0] ,k);
						}else{
							printf("El hijo trackeado termino 10 punto\n");
						}
					} else {
						printf("El comando %s est...DAFAQ!?\n", datos.args[0]);
					}
					fflush(stdout);
					fflush(stdin);
					_exit(0);
					fflush(stdout);
					fflush(stdin);
				}
		  } // Padre no hace nada, lopeea
		}
	}
  return 0;
}

// Ver por que cuando llamo a mi nieto mi hijo llega a decir que ampersand bien
// Limpiar y funciones
