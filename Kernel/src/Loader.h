#ifndef LOADER_H_
#define LOADER_H_


#include "Kernel.h"

int unPIDGlobal = 100;
int unTIDGlobal = 1000;

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
t_TCB_Kernel* loaderCrearTCB(t_kernel* self,t_codigoBeso *unCodigo,t_socket* socketNuevoCliente);
uint32_t loaderCrearSegmento(t_kernel* self, t_CrearSegmentoBeso* codigoBesoAMSP);
int loaderEscribirMemoria(t_kernel* self,t_TCB_Kernel* unTCB, t_codigoBeso *unCodigo,  t_socket* socketNuevoCliente);




void loaderEscuchaProgramaBeso(t_kernel* self);
void loaderTrabajaConMSP(t_kernel* self);


#endif /* BOOT_H_ */
