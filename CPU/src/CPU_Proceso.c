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
	t_ServiciosAlPlanificador* serviciosAlPlanificador = malloc(sizeof(t_ServiciosAlPlanificador));

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

	while(1){
		//1) Paso, recibir Quantum
		cpuRecibirQuantum(self);
		//2) Paso, recibir TCB
		cpuRecibirTCB(self);
		//3) Paso, Procesa TCB
		valorCPU = cpuProcesarTCB(self, serviciosAlPlanificador);

		log_info(self->loggerCPU, "CPU: valorCPU == %d",valorCPU);

		switch(valorCPU){

		case SIN_ERRORES:
			cpuEnviarPaqueteAPlanificador(self, CAMBIO_DE_CONTEXTO);
			log_info(self->loggerCPU, "CPU: envia un CAMBIO_DE_CONTEXTO");
			break;
		case INTERRUPCION:
			cpuEnviarPaqueteAPlanificador(self, INTERRUPCION);
			log_info(self->loggerCPU, "CPU: envia una INTERRUPCION");
			break;
		case ENTRADA_ESTANDAR:
			cpuEnviarPaqueteAPlanificador(self, ENTRADA_ESTANDAR);
			log_info(self->loggerCPU, "CPU: envia una ENTRADA_ESTANDAR");
			break;
		case SALIDA_ESTANDAR:
			cpuEnviarPaqueteAPlanificador(self, SALIDA_ESTANDAR);
			log_info(self->loggerCPU, "CPU: envia una SALIDA_ESTANDAR");
			break;
		case CREAR_HILO:
			cpuEnviarPaqueteAPlanificador(self, CREAR_HILO);
			log_info(self->loggerCPU, "CPU: envia un CREAR_HILO");
			break;
		case JOIN_HILO:
			cpuEnviarPaqueteAPlanificador(self, JOIN_HILO);
			log_info(self->loggerCPU, "CPU: envia un JOIN_HILO");
			break;
		case BLOK_HILO:
			cpuEnviarPaqueteAPlanificador(self, BLOK_HILO);
			log_info(self->loggerCPU, "CPU: envia un BLOK_HILO");
			break;
		case WAKE_HILO:
			cpuEnviarPaqueteAPlanificador(self, WAKE_HILO);
			log_info(self->loggerCPU, "CPU: envia un WAKE_HILO");
			break;
		default:
			cpuEnviarPaqueteAPlanificador(self, MENSAJE_DE_ERROR);
			log_error(self->loggerCPU, "CPU: envia un MENSAJE_DE_ERROR");
			return MENSAJE_DE_ERROR;
		}

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
