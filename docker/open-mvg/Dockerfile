FROM freakthemighty/openmvg
WORKDIR /root

# Install basics
RUN apt-get update && apt-get install -y \
	build-essential \
	git \
	cmake \
	wget \
	python

# Clone benchmark
RUN git clone https://github.com/mbuckler/SfM_quality_evaluation.git

# Install vim for my sanity
RUN apt-get install vim -y

# Re-set working directory
WORKDIR /root/SfM_quality_evaluation
