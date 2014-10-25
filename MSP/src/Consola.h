/*
 * Consola.h
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/log.h>

void *iniciarConsolaMSP();
void consolaCrearSegmento();
void consolaDestruirSegmento();
void consolaEscribirMemoria();
void consolaLeerMemoria();
void imprimirTablaDeSegmentos();
void consolaImprimirTablaDePaginas();
void imprimirMarcos();


#endif /* CONSOLA_H_ */
