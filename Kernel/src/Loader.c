
#include "Loader.h"

void kernel_comenzar_Loader(t_kernel* self){


	loaderEscuchaProgramaBeso(self);


}


void loaderEscuchaProgramaBeso(t_kernel* self){

	t_socket *socketEscucha, *socketNuevaConexion;
	listaDeProgramas = list_create();
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(socketEscucha = socket_createServer(self->puertoLoader))){
		log_error(self->loggerLoader, "Loader: Error al crear socket para escuchar Programas. %s", strerror(errno));
	}

	if(!socket_listen(socketEscucha)){
		log_error(self->loggerLoader, "Loader:Error al poner a escuchar al Loader: %s", strerror(errno));
	}

	log_info(self->loggerLoader, "Loader: Ya se esta escuchando conexiones entrantes en el puerto: %d",self->puertoLoader);

	FD_SET(socketEscucha->descriptor, &master);
    fdmax = socketEscucha->descriptor; /* seguir la pista del descriptor de fichero mayor*/

    /* bucle principal*/
   	while(1){
        read_fds = master;
        //printf("antes select: %d\n",  1111);
        int selectResult = select(fdmax+1, &read_fds, NULL, NULL, NULL);
        log_info(self->loggerLoader,"Loader: Select= %d",selectResult);
        //printf("after select: %d\n",  1111);
        if (selectResult == -1){
        	log_error(self->loggerLoader, "Error en el select del Loader.");
            exit(1);
        }else if (selectResult == 0){

        } else{
			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer
				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					log_info(self->loggerLoader,"Se encontraron datos en el elemento de la lista i=%d, descriptorEscucha=%d",i,socketEscucha->descriptor);


					if(i == socketEscucha->descriptor){  //gestionar nuevas conexiones
						if((socketNuevaConexion = socket_acceptClient(socketEscucha))==0) {
							log_error(self->loggerLoader, "Loader: Error en el accep  Loader");
						}else  {
							log_debug(self->loggerLoader, "Loader: ACCEP completo! ");
							atenderNuevaConexionPrograma(self, socketNuevaConexion, &master, &fdmax);
						}

					}else{ //sino no es una nueva conexion busca un programa en la lista
						log_debug(self->loggerLoader, "Loader:Mensaje del Programa descriptor= %d.", i);
						t_programa* programaCliente = obtenerProgramaConsolaSegunDescriptor(self,i);
						log_debug(self->loggerLoader, "Loader: Mensaje del Programa PID= %d.", programaCliente->programaTCB.pid);
						atienderProgramaConsola(self,programaCliente, &master);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}


t_programa* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor)
{
	log_info(self->loggerLoader,"Loader:Obteniendo Programa descriptor buscado %d",descriptor);

	bool _esProgramaDescriptor(t_programa* programa) {
		return (programa->socketProgramaConsola->descriptor == descriptor);
	}
	t_programa* programaBuscado = list_find(listaDeProgramas, (void*)_esProgramaDescriptor);
	log_info(self->loggerLoader," Loader: Se encontro programa %d",programaBuscado->programaTCB.pid);
	return programaBuscado;
}


void atienderProgramaConsola(t_kernel* self,t_programa* programa, fd_set* master)
{
	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(programa->socketProgramaConsola, paquete)) < 0) {
		log_error(self->loggerLoader, "El programa %d ha cerrado la conexion.", programa->programaTCB.pid);
		FD_CLR(programa->socketProgramaConsola->descriptor, master); // eliminar del conjunto maestro
		close(programa->socketProgramaConsola->descriptor);

		bool esPrograma(t_programa* programaEnLista){
			return (programaEnLista->programaTCB.pid == programa->programaTCB.pid);// hay que ver si uso el TID
		}
		list_remove_by_condition(listaDeProgramas, (void*)esPrograma);

	}else{
		if(paquete->header.type == ESCRIBIR_MEMORIA){//FINALIZAR_PROGRAMA no lo toma _MMMM..
			log_info(self->loggerLoader, " Loader:Llego con exito el FINALIZAR_PROGRAMA del programa %d", programa->programaTCB.pid);
		}else{
			log_error(self->loggerLoader, "Loader: Tipo de mensaje enviado por el Programa %d no identificado.", programa->programaTCB.pid);
		}
	}
	free(paquete);
}


//Esta funcion tiene que mandar crear los TCB con ayuda de la msp y luego ponerlo en la cola de nuevo

//primero el programaBeso le manda al la Loadear HANDSHAKE_PROGRAMA
//segundo el programaBeso se bloquea esperando una nuespuesta, en este caso el loader manda
//tercero el programa

void atenderNuevaConexionPrograma(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax)
{
	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerLoader, " Loader:Error o conexión cerrada por el Cliente correspondiente.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	} else {

		switch(paquete->header.type){
					case HANDSHAKE_PROGRAMA:

						if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_LOADER,0, NULL) >= 0)
							log_info(self->loggerLoader, " Loader:Se envió HANDSHAKE_LOADER correctamente");
						else
							log_error(self->loggerLoader, " Loader: Error enviar HANDSHAKE_LOADER");

						log_info(self->loggerLoader, " Loader:Se envió HANDSHAKE_LOADER correctamente");
						socket_sendPaquete(socketNuevoCliente, FINALIZAR_PROGRAMA_EXITO,0, NULL);
					break;
					case ERROR_POR_SEGMENTATION_FAULT:

						if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_SEGMENTATION_FAULT,0, NULL) >= 0)
							log_info(self->loggerLoader, " Loader:Se envió ERROR_POR_SEGMENTATION_FAULT correctamente");
						else
							log_error(self->loggerLoader, " Loader: Error enviar HANDSHAKE_LOADER");

						log_info(self->loggerLoader, " Loader:Se envió HANDSHAKE_LOADER correctamente");
					break;
					default:
						log_error(self->loggerLoader, "Tipo de mensaje enviado por el Programa no identificado.");
					break;

					}

	}
    free(paquete);
}


void loaderCrearTCB(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid){


}


int loaderCrearSegmento(t_kernel* self,int pid, int tamanio){


	return 1;
}
