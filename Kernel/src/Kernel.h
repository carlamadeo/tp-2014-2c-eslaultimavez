#ifndef KERNEL_H_
#define KERNEL_H_

#include <semaphore.h>
#include <arpa/inet.h>
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




typedef struct {
        int id;
        t_socket* socket;
        int pid;
} t_cpu;


typedef struct {
		t_TCB_Kernel TCB;
        int tamanioEnBytes;
        t_socket* socket;
} t_programaEnKernel;


typedef struct {
		t_TCB_Kernel TCB;
        t_socket* socket;
        int motivo;
        char * bloqueado;
} t_block_proceso;



t_list* cola_new;
t_list* cola_ready;
t_list* cola_exec;
t_list* cola_block;
t_list* cola_exit;



int iretThread;

// Semaforos
sem_t sem_multiprog;    // Contador de la multiprogramacion (Ready + Exec + Block)
sem_t sem_new;          // Contador de PCBs en New
sem_t sem_ready;        // Contador de PCBs en Ready
sem_t sem_exit;         // Contador de PCBs en Exit
sem_t sem_cpuLibre;     // Contador de CPUs libres
sem_t sem_cpuExec;      // Contador de CPUs en ejecucion
sem_t cola_io;

// Mutexs
sem_t mutex_new;         // Mutex cola New
sem_t mutex_ready;      // Mutex cola Ready
sem_t mutex_exec;       // Mutex cola Exec
sem_t mutex_exit;       // Mutez cola Exit
sem_t mutex_block;      // Mutez cola Block
sem_t mutex_cpuLibre;   // Mutex cola de CPUs libres
sem_t mutex_cpuExec;    // Mutex cola de CPUs procesando
sem_t mutex_semaforos;  // Mutex cola de Semaforos
sem_t mutex_pedidos;    // Mutex cola de Semaforos
t_list* listaDeCPUExec, *listaDeCPULibres, *listaDeIO, *listaDeGlobales;



int cantColaNew;
int cantColaReady;
int cantColaExec;
int cantColaExit;
int cantColaBlock;


typedef char* t_variable_compartida;

/*---------------------- Funciones del Kernel 6 -----------------------------------------------*/
/*---------------------- Funciones del Kernel 6 -----------------------------------------------*/

int kernel_imprimirListas();
void kernel_errorAlPrograma(t_socket* socket, int error);
void kernel_finalizarPrograma2(t_programaEnKernel* programaEnKernel);
void kernel_ponerCpuEnLibre(int descriptor);

pthread_t LoaderHilo, PlanificadorHilo;

void kernel_crearColaDeEstados(void);
int  kernel_escuchar_conexiones(void);

void verificar_argumentosKernel(int argc, char* argv[]);

void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa);


#endif /* KERNEL_H_ */
