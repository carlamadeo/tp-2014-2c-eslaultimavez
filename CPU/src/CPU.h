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
		} t_TCB_CPU;

//Codigos ESO

#define LOAD 1
#define GETM 2
#define SETM 3
#define MOVR 4
#define ADDR 5
#define SUBR 6
#define MULR 7
#define MODR 8
#define DIVR 9
#define INCR 10
#define DECR 11
#define COMP 12
#define CGEQ 13
#define CLEQ 14
#define GOTO 15
#define JMPZ 16
#define JPNZ 17
#define INTE 18
#define SHIF 19
#define NOPP 20
#define PUSH 21
#define TAKE 22
#define XXXX 23
#define MALC 24
#define FREE 25
#define INNN 26
#define INNC 27
#define OUTN 28
#define OUTC 29
#define CREA 30
#define JOIN 31
#define BLOK 32
#define WAKE 33

/*
 * Estucturas para usar en CPU!
 * Estas son las primeras que tiene que cambiar y decidir si van o no van en el tp
 */


		struct lista_TCBs{
			t_TCB_CPU tcb;
			struct lista_TCBs *proximo;
		};


		typedef struct {
			t_socket * socket;
			t_socket_client * socket_client;
		} t_socket_conexion;

		typedef struct {
			u_int32_t numero;
		} t_envio_num;

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

  /*definidos para serializar/deserializar */

		typedef struct{
			int tamanio;
			char *data;
		}t_paquete_MSP;





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



t_TCB_CPU* tcb;
struct lista_TCBs* primero;
struct lista_TCBs* ultimo;


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
void ejecutar_instruccion(int linea, t_TCB_CPU* tcb);
void cpuProcesar_tcb(int pid, t_TCB_CPU* tcb);
void cambioContexto(t_TCB_CPU* tcb);
int determinar_registro(char registro);
#endif /* CPU_H_ */
