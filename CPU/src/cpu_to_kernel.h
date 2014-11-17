#ifndef CPU_TO_KERNEL_H_
#define CPU_TO_KERNEL_H_

#include "CPU.h"

t_socket_client* conectarCPUConKernel(t_CPU* self);
void cpuRealizarHandshakeConKernel(t_CPU* self);


#endif /* CPU_TO_KERNEL_H_ */
