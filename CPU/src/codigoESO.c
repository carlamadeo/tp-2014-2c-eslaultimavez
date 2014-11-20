#include "codigoESO.h"
#include "CPU.h"



void LOAD_ESO (int registro, int32_t numero, t_TCB_CPU* tcb){
	if(registro!=-1){
		tcb->registro_de_programacion[registro]=numero;

	}
	log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado\n %d", tcb->pid);
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


		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA,leer_bytes->tamanio, leer_bytes->data)<=0){
			log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP\n %d", tcb->pid);

		}

		free(data);
		free(leer_bytes);

		t_socket_paquete *paquete_MSP = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
		if(socket_recvPaquete(self->socketMSP->socket, paquete_MSP) > 0){
			if(paquete_MSP->header.type == LEER_MEMORIA){
				log_info(self->loggerCPU, "CPU: recibiendo contenido de direccion:\n %d", tcb->registro_de_programacion[segundo_registro]);
				char *contenido = malloc(sizeof(char)*4);
				memcpy(contenido, paquete_MSP->data, sizeof(char)*4);
				/*resguardo el contenido en primer registro*/
				tcb->registro_de_programacion[primer_registro]=*contenido;
			} else {
				log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP:\n %d", paquete_MSP->header.type);

			}
		}else{
			log_info(self->loggerCPU, "CPU: MSP ha cerrado su conexion\n");
			printf("CPU: MSP ha cerrado su conexion\n");
			exit(-1);
		}

		free(paquete_MSP);
		usleep(100);

	}

	log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", tcb->pid);
}



void SETM_ESO (int numero, int primer_registro, int segundo_registro, t_TCB_CPU* tcb){

	if(numero<=sizeof(uint32_t)){

		char *data=malloc(sizeof(int)+sizeof(uint32_t)+sizeof(int)); /*pid+direccion_logica*/
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

		if (socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
			log_info(self->loggerCPU, "CPU: fallo: ESCRIBIR_MEMORIA\n %d", tcb->pid);

		}
	}

}


void MOVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	tcb->registro_de_programacion[primer_registro]=tcb->registro_de_programacion[segundo_registro];
}


void ADDR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]+auxiliar;

}

void SUBR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]-auxiliar;

}

void MULR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]*auxiliar;
}



void MODR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]%auxiliar;
}

void DIVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	if (auxiliar==0){
		log_info(logger, "fallo de division por cero %d", tcb->pid);
		if (socket_sendPaquete(self->socketPlanificador->socket,MENSAJE_DE_ERROR,sizeof(int),&(self->tcb->pid))<=0){
				log_error(self->loggerCPU, "CPU: fallo: MENSAJE_DE_ERROR\n");
		}
	}else{
		tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]/auxiliar;
	}
}

void INCR_ESO (int registro, t_TCB_CPU* tcb){

	tcb->registro_de_programacion[registro]++;

}

void DECR_ESO (int registro, t_TCB_CPU* tcb){

	tcb->registro_de_programacion[registro]--;
}

void COMP_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	if (tcb->registro_de_programacion[primer_registro]==tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}

void CGEQ_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	if (tcb->registro_de_programacion[primer_registro]>=tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}

void CLEQ_ESO(int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	if (tcb->registro_de_programacion[primer_registro]<=tcb->registro_de_programacion[segundo_registro]){
		tcb->registro_de_programacion[0]=1;
	}else{
		tcb->registro_de_programacion[0]=0;
	}

}
void GOTO_ESO (int registro, t_TCB_CPU* tcb){
	uint32_t auxiliar=tcb->base_segmento_codigo;
	auxiliar+=(uint32_t)tcb->registro_de_programacion[registro];
	tcb->puntero_instruccion=auxiliar;
}

void JMPZ_ESO(int numero, t_TCB_CPU* tcb){
	if(tcb->registro_de_programacion[0]==0){
		uint32_t auxiliar=(uint32_t)numero;
		auxiliar+=tcb->base_segmento_codigo;
		tcb->puntero_instruccion=auxiliar;
	}
}

