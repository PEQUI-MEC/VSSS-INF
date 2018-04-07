FROM ubuntu:17.10

#OPENCV
RUN apt-get update \
    && apt-get install -y \
        build-essential \
        cmake \ 
        git \ 
        libgtk2.0-dev \
        pkg-config \
        libavcodec-dev \ 
        libavformat-dev \
        libswscale-dev \ 
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opencv_workspace
RUN git clone \
        --depth 1 \
        --branch 3.4.1 \
        https://github.com/opencv/opencv.git 
RUN git clone \
        --depth 1 \
        --branch 3.4.1 \
        https://github.com/opencv/opencv_contrib.git  
WORKDIR ./build
RUN cmake \ 
        -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_INSTALL_PREFIX=/usr/local ../opencv \
        -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib/modules ../opencv \
    && make -j7 \
    && make install \
    && rm -r /opencv_workspace 

#XBEE
WORKDIR /Xbee_workspace
RUN git clone \
        --depth 1 \ 
        --branch v3.0.11 \
        https://github.com/attie/libxbee3.git 
WORKDIR ./libxbee3
RUN make configure \
    && make -j7 \
    && make install \
    && rm -r /Xbee_workspace 

#VSSS
WORKDIR /VSS_workspace
RUN apt-get update \
    && apt-get install -y \
        libboost-all-dev \
        libv4l-dev \
        libv4lconvert0 \
        libgtkmm-3.0-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . .
RUN ./build.sh
CMD ./P137
