#include "Memoria.h"
#include "Kernel.h"
#include "commons/log.h"
#include "commons/socketInBigBang.h"
#include "commons/protocolStructInBigBang.h"
#include <stdlib.h>
#include <unistd.h>

extern t_log *MSPlogger;

void *mspLanzarHiloKernel(t_socket  *socketKernel){
	t_socket_paquete *paquete;


	if (socket_sendPaquete(socketKernel, HANDSHAKE_MSP, 0, NULL) > 0)
		log_info(MSPlogger, "MSP:Handshake con Kernel!");

	else
		log_error(MSPlogger, "MSP:Error al recibir los datos del Kernel!");


	//Aqui es en donde se comunican los procesos, importante que esto se defina lo antes posible!
	while(1){

		paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
		if (socket_recvPaquete(socketKernel, paquete) >= 0){

			//por cada paquete se tiene que agregar un case con un numero distinto
			switch(paquete->header.type){

				case CREAR_SEGMENTO:
					log_info(MSPlogger, "Kernel esta creando un segmento...");
					t_envio_numMSP* datos = (t_envio_numMSP*) (paquete->data);
					log_info(MSPlogger,"Abriendo el paquete de kernel, el paquete contiene PID: %d y tamaño : %d",datos->pid, datos->baseDelSegmento);

					sem_wait(&mutex);
						uint32_t direccionBase = mspCrearSegmento(datos->pid,datos->baseDelSegmento);
						t_envio_numMSP *datos2 =malloc(sizeof(t_envio_numMSP ));
						datos2->pid=12;
						datos2->baseDelSegmento=direccionBase;// aca es muy loco pero estoy cambiando un tamaño por un puntero:::: ideas de gonza canto!
						if (socket_sendPaquete(socketKernel, CREAR_SEGMENTO, sizeof(t_envio_numMSP), datos) > 0) {
								log_info(MSPlogger, "Se envia a kernel el inicio : %d del segmento creado para el programa :%d", datos2->baseDelSegmento,datos2->pid);

						}
								free(datos);
					sem_post(&mutex);
					printf("MSP:Mensaje Crear Segmento Enviado");
					break;
				case DESTRUIR_SEGMENTO:
					log_info(MSPlogger, "Kernel esta pidiendo destruir un segmento");
					int datos_destruir = *(int*) (paquete->data);
					log_info(MSPlogger, "Realizando destruir segmentos del programa: %d...",datos_destruir);

					sem_wait(&mutex);
					//	mspConsolaDestruirSegmentos(datos_destruir, 1);//si es 1 es de kernel validar!!!!
					sem_post(&mutex);
					break;

			}// fin del switch

		}else{  //fin del if del recv
			log_error(MSPlogger, "El Kernel ha cerrado la conexion.");
			close(socketKernel->descriptor);
			exit(-1);
		}

		socket_freePaquete(paquete);
	}//fin del while(1)
	return NULL;
}
