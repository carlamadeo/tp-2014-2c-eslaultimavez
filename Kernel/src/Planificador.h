#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "Kernel.h"

typedef struct {
        t_socket* socket;
        int motivo;
        char* bloqueado;
} t_block_proceso;

typedef struct {
        int id;
        t_TCB_Kernel* TCB;
        t_socket* socket;

} t_cpu;


t_list* cola_ready;
t_list* cola_exec;
t_list* cola_block;
t_list* cola_exit;

t_list* cola_CPU_Disponibles;
t_list* cola_CPU_Libres;

// Semaforos
sem_t mutex_ready;      // Mutex cola Ready
sem_t mutex_exec;       // Mutex cola Exec
sem_t mutex_exit;       // Mutex cola Exit
sem_t mutex_cpuLibre;   // Mutex cola de CPUs libres
sem_t mutex_cpuExec;    // Mutex cola de CPUs procesando
t_list* listaDeCPUExec, *listaDeCPULibres;

t_TCB_Kernel* test_TCB ();

t_list* listaCpu;


void kernel_comenzar_Planificador(t_kernel* self);
t_cpu* obtenerCPUSegunDescriptor(t_kernel* self,int descriptor);
void atenderNuevaConexionCPU(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax);
void atenderCPU(t_kernel* self,t_cpu* cpu, fd_set* master);

t_programaEnKernel* obtenerTCBdeReady(t_kernel* self);
void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa);

#endif /* BOOT_H_ */
