//
// OpenCV Demosiac Benchmark
//
// This file loads a single channel image file and then performs
// demosiacing

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

int main(int argc, char** argv )
{

  Mat in_img;

  in_img = imread("../benchmark_images/beer_hall.raw_1C.png",CV_16UC1);

  // Demosiac (debayer) the image
  Mat bgr_3C;
  //cvtColor(raw_1C, bgr_3C, CV_BayerRG2RGB);
  cvtColor(in_img, bgr_3C, CV_BayerGB2RGB);

  // Write the demosiaced 3 channel representation to file
  imwrite("out.png", bgr_3C);


  return 0;

}
