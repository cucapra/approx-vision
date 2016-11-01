FROM kaixhin/cuda-torch:7.0
WORKDIR /root

# Copy CUDNN files to CUDA directories
RUN cp /usr/lib/x86_64-linux-gnu/libcudnn* /usr/local/cuda/lib64
RUN cp /usr/include/cudnn* /usr/local/cuda/include
RUN chmod a+r /usr/local/cuda/lib64/libcudnn*
RUN export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

# Install the MS COCO API 
RUN git clone https://github.com/pdollar/coco.git
WORKDIR /root/coco
RUN luarocks make LuaAPI/rocks/coco-scm-1.rockspec

# Install other dependencies
RUN luarocks install image
RUN luarocks install tds
RUN luarocks install json
RUN luarocks install nnx
RUN luarocks install optim
RUN luarocks install inn
RUN luarocks install cutorch
RUN luarocks install nn
RUN luarocks install cunn

# Ensure that we get Version 4 of CUDNN (since we are using CUDA 7.0)
WORKDIR /root
RUN git clone -b R4 https://github.com/soumith/cudnn.torch.git
WORKDIR /root/cudnn.torch
RUN luarocks make cudnn-scm-1.rockspec

# Install vim (for my sanity)
RUN apt-get install vim -y

# Installing DeepMask
RUN mkdir /root/deepmask-repo
ENV DEEPMASK /root/deepmask-repo/
RUN git clone https://github.com/facebookresearch/deepmask.git $DEEPMASK

# Reset run directory
WORKDIR $DEEPMASK

# Set up directory for pretrained models
RUN mkdir -p $DEEPMASK/pretrained/deepmask
RUN mkdir -p $DEEPMASK/pretrained/sharpmask
RUN mkdir -p $DEEPMASK/data
