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
#include "mspConfig.h"
#include "mspConsola.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

t_sockets *socketsKernel;
t_MSP *self;
pthread_rwlock_t rw_memoria;

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

	pthread_rwlock_init(&rw_memoria, NULL);

	//pthread_join(mspConsolathreadNum, NULL);
	log_info(self->logMSP, "Finalizando la MSP...");

	destruirConfiguracionMSP();

	pthread_rwlock_destroy(&rw_memoria);

	return EXIT_SUCCESS;

}


void crearHilosConexiones(){

	log_info(self->logMSP, "Creando un hilo escucha...");
	socketsKernel = malloc(sizeof(t_sockets));

	socketsKernel->socketMSP = socket_createServer(self->puerto);

	if (socketsKernel->socketMSP == NULL){
		log_error(self->logMSP, "MSP: Error al crear socket para el Kernel.");

	}

	if(!socket_listen(socketsKernel->socketMSP)){
		log_error(self->logMSP, "MSP: Error al poner a escuchar a la MSP: %s", strerror(errno));

	}

	else
		log_info(self->logMSP, "MSP: Escuchando conexiones entrantes en el puerto: %d", self->puerto);


	int contadorCpu = 0;
	t_socket* unSocket;
	while(1){

		unSocket = socket_acceptClient(socketsKernel->socketMSP);

		t_socket_paquete *paqueteCliente = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
		socket_recvPaquete(unSocket, paqueteCliente);

		if(paqueteCliente->header.type == HANDSHAKE_CPU){
			int mspHiloCPUInt = pthread_create(&mspHiloCpus[contadorCpu], NULL, (void *)mspLanzarHiloCPU, unSocket);
			if(mspHiloCPUInt){
				log_error(self->logMSP, "Error - pthread_create() return code: %d\n", mspHiloCPUInt);

			}
			log_debug(self->logMSP, "MSP: cantidad de CPUs conectadas: %d", contadorCpu);

		}else{
			if(paqueteCliente->header.type == HANDSHAKE_KERNEL){
				int mspHiloKernelInt = pthread_create(&mspHiloKernel, NULL, (void *)mspLanzarHiloKernel, unSocket);
				if(mspHiloKernelInt){
					log_error(self->logMSP, "Error - pthread_create() return code: %d\n", mspHiloKernelInt);

				}
			}

		}
		contadorCpu++;
	}


	free(socketsKernel);
}
