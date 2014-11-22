
#ifndef KERNELCONFIG_H_
#define KERNELCONFIG_H_
#include "Kernel.h"
#include <stdbool.h>

bool cargarConfiguracionKernel(char *config_file, t_kernel* self, t_config *configKernel);

bool validarConfiguracionKernel(t_config* configKernel);

void destruirConfiguracionKernel(t_kernel *self, t_config *configKernel);

#endif /* KERNELCONFIG_H_ */
