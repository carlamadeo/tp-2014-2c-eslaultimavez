/*
 * ejecucion.h
 *
 *  Created on: 26/11/2014
 *      Author: utnso
 */

#ifndef EJECUCION_H_
#define EJECUCION_H_

#define LOAD 0
#define GETM 1
#define SETM 2
#define MOVR 3
#define ADDR 4
#define SUBR 5
#define MULR 6
#define MODR 7
#define DIVR 8
#define INCR 9
#define DECR 10
#define COMP 11
#define CGEQ 12
#define CLEQ 13
#define GOTO 14
#define JMPZ 15
#define JPNZ 16
#define INTE 17
#define SHIF 18
#define NOPP 19
#define PUSH 20
#define TAKE 21
#define XXXX 22
#define MALC 23
#define FREE 24
#define INNN 25
#define INNC 26
#define OUTN 27
#define OUTC 28
#define CREA 29
#define JOIN 30
#define BLOK 31
#define WAKE 32
#define CANTIDAD_INSTRUCCIONES 33

t_list* parametros;

int cpuProcesarTCB(t_CPU *self,t_ServiciosAlPlanificador* serviciosAlPlanificador);
int determinar_registro(char registro);
int ejecutar_instruccion(t_CPU *self, int indice,  t_ServiciosAlPlanificador* serviciosAlPlanificador);

#endif /* EJECUCION_H_ */
