#include "cpuKernel.h"
#include "CPU_Proceso.h"
#include <stdlib.h>

t_CPU *self;

void cpuConectarConKernel(){

	log_info(self->loggerCPU, "CPU: Conectando con el Planificador...");

	self->socketPlanificador = socket_createClient();

	if (self->socketPlanificador  == NULL )
		log_error(self->loggerCPU, "Error al crear socket del Proceso CPU!");

	if (socket_connect(self->socketPlanificador, self->ipPlanificador, self->puertoPlanificador) == 0)
		log_error(self->loggerCPU, "CPU: Error al hacer el connect con el Planificador!");

	else
		cpuRealizarHandshakeConKernel(self);

}

void cpuRealizarHandshakeConKernel(){

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	log_info(self->loggerCPU, "La CPU solicita HANDSHAKE con KERNEL.");

	if (socket_sendPaquete(self->socketPlanificador->socket, HANDSHAKE_CPU, 0, NULL) > 0)
		log_info(self->loggerCPU, "CPU le envia HANDSHAKE al KERNEL!");

	if (socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0) {

		if(paquete->header.type == HANDSHAKE_PLANIFICADOR)
			log_info(self->loggerCPU, "CPU: Recibe del Planificador HANDSHAKE_PLANIFICADOR");

		else
			log_error(self->loggerCPU, "CPU: Error al recibir HANDSHAKE_PLANIFICADOR del Planificador");
	}

	else
		log_error(self->loggerCPU, "CPU: Error al recibir paquete del Planificador.");

}


void cpuRecibirTCB(){

	t_socket_paquete *paquetePlanificadorTCB = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));
	t_TCB_CPU* unTCBNuevo = (t_TCB_CPU*) malloc(sizeof(t_TCB_CPU));

	if(socket_recvPaquete(self->socketPlanificador->socket, paquetePlanificadorTCB) >= 0){

		printf("TCB_NUEVO %d \n", paquetePlanificadorTCB->header.type);   //no Borrar sirve para como debug Jorge

		if(paquetePlanificadorTCB->header.type == TCB_NUEVO){

			unTCBNuevo = (t_TCB_CPU*) paquetePlanificadorTCB->data;

			self->tcb = unTCBNuevo;

			log_debug(self->loggerCPU, "CPU: recibio un TCB_NUEVO con PID: %d TID:%d KM:%d", self->tcb->pid, self->tcb->tid, self->tcb->km);
			//printf("TCB_NUEVO TID %d \n", self->tcb->tid );  //no Borrar sirve para como debug Jorge
			//printf("TCB_NUEVO PID %d \n", self->tcb->pid);   //no Borrar sirve para como debug Jorge

			//				registros_cpu->I = (uint32_t)nuevo_tcb->tcb->pid;
			//				registros_cpu->K = (uint32_t)nuevo_tcb->tcb->km;
			//				registros_cpu->M = nuevo_tcb->tcb->base_segmento_codigo;
			//				registros_cpu->P = nuevo_tcb->tcb->puntero_instruccion;
			//				registros_cpu->S = nuevo_tcb->tcb->cursor_stack;
			//				registros_cpu->X = nuevo_tcb->tcb->base_stack;
			//				int i;
			//				for(i=0; i<=4; i++){
			//					registros_cpu->registros_programacion[i]=nuevo_tcb->tcb->registro_de_programacion[i];
			//				}
			//hilo_log = (t_hilo_log *) nuevo_tcb->tcb;
			//
		}

		else
			log_error(self->loggerCPU, "CPU: error al recibir de planificador TCB_NUEVO");
	}
	else
		log_error(self->loggerCPU, "CPU: Error al recibir un paquete del planificador");

	free(paquetePlanificadorTCB);
}


void cpuRecibirQuantum(){

	t_socket_paquete *paquetePlanificadorQuantum = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_quantumCPU* unQuantum =  (t_quantumCPU*) malloc(sizeof(t_quantumCPU));

	if(socket_recvPaquete(self->socketPlanificador->socket, paquetePlanificadorQuantum) >= 0){

		t_quantumCPU* unQuantum = (t_quantumCPU*) paquetePlanificadorQuantum->data;
		self->quantum = unQuantum->quantumCPU;

		log_debug(self->loggerCPU, "CPU: recibe un quantum: %d",self->quantum);
	}

	else
		log_error(self->loggerCPU, "CPU: error al recibir un quantum");

	free(paquetePlanificadorQuantum);
}

int cpuEnviaTermineUnaLinea(){

	if (socket_sendPaquete(self->socketPlanificador->socket, CPU_TERMINE_UNA_LINEA, 0, NULL) <= 0){
		log_info(self->loggerCPU, "CPU: Fallo envio de CPU_TERMINE_UNA_LINEA, PID: %d", self->tcb->pid);
		return EXIT_FAILURE;
	}

	else{

		log_info(self->loggerCPU, "CPU: Envia al Planificador CPU_TERMINE_UNA_LINEA.");

		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0)
			return paquete->header.type;

		else{
			log_info(self->loggerCPU, "CPU: Error al esperar un paquete del planificador.");
			return EXIT_FAILURE;
		}

	}

}


void cpuCambioContexto(){

	//antes de hacer el send, deberia actualizarce el STACK
	//el stack sufre modificaciones cuando se ejecutan las instrucciones ESO, no se actualiza, lo que se actualiza es el TCB.
	if (socket_sendPaquete(self->socketPlanificador->socket, CAMBIO_DE_CONTEXTO, sizeof(t_TCB_CPU), self->tcb) <= 0)
		log_error(self->loggerCPU, "CPU: Falló cambio de conexto");

	else
		log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO");

}

void cpuEnviaInterrupcion(int tamanio, char *algo){

	log_info(self->loggerCPU, "CPU: Solicitud de Interrupcion al Kernel");

	if (socket_sendPaquete(self->socketPlanificador->socket, INTERRUPCION, tamanio, algo) <= 0)
		log_error(self->loggerCPU, "CPU: Error de Interrupcion");

	else
		log_info(self->loggerCPU, "CPU: Interrupcion realizada correctamente");

}

void cpuFinalizarProgramaExitoso(int tamanio, char *algo){

	log_info(self->loggerCPU, "CPU: Envia FINALIZAR_PROGRAMA_EXITO al Kernel");

	if (socket_sendPaquete(self->socketPlanificador->socket, FINALIZAR_PROGRAMA_EXITO, tamanio, algo) <= 0)
		log_error(self->loggerCPU, "CPU: Error de finalizacion de proceso %d", self->tcb->pid);

	else
		log_info(self->loggerCPU, "CPU: Finalizacion de proceso correctamente %d", self->tcb->pid);

}
