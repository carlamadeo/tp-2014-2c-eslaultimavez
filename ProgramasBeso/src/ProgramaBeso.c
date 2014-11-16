/*
 ============================================================================
 Name        : ProgramaBeso.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "ProgramaBeso.h"
#include "programaBesoKernel.h"
#include "commons/config.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {


	if( argc != 2 ){
		printf("Modo de empleo: ./ProgramaBeso archivoBESO.bc\n");
		exit(EXIT_FAILURE);
	}

	char* config_file = getenv("ESO_CONFIG");

	if(config_file == NULL){
		printf("Debe definir la variable de entorno ESO_CONFIG.\n");
		return EXIT_FAILURE;
	}

	t_programaBESO* self = consolaCargarConfiguracion(config_file); // Cargo la configuracion

	//Creo un archivo de log para cada consola de programa beso conectada
	char *nombreLog = malloc(strlen("logProgramaBeso_.log") + sizeof(int) + 1);

	sprintf(nombreLog, "%s%d%s", "logProgramaBeso_", getpid(), ".log");

	self->loggerProgramaBESO = log_create(nombreLog, "ProgramaBeso", 1, LOG_LEVEL_DEBUG); //Creo el archivo Log

	consolaHacerConexionconLoader(self);
	consolaComunicacionLoader(self, argv[1]);

	log_destroy(self->loggerProgramaBESO);
	free(self->ipLoader);
	free(self);

	return EXIT_SUCCESS;
}


t_programaBESO* consolaCargarConfiguracion(char* config_file){

	t_programaBESO* self = malloc(sizeof(t_programaBESO));
	t_config* config = config_create(config_file);

	if(!config_has_property(config, "PUERTO_KERNEL") || !config_has_property(config, "IP_KERNEL"))
		log_error(self->loggerProgramaBESO, "Archivo de configuracion invalido");

	self->puertoLoader = config_get_int_value(config, "PUERTO_KERNEL");
	self->ipLoader = string_duplicate(config_get_string_value(config, "IP_KERNEL"));

	config_destroy(config);
	return self;
}
