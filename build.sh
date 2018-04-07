#!/bin/bash

printf "\n\033[1;33m|------- VSSS 2018 - PEQUI MECÂNICO INF -------|\033[0m\n"

mkdir build
cd build
cmake ..
make
cd ..
