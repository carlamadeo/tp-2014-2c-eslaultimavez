#include "codigoESO.h"
#include "ejecucion.h"
#include "CPU_Proceso.h"
//#include "commons/cpu.h"
#include "commons/string.h"
#include "cpuMSP.h"
#include "cpuKernel.h"

char *instrucciones[] = {"LOAD", "GETM", "SETM", "MOVR", "ADDR", "SUBR", "MULR", "MODR", "DIVR", "INCR", "DECR",
		"COMP", "CGEQ", "CLEQ", "GOTO", "JMPZ", "JPNZ", "INTE", "SHIF", "NOPP", "PUSH", "TAKE", "XXXX", "MALC", "FREE", "INNN",
		"INNC", "OUTN", "OUTC", "CREA", "JOIN", "BLOK", "WAKE"};


int LOAD_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

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

		reg = determinar_registro(registro);
	}

	if(reg != -1){

		if(reg <= 5)
			self->tcb->registro_de_programacion[reg] = numero;

		else{
			switch(reg){
			case 6: self->tcb->base_segmento_codigo = (uint32_t) numero; break;
			case 7: self->tcb->puntero_instruccion = (uint32_t) numero; break;
			case 8: self->tcb->base_stack = (uint32_t) numero; break;
			case 9: self->tcb->cursor_stack = (uint32_t) numero; break;
			}
		}

		imprimirNumeroYRegistro(registro, numero, "LOAD");
		cpuInicializarRegistrosCPU(self, registros_cpu);
		cambio_registros(registros_cpu);

		log_info(self->loggerCPU, "CPU: LOAD ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	}

	else{
		log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
		estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
	}

	free(lecturaDeMSP);
	free(registros_cpu);
	return estado_bloque;
}


int GETM_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_bloque = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);

	if (estado_bloque == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion GETM");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		free(lecturaDeMSP);

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "GETM");

			int tamanioMSP = sizeof(char);
			char *lecturaMSP = malloc(sizeof(char));
			char leido;


			if(regB <= 5){
				estado_bloque = cpuLeerMemoria(self, (uint32_t)self->tcb->registro_de_programacion[regB], lecturaMSP, tamanioMSP);
			}
			else{
				switch(regB){
				case 6:estado_bloque = cpuLeerMemoria(self, self->tcb->base_segmento_codigo, lecturaMSP, tamanioMSP);break;
				case 7:estado_bloque = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaMSP, tamanioMSP);break;
				case 8:estado_bloque = cpuLeerMemoria(self, self->tcb->base_stack, lecturaMSP, tamanioMSP);break;
				case 9:estado_bloque = cpuLeerMemoria(self, self->tcb->cursor_stack, lecturaMSP, tamanioMSP);break;
				}
			}

			if(estado_bloque == SIN_ERRORES){
				//David: en que momento se asigna el valor obtenido en el primer registro (regA)??? para mi falta:
				if(regA <= 5)
					self->tcb->registro_de_programacion[regA] = (int32_t)lecturaMSP[0];

				else{
					switch(regA){
					case 6:self->tcb->base_segmento_codigo = (uint32_t)lecturaMSP[0];break;
					case 7:self->tcb->puntero_instruccion = (uint32_t)lecturaMSP[0];break;
					case 8:self->tcb->base_stack = (uint32_t)lecturaMSP[0];break;
					case 9:self->tcb->cursor_stack = (uint32_t)lecturaMSP[0];break;
					}
				}

				cpuInicializarRegistrosCPU(self, registros_cpu);
				cambio_registros(registros_cpu);

				free(lecturaMSP);
				log_info(self->loggerCPU, "CPU: GETM ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			}

			else
				log_error(self->loggerCPU, "CPU: Ha ocurrido un error al leer la memoria para el PID %d", self->tcb->pid);

		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}
	}

	free(registros_cpu);
	return estado_bloque;
}


int SETM_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

	int tamanio = 6;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int numero, regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;
		cpuInicializarRegistrosCPU(self, registros_cpu);
		cambio_registros(registros_cpu);
		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion SETM");

		memcpy(&(numero), lecturaDeMSP, sizeof(int));
		memcpy(&(registroA), lecturaDeMSP + sizeof(int), sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(int) + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) && (regB != -1)){

			if(numero <= sizeof(uint32_t)){

				imprimirDosRegistrosUnNumero(registroA, registroB, numero, "SETM");

				char* byte_a_escribir = malloc(sizeof(int32_t));
				if(regB <= 5)
					memcpy(byte_a_escribir, &(self->tcb->registro_de_programacion[regB]), numero);

				else{
					switch(regB){
					case 6:memcpy(byte_a_escribir, &(self->tcb->base_segmento_codigo), numero);break;
					case 7:memcpy(byte_a_escribir, &(self->tcb->puntero_instruccion), numero);break;
					case 8:memcpy(byte_a_escribir, &(self->tcb->base_stack), numero);break;
					case 9:memcpy(byte_a_escribir, &(self->tcb->cursor_stack), numero);break;
					}
				}

				if(regA <= 5)
					estado_bloque = cpuEscribirMemoria(self, (uint32_t)self->tcb->registro_de_programacion[regA], byte_a_escribir, numero);

				else{
					switch(regA){
					case 6:estado_bloque = cpuEscribirMemoria(self, self->tcb->base_segmento_codigo, byte_a_escribir, numero);break;
					case 7:estado_bloque = cpuEscribirMemoria(self, self->tcb->puntero_instruccion, byte_a_escribir, numero);break;
					case 8:estado_bloque = cpuEscribirMemoria(self, self->tcb->base_stack, byte_a_escribir, numero);break;
					case 9:estado_bloque = cpuEscribirMemoria(self, self->tcb->cursor_stack, byte_a_escribir, numero);break;
					}
				}

				log_info(self->loggerCPU, "CPU: SETM ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

				free(byte_a_escribir);

			}

		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;
}


int MOVR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "MOVR");

			if(regA <= 5){
				if(regB <= 5)
					self->tcb->registro_de_programacion[regA] = self->tcb->registro_de_programacion[regB];

				else{
					switch(regB){
					case 6:self->tcb->registro_de_programacion[regA] = self->tcb->base_segmento_codigo;break;
					case 7:self->tcb->registro_de_programacion[regA] = self->tcb->puntero_instruccion;break;
					case 8:self->tcb->registro_de_programacion[regA] = self->tcb->base_stack;break;
					case 9:self->tcb->registro_de_programacion[regA] = self->tcb->cursor_stack;break;
					}
				}
			}

			else{
				if(regB <= 5){
					switch(regA){
					case 6:self->tcb->base_segmento_codigo = (uint32_t)self->tcb->registro_de_programacion[regB];break;
					case 7:self->tcb->puntero_instruccion = (uint32_t)self->tcb->registro_de_programacion[regB];break;
					case 8:self->tcb->base_stack = (uint32_t)self->tcb->registro_de_programacion[regB];break;
					case 9:self->tcb->cursor_stack = (uint32_t)self->tcb->registro_de_programacion[regB];break;
					}
				}

				else{
					switch(regB){
					case 6:
						switch(regA){
						case 7:self->tcb->puntero_instruccion = self->tcb->base_segmento_codigo ;break;
						case 8:self->tcb->base_stack = self->tcb->base_segmento_codigo ;break;
						case 9:self->tcb->cursor_stack = self->tcb->base_segmento_codigo ;break;
						}
						break;

						case 7:
							switch(regA){
							case 6:self->tcb->base_segmento_codigo = self->tcb->puntero_instruccion;break;
							case 8:self->tcb->base_stack =self->tcb->puntero_instruccion ;break;
							case 9:self->tcb->cursor_stack =self->tcb->puntero_instruccion;break;
							}
							break;

							case 8:
								switch(regA){
								case 6:self->tcb->base_segmento_codigo = self->tcb->base_stack;break;
								case 7:self->tcb->puntero_instruccion = self->tcb->base_stack;break;
								case 9:self->tcb->cursor_stack =self->tcb->base_stack;break;
								}

								break;
								case 9:
									switch(regA){
									case 6:self->tcb->base_segmento_codigo = self->tcb->cursor_stack;break;
									case 7:self->tcb->puntero_instruccion = self->tcb->cursor_stack;break;
									case 8:self->tcb->base_stack = self->tcb->cursor_stack;break;
									}

									break;
					}
				}
			}

			log_info(self->loggerCPU, "CPU: MOVR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}
	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;
}


