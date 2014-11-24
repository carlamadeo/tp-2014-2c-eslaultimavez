#ifndef MSP_H_
#define MSP_H_

#include "commons/log.h"
#include "commons/collections/list.h"
#include "commons/socketInBigBang.h"
#include <arpa/inet.h>
#include <stdint.h>

typedef struct{
	t_socket* socketMSP;
	t_socket* socketClienteCPU;
	t_socket* socketClienteKernel;
}t_sockets;

typedef struct{
	t_log *logMSP;
	int puerto;
	uint16_t modoSustitucionPaginas;
	double cantidadMemoriaPrincipal;
	double cantidadMemoriaSecundaria;
	char *memoria;
	t_list *programas;
	t_list *marcosLibres;
	t_list *marcosOcupados;
}t_MSP;

pthread_t *mspHiloCpus;
pthread_t *mspHiloKernel;
pthread_t *mspConsolaHilo;

void mspLanzarhiloMSPCONSOLA();
void mspLanzarConexiones();
void crearHilosConexiones();


#endif /* MSP_H_ */
