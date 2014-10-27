#ifndef MSP_H_
#define MSP_H_

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

#include "Consola.h"
#include "ConfigMSP.h"
#include "Kernel.h"
#include "CPU.h"

sem_t mutex;
t_list* lista_procesos;
t_list* cola_paquetes;

t_socket *socketKernel;
t_socket *socketCpus;
struct sockaddr_in direccionCliente;
struct sockaddr_in kernelDireccion;
struct sockaddr_in cpuDireccion;

pthread_t mspHilo;
pthread_t* mspHiloCpus; // se tiene un vector de los hilos que hay que ir sacando/poniendo dependiendo de las cpus
pthread_t mspHiloKernel;
pthread_t mspConsolaHilo;

void mspLanzarhiloMSPCONSOLA();
int mspLanzarhilo();

#endif /* MSP_H_ */
