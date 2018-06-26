#!/bin/bash

cd planificador/Debug/

make clean

make all

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-Simuladores/libraries/Debug

./planificador