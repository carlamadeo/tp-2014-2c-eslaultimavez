/*
 * protocolStruct.h
 *
 *  Created on: 26/04/2014
 *      Author: utnso
 */

#ifndef PROTOCOLSTRUCT_H_
#define PROTOCOLSTRUCT_H_


	//Código operación:       	Payload:
	#define HANDSHAKE_PROGRAMA 0
	#define HANDSHAKE_PLANIFICADOR 1
	#define HANDSHAKE_LOADER 2
	#define HANDSHAKE_MSP 3
	#define HANDSHAKE_CPU 4
	#define HANDSHAKE_KERNEL 5
	#define CREAR_SEGMENTO 10
	#define DESTRUIR_SEGMENTO 11
	#define CAMBIAR_PROCESO_ACTIVO 12
	#define CPU_SEGUI_EJECUTANDO 13
	#define CPU_NUEVO_TCB 67
	#define CPU_DAME_TCB 15
	#define	KERNEL_FIN_TCB_QUANTUM 16
	#define CPU_TERMINE_UNA_LINEA 58

	//CPU
	#define PEDIDO_DE_VARIABLE_COMPARTIDA 50
	#define ASIGNAR_VARIABLE_COMPARTIDA 51
	#define ENTRADA_SALIDA_A_KERNEL 52
	#define WAIT_KERNEL 53
	#define SIGNAL_KERNEL 54
	#define IMPRIMIR_TEXTO 55
	#define IMPRIMIR_VALOR_KERNEL 56
	#define FINALIZAR_PROGRAMA 57
	#define LINEA_A_EJECUTAR 61
	#define BLOCK_PROCESS 62
 	#define SOLICITAR_BYTES 63
	#define BYTES_RECIBIDOS 68
	#define GRABAR_BYTES 64
	#define DEFINIR_VARIABLE 65
	#define VARIABLE_DEFINIDA_CORRECTAMENTE 66
	#define CAMBIO_DE_PROCESO_ACTIVO 68
	#define DESCONECTAR_CPU 69
	#define PROCESO_BLOQUEADO 80
	#define PROCESO_TERMINADO 81
	#define QUANTUM_TERMINADO 82
	#define PROCESO_BLOQUEADO_POR_SEMAFORO 83

	//MSP
	#define ERROR_EXCEPCION 90
	#define INICIO_KERNEL 91
	#define NUEVO_CPU 92
	#define NUEVA_CONSOLA 93
	#define FINALIZAR 100

	//Kernel recibe de MSP
	#define BASE 120
	#define BASE_MSP 121

	//Respuestas:
	//El payload de las respuestas (si no se aclara) es NULL o código de error.
	#define CODIGO_RECIBIDO 40 	//puerto y codigo del Programa al PLP.

	//Estructura para lo que quieran. Poner las structs necesarias.
	typedef struct { //HANDSHAKE_PROGRAMACONSOLA
		char* programaConsola;
		int puertoProgramaConsola;
	} t_info_programa_handshake;


	typedef struct { //CODIGO_RECIBIDO
		char* programaConsola;
		int id;
	} t_programaRecibido;

	typedef struct { //HANDSHAKE_TCB
		int pid;
	} t_info_TCB_handshake;


#endif /* PROTOCOLSTRUCT_H_ */
