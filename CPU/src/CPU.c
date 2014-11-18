/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU.h"
#include "cpu_to_msp.h"
#include "cpu_to_kernel.h"
#include "codigoESO.h"



int main(int argc, char** argv) {

	verificar_argumentosCPU(argc, argv);
	char* config_file = argv[1];
	t_CPU* self = cpu_cargar_configuracion(config_file);

	self->loggerCPU = log_create("logCPU.log", "CPU", 1, LOG_LEVEL_DEBUG);
	self->socketMSP  = cpuConectarConMPS(self);
	self->socketPlanificador = conectarCPUConKernel(self);

	while(1){
		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){
			if( paquete->header.type == TCB_NUEVO ){
				self->tcb=(t_TCB_CPU*)paquete->data;
				cpuProcesar_tcb(self);
			} else {
				log_error(self->loggerCPU, "Se recibio un codigo inesperado de KERNEL en main de cpu: %d", paquete->header.type);
			}
		} else {
			log_info(self->loggerCPU, "CPU: Error al recibir un paquete.");
			exit(-1);
		}

		free(paquete);
		usleep(100);
	}//Fin del while 1



	/* De aca en adelante, se tiene que eliminar porque se desconecto la CPU */

	log_info(self->loggerCPU, "Se desconecto la CPU. Elimino todo");
	exit(0);

	close(socketDelKernel->descriptor);
	close(socketDelMSP->descriptor);
}





void cpuProcesar_tcb(t_CPU* self){

	log_info(self->loggerCPU, "CPU: Comienzo a procesar el TCB de pid:\n %d", self->tcb->pid);

	/*pregunto por KM del tcb a ejecutar*/
	int seguir_ejecucion=1;
	while(seguir_ejecucion==1){
		//pido los primeros 4 bytes especificados por PID+Puntero_Instruccion

		t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
		unCPU_LEER_MEMORIA->pid = self->tcb->pid;
		unCPU_LEER_MEMORIA->tamanio = self->tcb->tamanio_segmento_codigo;
		unCPU_LEER_MEMORIA->direccionVirtual = self->tcb->puntero_instruccion;
		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_CPU_LEER_MEMORIA), unCPU_LEER_MEMORIA)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP\n %d", self->tcb->pid);
			//cpuCambioDeConextoError();
		}

		log_info(self->loggerCPU, "CPU: envio a MSP: LEER_MEMORIA\n");

		t_socket_paquete *paquete_MSP = malloc(sizeof(t_socket_paquete));
		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: recibo un LEER_MEMORIA:\n %d ", paquete_MSP->header.type);
				/*reservo memoria en una variable linea para guardar los 4 caracteres de ESO*/
				char *nombre_instruccion = malloc(sizeof(char)*4);
				/*guardo en la variable linea los 4 caracteres que forman el nombre de la instruccion ESO*/
				int doffset=0, dtemp_size=0;
				memcpy(nombre_instruccion, paquete_MSP->data, dtemp_size=sizeof(char));
				doffset=dtemp_size;
				memcpy(nombre_instruccion + doffset, paquete_MSP->data + doffset, dtemp_size=sizeof(char));
				doffset+=dtemp_size;
				memcpy(nombre_instruccion + doffset, paquete_MSP->data + doffset, dtemp_size=sizeof(char));
				doffset+=dtemp_size;
				memcpy(nombre_instruccion + doffset, paquete_MSP->data + doffset, dtemp_size=sizeof(char));

				char instrucciones_eso[]={'L','O','A','D','G','E','T','M','S','E','T','M','M','O','V','R','A','D','D','R','S','U','B','R','M','U','L','R','M','O','D','R','D','I','V','R','I','N','C','R','D','E','C','R','C','O','M','P','C','G','E','Q','C','L','E','Q','G','O','T','O','J','M','P','Z','J','N','P','Z','I','N','T','E','S','H','I','F','N','O','P','P','P','U','S','H','T','A','K','E','X','X','X','X','M','A','L','C','F','R','E','E','I','N','N','N','I','N','N','C','O','U','T','N','O','U','T','C','C','R','E','A','J','O','I','N','B','L','O','K','W','A','K','E','\0'};
				int instruccion=1, indice=0;
				while (instrucciones_eso[indice]!='\0'){
					if (strncmp( &(instrucciones_eso[indice]), nombre_instruccion,1 )==0){
						indice++;
						if(strncmp( &(instrucciones_eso[indice]), nombre_instruccion+1, 1 )==0){
							indice++;
							if(strncmp( &(instrucciones_eso[indice]), nombre_instruccion+2, 1 )==0){
								indice++;
								if(strncmp( &(instrucciones_eso[indice]), nombre_instruccion+3, 1 )==0){
									ejecutar_instruccion(instruccion, self->tcb);
									break;
								}else{
									indice++;
								}
							}else{
								indice+=2;
							}
						}else{
							indice+=3;
						}
					}else{
						indice+=4;
					}
					instruccion++;
				} //fin de while

			} else {
				log_error(self->loggerCPU, "CPU: recibio un codigo inesperado de la MSP:\n %d", paquete_MSP->header.type);

			}
		}else{
			log_info(self->loggerCPU, "CPU: ha cerrado su conexion dentro del procesar TCB\n");
			exit(-1);
		}

		free(paquete_MSP);
		//Sleep para que no se tilde
		usleep(100);


		t_CPU_TERMINE_UNA_LINEA* unTerminoLinea = malloc(sizeof(t_CPU_TERMINE_UNA_LINEA));
		unTerminoLinea->pid=self->tcb->pid;
		unTerminoLinea->tid=self->tcb->tid;
		if (socket_sendPaquete(self->socketPlanificador->socket, CPU_TERMINE_UNA_LINEA, sizeof(t_CPU_TERMINE_UNA_LINEA), unTerminoLinea)<=0){
			log_info(self->loggerCPU, "CPU: fallo pedido de datos de Consola\n %d", self->tcb->pid);
		}

		log_info(self->loggerCPU, "CPU: envio al Planificador CPU_TERMINE_UNA_LINEA\n ");
		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){
			switch(paquete->header.type){
			case CPU_SEGUI_EJECUTANDO:
				seguir_ejecucion=1;
				break;
			case KERNEL_FIN_TCB_QUANTUM:
				cambioContexto(self->tcb);
				seguir_ejecucion=0;
				break;
			case ERROR_POR_DESCONEXION_DE_CONSOLA:
				break;
			default:
				log_info(self->loggerCPU, "Codigo inesperado de KERNEL\n");
				printf("Codigo inesperado de KERNEL\n");
				exit(-1);
			}
		}else{
			log_info(self->loggerCPU, "KERNEL ha cerrado su conexion\n");
			exit(-1);
		}

	}//fin while(1)

}

