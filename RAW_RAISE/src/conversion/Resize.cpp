#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 5 )
  {
      printf("usage: \n./Resize <path/to/input/image> <path/to/output/image> <input image type> <width> <height>\n");
      return -1;
  }

  const char * in_path = argv[1];
  const char * out_path = argv[2];

  int width = atoi(argv[3]);
  int height = atoi(argv[3]);

  Mat image = imread(in_path, CV_LOAD_IMAGE_COLOR);

  resize(image, image, Size(width, height));

  imwrite((std::string(out_path)).c_str(), image);

  return 0;
}