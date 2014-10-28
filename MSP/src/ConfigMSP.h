/*
 * CSonfigMSP.h
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#ifndef CONFIGMSP_H_
#define CONFIGMSP_H_

#include <commons/collections/list.h>

#define KILOBYTES_A_BYTES 1024
#define MEGABYTES_A_BYTES 1048576
#define FIFO 0
#define CLOCK_MODIFICADO 1

bool cargarConfiguracionMSP(char *config);
bool validarConfiguracionMSP();
void cargarMarcos(t_list *listaMarcos);
void destruirConfiguracionMSP();

#endif /* CONFIGMSP_H_ */