int ADDR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion ADDR");

		memcpy(&(registroA), (lecturaDeMSP), 1);
		memcpy(&(registroB), (lecturaDeMSP) + 1, 1);
		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "ADDR");
			int suma;
			if(regA <= 5){
				if(regB <= 5)
					suma = self->tcb->registro_de_programacion[regA] + self->tcb->registro_de_programacion[regB];

				else{
					switch(regB){
					case 6:suma = self->tcb->registro_de_programacion[regA] + self->tcb->base_segmento_codigo;break;
					case 7:suma = self->tcb->registro_de_programacion[regA] + self->tcb->puntero_instruccion;break;
					case 8:suma = self->tcb->registro_de_programacion[regA] + self->tcb->base_stack;break;
					case 9:suma = self->tcb->registro_de_programacion[regA] + self->tcb->cursor_stack;break;
					}
				}
			}
			else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:suma = self->tcb->base_segmento_codigo + self->tcb->base_segmento_codigo;break;
					case 7:suma = self->tcb->puntero_instruccion + self->tcb->base_segmento_codigo ;break;
					case 8:suma = self->tcb->base_stack + self->tcb->base_segmento_codigo ;break;
					case 9:suma = self->tcb->cursor_stack + self->tcb->base_segmento_codigo ;break;
					}
					break;
					case 7:
						switch(regA){
						case 6:suma = self->tcb->base_segmento_codigo + self->tcb->puntero_instruccion;break;
						case 7:suma = self->tcb->puntero_instruccion + self->tcb->puntero_instruccion; break;
						case 8:suma = self->tcb->base_stack + self->tcb->puntero_instruccion ;break;
						case 9:suma = self->tcb->cursor_stack + self->tcb->puntero_instruccion;break;
						}
						break;
						case 8:
							switch(regA){
							case 6:suma = self->tcb->base_segmento_codigo + self->tcb->base_stack;break;
							case 7:suma = self->tcb->puntero_instruccion + self->tcb->base_stack;break;
							case 8:suma = self->tcb->base_stack + self->tcb->base_stack;break;
							case 9:suma = self->tcb->cursor_stack + self->tcb->base_stack;break;
							}
							break;
							case 9:
								switch(regA){
								case 6:suma = self->tcb->base_segmento_codigo + self->tcb->cursor_stack;break;
								case 7:suma = self->tcb->puntero_instruccion + self->tcb->cursor_stack;break;
								case 8:suma = self->tcb->base_stack + self->tcb->cursor_stack;break;
								case 9:suma = self->tcb->cursor_stack + self->tcb->cursor_stack;break;
								}
								break;

				}

			}

			self->tcb->registro_de_programacion[0] = (int32_t)suma;
			log_info(self->loggerCPU, "CPU: ADDR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;
}


