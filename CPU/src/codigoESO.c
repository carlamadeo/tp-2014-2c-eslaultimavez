#include "codigoESO.h"
#include "CPU.h"
#include "cpuMSP.h"
#include "cpuKernel.h"
#include "commons/protocolStructInBigBang.h"
#include "cpu.h"


void LOAD_ESO (int registro, int32_t numero, t_TCB_CPU* tcb){
	if(registro!=-1){
		tcb->registro_de_programacion[registro]=numero;
		cambio_registros(registros_cpu);

	}
	log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado\n %d", tcb->pid);
}

int GETM_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){

	if((primer_registro!=-1)&&(segundo_registro!=-1)){


		t_lectura_MSP * lecturaDeMSP = malloc(sizeof(t_lectura_MSP));
		t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
		//se hace el control para saber a donde apuntar dependiendo de si se trata un tcb usuario o kernel...
		unCPU_LEER_MEMORIA->pid = tcb->pid;
		unCPU_LEER_MEMORIA->tamanio = sizeof(int32_t);
		unCPU_LEER_MEMORIA->direccionVirtual = (uint32_t)tcb->registro_de_programacion[1];

		int estado_lectura = cpuLeerMemoria(self, unCPU_LEER_MEMORIA->pid, unCPU_LEER_MEMORIA->direccionVirtual, lecturaDeMSP->data, unCPU_LEER_MEMORIA->tamanio, self->socketMSP->socket);

		if (estado_lectura == ERROR_POR_SEGMENTATION_FAULT) return ERROR_POR_SEGMENTATION_FAULT;




		/*

		char *data=malloc(sizeof(int)+sizeof(uint32_t)); //pid+direccion_logica
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
				//resguardo el contenido en primer registro
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

		*/
	}

	log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", tcb->pid);
	return SIN_ERRORES;
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

		cambio_registros(registros_cpu);

		if (socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
			log_info(self->loggerCPU, "CPU: fallo: ESCRIBIR_MEMORIA\n %d", tcb->pid);

		}
	}

}


void MOVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	tcb->registro_de_programacion[primer_registro]=tcb->registro_de_programacion[segundo_registro];
	cambio_registros(registros_cpu);
}


void ADDR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]+auxiliar;
	cambio_registros(registros_cpu);
}

void SUBR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]-auxiliar;
	cambio_registros(registros_cpu);
}

void MULR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]*auxiliar;
	cambio_registros(registros_cpu);
}



void MODR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	tcb->registro_de_programacion[0]=tcb->registro_de_programacion[primer_registro]%auxiliar;
	cambio_registros(registros_cpu);
}

void DIVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb){
	int32_t auxiliar=tcb->registro_de_programacion[segundo_registro];
	if (auxiliar==0){
		log_info(self->loggerCPU, "fallo de division por cero %d", tcb->pid);
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
		log_info(self->loggerCPU, "Error de Interrupcion %d", tcb->pid);


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

int MALC_ESO (t_TCB_CPU* tcb){
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

	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, alocar_bytes->tamanio, alocar_bytes->data)<=0){
		log_info(self->loggerCPU, "CPU: Error de memoria llena en MSP\n %d", tcb->pid);
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

	}
	free(alocar_bytes);
	free(data);
}


int FREE_ESO(t_TCB_CPU* tcb){

	char *data=malloc(sizeof(int)+sizeof(uint32_t)); /*pid+tamaño segun registro*/
	t_paquete_MSP *leer_byte = malloc(sizeof(t_paquete_MSP));

	int soffset=0, stmp_size=0;
	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
	soffset=stmp_size;
	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(uint32_t));
	soffset+=stmp_size;

	leer_byte->tamanio = soffset;
	leer_byte->data = data;

	if (socket_sendPaquete(self->socketMSP->socket, DESTRUIR_SEGMENTO, leer_byte->tamanio, leer_byte->data)<=0){
		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);

	}
	free(data);
	free(leer_byte);

}

