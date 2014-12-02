
#include "Loader.h"
#include "kernelMSP.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int unPIDGlobal = 1;
int unTIDGlobal = 1;


void kernel_comenzar_Loader(t_kernel* self){

	t_socket *socketEscucha, *socketNuevaConexion;
	listaDeProgramasDisponibles = list_create();
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(socketEscucha = socket_createServer(self->puertoLoader)))
		log_error(self->loggerLoader, "Loader: Error al crear socket para escuchar Programas: %s", strerror(errno));


	if(!socket_listen(socketEscucha))
		log_error(self->loggerLoader, "Loader: Error al poner a escuchar al Loader: %s", strerror(errno));

	else
		log_info(self->loggerLoader, "Loader: Escuchando conexiones entrantes en el puerto: %d",self->puertoLoader);

	FD_SET(socketEscucha->descriptor, &master);
	fdmax = socketEscucha->descriptor; /* seguir la pista del descriptor de fichero mayor*/

	/* bucle principal*/
	while(1){
		read_fds = master;
		//printf("antes select: %d\n",  1111);
		int selectResult = select(fdmax+1, &read_fds, NULL, NULL, NULL);
		log_info(self->loggerLoader,"Loader: Select = %d",selectResult);
		//printf("after select: %d\n",  1111);
		if (selectResult == -1){
			log_error(self->loggerLoader, "Error en el select del Loader.");
			exit(1);
		}

		else if (selectResult == 0){
			log_error(self->loggerLoader, "Error en el select del Loader: selectResult==0 ");
		}

		else{

			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer

				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					log_info(self->loggerLoader,"Se encontraron datos en el elemento de la lista i = %d, descriptorEscucha = %d",i,socketEscucha->descriptor);

					if(i == socketEscucha->descriptor){  //gestionar nuevas conexiones

						if((socketNuevaConexion = socket_acceptClient(socketEscucha)) == 0)
							log_error(self->loggerLoader, "Loader: Error en el accept");

						else{
							log_debug(self->loggerLoader, "Loader: Accept completo!");
							atenderNuevaConexionPrograma(self, socketNuevaConexion, &master, &fdmax);
						}

					}

					else{ //sino no es una nueva conexion busca un programa en la lista

						log_debug(self->loggerLoader, "Loader: Mensaje del Programa descriptor = %d.", i);
						t_programaEnKernel* programaCliente = obtenerProgramaConsolaSegunDescriptor(self,i);
						log_debug(self->loggerLoader, "Loader: Mensaje del Programa PID = %d.", programaCliente->programaTCB->pid);
						atienderProgramaConsola(self,programaCliente, &master);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}




//Busca una conexion ya existente
t_programaEnKernel* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor){

	log_info(self->loggerLoader,"Loader: Obteniendo Programa descriptor buscado %d", descriptor);

	bool _esProgramaDescriptor(t_programaEnKernel* programa) {
		return (programa->socketProgramaConsola->descriptor == descriptor);
	}

	t_programaEnKernel* programaBuscado = list_find(listaDeProgramasDisponibles, (void*)_esProgramaDescriptor);

	log_info(self->loggerLoader,"Loader: Se encontro programa %d", programaBuscado->programaTCB->pid);

	return programaBuscado;
}





//En esta funcion hay que definir bien las cosas por eso la comente
//ver que pasa cuando se quiere trabajar con un cliente pre existen
//solo se me ocurre que es para detectar la desconexion de un programaBESO nada mas!
void atienderProgramaConsola(t_kernel* self,t_programaEnKernel* programa, fd_set* master){
	/*
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
		if(paquete->header.type == FINALIZAR_PROGRAMA_EXITO){//FINALIZAR_PROGRAMA no lo toma _MMMM..
			log_info(self->loggerLoader, "Loader:Llego con exito el FINALIZAR_PROGRAMA del programa %d", programa->programaTCB.pid);
		}else{
			log_error(self->loggerLoader, "Loader: Tipo de mensaje enviado por el Programa %d no identificado.", programa->programaTCB.pid);
		}
	}
	free(paquete);*/
}


//Esta funcion tiene que mandar crear los TCB con ayuda de la msp y luego ponerlo en la cola de nuevo

//primero el programaBeso le manda al la Loadear HANDSHAKE_PROGRAMA
//segundo el programaBeso se bloquea esperando una nuespuesta, en este caso el loader manda
//tercero el programa

void atenderNuevaConexionPrograma(t_kernel* self, t_socket* socketNuevoCliente, fd_set* master, int* fdmax){

	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerLoader, "Loader: Error o conexión cerrada por el Cliente.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	}

	else{

		if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_LOADER, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola HANDSHAKE_LOADER");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar los datos de la Consola.");

		//se recibe el codigo del archivo Beso
		t_socket_header header;

		if(recv(socketNuevoCliente->descriptor, &header, sizeof(t_socket_header), NULL) != sizeof(t_socket_header))
			log_error(self->loggerLoader, "Loader: No se ha podido recibir la informacion de la Consola");

		int sizePrograma = header.length - sizeof(t_socket_header);
		char *programaBeso = malloc(sizePrograma);
		memset(programaBeso, 0, sizePrograma + 1);

		if(recv(socketNuevoCliente->descriptor, programaBeso, sizePrograma, NULL) != sizePrograma)
			log_error(self->loggerLoader, "Loader: No se ha podido recibir el programa Beso de la Consola");

		else
			log_info(self->loggerLoader,"Loader: Recibio un programaBeso de Consola: %s", programaBeso);

		t_TCB_Kernel* unTCBenLoader = loaderCrearTCB(self, programaBeso, socketNuevoCliente, sizePrograma);
		log_info(self->loggerLoader, "Loader: TCB completo.");

		//al TCB se lo agrega al final de la Cola NEW con su socket correspondiente
		t_programaEnKernel *unPrograma = malloc(sizeof(t_programaEnKernel));
		unPrograma->programaTCB = unTCBenLoader;
		unPrograma->socketProgramaConsola = socketNuevoCliente;

		sem_wait(&mutex_new);
		list_add(cola_new,unPrograma);
		list_add(listaDeProgramasDisponibles,unPrograma);
		sem_post(&mutex_new);
		log_info(self->loggerLoader,"Loader: Agrego un elemento a la Cola New con el PID:%d  TID:%d ", unTCBenLoader->pid, unTCBenLoader->tid);
		sem_post(&mutex_BloqueoPlanificador);


		//Luego se tiene que actualizar las lista que se usan en el select
		FD_SET(socketNuevoCliente->descriptor, master); /*añadir al conjunto maestro*/
		if (socketNuevoCliente->descriptor > *fdmax) {
			log_info(self->loggerLoader, "Se actualiza y añade el conjunto maestro %d", socketNuevoCliente->descriptor);
			*fdmax = socketNuevoCliente->descriptor; /*actualizar el máximo*/
		}

		//free(unPrograma);
		//free(programaBeso);
	}//fin de else

	socket_freePaquete(paquete);
}


