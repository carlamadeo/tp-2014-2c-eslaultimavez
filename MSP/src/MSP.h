#ifndef MSP_H_
#define MSP_H_

#include "mspKernel.h"
#include "commons/collections/list.h"
#include "commons/socketInBigBang.h"
#include <arpa/inet.h>



t_socket *socketKernel;
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