int SUBR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "SUBR");

			int resta;
			if(regA <= 5){
				if(regB <= 5)
					resta = self->tcb->registro_de_programacion[regA] - self->tcb->registro_de_programacion[regB];

				else{
					switch(regB){
					case 6:resta = self->tcb->registro_de_programacion[regA] - self->tcb->base_segmento_codigo;break;
					case 7:resta = self->tcb->registro_de_programacion[regA] - self->tcb->puntero_instruccion;break;
					case 8:resta = self->tcb->registro_de_programacion[regA] - self->tcb->base_stack;break;
					case 9:resta = self->tcb->registro_de_programacion[regA] - self->tcb->cursor_stack;break;
					}
				}
			}

			else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:resta = self->tcb->base_segmento_codigo - self->tcb->base_segmento_codigo;break;
					case 7:resta = self->tcb->puntero_instruccion - self->tcb->base_segmento_codigo ;break;
					case 8:resta = self->tcb->base_stack - self->tcb->base_segmento_codigo ;break;
					case 9:resta = self->tcb->cursor_stack - self->tcb->base_segmento_codigo ;break;
					}
					break;
					case 7:
						switch(regA){
						case 6:resta = self->tcb->base_segmento_codigo - self->tcb->puntero_instruccion;break;
						case 7:resta = self->tcb->puntero_instruccion - self->tcb->puntero_instruccion; break;
						case 8:resta = self->tcb->base_stack - self->tcb->puntero_instruccion ;break;
						case 9:resta = self->tcb->cursor_stack - self->tcb->puntero_instruccion;break;
						}
						break;
						case 8:
							switch(regA){
							case 6:resta = self->tcb->base_segmento_codigo - self->tcb->base_stack;break;
							case 7:resta = self->tcb->puntero_instruccion - self->tcb->base_stack;break;
							case 8:resta = self->tcb->base_stack - self->tcb->base_stack;break;
							case 9:resta = self->tcb->cursor_stack - self->tcb->base_stack;break;
							}
							break;
							case 9:
								switch(regA){
								case 6:resta = self->tcb->base_segmento_codigo - self->tcb->cursor_stack;break;
								case 7:resta = self->tcb->puntero_instruccion - self->tcb->cursor_stack;break;
								case 8:resta = self->tcb->base_stack - self->tcb->cursor_stack;break;
								case 9:resta = self->tcb->cursor_stack - self->tcb->cursor_stack;break;
								}
								break;

				}

			}
			self->tcb->registro_de_programacion[0] = (int32_t)resta;

			log_info(self->loggerCPU, "CPU: SUBR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}

int MULR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "MULR");
			int mult;
			if(regA <= 5){

				if(regB <= 5)
					mult = self->tcb->registro_de_programacion[regA] * self->tcb->registro_de_programacion[regB];

				else{
					switch(regB){
					case 6:mult = self->tcb->registro_de_programacion[regA] * self->tcb->base_segmento_codigo;break;
					case 7:mult = self->tcb->registro_de_programacion[regA] * self->tcb->puntero_instruccion;break;
					case 8:mult = self->tcb->registro_de_programacion[regA] * self->tcb->base_stack;break;
					case 9:mult = self->tcb->registro_de_programacion[regA] * self->tcb->cursor_stack;break;
					}
				}
			}

			else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:mult = self->tcb->base_segmento_codigo * self->tcb->base_segmento_codigo;break;
					case 7:mult = self->tcb->puntero_instruccion * self->tcb->base_segmento_codigo ;break;
					case 8:mult = self->tcb->base_stack * self->tcb->base_segmento_codigo ;break;
					case 9:mult = self->tcb->cursor_stack * self->tcb->base_segmento_codigo ;break;
					}
					break;
					case 7:
						switch(regA){
						case 6:mult = self->tcb->base_segmento_codigo * self->tcb->puntero_instruccion;break;
						case 7:mult = self->tcb->puntero_instruccion * self->tcb->puntero_instruccion; break;
						case 8:mult = self->tcb->base_stack * self->tcb->puntero_instruccion ;break;
						case 9:mult = self->tcb->cursor_stack * self->tcb->puntero_instruccion;break;
						}
						break;
						case 8:
							switch(regA){
							case 6:mult = self->tcb->base_segmento_codigo * self->tcb->base_stack;break;
							case 7:mult = self->tcb->puntero_instruccion * self->tcb->base_stack;break;
							case 8:mult = self->tcb->base_stack * self->tcb->base_stack;break;
							case 9:mult = self->tcb->cursor_stack * self->tcb->base_stack;break;
							}
							break;
							case 9:
								switch(regA){
								case 6:mult = self->tcb->base_segmento_codigo * self->tcb->cursor_stack;break;
								case 7:mult = self->tcb->puntero_instruccion * self->tcb->cursor_stack;break;
								case 8:mult = self->tcb->base_stack * self->tcb->cursor_stack;break;
								case 9:mult = self->tcb->cursor_stack * self->tcb->cursor_stack;break;
								}
								break;

				}

			}
			self->tcb->registro_de_programacion[0] = (int32_t)mult;
			log_info(self->loggerCPU, "CPU: MULR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int MODR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "MODR");
			int modulo;
			if(regA <= 5){
				if(regB <= 5)
					modulo = self->tcb->registro_de_programacion[regA] %  self->tcb->registro_de_programacion[regB];

				else{
					switch(regB){
					case 6:modulo = self->tcb->registro_de_programacion[regA] % self->tcb->base_segmento_codigo;break;
					case 7:modulo = self->tcb->registro_de_programacion[regA] % self->tcb->puntero_instruccion;break;
					case 8:modulo = self->tcb->registro_de_programacion[regA] % self->tcb->base_stack;break;
					case 9:modulo = self->tcb->registro_de_programacion[regA] % self->tcb->cursor_stack;break;
					}
				}
			}

			else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:modulo = self->tcb->base_segmento_codigo % self->tcb->base_segmento_codigo;break;
					case 7:modulo = self->tcb->puntero_instruccion % self->tcb->base_segmento_codigo ;break;
					case 8:modulo = self->tcb->base_stack % self->tcb->base_segmento_codigo ;break;
					case 9:modulo = self->tcb->cursor_stack % self->tcb->base_segmento_codigo ;break;
					}
					break;
					case 7:
						switch(regA){
						case 6:modulo = self->tcb->base_segmento_codigo % self->tcb->puntero_instruccion;break;
						case 7:modulo = self->tcb->puntero_instruccion % self->tcb->puntero_instruccion; break;
						case 8:modulo = self->tcb->base_stack % self->tcb->puntero_instruccion ;break;
						case 9:modulo = self->tcb->cursor_stack % self->tcb->puntero_instruccion;break;
						}
						break;
						case 8:
							switch(regA){
							case 6:modulo = self->tcb->base_segmento_codigo % self->tcb->base_stack;break;
							case 7:modulo = self->tcb->puntero_instruccion % self->tcb->base_stack;break;
							case 8:modulo = self->tcb->base_stack % self->tcb->base_stack;break;
							case 9:modulo = self->tcb->cursor_stack % self->tcb->base_stack;break;
							}
							break;
							case 9:
								switch(regA){
								case 6:modulo = self->tcb->base_segmento_codigo % self->tcb->cursor_stack;break;
								case 7:modulo = self->tcb->puntero_instruccion % self->tcb->cursor_stack;break;
								case 8:modulo = self->tcb->base_stack % self->tcb->cursor_stack;break;
								case 9:modulo = self->tcb->cursor_stack % self->tcb->cursor_stack;break;
								}
								break;

				}

			}
			self->tcb->registro_de_programacion[0] = (int32_t)modulo;

			log_info(self->loggerCPU, "CPU: MODR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int DIVR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "DIVR");

			//int32_t auxiliar = self->tcb->registro_de_programacion[regB];
			int divis;

			if(regB <= 5){
				if (self->tcb->registro_de_programacion[regB] != 0){
					if(regA <= 5)
						divis = self->tcb->registro_de_programacion[regA] /  self->tcb->registro_de_programacion[regB];

					else{
						switch(regA){
						case 6:divis = self->tcb->base_segmento_codigo /  self->tcb->registro_de_programacion[regB];break;
						case 7:divis = self->tcb->puntero_instruccion /  self->tcb->registro_de_programacion[regB] ;break;
						case 8:divis = self->tcb->base_stack /  self->tcb->registro_de_programacion[regB];break;
						case 9:divis = self->tcb->cursor_stack  /  self->tcb->registro_de_programacion[regB];break;
						}
					}
				}

				else{
					log_error(self->loggerCPU, "CPU: error de intento de division por cero");
					estado_bloque = ERROR_POR_EJECUCION_ILICITA; //TODO No se que poner, no podemos poner un error por cada cosa, ver como solucionarlo!!
				}
			}

			else{
				switch(regB){
				case 6:
					if(self->tcb->base_segmento_codigo != 0){
						switch(regA){
						case 6:divis = self->tcb->base_segmento_codigo / self->tcb->base_segmento_codigo;break;
						case 7:divis = self->tcb->puntero_instruccion / self->tcb->base_segmento_codigo ;break;
						case 8:divis = self->tcb->base_stack / self->tcb->base_segmento_codigo ;break;
						case 9:divis = self->tcb->cursor_stack / self->tcb->base_segmento_codigo ;break;
						}
					}

					else{
						log_error(self->loggerCPU, "CPU: error de intento de division por cero");
						estado_bloque = ERROR_POR_EJECUCION_ILICITA; //TODO No se que poner, no podemos poner un error por cada cosa, ver como solucionarlo!!
					}
					break;
				case 7:
					if(self->tcb->puntero_instruccion != 0){
						switch(regA){
						case 6:divis = self->tcb->base_segmento_codigo / self->tcb->puntero_instruccion;break;
						case 7:divis = self->tcb->puntero_instruccion / self->tcb->puntero_instruccion; break;
						case 8:divis = self->tcb->base_stack / self->tcb->puntero_instruccion ;break;
						case 9:divis = self->tcb->cursor_stack / self->tcb->puntero_instruccion;break;
						}
					}

					else{
						log_error(self->loggerCPU, "CPU: error de intento de division por cero");
						estado_bloque = ERROR_POR_EJECUCION_ILICITA; //TODO No se que poner, no podemos poner un error por cada cosa, ver como solucionarlo!!
					}
					break;
				case 8:
					if(self->tcb->base_stack != 0){
						switch(regA){
						case 6:divis = self->tcb->base_segmento_codigo / self->tcb->base_stack;break;
						case 7:divis = self->tcb->puntero_instruccion / self->tcb->base_stack;break;
						case 8:divis = self->tcb->base_stack / self->tcb->base_stack;break;
						case 9:divis = self->tcb->cursor_stack / self->tcb->base_stack;break;
						}
					}

					else{
						log_error(self->loggerCPU, "CPU: error de intento de division por cero");
						estado_bloque = ERROR_POR_EJECUCION_ILICITA; //TODO No se que poner, no podemos poner un error por cada cosa, ver como solucionarlo!!
					}
					break;
				case 9:
					if(self->tcb->cursor_stack != 0){
						switch(regA){
						case 6:divis = self->tcb->base_segmento_codigo / self->tcb->cursor_stack;break;
						case 7:divis = self->tcb->puntero_instruccion / self->tcb->cursor_stack;break;
						case 8:divis = self->tcb->base_stack / self->tcb->cursor_stack;break;
						case 9:divis = self->tcb->cursor_stack / self->tcb->cursor_stack;break;
						}
					}

					else{
						log_error(self->loggerCPU, "CPU: error de intento de division por cero");
						estado_bloque = ERROR_POR_EJECUCION_ILICITA; //TODO No se que poner, no podemos poner un error por cada cosa, ver como solucionarlo!!
					}
					break;
				}


			}

			if(estado_bloque != ERROR_POR_EJECUCION_ILICITA)
				self->tcb->registro_de_programacion[0] = (int32_t)divis;


			log_info(self->loggerCPU, "CPU: DIVR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);

		}else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int INCR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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
			if(reg <= 5){
				self->tcb->registro_de_programacion[reg]++;
			}

			else{
				switch(reg){
				case 6:
					self->tcb->base_segmento_codigo++;
					break;
				case 7:
					self->tcb->puntero_instruccion++;
					break;
				case 8:
					self->tcb->base_stack++;
					break;
				case 9:
					self->tcb->cursor_stack++;
					break;
				}
			}

			imprimirUnRegistro(registro, "INCR");
			log_info(self->loggerCPU, "CPU: INCR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int DECR_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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
			if(reg <= 5){
				self->tcb->registro_de_programacion[reg]--;
			}else{
				switch(reg){
				case 6: self->tcb->base_segmento_codigo--; break;
				case 7: self->tcb->puntero_instruccion--; break;
				case 8: self->tcb->base_stack--; break;
				case 9: self->tcb->cursor_stack--; break;
				}
			}

			imprimirUnRegistro(registro, "DECR");
			log_info(self->loggerCPU, "CPU: DECR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int COMP_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

	int tamanio = 2;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registroA, registroB;
	int regA, regB;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){
		log_info(self->loggerCPU, "Recibiendo parametros de instruccion COMP");

		memcpy(&(registroA), lecturaDeMSP, sizeof(char));
		memcpy(&(registroB), lecturaDeMSP + sizeof(char), sizeof(char));

		regA = determinar_registro(registroA);
		regB = determinar_registro(registroB);

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "COMP");
			if(regA <= 5){
				if(regB <= 5){
					if (self->tcb->registro_de_programacion[regA] == self->tcb->registro_de_programacion[regB])
						self->tcb->registro_de_programacion[0] = 1;

					else
						self->tcb->registro_de_programacion[0] = 0;
				}else{
					switch(regB){
					case 6:
						if (self->tcb->registro_de_programacion[regA] == self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 7:
						if (self->tcb->registro_de_programacion[regA] == self->tcb->puntero_instruccion)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 8:
						if (self->tcb->registro_de_programacion[regA] == self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 9:
						if (self->tcb->registro_de_programacion[regA] == self->tcb->cursor_stack)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					}
				}
			}else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:
						if (self->tcb->base_segmento_codigo == self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 7:
						if (self->tcb->puntero_instruccion == self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 8:
						if (self->tcb->base_stack == self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 9:
						if (self->tcb->cursor_stack == self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					}
					break;
					case 7:
						switch(regA){
						case 6:
							if (self->tcb->base_segmento_codigo == self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 7:
							if (self->tcb->puntero_instruccion == self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 8:
							if (self->tcb->base_stack == self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 9:
							if (self->tcb->cursor_stack == self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						}
						break;
						case 8:
							switch(regA){
							case 6:
								if (self->tcb->base_segmento_codigo == self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 7:
								if (self->tcb->puntero_instruccion == self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 8:
								if (self->tcb->base_stack == self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 9:
								if (self->tcb->cursor_stack == self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							}
							break;
							case 9:
								switch(regA){
								case 6:
									if (self->tcb->base_segmento_codigo == self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 7:
									if (self->tcb->puntero_instruccion == self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 8:
									if (self->tcb->base_stack == self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 9:
									if (self->tcb->cursor_stack == self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								}
								break;

				}

			}

			log_info(self->loggerCPU, "CPU: COMPR ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int CGEQ_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "CGEQ");

			if(regA <= 5){
				if(regB <= 5){
					if (self->tcb->registro_de_programacion[regA] >= self->tcb->registro_de_programacion[regB])
						self->tcb->registro_de_programacion[0] = 1;

					else
						self->tcb->registro_de_programacion[0] = 0;
				}else{
					switch(regB){
					case 6:
						if (self->tcb->registro_de_programacion[regA]>=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 7:
						if (self->tcb->registro_de_programacion[regA]>= self->tcb->puntero_instruccion)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 8:
						if (self->tcb->registro_de_programacion[regA] >=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 9:
						if (self->tcb->registro_de_programacion[regA] >=  self->tcb->cursor_stack)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					}
				}
			}else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:
						if (self->tcb->base_segmento_codigo>= self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 7:
						if (self->tcb->puntero_instruccion>= self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 8:
						if (self->tcb->base_stack >=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 9:
						if (self->tcb->cursor_stack >=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					}
					break;
					case 7:
						switch(regA){
						case 6:
							if (self->tcb->base_segmento_codigo >=  self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 7:
							if (self->tcb->puntero_instruccion >= self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 8:
							if (self->tcb->base_stack >=  self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 9:
							if (self->tcb->cursor_stack >=  self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						}
						break;
						case 8:
							switch(regA){
							case 6:
								if (self->tcb->base_segmento_codigo >=  self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 7:
								if (self->tcb->puntero_instruccion >=  self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 8:
								if (self->tcb->base_stack>=  self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 9:
								if (self->tcb->cursor_stack >=  self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							}
							break;
							case 9:
								switch(regA){
								case 6:
									if (self->tcb->base_segmento_codigo >=  self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 7:
									if (self->tcb->puntero_instruccion >=  self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 8:
									if (self->tcb->base_stack>= self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 9:
									if (self->tcb->cursor_stack >=  self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								}
								break;

				}

			}


			log_info(self->loggerCPU, "CPU: CGEQ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int CLEQ_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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

		if((regA != -1) && (regB != -1)){

			imprimirDosRegistros(registroA, registroB, "CLEQ");


			if(regA <= 5){
				if(regB <= 5){
					if (self->tcb->registro_de_programacion[regA] <= self->tcb->registro_de_programacion[regB])
						self->tcb->registro_de_programacion[0] = 1;

					else
						self->tcb->registro_de_programacion[0] = 0;
				}else{
					switch(regB){
					case 6:
						if (self->tcb->registro_de_programacion[regA]<=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 7:
						if (self->tcb->registro_de_programacion[regA]<= self->tcb->puntero_instruccion)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 8:
						if (self->tcb->registro_de_programacion[regA] <=   self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 9:
						if (self->tcb->registro_de_programacion[regA] <=   self->tcb->cursor_stack)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					}
				}
			}else{
				switch(regB){
				case 6:
					switch(regA){
					case 6:
						if (self->tcb->base_segmento_codigo<= self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 7:
						if (self->tcb->puntero_instruccion<=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 8:
						if (self->tcb->base_stack <=  self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					case 9:
						if (self->tcb->cursor_stack <=   self->tcb->base_segmento_codigo)
							self->tcb->registro_de_programacion[0] = 1;
						else
							self->tcb->registro_de_programacion[0] = 0;
						break;
					}
					break;
					case 7:
						switch(regA){
						case 6:
							if (self->tcb->base_segmento_codigo <=   self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 7:
							if (self->tcb->puntero_instruccion <=  self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 8:
							if (self->tcb->base_stack <=   self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						case 9:
							if (self->tcb->cursor_stack <=  self->tcb->puntero_instruccion)
								self->tcb->registro_de_programacion[0] = 1;
							else
								self->tcb->registro_de_programacion[0] = 0;
							break;
						}
						break;
						case 8:
							switch(regA){
							case 6:
								if (self->tcb->base_segmento_codigo <=   self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 7:
								if (self->tcb->puntero_instruccion <=   self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 8:
								if (self->tcb->base_stack<=   self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							case 9:
								if (self->tcb->cursor_stack <=   self->tcb->base_stack)
									self->tcb->registro_de_programacion[0] = 1;
								else
									self->tcb->registro_de_programacion[0] = 0;
								break;
							}
							break;
							case 9:
								switch(regA){
								case 6:
									if (self->tcb->base_segmento_codigo <=   self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 7:
									if (self->tcb->puntero_instruccion <=  self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 8:
									if (self->tcb->base_stack<= self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								case 9:
									if (self->tcb->cursor_stack <=   self->tcb->cursor_stack)
										self->tcb->registro_de_programacion[0] = 1;
									else
										self->tcb->registro_de_programacion[0] = 0;
									break;
								}
								break;

				}

			}


			log_info(self->loggerCPU, "CPU: CLEQ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

		else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int GOTO_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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

			imprimirUnRegistro(registro, "GOTO");
			if(reg <= 5){
				if((self->tcb->base_segmento_codigo + self->tcb->registro_de_programacion[reg]) <= self->tcb->tamanio_segmento_codigo){
					self->tcb->puntero_instruccion = self->tcb->registro_de_programacion[reg];
				}else{
					estado_bloque = ERROR_POR_SEGMENTATION_FAULT;
				}
			}else{
				switch(reg){
				case 6:
					if((self->tcb->base_segmento_codigo + self->tcb->base_segmento_codigo) <= self->tcb->tamanio_segmento_codigo){
						self->tcb->puntero_instruccion = self->tcb->base_segmento_codigo;
					}else{
						estado_bloque = ERROR_POR_SEGMENTATION_FAULT;
					}break;
				case 7:
					if((self->tcb->base_segmento_codigo + self->tcb->puntero_instruccion) <= self->tcb->tamanio_segmento_codigo){
						//no hace nada, el salto es al puntero actual
					}else{
						estado_bloque = ERROR_POR_SEGMENTATION_FAULT;
					}break;
				case 8:
					if((self->tcb->base_segmento_codigo + self->tcb->base_stack) <= self->tcb->tamanio_segmento_codigo){
						self->tcb->puntero_instruccion = self->tcb->base_stack;
					}else{
						estado_bloque = ERROR_POR_SEGMENTATION_FAULT;
					}
					break;
				case 9:
					if((self->tcb->base_segmento_codigo + self->tcb->cursor_stack) <= self->tcb->tamanio_segmento_codigo){
						self->tcb->puntero_instruccion = self->tcb->cursor_stack;
					}else{
						estado_bloque = ERROR_POR_SEGMENTATION_FAULT;
					}break;
				}
			}
			log_info(self->loggerCPU, "CPU: GOTO ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);

		}else{
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
			estado_bloque = ERROR_REGISTRO_DESCONOCIDO;
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}

int JMPZ_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
	int tamanio = 4;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	uint32_t direccion;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JMPZ");

		memcpy(&(direccion), lecturaDeMSP, sizeof(uint32_t));

		imprimirUnNumero((int)direccion, "JMPZ");

		if(self->tcb->registro_de_programacion[0] == 0){

			if((self->tcb->base_segmento_codigo + direccion) <= self->tcb->tamanio_segmento_codigo){
				self->tcb->puntero_instruccion = direccion;
				log_info(self->loggerCPU, "CPU: JMPZ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				cpuInicializarRegistrosCPU(self, registros_cpu);
				cambio_registros(registros_cpu);
			}

			else
				estado_bloque = ERROR_POR_SEGMENTATION_FAULT;

		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int JPNZ_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
	int tamanio = 4;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	uint32_t direccion;

	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion JPNZ");

		memcpy(&(direccion), lecturaDeMSP, sizeof(uint32_t));

		imprimirUnNumero((int)direccion, "JPNZ");

		if(self->tcb->registro_de_programacion[0] != 0){

			if((self->tcb->base_segmento_codigo + direccion) <= self->tcb->tamanio_segmento_codigo){
				self->tcb->puntero_instruccion = direccion;
				log_info(self->loggerCPU, "CPU: JPNZ ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				cpuInicializarRegistrosCPU(self, registros_cpu);
				cambio_registros(registros_cpu);
			}

			else
				estado_bloque = ERROR_POR_SEGMENTATION_FAULT;

		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int INTE_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
	int tamanio = 4;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	uint32_t direccion;
	t_ServiciosAlPlanificador* serviciosAlPlanificador = malloc(sizeof(t_ServiciosAlPlanificador));


	int estado_lectura = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);
	int estado_bloque = estado_lectura;

	if (estado_lectura == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion INTE");

		memcpy(&(direccion), lecturaDeMSP, sizeof(uint32_t));

		imprimirUnNumero(direccion, "INTE");

		cpuInicializarRegistrosCPU(self, registros_cpu);
		cambio_registros(registros_cpu);

		self->unaDireccion = direccion;

		cpuEnviaInterrupcion(self);

		log_info(self->loggerCPU, "CPU: INTE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return INTERRUPCION;

	//tengo que tomar esa direccion y pasarcela al Kernel con el TCB

}

//void FLCL(t_CPU *self){
//
//}

int SHIF_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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
			if(reg <= 5){
				if(numero > 0)
					self->tcb->registro_de_programacion[reg]>>=numero;
				else
					self->tcb->registro_de_programacion[reg]<<=numero;
			}else{
				switch(reg){
				case 6:
					if(numero > 0)
						self->tcb->base_segmento_codigo>>=numero;
					else
						self->tcb->base_segmento_codigo<<=numero;
					break;
				case 7:
					if(numero > 0)
						self->tcb->puntero_instruccion>>=numero;
					else
						self->tcb->puntero_instruccion<<=numero;
					break;
				case 8:
					if(numero > 0)
						self->tcb->base_stack>>=numero;
					else
						self->tcb->base_stack<<=numero;
					break;
				case 9: self->tcb->cursor_stack++; break;
				}
			}

			imprimirNumeroYRegistro(registro, numero, "SHIF");

			log_info(self->loggerCPU, "CPU: SHIF ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
		}

	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;

}


int NOPP_ESO(t_CPU *self){
	//no hace nada

	log_info(self->loggerCPU, "CPU: NOPP ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	return SIN_ERRORES;
}


int PUSH_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
	int tamanio = 5;
	char *lecturaDeMSP = malloc(sizeof(char)*tamanio + 1);
	char registro;
	int numero, reg;

	int estado_bloque = cpuLeerMemoria(self, self->tcb->puntero_instruccion, lecturaDeMSP, tamanio);

	if (estado_bloque == SIN_ERRORES){

		self->tcb->puntero_instruccion += tamanio;

		log_info(self->loggerCPU, "CPU: Recibiendo parametros de instruccion PUSH");

		memcpy(&(numero), lecturaDeMSP, sizeof(int32_t));
		memcpy(&(registro), lecturaDeMSP + sizeof(int32_t), sizeof(char));

		reg = determinar_registro(registro);

		if((reg != -1)){

			imprimirNumeroYRegistro(registro, numero, "PUSH");

			if(numero <= sizeof(uint32_t)){

				char* byte_a_escribir = malloc(sizeof(int32_t));
				if(reg <= 5){
					memcpy(byte_a_escribir, &(self->tcb->registro_de_programacion[reg]), numero);
				}else{
					switch(reg){
					case 6:
						memcpy(byte_a_escribir, &(self->tcb->base_segmento_codigo), numero);
						break;
					case 7:
						memcpy(byte_a_escribir, &(self->tcb->puntero_instruccion), numero);
						break;
					case 8:
						memcpy(byte_a_escribir, &(self->tcb->base_stack), numero);
						break;
					case 9:
						memcpy(byte_a_escribir, &(self->tcb->cursor_stack), numero);
						break;
					}
				}

				estado_bloque = cpuEscribirMemoria(self, (uint32_t)self->tcb->cursor_stack, byte_a_escribir, numero);

				free(byte_a_escribir);


				if (estado_bloque == SIN_ERRORES){
					self->tcb->cursor_stack += numero;
					log_info(self->loggerCPU, "CPU: PUSH ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
					cpuInicializarRegistrosCPU(self, registros_cpu);
					cambio_registros(registros_cpu);
				}
			}
		}

		else
			return ERROR_REGISTRO_DESCONOCIDO;
	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;
}


int TAKE_ESO(t_CPU *self){

	t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
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

			imprimirNumeroYRegistro(registro, numero, "TAKE");

			if(numero <= sizeof(uint32_t)){

				char *lecturaDeMSP2 = malloc(sizeof(char) * numero + 1);

				//se hace el control para saber a donde apuntar dependiendo de si se trata un tcb usuario o kernel...
				int estado_lectura = cpuLeerMemoria(self, self->tcb->cursor_stack, lecturaDeMSP2, numero);
				estado_bloque = estado_lectura;

				if (estado_lectura == SIN_ERRORES){
					if(reg <= 5){
						self->tcb->registro_de_programacion[reg] = (int32_t)lecturaDeMSP2;
					}else{
						switch(reg){
						case 6: self->tcb->base_segmento_codigo = (int32_t)lecturaDeMSP2; break;
						case 7: self->tcb->puntero_instruccion = (int32_t)lecturaDeMSP2; break;
						case 8: self->tcb->base_stack = (int32_t)lecturaDeMSP2; break;
						case 9: self->tcb->cursor_stack = (int32_t)lecturaDeMSP2; break;
						}
					}
					self->tcb->cursor_stack -= numero;
					log_info(self->loggerCPU, "CPU: TAKE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
					cpuInicializarRegistrosCPU(self, registros_cpu);
					cambio_registros(registros_cpu);
				}

				free(lecturaDeMSP2);
			}
		}

		else
			log_error(self->loggerCPU, "CPU: Error registro de programacion no encontrado %d", self->tcb->pid);
	}

	free(registros_cpu);
	free(lecturaDeMSP);
	return estado_bloque;
}


int XXXX_ESO(t_CPU *self){

	fin_ejecucion();

	//char *data = malloc(sizeof(t_TCB_CPU_Kernel));
	//memcpy(data, self->tcb, sizeof(t_TCB_CPU_Kernel));
	//int estado = cpuFinalizarProgramaExitoso(self, data);
	ejecucion_instruccion("XXXX", parametros);

	log_info(self->loggerCPU, "CPU: XXXX ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
	//free(data);
	return FINALIZAR_PROGRAMA_EXITO;

}


//Instrucciones Protegidas, KM=1   (ninguna de estas operaciones tiene operadores)

int MALC_ESO(t_CPU *self){

	int estado_malc;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion MALC");

		t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));

		estado_malc = cpuCrearSegmento(self, self->tcb->pid, self->tcb->registro_de_programacion[0]);

		switch(estado_malc){

		case ERROR_POR_TAMANIO_EXCEDIDO:
			log_info(self->loggerCPU, "CPU: MALC ERROR_POR_TAMANIO_EXCEDIDO  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			return ERROR_POR_TAMANIO_EXCEDIDO;

		case ERROR_POR_MEMORIA_LLENA:
			log_info(self->loggerCPU, "CPU: MALC ERROR_POR_MEMORIA_LLENA  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			return ERROR_POR_MEMORIA_LLENA;

		case ERROR_POR_NUMERO_NEGATIVO:
			log_info(self->loggerCPU, "CPU: MALC ERROR_POR_NUMERO_NEGATIVO  para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			return ERROR_POR_NUMERO_NEGATIVO;

		default:
			self->tcb->registro_de_programacion[0] = estado_malc;
			log_info(self->loggerCPU, "CPU: MALC ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);
			return SIN_ERRORES;
		}

		ejecucion_instruccion("MALC", parametros);
		free(registros_cpu);
	}

	else
		estado_malc = ERROR_POR_EJECUCION_ILICITA;

	return estado_malc;
}


int FREE_ESO(t_CPU *self){

	int estado_free;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion FREE");
		int direccionVirtual = self->tcb->registro_de_programacion[0];

		estado_free = cpuDestruirSegmento(self, direccionVirtual);

		if(estado_free == ERROR_POR_SEGMENTO_DESCONOCIDO)
			log_info(self->loggerCPU, "CPU: FREE ejecutado con ERROR_POR_SEGMENTO_DESCONOCIDO para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		else{
			log_info(self->loggerCPU, "CPU: FREE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			ejecucion_instruccion("FREE", parametros);
		}
	}

	else
		estado_free = ERROR_POR_EJECUCION_ILICITA;


	return estado_free;
}


void printfEntradaStandarCPU(t_entrada_estandar* entrada){
	printf("entrada pid: %d\n",entrada->pid);
	printf("entrada tamanio: %d\n",entrada->tamanio);
	printf("entrada tipo: %d\n",entrada->tipo);

}


int INNN_ESO(t_CPU *self){

	int estado_innn;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion INNN");

		t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
		int *intRecibido = malloc(sizeof(int));

		estado_innn = cpuSolicitarEntradaEstandar(self, sizeof(int), ENTRADA_ESTANDAR_INT);

		if(estado_innn == SIN_ERRORES){
			estado_innn = reciboEntradaEstandarINT(self, intRecibido);

			self->tcb->registro_de_programacion[0] = (int32_t)intRecibido;

			cpuInicializarRegistrosCPU(self, registros_cpu);
			cambio_registros(registros_cpu);

			if(estado_innn == SIN_ERRORES){
				log_info(self->loggerCPU, "CPU: INNN ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				ejecucion_instruccion("INNN", parametros);
			}

			else
				log_error(self->loggerCPU, "CPU: Ha ocurrido un error al ejecutar la instruccion INNN para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		}

		free(intRecibido);
		free(registros_cpu);
	}

	else
		estado_innn = ERROR_POR_EJECUCION_ILICITA;


	return estado_innn;

}


int INNC_ESO(t_CPU *self){

	int estado_innc;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion INNC");

		t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu));
		char *charRecibido = malloc(sizeof(self->tcb->registro_de_programacion[1]));

		estado_innc = cpuSolicitarEntradaEstandar(self, self->tcb->registro_de_programacion[1], ENTRADA_ESTANDAR_TEXT);

		if(estado_innc == SIN_ERRORES){

			estado_innc = reciboEntradaEstandarCHAR(self, charRecibido, self->tcb->registro_de_programacion[1]);

			if(estado_innc == SIN_ERRORES){
				log_info(self->loggerCPU, "INNC_ESO: Recibe una cadena de la Consola: %s", charRecibido);

				int estadoEscritura = cpuEscribirMemoria(self, self->tcb->registro_de_programacion[0], charRecibido, self->tcb->registro_de_programacion[1]);

				if(estadoEscritura == ERROR_POR_SEGMENTATION_FAULT){
					log_info(self->loggerCPU, "CPU: INNC ejecutado con ERROR_POR_SEGMENTATION_FAULT para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
					estado_innc = ERROR_POR_SEGMENTATION_FAULT;
				}

				else {
					log_info(self->loggerCPU, "CPU: INNC ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
					ejecucion_instruccion("INNC", parametros);
				}
			}
		}

		free(registros_cpu);
		free(charRecibido);
	}

	else
		estado_innc = ERROR_POR_EJECUCION_ILICITA;


	return estado_innc;
}


int OUTN_ESO(t_CPU *self){

	int estado_outn;
	int tamanio;
	char *enviarAKernelChar = malloc(sizeof(int32_t));

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion OUTN");
		sprintf(enviarAKernelChar, "%d", self->tcb->registro_de_programacion[0]);
		estado_outn = cpuEnviarSalidaEstandar(self, enviarAKernelChar, sizeof(int32_t));

		if(estado_outn == SIN_ERRORES)
			ejecucion_instruccion("OUTN", parametros);
	}

	else
		estado_outn = ERROR_POR_EJECUCION_ILICITA;

	return estado_outn;
}


int OUTC_ESO(t_CPU* self){

	int estado_outc;
	int tamanio;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion OUTC");

		char* lecturaDeMSP = malloc(self->tcb->registro_de_programacion[1]);

		//TODO Esto lo cambie porque dice que se lee lo que apunta el registro A no el cursor_de_stack, esta bien??
		int estado_lectura = cpuLeerMemoriaSinKM(self, self->tcb->registro_de_programacion[0], lecturaDeMSP, self->tcb->registro_de_programacion[1]);

		if (estado_lectura == ERROR_POR_SEGMENTATION_FAULT)
			estado_outc = ERROR_POR_SEGMENTATION_FAULT;

		else{
			tamanio = strlen(lecturaDeMSP);
			estado_outc = cpuEnviarSalidaEstandar(self, lecturaDeMSP, tamanio);

			if(estado_outc == SIN_ERRORES){
				log_info(self->loggerCPU, "CPU: OUTC ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
				ejecucion_instruccion("OUTC", parametros);
			}
		}

		free(lecturaDeMSP);
	}

	else
		estado_outc = ERROR_POR_EJECUCION_ILICITA;


	return estado_outc;
}

//TODO Ver como es esta funcion
int CREA_ESO(t_CPU *self){ 	// CREA un hilo hijo de TCB

	int estado_crea = SIN_ERRORES;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion CREA");
		t_crea_hilo* crear_hilo = malloc(sizeof(t_crea_hilo));
		crear_hilo->pid = self->tcb->pid;
		crear_hilo->puntero_instruccion = self->tcb->registro_de_programacion[1];
		crear_hilo->tid = self->tcb->tid + 1;
		crear_hilo->km = 0;
		crear_hilo->base_segmento_codigo = self->tcb->base_segmento_codigo;
		crear_hilo->tamanio_segmento_codigo = self->tcb->tamanio_segmento_codigo;
		crear_hilo->base_stack = self->tcb->base_stack;
		crear_hilo->cursor_stack = self->tcb->cursor_stack;
		crear_hilo->registro_de_programacion[0] = self->tcb->registro_de_programacion[0];
		crear_hilo->registro_de_programacion[1] = self->tcb->registro_de_programacion[1];
		crear_hilo->registro_de_programacion[2] = self->tcb->registro_de_programacion[2];
		crear_hilo->registro_de_programacion[3] = self->tcb->registro_de_programacion[3];
		crear_hilo->registro_de_programacion[4] = self->tcb->registro_de_programacion[4];

		if (socket_sendPaquete(self->socketPlanificador->socket, CREAR_HILO, sizeof(t_crea_hilo), crear_hilo) <= 0){
			log_info(self->loggerCPU, "CPU: CREA ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			estado_crea = MENSAJE_DE_ERROR;
		}

		log_info(self->loggerCPU, "CPU: CREA ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
		ejecucion_instruccion("CREA", parametros);

		free(crear_hilo);
	}

	else
		estado_crea = ERROR_POR_EJECUCION_ILICITA;


	return estado_crea;
}


int JOIN_ESO(t_CPU *self){

	int estado_join = SIN_ERRORES;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion JOIN");
		t_join* joinear = malloc(sizeof(t_join));
		joinear->pid = self->tcb->pid;
		joinear->tid_llamador = self->tcb->tid;
		joinear->tid_esperar = self->tcb->registro_de_programacion[0];

		if (socket_sendPaquete(self->socketPlanificador->socket, JOIN_HILO, sizeof(t_join), joinear) <= 0){
			log_info(self->loggerCPU, "CPU: JOIN ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			estado_join = MENSAJE_DE_ERROR;
		}

		log_info(self->loggerCPU, "CPU: JOIN ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		ejecucion_instruccion("JOIN", parametros);
		free(joinear);
	}

	else
		estado_join = ERROR_POR_EJECUCION_ILICITA;


	return estado_join;

}


int BLOK_ESO(t_CPU *self){

	int estado_blok = SIN_ERRORES;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion BLOK");
		t_bloquear* blocker = malloc(sizeof(t_bloquear));
		blocker->pid = self->tcb->pid;
		blocker->tid = self->tcb->tid;
		blocker->km = self->tcb->km;
		blocker->id_recurso = self->tcb->registro_de_programacion[1];

		if (socket_sendPaquete(self->socketPlanificador->socket, BLOK_HILO, sizeof(t_bloquear), blocker) <= 0){
			log_info(self->loggerCPU, "CPU: BLOK ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			estado_blok = MENSAJE_DE_ERROR;
		}


		else
			log_info(self->loggerCPU, "CPU: BLOK ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		ejecucion_instruccion("BLOK", parametros);
		free(blocker);
	}

	else
		estado_blok = ERROR_POR_EJECUCION_ILICITA;

	return estado_blok;

}

int WAKE_ESO(t_CPU *self){

	int estado_wake = SIN_ERRORES;

	if(self->tcb->km == 1){

		log_info(self->loggerCPU, "CPU: Ejecutando instruccion WAKE");
		t_despertar* despertar = malloc(sizeof(t_despertar));
		despertar->id_recurso = self->tcb->registro_de_programacion[1];

		if (socket_sendPaquete(self->socketPlanificador->socket, WAKE_HILO ,sizeof(t_despertar), despertar) <= 0){
			log_info(self->loggerCPU, "CPU: WAKE ejecutado con error para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);
			estado_wake = MENSAJE_DE_ERROR;
		}

		else
			log_info(self->loggerCPU, "CPU: WAKE ejecutado con exito para PID: %d TID: %d", self->tcb->pid, self->tcb->tid);

		ejecucion_instruccion("WAKE", parametros);
		free(despertar);
	}

	else
		estado_wake = ERROR_POR_EJECUCION_ILICITA;

	return estado_wake;
}

//Funciones para la impresion de los logs de la catedra

void imprimirUnNumero(uint32_t numero, char* funcion){

	char *numeroEnString = malloc(1);
	sprintf(numeroEnString, "%0.8p", numero);
	list_add(parametros, numeroEnString);
	ejecucion_instruccion(funcion, parametros);
	free(numeroEnString);
}


void imprimirNumeroYRegistro(char registro, int numero, char* funcion){

	char *registroEnString = malloc(1);
	sprintf(registroEnString, "%c", registro);
	char *numeroEnString = malloc(1);
	sprintf(numeroEnString, "%d", numero);

	list_add(parametros, registroEnString);
	list_add(parametros, numeroEnString);

	ejecucion_instruccion(funcion, parametros);

	free(registroEnString);
	free(numeroEnString);
}


void imprimirDosRegistrosUnNumero(char registroA, char registroB, int numero, char* funcion){

	char *registroAEnString = malloc(1);
	sprintf(registroAEnString, "%c", registroA);
	char *registroBEnString = malloc(1);
	sprintf(registroBEnString, "%c", registroB);
	char *numeroEnString = malloc(1);
	sprintf(numeroEnString, "%d", numero);

	list_add(parametros, registroAEnString);
	list_add(parametros, registroBEnString);
	list_add(parametros, numeroEnString);

	ejecucion_instruccion(funcion, parametros);

	free(registroAEnString);
	free(registroBEnString);
	free(numeroEnString);
}


void imprimirDosRegistros(char registroA, char registroB, char* funcion){

	char *registroAEnString = malloc(1);
	sprintf(registroAEnString, "%c", registroA);
	char *registroBEnString = malloc(1);
	sprintf(registroBEnString, "%c", registroB);

	list_add(parametros, registroAEnString);
	list_add(parametros, registroBEnString);

	ejecucion_instruccion(funcion, parametros);

	free(registroAEnString);
	free(registroBEnString);

}


void imprimirUnRegistro(char registro, char* funcion){

	char *registroEnString = malloc(1);
	sprintf(registroEnString, "%c", registro);
	list_add(parametros, registroEnString);

	ejecucion_instruccion(funcion, parametros);

	free(registroEnString);
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
