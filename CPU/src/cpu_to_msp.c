
#include "cpu_to_msp.h"


t_socket_client* cpuConectarConMPS(t_CPU* self) {

	log_info(self->loggerCPU, "Conectando con el Proceso MSP...");

	self->socketMSP = socket_createClient();

	if (self->socketMSP  == NULL ) {
		log_error(logger, "Error al crear socket del Proceso CPU!");
	}
	log_info(self->loggerCPU, "CPU: Antes del connet (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMSP);
	if (socket_connect(self->socketMSP, self->ipMsp, self->puertoMSP) == 0)
			log_error(self->loggerCPU, "CPU: Error al hacer el Boot con la MSP!");

		else{
			log_info(self->loggerCPU, "CPU: Conectado con la MSP (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMSP);
			cpuRealizarHandshakeConMSP(self);
		}

	return self->socketMSP;
}

void cpuRealizarHandshakeConMSP(t_CPU* self) {
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	log_debug(self->loggerCPU, "CPU: Envia HANDSHAKE a MSP.");

	if (socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_CPU, 0, NULL) > 0) {
		log_info(logger, "CPU le envia HANDSHAKE a la MSP!");
	}

	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP){
			log_info(self->loggerCPU, "HANDSHAKE realizado on MSP");
		} else {
			log_error(self->loggerCPU, "Codigo inesperado recibido en el HANDSHAKE de MSP: %d", paquete->header.type);
		}
	} else {
		log_error(self->loggerCPU, "Error al recibir los datos de la MSP en el HANDSHAKE!");
	}
}
