#ifndef LOADER_H_
#define LOADER_H_


#include "Kernel.h"


pthread_t programaBesoHilo, conexionConMSPHilo;
t_list* listaDeProgramas;

typedef struct{
	int estado;
}t_confirmacionEscritura;


typedef struct{
	char * codigoBeso;
}t_codigoBeso;

typedef struct {
	int pid;
	int tamanio;
} t_CrearSegmentoBeso;

typedef struct{
	uint32_t direccionBase;
}t_msp_DireccionBase;

typedef struct {
	int pid;
	uint32_t direccionVirtual;
	char *bufferCodigoBeso;
	int tamanio;
} t_EscribirSegmentoBeso;

typedef struct {
	t_TCB_Kernel programaTCB;
	t_socket* socketProgramaConsola;
} t_programa;

void kernel_comenzar_Loader(t_kernel* self);
t_programa* obtenerProgramaConsolaSegunDescriptor(t_kernel* self,int descriptor);
void atienderProgramaConsola(t_kernel* self,t_programa* programa, fd_set* master);
void atenderNuevaConexionPrograma(t_kernel* self,t_socket* socketNuevoCliente, fd_set* master, int* fdmax);
void loaderCrearTCB(t_kernel* self, char* codigoPrograma, int tamanioEnBytes, int pid, int tid);
int loaderCrearSegmento(t_kernel* self,int pid, int tamanio);

void loaderEscuchaProgramaBeso(t_kernel* self);
void loaderTrabajaConMSP(t_kernel* self);


#endif /* BOOT_H_ */
