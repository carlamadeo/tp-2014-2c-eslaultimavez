/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU_Proceso.h"
#include "codigoESO.h"
#include "cpuMSP.h"
#include "cpuKernel.h"
#include "cpuConfig.h"
#include <sys/types.h>
#include <stdlib.h>

t_CPU *self;

int main(int argc, char** argv) {

	verificar_argumentosCPU(argc, argv);
	self = malloc(sizeof(t_CPU));

	t_config *configCPU;

	char *nombreLog = malloc(strlen("logCPU_.log") + sizeof(int) + 1);
	sprintf(nombreLog, "%s%d%s", "logCPU_", getpid(), ".log");
	self->loggerCPU = log_create(nombreLog, "CPU", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log
	free(nombreLog);

	if(!cargarConfiguracionCPU(argv[1], configCPU)){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	cpuConectarConMPS();
	cpuConectarConKernel();

	while(1){
		//1) Paso, recibir Quantum
		cpuRecibirQuantum();
		//2) Paso, recibir TCB
		cpuRecibirTCB();
		//3) Paso, Procesa TCB
		cpuProcesarTCB();
	}

	log_info(self->loggerCPU, "Se desconecto la CPU. Elimino todo");

	close(self->socketPlanificador->socket->descriptor);
	close(self->socketMSP->socket->descriptor);

	destruirConfiguracionCPU(configCPU);
	return EXIT_SUCCESS;
}


void verificar_argumentosCPU(int argc, char* argv[]){

	if( argc != 2 ){
		printf("Modo de empleo: ./CPU cpuConfig.cfg\n");
		perror("CPU no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}
