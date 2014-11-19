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
#include "cpuMSP.h"
#include "cpuKernel.h"
#include "codigoESO.h"


int main(int argc, char** argv) {

	verificar_argumentosCPU(argc, argv);
	char* config_file = argv[1];
	t_CPU* self = cpu_cargar_configuracion(config_file);

	self->loggerCPU = log_create("logCPU.log", "CPU", 1, LOG_LEVEL_DEBUG);
	self->socketMSP = cpuConectarConMPS(self);
	self->socketPlanificador = conectarCPUConKernel(self);

	while(1){
		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){

			if(paquete->header.type == TCB_NUEVO){
				self->tcb = (t_TCB_CPU*)paquete->data;
				cpuProcesar_tcb(self);
			}
			else
				log_error(self->loggerCPU, "Se recibio un codigo inesperado de KERNEL en main de cpu: %d", paquete->header.type);
		}

		else {
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

	log_info(self->loggerCPU, "CPU: Comienzo a procesar el TCB de pid: %d", self->tcb->pid);

	/*pregunto por KM del tcb a ejecutar*/
	int seguir_ejecucion = 1;

	while(seguir_ejecucion){
		//pido los primeros 4 bytes especificados por PID+Puntero_Instruccion

		t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
		unCPU_LEER_MEMORIA->pid = self->tcb->pid;
		unCPU_LEER_MEMORIA->tamanio = sizeof(char)*4;

		//unCPU_LEER_MEMORIA->tamanio = self->tcb->tamanio_segmento_codigo; // esto esta mal, vos tenes que pedirle 4 byte a la MSP no el .bc completo!
		unCPU_LEER_MEMORIA->direccionVirtual = self->tcb->puntero_instruccion;
		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_CPU_LEER_MEMORIA), unCPU_LEER_MEMORIA)<=0){
			log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
			//cpuCambioDeConextoError();
		}

		log_info(self->loggerCPU, "CPU: Envia a MSP LEER_MEMORIA");

		t_socket_paquete *paquete_MSP = malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibe un LEER_MEMORIA: %d ", paquete_MSP->header.type);
				/*reservo memoria en una variable linea para guardar los 4 caracteres de ESO*/
				char *nombre_instruccion = malloc(sizeof(char)*4);
				/*guardo en la variable nombre_instruccion los 4 caracteres que forman el nombre de la instruccion ESO*/
				int doffset = 0, dtemp_size = 0;
				memcpy(nombre_instruccion, paquete_MSP->data, dtemp_size=sizeof(char));
				doffset = dtemp_size;
				memcpy(nombre_instruccion + doffset, paquete_MSP->data + doffset, dtemp_size=sizeof(char));
				doffset += dtemp_size;
				memcpy(nombre_instruccion + doffset, paquete_MSP->data + doffset, dtemp_size=sizeof(char));
				doffset += dtemp_size;
				memcpy(nombre_instruccion + doffset, paquete_MSP->data + doffset, dtemp_size=sizeof(char));

				char instrucciones_eso[] = {'L','O','A','D','G','E','T','M','S','E','T','M','M','O','V','R','A','D','D','R','S','U','B','R','M','U','L','R','M','O','D','R','D','I','V','R','I','N','C','R','D','E','C','R','C','O','M','P','C','G','E','Q','C','L','E','Q','G','O','T','O','J','M','P','Z','J','N','P','Z','I','N','T','E','S','H','I','F','N','O','P','P','P','U','S','H','T','A','K','E','X','X','X','X','M','A','L','C','F','R','E','E','I','N','N','N','I','N','N','C','O','U','T','N','O','U','T','C','C','R','E','A','J','O','I','N','B','L','O','K','W','A','K','E','\0'};

				/*
				 * NO ACCEDE A LA FUNCION EJECUTAR_INSTRUCCION PORQUE ALTERASTE PRIMERAMENTE LOS CODIGOS ESO EN EL CPU.H
				 * ESTE ALGORITMO SE BASABA EN DICHOS CODIGOS PARA AGREGAR EL PARAMENTRO ADECUADO A LA FUNCION EJECUTAR_INSTRUCCION
				 * POR LO TANTO MAS ABAJO ADECUAMOS EL ALGORITMO A TUS CODIGOS
				 instruccion=1, indice=0;
				//ERROR NO LLEGA A LA FUNCION ejecutar_instruccion
				if(strncmp( &(instrucciones_eso[indice]), nombre_instruccion+2, 1 )==0){
					indice++;
					if(strncmp( &(instrucciones_eso[indice]), nombre_instruccion+3, 1 )==0){
						//usleep(self->retardo); //tiempo que debe esperar la CPU antes de ejecutar una instruccion ESO
						ejecutar_instruccion(instruccion, self);
						free(nombre_instruccion);
						break;
					}else
						indice++;
				}

				 */

				int instruccion=1001, indice=0;
				while (instrucciones_eso[indice]!='\0'){

					if (strncmp(&(instrucciones_eso[indice]), nombre_instruccion, 1) == 0){

						indice++;
						if(strncmp(&(instrucciones_eso[indice]), nombre_instruccion + 1, 1) == 0){

							indice++;
							if(strncmp(&(instrucciones_eso[indice]), nombre_instruccion+2, 1) == 0){
								indice++;

								if(strncmp(&(instrucciones_eso[indice]), nombre_instruccion+3, 1) == 0){
									ejecutar_instruccion(instruccion, self);
									break;
								}
								else
									indice++;
							}
							else
								indice+=2;
						}
						else
							indice+=3;
					}
					else
						indice+=4;

					instruccion++;
				} //fin while de algoritmo detectar instruccion ESO

			}
			else
				log_error(self->loggerCPU, "CPU: Recibio un codigo inesperado de la MSP:\n %d", paquete_MSP->header.type);

		}

		else{
			log_info(self->loggerCPU, "CPU: Ha cerrado su conexion dentro del procesar TCB\n");
			exit(-1);
		}

		free(paquete_MSP);
		//Sleep para que no se tilde
		usleep(100);


		t_CPU_TERMINE_UNA_LINEA* unTerminoLinea = malloc(sizeof(t_CPU_TERMINE_UNA_LINEA));
		unTerminoLinea->pid = self->tcb->pid;
		unTerminoLinea->tid = self->tcb->tid;

		log_info(self->loggerCPU, "CPU: Envia al Planificador CPU_TERMINE_UNA_LINEA");

		if (socket_sendPaquete(self->socketPlanificador->socket, CPU_TERMINE_UNA_LINEA, sizeof(t_CPU_TERMINE_UNA_LINEA), unTerminoLinea) <= 0)
			log_info(self->loggerCPU, "CPU: Fallo pedido de datos de Consola %d", self->tcb->pid);

		else{
			t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

			if(socket_recvPaquete(self->socketPlanificador->socket, paquete) >= 0){

				switch(paquete->header.type){

				case CPU_SEGUI_EJECUTANDO:
					break;
				case KERNEL_FIN_TCB_QUANTUM:
					cambioContexto(self);
					seguir_ejecucion = 0;
					break;
				case ERROR_POR_DESCONEXION_DE_CONSOLA:
					log_info(self->loggerCPU, "CPU: Recibe un ERROR_POR_DESCONEXION_DE_CONSOLA");
					break;
				default:
					log_info(self->loggerCPU, "CPU: Recibe un inesperado al mandar un CPU_TERMINE_UNA_LINEA");
					exit(-1);
				}
			}

			else{
				log_info(self->loggerCPU, "CPU: Error al esperar un paquete");
				exit(-1);
			}
		}

	}//fin while(1)

}

