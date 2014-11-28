#ifndef PLANIFICADOR_MENSAJE_H_
#define PLANIFICADOR_MENSAJE_H_

#include "Planificador.h"

typedef struct {
	t_TCB_Kernel *tcb;
	uint32_t direccion;
} t_interrupcionKernel;


typedef struct{
	int pid;
	int32_t tamanio;
	int tipo;
} t_entrada_estandarKenel;


typedef struct{
	int pid;
	char* cadena;
} t_salida_estandarKernel;

typedef struct {
	int tid_llamador;
	int tid_esperar;
} t_joinKernelKernel;

typedef struct {
	t_TCB_Kernel* tcb;
	int32_t id_recurso;
} t_bloquearKernel;

typedef struct {
	int32_t id_recurso;
} t_despertarKernel;

typedef struct {
	t_TCB_Kernel* tcb;
} t_crea_hiloKernel;

void agregarEnListaDeCPU(int id,  t_socket* socketCPU);
void ejecutar_CPU_TERMINE_UNA_LINEA (t_kernel* self,t_socket* socketNuevoCliente);

void ejecutar_UNA_INTERRUPCION(t_kernel* self);

void ejecutar_UNA_ENTRADA_STANDAR();
void ejecutar_UNA_SALIDA_ESTANDAR();
void ejecutar_UN_CREAR_HILO();
void ejecutar_UN_JOIN_HILO();
void ejecutar_UN_BLOK_HILO();
void ejecutar_UN_WAKE_HILO();

#endif // PLANIFICADOR_MENSAJE_H_
