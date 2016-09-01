//
// OpenCV Load 1 Channel Image Benchmark
//
// This file loads a single channel image file and nothing else.
// Its primary purpose is to have its profile subtracted
// from each of the OpenCV stage profiles.

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

int main(int argc, char** argv )
{

  Mat in_img;

  in_img = imread("../benchmark_images/beer_hall.raw_1C.png",CV_16UC1);

  imwrite("out.png",in_img);

  return 0;

}
