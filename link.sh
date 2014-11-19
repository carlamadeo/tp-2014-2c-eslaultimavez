#!/bin/bash
# Crea los links simbolicos de los archivos de configuracion dentro del directorio Debug para ejecutar desde consola.
for x in MSP CPU Kernel ProgramaBeso; do cd ${x}/Debug; ln -s ../${x}.conf ${x}.conf;