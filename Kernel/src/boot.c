#include "boot.h"
#include "kernelMSP.h"
#include "Kernel.h"
#include "kernelConfig.h"
#include "commons/socketInBigBang.h"
#include <stdlib.h>
#include <errno.h>


void crearTCBKERNEL(t_kernel* self){

	log_info(self->loggerKernel, "Boot: Creando TCB...");

	char *codigoSC;

	self->tcbKernel = malloc(sizeof(t_TCB_Kernel));
	self->tcbKernel->pid = 0;
	self->tcbKernel->tid = 0;
	self->tcbKernel->km  = 1;

	self->tcbKernel->tamanio_segmento_codigo = obtenerCodigoBesoSystemCall(self, codigoSC);
	self->tcbKernel->base_segmento_codigo = kernelCrearSegmento(self, self->tcbKernel->pid, self->tcbKernel->tamanio_segmento_codigo);
	self->tcbKernel->puntero_instruccion  = self->tcbKernel->base_segmento_codigo;

	//enviarCodigoSystemCallsAMSP(self, codigoSC, self->tcbKernel->tamanio_segmento_codigo);

	self->tcbKernel->base_stack = kernelCrearSegmento(self, self->tcbKernel->pid, self->tamanioStack);
	self->tcbKernel->cursor_stack = self->tcbKernel->base_stack;

	self->tcbKernel->registro_de_programacion[0] = 0;
	self->tcbKernel->registro_de_programacion[1] = 0;
	self->tcbKernel->registro_de_programacion[2] = 0;
	self->tcbKernel->registro_de_programacion[3] = 0;

	log_info(self->loggerKernel, "Boot: Completado con éxito!");

	//return programaEnElKernel;
}


//esta funcion tiene que leer el archivo
int obtenerCodigoBesoSystemCall(t_kernel *self, char *codigoSystemCall){

	FILE *archivoBesoSystemCall = fopen("../Resources/besoSystemCall.bc", "r");

	if(archivoBesoSystemCall == 0){
		log_error(self->loggerKernel, "Kernel: Error al abrir el archivo besoSystemCall");
		exit(-1);
	}

	fseek(archivoBesoSystemCall, 0, SEEK_END);	//Me coloco al final del fichero para saber el tamanio
	size_t sizeArchivoBesoSC = ftell(archivoBesoSystemCall);
	fseek(archivoBesoSystemCall, 0, SEEK_SET);	//Me coloco al principio del fichero para leerlo

	codigoSystemCall = malloc(sizeof(char)*sizeArchivoBesoSC);

	fread(codigoSystemCall, 1, sizeArchivoBesoSC, archivoBesoSystemCall);

	return sizeArchivoBesoSC;
}

void enviarCodigoSystemCallsAMSP(t_kernel *self, char *codigoSystemCall, int tamanioSC){
	t_socket *socketEscucha, *socketNuevaConexion;

	if (!(socketEscucha = socket_createServer(self->puertoLoader)))
		log_error(self->loggerLoader, "Boot: Error al crear socket para escuchar Programas: %s", strerror(errno));

	if(!socket_listen(socketEscucha))
		log_error(self->loggerLoader, "Boot: Error al poner a escuchar al Loader: %s", strerror(errno));

	else
		log_info(self->loggerLoader, "Boot: Escuchando conexiones entrantes en el puerto: %d",self->puertoLoader);

	if((socketNuevaConexion = socket_acceptClient(socketEscucha)) == 0)
		log_error(self->loggerLoader, "Boot: Error en el accept");

	else{
		log_debug(self->loggerLoader, "Boot: Accept completo!");

	}
	int unaRespuesta = kernelEscribirMemoria(self, self->tcbKernel->pid, self->tcbKernel->base_segmento_codigo, codigoSystemCall, tamanioSC, socketNuevaConexion);
}
	/*off_t offset = 0;

	t_socket_header header;
	header.length = sizeof(header) + sizeArchivoBesoSC;

	if(send(self->socketKernel->socket->descriptor, &header, sizeof(t_socket_header), 0) != sizeof(t_socket_header) || sendfile(self->socketKernel->socket->descriptor, archivoBeso->_fileno, &offset, sizeArchivoBesoSC) != sizeArchivoBesoSC)
		log_error(self->loggerProgramaBESO, "Boot: No se pudo enviar el archivo de System Calls a la MSP");

	log_info(self->loggerProgramaBESO, "Consola: Espera respuesta del kernel");
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_datosKernel* datosAKernel = malloc(sizeof(t_datosKernel));
	t_datosMostrarConsola* datosDeKernel = (t_datosMostrarConsola*) (paquete->data);

	datosAKernel->codigoBeso = self->codigo;

	if (socket_sendPaquete(self->socketKernel->socket, CODIGO_BESO, sizeof(t_datosKernel), datosAKernel) > 0)
		log_info(self->loggerProgramaBESO, "Consola: Los datos del Programa Beso se enviaron correctamente al Kernel");


	while(1){
		if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0){

			switch(datosDeKernel->codigo){

			case ERROR_POR_TAMANIO_EXCEDIDO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por tamaño de segmento excedido");
				break;
			case ERROR_POR_MEMORIA_LLENA:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por memoria llena");
				break;
			case ERROR_POR_NUMERO_NEGATIVO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por solicitar un tamaño de segmento negativo");
				break;
			case ERROR_POR_SEGMENTO_INVALIDO:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error por segmento invalido");
				break;
			case ERROR_POR_SEGMENTATION_FAULT:
				log_error(self->loggerProgramaBESO,"Consola: Se ha recibido un error del tipo Segmentation Fault");
				break;
			default:
				log_info(self->loggerProgramaBESO,"Consola: Recibe OK: %s",datosDeKernel->mensaje);
				break;

			}// fin del switch


		}
		else{
			log_error(self->loggerProgramaBESO, "Consola: El Kernel ha cerrado la conexion.");
			close(self->socketKernel->socket->descriptor);
			exit(-1);
		}
	}

	free(datosAKernel->codigoBeso);
	free(datosAKernel);
	socket_freePaquete(paquete);*/

