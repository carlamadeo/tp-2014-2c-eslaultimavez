#include "codigoESO.h"
#include "CPU.h"



void LOAD_ESO (int registro, int32_t numero, t_TCB_CPU* tcb){
	if(registro!=-1){
		tcb->registro_de_programacion[registro]=numero;
		//break;
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
				//break;
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
		//break;
	}

	log_error(logger, "Error: registro de programacion no encontrado %d", tcb->pid);
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

		if (socket_sendPaquete((t_socket*)socketDelMSP, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
			log_info(logger, "Error de escritura en MSP %d", tcb->pid);
			//cpuCambioDeContextoError();
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
		//cpuCambioDeContextoError();
		/*salida estandar informando a la consola que hubo un error de division por cero*/
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


	if (socket_sendPaquete((t_socket*)socketDelKernel, INTERRUPCION,stmp_size, data)<=0){  //22 corresponde a interrupcion
		log_info(logger, "Error de Interrupcion %d", tcb->pid);
		//cpuCambioDeContextoError();

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

		if (socket_sendPaquete((t_socket*)socketDelMSP, ESCRIBIR_MEMORIA,paquete_send->tamanio, paquete_send->data)<=0){
			log_info(logger, "Error de escritura en MSP %d", tcb->pid);

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


		if (socket_sendPaquete((t_socket*)socketDelMSP, LEER_MEMORIA,leer_bytes->tamanio, leer_bytes->data)<=0){
			log_info(logger, "Error en envio de direccion a la MSP %d", tcb->pid);
			//cpuCambioDeContextoError();
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


//	if (socket_sendPaquete((t_socket*)socketDelKernel, FIN_PROGRAMA ,stmp_size, data)<=0){
//		log_info(logger, "Error de finalizacion de proceso %d", tcb->pid);
//		//cpuCambioDeContextoError();
//	}
	free(data);

}



//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

void MALC_ESO (t_TCB_CPU* tcb){
	//CREAR_SEGMENTO


	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+(tamanio)registro_de_programacion['A']*/
	t_paquete_MSP *grabar_bytes = malloc(sizeof(t_paquete_MSP));

	int soffset=0, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	soffset=stmp_size;
	memcpy(data + soffset, tcb->registro_de_programacion[0], stmp_size=sizeof(int32_t));
	soffset+=stmp_size;

	grabar_bytes->tamanio = soffset;
	grabar_bytes->data = data;

	if (socket_sendPaquete((t_socket*)socketDelMSP, CREAR_SEGMENTO, grabar_bytes->tamanio, grabar_bytes->data)<=0){
		log_info(logger, "Error de memoria llena en MSP %d", tcb->pid);
		//cpuCambioDeContextoError();
	}
	t_socket_paquete *paquete_MSP = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(socketDelMSP, paquete_MSP) > 0){
		if(paquete_MSP->header.type == CREAR_SEGMENTO){
			log_info(logger, "recibiendo direccion virtual ", tcb->pid);
			char *contenido = malloc(sizeof(uint32_t));
			memcpy(contenido, paquete_MSP->data, sizeof(char)*4);
			/*resguardo la direccion virtual en el registro A*/
			tcb->registro_de_programacion[0]=*contenido;
			//break;
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
	free(data);
	free(grabar_bytes);

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

	if (socket_sendPaquete((t_socket*)socketDelMSP, DESTRUIR_SEGMENTO, leer_byte->tamanio, leer_byte->data)<=0){
		log_info(logger, "Error en envio de direccion a la MSP %d", tcb->pid);

	}
	free(data);
	free(leer_byte);

}

void INNN_ESO(t_TCB_CPU* tcb){


	char *data=malloc(sizeof(int)); /*TCB+direccion_SysCall*/
	int stmp_size=0;
	//memcpymemcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	//soffset=stmp_size;


//	if (socket_sendPaquete((t_socket*)socketDelKernel, ENTRADA_ESTANDAR_NUMERO,stmp_size, data)<=0){  //22 corresponde a interrupcion
//		log_info(logger, "Error de Interrupcion %d", tcb->pid);
//		//cpuCambioDeContextoError();
//
//	}
	free(data);

	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
//	if(socket_recvPaquete(socketDelMSP, paquete_KERNEL) > 0){
//		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR_NUMERO){
//			log_info(logger, "recibiendo numero ingresado por consola ", tcb->pid);
//			char *contenido = malloc(sizeof(uint32_t));
//			memcpy(contenido, paquete_KERNEL->data, sizeof(char)*4);
//			/*resguardo la direccion virtual en el registro A*/
//			tcb->registro_de_programacion[0]=contenido;
//			//break;
//		} else {
//			log_error(logger, "Se recibio un codigo inesperado de MSP: %d", paquete_KERNEL->header.type);
//			//cpuCambioDeContextoError();
//		}
//	}else{
//		log_info(logger, "MSP ha cerrado su conexion");
//		//cpuCambioDeContextoError();
//		printf("MSP ha cerrado su conexion\n");
//		exit(-1);
//	}

	free(paquete_KERNEL);

	usleep(100);

}
void INNC_ESO(t_TCB_CPU* tcb){

	t_paquete_MSP *pedir_bytes = malloc(sizeof(t_paquete_MSP));

	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+(tamanio)registro_de_programacion['B']*/
	int soffset, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	memcpy(data + soffset, tcb->registro_de_programacion[1], stmp_size=sizeof(int32_t));
	soffset+=stmp_size;
	pedir_bytes->tamanio=soffset;
	pedir_bytes->data=data;

//	if (socket_sendPaquete((t_socket*)socketDelKernel, ENTRADA_ESTANDAR_CHAR,pedir_bytes->tamanio, pedir_bytes->data)<=0){  //22 corresponde a interrupcion
//		log_info(logger, "Error de Interrupcion %d", tcb->pid);
//		//cpuCambioDeContextoError();
//
//	}
	free(data);
	free(pedir_bytes);

	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
//	if(socket_recvPaquete(socketDelMSP, paquete_KERNEL) > 0){
//		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR_CHAR){
//			log_info(logger, "recibiendo CADENA ingresado por consola ", tcb->pid);
//			char *contenido = malloc(sizeof(uint32_t));
//			memcpy(contenido, paquete_KERNEL->data, sizeof(char));
//			//if(contenido<=tcb->registro_de_programacion[1]){
//
//			/*resguardo en la direccion virtual apuntada por el registro A*/
//			//if contenido<=tcb->registro_de_programacion[0]{
//			char *data=malloc(sizeof(int)+sizeof(uint32_t)+sizeof(char));  /*pid+direccion_logica+datos_a_grabar*/
//			t_paquete_MSP *grabar_byte = malloc(sizeof(t_paquete_MSP));
//			int soffset=0, stmp_size=0;
//			memcpy(grabar_byte, &(tcb->pid), stmp_size=(sizeof(int)));
//			soffset=stmp_size;
//			memcpy(grabar_byte + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(uint32_t));
//			soffset+=stmp_size;
//			memcpy(grabar_byte + soffset, contenido, stmp_size=sizeof(char));
//			soffset+=stmp_size;
//
//			grabar_byte->tamanio = soffset;
//			grabar_byte->data = data;
//
//			if (socket_sendPaquete((t_socket*)socketDelMSP, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
//				log_info(logger, "Error de escritura en MSP %d", tcb->pid);
//				//cpuCambioDeContextoError();
//			}
//			//}else{}
//		} else {
//			log_error(logger, "Se recibio un codigo inesperado de MSP: %d", paquete_KERNEL->header.type);
//			//cpuCambioDeContextoError();
//		}
//	}else{
//		log_info(logger, "MSP ha cerrado su conexion");
//		//cpuCambioDeContextoError();
//		printf("MSP ha cerrado su conexion\n");
//		exit(-1);
//	}

	free(paquete_KERNEL);

	usleep(100);

}
void OUTN_ESO(t_TCB_CPU* tcb){

}
void OUTC_ESO(t_TCB_CPU* tcb){

}
void CREA_ESO(t_TCB_CPU* tcb){

}
void JOIN_ESO(t_TCB_CPU* tcb){

}

void BLOK_ESO(t_TCB_CPU* tcb){

}

void WAKE_ESO(t_TCB_CPU* tcb){

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
