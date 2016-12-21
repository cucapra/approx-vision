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

# Clone the automation script and copy it to caffe
WORKDIR /root
RUN git clone https://github.com/mbuckler/lenet-script.git
RUN cp lenet-script/run-lenet.py /root/caffe/

# Start user in the caffe directory
WORKDIR /root/caffe
