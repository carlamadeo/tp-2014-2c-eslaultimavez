#ifndef CPUKERNEL_H_
#define CPUKERNEL_H_

#include "CPU_Proceso.h"

#define TAMANIO_MAXIMO 1000

typedef struct {
	t_TCB_CPU* tcb;
	int quantum;
} t_TCB_CPU_nuevo;

/*
 * Nombre: t_entrada_estandar
 * Descripcion: envia al Kernel un pid, un tamanio maximo de entrada y un identificador de tipo de entrada de datos por consola.
 * Mensaje: ENTRADA_ESTANDAR
 *
 * 1 = numero (-2.147.483.648 a -2.147.483.648)
 * 2 = string (el tamanio de la cadena ingresada es indicado por el tamanio)
 *
 * */

typedef struct{
	int pid;
	int32_t tamanio;
	int tipo;
	int idCPU;
} t_entrada_estandar;


typedef struct{
	int numero;
} t_entrada_numeroCPU;


typedef struct{
	char entradaEstandar[TAMANIO_MAXIMO];
} t_entrada_charCPU;

/*
 * Nombre: t_salida_estandar
 * Descripcion: envia al Kernel un pid y una cadena para que se muestre por pantalla.
 * Mensaje: SALIDA_ESTANDAR
 * */

typedef struct{
	int pid;
	char cadena[TAMANIO_MAXIMO];
} t_salida_estandar;

/*
 * Nombre: t_interrupcion
 * Descripcion: envia al Kernel un tcb y una direccion virtual (desplazamiento dentro del systemcall.bc)
 * 				el CPU queda bloqueado esperando que reciba el tcbkernel, con todos los campos iguales al tcb enviado, excepto
 * 				por el puntero_instruccion, donde debe ir la direccion virtual enviada.
 * Mensaje: INTERRUPCION
 * */

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
} t_interrupcion;

/*
 * Nombre: t_join
 * Descripcion: envia al Kernel un tid_llamador, que se bloquea hasta que el tid_esperar termine su ejecucion.
 * Mensaje: JOIN_HILO
 * */

typedef struct {
	int pid;
	int tid_llamador;
	int tid_esperar;
} t_join;


/*
 * Nombre: t_bloquear
 * Descripcion: envia al Kernel un tcb y un id de recurso, indicando que el tcb debe bloquearse en la cola del recurso
 * 				asociado al id de recurso.
 * Mensaje: BLOK_HILO
 * */

typedef struct {
	int pid;
	int tid;
	short km;
	int32_t id_recurso;
} t_bloquear;

/*
 * Nombre: t_despertar
 * Descripcion: envia al Kernel un tcb y un id de recurso, indicando que el tcb debe bloquearse en la cola del recurso
 * 				asociado al id de recurso.
 * Mensaje: WAKE_HILO
 * */

typedef struct {
	int32_t id_recurso;
} t_despertar;

/*
 * Nombre: t_crear_hilo
 * Descripcion: envia al Kernel un tcb indicando que se debe crear un hilo hijo y luego planificarlo.
 * Mensaje: CREA_HILO
 *
 * */

typedef struct {
	int pid;
	int tid;
} t_crea_hilo;


typedef struct {
	t_TCB_CPU_nuevo* tcbNuevo;
	t_entrada_estandar* entradaEstandar;
	t_salida_estandar*  salidaEstandar;
	t_interrupcion* instruccion;
	t_join* join;
	t_bloquear* bloquear;
	t_despertar* despear;
	t_crea_hilo* crearHijo;
} t_ServiciosAlPlanificador;


typedef struct {
	int pid;
	int tid;
	int identificadorError;
} t_error;


int cpuFinalizarInterrupcion(t_CPU *self);
void cpuConectarConKernel(t_CPU *self);
void cpuRealizarHandshakeConKernel(t_CPU *self);
int cpuRecibirTCB(t_CPU *self);
int cpuRecibirQuantum(t_CPU *self);
void cpuEnviarPaqueteAPlanificador(t_CPU *self, int paquete);
void cpuTerminarQuantum(t_CPU *self);
int cpuEnviaInterrupcion(t_CPU *self);
void cpuRecibeInterrupcion(t_CPU *self);
int cpuFinalizarProgramaExitoso(t_CPU *self);
int cpuFinalizarHiloExitoso(t_CPU *self);
int cpuSolicitarEntradaEstandar(t_CPU *self, int tamanio, int tipo);
int reciboEntradaEstandarINT(t_CPU *self, int *recibido);
int reciboEntradaEstandarCHAR(t_CPU *self, char *recibido, int tamanio);
int cpuEnviarSalidaEstandar(t_CPU *self, char *salidaEstandar, int tamanio);



#endif /* CPUKERNEL_H_ */
