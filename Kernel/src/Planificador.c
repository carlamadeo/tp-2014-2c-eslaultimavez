#include "planificadorMensajesCPU.h"
#include "commons/protocolStructInBigBang.h"
#include <errno.h>
#include <unistd.h>

//t_list* cola_new;
//t_list* cola_ready;
int idCPU=250;


void kernel_comenzar_Planificador(t_kernel* self){

	iretThreadPlanificador = pthread_create( &hiloAtiendeCPU, NULL, (void*) planificadorEscucharConexionesCPU, self);
	if(iretThreadPlanificador) {
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
		exit(EXIT_FAILURE);
	}
	iretThreadPlanificador = pthread_create( &hiloMandaEjectutarTCB, NULL, (void*) pasarTCB_Ready_A_Exec, self);
	if(iretThreadPlanificador) {
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iretThread);
		exit(EXIT_FAILURE);
	}
	pthread_join(hiloAtiendeCPU, NULL);
	pthread_join(hiloMandaEjectutarTCB, NULL);

}

void pasarTCB_Ready_A_Exec(t_kernel* self){

	log_debug(self->loggerPlanificador,"Planificador: Comienza a ejecutarse Hilo de Ready a Exec.");

	while(1){

		sem_wait(&sem_ready);
		sem_wait(&mutex_ready);//se bloquea la cola_READY hasta que tenga un TCB cargado
		log_info(self->loggerPlanificador,"Planificador: Se encuentra un proceso en ready");
		sem_wait(&mutex_exec);//BLOQUEO LISTA DE EXEC

		sem_wait(&mutex_cpuLibre);//se bloquea hasta que haya una CPU libre
		sem_wait(&mutex_cpuExec);//se bloquea lista de CPU disponibles
		log_info(self->loggerPlanificador,"Planificador: Se encuentra una CPU libre");

		log_info(self->loggerPlanificador,"Planificador:tamanio de la cola ready: %d",list_size(cola_ready));
		t_programaEnKernel* programaParaExec = list_remove(cola_ready, 0); //se remueve el primer elemento de la cola READY
		log_info(self->loggerPlanificador,"Planificador: Mando a ejecutar el proceso Beso con  PID:%d TID:%d KM:%d",programaParaExec->programaTCB->pid,programaParaExec->programaTCB->tid,programaParaExec->programaTCB->km);

		list_add(cola_exec, programaParaExec); // se agrega el programa en cola EXEC

		log_info(self->loggerPlanificador,"Planificador:cantidad de exec %d",list_size(cola_exec));
		log_info(self->loggerPlanificador,"Planificador:cantidad CPU LIBRE = %d / CPU EXEC = %d",list_size(listaDeCPULibres),list_size(listaDeCPUExec));


		sem_wait(&mutex_cpuLibre);
		t_cpu* cpuLibre = malloc(sizeof(t_cpu));
		cpuLibre = list_remove(listaDeCPULibres,0);
		//log_info(self->loggerPlanificador,"PID::: %d",programaParaExec->programaTCB->pid);
		cpuLibre->TCB = programaParaExec->programaTCB;
		list_add(listaDeCPUExec,cpuLibre);
		log_info(self->loggerPlanificador,"Planificador: cantidad CPU LIBRE = %d / CPU EXEC = %d",list_size(listaDeCPULibres),list_size(listaDeCPUExec));

		mandarEjecutarPrograma(self,programaParaExec, cpuLibre->socket);

		sem_post(&mutex_ready);   // se desbloquea cola READY
		sem_post(&mutex_exec);    // se desbloquea cola EXEC
		sem_post(&mutex_cpuLibre);// se desbloquea cola CPU LIBRE
		sem_post(&mutex_cpuExec); // se desbloquea cola CPU OCUPADAS
	}
}

void mandarEjecutarPrograma(t_kernel* self,t_programaEnKernel* programa, t_socket* socketCPU){

	log_info(self->loggerPlanificador,"TEST 1");
	t_TCB_Kernel* unTCBaEXEC = malloc (sizeof(t_TCB_Kernel));
	unTCBaEXEC = programa->programaTCB;
	log_info(self->loggerPlanificador,"PID = %d CURSOR POINTER= %d",programa->programaTCB->pid,programa->programaTCB->puntero_instruccion);

	log_info(self->loggerPlanificador,"TEST 2");

	t_QUANTUM* unQuantum = malloc(sizeof(t_QUANTUM));
	unQuantum->quantum = self->quantum;

	//se manda un QUANTUM a CPU
	socket_sendPaquete(socketCPU, QUANTUM, sizeof(t_QUANTUM), unQuantum);
	log_debug(self->loggerPlanificador, "Planificador: envia un quantum: %d", unQuantum->quantum);

	log_info(self->loggerPlanificador,"TEST 3");


	if(socket_sendPaquete(socketCPU, TCB_NUEVO, sizeof(t_TCB_Kernel), unTCBaEXEC) > 0){ ////CPU_NUEVO_PCB
		log_info(self->loggerPlanificador,"Planificador: envio el TCB  a ejecutar en una CPU");
	} else {
		log_info(self->loggerPlanificador,"Planificador: error el enviar a ejecutar un TCB a CPU");
	}

	log_info(self->loggerPlanificador,"TEST 4");
}

