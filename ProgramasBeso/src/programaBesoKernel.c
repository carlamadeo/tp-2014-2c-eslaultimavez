#include "programaBesoKernel.h"
#include "commons/protocolStructInBigBang.h"
#include <errno.h>

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
		log_info(self->loggerProgramaBESO, "Consola se presenta al Kernel!");

	if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0) {
		if(paquete->header.type == HANDSHAKE_LOADER)
			log_info(self->loggerProgramaBESO, "Consola: Se realizaron los Handshake con el Kernel correctamente");
	}
	else
		log_error(self->loggerProgramaBESO, "Consola: Error al recibir los datos del Kernel.");

}

void consolaComunicacionLoader(t_programaBESO* self){

	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	t_datosKernel* datosAKernel = malloc(sizeof(t_datosKernel));
	t_datosMostrarConsola* datosDeKernel = (t_datosMostrarConsola*) (paquete->data);

	datosAKernel->codigoBeso = self->codigo;

	if (socket_sendPaquete(self->socketKernel->socket, CODIGO_BESO, sizeof(t_datosKernel), datosAKernel) > 0)
		log_info(self->loggerProgramaBESO, "Consola: Los datos del Programa Beso se enviaron correctamente al Kernel");


	while(1){
		if (socket_recvPaquete(self->socketKernel->socket, paquete) >= 0){
			if(datosDeKernel->codigo == MENSAJE_DE_ERROR)
				log_error(self->loggerProgramaBESO, datosDeKernel->mensaje);
			else
				log_info(self->loggerProgramaBESO, datosDeKernel->mensaje);
		}
	}

	free(datosAKernel->codigoBeso);
	free(datosAKernel);
	socket_freePaquete(paquete);
}
