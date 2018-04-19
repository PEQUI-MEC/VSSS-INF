#!/bin/bash

# file="/tmp/progress_2"
# echo 0 > $file

installOpenCV() {
    echo -e "\033[1;33m\tINSTALLING OPENCV... \033[0m\n"

    mkdir OpenCV-install
    cd OpenCV-install
    git clone --depth=1 --branch 3.4.1 https://github.com/opencv/opencv.git
    git clone --depth=1 --branch 3.4.1 https://github.com/opencv/opencv_contrib.git
    cd opencv
    mkdir build
    cd build
    cmake \
        -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
    echo 33 > $file
    thread_num=$(nproc)
    make -j$((++thread_num))
    echo 66 > $file
    sudo -k make install
    cd ..
    cd ..
    rm -rf /opencv*
    #echo 100 > $file
    echo -e "\033[0;31m\nOpenCV successfully installed.\033[0m\n"
}

echo -e "\033[1;37m\tVerifying OpenCV installation \033[0m\n"

pkg-config --libs opencv 2>opencvIsInstalled 1>/dev/null
if [ -s opencvIsInstalled ]; then
    installOpenCV
    #echo 5
else
    echo -e "\033[0;32m\t OpenCV \033[1;32minstalled\033[0m\n"
    #echo 2
fi

#echo 1
