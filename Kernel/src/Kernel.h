#ifndef KERNEL_H_
#define KERNEL_H_

#include <semaphore.h>
#include <arpa/inet.h>
#include <commons/collections/list.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/socketInBigBang.h"
#include "commons/protocolStructInBigBang.h"

/*----------------------Estructuras del Kernel----------------------------------------*/
#define MAXDATASIZE 1024
#define PATH_CONFIG "archivoConfiguracion.cfg"
#define PATH_LOG "logs/trace.log"


typedef struct {
	int pid;
	int tid;
	short km;
	uint32_t base_segmento_codigo;
	int tamanio_segmento_codigo;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registro_de_programacion[4];
} t_TCB_Kernel;

typedef struct {
	int quamtum;
} t_QUAMTUM;


typedef struct {
	t_socket_client* socketMSP;
	t_socket* socketCPU;
	t_socket* socketConsola;
	t_log* loggerKernel;
	t_log* loggerPlanificador;
	t_log* loggerLoader;
	int puertoLoader;
	int puertoPlanificador;
	char* ipMsp;
	int puertoMsp;
	int quamtum;
	char* systemCalls;
	int tamanioStack;
	t_TCB_Kernel* tcbKernel;
} t_kernel;


//es la que comparte el loader y el planificador, con los programas cargados
t_list* cola_new;
sem_t mutex_new;
sem_t mutex_BloqueoPlanificador;// Mutex cola New
t_list* listaDeProgramasDisponibles;

//La necesita el Boot
t_list* cola_block;
sem_t mutex_block;      // Mutex cola Block

typedef struct {
	t_TCB_Kernel* programaTCB;
	t_socket* socketProgramaConsola;
} t_programaEnKernel;

int iretThread;


pthread_t LoaderHilo, PlanificadorHilo;

void kernel_crearColaDeEstados(void);
int  kernel_escuchar_conexiones(void);
void verificar_argumentosKernel(int argc, char* argv[]);



#endif /* KERNEL_H_ */
