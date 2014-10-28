#ifndef KERNEL_H_
#define KERNEL_H_

#include <semaphore.h>




typedef struct{
	u_int32_t baseDelSegmento;
	u_int32_t pid;
}t_envio_num;

sem_t mutex;


// Funciones propias de la MSP_TO_KERNEL
void *mspLanzarHiloKernel(void *arg);


#endif /* KERNEL_H_ */
