/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"
#include "Loader.h"
#include "Planificador.h"
#include "boot.h"
#include "commons/config.h"
#include "kernelMSP.h"
#include "kernelConfig.h"
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char** argv) {

	verificar_argumentosKernel(argc, argv);
	t_kernel *self = malloc(sizeof(t_kernel));
	t_config *configKernel;

	if(!cargarConfiguracionKernel(argv[1], self, configKernel)){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	//printf("El ip cargado es %s\n", self->ipMsp);
	//printf("El puerto cargado es %d\n", self->puertoMsp);
	//log_info(self->loggerKernel, "Kernel: Comienza a ejecutar.");

	cola_new = list_create();
	cola_ready = list_create();
	cola_exit = list_create();
	cola_exec = list_create();
	cola_block = list_create();
	listaDeCPUExec = list_create();
	listaDeCPULibres = list_create();


	sem_init(&mutex_cpuLibre, 0, 1);
	sem_init(&mutex_cpuExec, 0, 1);
	sem_init(&mutex_exit, 0,1);
	sem_init(&mutex_exec, 0, 1);
	sem_init(&mutex_ready, 0, 1);
	sem_init(&mutex_new, 0, 1);
	sem_init(&mutex_block, 0, 1);
	sem_init(&cola_io, 0, 1);
	sem_init(&mutex_semaforos, 0, 1);
	sem_init(&sem_new, 0, 0);
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_exit, 0, 0);
	sem_init(&sem_cpuLibre, 0, 0);
	sem_init(&sem_cpuExec, 0, 0);


	//hace el boot y le manda a la msp el archivo de SystemCall
	hacer_conexion_con_msp(self);

	//El codigo se levanta de las system calls
	crearTCBKERNEL(self);
	//
	//	//Esto lo hace despues de Bootear
	//
	iretThread = pthread_create( &LoaderHilo, NULL, (void*) kernel_comenzar_Loader, self);
	if(iretThread) {
		printf(stderr,"Error - pthread_create() return code: %d\n", iretThread);
		exit(EXIT_FAILURE);
	}

	iretThread = pthread_create( &PlanificadorHilo, NULL, (void*) kernel_comenzar_Planificador, self);
	if(iretThread) {
		printf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
		exit(EXIT_FAILURE);
	}

	pthread_join(LoaderHilo, NULL);
	pthread_join(PlanificadorHilo, NULL);

	destruirConfiguracionKernel(self, configKernel);
	return EXIT_SUCCESS;
}

void verificar_argumentosKernel(int argc, char* argv[]){
	if( argc != 2 ){
		printf("Modo de empleo: ./Kernel mspKernel.cfg\n");
		exit (EXIT_FAILURE);
	}
}


void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa){
	sem_wait(&mutex_exit);//BLOQUEO LISTA DE EXIT
	list_add(cola_exit, programa);
	sem_post(&mutex_exit);
}



