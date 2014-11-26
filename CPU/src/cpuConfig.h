/*
 * cpuConfig.h
 *
 *  Created on: 19/11/2014
 *      Author: utnso
 */

#ifndef CPUCONFIG_H_
#define CPUCONFIG_H_

#include "CPU_Proceso.h"

bool cargarConfiguracionCPU(char *config_file, t_CPU* self, t_config *configCPU);
bool validarConfiguracionCPU(t_config* configCPU);
void destruirConfiguracionCPU(t_CPU *self, t_config *configCPU);
#endif /* CPUCONFIG_H_ */
