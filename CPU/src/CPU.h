#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include "commons/socketInBigBang.h"
#include "commons/protocolStructInBigBang.h"
#include "codigoESO.h"
#include "commons/log.h"
#include "commons/collections/list.h"
#include "commons/panel.h"
#include "commons/cpu.h"
#include <unistd.h>

t_log *logger;
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

//Codigos ESO

#define LOAD 1001
#define GETM 1002
#define SETM 1003
#define MOVR 1004
#define ADDR 1005
#define SUBR 1006
#define MULR 1007
#define MODR 1008
#define DIVR 1009
#define INCR 1010
#define DECR 1011
#define COMP 1012
#define CGEQ 1013
#define CLEQ 1014
#define GOTO 1015
#define JMPZ 1016
#define JPNZ 1017
#define INTE 1018
#define SHIF 1019
#define NOPP 1020
#define PUSH 1021
#define TAKE 1022
#define XXXX 1023
#define MALC 1024
#define FREE 1025
#define INNN 1026
#define INNC 1027
#define OUTN 1028
#define OUTC 1029
#define CREA 1030
#define JOIN 1031
#define BLOK 1032
#define WAKE 1033
#define CANTIDAD_INSTRUCCIONES 33

/*
 * Estucturas para usar en CPU!
 * Estas son las primeras que tiene que cambiar y decidir si van o no van en el tp
 */

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
t_TCB_CPU* tcb;
t_list* lista;
t_socket* socketDelKernel;
t_socket* socketDelMSP;

/*
 * Aqui se declaran las funciones
 */

void verificar_argumentosCPU(int argc, char* argv[]);
void ejecutar_instruccion(int linea, t_CPU* self);
int cpuProcesar_tcb(t_CPU* self);
//void cpuProcesar_tcb(int pid, t_TCB_CPU* nuevo);
void cambioContexto(t_CPU* self);
int determinar_registro(char registro);


//Instrucciones de usuario


void LOAD_ESO (int registro, int32_t numero, t_TCB_CPU* tcb); //Carga en el registro, el número dado.

void GETM_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb);	//Obtiene el valor de memoria apuntado por el segundo registro.
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

void INNN_ESO (t_TCB_CPU* tcb);

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
