
#ifndef KERNELCONFIG_H_
#define KERNELCONFIG_H_

#include "commons/socketInBigBang.h"
#include "commons/log.h"
#include "commons/config.h"

#include <stdbool.h>

typedef struct {
	t_socket_client* socketMSP;
	t_socket* socketCPU;
	t_socket* socketConsola;
	t_log* loggerKernel;
	t_log* loggerPlanificador;
	t_log* loggerLoader;
	int puertoLoader;
	int puertoPlanificador;
	char* ipMsp;
	int puertoMsp;
	int quamtum;
	char* systemCalls;
	int tamanioStack;
} t_kernel;

bool cargarConfiguracionKernel(char *config_file, t_kernel* self, t_config *configKernel);

bool validarConfiguracionKernel(t_config* configKernel);

void destruirConfiguracionKernel(t_kernel *self, t_config *configKernel);

#endif /* KERNELCONFIG_H_ */
