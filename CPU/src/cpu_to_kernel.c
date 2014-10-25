
#include "cpu_to_kernel.h"

t_socket_conexion* conectarCPUConKernel(void) {
	t_socket_conexion* socket_conexion = malloc( sizeof(t_socket_conexion) );
	log_debug(logger, "Conectando con el Proceso Kernel...");

	socket_conexion->socket_client = socket_createClient();

	if (socket_conexion->socket_client == NULL ) {
		log_error(logger, "Error al crear socket del Proceso CPU!");
	}
	if (!socket_connect(socket_conexion->socket_client, info_conexion_KERNEL.ip, info_conexion_KERNEL.port )) {
		log_error(logger, "Error al conectar el Proceso CPU con el Proceso KERNEL! A la ip %s con puerto %d", info_conexion_KERNEL.ip, info_conexion_KERNEL.port);
		perror("Error al conectar el Proceso CPU con el Proceso KERNEL!");
		exit(-1);
	}

	log_info(logger, "CPU Conectado con el KERNEL (IP:%s/Puerto:%d)!", info_conexion_KERNEL.ip, info_conexion_KERNEL.port);

	return socket_conexion;
}

void cpuRealizarHandshakeConKernel() {
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	log_debug(logger, "La CPU solicita HANDSHAKE con KERNEL.");

	if (socket_sendPaquete(socketDelKernel, HANDSHAKE_CPU, 0, NULL) > 0) {
		log_info(logger, "CPU le envia HANDSHAKE al KERNEL!");
	}

	if (socket_recvPaquete(socketDelKernel, paquete) >= 0) {
		if(paquete->header.type == 2){
			t_handshake_cpu_kernel* handshake = malloc(sizeof(t_handshake_cpu_kernel));
			handshake = (t_handshake_cpu_kernel *) (paquete->data);
			retardo   = handshake->retardo;
			quantum   = handshake->quantum;
			stack	  = handshake->stack;
			free(handshake);
			log_info(logger, "HANDSHAKE realizado con KERNEL, Me da un retardo de %d y quantum", retardo, quantum);
		} else {
			log_error(logger, "Codigo inesperado recibido en el HANDSHAKE de KERNEL: %d", paquete->header.type);
		}
	} else {
		log_error(logger, "Error al recibir los datos del Kernel en el HANDSHAKE!!");
	}
}
