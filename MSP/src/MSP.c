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

//	int mspConsolathreadNum = pthread_create(&mspConsolaHilo, NULL, &mspLanzarhiloMSPCONSOLA, NULL);
//	if(mspConsolathreadNum) {
//		log_error(MSPlogger, "Error - pthread_create() return code: %d\n", mspConsolathreadNum);
//		exit(EXIT_FAILURE);
//	}

	int mspHiloNum = pthread_create(&mspHilo, NULL, (void*) mspLanzarhilo, NULL);
	if(mspHiloNum) {
		log_error(MSPlogger, "Error - pthread_create() return code: %d\n", mspHiloNum);
		exit(EXIT_FAILURE);
	}

	pthread_rwlock_init(&rw_memoria, NULL);

	pthread_join(mspHilo, NULL);
	log_info(MSPlogger, "Finalizando la consola de la MSP...");
	//pthread_join(mspConsolathreadNum, NULL);

	destruirConfiguracionMSP();

	log_destroy(MSPlogger);
	return EXIT_SUCCESS;

}

int mspLanzarhilo(){

	int i = 1;
	t_socket *socketEscucha, *socketNuevaConexion;

	log_info(MSPlogger, "Creando un hilo escucha...");


	if (!(socketEscucha = socket_createServer(puertoMSP))) {
		log_error(MSPlogger, "Error al crear socket Escucha: %s.", strerror(errno));
		return EXIT_FAILURE;
	}

	if(!socket_listen(socketEscucha)) {
		log_error(MSPlogger, "Error al poner a escuchar descriptor: %s.", strerror(errno));
		return EXIT_FAILURE;
	}

	log_info(MSPlogger, "Escuchando conexiones entrantes de la MSP en el Puerto:%d y en el descriptor: %d ",puertoMSP, socketEscucha->descriptor );

	while(1){
			socketNuevaConexion = socket_acceptClient(socketEscucha);
			t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
			socket_recvPaquete(socketNuevaConexion, paquete);

			if(paquete->header.type == HANDSHAKE_KERNEL){
				socketKernel = socketNuevaConexion;
				kernelDireccion = direccionCliente;
				mspLanzarHiloKernel(socketKernel);// cambiar esto tiene que ser un hilo!!!
				log_info(MSPlogger,"Hilo Kernel creado correctamente.");
			}

			if(paquete->header.type == HANDSHAKE_CPU){
				socketCpus = socketNuevaConexion;
				cpuDireccion = direccionCliente;
				//mspHiloCpus = realloc(mspHiloCpus, sizeof(pthread_t) * i + 1);
				mspLanzarHiloCPU(socketCpus); //cambiar esto por un hilo!!!
				//pthread_create(&mspHiloCpus[i], NULL, mspLanzarHiloCPU, (void *)socketCpus);
				log_info(MSPlogger,"Hilo CPU creado correctamente.");
				i++;
			}

			socket_freePaquete(paquete);
		}
	return 0;
}