void JPNZ_ESO(int numero, t_TCB_CPU* tcb){
	if(tcb->registro_de_programacion[0]!=0){
		uint32_t auxiliar=(uint32_t)numero;
		auxiliar+=tcb->base_segmento_codigo;
		tcb->puntero_instruccion=auxiliar;
	}
}

void INTE_ESO(uint32_t direccion, t_TCB_CPU* tcb){
	//tengo que tomar esa direccion y pasarcela al Kernel con el TCB

	tcb->puntero_instruccion+=1; //incremento el puntero de instruccion, porque hago cambio de conexto
	char *direccion_send=malloc(sizeof(char));
	*direccion_send=direccion;
	char *data=malloc(sizeof(t_TCB_CPU)+sizeof(uint32_t)); /*TCB+direccion_SysCall*/
	int soffset=0, stmp_size=0;
	memcpy(data, tcb, stmp_size=(sizeof(t_TCB_CPU)));
	soffset=stmp_size;
	memcpy(data + soffset, direccion_send, stmp_size=sizeof(uint32_t));
	soffset+=stmp_size;


	if (socket_sendPaquete(self->socketPlanificador->socket, INTERRUPCION,stmp_size, data)<=0){  //22 corresponde a interrupcion
		log_info(logger, "Error de Interrupcion %d", tcb->pid);


	}
	free(direccion_send);
	free(data);

}
/*
void FLCL(){

}*/

void SHIF_ESO (int numero, int registro, t_TCB_CPU* tcb){
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


void PUSH_ESO (int numero, int registro, t_TCB_CPU* tcb){

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

		if (socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA,paquete_send->tamanio, paquete_send->data)<=0){
			log_info(self->loggerCPU, "CPU: Error de escritura en MSP\n %d", tcb->pid);

		}
		free(grabar_byte);
		free(paquete_send);
		tcb->cursor_stack+=numero; //actualizo el cursor de stack

	}
}

void TAKE_ESO (int numero, int registro, t_TCB_CPU* tcb){

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


		if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA,leer_bytes->tamanio, leer_bytes->data)<=0){
			log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);

		}

		free(data);
		free(leer_bytes);
		tcb->cursor_stack-=numero; //actualizo el cursor de stack

	}
}

void XXXX_ESO (t_TCB_CPU* tcb){


	char *data=malloc(sizeof(t_TCB_CPU)); /*TCB*/
	int stmp_size=0;
	memcpy(data, tcb, stmp_size=(sizeof(t_TCB_CPU)));


	if (socket_sendPaquete(self->socketPlanificador->socket, FINALIZAR_PROGRAMA_EXITO ,stmp_size, data)<=0){
		log_info(self->loggerCPU, "CPU: Error de finalizacion de proceso %d", tcb->pid);
	}
	free(data);

}



//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

void MALC_ESO (t_TCB_CPU* tcb){
	//CREAR_SEGMENTO
	char *data=malloc(sizeof(int)+sizeof(int32_t)); /*pid+(tamanio)registro_de_programacion['A']*/
	t_paquete_MSP *alocar_bytes = malloc(sizeof(t_paquete_MSP));

	int soffset=0, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	soffset=stmp_size;
	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(int32_t));
	soffset+=stmp_size;

	alocar_bytes->tamanio = soffset;
	alocar_bytes->data = data;

	if (socket_sendPaquete(self->socketPlanificador->socket, CREAR_SEGMENTO, alocar_bytes->tamanio, alocar_bytes->data)<=0){
		log_info(self->socketMSP->socket, "CPU: Error de memoria llena en MSP\n %d", tcb->pid);
	}
	t_socket_paquete *paquete_MSP = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_MSP) > 0){
		if(paquete_MSP->header.type == CREAR_SEGMENTO){
			log_info(self->loggerCPU, "CPU: recibiendo direccion virtual...\n %d ", tcb->pid);
			char *contenido = malloc(sizeof(uint32_t));
			memcpy(contenido, paquete_MSP->data, sizeof(char)*4);
			/*resguardo la direccion virtual en el registro A*/
			tcb->registro_de_programacion[0]=*contenido;
			free(paquete_MSP);
			free(contenido);
			} else {
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP:\n %d", paquete_MSP->header.type);
					}
	}else{
		log_info(self->loggerCPU, "CPU: MSP ha cerrado su conexion");
		printf("MSP ha cerrado su conexion\n");
		exit(-1);
	}
	free(alocar_bytes);
	free(data);
}


