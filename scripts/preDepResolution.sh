#!/bin/bash
set -e

notInstalled=()

dependencies=(
    #pré-dependencies
    build-essential
    cmake
    git
    libgtk2.0-dev
    pkg-config
    libavcodec-dev
    libavformat-dev
    libswscale-dev

    #P137 dependencies
    libboost-all-dev
    libv4l-dev
    libv4lconvert0
    libgtkmm-3.0-dev
)

installLibXbee() {
    echo -e "\033[0;31m\nInstalling libxbee3... \033[0m\n"
    git clone https://github.com/attie/libxbee3.git
    cd libxbee3
    make configure
    make
    make install
    cd ..
    rm -rf libxbee3
    echo -e "\033[1;32mlibxbee3 successfully installed.\033[0m\n"
    echo 2 #completed
}

installPreReq() {
    for nI in "${notInstalled[@]}"; do
        echo -e "\033[0;31m\t $nI \033[1;31mnot installed\033[0m"
        echo -e "\033[0;33m\n\t Installing \033[1;33m$nI\033[0;33m...\033[0m\n"
        apt-get install -y -qq "$nI" > /dev/null
    done

    if [ ! -s /usr/lib/libxbee.so ]; then
        installLibXbee
    fi

    echo -e "\033[1;32m\n\t Libraries successfully installed.\033[0m\n"
}

verifyDependencies() {
    echo -e "\033[1;37m\tVerifying installed dependencies\033[0m\n"
    for dep in "${dependencies[@]}"; do
        if dpkg-query -Wf'${Status}' "$dep" 2>/dev/null | grep -q "ok installed"; then
            echo -e "\033[0;32m\t $dep \033[1;32minstalled \033[0m\n"
        else
            notInstalled+=("$dep")
        fi
    done

    if [ ${#notInstalled[@]} -gt 0 ]; then
    #    export p_step=100/${#notInstalled[@]}
        installPreReq
    #else
        #echo "100" > $file
        #echo 2 #passed
    fi
}

verifyDependencies
