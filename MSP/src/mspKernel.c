#include "Memoria.h"
#include "mspKernel.h"
#include "commons/log.h"
#include "commons/socketInBigBang.h"
#include "commons/protocolStructInBigBang.h"
#include <stdlib.h>
#include <unistd.h>

t_log *MSPlogger;

void *mspLanzarHiloKernel(t_socket  *socketKernel){
	t_socket_paquete *paquete;

	if (socket_sendPaquete(socketKernel, HANDSHAKE_MSP, 0, NULL) > 0)
		log_info(MSPlogger, "MSP: Handshake con Kernel!");

	else
		log_error(MSPlogger, "MSP: Error al recibir los datos del Kernel.");


	while(1){

		paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if (socket_recvPaquete(socketKernel, paquete) >= 0){

			switch(paquete->header.type){

			case CREAR_SEGMENTO:
				crearSegmentoKernel(socketKernel, paquete);
				break;
			case DESTRUIR_SEGMENTO:
				destruirSegmentoKernel(socketKernel, paquete);
				break;
			default:
				break;

			}// fin del switch

		}

		else{  //fin del if del recv
			log_error(MSPlogger, "El Kernel ha cerrado la conexion.");
			close(socketKernel->descriptor);
			exit(-1);
		}

		//socket_freePaquete(paquete);
	}//fin del while(1)

	return NULL;
}

void crearSegmentoKernel(t_socket  *socketKernel, t_socket_paquete *paquete){

	t_socket_paquete *paqueteAKernel;

	log_info(MSPlogger, "Kernel esta solicitando la creacion de un segmento...");

	t_envio_numMSP* datosKernel = (t_envio_numMSP*) (paquete->data);

	log_info(MSPlogger,"Abriendo el paquete del Kernel: El paquete contiene PID: %d, Tamaño : %d", datosKernel->pid, datosKernel->tamanio);

	buscarCrearPrograma(datosKernel->pid);

	//Esto envía ERROR_POR_TAMANIO_EXCEDIDO o ERROR_POR_MEMORIA_LLENA o ERROR_POR_NUMERO_NEGATIVO o ERROR_POR_SEGMENTO_INVALIDO
	//o ERROR_POR_SEGMENTATION_FAULT o la base del segmento si no hubo ningún problema
	paqueteAKernel->data = mspCrearSegmento(datosKernel->pid, datosKernel->tamanio);

	if (socket_sendPaquete(socketKernel, CREAR_SEGMENTO, sizeof(paqueteAKernel), paqueteAKernel) > 0)
		log_info(MSPlogger, "Los datos de creacion de Segmento se han enviado al Kernel correctamente");

	free(datosKernel);
	socket_freePaquete(paqueteAKernel);
}

void destruirSegmentoKernel(t_socket  *socketKernel, t_socket_paquete *paquete){

	t_socket_paquete *paqueteAKernel;

	log_info(MSPlogger, "Kernel esta solicitando la destruccion de un segmento...");

	t_envio_numMSP* datosKernel = (t_envio_numMSP*) (paquete->data);

	log_info(MSPlogger,"Abriendo el paquete del Kernel: El paquete contiene PID: %d, Tamaño : %d", datosKernel->pid, datosKernel->tamanio);

	//Esto envía SIN_ERRORES si se destruyó el segmento correctamente
	//o ERROR_POR_SEGMENTO_DESCONOCIDO si el segmento indicado es incorrecto
	paqueteAKernel->data = mspDestruirSegmento(datosKernel->pid, datosKernel->tamanio);

	if (socket_sendPaquete(socketKernel, DESTRUIR_SEGMENTO, sizeof(paqueteAKernel), paqueteAKernel) > 0)
		log_info(MSPlogger, "Los datos de destruccion de Segmento se han enviado al Kernel correctamente");

	free(datosKernel);
	socket_freePaquete(paqueteAKernel);
}