void FREE_ESO(t_TCB_CPU* tcb){

	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+tamaño segun registro*/
	t_paquete_MSP *leer_byte = malloc(sizeof(t_paquete_MSP));

	int soffset=0, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	soffset=stmp_size;
	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(uint32_t));
	soffset+=stmp_size;

	leer_byte->tamanio = soffset;
	leer_byte->data = data;

	if (socket_sendPaquete(self->socketPlanificador->socket, DESTRUIR_SEGMENTO, leer_byte->tamanio, leer_byte->data)<=0){
		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);

	}
	free(data);
	free(leer_byte);

}

void INNN_ESO(t_TCB_CPU* tcb){


	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR,sizeof(int), &(tcb->pid))<=0){
		log_info(self->loggerCPU, "CPU: Error de ENTRADA_ESTANDAR\n %d", tcb->pid);
	}

	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR){
			log_info(self->loggerCPU, "CPU: recibiendo numero ingresado por consola\n %d", tcb->pid);
			tcb->registro_de_programacion[0]=(int)paquete_KERNEL;
		} else {
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP:\n %d", paquete_KERNEL->header.type);

		}
	}else{
		log_info(self->loggerCPU, "CPU: MSP ha cerrado su conexion\n");
		printf("CPU: MSP ha cerrado su conexion\n");
		exit(-1);
	}

	free(paquete_KERNEL);
	usleep(100);

}
void INNC_ESO(t_TCB_CPU* tcb){

	t_paquete_MSP *pedir_cadena = malloc(sizeof(t_paquete_MSP));

	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+(tamanio)registro_de_programacion['B']*/
	int soffset=0, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	memcpy(data + soffset, &(tcb->registro_de_programacion[1]), stmp_size=sizeof(int32_t));
	soffset+=stmp_size;
	pedir_cadena->tamanio=soffset;
	pedir_cadena->data=data;

	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR_CHAR,pedir_cadena->tamanio, pedir_cadena->data)<=0){  //22 corresponde a interrupcion
		log_info(self->loggerCPU, "CPU: Error de ENTRADA_ESTANDAR_CHAR\n %d", tcb->pid);


	}
	free(data);
	free(pedir_cadena);

	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
	if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR_CHAR){
			log_info(self->loggerCPU, "CPU: recibiendo CADENA ingresado por consola ", tcb->pid);
			char *cadena = malloc(sizeof(uint32_t));
			memcpy(cadena, paquete_KERNEL->data, sizeof(char));

			char *data=malloc(sizeof(int)+sizeof(uint32_t)+ tcb->registro_de_programacion[1]);  /*pid+direccion_logica+datos_a_grabar*/
			t_paquete_MSP *grabar_byte = malloc(sizeof(t_paquete_MSP));
			int soffset=0, stmp_size=0;
			memcpy(grabar_byte, &(tcb->pid), stmp_size=(sizeof(int)));
			soffset=stmp_size;
			memcpy(grabar_byte + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(uint32_t));
			soffset+=stmp_size;
			memcpy(grabar_byte + soffset, cadena, stmp_size=tcb->registro_de_programacion[1]);
			soffset+=stmp_size;

			grabar_byte->tamanio = soffset;
			grabar_byte->data = data;

			if (socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
				log_info(self->loggerCPU, "CPU: Error de escritura en MSP\n %d", tcb->pid);
			}
		} else {
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP:\n %d", paquete_KERNEL->header.type);
				}
	}else{
		log_info(self->loggerCPU, "MSP ha cerrado su conexion\n");
		printf("MSP ha cerrado su conexion\n");
		exit(-1);
	}

	free(paquete_KERNEL);

	usleep(100);

}
void OUTN_ESO(t_TCB_CPU* tcb){

	t_paquete_MSP *mostrar_numero = malloc(sizeof(t_paquete_MSP));

	char *data=malloc(sizeof(int)+sizeof(int32_t)); /*pid+(tamanio)registro_de_programacion['B']*/
	int soffset, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(int32_t));
	soffset+=stmp_size;
	mostrar_numero->tamanio=soffset;
	mostrar_numero->data=data;

	if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR,mostrar_numero->tamanio, mostrar_numero->data)<=0){
		log_info(self->loggerCPU, "CPU: Error de SALIDA_ESTANDAR\n %d", tcb->pid);


	}
	free(data);
	free(mostrar_numero);

}
void OUTC_ESO(t_TCB_CPU* tcb){

	t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
	unCPU_LEER_MEMORIA->pid = self->tcb->pid;
	unCPU_LEER_MEMORIA->tamanio = tcb->registro_de_programacion[1];
	unCPU_LEER_MEMORIA->direccionVirtual = tcb->registro_de_programacion[0];
	if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_CPU_LEER_MEMORIA), unCPU_LEER_MEMORIA)<=0){
		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
	}

	t_socket_paquete *cadena = malloc(sizeof(t_socket_paquete));

	if(socket_recvPaquete(self->socketMSP->socket, cadena) > 0){
		if(cadena->header.type == LEER_MEMORIA){
			char *mostrar_cadena=malloc(tcb->registro_de_programacion[1]);
			memcpy(mostrar_cadena, cadena->data, tcb->registro_de_programacion[1]);

			char *data=malloc(sizeof(int)+tcb->registro_de_programacion[1]); /*pid+(tamanio)registro_de_programacion['B']*/
				int soffset=0, stmp_size=0;
				memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
				memcpy(data + soffset, mostrar_cadena, stmp_size=tcb->registro_de_programacion[1]);
				soffset+=stmp_size;

				if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR_CHAR ,soffset, data)<=0){
					log_info(self->loggerCPU, "CPU: Error de SALIDA_ESTANDAR_CHAR\n %d", tcb->pid);

				}
				free(data);
				free(mostrar_cadena);
		}
	}


}
void CREA_ESO(t_TCB_CPU* tcb){

}
void JOIN_ESO(t_TCB_CPU* tcb){

}

void BLOK_ESO(t_TCB_CPU* tcb){

	t_paquete_MSP *envio_bytes = malloc(sizeof(t_paquete_MSP));

	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+(tamanio)registro_de_programacion['B']*/
	int soffset=0, stmp_size=0;
	memcpy(data, tcb, stmp_size=(sizeof(t_TCB_CPU)));
	memcpy(data + soffset, &(tcb->registro_de_programacion[1]), stmp_size=sizeof(int32_t));
	soffset+=stmp_size;
	envio_bytes->tamanio=soffset;
	envio_bytes->data=data;

	if (socket_sendPaquete(self->socketPlanificador->socket, /*BLOK_HILO*/ 32,envio_bytes->tamanio, envio_bytes->data)<=0){
		log_info(self->loggerCPU, "CPU: Error de BLOQUEO\n %d", tcb->pid);
	}
	free(data);
	free(envio_bytes);


}

void WAKE_ESO(t_TCB_CPU* tcb){

	if (socket_sendPaquete(self->socketPlanificador->socket, /*WAKE_HILO*/33,sizeof(int32_t), &(tcb->registro_de_programacion[1]))<=0){
		log_info(self->loggerCPU, "CPU: Error de DESPERTAR\n %d", tcb->pid);


	}
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
