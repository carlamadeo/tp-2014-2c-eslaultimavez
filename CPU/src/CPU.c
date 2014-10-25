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
#include "string.h"
/*
 * En el main esta la logica principal, basicamente llamar a las funciones que hacen las tareas.
 * Tiene un par de errores para que las modifique, son errores de logica, así que metan mano y EXITOS chicos!
 */
int main(void) {
	logger = log_create(PATH_LOG, "CPU", 0, LOG_LEVEL_DEBUG); //Creo el archivo Log

	//Cargo la configuracion
	cpuCargar_configuracionCPU();

	t_socket_conexion* socket_MSP 	  = cpuConectarConMPS();
	t_socket_conexion* socket_kernel = cpuConectarConKernel();

	t_socket_client* socketKernel = socket_kernel->socket_client;
	t_socket_client* socketMSP 	  = socket_MSP->socket_client;

	socketDelKernel = socketKernel->socket;
	socketDelMSP	= socketMSP->socket;

	if( socketMSP != NULL ){
		log_debug(logger, "Conectado con MPS");
		cpuRealizarHandshakeConMSP();
	}

	if( socketKernel != NULL ){
		log_debug(logger, "Conectado con KERNEL");
		cpuRealizarHandshakeConKernel();
		//pregunte si hay un TCB para procesar, si hay: short existe_tcb_en_CPU =1;
	}

	short existe_tcb_en_CPU =1; //tiene que decidir donde se pone este valor  cero y cuando se tiene que poner en uno de vuelta.


	while(existe_tcb_en_CPU){
		//Recibo un TCB
		t_socket_paquete *paquete = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));

		if(socket_recvPaquete(socketDelKernel, paquete) >= 0){
			if( paquete->header.type == 67 ){ //CPU_NUEVO_TCB
				tcb = (t_TCB *) (paquete->data);
				log_info(logger, "El program vuelve con PC = %d", tcb->puntero_instruccion);
				log_info(logger, "TCB recibido con pid %d. Comienzo a ejecutar", tcb->pid);
				cpuProcesar_tcb(socketDelKernel);
				free(tcb);
			} else {
				log_error(logger, "Se recibio un codigo inesperado de KERNEL en main de cpu: %d", paquete->header.type);
			}
		} else {
			log_info(logger, "Kernel ha cerrado su conexion");
			printf("Kernel ha cerrado su conexion\n");
			exit(-1);
		}

		free(paquete);

		//Sleep para que no se tilde
		usleep(100);
		existe_tcb_en_CPU =0; //para cortar con el while

	}

	/* De aca en adelante, se tiene que eliminar porque se desconecto la CPU */

	log_info(logger, "Se desconecto la CPU. Elimino todo");
	exit(0);

	close(socketDelKernel->descriptor);
	close(socketDelMSP->descriptor);

	return 0;
}







/*
 * Cuando ya sepan que es lo que hace la CPU con los TCB que recibe recien ahí empiecen a ver
 * que es lo que tiene que procesar, con sus respectivas validaciones, y con quien se tiene que comunicar.
 *
 */
void cpuProcesar_tcb(t_socket* socketDelKernel){
	log_info(logger, "Comienzo a procesar el TCB %d", tcb->pid);
	/*diferenciar entre TCB kernel o user*/
	/*calculo la direccion real*/
	uint32_t direccion_real;
	/*pregunto si no me paso del segmento, caso afirmativo es segmentation fault*/
	if(direccion_real<tcb->tamanio_segmento_codigo){
	//suponiendo que 0 corresponde a que estoy enviando una direccion y estoy pidiendo los primeros 4 bytes
	if (socket_sendPaquete(*socketDelMSP, 0, sizeof(int32_t), direccion_real)<=0){
		log_info(logger, "Error en envio de direccion a la MSP %d", tcb->pid);

	t_socket_paquete *paquete_MSP = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(socketDelKernel, paquete_MSP) >= 0){
				if( paquete_MSP->header.type == 4 ){ //suponiendo que 4 es que estoy recibiendo los primeros 4 bytes de instruccion

					char linea[4];
					//char* linea = malloc(sizeof(char)*4);
					//linea = (char)paquete_MSP->data;
					log_info(logger, "recibo primeros 4 bytes de instruccion %d", tcb->puntero_instruccion);
					/*ejecuto la linea, aca solo tengo los primeros 4 bytes, en el case dependiendo de que
					 * instruccion se trate pido el resto de los paramtros*/
					cpuEjecutar_una_linea(linea);
				} else {
					log_error(logger, "Se recibio un codigo inesperado de MSP: %d", paquete_MSP->header.type);
				}
	}else{
		log_info(logger, "MSP ha cerrado su conexion");
		printf("MSP ha cerrado su conexion\n");
		exit(-1);
		}

	    free(paquete_MSP);
		//Sleep para que no se tilde
		usleep(100);

		} else {
			log_info(logger, "segmentation fault provocada por PID: %d", tcb->pid);}
		}
	}


