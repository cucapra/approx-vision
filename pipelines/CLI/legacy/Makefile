
# Makefile for pipeline compilation

version = '1'

all:
	g++ pipeline_V$(version).cpp CameraModel.cpp PipelineUtil.cpp ../common/pipe_stages.cpp ../common/LoadCamModel.cpp ../common/MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` `pkg-config opencv --cflags --libs` -lpthread -ldl -lm -o pipeline_V$(version).o -std=c++11

clean:
	$(RM) *.o
