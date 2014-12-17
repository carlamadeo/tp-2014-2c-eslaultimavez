#!/bin/bash
dir=`pwd`

for x in Commons MSP Kernel CPU ProgramasBeso 
do
	cd ${x}/make
	make clean
	make all
	cd ${dir}
done 
