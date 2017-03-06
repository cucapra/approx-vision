
all:
	g++ -o Halide_lut Halide_lut.cpp LoadCamModel.cpp MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lm -std=c++11 
	g++ -o Halide_gamut_rbf Halide_gamut_rbf.cpp LoadCamModel.cpp MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lm -std=c++11 
	g++ -o Halide_wb+transform Halide_wb+transform.cpp LoadCamModel.cpp MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lm -std=c++11 
	g++ -o Halide_3C_dummy Halide_3C_dummy.cpp LoadCamModel.cpp MatrixOps.cpp -g -I ~/packages/Halide/include/ -I ~/packages/Halide/tools -L ~/packages/Halide/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lm -std=c++11 
	g++ -o OpenCV_1C_dummy OpenCV_1C_dummy.cpp `pkg-config opencv --cflags --libs` -lm 
	g++ -o OpenCV_3C_dummy OpenCV_3C_dummy.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_wb+transform OpenCV_wb+transform.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_debayer OpenCV_debayer.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_denoise OpenCV_denoise.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_jpeg OpenCV_jpeg.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_log OpenCV_log.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_lut_1C OpenCV_lut_1C.cpp `pkg-config opencv --cflags --libs` -lm
	g++ -o OpenCV_lut_3C OpenCV_lut_3C.cpp `pkg-config opencv --cflags --libs` -lm
