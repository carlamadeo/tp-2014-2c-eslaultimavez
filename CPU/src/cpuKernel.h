#ifndef CPUKERNEL_H_
#define CPUKERNEL_H_

#include "CPU.h"

t_socket_client* conectarCPUConKernel(t_CPU* self);
void cpuRealizarHandshakeConKernel(t_CPU* self);


#endif /* CPUKERNEL_H_ */
