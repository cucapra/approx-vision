/////////////////////////////////////////////////////////////////////////
//  Image Processing Pipeline Simulator
//
//    This code simulates the image processing pipeline from sensor to 
//    output image. Input is expected to be raw camera pixel values which
//    are interpreted as irradiance values. The current version receives
//    a single file and produces processed or unprocessed versions
//    (using LibRaw) which are then compressed or left uncompressed
//    (using OpenCV).
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

/////////////////////////////////////////////////////////////////////////
// Function to load and process input raw file
/////////////////////////////////////////////////////////////////////////

CV_EXPORTS_W int load_and_proc(const char * path, cv::Mat & output, int process)
{
  // Establish use of LibRaw
  LibRaw RawProcessor;
  #define imdat RawProcessor.imgdata

  // Function failure flag
  int ret;

  ///////////////////////////////////////////////////////////////////////
  // LibRaw dcraw settings
  ///////////////////////////////////////////////////////////////////////

  //   If no processing is to be done
  if (process == 0) {
    // Disable demosiac
    imdat.params.no_interpolation = 1;
    // Disable automatic brightening
    imdat.params.no_auto_bright   = 1;
    // Disable color scaling from camera maximum to 64k
    imdat.params.no_auto_scale    = 1;
  }
  //   If processing is to be done
  else if (process == 1) {
    // Use camera white balance (automatic version also available)
    imdat.params.use_camera_wb    = 1;
  }

  ///////////////////////////////////////////////////////////////////////

  // Read in raw image with LibRaw
  if ((ret = RawProcessor.open_file(path)) != LIBRAW_SUCCESS)
  {
      fprintf(stderr, path, libraw_strerror(ret));
      return -1;
  }
  if ((ret = RawProcessor.unpack()) != LIBRAW_SUCCESS)
  {
      return -1;
  }

  // Process the raw image according to dcraw settings specified above
  int check = RawProcessor.dcraw_process();
  libraw_processed_image_t *image_ptr = RawProcessor.dcraw_make_mem_image(&check);

  // Convert the raw image to an OpenCV Mat
  //   Note: The argument CV_8UC3 parses as follows
  //   CV_<bit-depth>{U|S|F}C(<number_of_channels>)
  output = cv::Mat(cv::Size(image_ptr->width, image_ptr->height), 
    CV_8UC3, image_ptr->data, cv::Mat::AUTO_STEP);

  // Convert from the default RGB color space to the correct BGR color space
  //   This conversion is trivial and therefore can be done
  //   even in pipelines where no color mapping is requested.
  cv::cvtColor(output, output, CV_RGB2BGR);
}


using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 2 )
  {
      printf("usage: ImgPipe <Input_Image_Path>\n");
      return -1;
  }

  // Declare mat for input image
  Mat in_img; 

  // Path to input file
  std::string in_path = std::string(argv[1]);

  ///////////////////////////////////////////////////////////////////////
  // Without Pipeline Processing
  ///////////////////////////////////////////////////////////////////////

  // Read in image and export the unprocessed version
  load_and_proc(in_path.c_str(), in_img, 0);

  // Print the compressed and uncompressed versions to file
  //imwrite( "../imgs/pipe_out_unproc.jpg", in_img );
  imwrite( (in_path+".unproc.jpg" ).c_str(), in_img );
  imwrite( (in_path+".unproc.tiff").c_str(), in_img );

  ///////////////////////////////////////////////////////////////////////
  // With Pipeline Processing
  ///////////////////////////////////////////////////////////////////////

  // Read in image and export the processed version
  load_and_proc(argv[1], in_img, 1);

  // Print the compressed and uncompressed versions to file
  imwrite( (in_path+".proc.jpg" ).c_str(), in_img );
  imwrite( (in_path+".proc.tiff").c_str(), in_img );


  return 0;
}
