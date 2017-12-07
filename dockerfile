FROM ubuntu:17.10

RUN apt-get update

#OPENCV
RUN apt-get install -y \
        build-essential \
        cmake \ 
        git \ 
        libgtk2.0-dev \
        pkg-config \
        libavcodec-dev \ 
        libavformat-dev \
        libswscale-dev

WORKDIR /opencv_workspace
RUN git clone https://github.com/opencv/opencv.git --depth 1
WORKDIR ./opencv/build
RUN cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
RUN make
RUN make install

#VSSS
WORKDIR /VSS_workspace
RUN apt-get install -y \
        libboost-all-dev \
        libv4l-dev \
        libv4lconvert0 \
        libgtkmm-3.0-dev

COPY . .
RUN ./buildVSSS.sh
CMD ./runVSSS.sh