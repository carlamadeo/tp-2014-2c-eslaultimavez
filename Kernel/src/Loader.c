
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
    int n=0;
	while(n<10){
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
							log_debug(self->loggerLoader, "Nueva conexion al Loader, ACCEP completo! ");
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
	n++;
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


void atenderNuevaConexionPrograma(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax)
{
	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));


	int id_procesosConsolas=0;
    if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerLoader, " Loader:Error o conexión cerrada por el Cliente correspondiente.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	} else {
		if(paquete->header.type == HANDSHAKE_PROGRAMA){
			int valorPrograma = *(int *) (paquete->data);
			log_info(self->loggerLoader, " Loader: Llego HANDSHAKE_PROGRAMA y contiene: %d",valorPrograma);
			id_procesosConsolas++;

			if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_LOADER,0, NULL) >= 0) {
				log_info(self->loggerLoader, " Loader:Se envió HANDSHAKE_LOADER correctamente");
			} else {
				log_error(self->loggerLoader, " Loader: Error enviar HANDSHAKE_LOADER");
			}

			t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
			if (socket_recvPaquete(socketNuevoCliente, paquete) < 0) {
				log_error(self->loggerLoader, "Error al recibir Solicitudes del Programa.");
				FD_CLR(socketNuevoCliente->descriptor, master);
				close(socketNuevoCliente->descriptor);
			} else {
				char * codigoEntrante = malloc(valorPrograma);
				log_info(self->loggerLoader, "Tamaño real del codigoEntrante: %d\n", sizeof(*codigoEntrante));
				codigoEntrante = (char*) (paquete->data);

				if(paquete->header.type == CAMBIO_DE_CONTEXTO ){// cambiar por CAMBIAR_PROCESO_ACTIVO
					log_info(self->loggerLoader, "Llego con exito el codigo del programa %d. Y ahora lo guardamos %s",id_procesosConsolas, codigoEntrante);
					//agregarEnListaDeProgramas(codigoEntrante, socketNuevoCliente, id_procesos, pesoDeCodigo, &master, &fdmax);
				} else {
					log_error(self->loggerLoader, "Tipo de mensaje enviado por el Programa %d no identificado.",id_procesosConsolas);
				}
			}


		}else{
			log_error(self->loggerLoader, "Tipo de mensaje enviado por el Programa no identificado.");
		}
	}
    free(paquete);
}


void loaderCrearTCB(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid){

	uint32_t stack;
	t_programaEnKernel* programaEnElKernel = malloc( sizeof(t_programaEnKernel) );
	log_info(self->loggerKernel, "Kernel: Crear un TCB.");

	//t_medatada_program* metadata = metadata_desde_literal(codigoPrograma);

	programaEnElKernel->TCB.pid=pid;
	programaEnElKernel->TCB.tid= tid;

	//ver porque rompre
	//programaEnElKernel->PCB.puntero_instruccion = metadata->instruccion_inicio;


	programaEnElKernel->TCB.base_segmento_codigo= kernelCrearSegmento(self,pid, tamanioEnBytes); //beso


	if(programaEnElKernel->TCB.base_segmento_codigo == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		//return NULL;
	}


	programaEnElKernel->TCB.base_stack = kernelCrearSegmento(self,pid, self->tamanioStack);
	if(programaEnElKernel->TCB.base_stack == -1){
		finalizarProgramaEnPlanificacion(programaEnElKernel);
		//return NULL;
	}

	programaEnElKernel->TCB.cursor_stack = programaEnElKernel->TCB.base_stack;

	//faltan todos los logs
	log_info(self->loggerKernel, "PID %d TID: %d\n",programaEnElKernel->TCB.pid, programaEnElKernel->TCB.tid);

	//return programaEnElKernel;
}


int loaderCrearSegmento(t_kernel* self,int pid, int tamanio){

	int direccionLogica;
	t_envio_num_EnKernel* datos = malloc(sizeof(t_envio_num_EnKernel));
	datos->num = tamanio;
	datos->pid = pid;
	t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, sizeof(t_envio_num_EnKernel), datos) > 0) {
		log_info(self->loggerKernel, "Kernel: Mando Tamaño del Segmento %d para el proceso %d!",datos->num,datos->pid);

		if(socket_recvPaquete(self->socketMSP->socket, paquete) >= 0){
			if(paquete->header.type == CREAR_SEGMENTO){
				datos = (t_envio_num_EnKernel*) paquete->data;
				direccionLogica = datos->num;
				//log_info(self->loggerKernel, "RECIBIDOS DATOS: PID=%d / Indice Segmento=%d", datos->pid, direccionLogica);
				log_info(self->loggerKernel, "RECIBIDOS DATOS");
			}
		}else{
			log_info(self->loggerKernel, "KERNEL: ERROR DATOS NO RECIBIDOS");
			return -1;
		}
	}

	free(paquete);
	free(datos);
	return direccionLogica;
}