void cambioContexto(t_CPU* self){

	//antes de hacer el send, deberia actualizarce el STACK
	//el stack sufre modificaciones cuando se ejecutan las instrucciones ESO, no se actualiza, lo que se actualiza es el TCB.
	if (socket_sendPaquete(self->socketPlanificador->socket,CAMBIO_DE_CONTEXTO,sizeof(t_TCB_CPU),self->tcb)<=0)
		log_error(self->loggerCPU, "CPU: Falló cambio de conexto");

	else
		log_info(self->loggerCPU, "CPU: Envia al Planificador: CAMBIO_DE_CONTEXTO");

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

} //fin while procesar TCB


void ejecutar_instruccion(int linea, t_CPU* self){

	t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA)); //pid+puntero_instruccion+tamanio_parametros
	t_socket_paquete *paquete_MSP = malloc(sizeof(t_socket_paquete));

	unCPU_LEER_MEMORIA->pid = self->tcb->pid;
	unCPU_LEER_MEMORIA->direccionVirtual = (self->tcb->puntero_instruccion)+4;

	switch(linea){
	case LOAD:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion LOAD" );
				char registro;
				int32_t numero;
				memcpy(&(registro), paquete_MSP->data, sizeof(char));
				memcpy(&(numero), (paquete_MSP->data) + sizeof(char),sizeof(int32_t));
				int reg = determinar_registro(registro);
				LOAD_ESO(reg, numero, self->tcb);
			}
		}

		break;

	case GETM:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GETM");
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				GETM_ESO(regA,regB, self->tcb);

			}
		}

		break;

	case SETM:

		unCPU_LEER_MEMORIA->tamanio = 6;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SETM");
				char registroA, registroB;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int));
				memcpy(&(registroA), (paquete_MSP->data)+ sizeof(int), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(int)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				SETM_ESO(numero,regA,regB, self->tcb);
			}
		}

		break;

	case MOVR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MOVR");
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				MOVR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case ADDR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if(paquete_MSP->header.type == LEER_MEMORIA){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion ADDR");
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				ADDR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case SUBR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA) <= 0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SUBR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				SUBR_ESO(regA, regB, self->tcb);

			}
		}


		break;

	case MULR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){

			if( paquete_MSP->header.type == LEER_MEMORIA ){

				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MULR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				MULR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case MODR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MODR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				MODR_ESO(regA, regB, self->tcb);
			}
		}

		break;
	case DIVR:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DIVR" );
				char registroA, registroB;
				memcpy(&(registroA), (paquete_MSP->data), sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data)+ sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				DIVR_ESO(regA, regB, self->tcb);
			}
		}

		break;

	case INCR:

		unCPU_LEER_MEMORIA->tamanio = 1;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion INCR" );
				char registro;
				memcpy(&(registro), (paquete_MSP->data), sizeof(char));
				int reg = determinar_registro(registro);
				INCR_ESO(reg,self->tcb);
			}
		}

		break;

	case DECR:

		unCPU_LEER_MEMORIA->tamanio = 1;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "recibiendo parametros de instruccion DECR" );
				char registro;
				memcpy(&(registro), (paquete_MSP->data), sizeof(char));
				int reg = determinar_registro(registro);
				DECR_ESO(reg,self->tcb);
			}
		}

		break;

	case COMP:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion COMP" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				COMP_ESO(regA,regB, self->tcb);
			}
		}

		break;

	case CGEQ:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CGEQ" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				CGEQ_ESO(regA,regB, self->tcb);
			}
		}

		break;

	case CLEQ:

		unCPU_LEER_MEMORIA->tamanio = 2;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CLEQ" );
				char registroA, registroB;
				memcpy(&(registroA), paquete_MSP->data, sizeof(char));
				memcpy(&(registroB), (paquete_MSP->data) + sizeof(char),sizeof(char));
				int regA = determinar_registro(registroA);
				int regB = determinar_registro(registroB);
				CLEQ_ESO(regA,regB, self->tcb);
			}
		}

		break;
	case GOTO:

		unCPU_LEER_MEMORIA->tamanio = 1;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GOTO" );
				char registro;
				memcpy(&(registro), paquete_MSP->data, sizeof(char));
				int reg = determinar_registro(registro);
				GOTO_ESO(reg,self->tcb);
			}
		}

		break;

	case JMPZ:

		unCPU_LEER_MEMORIA->tamanio = 4;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JMPZ" );
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				//GETM_ESO(numero, self->tcb);
			}
		}

		break;

	case JPNZ:

		unCPU_LEER_MEMORIA->tamanio = 4;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JPNZ" );
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				//GETM_ESO(numero, self->tcb);
			}
		}

		break;

	case INTE:

		unCPU_LEER_MEMORIA->tamanio = 4;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion INTE" );
				uint32_t direccion;
				memcpy(&(direccion), paquete_MSP->data, sizeof(uint32_t));
				INTE_ESO(direccion, self->tcb);
			}
		}

		break;

	case SHIF:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SHIF" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg = determinar_registro(registro);
				SHIF_ESO(numero, reg, self->tcb);
			}
		}

		break;

	case NOPP: break;

	case PUSH:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion PUSH" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg = determinar_registro(registro);
				PUSH_ESO(numero, reg, self->tcb);
			}
		}

		break;

	case TAKE:

		unCPU_LEER_MEMORIA->tamanio = 5;

		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, unCPU_LEER_MEMORIA->tamanio, unCPU_LEER_MEMORIA)<=0){
			log_error(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
			break;
		}

		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if( paquete_MSP->header.type == LEER_MEMORIA ){
				log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion TAKE" );
				char registro;
				int32_t numero;
				memcpy(&(numero), paquete_MSP->data, sizeof(int32_t));
				memcpy(&(registro), (paquete_MSP->data) + sizeof(int32_t),sizeof(char));
				int reg = determinar_registro(registro);
				TAKE_ESO(numero, reg, self->tcb);
			}
		}

		break;
	case XXXX:
		XXXX_ESO(self->tcb);
		break;
	case MALC: MALC_ESO(self->tcb); break;
	case FREE: FREE_ESO(self->tcb); break;
	case INNN: INNN_ESO(self->tcb); break;
	case INNC: INNC_ESO(self->tcb); break;
	case OUTN: OUTN_ESO(self->tcb); break;
	case OUTC: OUTC_ESO(self->tcb); break;
	case CREA: CREA_ESO(self->tcb); break;
	case JOIN: JOIN_ESO(self->tcb); break;
	case BLOK: BLOK_ESO(self->tcb); break;
	default: break;

	}

}


void verificar_argumentosCPU(int argc, char* argv[]){
	if( argc != 2 ){
		printf("Modo de empleo: ./CPU cpuConfig.cfg\n");
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

