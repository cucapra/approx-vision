/////////////////////////////////////////////////////////////
//  Image Processing Pipeline Simulator
//
//  Built using multiple image processing stage simulators
//  which leverage the OpenCV library
//
//  Open CV docs: http://docs.opencv.org/3.1.0/
//  
//  Author: Mark Buckler
//
/////////////////////////////////////////////////////////////

#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 2 )
  {
      printf("usage: ImgPipe <Input_Image_Path>\n");
      return -1;
  }

  // Read in image
  Mat in_img;
  in_img = imread( argv[1], 1 );
  if ( !in_img.data )
  {
      printf("No image data \n");
      return -1;
  }

  // Process image with desired stages
  Mat sense_o, amp_o, adc_o, demos_o, denos_o, white_o;
  Mat color_o, comp_o, vis_o;

  ///////////////////////////////////////////////////////////
  // Mixed Signal Sensing
  ///////////////////////////////////////////////////////////

  // Photodiode sensing 
  // I: Irradiance, O: Voltage
  sense_o = in_img;
  
  // Amplifier 
  // I: Voltage, O: Voltage
  amp_o = sense_o;

  // ADC
  // I: Voltage, O: Quantized digital values
  adc_o = amp_o;

  ///////////////////////////////////////////////////////////
  // Image Sensor Processing (ISP)
  ///////////////////////////////////////////////////////////

  // Demosiacing
  // cv::demosaicing
  demos_o = adc_o;

  // Denoising
  // cv::denoise_TVL1, cv::fastNlMeansDenoising, cv::fastNlMeansDenoisingColored
  denos_o = demos_o;
  
  // White balancing
  // cv::balanceWhite
  white_o = denos_o;
  
  // Color mapping
  cvtColor( white_o, color_o, CV_BGR2GRAY );

  // Compression
  comp_o = color_o;
  imwrite( "../../imgs/pipe_output.jpg", color_o );

  ///////////////////////////////////////////////////////////
  // CPU/GPU/VPU Processing
  ///////////////////////////////////////////////////////////

  // Actual computer vision application. Blank for now

  // Display resulting image 
  namedWindow("Display Image", WINDOW_AUTOSIZE );
  imshow("Display Image", comp_o);
  waitKey(0);

  return 0;

}
