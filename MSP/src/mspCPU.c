#include "MSP.h"
#include "mspCPU.h"
#include "commons/log.h"
#include "commons/collections/list.h"
#include <stdlib.h>
#include <unistd.h>

extern t_log *MSPlogger;
extern t_list *cola_paquetes;

void *mspLanzarHiloCPU(void *arg){
	t_nodo_cola* nodo_cola;
	t_nodo_cola* nodo_cola_encontrado;
	t_socket  *socketCpu;
	socketCpu = (t_socket*) arg;
	t_socket_paquete *paquete;

	if (socket_sendPaquete(socketCpu, 3, 0, NULL) > 0)
		log_info(MSPlogger, "Handshake con CPU!");

	else
		log_error(MSPlogger, "Error al recibir los datos de la MSP!");


	while(1){

		paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if (socket_recvPaquete(socketCpu, paquete) >= 0){

			nodo_cola = malloc(sizeof(t_nodo_cola));
			nodo_cola->paquete=paquete;
			nodo_cola->socket=socketCpu;
			list_add(cola_paquetes, nodo_cola); //muy importante!

			bool _esCPUDescriptor(t_nodo_cola* nodo_cola1){
				return (nodo_cola1->socket->descriptor== socketCpu->descriptor );
			}

			nodo_cola_encontrado = list_remove_by_condition(cola_paquetes,(void*)_esCPUDescriptor);
			paquete = nodo_cola_encontrado->paquete;

			//por cada mensaje de la cpu a la msp tiene que ser atendida aqui dentro del switch!!!!!!
			switch(paquete->header.type){

			case 63://cpu->dame una instruccion
				break;

			case 64://cpu->dame una instruccion system call
				break;
				// hay muchas más!!!
			}//fin del switch

			free(nodo_cola_encontrado);

		}

		else{// fin del if del recibe

			log_error(MSPlogger, "CPU ha cerrado la conexion.");
			close(socketCpu->descriptor);

			free(socketCpu);
			break;
		}

		socket_freePaquete(paquete);

	}//fin del while(1)
	return NULL;
}

