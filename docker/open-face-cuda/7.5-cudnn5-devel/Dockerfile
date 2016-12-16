FROM nvidia/cuda:7.5-cudnn5-devel
WORKDIR /root

# Copy CUDNN files to CUDA directories
RUN cp /usr/lib/x86_64-linux-gnu/libcudnn* /usr/local/cuda/lib64
RUN cp /usr/include/cudnn* /usr/local/cuda/include
RUN chmod a+r /usr/local/cuda/lib64/libcudnn*
RUN export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

# Install general dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    gfortran \
    git \
    graphicsmagick \
    libgraphicsmagick1-dev \
    libatlas-dev \
    libavcodec-dev \
    libavformat-dev \
    libboost-all-dev \
    libgtk2.0-dev \
    libjpeg-dev \
    liblapack-dev \
    libswscale-dev \
    pkg-config \
    python-dev \
    python-numpy \
    python-protobuf\
    software-properties-common \
    zip \
    vim \
    wget \
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Install Torch
RUN apt-get update && apt-get install -y \
  ipython3 \
  libssl-dev \
  libzmq3-dev \
  python-zmq \
  python-pip

# Install Jupyter Notebook for iTorch
RUN pip install notebook ipywidgets

# Run Torch7 installation scripts
RUN git clone https://github.com/torch/distro.git /root/torch --recursive 
WORKDIR /root/torch 
RUN ./install-deps
RUN ./install.sh

# Set ~/torch as working directory
WORKDIR /root/torch

# Export environment variables manually
ENV LUA_PATH='/root/.luarocks/share/lua/5.1/?.lua;/root/.luarocks/share/lua/5.1/?/init.lua;/root/torch/install/share/lua/5.1/?.lua;/root/torch/install/share/lua/5.1/?/init.lua;./?.lua;/root/torch/install/share/luajit-2.1.0-beta1/?.lua;/usr/local/share/lua/5.1/?.lua;/usr/local/share/lua/5.1/?/init.lua'
ENV LUA_CPATH='/root/.luarocks/lib/lua/5.1/?.so;/root/torch/install/lib/lua/5.1/?.so;./?.so;/usr/local/lib/lua/5.1/?.so;/usr/local/lib/lua/5.1/loadall.so'
ENV PATH=/root/torch/install/bin:$PATH
ENV LD_LIBRARY_PATH=/root/torch/install/lib:$LD_LIBRARY_PATH
ENV DYLD_LIBRARY_PATH=/root/torch/install/lib:$DYLD_LIBRARY_PATH
ENV LUA_CPATH='/root/torch/install/lib/?.so;'$LUA_CPATH

RUN curl -sk https://raw.githubusercontent.com/mbuckler/fblualib/master/install_all.sh | bash

RUN luarocks install nn
RUN luarocks install dpnn
RUN luarocks install cutorch
RUN luarocks install cunn
RUN luarocks install image
RUN luarocks install optim
RUN luarocks install optnet
RUN luarocks install csvigo
RUN luarocks install torchx
RUN luarocks install tds




# Ensure that we get Version 4 of CUDNN (since we are using CUDA 7.0)
#WORKDIR /root
#RUN git clone -b R4 https://github.com/soumith/cudnn.torch.git
#WORKDIR /root/cudnn.torch 
#RUN luarocks make cudnn-scm-1.rockspec

# Install OpenCV
RUN cd ~ && \
    mkdir -p ocv-tmp && \
    cd ocv-tmp && \
    curl -L https://github.com/Itseez/opencv/archive/2.4.11.zip -o ocv.zip && \
    unzip ocv.zip && \
    cd opencv-2.4.11 && \
    mkdir release && \
    cd release && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D CUDA_ARCH_BIN=5.2 \
          -D CUDA_ARCH_PTX=5.2 \
          -D BUILD_PYTHON_SUPPORT=ON \
          .. && \
    make -j8 && \
    make install && \
    rm -rf ~/ocv-tmp

# Install dlib
RUN cd ~ && \
    mkdir -p dlib-tmp && \
    cd dlib-tmp && \
    curl -L \
         https://github.com/davisking/dlib/archive/v19.0.tar.gz \
         -o dlib.tar.bz2 && \
    tar xf dlib.tar.bz2 && \
    cd dlib-19.0/python_examples && \
    mkdir build && \
    cd build && \
    cmake ../../tools/python && \
    cmake --build . --config Release && \
    cp dlib.so /usr/local/lib/python2.7/dist-packages && \
    rm -rf ~/dlib-tmp

# Install OpenFace dependencies
RUN apt-get update 
RUN apt-get install curl -y 
RUN apt-get install git -y 
RUN apt-get install graphicsmagick -y 
RUN apt-get install python-pip -y 
RUN apt-get install python-nose -y 
RUN apt-get install python-scipy -y 
RUN apt-get install python-pandas -y 
RUN apt-get install python-protobuf -y 
RUN apt-get install zip -y 
RUN apt-get install wget -y 
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* 

RUN luarocks install graphicsmagick

# Install OpenFace
WORKDIR /root
RUN apt-get install vim
RUN git clone --recursive https://github.com/mbuckler/openface.git
RUN cd ~/openface && \
    ./models/get-models.sh && \
    pip2 install -r requirements.txt && \
    python2 setup.py install && \
    pip2 install -r demos/web/requirements.txt && \
    pip2 install -r training/requirements.txt

