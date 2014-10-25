/*
 * CSonfigMSP.h
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#ifndef CONFIGMSP_H_
#define CONFIGMSP_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define KILOBYTES_A_BYTES 1024
#define MEGABYTES_A_BYTES 1048576
#define FIFO 0
#define CLOCK_MODIFICADO 1

bool cargarConfiguracionMSP(char *config);
bool validarConfiguracionMSP();
void cargarMarcos(t_list *listaMarcos);

#endif /* CONFIGMSP_H_ */