int INNN_ESO(t_TCB_CPU* tcb){


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

	}

	free(paquete_KERNEL);

}
void INNC_ESO(t_TCB_CPU* tcb){

int INNC_ESO(t_TCB_CPU* tcb){
	int estado_innc;
	t_entrada_estandar* pedir_cadena = malloc(sizeof(t_entrada_estandar));

	pedir_cadena->pid = tcb->pid;
	pedir_cadena->tamanio = tcb->registro_de_programacion[1];
	pedir_cadena->tipo  = 2; //2 es un string

	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR,sizeof(t_entrada_estandar), pedir_cadena)<=0){  //22 corresponde a interrupcion
		log_info(self->loggerCPU, "CPU: Error de ENTRADA_ESTANDAR_CHAR\n %d", tcb->pid);
	}
	free(pedir_cadena);

	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
	if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR){
			log_info(self->loggerCPU, "CPU: recibiendo CADENA ingresado por consola ", tcb->pid);
			char *cadena = malloc(tcb->registro_de_programacion[1]);
			t_datos_aMSP escritura_msp = malloc(sizeof(tcb->registro_de_programacion[1]));
			memcpy(cadena, paquete_KERNEL->data, tcb->registro_de_programacion[1]);
			int estadoEscritura = cpuEscribirMemoria(self, tcb->registro_de_programacion[0], cadena, sizeof(tcb->registro_de_programacion[1]));

			if(estadoEscritura == ERROR_POR_SEGMENTATION_FAULT){
				//no uso directamente el return porque sino nunca libero el paquete_KERNEL y devuelvo un estado que puede variar
				estado_innc = ERROR_POR_SEGMENTATION_FAULT;
			} else {
			free(cadena);
			estado_innc = SIN_ERRORES;
			}
	}else{
		log_info(self->loggerCPU, "CPU: codigo inesperado de MSP");
		printf("CPU: codigo inesperado de MSP\n");
		estado_innc = MENSAJE_DE_ERROR;
		}
	free(paquete_KERNEL);
	}
	return estado_innc;
}

