/////////////////////////////////////////////////////////////////////////
//  Image Processing Pipeline Simulator
//
//    This code simulates the image processing pipeline from sensor to 
//    output image. Input is expected to be raw camera pixel values which
//    are interpreted as irradiance values. The current version receives
//    a single 12 bit raw file and produces images at various stages
//    within the image processing pipeline by using the LibRaw and OpenCV
//    libraries.
//  
//  Input:
//    Single raw image file (tested with Nikon .NEF)
//  
//  Output:
//    <infilename>.raw_12B_3C.tiff: 
//      The raw data in uncompressed 12 bit, represented with 3 rgb channels
//    <infilename>.bgr_12B_3C.tiff:
//      The result of OpenCV demosiacing (debayering)
//    <infilename>.dcraw_12B.tiff:
//      The result of LibRaw dcraw processing (open source image processing) 
//    <infilename>.dcraw_12B.jpg:
//      The result of LibRaw dcraw, but compressed as a jpg
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

  // Place the raw_image into 3 channel data structure
  RawProcessor.raw2image();

 
  ////////////////////////////////////////////////////////////////////
  //  Raw data  //////////////////////////////////////////////////////

  // Extract raw data stored in 16 bit, 4 channel image
  Mat raw_12B_4C = Mat(
    imgdata.sizes.height,
    imgdata.sizes.width,
    CV_16UC4,
    imgdata.image
    );

  // Split the image into different channels
  vector<Mat> rg1bg2Channels;
  split(raw_12B_4C, rg1bg2Channels);

  // Add the two green channels together
  Mat green_sum = rg1bg2Channels[1] + rg1bg2Channels[3];
  Mat zeros     = Mat::zeros(Size(raw_12B_4C.cols, raw_12B_4C.rows), CV_16UC1);

  // Merge the three channels into the final raw image
  // NOTE: OpenCV holds images in BGR order rather than more common RGB
  Mat         raw_12B_3C;
  vector<Mat> bgrChannels;
  // Blue channel
  bgrChannels.push_back(rg1bg2Channels[2]);
  // Green channel
  bgrChannels.push_back(        green_sum);
  // Red channel
  bgrChannels.push_back(rg1bg2Channels[0]);
  // Merge channels
  merge(bgrChannels, raw_12B_3C);

  // Scale up 12 bit raw elements to fit 16 bit mat entries
  // (multiply all elements  by 2^4)
  raw_12B_3C.convertTo(raw_12B_3C, CV_16UC1, 16);

  // Write the raw 12 bit 3 channel representation to file
  imwrite( (std::string(in_path)+".raw_12B_3C.tiff").c_str(), raw_12B_3C);


  ////////////////////////////////////////////////////////////////////
  //  Demosiaced  ////////////////////////////////////////////////////

  // Raw data stored in 16 bit, 1 channel image
  Mat raw_12B_1C = Mat(
    imgdata.sizes.raw_height,
    imgdata.sizes.raw_width,
    CV_16UC1,
    imgdata.rawdata.raw_image,
    Mat::AUTO_STEP
    );

  // Scale up 12 bit raw elements to fit 16 bit mat entries
  // (multiply all elements  by 2^4)
  raw_12B_1C.convertTo(raw_12B_1C, CV_16UC1, 16);

  // Demosiac (debayer) the image
  Mat bgr_12B_3C;
  cvtColor(raw_12B_1C, bgr_12B_3C, CV_BayerGR2RGB);
  
  // Write the demosiaced 12 bit 3 channel representation to file
  imwrite( (std::string(in_path)+".bgr_12B_3C.tiff").c_str(), bgr_12B_3C);

  // Free the RawProcessor to reload image for dcraw processing
  RawProcessor.recycle();


  ////////////////////////////////////////////////////////////////////
  //  Dcraw processed  ///////////////////////////////////////////////

  // Reload image (needs to be re-initialized for dcraw processing)
  RawProcessor.open_file(in_path);
  RawProcessor.unpack();

  // Dcraw settings
  imgdata.params.use_camera_wb = 1;

  // Process raw data using LibRaw implementation of dcraw
  int check = RawProcessor.dcraw_process();
  libraw_processed_image_t *image_ptr = RawProcessor.dcraw_make_mem_image(&check);

  // Convert the raw image to an OpenCV Mat
  //   Note: The argument CV_8UC3 parses as follows
  //   CV_<bit-depth>{U|S|F}C(<number_of_channels>)
  Mat dcraw_12B;
  dcraw_12B = Mat(Size(image_ptr->width, image_ptr->height), 
    CV_8UC3, image_ptr->data, Mat::AUTO_STEP);

  // Convert from the default RGB color space to the correct BGR color space
  //   This conversion is trivial and therefore can be done
  //   even in pipelines where no color mapping is requested.
  cv::cvtColor(dcraw_12B, dcraw_12B, CV_RGB2BGR);
 
  // Write the dcrawed representation to file
  imwrite( (std::string(in_path)+".dcraw_12B.tiff").c_str(), dcraw_12B);

 
  ////////////////////////////////////////////////////////////////////
  //  Dcraw processed and compressed  ////////////////////////////////

  // Print the compressed version of the dcraw processed output
  imwrite( (std::string(in_path)+".dcraw_12B.jpg").c_str(), dcraw_12B);

  // Free RawProcessor
  RawProcessor.recycle();

  return 0;
}
