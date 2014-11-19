#include "Memoria.h"
#include "mspCPU.h"
#include "mspPrograma.h"
#include "commons/log.h"
#include "commons/socketInBigBang.h"
#include "commons/protocolStructInBigBang.h"
#include <stdlib.h>
#include <unistd.h>


t_log *MSPlogger;

void *mspLanzarHiloCPU(t_socket * socketCPU){
	t_socket_paquete *paquete;
	int i = 1;

	log_info(MSPlogger,"Hilo CPU creado correctamente.");

	if (socket_sendPaquete(socketCPU, HANDSHAKE_MSP, 0, NULL) > 0)
		log_info(MSPlogger, "MSP: Handshake con CPU!");


	else
		log_error(MSPlogger, "MSP: Error al recibir los datos de la CPU.");


	while(i){

		paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if (socket_recvPaquete(socketCPU, paquete) >= 0){

			switch(paquete->header.type){

			case CREAR_SEGMENTO:
				crearSegmentoCPU(socketCPU, paquete);
				break;
			case DESTRUIR_SEGMENTO:
				destruirSegmentoCPU(socketCPU, paquete);
				break;
			case ESCRIBIR_MEMORIA:
				escribirMemoriaCPU(socketCPU, paquete);
				break;
			case LEER_MEMORIA:
				leerMemoriaCPU(socketCPU, paquete);
				break;
			default:
				break;
			}
		}

		else{
			log_error(MSPlogger, "MSP: El CPU ha cerrado la conexion.");
			close(socketCPU->descriptor);
			i = 0;
		}

		socket_freePaquete(paquete);
	}//fin del while(1)

	return NULL;
}


void crearSegmentoCPU(t_socket  *socketCPU, t_socket_paquete *paquete){

	t_datos_aCPUSegmento* datosACPU = malloc(sizeof(t_datos_aCPUSegmento));
	t_datos_deCPUCrearSegmento* datosDeCPU = (t_datos_deCPUCrearSegmento*) (paquete->data);

	log_info(MSPlogger, "MSP: CPU esta solicitando la creacion de un segmento...");

	log_info(MSPlogger, "MSP: Abriendo el paquete del CPU: El paquete contiene PID: %d, Tamaño : %d", datosDeCPU->pid, datosDeCPU->tamanio);

	buscarCrearPrograma(datosDeCPU->pid);

	//Esto envía ERROR_POR_TAMANIO_EXCEDIDO o ERROR_POR_MEMORIA_LLENA o ERROR_POR_NUMERO_NEGATIVO o ERROR_POR_SEGMENTO_INVALIDO
	//o ERROR_POR_SEGMENTATION_FAULT o la base del segmento si no hubo ningún problema
	datosACPU->recibido = mspCrearSegmento(datosDeCPU->pid, datosDeCPU->tamanio);

	if (socket_sendPaquete(socketCPU, CREAR_SEGMENTO, sizeof(t_datos_aCPUSegmento), datosACPU) > 0)
		log_info(MSPlogger, "MSP: Los datos de creacion de Segmento se han enviado al CPU correctamente");

	free(datosACPU);
	free(datosDeCPU);
}


void destruirSegmentoCPU(t_socket  *socketCPU, t_socket_paquete *paquete){

	t_datos_aCPUSegmento* datosACPU = malloc(sizeof(t_datos_aCPUSegmento));
	t_datos_deCPUDestruirSegmento* datosDeCPU = (t_datos_deCPUDestruirSegmento*) (paquete->data);

	log_info(MSPlogger, "MSP: CPU esta solicitando la destruccion de un segmento...");

	log_info(MSPlogger, "MSP: Abriendo el paquete del CPU: El paquete contiene PID: %d, Tamaño : %d", datosDeCPU->pid, datosDeCPU->direccionBase);

	//Esto envía SIN_ERRORES si se destruyó el segmento correctamente
	//o ERROR_POR_SEGMENTO_DESCONOCIDO si el segmento indicado es incorrecto
	datosACPU->recibido = mspDestruirSegmento(datosDeCPU->pid, datosDeCPU->direccionBase);

	if (socket_sendPaquete(socketCPU, CREAR_SEGMENTO, sizeof(t_datos_aCPUSegmento), datosACPU) > 0)
		log_info(MSPlogger, "MSP: Los datos de destruccion de Segmento se han enviado al CPU correctamente");

	free(datosACPU);
	free(datosDeCPU);
}


void escribirMemoriaCPU(t_socket  *socketCPU, t_socket_paquete *paquete){

	t_datos_aCPUEscritura* datosACPU = malloc(sizeof(t_datos_aCPUEscritura));
	t_datos_deCPUEscritura* datosDeCPU = (t_datos_deCPUEscritura*) (paquete->data);

	log_info(MSPlogger, "MSP: CPU esta solicitando leer la memoria...");

	log_info(MSPlogger, "MSP: Abriendo el paquete del CPU: El paquete contiene PID: %d, Direccion Virtual : %0.8p, Tamaño: %d", datosDeCPU->pid, datosDeCPU->direccionVirtual, datosDeCPU->tamanio);

	//Esto envía SIN_ERRORES si se escribió la memoria correctamente
	//o ERROR_POR_SEGMENTATION_FAULT si se intentó escribir memoria inválida
	datosACPU->estado = mspEscribirMemoria(datosDeCPU->pid, datosDeCPU->direccionVirtual, datosDeCPU->buffer, datosDeCPU->tamanio);

	if (socket_sendPaquete(socketCPU, ESCRIBIR_MEMORIA, sizeof(t_datos_aCPUEscritura), datosACPU) > 0)
		log_info(MSPlogger, "MSP: Los datos de lectura de memoria se han enviado al CPU correctamente");

	free(datosACPU);
	free(datosDeCPU->buffer);
	free(datosDeCPU);
}


void leerMemoriaCPU(t_socket  *socketCPU, t_socket_paquete *paquete){

	//TODO ver si es necesario hacer un malloc a datosACPU->lectura!!!
	//jorge Para mi si es muy importante!
	//es mas tiene que estar inicialisada con un valor
	//porque la pasas como parametro en la funcion mspLeerMemoria y fijate que esta vacia
	t_datos_aCPULectura* datosACPU = malloc(sizeof(t_datos_aCPULectura));
	t_datos_deCPULectura* datosDeCPU = (t_datos_deCPULectura*) (paquete->data);

	log_info(MSPlogger, "MSP: CPU esta solicitando leer la memoria...");

	log_info(MSPlogger, "MSP: Abriendo el paquete del CPU: El paquete contiene PID: %d, Direccion Virtual : %0.8p, Tamaño: %d", datosDeCPU->pid, datosDeCPU->direccionVirtual, datosDeCPU->tamanio);

	//Esto envía SIN_ERRORES si se leyó la memoria correctamente
	//o ERROR_POR_SEGMENTATION_FAULT si se intentó leer memoria inválida
	datosACPU->estado = mspLeerMemoria(datosDeCPU->pid, datosDeCPU->direccionVirtual, datosDeCPU->tamanio, datosACPU->lectura);

	if (socket_sendPaquete(socketCPU, LEER_MEMORIA, sizeof(t_datos_aCPULectura), datosACPU) > 0)
		log_info(MSPlogger, "MSP: Los datos de lectura de memoria se han enviado al CPU correctamente");

	//free(datosACPU->lectura);  //jorge comente esto porque tira SEGMENTATION_FAULT
	free(datosACPU);
	free(datosDeCPU);
}
