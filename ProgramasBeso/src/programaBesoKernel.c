#include "programaBesoKernel.h"
#include "commons/protocolStructInBigBang.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

void consolaHacerConexionconLoader(t_programaBESO* self){

	self->socketKernel = socket_createClient();

	if (self->socketKernel == NULL )
		log_error(self->loggerProgramaBESO, "Consola: Error al crear socket con el Kernel!");

	if (socket_connect(self->socketKernel, self->ipLoader, self->puertoLoader) == 0)
		log_error(self->loggerProgramaBESO, "Consola: Error al conectarse con el Kernel.");

	else{
		log_info(self->loggerProgramaBESO, "Consola: Conectado con el Kernel (IP: %s/Puerto: %d)!", self->ipLoader, self->puertoLoader);
		realizarHandshakeConLoader(self);
	}

}

void realizarHandshakeConLoader(t_programaBESO* self){

	t_socket_paquete *paquete = (t_socket_paquete*) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketKernel->socket, HANDSHAKE_PROGRAMA, 0, NULL) > 0)
		log_info(self->loggerProgramaBESO, "Consola: Envia al Kernel: HANDSHAKE_PROGRAMA ");

	if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_LOADER)
			log_info(self->loggerProgramaBESO, "Consola: Recibe del Kernel: HANDSHAKE_LOADER");
	}
	else
		log_error(self->loggerProgramaBESO, "Consola: Error al recibir los datos del Kernel.");

}

void consolaComunicacionLoader(t_programaBESO* self, char *parametro){

	FILE *archivoBeso = fopen(parametro, "r");

	if(archivoBeso == 0){
		log_error(self->loggerProgramaBESO, "Ha ocurrido un problema al abrir el archivo");
		exit(-1);
	}

	fseek(archivoBeso, 0, SEEK_END);	//Me coloco al final del fichero para saber el tamanio
	size_t sizeArchivoBeso = ftell(archivoBeso);
	fseek(archivoBeso, 0, SEEK_SET);	//Me coloco al principio del fichero para leerlo

	off_t offset = 0;

	t_socket_header header;
	header.length = sizeof(header) + sizeArchivoBeso;

	if(send(self->socketKernel->socket->descriptor, &header, sizeof(t_socket_header), 0) != sizeof(t_socket_header) || sendfile(self->socketKernel->socket->descriptor, archivoBeso->_fileno, &offset, sizeArchivoBeso) != sizeArchivoBeso)
		log_error(self->loggerProgramaBESO, "Consola: No se pudo enviar el archivo");

	log_info(self->loggerProgramaBESO, "Consola: Espera respuesta del kernel");
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_datosKernel* datosAKernel = malloc(sizeof(t_datosKernel));
	t_datosMostrarConsola* datosDeKernel = (t_datosMostrarConsola*) (paquete->data);

	datosAKernel->codigoBeso = self->codigo;

	if (socket_sendPaquete(self->socketKernel->socket, CODIGO_BESO, sizeof(t_datosKernel), datosAKernel) > 0)
		log_info(self->loggerProgramaBESO, "Consola: Los datos del Programa Beso se enviaron correctamente al Kernel");


	while(1){
		if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0){
			if(datosDeKernel->codigo == MENSAJE_DE_ERROR)
				log_error(self->loggerProgramaBESO, "Consola: Recibe un MENSAJE_DE_ERROR: %s", datosDeKernel->mensaje);
			else
				log_info(self->loggerProgramaBESO, "Consola: Recibe OK: %s",datosDeKernel->mensaje);
		}
		else{
			log_error(self->loggerProgramaBESO, "Consola: El Kernel ha cerrado la conexion.");
			close(self->socketKernel->socket->descriptor);
			exit(-1);
		}
	}

	free(datosAKernel->codigoBeso);
	free(datosAKernel);
	socket_freePaquete(paquete);
}
