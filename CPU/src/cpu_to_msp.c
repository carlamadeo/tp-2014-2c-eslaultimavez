
#include "cpu_to_msp.h"


t_socket_conexion* cpuConectarConMPS() {

	t_socket_conexion* socket_conexion = malloc( sizeof(t_socket_conexion) );

	log_debug(logger, "Conectando con el Proceso MSP...");

	socket_conexion->socket_client = socket_createClient();

	if (socket_conexion->socket_client == NULL ) {
		log_error(logger, "Error al crear socket del Proceso CPU!");
	}
	if (!socket_connect( socket_conexion->socket_client, info_conexion_MSP.ip, info_conexion_MSP.port )) {
		log_error(logger, "Error al conectar el Proceso CPU con el Proceso MSP!");
		perror("Error al conectar el Proceso CPU con el Proceso MSP!");
		exit(-1);
	}

	log_info(logger, "CPU Conectado con la MSP (IP:%s/Puerto:%d)!", info_conexion_MSP.ip, info_conexion_MSP.port);

	return socket_conexion;
}

void cpuRealizarHandshakeConMSP() {
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	log_debug(logger, "La CPU solicita HANDSHAKE con MSP.");

	if (socket_sendPaquete(socketDelMSP, HANDSHAKE_CPU, 0, NULL) > 0) {
		log_info(logger, "CPU le envia HANDSHAKE a la MSP!");
	}

	if (socket_recvPaquete(socketDelMSP, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_MSP){
			log_info(logger, "HANDSHAKE realizado on MSP");
		} else {
			log_error(logger, "Codigo inesperado recibido en el HANDSHAKE de MSP: %d", paquete->header.type);
		}
	} else {
		log_error(logger, "Error al recibir los datos de la MSP en el HANDSHAKE!");
	}
}
