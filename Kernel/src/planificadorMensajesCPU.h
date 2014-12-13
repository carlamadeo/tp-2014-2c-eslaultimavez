#ifndef PLANIFICADOR_MENSAJE_H_
#define PLANIFICADOR_MENSAJE_H_

#include <semaphore.h>
#include "Planificador.h"

typedef struct {
	int pid;
	int tid;
	short km;
	uint32_t base_segmento_codigo;
	int tamanio_segmento_codigo;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registro_de_programacion[5];
	uint32_t direccionKM;
} t_interrupcionKernel;


typedef struct {
	int pid;
	int tid;
} t_crea_hiloKernel;


//Esta estructura es la que se usa para la lista de System Calls
typedef struct {
	t_programaEnKernel *programa;
	uint32_t direccionKM;
} t_TCBSystemCalls;

//--------------------------//

//Estas estructuras son para saber que hilo esta bloqueado por otro
typedef struct {
	int pid;
	int TIDbloqueador;
	t_list *hilosBloqueados;
} t_BloqueadoPorOtro;

typedef struct {
	int pid;
	int tid_llamador;
	int tid_esperar;
} t_joinKernel;

//--------------------------//

//Estas estructuras son para el bloqueo y desbloqueo de hilos
//Bloqueados por algun recurso. listaBloqueados contiene una estructura t_programaEnKernel
typedef struct{
	int identificador;
	t_list *listaBloqueados;
}t_recurso;

typedef struct {
	int pid;
	int tid;
	short km;
	int32_t id_recurso;
} t_bloquearKernel;

typedef struct {
	int32_t id_recurso;
} t_despertarKernel;

//----------------------------//

typedef struct{
	int pid;
	int32_t tamanio;
	int tipo;
} t_entrada_estandarKenel;


typedef struct{
	int numero;
} t_entrada_numeroKernel;


typedef struct{
	char texto[1000];
} t_entrada_textoKernel;


typedef struct{
	int pid;
	char cadena[1000];
} t_salida_estandarKernel;

typedef struct {
	int pid;
	int tid;
	int identificadorError;
} t_errorKernel;

void agregarEnListaDeCPU(t_kernel* self, int id, t_socket* socketCPU);

void ejecutar_FINALIZAR_PROGRAMA_EXITO(t_kernel* self, t_socket_paquete *paqueteTCB);
void ejecutar_TERMINAR_QUANTUM(t_kernel* self, t_socket_paquete *paqueteTCB);
void cpuLibreAOcupada(t_cpu *CPU);
void cpuOcupadaALibre(t_cpu *CPU);
void pasarProgramaDeExecAReady(t_TCB_Kernel *TCB);
int programaBesoExiste(t_kernel* self, t_TCB_Kernel* TCBRecibido);


void ejecutar_UNA_INTERRUPCION(t_kernel* self, t_socket_paquete* paquete);
void ejecutar_FIN_DE_INTERRUPCION(t_kernel* self, t_socket_paquete* paquete);
t_socket *pasarProgramaDeExecABlock(t_TCB_Kernel *TCB);
void agregarTCBAColaSystemCalls(t_TCB_Kernel* TCBInterrupcion, uint32_t direccionKM);
void modificarTCBKM(t_TCB_Kernel *TCBKernel, t_TCBSystemCalls *TCBSystemCall);
void pasarProgramaDeBlockAReady(t_TCB_Kernel *TCB, t_socket *socketConsola);
void convertirLaInterrupcionEnTCB(t_interrupcionKernel *interrupcion, t_TCB_Kernel *TCBInterrupcion);
void volverTCBAModoNoKernel(t_TCB_Kernel * TCBKernel, t_TCB_Kernel *unTCB);

void ejecutar_UNA_ENTRADA_ESTANDAR(t_kernel* self, t_cpu *cpu, t_socket_paquete* paquete);
void ejecutar_UNA_SALIDA_ESTANDAR(t_kernel* self, t_cpu *cpu, t_socket_paquete* paquete);
void ejecutar_UN_CREAR_HILO(t_kernel* self, t_socket_paquete* paquete);
void ejecutar_UN_JOIN_HILO(t_kernel* self, t_socket_paquete* paquete);
void ejecutar_UN_BLOCK_HILO(t_kernel* self, t_socket_paquete* paquete);
void ejecutar_UN_WAKE_HILO(t_kernel* self, t_socket_paquete* paquete);
void ejecutar_UN_MENSAJE_DE_ERROR(t_kernel* self, t_socket_paquete* paquete);
void copiarValoresDosTCBs(t_TCB_Kernel *tcbHasta, t_TCB_Kernel *tcbDesde);

/*
uint32_t *recibirUnaDireccion(t_kernel* self, t_socket *socketCPU);
void enviarTCByQUANTUMCPU(t_kernel* self,t_socket *socketNuevaConexionCPU,t_TCB_Kernel* tcbKernel);
void recibirTCB(t_kernel* self,t_socket *socketNuevaConexionCPU,t_interrupcionKernel* unaInterripcion);

void printfEntradaStandar(t_entrada_estandarKenel* entrada);
void ejecutar_UN_CREAR_HILO(t_kernel* self);
void ejecutar_UN_JOIN_HILO(t_kernel* self);
void ejecutar_UN_BLOK_HILO(t_kernel* self);
void ejecutar_UN_WAKE_HILO(t_kernel* self);
void printTCBKernel(t_TCB_Kernel* unTCB);
t_programaEnKernel* obtenerProgramaDeReady(t_TCB_Kernel* tcb);
*/

#endif // PLANIFICADOR_MENSAJE_H_
