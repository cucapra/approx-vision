//
// OpenCV Load 3 Channel Image Benchmark
//
// This file loads a three channel image and performs
// denoising

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

int main(int argc, char** argv )
{

  Mat in_img, out_img;

  in_img = imread("../benchmark_images/beer_hall.raw_3C.png",CV_16UC3);

  in_img.convertTo(in_img,CV_8U);

  fastNlMeansDenoising(in_img,out_img);

  imwrite("out.png",out_img);

  return 0;

}
