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

	consolaExtraer_programaBeso(self, argv[1]);
	consolaHacerConexionconLoader(self);
	consolaComunicacionLoader(self);

	log_destroy(self->loggerProgramaBESO);
	free(self->codigo);
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


void consolaExtraer_programaBeso(t_programaBESO *self, char *parametro){

	FILE *archivoBeso = fopen(parametro, "rb");

	if(archivoBeso == 0){
		log_error(self->loggerProgramaBESO, "El archivo no pudo abrirse");
		exit(-1);
	}

	fseek(archivoBeso, 0, SEEK_END);	//Me coloco al final del fichero para saber el tamanio
	int sizeArchivoBeso = ftell(archivoBeso);
	fseek(archivoBeso, 0, SEEK_SET);	//Me coloco al principio del fichero para leerlo

	self->codigo = malloc(sizeof(char)*sizeArchivoBeso + 1);

	int leido = fread(self->codigo, 1, sizeArchivoBeso, archivoBeso);

	if(leido == 0){
		log_info(self->loggerProgramaBESO, "El archivo no contiene codigo. Cerrando Programa...");
		exit(-1);
	}

	fclose(archivoBeso);
}