//Tiene que hacer el HANDSHAKE con la MSP
//Para luego crear segmento y escribir en memoria

t_TCB_Kernel* loaderCrearTCB(t_kernel* self, char *programaBeso, t_socket* socketNuevoCliente, int tamanioBeso){

	t_TCB_Kernel* unTCB = malloc(sizeof(t_TCB_Kernel));

	unTCB->pid = unPIDGlobal;
	unTCB->tid = unTIDGlobal;
	unTCB->km = 0;

	unPIDGlobal ++;
	unTIDGlobal ++;

	unTCB->tamanio_segmento_codigo = tamanioBeso;
	unTCB->base_segmento_codigo = kernelCrearSegmento(self, unTCB->pid, tamanioBeso);
	unTCB->puntero_instruccion = unTCB->base_segmento_codigo;
	//funciona de escribir en memoria
	int unaRespuesta = kernelEscribirMemoria(self, unTCB->pid, unTCB->base_segmento_codigo, programaBeso, tamanioBeso, self->socketMSP->socket);

	//Validar check de ERROR y si hay un error mandar a ProgramaBeso
	loaderValidarEscrituraEnMSP(self,socketNuevoCliente,unaRespuesta);


	unTCB->base_stack = kernelCrearSegmento(self, unTCB->pid, self->tamanioStack);
	unTCB->cursor_stack = unTCB->base_stack;
	log_info(self->loggerLoader,"Loader: La direccion de base de stack es: %0.8p para el PID: %d y TID:%d", self->tcbKernel->base_stack, self->tcbKernel->pid, self->tcbKernel->tid);
	unTCB->registro_de_programacion[0]=0;
	unTCB->registro_de_programacion[1]=0;
	unTCB->registro_de_programacion[2]=0;
	unTCB->registro_de_programacion[3]=0;
	unTCB->registro_de_programacion[4]=0;
	return unTCB;
}

void loaderValidarEscrituraEnMSP(t_kernel* self, t_socket* socketNuevoCliente, int unaRespuesta){

	switch(unaRespuesta){

	case ERROR_POR_TAMANIO_EXCEDIDO:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_TAMANIO_EXCEDIDO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_MEMORIA_LLENA:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_MEMORIA_LLENA, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_MEMORIA_LLENA");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_NUMERO_NEGATIVO:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_NUMERO_NEGATIVO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_NUMERO_NEGATIVO");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_SEGMENTO_INVALIDO:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_SEGMENTO_INVALIDO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_SEGMENTO_INVALIDO");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_TAMANIO_EXCEDIDO");
		break;
	case ERROR_POR_SEGMENTATION_FAULT:
		if (socket_sendPaquete(socketNuevoCliente, ERROR_POR_TAMANIO_EXCEDIDO, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: Envia a Consola: ERROR_POR_SEGMENTATION_FAULT");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar a Consola: ERROR_POR_SEGMENTATION_FAULT");
		break;
	default:
		log_info(self->loggerLoader, "Loader: Escribio en la MSP, correctamente");
		break;

	}// fin del switch

}