int OUTN_ESO(t_TCB_CPU* tcb){

	t_paquete_MSP *mostrar_numero = malloc(sizeof(t_paquete_MSP));

	char *data=malloc(sizeof(int)+sizeof(int32_t)); /*pid+(tamanio)registro_de_programacion['B']*/
	int soffset=0, stmp_size=0;
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
int OUTC_ESO(t_TCB_CPU* tcb){

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

				if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR ,soffset, data)<=0){
					log_info(self->loggerCPU, "CPU: Error de SALIDA_ESTANDAR_CHAR\n %d", tcb->pid);

				}
				free(data);
				free(mostrar_cadena);
		}
	}


}
int CREA_ESO(t_TCB_CPU* tcb){ 	// CREA un hilo hijo de TCB
	t_TCB_CPU* tcb_hijo=malloc(sizeof(t_TCB_CPU));
	tcb_hijo->pid=tcb->registro_de_programacion[0];
	tcb_hijo->tid=(tcb->tid)+1;
	tcb_hijo->km=0;				//se pasa a modo usuario
	tcb_hijo->puntero_instruccion=tcb->registro_de_programacion[1];

	//creando stack del tcb hijo...
	// tengo que pedirle al kernel que me cree el segmento de stack y luego recibir la base, despues duplicar el contenido del stack del padre y actualizar el cursor (cursor-base del padre)

	//CPU->KERNEL : pidiendo al Kernel que cree el STACK del TCB hijo
	char *pedir_stack=malloc(sizeof(int)); /*pid+(tamanio)registro_de_programacion['A']*/
	memcpy(pedir_stack, &(tcb_hijo->pid), sizeof(int));
	log_info(self->loggerCPU, "CPU: solicitando a Kernel creacion de stack PID:\n %s", tcb_hijo->pid);
	if (socket_sendPaquete(self->socketPlanificador->socket, /*CREAR_STACK*/35, sizeof(int), pedir_stack)<=0){
		log_info(self->loggerCPU, "CPU: Error al pedir stack\n %d", tcb->pid);
	}
	t_socket_paquete *base_stack = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketPlanificador->socket, base_stack) > 0){
		if(base_stack->header.type == /*CREAR_STACK*/35){
			log_info(self->loggerCPU, "CPU: recibiendo base de stack...\n %d ", tcb->pid);
			tcb_hijo->base_stack = (int32_t)(base_stack->data);
			free(base_stack);
			} else {
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de Kernel:\n %d", base_stack->header.type);
					}
	}else{
		log_info(self->loggerCPU, "CPU: Kernel ha cerrado su conexion");
		printf("Kernel ha cerrado su conexion\n");

	}
	free(pedir_stack);

	// fin crear stack del tcb hijo...

	// inicializando Stack...
	//primero: leer el STACK completo del padre

	t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
	unCPU_LEER_MEMORIA->pid = self->tcb->pid;
	unCPU_LEER_MEMORIA->tamanio = 1024;
	unCPU_LEER_MEMORIA->direccionVirtual = tcb->base_stack;
	if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_CPU_LEER_MEMORIA), unCPU_LEER_MEMORIA)<=0){
		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
	}

	t_socket_paquete *datos_memoria = malloc(sizeof(t_socket_paquete));

	if(socket_recvPaquete(self->socketMSP->socket, datos_memoria) > 0){
		if(datos_memoria->header.type == LEER_MEMORIA){
			char *stack_tcb = malloc(1024);
			memcpy(stack_tcb, datos_memoria->data, 1024);
			//segundo: copiar el contenido del stack recibido en el stack del tcb_hijo: ESCRIBIR_MEMORIA
			char *data=malloc(sizeof(int)+sizeof(uint32_t)+ tcb->registro_de_programacion[1]);  /*pid+direccion_logica+datos_a_grabar*/
			t_paquete_MSP *grabar_byte = malloc(sizeof(t_paquete_MSP));
			int soffset=0, stmp_size=0;
			memcpy(grabar_byte, &(tcb_hijo->pid), stmp_size=(sizeof(int)));
			soffset=stmp_size;
			memcpy(grabar_byte + soffset, &(tcb_hijo->base_stack), stmp_size=sizeof(uint32_t));
			soffset+=stmp_size;
			memcpy(grabar_byte + soffset, stack_tcb, stmp_size=1024);
			soffset+=stmp_size;

			grabar_byte->tamanio = soffset;
			grabar_byte->data = data;

			if (socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
				log_info(self->loggerCPU, "CPU: Error de escritura en MSP\n %d", tcb->pid);
			}

		}
	}

	//tercero: inicializar el cursor de stack del tcb_hijo
	uint32_t avance_stack = tcb->cursor_stack - tcb->base_stack;
	tcb_hijo->cursor_stack = avance_stack;

 	// fin inicializar Stack...

	// enviando a Kernel el TCB a planificar..

	log_info(self->loggerCPU, "CPU: solicitando creacion de TCB HIJO...\n %d", tcb_hijo->pid);
	if (socket_sendPaquete(self->socketPlanificador->socket, CREAR_HILO, sizeof(t_TCB_CPU) , tcb_hijo)<=0){
				log_info(self->loggerCPU, "CPU: Error de CREAR_HILO_HIJO\n %d", tcb->pid);

	}
	free(tcb_hijo);

}
int JOIN_ESO(t_TCB_CPU* tcb){
	t_paquete_MSP *envio_join = malloc(sizeof(t_paquete_MSP));
	char *data=malloc((sizeof(int))+sizeof(int32_t)); /*pid+tid llamador+(tid a esperar)registro_de_programacion['B']*/
		int soffset=0, stmp_size=0;
		memcpy(data, &(tcb->tid), stmp_size=(sizeof(int)));
		memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(int32_t));
		soffset+=stmp_size;
		envio_join->tamanio=soffset;
		envio_join->data=data;

		if (socket_sendPaquete(self->socketPlanificador->socket, /* JOIN_HILO*/ 33,envio_join->tamanio, envio_join->data)<=0){
			log_info(self->loggerCPU, "CPU: Error de JOIN\n %d", tcb->pid);


		}
		free(data);
		free(envio_join);

}

int BLOK_ESO(t_TCB_CPU* tcb){

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

int WAKE_ESO(t_TCB_CPU* tcb){

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
