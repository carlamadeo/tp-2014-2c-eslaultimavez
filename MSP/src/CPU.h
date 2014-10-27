#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/socketInBigBang.h>
#include <commons/protocolStructInBigBang.h>

typedef struct{
	t_socket_paquete *paquete;
	t_socket *socket;
}t_nodo_cola;


// Funciones propias de la MSP_TO_CPU
void *mspLanzarHiloCPU(void *arg);


#endif /* CPU_H_ */
