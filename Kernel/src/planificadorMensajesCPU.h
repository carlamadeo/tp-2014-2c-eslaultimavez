#ifndef PLANIFICADOR_MENSAJE_H_
#define PLANIFICADOR_MENSAJE_H_

#include "Planificador.h"

typedef struct {
	t_TCB_Kernel *tcb;
	uint32_t direccion;
} t_interrupcionKernel;


typedef struct {
	uint32_t direccion;
} t_interrupcionDireccionKernel;

typedef struct{
	int pid;
	int32_t tamanio;
	int tipo;
} t_entrada_estandarKenel;

typedef struct{
	int numero;
} t_entrada_numeroKernel;

typedef struct{
	char* texto;
} t_entrada_textoKernel;

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

typedef struct {
	int ramaDelHiloPrincipal;
	t_TCB_Kernel* tcb;
} t_crea_hiloKernelSecundario;


void enviarTCByQUANTUMCPU(t_kernel* self,t_socket *socketNuevaConexionCPU,t_TCB_Kernel* tcbKernel);
void recibirUnaDireccion(t_kernel* self,t_socket *socketNuevaConexionCPU,t_interrupcionKernel* unaInterripcion);
void recibirTCB(t_kernel* self,t_socket *socketNuevaConexionCPU,t_interrupcionKernel* unaInterripcion);
void agregarEnListaDeCPU(int id,  t_socket* socketCPU);
void ejecutar_CPU_TERMINE_UNA_LINEA (t_kernel* self,t_socket* socketNuevoCliente);
void ejecutar_UN_CAMBIO_DE_CONTEXTO(t_kernel* self,t_socket *socketNuevaConexionCPU);
void ejecutar_FINALIZAR_PROGRAMA_EXITO(t_kernel* self, t_socket *socketNuevaConexionCPU);
void ejecutar_UNA_INTERRUPCION(t_kernel* self,t_socket *socketNuevaConexionCPU);
void printfEntradaStandar(t_entrada_estandarKenel* entrada);
void ejecutar_UNA_ENTRADA_STANDAR(t_kernel* self, t_socket *socketNuevaConexionCPU);
void ejecutar_UNA_SALIDA_ESTANDAR(t_kernel* self);
void ejecutar_UN_CREAR_HILO(t_kernel* self);
void ejecutar_UN_JOIN_HILO(t_kernel* self);
void ejecutar_UN_BLOK_HILO(t_kernel* self);
void ejecutar_UN_WAKE_HILO(t_kernel* self);
void printTCBKernel(t_TCB_Kernel* unTCB);
t_programaEnKernel* obtenerProgramaDeReady(t_TCB_Kernel* tcb);

#endif // PLANIFICADOR_MENSAJE_H_
