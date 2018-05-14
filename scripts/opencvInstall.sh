#!/bin/bash

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
    make install
    cd ..
    cd ..
    rm -rf /opencv*
    #echo 100 > $file
    echo -e "\033[0;31m\nOpenCV successfully installed.\033[0m\n"
}

installOpenCV_WITH_CUDA() {
    echo -e "\033[1;33m\tINSTALLING OPENCV WITH CUDA... \033[0m\n"

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
        -D WITH_CUDA=ON \
        -D WITH_CUBLAS=ON \
        -D WITH_TBB=ON \
        -D WITH_V4L=ON \
        -D WITH_OPENGL=ON \
        -D BUILD_TESTES=OFF \
        -D BUILD_PERF_TESTS=OFF \
        -D BUILD_opencv_java=OFF \
        -D CUDA_GENERATION=Auto \
        -D CUDA_NVCC_FLAGS="-D_FORCE_INLINES" \
        -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
    echo 33 > $file
    thread_num=$(nproc)
    make -j$((++thread_num))
    echo 66 > $file
    make install
    cd ..
    cd ..
    rm -rf /opencv*
    echo -e "\033[0;31m\nOpenCV successfully installed.\033[0m\n"
}

echo -e "\033[1;37m\tVerifying OpenCV installation \033[0m\n"

pkg-config --libs opencv 2>opencvIsInstalled 1>/dev/null
if [ -s opencvIsInstalled ]; then
    if [ "$1" == "cuda" ]; then
        installOpenCV_WITH_CUDA
    else
        installOpenCV
    fi
else
    echo -e "\033[0;32m\t OpenCV \033[1;32minstalled\033[0m\n"
fi
