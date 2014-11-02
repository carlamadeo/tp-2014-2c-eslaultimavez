#ifndef KERNEL_H_
#define KERNEL_H_

#include <semaphore.h>
#include "commons/socketInBigBang.h"



typedef struct{
	u_int32_t baseDelSegmento;
	u_int32_t pid;
}t_envio_numMSP;

sem_t mutex;


// Funciones propias de la MSP_TO_KERNEL
void* mspLanzarHiloKernel(t_socket  *socketKernel);


#endif /* KERNEL_H_ */
