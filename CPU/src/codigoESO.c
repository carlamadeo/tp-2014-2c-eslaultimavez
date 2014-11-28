#include "codigoESO.h"
#include "CPU_Proceso.h"
#include "commons/cpu.h"
#include "CPU_Proceso.h"
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

			int32_t auxiliar = self->tcb->registro_de_programacion[regB];

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

			self->tcb->registro_de_programacion[registro]++;

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

			self->tcb->registro_de_programacion[registro]--;

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
				self->tcb->puntero_instruccion = self->tcb->registro_de_programacion[registro];
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

}


//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

//int MALC_ESO (t_TCB_CPU* tcb){
//	//CREAR_SEGMENTO
//	char *data=malloc(sizeof(int)+sizeof(int32_t)); //pid+(tamanio)registro_de_programacion['A']
//	t_paquete_MSP *alocar_bytes = malloc(sizeof(t_paquete_MSP));
//
//	int soffset=0, stmp_size=0;
//	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
//	soffset=stmp_size;
//	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(int32_t));
//	soffset+=stmp_size;
//
//	alocar_bytes->tamanio = soffset;
//	alocar_bytes->data = data;
//
//	if (socket_sendPaquete(self->socketMSP->socket, CREAR_SEGMENTO, alocar_bytes->tamanio, alocar_bytes->data)<=0){
//		log_info(self->loggerCPU, "CPU: Error de memoria llena en MSP\n %d", tcb->pid);
//	}
//	t_socket_paquete *paquete_MSP = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
//	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_MSP) > 0){
//		if(paquete_MSP->header.type == CREAR_SEGMENTO){
//			log_info(self->loggerCPU, "CPU: Recibiendo direccion virtual...\n %d ", tcb->pid);
//			char *contenido = malloc(sizeof(uint32_t));
//			memcpy(contenido, paquete_MSP->data, sizeof(char)*4);
//			//resguardo la direccion virtual en el registro A
//			tcb->registro_de_programacion[0]=*contenido;
//			free(paquete_MSP);
//			free(contenido);
//		} else {
//			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP:\n %d", paquete_MSP->header.type);
//		}
//	}else{
//		log_info(self->loggerCPU, "CPU: MSP ha cerrado su conexion");
//		printf("MSP ha cerrado su conexion\n");
//
//	}
//	free(alocar_bytes);
//	free(data);
//}
//
//
//int FREE_ESO(t_TCB_CPU* tcb){
//
//	char *data=malloc(sizeof(int)+sizeof(uint32_t)); //pid+tamaño segun registro
//	t_paquete_MSP *leer_byte = malloc(sizeof(t_paquete_MSP));
//
//	int soffset=0, stmp_size=0;
//	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
//	soffset=stmp_size;
//	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(uint32_t));
//	soffset+=stmp_size;
//
//	leer_byte->tamanio = soffset;
//	leer_byte->data = data;
//
//	if (socket_sendPaquete(self->socketMSP->socket, DESTRUIR_SEGMENTO, leer_byte->tamanio, leer_byte->data)<=0){
//		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", tcb->pid);
//
//	}
//	free(data);
//	free(leer_byte);
//
//}
//
//int INNN_ESO(t_TCB_CPU* tcb){
//
//
//	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR,sizeof(int), &(tcb->pid))<=0){
//		log_info(self->loggerCPU, "CPU: Error de ENTRADA_ESTANDAR\n %d", tcb->pid);
//	}
//
//	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
//	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
//		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR){
//			log_info(self->loggerCPU, "CPU: Recibiendo numero ingresado por consola\n %d", tcb->pid);
//			tcb->registro_de_programacion[0]=(int)paquete_KERNEL;
//		} else {
//			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de MSP:\n %d", paquete_KERNEL->header.type);
//
//		}
//	}else{
//		log_info(self->loggerCPU, "CPU: MSP ha cerrado su conexion\n");
//		printf("CPU: MSP ha cerrado su conexion\n");
//
//	}
//
//	free(paquete_KERNEL);
//
//}
//
//int INNC_ESO(t_TCB_CPU* tcb){
//	int estado_innc;
//	t_entrada_estandar* pedir_cadena = malloc(sizeof(t_entrada_estandar));
//
//	pedir_cadena->pid = tcb->pid;
//	pedir_cadena->tamanio = tcb->registro_de_programacion[1];
//	pedir_cadena->tipo  = 2; //2 es un string
//
//	if (socket_sendPaquete(self->socketPlanificador->socket, ENTRADA_ESTANDAR,sizeof(t_entrada_estandar), pedir_cadena)<=0){  //22 corresponde a interrupcion
//		log_info(self->loggerCPU, "CPU: Error de ENTRADA_ESTANDAR_CHAR\n %d", tcb->pid);
//	}
//	free(pedir_cadena);
//
//	t_socket_paquete *paquete_KERNEL = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
//	if(socket_recvPaquete(self->socketPlanificador->socket, paquete_KERNEL) > 0){
//		if(paquete_KERNEL->header.type == ENTRADA_ESTANDAR){
//			log_info(self->loggerCPU, "CPU: Recibiendo CADENA ingresado por consola ", tcb->pid);
//			char *cadena = malloc(tcb->registro_de_programacion[1]);
//			t_datos_aMSP *escritura_msp = malloc(sizeof(t_datos_aMSP));
//			memcpy(cadena, paquete_KERNEL->data, tcb->registro_de_programacion[1]);
//			int estadoEscritura = cpuEscribirMemoria(self, tcb->registro_de_programacion[0], cadena, sizeof(tcb->registro_de_programacion[1]));
//
//			if(estadoEscritura == ERROR_POR_SEGMENTATION_FAULT){
//				//no uso directamente el return porque sino nunca libero el paquete_KERNEL y devuelvo un estado que puede variar
//				estado_innc = ERROR_POR_SEGMENTATION_FAULT;
//			} else {
//				free(cadena);
//				estado_innc = SIN_ERRORES;
//			}
//		}else{
//			log_info(self->loggerCPU, "CPU: codigo inesperado de MSP");
//			printf("CPU: codigo inesperado de MSP\n");
//			estado_innc = MENSAJE_DE_ERROR;
//		}
//		free(paquete_KERNEL);
//	}
//	return estado_innc;
//}
//
//
//int OUTN_ESO(t_TCB_CPU* tcb){
//
//	t_paquete_MSP *mostrar_numero = malloc(sizeof(t_paquete_MSP));
//
//	char *data=malloc(sizeof(int)+sizeof(int32_t)); //pid+(tamanio)registro_de_programacion['B']
//	int soffset=0, stmp_size=0;
//	memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
//	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(int32_t));
//	soffset+=stmp_size;
//	mostrar_numero->tamanio=soffset;
//	mostrar_numero->data=data;
//
//	if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR,mostrar_numero->tamanio, mostrar_numero->data)<=0){
//		log_info(self->loggerCPU, "CPU: Error de SALIDA_ESTANDAR\n %d", tcb->pid);
//
//
//	}
//	free(data);
//	free(mostrar_numero);
//
//}
//int OUTC_ESO(t_TCB_CPU* tcb){
//
//	t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
//	unCPU_LEER_MEMORIA->pid = self->tcb->pid;
//	unCPU_LEER_MEMORIA->tamanio = tcb->registro_de_programacion[1];
//	unCPU_LEER_MEMORIA->direccionVirtual = tcb->registro_de_programacion[0];
//	if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_CPU_LEER_MEMORIA), unCPU_LEER_MEMORIA)<=0){
//		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
//	}
//
//	t_socket_paquete *cadena = malloc(sizeof(t_socket_paquete));
//
//	if(socket_recvPaquete(self->socketMSP->socket, cadena) > 0){
//		if(cadena->header.type == LEER_MEMORIA){
//			char *mostrar_cadena=malloc(tcb->registro_de_programacion[1]);
//			memcpy(mostrar_cadena, cadena->data, tcb->registro_de_programacion[1]);
//
//			char *data=malloc(sizeof(int)+tcb->registro_de_programacion[1]); //pid+(tamanio)registro_de_programacion['B']
//			int soffset=0, stmp_size=0;
//			memcpy(data, &(tcb->pid), stmp_size=(sizeof(int)));
//			memcpy(data + soffset, mostrar_cadena, stmp_size=tcb->registro_de_programacion[1]);
//			soffset+=stmp_size;
//
//			if (socket_sendPaquete(self->socketPlanificador->socket, SALIDA_ESTANDAR ,soffset, data)<=0){
//				log_info(self->loggerCPU, "CPU: Error de SALIDA_ESTANDAR_CHAR\n %d", tcb->pid);
//
//			}
//			free(data);
//			free(mostrar_cadena);
//		}
//	}
//
//
//}
//int CREA_ESO(t_TCB_CPU* tcb){ 	// CREA un hilo hijo de TCB
//	t_TCB_CPU* tcb_hijo=malloc(sizeof(t_TCB_CPU));
//	tcb_hijo->pid=tcb->registro_de_programacion[0];
//	tcb_hijo->tid=(tcb->tid)+1;
//	tcb_hijo->km=0;				//se pasa a modo usuario
//	tcb_hijo->puntero_instruccion=tcb->registro_de_programacion[1];
//
//	//creando stack del tcb hijo...
//	// tengo que pedirle al kernel que me cree el segmento de stack y luego recibir la base, despues duplicar el contenido del stack del padre y actualizar el cursor (cursor-base del padre)
//
//	//CPU->KERNEL : pidiendo al Kernel que cree el STACK del TCB hijo
//	char *pedir_stack=malloc(sizeof(int)); //pid+(tamanio)registro_de_programacion['A']
//	memcpy(pedir_stack, &(tcb_hijo->pid), sizeof(int));
//	log_info(self->loggerCPU, "CPU: solicitando a Kernel creacion de stack PID:\n %s", tcb_hijo->pid);
//	if (socket_sendPaquete(self->socketPlanificador->socket, //CREAR_STACK, sizeof(int), pedir_stack)<=0){
//		log_info(self->loggerCPU, "CPU: Error al pedir stack\n %d", tcb->pid);
//	}
//	t_socket_paquete *base_stack = (t_socket_paquete *) malloc(sizeof(t_socket_paquete));
//	if(socket_recvPaquete(self->socketPlanificador->socket, base_stack) > 0){
//		if(base_stack->header.type == CREAR_STACK){
//			log_info(self->loggerCPU, "CPU: Recibiendo base de stack...\n %d ", tcb->pid);
//			tcb_hijo->base_stack = (int32_t)(base_stack->data);
//			free(base_stack);
//		} else {
//			log_error(self->loggerCPU, "CPU: Se recibio un codigo inesperado de Kernel:\n %d", base_stack->header.type);
//		}
//	}else{
//		log_info(self->loggerCPU, "CPU: Kernel ha cerrado su conexion");
//		printf("Kernel ha cerrado su conexion\n");
//
//	}
//	free(pedir_stack);
//
//	// fin crear stack del tcb hijo...
//
//	// inicializando Stack...
//	//primero: leer el STACK completo del padre
//
//	t_CPU_LEER_MEMORIA* unCPU_LEER_MEMORIA = malloc(sizeof(t_CPU_LEER_MEMORIA));
//	unCPU_LEER_MEMORIA->pid = self->tcb->pid;
//	unCPU_LEER_MEMORIA->tamanio = 1024;
//	unCPU_LEER_MEMORIA->direccionVirtual = tcb->base_stack;
//	if (socket_sendPaquete(self->socketMSP->socket, LEER_MEMORIA, sizeof(t_CPU_LEER_MEMORIA), unCPU_LEER_MEMORIA)<=0){
//		log_info(self->loggerCPU, "CPU: Error en envio de direccion a la MSP %d", self->tcb->pid);
//	}
//
//	t_socket_paquete *datos_memoria = malloc(sizeof(t_socket_paquete));
//
//	if(socket_recvPaquete(self->socketMSP->socket, datos_memoria) > 0){
//		if(datos_memoria->header.type == LEER_MEMORIA){
//			char *stack_tcb = malloc(1024);
//			memcpy(stack_tcb, datos_memoria->data, 1024);
//			//segundo: copiar el contenido del stack recibido en el stack del tcb_hijo: ESCRIBIR_MEMORIA
//			char *data=malloc(sizeof(int)+sizeof(uint32_t)+ tcb->registro_de_programacion[1]);  //pid+direccion_logica+datos_a_grabar
//			t_paquete_MSP *grabar_byte = malloc(sizeof(t_paquete_MSP));
//			int soffset=0, stmp_size=0;
//			memcpy(grabar_byte, &(tcb_hijo->pid), stmp_size=(sizeof(int)));
//			soffset=stmp_size;
//			memcpy(grabar_byte + soffset, &(tcb_hijo->base_stack), stmp_size=sizeof(uint32_t));
//			soffset+=stmp_size;
//			memcpy(grabar_byte + soffset, stack_tcb, stmp_size=1024);
//			soffset+=stmp_size;
//
//			grabar_byte->tamanio = soffset;
//			grabar_byte->data = data;
//
//			if (socket_sendPaquete(self->socketMSP->socket, ESCRIBIR_MEMORIA, grabar_byte->tamanio, grabar_byte->data)<=0){
//				log_info(self->loggerCPU, "CPU: Error de escritura en MSP\n %d", tcb->pid);
//			}
//
//		}
//	}
//
//	//tercero: inicializar el cursor de stack del tcb_hijo
//	uint32_t avance_stack = tcb->cursor_stack - tcb->base_stack;
//	tcb_hijo->cursor_stack = avance_stack;
//
//	// fin inicializar Stack...
//
//	// enviando a Kernel el TCB a planificar..
//
//	log_info(self->loggerCPU, "CPU: solicitando creacion de TCB HIJO...\n %d", tcb_hijo->pid);
//	if (socket_sendPaquete(self->socketPlanificador->socket, CREAR_HILO, sizeof(t_TCB_CPU) , tcb_hijo)<=0){
//		log_info(self->loggerCPU, "CPU: Error de CREAR_HILO_HIJO\n %d", tcb->pid);
//
//	}
//	free(tcb_hijo);
//
//}
//int JOIN_ESO(t_TCB_CPU* tcb){
//	t_paquete_MSP *envio_join = malloc(sizeof(t_paquete_MSP));
//	char *data=malloc((sizeof(int))+sizeof(int32_t)); //pid+tid llamador+(tid a esperar)registro_de_programacion['B']
//	int soffset=0, stmp_size=0;
//	memcpy(data, &(tcb->tid), stmp_size=(sizeof(int)));
//	memcpy(data + soffset, &(tcb->registro_de_programacion[0]), stmp_size=sizeof(int32_t));
//	soffset+=stmp_size;
//	envio_join->tamanio=soffset;
//	envio_join->data=data;
//
//	if (socket_sendPaquete(self->socketPlanificador->socket, //JOIN_HILO,envio_join->tamanio, envio_join->data)<=0){
//		log_info(self->loggerCPU, "CPU: Error de JOIN\n %d", tcb->pid);
//
//
//	}
//	free(data);
//	free(envio_join);
//
//}
//
//int BLOK_ESO(t_TCB_CPU* tcb){
//
//	t_paquete_MSP *envio_bytes = malloc(sizeof(t_paquete_MSP));
//
//	char *data=malloc(sizeof(int)+sizeof(uint32_t)); //pid+(tamanio)registro_de_programacion['B']
//	int soffset=0, stmp_size=0;
//	memcpy(data, tcb, stmp_size=(sizeof(t_TCB_CPU)));
//	memcpy(data + soffset, &(tcb->registro_de_programacion[1]), stmp_size=sizeof(int32_t));
//	soffset+=stmp_size;
//	envio_bytes->tamanio=soffset;
//	envio_bytes->data=data;
//
//	if (socket_sendPaquete(self->socketPlanificador->socket, //BLOK_HILO,envio_bytes->tamanio, envio_bytes->data)<=0){
//		log_info(self->loggerCPU, "CPU: Error de BLOQUEO\n %d", tcb->pid);
//	}
//	free(data);
//	free(envio_bytes);
//
//
//}
//
//int WAKE_ESO(t_TCB_CPU* tcb){
//
//	if (socket_sendPaquete(self->socketPlanificador->socket, //WAKE_HILO,sizeof(int32_t), &(tcb->registro_de_programacion[1]))<=0){
//		log_info(self->loggerCPU, "CPU: Error de DESPERTAR\n %d", tcb->pid);
//
//
//	}
//}
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
