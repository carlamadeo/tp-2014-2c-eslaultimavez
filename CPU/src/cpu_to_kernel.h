#ifndef CPU_TO_KERNEL_H_
#define CPU_TO_KERNEL_H_

#include "CPU.h"

int retardo, quantum, stack;
void cpuRealizarHandshakeConKernel();
t_socket_conexion* cpuConectarConKernel();

#endif /* CPU_TO_KERNEL_H_ */
