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

t_log *MSPlogger;

int main(int argc, char *argv[])
{
	if (argc != 2){
		printf("Modo de empleo: ./MSP mspConfig.cfg\n");
		return EXIT_SUCCESS;
	}

	MSPlogger = log_create("logMSP.log", "MSP", 1, LOG_LEVEL_TRACE);

	log_info(MSPlogger, "Iniciando consola de la MSP...");

	sem_init(&mutex, 0, 1);

	if(!cargarConfiguracionMSP(argv[1])){
		printf("Archivo de configuracion invalido\n");
		return EXIT_SUCCESS;
	}

	lista_procesos = list_create();
	cola_paquetes = list_create();

	int mspConsolatheadNum = pthread_create(&mspConsolaHilo, NULL, (void*) mspLanzarhiloMSPCONSOLA, NULL);
	if(mspConsolatheadNum) {
		fprintf(stderr,"Error - pthread_create() return code: %d\n",mspConsolatheadNum);
		exit(EXIT_FAILURE);
	}

	int mspHiloNum = pthread_create(&mspHilo, NULL, (void*) mspLanzarhilo, NULL);
	if(mspHiloNum) {
		fprintf(stderr,"Error - pthread_create() return code: %d\n", mspHiloNum);
		exit(EXIT_FAILURE);
	}

	pthread_join(mspConsolaHilo,NULL);
	pthread_join(mspHilo, NULL);

	log_info(MSPlogger, "Finalizando la consola de la MSP...");

	log_destroy(MSPlogger);
	return EXIT_SUCCESS;

}

int mspLanzarhilo(){
	int i = 1;
	int puertoMSP = 5050; //esto esta dentro del archivo de conf cambiar!!
	t_socket *socketEscucha, *socketNuevaConexion;

	log_error(MSPlogger, "Creando un hilo escucha, sin saber si es de Kernel o de CPU");

	if (!(socketEscucha = socket_createServer(puertoMSP))) {
		log_error(MSPlogger, "Error al crear socket Escucha del PLP: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	if(!socket_listen(socketEscucha)) {
		log_error(MSPlogger, "Error al poner a escuchar descriptor: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	log_info(MSPlogger, "Ya se esta escuchando conexiones entrantes..");

	while(1){
			socketNuevaConexion = socket_acceptClient(socketEscucha);
			t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
			socket_recvPaquete(socketNuevaConexion, paquete);

			if(paquete->header.type == 5){
				socketKernel=socketNuevaConexion;
				kernelDireccion=direccionCliente;
				pthread_create(&mspHiloKernel, NULL, mspLanzarHiloKernel, NULL);
				log_info(MSPlogger,"Hilo Kernel creado correctamente");
			}

			if(paquete->header.type == 4){
				socketCpus = socketNuevaConexion;
				cpuDireccion = direccionCliente;
				mspHiloCpus = realloc(mspHiloCpus, sizeof(pthread_t)*i+1);
				pthread_create(&mspHiloCpus[i], NULL, mspLanzarHiloCPU, (void *)socketCpus);
				log_info(MSPlogger,"Hilo CPU creado correctamente");
				i++;
			}

			socket_freePaquete(paquete);
		}
	return 0;
}


