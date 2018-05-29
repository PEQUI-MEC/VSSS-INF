#!/bin/bash

set -e

PATH_DEP="scripts/preDepResolution.sh"
chmod +x $PATH_DEP

# CUDA install function
function installCuda() {
    echo -e "This may take a long time depending on your internet connection..."
    echo -e "Make sure you have enough time to continue..."
    echo -e "If you want, you can cancel using ctrl + c command and exit installation anytime."
    printf "10"
    for time in {9..1}; do
        printf "...$time"
        sleep 1
    done

    wget https://developer.nvidia.com/compute/cuda/9.1/Prod/local_installers/cuda_9.1.85_387.26_linux
    sudo ./cuda_9.1.85_387.26_linux.run

    export PATH=/usr/local/cuda-9.1/bin${PATH:+:${PATH}}
    export LD_LIBRARY_PATH=/usr/local/cuda-9.1/lib64\${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

    rm -rf cuda_9.1.85_387.26_linux.run
}

echo -e "Preparing to install dependencies...\n"
./$PATH_DEP

echo -e "Verifying cuda installation...\n"
nvcc -V 1> cudaIsInstalled
if [ -s cudaIsInstalled ]; then
    echo -e "\033[0;32m\t CUDA \033[1;32minstalled\033[0m\n"
else
    installCuda
fi

chmod -x $PATH_DEP
