/*
 * mspConfig.h
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#ifndef MSPCONFIG_H_
#define MSPCONFIG_H_

#include <commons/collections/list.h>

#define KILOBYTES_A_BYTES 1024
#define MEGABYTES_A_BYTES 1048576
#define FIFO 0
#define CLOCK_MODIFICADO 1

bool cargarConfiguracionMSP(char *config);
bool validarConfiguracionMSP();
void cargarMarcos(t_list *listaMarcos);
void destruirConfiguracionMSP();

#endif /* MSPCONFIG_H_ */
