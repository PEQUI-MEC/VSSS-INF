#!/bin/bash
set -e

PATH_TO_DEP_RESOLUTION="scripts/dependencies.sh"

printf "\n\033[43m\033[1;30m|------- VSSS 2018 - PEQUI MECÂNICO INF -------|\033[0m\n\n"

if [ -e dep_ok ]; then
    if [ ! -d "build" ]; then 
        mkdir build
    fi
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
else
    chmod +x $PATH_TO_DEP_RESOLUTION
    if [ -f ../.dockerenv ]; then
        printf "\033[1;37mWe're inside MATRIX =[ \033[0m\n"
        ./$PATH_TO_DEP_RESOLUTION
    else
        printf "\033[1;37mWe're in the REAL world =] \033[0m\n\n"
        printf "\033[0;33mWe're going to need root privileges.\033[0m\n"
        sudo -s ./$PATH_TO_DEP_RESOLUTION
    fi
    
    if [ $? -eq 0 ]; then
        touch dep_ok
    else
        echo "\033[1;31mExited with error $?\033[0m"
    fi
    chmod -x $PATH_TO_DEP_RESOLUTION
    ./build.sh
fi

