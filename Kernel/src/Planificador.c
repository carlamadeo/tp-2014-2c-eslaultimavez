#include "Planificador.h"
#include "kernelConfig.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "commons/protocolStructInBigBang.h"
#include "commons/socketInBigBang.h"

void kernel_comenzar_Planificador(t_kernel* self){

	planificadorEscuchaCPU(self);
}


void planificadorEscuchaCPU(t_kernel* self){
	t_socket *socketEscucha, *socketNuevaConexion;
	listaCpu = list_create();
	fd_set master;   //conjunto maestro de descriptores de fichero
	fd_set read_fds; //conjunto temporal de descriptores de fichero para select()
	int fdmax,i;

	FD_ZERO(&master); /* borra los conjuntos maestro y temporal*/
	FD_ZERO(&read_fds);

	if (!(socketEscucha = socket_createServer(self->puertoPlanificador))){
		log_error(self->loggerPlanificador, "Planificador: Error al crear un socket para escuchar CPUs. %s", strerror(errno));
	}

	if(!socket_listen(socketEscucha)){
		log_error(self->loggerPlanificador, "Planificador:Error al poner a escuchar al Planificador: %s", strerror(errno));
	}

	log_info(self->loggerPlanificador, "Planificador: Ya se esta escuchando conexiones entrantes en el puerto: %d",self->puertoPlanificador);

	FD_SET(socketEscucha->descriptor, &master);
	fdmax = socketEscucha->descriptor; /* seguir la pista del descriptor de fichero mayor*/

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

					log_info(self->loggerPlanificador," Planificador:Se encontraron datos en el elemento de la lista i=%d, descriptorEscucha=%d",i,socketEscucha->descriptor);


					if(i == socketEscucha->descriptor){  //gestionar nuevas conexiones
						if((socketNuevaConexion = socket_acceptClient(socketEscucha))==0) {
							log_error(self->loggerPlanificador, "Loader: Error en el accep  Planificador");
						}else  {
							log_debug(self->loggerPlanificador, "Loader: ACCEP completo! ");
							atenderNuevaConexionCPU(self, socketNuevaConexion, &master, &fdmax);
						}

					}else{ //sino no es una nueva conexion busca un programa en la lista
						log_debug(self->loggerPlanificador, "Planificador:Mensaje del Programa descriptor= %d.", i);
						t_cpu* cpuCliente = obtenerCPUSegunDescriptor(self,i);
						log_debug(self->loggerPlanificador, "Planificador: Mensaje del CPU.", cpuCliente->id);
						atienderCPU(self,cpuCliente, &master);
					}
				}//fin del if FD_ISSET
			}// fin del for de las i
		}//Fin del else grande

	}//fin de while(1)
}


void atenderNuevaConexionCPU(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax){

	t_socket_paquete *paquete = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	if ((socket_recvPaquete(socketNuevoCliente, paquete)) < 0) {
		log_error(self->loggerPlanificador, "Planificador:Conexión cerrada con el CPU.");
		FD_CLR(socketNuevoCliente->descriptor, master);
		close(socketNuevoCliente->descriptor);
	}else{
		//Si recibe una señar de la CPU hace el
		if (socket_sendPaquete(socketNuevoCliente, HANDSHAKE_PLANIFICADOR, 0, NULL) >= 0)
			log_info(self->loggerPlanificador, "Planificador: envia HANDSHAKE_LOADER.");
		else
			log_error(self->loggerPlanificador, "Planificador: Error en el HANDSHAKE_LOADER con la CPU.");
	}

	//se tiene que mandar un TCB
	t_TCB_Kernel* unTCB = test_TCB();//esta funcion tiene que recibir un TCB del LOADER

	socket_sendPaquete(socketNuevoCliente, TCB_NUEVO,sizeof(t_TCB_Kernel), unTCB);
	log_info(self->loggerPlanificador, "Planificador: envia TCB_NUEVO.");
	free(paquete);


	//mientras no termine los quamtum o no alla alla interrupcion


	while (self->quamtum>0){
		self->quamtum--;
		t_socket_paquete *paqueteCPU = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
		socket_recvPaquete(socketNuevoCliente, paqueteCPU);


		switch(paqueteCPU->header.type){
		case CPU_TERMINE_UNA_LINEA:
			if (self->quamtum>0){
				socket_sendPaquete(socketNuevoCliente, CPU_SEGUI_EJECUTANDO,0, NULL);
				log_info(self->loggerPlanificador, "Planificador: envia CPU_SEGUI_EJECUTANDO.");

			}else{
				socket_sendPaquete(socketNuevoCliente, KERNEL_FIN_TCB_QUANTUM,0, NULL);
				log_info(self->loggerPlanificador, "Planificador: envia KERNEL_FIN_TCB_QUANTUM.");
			}
			break;

		default:
			log_error(self->loggerPlanificador, "Planificador:Conexión cerrada con CPU.");
			FD_CLR(socketNuevoCliente->descriptor, master);
			close(socketNuevoCliente->descriptor);
			break;

		}//fin switch(paqueteCPU->header.type)
	}//fin del while

	//Cuando sale del while(self->quamtum>0) se tiene que hacer un cambio de Contexto

	t_socket_paquete *paqueteCambioDeContexto = (t_socket_paquete *)malloc(sizeof(t_socket_paquete));
	socket_recvPaquete(socketNuevoCliente, paqueteCambioDeContexto);
	log_info(self->loggerPlanificador, "Planificador: recibe de  CPU: CAMBIO_DE_CONTEXTO.");
	free(paqueteCambioDeContexto);
}

void atienderCPU(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master){



}


t_cpu* obtenerCPUSegunDescriptor(t_kernel* self,int descriptor){

	log_info(self->loggerPlanificador,"Planificador:Obteniendo CPU segun el descriptor %d",descriptor);

	bool _esProgramaDescriptor(t_cpu* cpu) {
		return (cpu->socket->descriptor == descriptor);
	}
	t_cpu* cpuBuscado = list_find(listaCpu, (void*)_esProgramaDescriptor);
	log_info(self->loggerPlanificador," Planificador: Se encontro programa %d",cpuBuscado->id);
	return cpuBuscado;
}

t_TCB_Kernel* test_TCB (){
	t_TCB_Kernel* test_TCB = malloc(sizeof(t_TCB_Kernel));

	test_TCB->pid= 0;
	test_TCB->tid= 523;
	test_TCB->km = 1;
	test_TCB->base_segmento_codigo = 1025;
	test_TCB->tamanio_segmento_codigo = 1000;
	test_TCB->base_stack = 576;
	test_TCB->cursor_stack = 579;
	test_TCB->puntero_instruccion = 1;
	//test_TCB->registro_de_programacion = 56;



	return test_TCB;
}
