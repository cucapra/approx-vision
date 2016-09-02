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

  Mat in_img;

//  in_img = imread("../benchmark_images/beer_hall.raw_3C.png",CV_16UC3);
  in_img = imread("../benchmark_images/beer_hall.raw_3C.png");

  Mat in8bit(in_img.rows,in_img.cols,CV_8UC3);
  Mat out_img(in_img.rows,in_img.cols,CV_8UC3);

  in_img.convertTo(in8bit,CV_8UC3);

  fastNlMeansDenoisingColored(in8bit,out_img);

  //imwrite("out.png",out_img);

  return 0;

}