int cpuDeterminarRegProg(char registro){
		switch (registro){
		case 'A': return 0;
		case 'B': return 1;
		case 'C': return 2;
		case 'D': return 3;
		case 'E': return 4;
		}

}
 void cpuEjecutar_una_linea(char linea){

	switch(linea){
		case "LOAD":
			/*tengo que pedir a la MSP el resto de la instruccion*/
			char registro_aux=string_substring(linea, 6, 6); //substring que contiene el registro
			int registro=cpuDeterminarRegProg(registro_aux);
			char numero_char=string_substring_from(linea,8); // numero a guardar en registro
			int32_t numero=(int32_t)(numero_char-48);
			LOAD(registro,numero);
			break;
		case "GETM":
		case "SETM":
		case "MOVR":
		case "ADDR":/*tengo que pedir a la MSP el resto de la instruccion*/
					char primer_registro_aux = string_substring(linea, 6, 6); //substring que contiene el 1registro
					int primer_registro=cpuDeterminarRegProg(registro_aux);
					char segundo_registro_aux=string_substring(linea, 9, 9); //substring que contiene el 2registro
					int segundo_registro=cpuDeterminarRegProg(registro_aux);
					ADDR(primer_registro,segundo_registro);
					break;
		case "SUBR":
		case "MULR":
		case "MODR":
		case "DIVR":
		case "INCR":
		case "DECR":
		case "COMP":
		case "CGEQ":
		case "CLEQ":
		case "GOTO":
		case "JMPZ":
		case "JPNZ":
		case "INTE":
		case "FLCL":
		case "SHIF":
		case "NOPP":
		case "PUSH":
		case "TAKE":
		case "XXXX":
		case "MALC":
		case "FREE":
		case "INNN":
		case "INNC":
		case "OUTN":
		case "OUTC":
		case "CREA":
		case "JOIN":
		case "BLOK":
		case "WAKE":
	}
}



/*
 * Cuando llegan aca tiene que hacer un super CASE e invocar la 34 funciones
 */





/*
 * Esta es una funcion corta que solo
 */
void cpuCargar_configuracionCPU(){
	t_config * config = config_create(PATH_CONFIG);//apunta al archivo configuracion
	if (!config_has_property(config, "IP_MSP")) {//Averigua si hay "IP" en archivo logs
		 log_error(logger, "Falta el IP donde se encuentra ejecutando el Proceso MSP.");//Carga en log el error
		 perror("Falta el IP donde se encuentra ejecutando el Proceso MSP.");
	 } else{
		 info_conexion_MSP.ip = config_get_string_value(config, "IP_MSP"); 		//saca IP de config
		 log_debug(logger, "IP_MSP = %s", info_conexion_MSP.ip);
	 }
	if (!config_has_property(config, "Puerto_MSP")) {
		 log_error(logger, "Falta el Puerto TCP donde se encuentra escuchando el Proceso MSP."); //Carga en log el error
		 perror("Falta el Puerto TCP donde se encuentra escuchando el Proceso MSP.");
	 } else{
		 info_conexion_MSP.port = config_get_int_value(config, "Puerto_MSP");
		 log_debug(logger, "Puerto_MSP = %d", info_conexion_MSP.port); //Carga en log el puertogo
	 }
	if (!config_has_property(config, "IP_KERNEL")) {//Averigua si hay "IP" en archivo logs
		 log_error(logger, "Falta el IP donde se encuentra ejecutando el Proceso Kernel.");//Carga en log el error
		 perror("Falta el IP donde se encuentra ejecutando el Proceso Kernel.");
	 } else{
		 info_conexion_KERNEL.ip = config_get_string_value(config, "IP_KERNEL"); 		//saca IP de config
		 log_debug(logger, "IP_KERNEL = %s", info_conexion_KERNEL.ip);
	 }
	if (!config_has_property(config, "Puerto_KERNEL")) {
		 log_error(logger, "Falta el Puerto TCP donde se encuentra escuchando el Proceso Kernel."); //Carga en log el error
		 perror("Falta el Puerto TCP donde se encuentra escuchando el Proceso Kernel.");
	 } else{
		 info_conexion_KERNEL.port = config_get_int_value(config, "Puerto_KERNEL");
		 log_debug(logger, "Puerto_KERNEL = %d", info_conexion_KERNEL.port); //Carga en log el puertogo
	 }
}








