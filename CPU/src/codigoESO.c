#include "codigoESO.h"
#include "CPU.h"



void LOAD_ESO (int registro, int32_t numero, t_TCB_CPU* tcb){
		if(registro!=-1){
			tcb->registro_de_programacion[registro]=numero;
			break;
		}
		log_error(logger, "Error: registro de programacion no encontrado %d", tcb->pid);
}

void GETM_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){

	if((primer_registro!=-1)&&(segundo_registro!=-1)){

	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+direccion_logica*/
	t_paquete_MSP *leer_bytes = malloc(sizeof(t_paquete_MSP));
	int soffset=0, stmp_size=0;
    memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	soffset=stmp_size;
	memcpy(data + soffset, &(tcb->registro_de_programacion[segundo_registro]), stmp_size=(sizeof(uint32_t)));
	soffset+=stmp_size;

	leer_bytes->tamanio = soffset;
	leer_bytes->data = data;


	if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA,leer_bytes->tamanio, leer_bytes->data)<=0){
		log_info(logger, "Error en envio de direccion a la MSP %d", tcb->pid);
		//cpuCambioDeContextoError();
	}

	free(data);
	free(leer_bytes);

	/*hago el recv*/
	t_socket_paquete *paquete_MSP = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
		if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
					if(paquete_MSP->header.type == LEER_MEMORIA){
						log_info(logger, "recibiendo contenido de direccion %d", tcb->registro_de_programacion[segundo_registro]);
						char *contenido = malloc(sizeof(char)*4);
						memcpy(contenido, paquete_MSP->data, sizeof(char)*4);
						/*resguardo el contenido en primer registro*/
					 	tcb->registro_de_programacion[primer_registro]=*contenido;
					 	break;
					} else {
						log_error(logger, "Se recibio un codigo inesperado de MSP: %d", paquete_MSP->header.type);
						//cpuCambioDeContextoError();
					}
	}else{
		log_info(logger, "MSP ha cerrado su conexion");
		//cpuCambioDeContextoError();
		printf("MSP ha cerrado su conexion\n");
		exit(-1);
		}

		free(paquete_MSP);
		//Sleep para que no se tilde
		usleep(100);
		break;
		}

	log_error(logger, "Error: registro de programacion no encontrado %d", tcb->pid);
}



void SETM_ESO (int numero, int primer_registro, int segundo_registro, t_TCB* tcb){

	if(numero<=sizeof(uint32_t)){

			char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+direccion_logica*/
			t_paquete_MSP *grabar_byte = malloc(sizeof(t_paquete_MSP));
			int soffset=0, stmp_size=0;
			memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
			soffset=stmp_size;
			memcpy(data + soffset, &(tcb->registro_de_programacion[segundo_registro]), stmp_size=sizeof(uint32_t));
			soffset+=stmp_size;
			memcpy(data + soffset, &(tcb->registro_de_programacion[primer_registro]), stmp_size=numero);
			soffset+=stmp_size;

			grabar_byte->tamanio = soffset;
			grabar_byte->data = data;

			if (socket_sendPaquete((t_socket*)socketDelMSP, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
						log_info(logger, "Error de escritura en MSP %d", tcb->pid);
						//cpuCambioDeContextoError();
			}
	}

}


void MOVR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	tcb->registro_de_programacion[primer_registro]=tcb->registro_de_programacion[segundo_registro];
}


void ADDR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]+auxiliar;

}

void SUBR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]-auxiliar;

}

void MULR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]*auxiliar;
	}



void MODR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]%auxiliar;
}

void DIVR_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	if (auxiliar==0){
			log_info(logger, "fallo de division por cero %d", tcb->pid);
			//cpuCambioDeContextoError();
			/*salida estandar informando a la consola que hubo un error de division por cero*/
		}else{
			tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]/auxiliar;
	}
}

void INCR_ESO (int registro, t_TCB* tcb){

	tcb->registro_de_programacion[registro]++;

}

void DECR_ESO (int registro, t_TCB* tcb){

	tcb->registro_de_programacion[registro]--;
}

void COMP_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	if (tcb->registro_de_programacion[primer_registro]==tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}

