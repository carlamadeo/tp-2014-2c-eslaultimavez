#include "codigoESO.h"
#include "CPU_Proceso.h"
#include "commons/cpu.h"
#include "commons/string.h"
#include "cpuMSP.h"
#include "cpuKernel.h"
#include "ejecucion.h"

t_list* parametros;

int LOAD_ESO(t_CPU *self){

	char registro;
	int reg;
	int32_t numero;
	int tamanio = 5;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU,"CPU: Recibiendo parametros de instruccion LOAD");

		memcpy(&(registro), lecturaDeMSP, sizeof(char));
		memcpy(&(numero), lecturaDeMSP + sizeof(char), sizeof(int));

		printf("un registro de MSP %c : Jorge Ok\n", registro);
		printf("un Numero de MSP   %d : Jorge estan seguro que este numero va?\n", numero);

		reg = determinar_registro(registro);

	}

	if(reg != -1){

		//list_add(parametros, (void *)registro);
		//list_add(parametros, (void *)numero);
		//ejecucion_instruccion("LOAD", parametros);

		self->tcb->registro_de_programacion[reg] = numero;
		//cambio_registros(registros_cpu);
		log_info(self->loggerCPU, "CPU: LOAD ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	}

	else{
		log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
		estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
	}

	return estado_bloque;
}


int GETM_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GETM");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		free(lecturaDeMSP);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("GETM", parametros);

			int tamanioMSP = sizeof(int32_t);
			char *lecturaMSP = malloc(sizeof(char)*tamanioMSP);

			estado_lectura = cpuLeerMemoria(self, (uint32_t)self->tcb->registro_de_programacion[regB], lecturaMSP, tamanioMSP);

			estado_bloque = estado_lectura;

			log_info(self->loggerCPU, "CPU: GETM ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

			free(lecturaMSP);

		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}
	}

	return estado_bloque;
}


