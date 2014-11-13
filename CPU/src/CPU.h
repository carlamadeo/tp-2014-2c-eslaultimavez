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
#include <commons/collections/list.h>
#include <commons/socketInBigBang.h>
#include <commons/protocolStructInBigBang.h>
#include "codigoESO.h"

//linea para hacer push
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

		typedef struct {
					t_TCB_CPU* tcb;
					t_socket_client* socketPlanificador;
					t_socket_client* socketMSP;
					t_log* loggerCPU;
					int puertoPlanificador;
					int puertoMSP;
					char* ipPlanificador;
					char* ipMsp;
					int retardo;
				}t_CPU;

		typedef struct{
			t_TCB_CPU tcb;
			t_list* proximo;
		}t_lista_TCBs;


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

void verificar_argumentosCPU(int argc, char* argv[]);
t_CPU* cpu_cargar_configuracion(char* config_file);
void ejecutar_instruccion(int linea, t_TCB_CPU* tcb);
void cpuProcesar_tcb(int pid, t_TCB_CPU* tcb);
void cambioContexto(t_TCB_CPU* tcb);
int determinar_registro(char registro);


//Instrucciones de usuario


void LOAD_ESO (int registro, int32_t numero, t_TCB_CPU* tcb); //Carga en el registro, el número dado.

void GETM_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb);	//Obtiene el valor de memoria apuntado por el segundo registro.
											//El valor obtenido lo asigna en el primer registro.

void SETM_ESO (int numero, int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Pone tantos bytes desde el segundo registro,
													//hacia la memoria apuntada por el primer registro.


void MOVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Copia el valor del segundo registro hacia el primero.

void ADDR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Suma el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.


void SUBR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Resta el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void MULR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb);//Multiplica el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void MODR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Obtiene el resto de la división del primer registro con el segundo registro.
										//El resultado de la operación se almacena en el registro A.

void DIVR_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Divide el primer registro con el segundo registro.
										//El resultado de la operación se almacena en el
										//registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV
										//y no se hace la operación.

void INCR_ESO (int registro, t_TCB_CPU* tcb); //Incrementar una unidad al registro.
void DECR_ESO (int registro, t_TCB_CPU* tcb); //Decrementa una unidad al registro.


void COMP_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Compara si el primer registro es igual al segundo.
										//De ser verdadero, se almacena el valor 1.
										//De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.
void CGEQ_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Compara si el primer registro es mayor o igual al segundo.
										//De ser verdadero, se almacena el valor 1. De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.


void CLEQ_ESO (int primer_registro, int segundo_registro, t_TCB_CPU* tcb); //Compara si el primer registro es menor o igual al segundo.
										//De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.
										//El resultado de la operación se almacena en el registro A.

void GOTO_ESO (int registro, t_TCB_CPU* tcb); //Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro.
						 //El valor es el desplazamiento desde el inicio del programa.

void JMPZ_ESO(int direccion, t_TCB_CPU* tcb);   //Altera el flujo de ejecución, solo si el valor del registro A es cero,
                       //para ejecutar la instrucción apuntada por el registro.
                      //El valor es el desplazamiento desde el inicio del programa.

void JPNZ_ESO(int direccion, t_TCB_CPU* tcb); //Altera el flujo de ejecución, solo si el valor del registro A no es cero,
					 //para ejecutar la instrucción apuntada por el registro.
					//El valor es el desplazamiento desde el inicio del programa.

void INTE_ESO(uint32_t direccion, t_TCB_CPU* tcb); 	//Interrumpe la ejecución del programa para ejecutar la rutina del kernel
								//que se encuentra en la posición apuntada por la direccion.
								//El ensamblador admite ingresar una cadena indicando el nombre,
								//que luego transformará en el número correspondiente. Los posibles valores son:
								//“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”.
								//Invoca al servicio correspondiente en el proceso Kernel.
								//Notar que el hilo en cuestión debe bloquearse tras una interrupción.

/*void FLCL(); //Limpia el registro de flags.*/


void SHIF_ESO (int numero, int registro, t_TCB_CPU* tcb); //Desplaza los bits del registro, tantas veces como se indique en el Número.
								   //De ser desplazamiento positivo, se considera hacia la derecha.
								   //De lo contrario hacia la izquierda.

void NOPP_ESO (); //Consume un ciclo del CPU sin hacer nada.

void PUSH_ESO (int numero, int registro, t_TCB_CPU* tcb); //Apila los primeros bytes, indicado por el número, del registro hacia el stack.
									 //Modifica el valor del registro cursor de stack de forma acorde.

void TAKE_ESO (int numero, int registro, t_TCB_CPU* tcb); //Desapila los primeros bytes, indicado por el número, del stack hacia el registro.
									//Modifica el valor del registro de stack de forma acorde.

void XXXX_ESO (t_TCB_CPU* tcb); //Finaliza la ejecución.

//fin de instrucciones de usuario

#endif /* CPU_H_ */
