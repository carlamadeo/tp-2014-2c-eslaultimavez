#ifndef CPUPROCESO_H_
#define CPUPROCESO_H_

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
} t_registros_cpu_nuestro;
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
	int pid;
	int tid;
}t_CPU_TERMINE_UNA_LINEA;


t_hilo_log* hilo_log;


//FUNCIONES
void verificar_argumentosCPU(int argc, char* argv[]);


#endif /* CPU_H_ */
