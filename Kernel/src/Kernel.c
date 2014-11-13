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
#include "Planificador.h"
#include "Loader.h"

int main(int argc, char** argv) {

	verificar_argumentosKernel(argc, argv);
	char* config_file = argv[1];
	t_kernel* self = kernel_cargar_configuracion(config_file);

	self->loggerKernel = log_create("logKernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);
	self->loggerPlanificador = log_create("logKernel.log", "Planificador", 1, LOG_LEVEL_DEBUG);
	self->loggerLoader = log_create("logKernel.log", "Loader", 1, LOG_LEVEL_DEBUG);
	log_info((self->loggerKernel), "Kernel: Comienza a ejecutar.");

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
	log_info((self->loggerKernel), "Kernel: Boot completado con EXITO.");


	//Esto lo hace despues de Bootear

	iretThread = pthread_create( &LoaderHilo, NULL, (void*) kernel_comenzar_Loader, self);
		if(iretThread) {
			printf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
			exit(EXIT_FAILURE);
		}

//	iretThread = pthread_create( &PlanificadorHilo, NULL, (void*) kernel_comenzar_Planificador, self);
//	if(iretThread) {
//		printf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
//		exit(EXIT_FAILURE);
//	}


	pthread_join(LoaderHilo, NULL);
	//pthread_join(PlanificadorHilo, NULL);
	return EXIT_SUCCESS;
}

void verificar_argumentosKernel(int argc, char* argv[]){
	if( argc < 2 ){
		printf("Modo de empleo: ./Kernel mspKernel.cfg\n");
		perror("Kernel no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}


t_kernel* kernel_cargar_configuracion(char* config_file){
	t_kernel* self = malloc(sizeof(t_kernel));
	t_config* config = config_create(config_file);

	//se obtiene los datos del archivo
	self->puertoLoader = config_get_int_value(config, "PUERTO_LOADER");
	self->puertoPlanificador = config_get_int_value(config, "PUERTO_PLANIFICADOR");
	self->ipMsp = string_duplicate(config_get_string_value(config, "IP_MSP"));
	self->puertoMsp = config_get_int_value(config, "PUERTO_MSP");
	self->quamtum = string_duplicate(config_get_string_value(config, "QUANTUM"));
	self->systemCalls  = string_duplicate(config_get_string_value(config, "SYSCALLS"));
	self->tamanioStack = config_get_int_value(config, "TAMANIOSTACK");

	config_destroy(config);
	return self;
}



void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa){
	sem_wait(&mutex_exit);//BLOQUEO LISTA DE EXIT
	list_add(cola_exit, programa);
	sem_post(&mutex_exit);
}
