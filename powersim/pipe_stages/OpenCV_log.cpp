//
// OpenCV Load 3 Channel Image Benchmark
//
// This file loads a three channel image file and nothing else.
// Its primary purpose is to have its profile subtracted
// from each of the OpenCV stage profiles.

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

int main(int argc, char** argv )
{

  Mat in_img, out_img;

  in_img = imread("../benchmark_images/beer_hall.raw_3C.png");

  in_img.convertTo(in_img,CV_32F);
  in_img = in_img + 1;
  cv::log(in_img,in_img);
  cv::convertScaleAbs(in_img,in_img);
  cv::normalize(in_img,in_img,0,255,cv::NORM_MINMAX);

  //imwrite("out.png",in_img);

  return 0;

}
