#ifndef MSP_H_
#define MSP_H_

#include "mspKernel.h"
#include "commons/collections/list.h"
#include "commons/socketInBigBang.h"
#include <arpa/inet.h>

//semaforos.... Preguntar como deben ir los semaforos
//pthread_rwlock_t rw_estructuras;
//pthread_rwlock_t rw_memoria;

typedef struct {
	t_socket* socketMSP;
	t_socket* socketClienteCPU;
	t_socket* socketClienteKernel;
} t_MSP;


pthread_t* mspHiloCpus; // se tiene un vector de los hilos que hay que ir sacando/poniendo dependiendo de las cpus
pthread_t* mspHiloKernel;
pthread_t mspConsolaHilo;
//pthread_t hiloConexion[100];//Ver el maximo de hilos si definimos en archivo configuracion o no.
t_list* listaCPUs;

void mspLanzarhiloMSPCONSOLA();
void mspLanzarConexiones();
void crearHilosConexiones();


#endif /* MSP_H_ */
