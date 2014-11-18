
#include "Loader.h"
#include "commons/protocolStructInBigBang.h"
#include <errno.h>

int unPIDGlobal = 0;
int unTIDGlobal = 0;


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
		log_error(self->loggerLoader, "Loader: Error al poner a escuchar al Loader: %s", strerror(errno));
	}

	log_info(self->loggerLoader, "Loader:escuchando conexiones entrantes en el puerto: %d",self->puertoLoader);

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
		}

		else if (selectResult == 0){

		}

		else{

			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer

				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					log_info(self->loggerLoader,"Se encontraron datos en el elemento de la lista i=%d, descriptorEscucha=%d",i,socketEscucha->descriptor);

					if(i == socketEscucha->descriptor){  //gestionar nuevas conexiones

						if((socketNuevaConexion = socket_acceptClient(socketEscucha)) == 0)
							log_error(self->loggerLoader, "Loader: Error en el accept");

						else{
							log_debug(self->loggerLoader, "Loader: Accept completo! ");
							atenderNuevaConexionPrograma(self, socketNuevaConexion, &master, &fdmax);
						}

					}

					else{ //sino no es una nueva conexion busca un programa en la lista

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




//Busca una conexion ya existente
t_programa* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor){

	log_info(self->loggerLoader,"Loader:Obteniendo Programa descriptor buscado %d",descriptor);

	bool _esProgramaDescriptor(t_programa* programa) {
		return (programa->socketProgramaConsola->descriptor == descriptor);
	}

	t_programa* programaBuscado = list_find(listaDeProgramas, (void*)_esProgramaDescriptor);

	log_info(self->loggerLoader,"Loader: Se encontro programa %d",programaBuscado->programaTCB.pid);

	return programaBuscado;
}





//En esta funcion hay que definir bien las cosas por eso la comente
//ver que pasa cuando se quiere trabajar con un cliente pre existen
//solo se me ocurre que es para detectar la desconexion de un programaBESO nada mas!
void atienderProgramaConsola(t_kernel* self,t_programa* programa, fd_set* master){
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
		log_error(self->loggerLoader, "Loader: Error o conexión cerrada por el Cliente correspondiente.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	}

	else{

		if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_LOADER, 0, NULL) >= 0)
			log_info(self->loggerLoader, "Loader: enva a Consola: HANDSHAKE_LOADER");
		else
			log_error(self->loggerLoader, "Loader: Error al enviar los datos de la Consola.");

		//se recibe el codigo del archivo Beso
		t_socket_header header;

		//printf("se pone a esperar: \n");
		if(recv(socketNuevoCliente->descriptor, &header, sizeof(t_socket_header), NULL) != sizeof(t_socket_header))
			log_error(self->loggerLoader, "Loader: No se ha podido recibir la informacion de la Consola");

		int sizePrograma = header.length - sizeof(t_socket_header);
		//printf("sizePrograma: %d\n",sizePrograma);
		char *programaBeso = malloc(sizePrograma);
		//printf("programaBeso: %c\n",programaBeso);
		memset(programaBeso, 0, sizePrograma + 1);//Rompe en esta Linea
		//printf("programaBeso tranquilo: %c\n",programaBeso);
		if(recv(socketNuevoCliente->descriptor, programaBeso, sizePrograma, NULL) != sizePrograma)
			log_error(self->loggerLoader, "Loader: No se ha podido recibir el programa Beso de la Consola");

		log_info(self->loggerLoader," Loader:  recibio un programaBeso de Consola:  %s ", programaBeso);
		t_TCB_Kernel* unTCBenLoader = loaderCrearTCB(self, programaBeso, socketNuevoCliente);
		log_info(self->loggerLoader, "Loader: TCB completo.");

		//LUEGO EN ESTA PARTE PONERLO AL FINAL DE LA COLA NEW

	}

	free(paquete);
}




//Tiene que hacer el HANDSHAKE con la MSP
//Para luego crear segmento y escribir en memoria

