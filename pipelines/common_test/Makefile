
# Makefile for pipeline compilation

version = '1'

all:
	g++ pipeline_V$(version).cpp ../common_test/pipe_stages.cpp ../common_test/LoadCamModel.cpp ../common_test/MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` `pkg-config opencv --cflags --libs` -lpthread -ldl -lm -o pipeline_V$(version).o -std=c++11

clean:
	$(RM) *.o
