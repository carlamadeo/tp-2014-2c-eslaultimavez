#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "Kernel.h"

pthread_t hiloAtiendeCPU, hiloMandaEjectutarTCB;
int iretThreadPlanificador;

t_TCB_Kernel* test_TCB ();

void mandarEjecutarPrograma(t_kernel* self,t_programaEnKernel* programa, t_socket* socket);
void pasarTCB_Ready_A_Exec(t_kernel* self);
void planificadorEscucharConexionesCPU(t_kernel* self);
void kernel_comenzar_Planificador(t_kernel* self);
t_cpu* obtenerCPUSegunDescriptor(t_kernel* self,int descriptor);
void atenderNuevaConexionCPU(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax);
void atenderCPU(t_kernel* self,t_socket *socketNuevaConexionCPU, t_cpu* cpu, fd_set* master);

t_programaEnKernel* obtenerTCBdeReady(t_kernel* self);
void finalizarProgramaEnPlanificacion(t_programaEnKernel* programa);

#endif /* BOOT_H_ */
