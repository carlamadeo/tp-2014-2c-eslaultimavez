#ifndef CPU_H_
#define CPU_H_

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "commons/socketInBigBang.h"
#include "commons/protocolStructInBigBang.h"
#include "commons/collections/list.h"
#include "commons/log.h"
#include "commons/panel.h"
#include "commons/cpu.h"
#include "commons/config.h"
#include "codigoESO.h"

//t_log *logger;
//linea para hacer push
//#include <commons/threadInBigBang.h>


typedef struct {
	int pid;
	int tid;
	short km;
	uint32_t base_segmento_codigo;
	int tamanio_segmento_codigo;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registro_de_programacion[5];
} t_TCB_CPU;

typedef struct{
	int quantumCPU;
}t_quantumCPU;
//Codigos ESO

#define LOAD 0
#define GETM 1
#define SETM 2
#define MOVR 3
#define ADDR 4
#define SUBR 5
#define MULR 6
#define MODR 7
#define DIVR 8
#define INCR 9
#define DECR 10
#define COMP 11
#define CGEQ 12
#define CLEQ 13
#define GOTO 14
#define JMPZ 15
#define JPNZ 16
#define INTE 17
#define SHIF 18
#define NOPP 19
#define PUSH 20
#define TAKE 21
#define XXXX 22
#define MALC 23
#define FREE 24
#define INNN 25
#define INNC 26
#define OUTN 27
#define OUTC 28
#define CREA 29
#define JOIN 30
#define BLOK 31
#define WAKE 32
#define CANTIDAD_INSTRUCCIONES 33

/*
 * Estucturas para usar en CPU!
 * Estas son las primeras que tiene que cambiar y decidir si van o no van en el tp

 //Soy jorge comento esto porque ya esta definido en cpu.c
typedef struct {
	int32_t registros_programacion[5]; //A, B, C, D y E
	uint32_t M; //Base de segmento de código
	uint32_t P; //Puntero de instrucción
	uint32_t X; //Base del segmento de Stack
	uint32_t S; //Cursor de stack
	uint32_t K; //Kernel Mode
	uint32_t I; //PID
} t_registros_cpu;*/


typedef struct {
	t_TCB_CPU* tcb;
	t_socket_client* socketPlanificador;
	t_socket_client* socketMSP;
	t_log* loggerCPU;
	int puertoPlanificador;
	int puertoMSP;
	char* ipPlanificador;
	char* ipMsp;
	int quantum;
	int retardo;
}t_CPU;


typedef struct{
	int tamanio;
	char *data;
}t_paquete_MSP;

typedef struct{
	int estado;
	char data[1000];
}t_lectura_MSP;

typedef struct{
	int pid;
	uint32_t direccionVirtual;
	int tamanio;
}t_CPU_LEER_MEMORIA;


typedef struct{
	int pid;
	int tid;
}t_CPU_TERMINE_UNA_LINEA;



/*
 * Fin de las estucturas para comunicar CPU con Kernel
 */

t_CPU* self;
//t_registros_cpu* registros_cpu;
t_hilo* hilo_log;

/*
 * FUNCIONES:
 */

void verificar_argumentosCPU(int argc, char* argv[]);
int ejecutar_instruccion(int linea, t_CPU* self);
int cpuProcesar_tcb(t_CPU* self);
//void cpuProcesar_tcb(int pid, t_TCB_CPU* nuevo);
void cambioContexto(t_CPU* self);
int determinar_registro(char registro);


//Instrucciones de usuario


void LOAD_ESO (int registro, int32_t numero, t_CPU* self); //Carga en el registro, el número dado.

int GETM_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb);	//Obtiene el valor de memoria apuntado por el segundo registro.
//El valor obtenido lo asigna en el primer registro.

void SETM_ESO (int numero, int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Pone tantos bytes desde el segundo registro,
//hacia la memoria apuntada por el primer registro.


void MOVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Copia el valor del segundo registro hacia el primero.

void ADDR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Suma el primer registro con el segundo registro.
//El resultado de la operación se almacena en el registro A.


void SUBR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Resta el primer registro con el segundo registro.
//El resultado de la operación se almacena en el registro A.

void MULR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb);//Multiplica el primer registro con el segundo registro.
//El resultado de la operación se almacena en el registro A.

void MODR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Obtiene el resto de la división del primer registro con el segundo registro.
//El resultado de la operación se almacena en el registro A.

void DIVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Divide el primer registro con el segundo registro.
//El resultado de la operación se almacena en el
//registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV
//y no se hace la operación.

void INCR_ESO (int registro, t_TCB_CPU* tcb); //Incrementar una unidad al registro.
void DECR_ESO (int registro, t_TCB_CPU* tcb); //Decrementa una unidad al registro.


void COMP_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Compara si el primer registro es igual al segundo.
//De ser verdadero, se almacena el valor 1.
//De lo contrario el valor 0.
//El resultado de la operación se almacena en el registro A.
void CGEQ_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Compara si el primer registro es mayor o igual al segundo.
//De ser verdadero, se almacena el valor 1. De lo contrario el valor 0.
//El resultado de la operación se almacena en el registro A.


void CLEQ_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Compara si el primer registro es menor o igual al segundo.
//De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.
//El resultado de la operación se almacena en el registro A.

void GOTO_ESO (int registro, t_TCB_CPU* tcb); //Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro.
//El valor es el desplazamiento desde el inicio del programa.

void JMPZ_ESO(int direccion, t_TCB_CPU* tcb);   //Altera el flujo de ejecución, solo si el valor del registro A es cero,
//para ejecutar la instrucción apuntada por el registro.
//El valor es el desplazamiento desde el inicio del programa.

void JPNZ_ESO(int direccion, t_TCB_CPU* tcb); //Altera el flujo de ejecución, solo si el valor del registro A no es cero,
//para ejecutar la instrucción apuntada por el registro.
//El valor es el desplazamiento desde el inicio del programa.

void INTE_ESO(uint32_t direccion, t_TCB_CPU* tcb); 	//Interrumpe la ejecución del programa para ejecutar la rutina del kernel
//que se encuentra en la posición apuntada por la direccion.
//El ensamblador admite ingresar una cadena indicando el nombre,
//que luego transformará en el número correspondiente. Los posibles valores son:
//“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”.
//Invoca al servicio correspondiente en el proceso Kernel.
//Notar que el hilo en cuestión debe bloquearse tras una interrupción.

/*void FLCL(); //Limpia el registro de flags.*/

int INNN_ESO (t_TCB_CPU* tcb);

void SHIF_ESO (int numero, int registro, t_TCB_CPU* tcb); //Desplaza los bits del registro, tantas veces como se indique en el Número.
//De ser desplazamiento positivo, se considera hacia la derecha.
//De lo contrario hacia la izquierda.

void NOPP_ESO (); //Consume un ciclo del CPU sin hacer nada.

void PUSH_ESO (int numero, int registro, t_TCB_CPU* tcb); //Apila los primeros bytes, indicado por el número, del registro hacia el stack.
//Modifica el valor del registro cursor de stack de forma acorde.

void TAKE_ESO (int numero, int registro, t_TCB_CPU* tcb); //Desapila los primeros bytes, indicado por el número, del stack hacia el registro.
//Modifica el valor del registro de stack de forma acorde.

void XXXX_ESO (t_TCB_CPU* tcb); //Finaliza la ejecución.

//fin de instrucciones de usuario

#endif /* CPU_H_ */
