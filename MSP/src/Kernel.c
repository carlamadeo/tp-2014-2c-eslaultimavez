#include "Kernel.h"

extern t_log *MSPlogger;

void *mspLanzarHiloKernel(void *arg){
	t_socket_paquete *paquete;
	t_socket  *socketKernel;
	socketKernel = (t_socket*) arg;

	if (socket_sendPaquete(socketKernel, 3, 0, NULL) > 0) {
			log_info(MSPlogger, "Handshake con kernel!");
	}else{
		log_error(MSPlogger, "Error al recibir los datos del Kernel!");
	}


	//Aqui es en donde se comunican los procesos, importante que esto se defina lo antes posible!
	while(1){
		paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
		if (socket_recvPaquete(socketKernel, paquete) >= 0){


			//por cada paquete se tiene que agregar un case con un numero distinto
			switch(paquete->header.type){
			case 6: //crear un segmento
				log_info(MSPlogger, "Kernel esta creando un segmento...");
				t_envio_num* datos = (t_envio_num*) (paquete->data);
				log_info(MSPlogger,"Abriendo el paquete de kernel, el paquete contiene PID: %d y tamaño : %d",datos->pid, datos->baseDelSegmento);

				sem_wait(&mutex);
			//	mspConsolaCrearSegmento(datos->pid,datos->baseDelSegmento,1);
				sem_post(&mutex);
				break;
			case 11:
				log_info(MSPlogger, "Kernel esta pidiendo destruir un segmento");
				int datos_destruir = *(int*) (paquete->data);
				log_info(MSPlogger, "Realizando destruir segmentos del programa: %d...",datos_destruir);

				sem_wait(&mutex);
			//	mspConsolaDestruirSegmentos(datos_destruir, 1);//si es 1 es de kernel validar!!!!
				sem_post(&mutex);
				break;

			}// fin del switch

		}else{  //fin del if del recv
			log_error(MSPlogger, "El KERNEL ha cerrado la conexion.");
			close(socketKernel->descriptor);
			exit(-1);
		}

		socket_freePaquete(paquete);
	}//fin del while(1)
	return NULL;
}
