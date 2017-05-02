/////////////////////////////////////////////////////////////////////////
//
//  This script is built to preprocess raw files for both the Halide and 
//  Matlab reversible imaging pipelines. Raw files are used as input
//  to the forward imaging pipelines, and as reference output for the 
//  the backward imaging pipelines. Since neither Halide nor Matlab can
//  read raw file formats (such as Nikon .NEF), LibRaw is used to 
//  receive raw data, and OpenCV is used to scale, preprocess as needed,
//  and write readable files.
//
//  Input:
//    Filepath to raw image input file (.NEF)
//    Filepath to png image output file
//    Number of bits used to represent raw image input file
//    
//  Output:
//    Saves scaled 32 bit float point 3 channel image to output file path
//
//
/////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 4 )
  {
      printf("usage: \n./RawToPng <path/to/input/image> <path/to/output/image> <raw file bitdepth>\n");
      return -1;
  }

  // LibRaw raw data extraction 
  // Establish use of LibRaw
  LibRaw RawProcessor;
  #define imgdata RawProcessor.imgdata

  // Function failure flag
  int ret;

  // Path to input and output
  const char * in_path = argv[1];
  const char * out_path = argv[2];

  // Bitdepth of raw input
  int num_raw_bits = atoi(argv[3]);

  // Read in raw image with LibRaw
  if ((ret = RawProcessor.open_file(in_path)) != LIBRAW_SUCCESS)
  {
      fprintf(stderr, in_path, libraw_strerror(ret));
      return -1; 
  }

  // Unpack the raw image, storing it in imgdata.rawdata.raw_image
  if ((ret = RawProcessor.unpack()) != LIBRAW_SUCCESS)
  {
      return -1; 
  }
 
  // Extract raw data stored in 16 bit, 1 channel image
  Mat raw_1C = Mat(
    imgdata.sizes.raw_height,
    imgdata.sizes.raw_width,
    CV_16UC1,
    imgdata.rawdata.raw_image
  );

  // Scale the data to fit the 16 bit representation
  int scale = 1 << (16-num_raw_bits);
  raw_1C = raw_1C * scale;

  Mat raw_3C = Mat(
    imgdata.sizes.raw_height, 
    imgdata.sizes.raw_width, 
    CV_32FC3  
  );
  vector<Mat> three_channels;
  split(raw_3C, three_channels);

  // 1 channel to 3 channel
  // Note: OpenCV stores as BGR not RGB
  float scale_float = pow(2, 16);
  int color_channel = 0;
  for (int y=0; y<raw_3C.rows; y++) {
    for (int x=0; x<raw_3C.cols; x++) {
      // BGR
      if (y % 2 == 0) { // even row
        if (x % 2 == 0) { color_channel = 2; } // red
        else {            color_channel = 1; } // green
      }
      else { // odd row
        if (x % 2 == 0) { color_channel = 1; } // green
        else {            color_channel = 0; } // blue
      }
      three_channels[color_channel].at<float>(y, x) 
            = (float)raw_1C.at<unsigned short>(y, x);
    }
  }
  merge(three_channels, raw_3C);

  imwrite((std::string(out_path)).c_str(), raw_3C);

  return 0;
}
