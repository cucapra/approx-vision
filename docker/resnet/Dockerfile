FROM kaixhin/cuda-torch:7.0
WORKDIR /root

# Copy CUDNN files to CUDA directories
RUN cp /usr/lib/x86_64-linux-gnu/libcudnn* /usr/local/cuda/lib64
RUN cp /usr/include/cudnn* /usr/local/cuda/include
RUN chmod a+r /usr/local/cuda/lib64/libcudnn*
RUN export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

# Install dependencies
RUN luarocks install nn
RUN luarocks install cutorch
RUN luarocks install cunn

# Ensure that we get Version 4 of CUDNN (since we are using CUDA 7.0)
WORKDIR /root
RUN git clone -b R4 https://github.com/soumith/cudnn.torch.git
WORKDIR /root/cudnn.torch 
RUN luarocks make cudnn-scm-1.rockspec

# Install wget and python-dev
RUN apt-get install wget python-dev -y

# Install vim (for my sanity)
RUN apt-get install vim -y

# Install numpy for data analyis
RUN pip install numpy

# Clone our modified ResNet
WORKDIR /root
RUN git clone https://github.com/mbuckler/fb.resnet.torch.git

# Clone cifar.torch
RUN git clone https://github.com/mbuckler/cifar.torch.git

# Start us in the ResNet directory
WORKDIR /root/fb.resnet.torch
