#ifndef KERNEL_H_
#define KERNEL_H_

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
	u_int32_t baseDelSegmento;
	u_int32_t pid;
}t_envio_num;

sem_t mutex;


// Funciones propias de la MSP_TO_KERNEL
void *mspLanzarHiloKernel(void *arg);


#endif /* KERNEL_H_ */