void planificadorEscucharConexionesCPU(t_kernel* self){
	listaSystemCall = list_create();
	listaDeCPULibres = list_create();
	t_socket *socketNuevaConexionCPU;
	//listaCpu =
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(self->socketCPU = socket_createServer(self->puertoPlanificador))){
		log_error(self->loggerPlanificador, "Planificador: Error al crear un socket para escuchar CPUs. %s", strerror(errno));
	}

	if(!socket_listen(self->socketCPU)){
		log_error(self->loggerPlanificador, "Planificador:Error al poner a escuchar al Planificador: %s", strerror(errno));
	}

	log_info(self->loggerPlanificador, "Planificador: Ya se esta escuchando conexiones entrantes en el puerto: %d",self->puertoPlanificador);

	FD_SET(self->socketCPU->descriptor, &master);
	fdmax = self->socketCPU->descriptor; /* seguir la pista del descriptor de fichero mayor*/


	/* bucle principal*/
	while(1){
		read_fds = master;
		//printf("antes select: %d\n",  1111);
		int selectResult = select(fdmax+1, &read_fds, NULL, NULL, NULL);
		log_info(self->loggerPlanificador,"Planificador: Select= %d",selectResult);
		//printf("after select: %d\n",  1111);
		if (selectResult == -1){
			log_error(self->loggerPlanificador, "Planificador: Error en el select del Planificador.");
			exit(1);
		}else if (selectResult == 0){

		} else{
			for(i = 0; i <= fdmax; i++){ //explorar conexiones existentes en busca de datos que leer
				if (FD_ISSET(i, &read_fds)){ //¡¡tenemos datos!!

					log_info(self->loggerPlanificador," Planificador:Se encontraron datos en el elemento de la lista i=%d, descriptorEscucha=%d",i,self->socketCPU->descriptor);


					if(i == self->socketCPU->descriptor){  //gestionar nuevas conexiones
						if((socketNuevaConexionCPU = socket_acceptClient(self->socketCPU))==0) {
							log_error(self->loggerPlanificador, "Planificador: Error en el accep  Planificador");
						}else  {
							log_debug(self->loggerPlanificador, "Planificador: Accept completo! ");
							atenderNuevaConexionCPU(self, socketNuevaConexionCPU, &master, &fdmax);
						}

					}else{ //sino no es una nueva conexion busca un programa en la lista
						log_debug(self->loggerPlanificador, "Planificador:Mensaje del Programa descriptor= %d.", i);
						t_cpu* cpuCliente = obtenerCPUSegunDescriptor(self,i);
						log_debug(self->loggerPlanificador, "Planificador: Mensaje del CPU: %d", cpuCliente->socket->descriptor);
						atenderCPU(self,socketNuevaConexionCPU,cpuCliente, &master);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}


void atenderNuevaConexionCPU(t_kernel* self, t_socket* socketNuevoCliente, fd_set* master, int* fdmax){

	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerPlanificador, "Planificador:Conexión cerrada con el CPU.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	}else{
		//Si recibe una señar de la CPU hace el
		if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_PLANIFICADOR, 0, NULL) >= 0){
			log_info(self->loggerPlanificador, "Planificador: envia HANDSHAKE_PLANIFICADOR.");

			idCPU++;
			//sem_wait(&mutex_cpuLibre);
			agregarEnListaDeCPU(self,idCPU, socketNuevoCliente);
			//sem_post(&mutex_cpuLibre);
		}
		else
			log_error(self->loggerPlanificador, "Planificador: Error en el HANDSHAKE_PLANIFICADOR con la CPU.");
	}

	//Se actualiza del select
	FD_SET(socketNuevoCliente->descriptor, master); /*añadir al conjunto maestro*/
	if (socketNuevoCliente->descriptor > *fdmax) {
		log_info(self->loggerPlanificador, "Se actualiza y añade el conjunto maestro %d", socketNuevoCliente->descriptor);
		*fdmax = socketNuevoCliente->descriptor; /*actualizar el máximo*/
	}

	/*t_programaEnKernel* unTCBCOLA = obtenerTCBdeReady(self);

	if (unTCBCOLA!= NULL){
		t_TCB_Kernel* unTCBaCPU = malloc(sizeof(t_TCB_Kernel));
		unTCBaCPU = unTCBCOLA->programaTCB;

		t_QUANTUM* unQuantum = malloc(sizeof(t_QUANTUM));
		unQuantum->quantum = self->quantum;

		//se manda un QUANTUM a CPU
		socket_sendPaquete(socketNuevoCliente, QUANTUM, sizeof(t_QUANTUM), unQuantum);
		log_debug(self->loggerPlanificador, "Planificador: envia un quantum: %d", unQuantum->quantum);

		//se mande un TCB a CPU
		socket_sendPaquete(socketNuevoCliente, TCB_NUEVO,sizeof(t_TCB_Kernel), unTCBaCPU);
		log_debug(self->loggerPlanificador, "Planificador: envia TCB_NUEVO con PID: %d TID:%d KM:%d", unTCBaCPU->pid,unTCBaCPU->tid,unTCBaCPU->km );

		printTCBKernel(unTCBaCPU);
		free(unQuantum);
		//free(unTCBaCPU);
	}*/

	socket_freePaquete(paquete);
}


void atenderCPU(t_kernel* self,t_socket *socketNuevaConexionCPU, t_cpu* cpu, fd_set* master){

	log_info(self->loggerPlanificador, "Planificador: LISTO PARA ATENDER CPUs" );

	t_socket_paquete *paqueteCPUAtendido = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	socket_recvPaquete(cpu->socket, paqueteCPUAtendido);

	printf("Valor para el switch: %d\n",paqueteCPUAtendido->header.type);

	switch(paqueteCPUAtendido->header.type){
	case CAMBIO_DE_CONTEXTO:
		ejecutar_UN_CAMBIO_DE_CONTEXTO(self, socketNuevaConexionCPU);
		break;
	case FINALIZAR_PROGRAMA_EXITO:
		ejecutar_FINALIZAR_PROGRAMA_EXITO(self, socketNuevaConexionCPU);
		break;
	case INTERRUPCION:
		ejecutar_UNA_INTERRUPCION(self, socketNuevaConexionCPU);
		break;
	default:
		//MENSAJE_DE_ERROR:
		ejecutar_UN_MENSAJE_DE_ERROR(self, socketNuevaConexionCPU,paqueteCPUAtendido->header.type);

		//esto es importante, va en otro lado
		//log_error(self->loggerPlanificador, "Planificador:Conexión cerrada con CPU.");
		//FD_CLR(cpu->socket->descriptor, master);
		//close(cpu->socket->descriptor);
		break;

	}//fin switch(paqueteCPU->header.type)

	socket_freePaquete(paqueteCPUAtendido);
}


t_cpu* obtenerCPUSegunDescriptor(t_kernel* self,int descriptor){

	log_info(self->loggerPlanificador,"Planificador: buscando el descriptor %d de un CPU",descriptor);
	bool _esCPUDescriptor(t_cpu* cpu) {
		return (cpu->socket->descriptor == descriptor);
	}
	t_cpu* cpuBuscado = list_find(listaDeCPUExec, (void*)_esCPUDescriptor);
	if(cpuBuscado!=NULL){
	}else{
		sem_wait(&mutex_cpuLibre);
		cpuBuscado = list_find(listaDeCPULibres, (void*)_esCPUDescriptor);
		sem_post(&mutex_cpuLibre);
	}
	log_info(self->loggerPlanificador,"Planificador: Se encontro CPU con descriptor: %d",cpuBuscado->socket->descriptor);

	//cpuBuscado->socket->descriptor = descriptor;
	return cpuBuscado;
}


t_programaEnKernel* obtenerTCBdeReady(t_kernel* self){

	//sem_wait(&mutex_BloqueoPlanificador);   //desbloquea al planificador!!! ERROR

	log_error(self->loggerPlanificador," Cantidad de elemento en la cola New: %d" ,list_size(cola_new));
	if (list_size(cola_new)>0){
		sem_wait(&mutex_new);
		t_programaEnKernel* programa = list_remove(cola_new, 0); //SE REMUEVE EL PRIMER PROGRAMA DE NEW
		sem_post(&mutex_new);

		sem_wait(&mutex_ready);
		list_add(cola_ready, programa);// SE AGREGA UN PROGRAMA EN READY
		sem_post(&mutex_ready);

		log_info(self->loggerPlanificador," Planificador: Obtiene un elemento de la Cola Ready con PID: %d y TID:%d" ,programa->programaTCB->pid,programa->programaTCB->tid );
		return programa;
	}

	return NULL;
}
//t_TCB_Kernel* test_TCB (){
//	t_TCB_Kernel* test_TCB = malloc(sizeof(t_TCB_Kernel));
//
//	test_TCB->pid= 0;
//	test_TCB->tid= 523;
//	test_TCB->km = 1;
//	test_TCB->base_segmento_codigo = 1048576;
//	test_TCB->tamanio_segmento_codigo = 1000;
//	test_TCB->base_stack = 2097152;
//	test_TCB->cursor_stack = 2097152;
//	test_TCB->puntero_instruccion = 1048576;
//	//test_TCB->registro_de_programacion = 56;
//
//
//	return test_TCB;
//}

void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa){
	sem_wait(&mutex_exit);//BLOQUEO LISTA DE EXIT
	list_add(cola_exit, programa);
	sem_post(&mutex_exit);
}
