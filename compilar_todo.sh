#!/bin/bash
dir=`pwd`

for x in Commons MSP Kernel CPU ProgramaBeso 
do
	cd ${x}
	make clean
	make all
	cd ${dir}
done 
