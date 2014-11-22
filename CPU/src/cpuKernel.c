
#include "cpuKernel.h"

t_socket_client* conectarCPUConKernel(t_CPU* self) {

	log_info(self->loggerCPU, "CPU: Conectando con el Planificador...");

	self->socketPlanificador = socket_createClient();

	if (self->socketPlanificador  == NULL ) {
			log_error(self->loggerCPU, "Error al crear socket del Proceso CPU!");
		}
	if (socket_connect(self->socketPlanificador, self->ipPlanificador, self->puertoPlanificador) == 0){
				log_error(self->loggerCPU, "CPU: Error al hacer el connect con el Planificador!");

			}else{
				//log_info(self->loggerCPU, "CPU: Conectado con el Planificador (IP:%s/Puerto:%d)!", self->ipPlanificador, self->puertoPlanificador);
				cpuRealizarHandshakeConKernel(self);
			}
	return self->socketPlanificador;
}

void cpuRealizarHandshakeConKernel(t_CPU* self){
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	log_info(self->loggerCPU, "La CPU solicita HANDSHAKE con KERNEL.");

	if (socket_sendPaquete(self->socketPlanificador->socket, HANDSHAKE_CPU, 0, NULL) > 0) {
		log_info(self->loggerCPU, "CPU le envia HANDSHAKE al KERNEL!");
	}

	if (socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0) {
			if(paquete->header.type == HANDSHAKE_PLANIFICADOR){
				log_info(self->loggerCPU, "CPU: Recibe del Planificador HANDSHAKE_PLANIFICADOR");
			} else {
				log_error(self->loggerCPU, "CPU: Error al recibir HANDSHAKE_PLANIFICADOR del Planificador");
			}
		} else {
			log_error(self->loggerCPU, "CPU: Error al recibir paquete del Planificador.");
		}
}