void cambioContexto(t_TCB_CPU* tcb){
	char *data=malloc(sizeof(t_TCB_CPU)); /*TCB*/
	int stmp_size=0;
	memcpy(data, tcb, stmp_size=(sizeof(t_TCB_CPU)));

	/*antes de hacer el send, deberia actualizarce el STACK?*/
	if (socket_sendPaquete(self->socketPlanificador->socket, KERNEL_FIN_TCB_QUANTUM,stmp_size, data)<=0){
		log_info(self->loggerCPU, "falló cambio de conexto\n %d", tcb->pid);

	}else{
		log_info(self->loggerCPU, "falló cambio de conexto\n %d", tcb->pid);
	}
}

int determinar_registro(char registro){

	switch(registro){
	case 'A': return 0;
	case 'B': return 1;
	case 'C': return 2;
	case 'D': return 3;
	case 'E': return 4;
	case 'F': return 5;
	default: return -1;
	}

}


void ejecutar_instruccion(int linea, t_TCB_CPU* tcb_actual){

	char *data=malloc(sizeof(int)+sizeof(uint32_t)+sizeof(char)); //pid+puntero_instruccion
	t_paquete_MSP *pedir_instruccion = malloc(sizeof(t_paquete_MSP));
	int soffset=0, stmp_size=0;
	char *tamanio_leer;
	t_socket_paquete *paquete_MSP = malloc(sizeof(t_socket_paquete));


	switch(linea){
	case LOAD:
		*tamanio_leer='5';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}

		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion LOAD\n" );
				char registro;
				int32_t numero;
				memcpy(&(registro), paquete_MSP->data, sizeof(char));
				memcpy(&(numero), (paquete_MSP->data) + sizeof(char),sizeof(int32_t));
				int reg=determinar_registro(registro);
				LOAD_ESO(reg, numero, tcb_actual);
			}
		}
		break;
	case GETM:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion GETM\n" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				GETM_ESO(regA,regB, tcb_actual);

			}
		}
		break;
	case SETM:
		*tamanio_leer='6';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion SETM\n" );
				char registroA, registroB;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int));
				memcpy(&(registroA), (paquete_MSP->data)+ sizeof(int), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(int)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				SETM_ESO(numero,regA,regB, tcb_actual);
			}
		}
		break;
	case MOVR:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion MOVR\n" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				MOVR_ESO(regA, regB, tcb_actual);
			}
		}
		break;
	case ADDR:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion ADDR\n" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				ADDR_ESO(regA, regB, tcb_actual);
			}
		}
		break;
	case SUBR:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion SUBR\n" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				SUBR_ESO(regA, regB, tcb_actual);
			}
		}


		break;
	case MULR:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion MULR\n" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				MULR_ESO(regA, regB, tcb_actual);
			}
		}

		break;
	case MODR:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion MODR\n" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				MODR_ESO(regA, regB, tcb_actual);
			}
		}

		break;
	case DIVR:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DIVR\n" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				DIVR_ESO(regA, regB, tcb_actual);
			}
		}

		break;
	case INCR:
		*tamanio_leer='1';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion INCR\n" );
				char registro;
				memcpy(&(registro), (paquete_MSP->data), sizeof(char));
				int reg=determinar_registro(registro);
				INCR_ESO(reg,tcb_actual);
			}
		}

		break;
	case DECR:
		*tamanio_leer='1';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DECR\n" );
				char registro;
				memcpy(&(registro), (paquete_MSP->data), sizeof(char));
				int reg=determinar_registro(registro);
				DECR_ESO(reg,tcb_actual);
			}
		}

		break;
	case COMP:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion COMP\n" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				COMP_ESO(regA,regB, tcb_actual);
			}
		}

		break;
	case CGEQ:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion CGEQ\n" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				CGEQ_ESO(regA,regB, tcb_actual);
			}
		}

		break;
	case CLEQ:
		*tamanio_leer='2';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion CLEQ\n" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA=determinar_registro(registroA);
				int regB=determinar_registro(registroB);
				CLEQ_ESO(regA,regB, tcb_actual);
			}
		}

		break;
	case GOTO:
		*tamanio_leer='1';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion GOTO\n" );
				char registro;
				memcpy(&(registro), paquete_MSP->data, sizeof(char));
				int reg=determinar_registro(registro);
				GOTO_ESO(reg,tcb_actual);
			}
		}

		break;
	case JMPZ:
		*tamanio_leer='4';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion JMPZ\n" );
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				//GETM_ESO(numero, tcb_actual);
			}
		}

		break;
	case JPNZ:
		*tamanio_leer='4';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion JPNZ\n" );
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				//GETM_ESO(numero, tcb_actual);
			}
		}

		break;
	case INTE:
		*tamanio_leer='4';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion INTE\n" );
				uint32_t direccion;
				memcpy(&(direccion), paquete_MSP->data, sizeof(uint32_t));
				INTE_ESO(direccion, tcb_actual);
			}
		}

		break;
	case SHIF:
		*tamanio_leer='5';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion LOAD\n" );

			}
		}

		break;
	case NOPP: break;
	case PUSH:
		*tamanio_leer='5';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion PUSH\n" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg=determinar_registro(registro);
				PUSH_ESO(numero, reg, tcb_actual);
			}
		}

		break;
	case TAKE:
		*tamanio_leer='5';
		memcpy(data, &(tcb_actual->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb_actual->puntero_instruccion)+4, stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data, tamanio_leer , stmp_size=sizeof(char));
		soffset+=stmp_size;

		pedir_instruccion->tamanio = soffset;
		pedir_instruccion->data = data;

		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA, pedir_instruccion->tamanio, pedir_instruccion->data)<=0){
			log_info(self->loggerCPU, "Error en envio de direccion a la MSP %d", tcb->pid);
		}


		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion TAKE\n" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg=determinar_registro(registro);
				TAKE_ESO(numero, reg, tcb_actual);
			}
		}

		break;
	case XXXX:
		XXXX_ESO(tcb_actual);
		break;
	case MALC: MALC_ESO(tcb_actual); break;
	case FREE: FREE_ESO(tcb_actual); break;
	case INNN: INNN_ESO(tcb_actual); break;
	case INNC: INNC_ESO(tcb_actual); break;
	case OUTN: OUTN_ESO(tcb_actual); break;
	case OUTC: OUTC_ESO(tcb_actual); break;
	case CREA: CREA_ESO(tcb_actual); break;
	case JOIN: JOIN_ESO(tcb_actual); break;
	case BLOK: BLOK_ESO(tcb_actual); break;
	default: break;

	}

}







void verificar_argumentosCPU(int argc, char* argv[]){
	if( argc < 2 ){
		printf("Modo de empleo: ./CPU cpu.cfg\n");
		perror("CPU no recibio las configuraciones");
		exit (EXIT_FAILURE);
	}
}

t_CPU* cpu_cargar_configuracion(char* config_file){

	t_CPU* self = malloc(sizeof(t_CPU));
	t_config* config = config_create(config_file);

	//se obtiene los datos del archivo

	self->puertoPlanificador = config_get_int_value(config, "PUERTO_KERNEL");
	self->puertoMSP = config_get_int_value(config, "PUERTO_MSP");
	self->ipPlanificador= string_duplicate(config_get_string_value(config, "IP_KERNEL"));
	self->ipMsp = string_duplicate(config_get_string_value(config, "IP_MSP"));
	self->retardo = config_get_int_value(config, "RETARDO");

	config_destroy(config);
	return self;
}








