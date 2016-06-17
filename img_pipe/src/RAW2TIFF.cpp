/////////////////////////////////////////////////////////////////////////
//
//  Raw to Tiff converter
//
//  This script is built to create a .tiff version of an input raw file
//  without performing any processing. It uses the LibRaw library to 
//  read raw files, and OpenCV to write the data to a lossless .tiff.
//  The resulting .tiff can then be more easily read by Matlab and 
//  other languages/systems.
//
//  NOTE: This script outputs in 16 bit form and does NOT perform any 
//  scaling. This means that the output may appear dark as your raw 
//  data will likely be represented with fewer than 16 bits.
//
//  Input:
//    Single raw image file (tested with 12 bit Nikon .NEF)
//  
//  Output:
//    <infilepath>/<infilename>.raw_1C.tiff: 
//      The raw data with lossless LZW tiff compression
//
//  Compile on Linux with the following:
//  g++ -o RAW2TIFF RAW2TIFF.cpp `pkg-config opencv --cflags --libs` -lraw -lm  
//
//  LibRaw docs: http://www.libraw.org/docs/API-CXX-eng.html
//  OpenCV docs: http://docs.opencv.org/3.1.0/
//
//  Author: Mark Buckler
//
/////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 2 )
  {
      printf("usage: ImgPipe <Input_Image_Path>\n");
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
  //  Raw data  //////////////////////////////////////////////////////

  // Extract raw data stored in 16 bit, 1 channel image
  Mat raw_1C = Mat(
    imgdata.sizes.raw_height,
    imgdata.sizes.raw_width,
    CV_16UC1,
    imgdata.rawdata.raw_image
    );

  // Write the raw 1 channel representation to file
  imwrite( (std::string(in_path)+".raw_1C.tiff").c_str(), raw_1C);

  return 0;
}
