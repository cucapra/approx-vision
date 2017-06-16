
# Makefile test

all:
	g++ RawToPng.cpp /approx-vision/pipelines/CLI/core/CameraModel.cpp /approx-vision/pipelines/CLI/core/PipelineUtil.cpp /approx-vision/pipelines/common/pipe_stages.cpp /approx-vision/pipelines/common/LoadCamModel.cpp /approx-vision/pipelines/common/MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` `pkg-config opencv --cflags --libs` -lpthread -ldl -lm -lraw -o RawToPng -std=c++11
	g++ -o Resize Resize.cpp `pkg-config opencv --cflags --libs` -lraw -lm 


