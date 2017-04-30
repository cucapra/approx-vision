/////////////////////////////////////////////////////////////////////////
//
//  Raw Image Preprocessor
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
//    Single raw image file (tested with 14 bit Nikon .NEF)
//  
//  Output:
//    <infilepath>/<infilename>.raw_1C.tiff:
//      A scaled version of the raw input, stored in a 1 channel tiff
//      Used by the Matlab pipeline
//    <infilepath>/<infilename>.demos_3C.png: 
//      Scaled and demosiaced raw input, stored in a 3 channel png
//      Used by the Halide pipeline
//
//  Author: Mark Buckler
//
/////////////////////////////////////////////////////////////////////////
//
// NOTE: You may need to change the demosaic pattern. It is different
//       for every camera, so be sure to check.
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
  if ( argc != 5 )
  {
      printf("usage: \n./RawPreproc path/to/image <raw file bitdepth> <out image width> <out image height>\n");
      return -1;
  }

  ////////////////////////////////////////////////////////////////////
  //  LibRaw raw data extraction  ////////////////////////////////////

  // Establish use of LibRaw
  LibRaw RawProcessor;
  #define imgdata RawProcessor.imgdata

  // Function failure flag
  int ret;

  // Path to input
  const char * in_path = argv[1];

  // Bitdepth of raw input
  int num_raw_bits = atoi(argv[2]);
  cout << num_raw_bits << endl;
  // Out image width, height
  int out_image_width = atoi(argv[3]);
  int out_image_height = atoi(argv[4]);

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
 
  ////////////////////////////////////////////////////////////////////
  //  Raw data for Matlab ////////////////////////////////////////////

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
    imgdata.sizes.raw_height / 2, 
    imgdata.sizes.raw_width  / 2, 
    CV_32FC3  
  );
  vector<Mat> three_channels;
  split(raw_3C, three_channels);

  // demosaic 1 channel to 3 channel
  // Note: OpenCV stores as BGR not RGB
  float scale_float = pow(2, 16);
  int row, col = 0;
  unsigned short b, g1, g2, r = 0;
  for (int y=0; y<raw_3C.rows; y++) {
    for (int x=0; x<raw_3C.cols; x++) {
      row = y * 2;
      col = x * 2;
      b  = raw_1C.at<unsigned short>(row, col);
      g1 = raw_1C.at<unsigned short>(row + 1, col);
      g2 = raw_1C.at<unsigned short>(row, col + 1);
      r  = raw_1C.at<unsigned short>(row + 1, col + 1);

      // BGR
      three_channels[0].at<float>(y, x) = (float) b / scale_float;
      three_channels[1].at<float>(y, x) = (float)((float) g1 + (float) g2) / scale_float;
      three_channels[2].at<float>(y, x) = (float) r / scale_float;

    }
  }
  merge(three_channels, raw_3C);

  // resize image
  Size out_size = cv::Size(out_image_width, out_image_height);
  Mat out_image = Mat(out_image_height, out_image_width, CV_32FC3);
  cv::resize(raw_3C, out_image, out_size);

  // Write the raw 1 channel representation to file
  imwrite( (std::string(in_path)+".3C.png").c_str(), out_image);

  return 0;
}
