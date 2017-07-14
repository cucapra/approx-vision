FROM kaixhin/cuda-caffe:8.0
WORKDIR /root

# Install wget and python-dev
RUN apt-get install wget python-dev -y

# Install vim (for my sanity)
RUN apt-get install vim -y

# Install numpy for data analyis
RUN pip install numpy easydict

# Install Faster RCNN dependencies
RUN apt-get install cmake cython python-opencv -y

# Clone Faster RCNN
RUN apt-get install git
RUN git config --global user.email "markabuckler@gmail.com"
RUN git config --global user.name "mbuckler"
RUN git clone --recursive https://github.com/mbuckler/py-faster-rcnn.git

WORKDIR /root/py-faster-rcnn

# Build Faster RCNN
WORKDIR /root/py-faster-rcnn/lib
RUN make

WORKDIR /root/py-faster-rcnn/caffe-fast-rcnn
# https://github.com/rbgirshick/py-faster-rcnn/issues/237
# https://github.com/rbgirshick/py-faster-rcnn/issues/509
RUN git remote add caffe https://github.com/BVLC/caffe.git
RUN git fetch caffe
RUN git merge -X theirs caffe/master
RUN sed -i '29d' include/caffe/layers/python_layer.hpp
RUN mkdir build
WORKDIR /root/py-faster-rcnn/caffe-fast-rcnn/build
RUN cmake ..
RUN make all -j16
RUN make pycaffe -j16

# Remaining dependency
RUN apt-get install python-tk -y

# Solve the lib1394 issue
RUN ln /dev/null /dev/raw1394

WORKDIR /root/py-faster-rcnn/

# Get the model files
RUN ./data/scripts/fetch_faster_rcnn_models.sh
RUN ./data/scripts/fetch_imagenet_models.sh

# Make the directory for our data
RUN mkdir data/VOCdevkit2007