void CGEQ_ESO (int primer_registro, int segundo_registro, t_TCB* tcb){
	if (tcb->registro_de_programacion[primer_registro]>=tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}

void CLEQ_ESO(int primer_registro, int segundo_registro, t_TCB* tcb){
	if (tcb->registro_de_programacion[primer_registro]<=tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}
void GOTO_ESO (int registro, t_TCB* tcb){
	uint32_t auxiliar=tcb->base_segmento_codigo;
	auxiliar+=(uint32_t)tcb->registro_de_programacion[registro];
	tcb->puntero_instruccion=auxiliar;
}

void JMPZ_ESO(int numero, t_TCB* tcb){
   if(tcb->registro_de_programacion[0]==0){
	   uint32_t auxiliar=(uint32_t)numero;
	   auxiliar+=tcb->base_segmento_codigo;
	   tcb->puntero_instruccion=auxiliar;
   }
}

void JPNZ_ESO(int numero, t_TCB* tcb){
	   if(tcb->registro_de_programacion[0]!=0){
		   uint32_t auxiliar=(uint32_t)numero;
		   auxiliar+=tcb->base_segmento_codigo;
		   tcb->puntero_instruccion=auxiliar;
	   }
}

void INTE_ESO(uint32_t direccion, t_TCB* tcb){
	//tengo que tomar esa direccion y pasarcela al Kernel con el TCB

		tcb->puntero_instruccion+=1; //incremento el puntero de instruccion, porque hago cambio de conexto
		char *direccion_send=malloc(sizeof(char));
		*direccion_send=direccion;
		char *data=malloc(sizeof(t_TCB)+sizeof(uint32_t)); /*TCB+direccion_SysCall*/
		int soffset=0, stmp_size=0;
		memcpy(data, tcb, stmp_size=(sizeof(t_TCB)));
		soffset=stmp_size;
		memcpy(data + soffset, direccion_send, stmp_size=sizeof(uint32_t));
		soffset+=stmp_size;


		if (socket_sendPaquete((t_socket*)socketDelKernel, 84,stmp_size, data)<=0){  //84 corresponde a interrupcion
					log_info(logger, "Error de Interrupcion %d", tcb->pid);
					//cpuCambioDeContextoError();

		}
		free(direccion_send);
		free(data);

}
/*
void FLCL(){

}*/

void SHIF_ESO (int numero, int registro, t_TCB* tcb){
	if(numero>0){
		int32_t auxiliar=tcb->registro_de_programacion[registro];
		int32_t resultado;
		resultado=auxiliar>>numero;
		tcb->registro_de_programacion[registro]=resultado;
	}else{
		int32_t auxiliar=tcb->registro_de_programacion[registro];
		int32_t resultado;
		resultado=auxiliar<<numero;
		tcb->registro_de_programacion[registro]=resultado;
	}

}


void NOPP_ESO (){
	/*no hace nada*/
}


void PUSH_ESO (int numero, int registro, t_TCB* tcb){

	if(numero<=sizeof(uint32_t)){
			char *datos_a_grabar=malloc(sizeof(uint32_t));
			memcpy(datos_a_grabar, &(tcb->registro_de_programacion[registro]), numero);

			char *grabar_byte=malloc(sizeof(int)+sizeof(uint32_t)*2); /*pid+direccion_logica+datos_a_grabar*/
			t_paquete_MSP *paquete_send = malloc(sizeof(t_paquete_MSP));
			int soffset=0, stmp_size=0;
			memcpy(grabar_byte, &(tcb->pid), stmp_size=(sizeof(int)));
			soffset=stmp_size;
			memcpy(grabar_byte + soffset, &(tcb->cursor_stack), stmp_size=sizeof(uint32_t));
			soffset+=stmp_size;
			memcpy(grabar_byte + soffset, datos_a_grabar, stmp_size=sizeof(uint32_t));

			paquete_send->tamanio = soffset;
			paquete_send->data = grabar_byte;

			if (socket_sendPaquete((t_socket*)socketDelMSP, ESCRIBIR_MEMORIA,paquete_send->tamanio, paquete_send->data)<=0){
						log_info(logger, "Error de escritura en MSP %d", tcb->pid);
						//cpuCambioDeContextoError();

			}
				free(grabar_byte);
				free(paquete_send);
				tcb->cursor_stack+=numero; //actualizo el cursor de stack

	}
}

void TAKE_ESO (int numero, int registro, t_TCB* tcb){

	if(numero<=sizeof(uint32_t)){

		char *data = malloc(sizeof(int)+sizeof(uint32_t)+sizeof(int)); /*pid+direccion_logica*/
		t_paquete_MSP *leer_bytes = malloc(sizeof(t_paquete_MSP));
		int soffset=0, stmp_size=0;
		memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
		soffset=stmp_size;
		memcpy(data + soffset, &(tcb->cursor_stack), stmp_size=(sizeof(uint32_t)));
		soffset+=stmp_size;
		memcpy(data + soffset, &(numero), stmp_size=sizeof(int));
		soffset+=stmp_size;

		leer_bytes->tamanio = soffset;
		leer_bytes->data = data;


		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA,leer_bytes->tamanio, leer_bytes->data)<=0){
			log_info(logger, "Error en envio de direccion a la MSP %d", tcb->pid);
			//cpuCambioDeContextoError();
		}

		free(data);
		free(leer_bytes);
		tcb->cursor_stack-=numero; //actualizo el cursor de stack

	}
}

void XXXX_ESO (t_TCB* tcb){


	char *data=malloc(sizeof(t_TCB)); /*TCB*/
	int stmp_size=0;
	memcpy(data, tcb, stmp_size=(sizeof(t_TCB)));


	if (socket_sendPaquete((t_socket*)socketDelKernel, 29 ,stmp_size, data)<=0){
				log_info(logger, "Error de finalizacion de proceso %d", tcb->pid);
				//cpuCambioDeContextoError();
	}
	free(data);

}



//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

void MALC_ESO (){

}
void FREE_ESO(){

}
void INNC_ESO(){

}
void OUTN_ESO(){

}
void OUTC_ESO(){

}
void CREA_ESO(){

}
void JOIN_ESO(){

}

void BLOK_ESO(){

}

void WAKE_ESO(){

}


//funciones de la system calls
/*void systemCallsMALLOC(){

}
void systemCallsFREE(){

}
void systemCallsINN(){

}
void systemCallsINC(){

}
void systemCallsOUTN(){

}
void systemCallsOUTC(){

}
void systemCallsCREATE_THREAD(){

}
void systemCallsJOIN_THREAD(){

}
void systemCallsWAIT(){

}

void systemCallsSIGNAL(){

}

void systemCallsSETSEM(){}
*/
