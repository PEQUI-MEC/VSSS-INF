#!/bin/bash

# builtin variables
OPENCV_VERSION="3.4.1"

set -e

PATH_TO_DEP_RESOLUTION="scripts/dependencies.sh"
PATH_TO_CUDA_RESOLUTION="scripts/cudaSupport.sh"

printf "\n\033[43m\033[1;30m|------- VSSS 2018 - PEQUI MECÂNICO INF -------|\033[0m\n\n"

function dep_resolution() {
    if [ -e dep_ok ]; then
        if [ ! -d "build" ]; then 
            mkdir build
        fi
        cd build
        cmake ..
        make
        cd ..
    else
        if [ -f ../.dockerenv ]; then
            printf "\033[1;37mWe're inside MATRIX =[ be creative...\033[0m\n"
            #if [ "$1"  = "cuda" ]; then
            #    chmod +x $PATH_TO_CUDA_RESOLUTION
            #    ./$PATH_TO_CUDA_RESOLUTION
            #fi
            chmod +x $PATH_TO_DEP_RESOLUTION
            ./$PATH_TO_DEP_RESOLUTION

        else
            printf "\033[1;37mWe're in the REAL world =] be careful...\033[0m\n\n"
            printf "\033[0;33mWe're going to need root privileges.\033[0m\n"
            if [ "$1"  = "cuda" ]; then
                chmod +x $PATH_TO_CUDA_RESOLUTION
                sudo -s ./$PATH_TO_CUDA_RESOLUTION
            else
                chmod +x $PATH_TO_DEP_RESOLUTION
                sudo -s ./$PATH_TO_DEP_RESOLUTION
            fi
        fi
        
        if [ $? -eq 0 ]; then
            touch dep_ok
        else
            echo "\033[1;31mExited with error $?\033[0m"
        fi
        chmod -x $PATH_TO_DEP_RESOLUTION $PATH_TO_CUDA_RESOLUTION
        ./build.sh
    fi
}

function usage() {
    printf "\033[1;37mUsage: $0 [-wc|--with-cuda] [-s|--simu] [--clean] [--help]\nOptions:\t
    
    -wc, --with-cuda\t\t installs with CUDA support.
    -s, --simu\t\t\t install simluator environment.
    --clean\t\t\t clean build folder.
    --help\t\t\t usage.
    \n"
    exit 1
}

while [ "$1" != "" ]; do
    case $1 in
        -wc|--with-cuda)
            printf "Instalando dependências com suporte a CUDA\n"
            dep_resolution cuda
        ;;
        -s|--simu)
            printf "Instalando simulador\n"
        ;;
        --clean)
            check_loadedConfig
            rm -rf ${WORKDIR}/OpenCV-install/opencv-${OPENCV_VERSION}/build/
            printf "\033[1;37mOpenCV build folder was removed \033[0m\n\n"
            rm -rf ${WORKDIR}/dep_ok
            rm -rf ${WORKDIR}/opencvIsInstalled
            printf "\033[1;37mdep_ok and opencvIsInstalled was removed \033[0m\n\n"
        ;;
        --help)
            usage
        ;;
        *)
            dep_resolution
        ;;
    esac
    shift
done

