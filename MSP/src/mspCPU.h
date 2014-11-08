#ifndef MSP_CPU_H_
#define MSP_CPU_H_


#include <commons/socketInBigBang.h>

typedef struct{
	t_socket_paquete *paquete;
	t_socket *socket;
}t_nodo_cola;


// Funciones propias de la MSP_TO_CPU
void *mspLanzarHiloCPU(void *arg);


#endif /* CPU_H_ */
