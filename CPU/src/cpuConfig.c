/*
 * cpuConfig.c
 *
 *  Created on: 19/11/2014
 *      Author: utnso
 */

#include "cpuConfig.h"

t_CPU *self;

const char configProperties[][25] = {"PUERTO_KERNEL", "PUERTO_MSP", "IP_KERNEL", "IP_MSP", "RETARDO"};

bool cargarConfiguracionCPU(char *config_file, t_config *configCPU){

	configCPU = config_create(config_file);

	if(!validarConfiguracionCPU(configCPU)) return false;

	//se obtiene los datos del archivo

	self->puertoPlanificador = config_get_int_value(configCPU, "PUERTO_KERNEL");
	self->puertoMSP = config_get_int_value(configCPU, "PUERTO_MSP");
	self->ipPlanificador= string_duplicate(config_get_string_value(configCPU, "IP_KERNEL"));
	self->ipMsp = string_duplicate(config_get_string_value(configCPU, "IP_MSP"));
	self->retardo = config_get_int_value(configCPU, "RETARDO");

	config_destroy(configCPU);
	return true;
}

bool validarConfiguracionCPU(t_config* configCPU){
	bool existen = true;
	int elements = sizeof(configProperties)/sizeof(configProperties[0]);
	int i;

	for(i = 0; i < elements; i++) {
		if(!config_has_property(configCPU, &configProperties[i])) {
			existen = false;
			break;
		}
	}

	return existen;
}

void destruirConfiguracionCPU(t_config *configCPU){
	free(self->ipMsp);
	free(self->ipPlanificador);
	free(self);
	config_destroy(configCPU);
}

