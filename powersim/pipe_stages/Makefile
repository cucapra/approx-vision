
all:
	g++ -o OpenCV_1C_dummy OpenCV_1C_dummy.cpp `pkg-config opencv --cflags --libs` -lm 
	g++ -o OpenCV_3C_dummy OpenCV_3C_dummy.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o Halide_3C_dummy Halide_3C_dummy.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lm -std=c++11 
	g++ -o OpenCV_debayer OpenCV_debayer.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_denoise OpenCV_denoise.cpp `pkg-config opencv --cflags --libs` -lm
