
# Makefile test

all:
	#g++ lesson_12_using_the_gpu.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lraw -lOpenCL -lrt -lm -o Lesson12 -std=c++11
	g++ CIFAR_Backpipe.cpp LoadCamModel.cpp MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lraw -lm -o convert -std=c++11
