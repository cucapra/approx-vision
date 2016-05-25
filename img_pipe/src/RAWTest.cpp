// Test of RAW loading

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>


CV_EXPORTS_W int load_image(const char * path, cv::Mat & output)
{
  LibRaw RawProcessor;

  int ret;

  #define imdat RawProcessor.imgdata

  //imdat.params.use_camera_wb = 1;
  //imdat.params.use_auto_wb = 0;

  imdat.params.no_interpolation = 1; // disables demosaic
  imdat.params.no_auto_scale    = 1; // disables scaling from camera maximum to 64 k
  imdat.params.no_auto_bright   = 1; // disables auto brighten

  if ((ret = RawProcessor.open_file(path)) != LIBRAW_SUCCESS)
  {
      fprintf(stderr, path, libraw_strerror(ret));
      return -1;
  }
  if ((ret = RawProcessor.unpack()) != LIBRAW_SUCCESS)
  {
      return -1;
  }

  int check = RawProcessor.dcraw_process();
  libraw_processed_image_t *image_ptr = RawProcessor.dcraw_make_mem_image(&check);

  output = cv::Mat(cv::Size(image_ptr->width, image_ptr->height), CV_8UC3, image_ptr->data, cv::Mat::AUTO_STEP);
  cv::cvtColor(output, output, 4);
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

  /*
  // Read in image (OpenCV version)
  in_img = imread( argv[1], 1 );
  if ( !in_img.data )
  {
      printf("No image data \n");
      return -1;
  }
  */

  // Read in image (libraw version)
  load_image(argv[1], in_img);

  // Color mapping
  //cvtColor( white_o, color_o, CV_BGR2GRAY );

  // Compression
  //imwrite( "../../imgs/pipe_output.jpg", color_o );

  // Display resulting image 
  namedWindow("Display Image", WINDOW_AUTOSIZE );
  imshow("Display Image", in_img);
  waitKey(0);

  return 0;

}



/*
Image::Image(const char* file) {
  LibRaw rawProcessor;
  libraw_processed_image_t *tmpImg;
 
  if(rawProcessor.open_file(file) != LIBRAW_SUCCESS) {
    printf("Failed to open with libraw\n");
  }
  else {
    rawProcessor.unpack();
    //rawProcessor.raw2image();

    //rawProcessor.imgdata.params.no_interpolation = 1;
    //rawProcessor.imgdata.params.no_auto_scale = 1;
    //rawProcessor.imgdata.params.no_auto_bright = 1;

    //int check = rawProcessor.dcraw_process();
    //tmpImg = rawProcessor.dcraw_make_mem_image(&check);
 
    // Init image
    Img = cv::Mat(rawProcessor.imgdata.sizes.width, rawProcessor.imgdata.sizes.height, CV_16UC3, rawProcessor.imgdata.rawdata.raw_image);
 
    // Init datas
    cameraModel = (std::string) rawProcessor.imgdata.idata.make + "-" + rawProcessor.imgdata.idata.model;
    shutterTime = rawProcessor.imgdata.other.shutter;
  }

  // Free rawProcessor
  rawProcessor.recycle();
}
*/
