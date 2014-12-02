/*
 * kernelConfig.c
 *
 *  Created on: 19/11/2014
 *      Author: utnso
 */

#include "kernelConfig.h"
#include "commons/string.h"
#include "commons/config.h"

const char configProperties[][25] = {"PUERTO_LOADER", "PUERTO_PLANIFICADOR", "IP_MSP", "PUERTO_MSP", "QUANTUM", "SYSCALLS", "TAMANIOSTACK"};

bool cargarConfiguracionKernel(char *config_file, t_kernel* self, t_config *configKernel){

	configKernel = config_create(config_file);

	if(!validarConfiguracionKernel(configKernel)) return false;

	self->loggerKernel = log_create("logKernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);
	self->loggerPlanificador = log_create("logPlanificador.log", "Planificador", 1, LOG_LEVEL_DEBUG);
	self->loggerLoader = log_create("logLoader.log", "Loader", 1, LOG_LEVEL_DEBUG);

	log_info(self->loggerKernel, "Cargando configuracion del Kernel..");

	//se obtiene los datos del archivo
	self->puertoLoader = config_get_int_value(configKernel, "PUERTO_LOADER");
	self->puertoPlanificador = config_get_int_value(configKernel, "PUERTO_PLANIFICADOR");
	self->ipMsp = string_duplicate(config_get_string_value(configKernel, "IP_MSP"));
	self->puertoMsp = config_get_int_value(configKernel, "PUERTO_MSP");
	self->quantum = config_get_int_value(configKernel, "QUANTUM");
	self->systemCalls  = string_duplicate(config_get_string_value(configKernel, "SYSCALLS"));
	self->tamanioStack = config_get_int_value(configKernel, "TAMANIOSTACK");

	return true;
}


bool validarConfiguracionKernel(t_config* configKernel){
	bool existen = true;
	int elements = sizeof(configProperties)/sizeof(configProperties[0]);
	int i;

	for(i = 0; i < elements; i++) {
		if(!config_has_property(configKernel, &configProperties[i])) {
			existen = false;
			break;
		}
	}

	return existen;
}

void destruirConfiguracionKernel(t_kernel *self, t_config *configKernel){

	free(cola_new);
	free(cola_ready);
	free(cola_exec);
	free(cola_block);
	free(cola_exit);


	free(cola_CPU_Disponibles);
	free(cola_CPU_Libres);
	free(listaDeCPUExec);
	free(listaDeCPULibres);
	free(listaCpu);
	free(listaSystemCall);

	free(self->tcbKernel);
	free(self->ipMsp);
	free(self->systemCalls);
	free(self);
	//config_destroy(configKernel);
}
