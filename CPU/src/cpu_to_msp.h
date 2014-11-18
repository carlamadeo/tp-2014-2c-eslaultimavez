#ifndef CPU_TO_MPS_H_
#define CPU_TO_MPS_H_

#include "CPU.h"

t_socket_client* cpuConectarConMPS(t_CPU* self);
void cpuRealizarHandshakeConMSP(t_CPU* self);


#endif /* CPU_TO_MPS_H_ */
