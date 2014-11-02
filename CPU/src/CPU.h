#ifndef CPU_H_
#define CPU_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/socketInBigBang.h>
#include <commons/protocolStructInBigBang.h>
#include "codigoESO.h"
//#include <commons/threadInBigBang.h>


/*
 * Estucturas para usar en CPU!
 * Estas son las primeras que tiene que cambiar y decidir si van o no van en el tp
 */
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
		} t_TCB;

		typedef struct {
			t_TCB TCB;
			int puertoConsola;
			int Quamtum;
			char ipConsola[20];
			t_socket* socket;
		} t_consolaESO;


		typedef struct {
			t_socket * socket;
			t_socket_client * socket_client;
		} t_socket_conexion;

		typedef struct {
			u_int32_t numero;
		} t_envio_numMSP;

		typedef struct {
			int longitud;
			int tiempo;
			int pid;
			int tid;
		} t_entrada_salida;

		typedef struct {
			int longitud;
			int pid;
			int tid;
		} t_semaforo_wait;

		typedef struct {
			char* ip;
			int port;
		} t_info_conexion;


		typedef struct {
			int mje_int;
			char r;
		} t_info_int;

		typedef struct {
			int offset;
			int tamanio;
		} t_valores_instruccion;


		typedef struct {
			int offset;
			int tamanio;
		} t_atributos_instr;


/*
 * Fin de las Estucturas para usar en CPU
 */




/*
* Estucturas para comunicar CPU con MSP
*/
		typedef struct {
			int pid;
			int tid;
		} t_info_programa_handshake_msp;


		typedef struct {
			int base;
			int offset;
			int tamanio;
			int pid;
			int tid;
		} t_solicitar_bytes;

		typedef struct {
			u_int32_t base;
			u_int32_t offset;
			u_int32_t tamanio;
			int pid;
			int tid;
		} t_grabar_bytes;
/*
 * Fin de las estucturas para comunicar CPU con MSP
 */




/*
 * Estucturas para comunicar CPU con Kernel
 */
		typedef struct {
			int retardo;
			int quantum;
			int stack;
		} t_handshake_cpu_kernel;
/*
 * Fin de las estucturas para comunicar CPU con Kernel
 */



t_TCB* tcb;
char * config_file;

#define PATH_LOG "logs/trace.log" //Donde esta el archivo LOG
#define PATH_CONFIG "../config"



t_socket* socketDelKernel;
t_socket* socketDelMSP;

t_log* logger;

t_info_conexion info_conexion_MSP;
t_info_conexion info_conexion_KERNEL;



/*
 * Aqui se declaran las funciones
 */
void cpuCargar_configuracionCPU();
void cpuEjecutar_una_linea(char linea);
void cpuProcesar_tcb(t_socket* socketDelKernel);
int cpuDeterminarRegProg(char Registro);

#endif /* CPU_H_ */
