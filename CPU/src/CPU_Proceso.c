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
#include "ejecucion.h"
#include <sys/types.h>
#include <stdlib.h>


int main(int argc, char** argv) {

	verificar_argumentosCPU(argc, argv);
	t_CPU *self = malloc(sizeof(t_CPU));
	int error = 0;

	int valorCPU;
	t_config *configCPU;

	char *nombreLog = malloc(strlen("logCPU_.log") + sizeof(int) + 1);
	sprintf(nombreLog, "%s%d%s", "logCPU_", getpid(), ".log");
	self->loggerCPU = log_create(nombreLog, "CPU", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log
	free(nombreLog);


	if(!cargarConfiguracionCPU(self, argv[1], configCPU)){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	cpuConectarConMPS(self);
	cpuConectarConKernel(self);

	while(!error){

		if (!cpuRecibirQuantum(self) || !cpuRecibirTCB(self)){
			log_info(self->loggerCPU, "Finalizando CPU...");
			error = 1;
		}

		else
			error = cpuProcesarTCB(self);
	}

	log_info(self->loggerCPU, "Se desconecto la CPU. Elimino todo");

	close(self->socketPlanificador->socket->descriptor);
	close(self->socketMSP->socket->descriptor);


	free(self);
	return EXIT_SUCCESS;
}


void verificar_argumentosCPU(int argc, char* argv[]){

	if( argc != 2 ){
		printf("Modo de empleo: ./CPU cpuConfig.cfg\n");
		perror("CPU no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}

void printTCBCPU(t_TCB_CPU* unTCB){

	printf("TCB PID: %d \n", unTCB->pid);
	printf("TCB TID: %d \n", unTCB->tid);
	printf("TCB  KM: %d \n", unTCB->km);

	printf("Base Segmento:   %0.8p \n",unTCB->base_segmento_codigo);
	printf("Tamanio Segmento: %d \n",unTCB->tamanio_segmento_codigo);
	printf("Puntero a Instruccion: %0.8p \n",unTCB->puntero_instruccion);

	printf("Base Stack:   %0.8p \n",unTCB->base_stack);
	printf("Cursor Stack: %0.8p \n",unTCB->cursor_stack);

	printf("Regristros A: %d\n", unTCB->registro_de_programacion[0]);
	printf("Regristros B: %d\n", unTCB->registro_de_programacion[1]);
	printf("Regristros C: %d\n", unTCB->registro_de_programacion[2]);
	printf("Regristros D: %d\n", unTCB->registro_de_programacion[3]);
	printf("Regristros E: %d\n", unTCB->registro_de_programacion[4]);

}


t_registros_cpu* cpuInicializarRegistrosCPU(t_CPU* self, t_registros_cpu* registros){

	int i;
	registros->I = (uint32_t)self->tcb->pid;
	registros->K = (uint32_t)self->tcb->km;
	registros->M = self->tcb->base_segmento_codigo;
	registros->P = self->tcb->puntero_instruccion;
	registros->S = self->tcb->cursor_stack;
	registros->X = self->tcb->base_stack;

	for(i = 0; i < 5 ; i++)
		registros->registros_programacion[i] = self->tcb->registro_de_programacion[i];

	return registros;
}

