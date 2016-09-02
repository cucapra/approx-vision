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

  Mat lookUpTable(1, 256, CV_8U);
  uchar* p = lookUpTable.data;
  for( int i = 0; i < 256; ++i)
      p[i] = 255-i;

  LUT(in_img, lookUpTable, out_img);

  //imwrite("out.png",in_img);

  return 0;

}
