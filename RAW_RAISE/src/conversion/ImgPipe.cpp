/////////////////////////////////////////////////////////////////////////
//
//  Image Processing Pipeline Simulator
//
//    This code simulates the image processing pipeline from sensor to 
//    output image. Input is expected to be raw camera pixel values which
//    are interpreted as irradiance values. The current version receives
//    a single raw file and produces images at various stages
//    within the image processing pipeline by using the LibRaw and OpenCV
//    libraries.
//  
//  Input:
//    Single raw image file (tested with Nikon .NEF)
//  
//  Output:
//    <infilename>.raw_3C.png: 
//      The raw data in uncompressed 16 bit, represented with 3 rgb channels
//    <infilename>.bgr_3C.png:
//      The result of OpenCV demosiacing (debayering)
//    <infilename>.dcraw.png:
//      The result of LibRaw dcraw processing (open source image processing) 
//    <infilename>.dcraw.jpg:
//      The result of LibRaw dcraw, but compressed as a jpg
//
//  LibRaw docs: http://www.libraw.org/docs/API-CXX-eng.html
//  OpenCV docs: http://docs.opencv.org/3.1.0/
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
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 3 )
  {
      printf("usage: ImgPipe path/to/image <raw file bitdepth>\n");
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
  int scale        = 1 << (16-num_raw_bits);

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

  // 3 Channel Scaled Raw ////////////////////////////////////////////

  // Place the raw_image into 4 channel image
  RawProcessor.raw2image();

  // Extract raw data stored in 16 bit, 4 channel image
  //   NOTE: Data here is taken from imgdata.image instead of 
  //   imgdata.raw_image. This is OK because no processing has
  //   been performed, and is useful to enable access to data in 
  //   the 4 channel format.
  Mat raw_4C = Mat(
    imgdata.sizes.height,
    imgdata.sizes.width,
    CV_16UC4,
    imgdata.image
    );

  // Split the image into different channels
  vector<Mat> rg1bg2Channels;
  split(raw_4C, rg1bg2Channels);

  // Add the two green channels together
  Mat green_sum = rg1bg2Channels[1] + rg1bg2Channels[3];
  Mat zeros     = Mat::zeros(Size(raw_4C.cols, raw_4C.rows), CV_16UC1);

  // Merge the three channels into the final raw image
  //   NOTE: OpenCV holds images in BGR order rather than more common RGB
  Mat         raw_3C;
  vector<Mat> bgrChannels;
  // Blue channel
  bgrChannels.push_back(rg1bg2Channels[2]);
  // Green channel
  bgrChannels.push_back(        green_sum);
  // Red channel
  bgrChannels.push_back(rg1bg2Channels[0]);
  // Merge channels
  merge(bgrChannels, raw_3C);

  // Scale up raw elements to fit 16 bit mat entries
  raw_3C.convertTo(raw_3C, CV_16UC1, scale);

  // Write the raw 3 channel representation to file
  imwrite( (std::string(in_path)+".raw_3C.png").c_str(), raw_3C);


  ////////////////////////////////////////////////////////////////////
  //  Demosiaced  ////////////////////////////////////////////////////

  // Raw data stored in 16 bit, 1 channel image
  Mat raw_1C = Mat(
    imgdata.sizes.raw_height,
    imgdata.sizes.raw_width,
    CV_16UC1,
    imgdata.rawdata.raw_image,
    Mat::AUTO_STEP
    );

  // Scale up raw elements to fit 16 bit mat entries
  raw_1C.convertTo(raw_1C, CV_16UC1, scale);

  // Demosiac (debayer) the image
  Mat bgr_3C;
  cvtColor(raw_1C, bgr_3C, CV_BayerRG2RGB);
  
  // Write the demosiaced 3 channel representation to file
  imwrite( (std::string(in_path)+".bgr_3C.png").c_str(), bgr_3C);

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
  Mat dcraw;
  dcraw = Mat(Size(image_ptr->width, image_ptr->height), 
    CV_8UC3, image_ptr->data, Mat::AUTO_STEP);

  // Convert from the default RGB color space to the correct BGR color space
  //   This conversion is trivial and therefore can be done
  //   even in pipelines where no color mapping is requested.
  cv::cvtColor(dcraw, dcraw, CV_RGB2BGR);
 
  // Write the dcrawed representation to file
  imwrite( (std::string(in_path)+".dcraw.png").c_str(), dcraw);

 
  ////////////////////////////////////////////////////////////////////
  //  Dcraw processed and compressed  ////////////////////////////////

  // Print the compressed version of the dcraw processed output
  imwrite( (std::string(in_path)+".dcraw.jpg").c_str(), dcraw);

  // Free RawProcessor
  RawProcessor.recycle();

  return 0;
}