t_TCB_Kernel* loaderCrearTCB(t_kernel* self, char *programaBeso, t_socket* socketNuevoCliente){

	t_TCB_Kernel* unTCB = malloc(sizeof(t_TCB_Kernel));

	unTCB->pid = unPIDGlobal;
	unTCB->tid = unTIDGlobal;
	unTCB->km = 0;

	unPIDGlobal ++;
	unTIDGlobal ++;
	//printf("sizePrograma:\n");
	//Si no esta levantada la MSP, tiene que romper!
	socket_sendPaquete(self->socketMSP->socket, HANDSHAKE_KERNEL,0, NULL);
	log_info(self->loggerLoader,"Loader: envia a MSP: HANDSHAKE_KERNEL.");


	t_socket_paquete *paqueteMSP = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));

	if (socket_recvPaquete(self->socketMSP->socket, paqueteMSP) >= 0){

		if(paqueteMSP->header.type == HANDSHAKE_MSP){

			log_info(self->loggerLoader,"Loader: Se recibo HANDSHAKE_MSP");

			//funciona que devuelte la base del segmente de codigo
			t_CrearSegmentoBeso* baseCodigoDeSegmento = malloc(sizeof(t_CrearSegmentoBeso));

			baseCodigoDeSegmento->tamanio = strlen(programaBeso); // ver que esto este bien!!!
			baseCodigoDeSegmento->pid = unTCB->pid;
			unTCB->base_segmento_codigo = loaderCrearSegmento(self,baseCodigoDeSegmento);

			//funciona de escribir en memoria
			int unaRespuesta = loaderEscribirMemoria(self, unTCB, programaBeso, socketNuevoCliente);

			//Validar check de ERROR y si hay un error mandar a ProgramaBeso
			switch(unaRespuesta){
			case ERROR_POR_TAMANIO_EXCEDIDO:

				break;
			case ERROR_POR_MEMORIA_LLENA:

				break;
			case ERROR_POR_NUMERO_NEGATIVO:

				break;
			case ERROR_POR_SEGMENTO_INVALIDO:

				break;
			case ERROR_POR_SEGMENTATION_FAULT:

				break;
			default:
				//log sin errrores
				break;

			}// fin del switch


			//falta ahora hacer el stack

			//funciona que devuelte la base del segmente de codigo
			t_CrearSegmentoBeso* baseCodigoDeStack = malloc(sizeof(t_CrearSegmentoBeso));

			baseCodigoDeStack->tamanio = self->tamanioStack; // ver que esto este bien!!!
			baseCodigoDeStack->pid = unTCB->pid;
			unTCB->base_stack = loaderCrearSegmento(self,baseCodigoDeStack);

		}// fin del if HANDSHAKE_MSP


	}else
		log_error(self->loggerLoader, "Loader: Error con la MSP.");

	free(paqueteMSP);

	return unTCB;
}


uint32_t loaderCrearSegmento(t_kernel* self, t_CrearSegmentoBeso* codigo){

	t_socket_paquete *paqueteCodigo = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_msp_DireccionBase* unaDireccionBase = (t_msp_DireccionBase *)malloc(sizeof(t_msp_DireccionBase));

	socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO,sizeof(t_CrearSegmentoBeso), codigo);

	socket_recvPaquete(self->socketMSP->socket, paqueteCodigo);

	unaDireccionBase = (t_msp_DireccionBase *) paqueteCodigo->data;

	log_info(self->loggerLoader, "Loader: Recibe Base de la MSP  %0.8p ", unaDireccionBase);

	// se valida afuera de la funcion
	free(paqueteCodigo);
	return unaDireccionBase->direccionBase;
}


int loaderEscribirMemoria(t_kernel* self,t_TCB_Kernel* unTCB, char *programaBeso, t_socket* socketNuevoCliente){

	t_EscribirSegmentoBeso* escrituraDeCodigo = malloc(sizeof(t_EscribirSegmentoBeso));
	t_socket_paquete *paqueteConfirmacionEscritura = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	t_confirmacionEscritura *unaConfirmacionEscritura = (t_confirmacionEscritura *)malloc(sizeof(t_confirmacionEscritura));

	escrituraDeCodigo->direccionVirtual = unTCB->base_segmento_codigo ;
	escrituraDeCodigo->pid = 545;
	escrituraDeCodigo->tamanio = sizeof(programaBeso); //ver que de donde sale!!!

	memcpy(escrituraDeCodigo->bufferCodigoBeso, programaBeso, strlen(programaBeso)); //importante, ver si tiene o no el /0

	socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA,sizeof(t_EscribirSegmentoBeso), escrituraDeCodigo);

	socket_recvPaquete(socketNuevoCliente, paqueteConfirmacionEscritura);

	unaConfirmacionEscritura = (t_confirmacionEscritura *) paqueteConfirmacionEscritura->data;

	switch(unaConfirmacionEscritura->estado){

	case ERROR_POR_SEGMENTATION_FAULT:

		break;
	default:

		break;

	}

	return unaConfirmacionEscritura->estado;
}
