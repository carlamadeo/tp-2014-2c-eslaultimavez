/*
 ============================================================================
 Name        : MSP.c
 Author      : utnso
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "MSP.h"
#include "mspKernel.h"
#include "mspCPU.h"
#include "commons/protocolStructInBigBang.h"
#include "commons/log.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

t_sockets *sockets;
t_MSP *self;

int main(int argc, char *argv[]){

	self = malloc(sizeof(t_MSP));

	if (argc != 2){
		printf("Modo de empleo: ./MSP mspConfig.cfg\n");
		return EXIT_SUCCESS;
	}

	self->logMSP = log_create("logMSP.log", "MSP", 1, LOG_LEVEL_TRACE);

	log_info(self->logMSP, "Iniciando consola de la MSP...");

	if(!cargarConfiguracionMSP(argv[1])){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	//IMPORTANTE CONSOLAAAAAAAAA!!!!
	//	int mspConsolathreadNum = pthread_create(&mspConsolaHilo, NULL, &mspLanzarhiloConsola, NULL);
	//	if(mspConsolathreadNum) {
	//		log_error(self->logMSP, "Error - pthread_create() return code: %d\n", mspConsolathreadNum);
	//		exit(EXIT_FAILURE);
	//	}

	crearHilosConexiones();

	log_info(self->logMSP, "Finalizando la consola de la MSP...");
	//pthread_join(mspConsolathreadNum, NULL);
	destruirConfiguracionMSP();

	log_destroy(self->logMSP);
	return EXIT_SUCCESS;

}

void crearHilosConexiones(){

	log_info(self->logMSP, "Creando un hilo escucha...");
	sockets = malloc(sizeof(t_sockets));

	sockets->socketMSP = socket_createServer(self->puerto);

	if (sockets->socketMSP == NULL)
		log_error(self->logMSP, "MSP: Error al crear socket para el kernel.");

	if(!socket_listen(sockets->socketMSP))
		log_error(self->logMSP, "MSP: Error al poner a escuchar al Loader: %s", strerror(errno));

	else
		log_info(self->logMSP, "MSP: Escuchando conexiones entrantes en el puerto: %d", self->puerto);

	sockets->socketClienteKernel = socket_acceptClient(sockets->socketMSP);

	t_socket_paquete *paqueteKernel = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	socket_recvPaquete(sockets->socketClienteKernel, paqueteKernel);

	if(paqueteKernel->header.type == HANDSHAKE_KERNEL){
		int mspHiloKernelInt = pthread_create(&mspHiloKernel, NULL, (void *)mspLanzarHiloKernel, NULL);
		if(mspHiloKernelInt){
			log_error(self->logMSP, "Error - pthread_create() return code: %d\n", mspHiloKernelInt);
			exit(EXIT_FAILURE);
		}
	}

	int contadorCpu = 0;

	while(1){

		t_socket *socketClienteCPU = socket_acceptClient(sockets->socketMSP);

		t_socket_paquete *paqueteCPU = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
		socket_recvPaquete(socketClienteCPU, paqueteCPU);

		if(paqueteCPU->header.type == HANDSHAKE_CPU){
			int mspHiloCPUInt = pthread_create(&mspHiloCpus[contadorCpu], NULL, (void *)mspLanzarHiloCPU, socketClienteCPU);
			if(mspHiloCPUInt){
				log_error(self->logMSP, "Error - pthread_create() return code: %d\n", mspHiloCPUInt);
				exit(EXIT_FAILURE);
			}

			log_debug(self->logMSP, "MSP: cantidad de CPUs conectadas: %d", contadorCpu);
		}
		contadorCpu++;
	}//fin del while
}


