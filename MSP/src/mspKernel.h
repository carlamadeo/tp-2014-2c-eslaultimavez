#ifndef MSP_KERNEL_H_
#define MSP_KERNEL_H_

#include <semaphore.h>
#include "commons/socketInBigBang.h"



typedef struct{
	int pid;
	int tamanio;
}t_envio_numMSP;

sem_t mutex;


// Funciones propias de la MSP_TO_KERNEL
void* mspLanzarHiloKernel(t_socket  *socketKernel);

void crearSegmentoKernel(t_socket  *socketKernel, t_socket_paquete *paquete);

void destruirSegmentoKernel(socketKernel, paquete);

#endif /* KERNEL_H_ */
