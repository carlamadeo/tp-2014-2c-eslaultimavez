/*
 ============================================================================
 Name        : MSP.c
 Author      : utnso
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include "commons/protocolStructInBigBang.h"
#include "commons/log.h"
#include "mspConfig.h"
#include "MSP.h"
#include "mspCPU.h"

t_log *MSPlogger;
int puertoMSP;
t_MSP* self;

//semaforo
pthread_rwlock_t rw_memoria;


int main(int argc, char *argv[]){

	if (argc != 2){
		printf("Modo de empleo: ./MSP mspConfig.cfg\n");
		return EXIT_SUCCESS;
	}

	MSPlogger = log_create("logMSP.log", "MSP", 1, LOG_LEVEL_TRACE);

	log_info(MSPlogger, "Iniciando consola de la MSP...");

	if(!cargarConfiguracionMSP(argv[1])){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	//IMPORTANTE CONSOLAAAAAAAAA!!!!
	//	int mspConsolathreadNum = pthread_create(&mspConsolaHilo, NULL, &mspLanzarhiloMSPCONSOLA, NULL);
	//	if(mspConsolathreadNum) {
	//		log_error(MSPlogger, "Error - pthread_create() return code: %d\n", mspConsolathreadNum);
	//		exit(EXIT_FAILURE);
	//	}

	crearHilosConexiones();

	log_info(MSPlogger, "Finalizando la consola de la MSP...");
	//pthread_join(mspConsolathreadNum, NULL);
	destruirConfiguracionMSP();

	log_destroy(MSPlogger);
	return EXIT_SUCCESS;

}

void crearHilosConexiones(){

	log_info(MSPlogger, "Creando un hilo escucha...");
	self = malloc(sizeof(t_MSP));

	self->socketMSP = socket_createServer(puertoMSP);

	if (self->socketMSP == NULL)
		log_error(MSPlogger, "MSP: Error al crear socket para el kernel.");

	if(!socket_listen(self->socketMSP))
		log_error(MSPlogger, "MSP: Error al poner a escuchar al Loader: %s", strerror(errno));

	else
		log_info(MSPlogger, "MSP: Escuchando conexiones entrantes en el puerto: %d",puertoMSP);

	self->socketClienteKernel = socket_acceptClient(self->socketMSP);

	t_socket_paquete *paqueteKernel = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	socket_recvPaquete(self->socketClienteKernel, paqueteKernel);

	if(paqueteKernel->header.type == HANDSHAKE_KERNEL){
		int mspHiloKernelInt = pthread_create(&mspHiloKernel, NULL, (void *)mspLanzarHiloKernel, self->socketClienteKernel);
		if(mspHiloKernelInt){
			log_error(MSPlogger, "Error - pthread_create() return code: %d\n", mspHiloKernelInt);
			exit(EXIT_FAILURE);
		}
	}

	int contadorCpu = 0;

	while(1){

		self->socketClienteCPU = socket_acceptClient(self->socketMSP);
		contadorCpu++;

		t_socket_paquete *paqueteCPU = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
		socket_recvPaquete(self->socketClienteCPU, paqueteCPU);

		if(paqueteCPU->header.type == HANDSHAKE_CPU){
			int mspHiloCPUInt = pthread_create(&mspHiloCpus, NULL, (void *)mspLanzarHiloCPU, self->socketClienteCPU);
			if(mspHiloCPUInt){
				log_error(MSPlogger, "Error - pthread_create() return code: %d\n", mspHiloCPUInt);
				exit(EXIT_FAILURE);
			}

			log_debug(MSPlogger, "MSP: cantidad de CPUs conectadas: %d",contadorCpu);
		}
	}//fin del while
}