int SETM_ESO(t_CPU *self){

	int tamanio = 6;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int numero, regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SETM");

		memcpy(&(numero), lecturaDeMSP, sizeof(int));
		memcpy(&(registroA), lecturaDeMSP + sizeof(int), sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(int) + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			if(numero <= sizeof(uint32_t)){

				//list_add(parametros, (void *)registroA);
				//list_add(parametros, (void *)registroB);
				//list_add(parametros, (void *)numero);
				//ejecucion_instruccion("SETM", parametros);

				char* byte_a_escribir = malloc(sizeof(int32_t));
				memcpy(byte_a_escribir, &(self->tcb->registro_de_programacion[regB]), numero);
				estado_bloque = cpuEscribirMemoria(self, (uint32_t)self->tcb->registro_de_programacion[regA], byte_a_escribir, numero);

				log_info(self->loggerCPU, "CPU: SETM ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

				free(byte_a_escribir);

			}

		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;
}


int MOVR_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MOVR");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);

			//ejecucion_instruccion("MOVR", parametros);

			self->tcb->registro_de_programacion[regA] = self->tcb->registro_de_programacion[regB];

			log_info(self->loggerCPU, "CPU: MOVR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			//cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}
	}

	free(lecturaDeMSP);
	return estado_bloque;
}


int ADDR_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion ADDR");

		memcpy(&(registroA), (lecturaDeMSP), sizeof(char));
		memcpy(&(registroB), (lecturaDeMSP) + sizeof(char), sizeof(char));
		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("ADDR", parametros);

			self->tcb->registro_de_programacion[0] = self->tcb->registro_de_programacion[regA] + self->tcb->registro_de_programacion[regB];

			log_info(self->loggerCPU, "CPU: ADDR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			//cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;
}


int SUBR_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SUBR");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char),sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("SUBR", parametros);

			self->tcb->registro_de_programacion[0] = self->tcb->registro_de_programacion[regA] - self->tcb->registro_de_programacion[regB];

			log_info(self->loggerCPU, "CPU: SUBR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			//cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}

int MULR_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MULR");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("MULR", parametros);

			self->tcb->registro_de_programacion[0] = self->tcb->registro_de_programacion[regA] * self->tcb->registro_de_programacion[regB];

			log_info(self->loggerCPU, "CPU: MULR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			//cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int MODR_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion MODR");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("MODR", parametros);

			self->tcb->registro_de_programacion[0] = self->tcb->registro_de_programacion[regA] % self->tcb->registro_de_programacion[regB];

			log_info(self->loggerCPU, "CPU: MODR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			//cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int DIVR_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion DIVR");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("DIVR", parametros);

			//int32_t auxiliar = self->tcb->registro_de_programacion[regB];

			if (self->tcb->registro_de_programacion[regB] != 0){
				self->tcb->registro_de_programacion[0] = self->tcb->registro_de_programacion[regA] / self->tcb->registro_de_programacion[regB];
				log_info(self->loggerCPU, "CPU: DIVR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			}

			else{
				log_error(self->loggerCPU, "CPU: error de intento de division por cero");
				estado_bloque = ERROR_POR_EJECUCION_ILICITA; //TODO No se que poner, no podemos poner un error por cada cosa, ver como solucionarlo!!
			}
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int INCR_ESO(t_CPU *self){

	int tamanio = 1;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int reg;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "Recibiendo parametros de instruccion INCR");

		memcpy(&(registro), (lecturaDeMSP), sizeof(char));

		reg = determinar_registro(registro);

		if((reg != -1)){

			//list_add(parametros, (void *)registro);
			//ejecucion_instruccion("INCR", parametros);

			self->tcb->registro_de_programacion[reg]++;

			log_info(self->loggerCPU, "CPU: INCR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int DECR_ESO(t_CPU *self){

	int tamanio = 1;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int reg;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "Recibiendo parametros de instruccion DECR");

		memcpy(&(registro), lecturaDeMSP, sizeof(char));

		reg = determinar_registro(registro);

		if((reg != -1)){

			//list_add(parametros, (void *)registro);
			//ejecucion_instruccion("DECR", parametros);

			self->tcb->registro_de_programacion[reg]--;

			log_info(self->loggerCPU, "CPU: DECR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int COMP_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "Recibiendo parametros de instruccion COMP");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("COMP", parametros);

			if (self->tcb->registro_de_programacion[regA] == self->tcb->registro_de_programacion[regB])
				self->tcb->registro_de_programacion[0] = 1;

			else
				self->tcb->registro_de_programacion[0] = 0;

			log_info(self->loggerCPU, "CPU: COMPR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int CGEQ_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CGEQ");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("CGEQ", parametros);

			if (self->tcb->registro_de_programacion[regA] >= self->tcb->registro_de_programacion[regB])
				self->tcb->registro_de_programacion[0] = 1;

			else
				self->tcb->registro_de_programacion[0] = 0;

			log_info(self->loggerCPU, "CPU: CGEQ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int CLEQ_ESO(t_CPU *self){

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion CLEQ");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) || (regB != -1)){

			//list_add(parametros, (void *)registroA);
			//list_add(parametros, (void *)registroB);
			//ejecucion_instruccion("CLEQ", parametros);

			if (self->tcb->registro_de_programacion[regA] <= self->tcb->registro_de_programacion[regB])
				self->tcb->registro_de_programacion[0] = 1;

			else
				self->tcb->registro_de_programacion[0] = 0;

			log_info(self->loggerCPU, "CPU: CLEQ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int GOTO_ESO(t_CPU *self){

	int tamanio = 1;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int reg;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "Recibiendo parametros de instruccion GOTO");

		memcpy(&(registro), (lecturaDeMSP), sizeof(char));

		reg = determinar_registro(registro);

		if((reg != -1)){

			//list_add(parametros, (void *)registro);
			//ejecucion_instruccion("GOTO", parametros);

			if((self->tcb->base_segmento_codigo + self->tcb->registro_de_programacion[reg]) <= self->tcb->tamanio_segmento_codigo){
				self->tcb->puntero_instruccion = self->tcb->registro_de_programacion[reg];
				log_info(self->loggerCPU, "CPU: GOTO ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			}

			else
				estado_bloque = ERROR_POR_SEGMENTATION_FAULT;
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}

int JMPZ_ESO(t_CPU *self){

	int tamanio = 4;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	uint32_t direccion;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JMPZ");

		memcpy(&(direccion), lecturaDeMSP, sizeof(uint32_t));

		//list_add(parametros, (void *)direccion);
		//ejecucion_instruccion("JMPZ", parametros);

		if(self->tcb->registro_de_programacion[0] == 0){

			if((self->tcb->base_segmento_codigo + direccion) <= self->tcb->tamanio_segmento_codigo){
				self->tcb->puntero_instruccion = direccion;
				log_info(self->loggerCPU, "CPU: JMPZ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			}

			else
				estado_bloque = ERROR_POR_SEGMENTATION_FAULT;

		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int JPNZ_ESO(t_CPU *self){

	int tamanio = 4;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	uint32_t direccion;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JPNZ");

		memcpy(&(direccion), lecturaDeMSP, sizeof(uint32_t));

		//list_add(parametros, (void *)direccion);
		//ejecucion_instruccion("JPNZ", parametros);

		if(self->tcb->registro_de_programacion[0] != 0){

			if((self->tcb->base_segmento_codigo + direccion) <= self->tcb->tamanio_segmento_codigo){
				self->tcb->puntero_instruccion = direccion;
				log_info(self->loggerCPU, "CPU: JPNZ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			}

			else
				estado_bloque = ERROR_POR_SEGMENTATION_FAULT;

		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int INTE_ESO(t_CPU *self){

	int tamanio = 4;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	uint32_t direccion;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion INTE");

		memcpy(&(direccion), lecturaDeMSP, sizeof(uint32_t));
		//list_add(parametros, (void *)direccion);
		//ejecucion_instruccion("INTE", parametros);

		cpuEnviaInterrupcion(self, direccion);

		log_info(self->loggerCPU, "CPU: INTE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

	}

	free(lecturaDeMSP);
	return estado_bloque;

	//tengo que tomar esa direccion y pasarcela al Kernel con el TCB

}

//void FLCL(t_CPU *self){
//
//}

int SHIF_ESO(t_CPU *self){

	int tamanio = 5;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int numero, reg;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SHIF");

		memcpy(&(numero), lecturaDeMSP, sizeof(int32_t));
		memcpy(&(registro), (lecturaDeMSP) + sizeof(char), sizeof(char));

		reg = determinar_registro(registro);

		if((reg != -1)){

			//list_add(parametros, (void *)numero);
			//list_add(parametros, (void *)registro);
			//ejecucion_instruccion("SHIF", parametros);

			if(numero > 0)
				self->tcb->registro_de_programacion[reg]>>=numero;
			else
				self->tcb->registro_de_programacion[reg]<<=numero;

			log_info(self->loggerCPU, "CPU: SHIF ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		}

	}

	free(lecturaDeMSP);
	return estado_bloque;

}


int NOPP_ESO(t_CPU *self){
	//no hace nada

	log_info(self->loggerCPU, "CPU: NOPP ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	return 1;
}


int PUSH_ESO(t_CPU *self){

	int tamanio = 5;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int numero, reg;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion PUSH");

		memcpy(&(numero), lecturaDeMSP, sizeof(int32_t));
		memcpy(&(registro), lecturaDeMSP + sizeof(char), sizeof(char));
		reg = determinar_registro(registro);

		if((reg != -1)){

			//list_add(parametros, (void *)numero);
			//list_add(parametros, (void *)registro);
			//ejecucion_instruccion("PUSH", parametros);

			if(numero <= sizeof(uint32_t)){

				char* byte_a_escribir = malloc(sizeof(int32_t));

				memcpy(byte_a_escribir, &(self->tcb->registro_de_programacion[reg]), numero);
				estado_bloque = cpuEscribirMemoria(self, (uint32_t)self->tcb->cursor_stack, byte_a_escribir, numero);

				free(byte_a_escribir);

				if (estado_bloque == SIN_ERRORES){
					self->tcb->cursor_stack += numero;
					log_info(self->loggerCPU, "CPU: PUSH ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				}
			}
		}
	}

	free(lecturaDeMSP);
	return estado_bloque;
}


int TAKE_ESO(t_CPU *self){

	int tamanio = 5;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int numero, reg;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion TAKE");

		memcpy(&(numero), lecturaDeMSP, sizeof(int32_t));
		memcpy(&(registro), lecturaDeMSP + sizeof(char), sizeof(char));

		reg = determinar_registro(registro);

		if((reg != -1)){

			//list_add(parametros, (void *)numero);
			//list_add(parametros, (void *)registro);
			//ejecucion_instruccion("TAKE", parametros);

			if(numero <= sizeof(uint32_t)){

				char *lecturaDeMSP2 = malloc(sizeof(char) * numero + 1);

				//se hace el control para saber a donde apuntar dependiendo de si se trata un tcb usuario o kernel...
				int estado_lectura = cpuLeerMemoria(self, self->tcb->cursor_stack, lecturaDeMSP2, numero);
				estado_bloque = estado_lectura;

				if (estado_lectura == SIN_ERRORES){
					self->tcb->registro_de_programacion[reg] = (int32_t)lecturaDeMSP2;
					self->tcb->cursor_stack -= numero;
					log_info(self->loggerCPU, "CPU: TAKE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				}

				free(lecturaDeMSP2);
			}
		}

		else
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
	}

	free(lecturaDeMSP);
	return estado_bloque;
}


int XXXX_ESO(t_CPU *self){

	fin_ejecucion();

	char *data = malloc(sizeof(t_TCB_CPU));
	memcpy(data, self->tcb, sizeof(t_TCB_CPU));
	int estado = cpuFinalizarProgramaExitoso(self, data);

	log_info(self->loggerCPU, "CPU: TAKE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	free(data);
	return estado;

}


//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

int MALC_ESO(t_CPU *self){
	//CREAR_SEGMENTO
	int estado_malc = cpuCrearSegmento(self, self->tcb->pid, self->tcb->registro_de_programacion[0]);
	switch(estado_malc){
	case ERROR_POR_TAMANIO_EXCEDIDO:
		log_info(self->loggerCPU, "CPU: MALC ERROR_POR_TAMANIO_EXCEDIDO  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return ERROR_POR_TAMANIO_EXCEDIDO;
	case ERROR_POR_MEMORIA_LLENA:
		log_info(self->loggerCPU, "CPU: MALC ERROR_POR_MEMORIA_LLENA  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return ERROR_POR_MEMORIA_LLENA;
	case ERROR_POR_NUMERO_NEGATIVO:
		log_info(self->loggerCPU, "CPU: MALC ERROR_POR_MEMORIA_LLENA  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return ERROR_POR_NUMERO_NEGATIVO;
	case ERROR_POR_SEGMENTO_INVALIDO:
		log_info(self->loggerCPU, "CPU: MALC ERROR_POR_MEMORIA_LLENA  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return ERROR_POR_SEGMENTO_INVALIDO;
	case ERROR_POR_SEGMENTATION_FAULT:
		log_info(self->loggerCPU, "CPU: MALC ERROR_POR_MEMORIA_LLENA  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return ERROR_POR_SEGMENTATION_FAULT;
	default:
		self->tcb->registro_de_programacion[0] = estado_malc;
		log_info(self->loggerCPU, "CPU: MALC ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return SIN_ERRORES;
	}
}
int FREE_ESO(t_CPU *self){
	int estado_free;
	t_destruirSegmento* destruir_segmento = malloc(sizeof(t_destruirSegmento));

	destruir_segmento->direccionVirtual = self->tcb->registro_de_programacion[0];
	destruir_segmento->pid = self->tcb->pid;
	estado_free = cpuDestruirSegmento(self);
	if(estado_free == ERROR_POR_SEGMENTO_DESCONOCIDO){
		log_info(self->loggerCPU, "CPU: FREE ejecutado con ERROR_POR_SEGMENTO_DESCONOCIDO para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	} else {
		log_info(self->loggerCPU, "CPU: FREE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			}
	free(destruir_segmento);
	return estado_free;
}

int INNN_ESO(t_CPU *self){

	int estado_innn;
	t_entrada_estandar* entrada_estandar = malloc(sizeof(t_entrada_estandar));
	entrada_estandar->pid = self->tcb->pid;
	entrada_estandar->tamanio = sizeof(int);
	entrada_estandar->tipo = 1;

	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR,sizeof(t_entrada_estandar), entrada_estandar)<=0){
		log_info(self->loggerCPU, "CPU: INNN ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		estado_innn = MENSAJE_DE_ERROR;
	}

	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR){
			log_info(self->loggerCPU, "CPU: INNN ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			self->tcb->registro_de_programacion[0]=(int)paquete_KERNEL;
			estado_innn = ENTRADA_ESTANDAR;
		} else {
			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP: %d", paquete_KERNEL->header.type);
			estado_innn = ERROR_POR_CODIGO_INESPERADO;

		}
	}else{
		log_info(self->loggerCPU, "CPU: INNN ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		printf("CPU: MSP ha cerrado su conexion\n");

	}

	free(paquete_KERNEL);
	free(entrada_estandar);
	return estado_innn;

}

int INNC_ESO(t_CPU *self){
	int estado_innc;
		t_entrada_estandar* pedir_cadena = malloc(sizeof(t_entrada_estandar));

		pedir_cadena->pid = self->tcb->pid;
		pedir_cadena->tamanio = self->tcb->registro_de_programacion[1];
		pedir_cadena->tipo  = 2; //2 es un string

		if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR,sizeof(t_entrada_estandar), pedir_cadena)<=0){  //22 corresponde a interrupcion
			log_info(self->loggerCPU, "CPU: INNC ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		}
		free(pedir_cadena);

		t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
		if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR){
				log_info(self->loggerCPU, "CPU: recibiendo CADENA ingresado por consola ", self->tcb->pid);
				char *cadena = malloc(self->tcb->registro_de_programacion[1]);
				memcpy(cadena, paquete_KERNEL->data, self->tcb->registro_de_programacion[1]);
				int estadoEscritura = cpuEscribirMemoria(self, self->tcb->registro_de_programacion[0], cadena, sizeof(self->tcb->registro_de_programacion[1]));

				if(estadoEscritura == ERROR_POR_SEGMENTATION_FAULT){
					log_info(self->loggerCPU, "CPU: INNC ejecutado con ERROR_POR_SEGMENTATION_FAULT para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
					estado_innc = ERROR_POR_SEGMENTATION_FAULT;
				} else {
				free(cadena);
				log_info(self->loggerCPU, "CPU: INNC ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				estado_innc = ENTRADA_ESTANDAR;
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


int OUTN_ESO(t_CPU *self){

	t_salida_estandar* salida_estandar = malloc(sizeof(t_salida_estandar));
	salida_estandar->pid = self->tcb->pid;
	salida_estandar->cadena = string_itoa(self->tcb->registro_de_programacion[0]);
	if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR,sizeof(t_salida_estandar), salida_estandar)<=0){  //22 corresponde a interrupcion
		log_info(self->loggerCPU, "CPU: OUTN ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		return MENSAJE_DE_ERROR;
	}
	free(salida_estandar);
	return SALIDA_ESTANDAR;
}

int OUTC_ESO(t_CPU* self){

	int estado_outc;
	char* lecturaDeMSP = malloc(self->tcb->registro_de_programacion[1]);

	int estado_lectura = cpuLeerMemoria(self, self->tcb->cursor_stack,  lecturaDeMSP, self->tcb->registro_de_programacion[1]);
	if (estado_lectura == ERROR_POR_SEGMENTATION_FAULT){
		estado_outc = ERROR_POR_SEGMENTATION_FAULT;
	}else{
			t_salida_estandar* salida_estandar = malloc(sizeof(t_salida_estandar));
			salida_estandar->pid = self->tcb->pid;
			salida_estandar->cadena = lecturaDeMSP;
			if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR,sizeof(t_salida_estandar), salida_estandar)<=0){  //22 corresponde a interrupcion
						log_info(self->loggerCPU, "CPU: OUTC ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
						estado_outc = MENSAJE_DE_ERROR;
			}
			free(salida_estandar);
			log_info(self->loggerCPU, "CPU: OUTN ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			estado_outc = SALIDA_ESTANDAR;
	}
	return estado_outc;

}
int CREA_ESO(t_CPU *self){ 	// CREA un hilo hijo de TCB
	int estado_crea;
	t_crea_hilo* crear_hilo = malloc(sizeof(t_crea_hilo));
	crear_hilo->tcb = self->tcb;
	if (socket_sendPaquete(self->socketPlanificador->socket, CREAR_HILO,sizeof(t_crea_hilo), crear_hilo)<=0){  //22 corresponde a interrupcion
		log_info(self->loggerCPU, "CPU: CREA ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				estado_crea = MENSAJE_DE_ERROR;
	}
	free(crear_hilo);
	log_info(self->loggerCPU, "CPU: CREA ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	estado_crea = CREAR_HILO;
	return estado_crea;

}
int JOIN_ESO(t_CPU *self){
	int estado_join;
	t_join* joinear = malloc(sizeof(t_join));
	joinear->tid_llamador = self->tcb->tid;
	joinear->tid_esperar = self->tcb->registro_de_programacion[0];
	if (socket_sendPaquete(self->socketPlanificador->socket, JOIN_HILO,sizeof(t_join), joinear)<=0){  //22 corresponde a interrupcion
		log_info(self->loggerCPU, "CPU: JOIN ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				estado_join = MENSAJE_DE_ERROR;
	}
	free(joinear);
	log_info(self->loggerCPU, "CPU: JOIN ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	estado_join = JOIN_HILO;
	return estado_join;

}


int BLOK_ESO(t_CPU *self){
	int estado_blok;
	 t_bloquear* blocker = malloc(sizeof(t_bloquear));
	 blocker->tcb = self->tcb;
	 blocker->id_recurso = self->tcb->registro_de_programacion[1];
	 if (socket_sendPaquete(self->socketPlanificador->socket, BLOK_HILO,sizeof(t_bloquear), blocker)<=0){
	 		log_info(self->loggerCPU, "CPU: BLOK ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	 				estado_blok = MENSAJE_DE_ERROR;
	 	} else{
	 	log_info(self->loggerCPU, "CPU: BLOK ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	 	estado_blok = JOIN_HILO;
	 	}
	 	free(blocker);
	 	return estado_blok;

}

int WAKE_ESO(t_CPU *self){
	int estado_wake;
	if (socket_sendPaquete(self->socketPlanificador->socket, WAKE_HILO ,sizeof(int32_t), &(self->tcb->registro_de_programacion[1]))<=0){
		log_info(self->loggerCPU, "CPU: WAKE ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		estado_wake = MENSAJE_DE_ERROR;

	}else{
		log_info(self->loggerCPU, "CPU: WAKE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		estado_wake = WAKE_HILO;
	}
	return estado_wake;
}


//
//
//
////funciones de la system calls
//void systemCallsMALLOC(){
//
//}
//void systemCallsFREE(){
//
//}
//void systemCallsINN(){
//
//}
//void systemCallsINC(){
//
//}
//void systemCallsOUTN(){
//
//}
//void systemCallsOUTC(){
//
//}
//void systemCallsCREATE_THREAD(){
//
//}
//void systemCallsJOIN_THREAD(){
//
//}
//void systemCallsWAIT(){
//
//}
//
//void systemCallsSIGNAL(){
//
//}
//
//void systemCallsSETSEM(){}
// */
