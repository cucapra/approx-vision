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

  int dim(256);

  Mat lut(1, &dim, CV_8UC3);
  for( int i = 0; i < 256; ++i) {
    lut.at<Vec3b>(i)[0]= 255-i;   // first channel  (B)
    lut.at<Vec3b>(i)[1]= 255-i/2; // second channel (G)
    lut.at<Vec3b>(i)[2]= 255-i/3; // ...            (R)     
  }
  LUT(in_img, lut, out_img);

  //imwrite("out.png",in_img);

  return 0;

}
