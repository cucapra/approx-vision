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

# Reset our working directory 
WORKDIR /
