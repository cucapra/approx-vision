FROM kaixhin/cuda-caffe:7.0
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
RUN git clone --recursive https://github.com/rbgirshick/py-faster-rcnn.git

WORKDIR /root/py-faster-rcnn

# Build Faster RCNN
WORKDIR /root/py-faster-rcnn/lib
RUN make

WORKDIR /root/py-faster-rcnn/caffe-fast-rcnn
RUN mkdir build
WORKDIR /root/py-faster-rcnn/caffe-fast-rcnn/build
RUN cmake ..
RUN make -j4 && make pycaffe

