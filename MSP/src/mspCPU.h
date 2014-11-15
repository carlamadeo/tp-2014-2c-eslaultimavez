#ifndef MSP_CPU_H_
#define MSP_CPU_H_

#include "commons/socketInBigBang.h"

typedef struct{
	t_socket_paquete *paquete;
	t_socket *socket;
}t_nodo_cola;

typedef struct{
	int pid;
	//uint32_t direccionVirtual;
	int tamanio;
	char* leido;
}t_envio_leerMSPcpu;

// Funciones propias de la MSP_TO_CPU
void *mspLanzarHiloCPU(t_socket* socketCpus);


#endif /* CPU_H_ */
