
#include "cpu_to_msp.h"


t_socket_client* cpuConectarConMPS(t_CPU* self) {

	//log_info(self->loggerCPU, "Conectando con el Proceso MSP...");

	self->socketMSP = socket_createClient();

	if (self->socketMSP  == NULL ) {
		log_error(logger, "Error al crear socket del Proceso CPU!");
	}
	log_info(self->loggerCPU, "CPU: Antes del connet (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMSP);
	if (socket_connect(self->socketMSP, self->ipMsp, self->puertoMSP) == 0)
			log_error(self->loggerCPU, "CPU: Error al hacer el connect con la MSP!");

		else{
			//log_info(self->loggerCPU, "CPU: Conectado con la MSP (IP:%s/Puerto:%d)!", self->ipMsp, self->puertoMSP);
			cpuRealizarHandshakeConMSP(self);
		}

	return self->socketMSP;
}

void cpuRealizarHandshakeConMSP(t_CPU* self) {
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_CPU, 0, NULL) > 0) {
		log_info(self->loggerCPU, "CPU le envia HANDSHAKE a la MSP!");
	}

	if (socket_recvPaquete(self->socketMSP->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP){
			log_info(self->loggerCPU, "CPU: Recibe de la MSP HANDSHAKE_MSP");
		} else {
			log_error(self->loggerCPU, "CPU: Error al recibir HANDSHAKE_MSP de la MSP");
		}
	} else {
		log_error(self->loggerCPU, "CPU: Error al recibir paquete de la MSP.");
	}
}
