# Start with both Halide and OpenCV
FROM mbuckler/cam-pipes 
WORKDIR /root/packages

# Install image converter for JPEG/PNG conversion
RUN apt-get install python-pip python-dev build-essential -y
RUN pip install --upgrade pip
RUN pip install --upgrade virtualenv
RUN pip install Pillow

# Install dependencies for OpenCV benchmarks
RUN pip install ipdb
RUN apt-get install python-matplotlib -y

# Install extra processing dependencies
RUN pip install --upgrade pip
RUN pip install psutil
RUN apt-get install rsync

# Install LibRaw
RUN wget https://www.libraw.org/data/LibRaw-0.18.2.tar.gz
RUN tar xzvf LibRaw-0.18.2.tar.gz
WORKDIR /root/packages/LibRaw-0.18.2
RUN ./configure
RUN make
RUN make install
RUN /sbin/ldconfig -v

# Install scheduling libraries
RUN pip install futures
RUN pip install findtools

# Reset our working directory 
WORKDIR /
