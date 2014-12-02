/*
 * mspConfig.c
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#include "mspConfig.h"
#include "Memoria.h"
#include "MSP.h"
#include "commons/config.h"
#include "commons/string.h"
#include <stdlib.h>
#include <string.h>

t_config *MSPConfig;
t_MSP *self;

const char configProperties[][25] = {"PUERTO", "CANTIDAD_MEMORIA", "CANTIDAD_SWAP", "SUST_PAGS"};

bool cargarConfiguracionMSP(char *config) {

	MSPConfig = config_create(config);

	if(!validarConfiguracionMSP()) return false;

	self->puerto = config_get_int_value(MSPConfig, "PUERTO");
	self->cantidadMemoriaPrincipal = config_get_double_value(MSPConfig, "CANTIDAD_MEMORIA");
	self->cantidadMemoriaSecundaria = config_get_double_value(MSPConfig, "CANTIDAD_SWAP");

	if(string_equals_ignore_case(config_get_string_value(MSPConfig, "SUST_PAGS"), "FIFO"))
		self->modoSustitucionPaginas = FIFO;
	else
		self->modoSustitucionPaginas = CLOCK_MODIFICADO;

	log_info(self->logMSP, "Reservando %g KiloBytes de memoria principal... ", self->cantidadMemoriaPrincipal);

	self->cantidadMemoriaPrincipal *= KILOBYTES_A_BYTES; //Los paso a Bytes

	//Alocar el tamanio obtenido por archivo de configuracion a la memoria
	self->memoria = malloc(self->cantidadMemoriaPrincipal);

	memset(self->memoria, 0, self->cantidadMemoriaPrincipal);

	if (self->memoria == 0) {
		log_error(self->logMSP, "No se pudo alocar la memoria, finalizando...");
		return false;
	}

	log_info(self->logMSP, "Memoria reservada correctamente");
	log_info(self->logMSP, "Se reservo %g MegaBytes para el archivo de paginacion.", self->cantidadMemoriaSecundaria);

	self->cantidadMemoriaSecundaria *= MEGABYTES_A_BYTES; //Los paso a Bytes

	self->programas = list_create();
	self->marcosLibres = list_create();
	self->marcosOcupados = list_create();

	cargarMarcos(self->marcosLibres);

	config_destroy(MSPConfig);

	return true;
}

void cargarMarcos(t_list *listaMarcos){
	int cantidadMarcos;
	int i;
	uint32_t inicioMarco = 0;

	cantidadMarcos = self->cantidadMemoriaPrincipal/TAMANIO_PAGINA;

	for(i = 0; i < cantidadMarcos; i++){
		t_marco *marco = malloc(sizeof(t_marco));

		marco->numero = i;
		marco->inicio = inicioMarco;
		marco->categoriaClockModificado = NOREFERENCIADA_NOMODIFICADA;
		list_add(listaMarcos, marco);
		inicioMarco+=TAMANIO_PAGINA;
	}
}


bool validarConfiguracionMSP(){
	bool existen = true;
	int elements = sizeof(configProperties)/sizeof(configProperties[0]);
	int i;

	for(i = 0; i < elements; i++) {
		if(!config_has_property(MSPConfig, &configProperties[i])) {
			existen = false;
			break;
		}
	}

	return existen;
}


void destruirConfiguracionMSP(){
	list_destroy_and_destroy_elements(self->programas, free);
	list_destroy_and_destroy_elements(self->marcosLibres, free);
	list_destroy_and_destroy_elements(self->marcosOcupados, free);

	free(self);
}
