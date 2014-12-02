#ifndef KERNEL_H_
#define KERNEL_H_

#include <semaphore.h>
#include <commons/collections/list.h>
#include "commons/log.h"
#include "commons/socketInBigBang.h"

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
	int quantum;
} t_QUANTUM;


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
	int quantum;
	char* systemCalls;
	int tamanioStack;
	t_TCB_Kernel* tcbKernel;
} t_kernel;



typedef struct {
	t_TCB_Kernel* programaTCB;
	t_socket* socketProgramaConsola;
} t_programaEnKernel;


typedef struct {
		int motivo;
        t_TCB_Kernel* tcbKernel;
} t_procesoBloquea;

typedef struct {
        int id;
        t_TCB_Kernel* TCB;
        t_socket* socket;
} t_cpu;

//Guardan un TCB,Socket, ID
//t_list* cola_new;  //es la que comparte el loader y el planificador, con los programas cargados
//t_list* cola_ready;
t_list* cola_exec;
t_list* cola_block; //POR CUATRO, bloqueado por systemCall, por recurso, por esperando un hilo
t_list* cola_exit;

//Guardan solo TCB para planificador
t_list* cola_CPU_Disponibles;
t_list* cola_CPU_Libres;
t_list* listaDeCPUExec;
t_list* listaDeCPULibres;
t_list* listaCpu;
t_list* listaSystemCall;
t_list* listaDeEsperaRecurso;


// Semaforos
sem_t mutex_new;
sem_t mutex_block;      // Mutex cola Block
sem_t mutex_ready;      // Mutex cola Ready
sem_t mutex_exec;       // Mutex cola Exec
sem_t mutex_exit;       // Mutex cola Exit
sem_t mutex_cpuLibre;   // Mutex cola de CPUs libres
sem_t mutex_cpuExec;    // Mutex cola de CPUs procesando
sem_t mutex_BloqueoPlanificador;// Mutex cola New


int iretThread;
pthread_t LoaderHilo, PlanificadorHilo;

void kernel_crearColaDeEstados(void);
int  kernel_escuchar_conexiones(void);
void verificar_argumentosKernel(int argc, char* argv[]);



#endif /* KERNEL_H_ */
