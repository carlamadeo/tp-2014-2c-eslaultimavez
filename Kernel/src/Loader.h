#ifndef LOADER_H_
#define LOADER_H_

#include "kernelConfig.h"

pthread_t hiloMandarNew, hiloMandarReady;
int iretThreadLoader;

typedef struct{
	char *codigoBeso;
}t_codigoBeso;


typedef struct{
	uint32_t direccionBase;
}t_msp_DireccionBase;

t_cpu* cpuPorIDEncontrado(t_kernel* self,int idCPU);
void kernel_comenzar_Loader(t_kernel* self);
void escuchar_conexiones_programa(t_kernel* self);
void pasarProgramaNewAReady(t_kernel* self);
t_programaEnKernel* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor);
void atenderProgramaConsola(t_kernel* self,t_programaEnKernel* programa, fd_set* master);
void eliminarPrograma(t_kernel* self, t_programaEnKernel* programa);
void atenderNuevaConexionPrograma(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax);
t_TCB_Kernel* loaderCrearTCB(t_kernel* self, char *programaBeso, t_socket* socketNuevoCliente, int tamanioBeso);
void loaderTrabajaConMSP(t_kernel* self);
void loaderValidarEscrituraEnMSP(t_kernel* self, t_socket* socketNuevoCliente, int unaRespuesta);

#endif /* BOOT_H_ */
