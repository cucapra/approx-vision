
#include "Halide.h"
#include "../common/halide_stages.h"
#include "../common/ImgPipeConfig.h"
#include "../common/LoadCamModel.h"
#include "../common/MatrixOps.h"
#include <stdio.h>
#include <math.h>
#include "halide_image_io.h"
#include <opencv2/opencv.hpp>

// This is the full reverse pipeline, converting sRGB to raw

// Reversible pipeline function
int main(int argc, char **argv) {

  using namespace std;  

  // Inform user of usage method
  if ( argc != 3 ) 
  {
      printf("usage: \n./convert in/data/name.bin out/data/name.bin \n");
      return -1; 
  }

  // Input data directory
  const char * in_data_path   = argv[1];
  // Output data directory
  const char * out_data_path  = argv[2];

  ///////////////////////////////////////////////////////////////////////////////////////
  // Import and format model data

  // Declare model parameters
  vector<vector<float>> Ts, Tw, TsTw;
  vector<vector<float>> ctrl_pts, weights, coefs;
  vector<vector<float>> rev_tone;

  // Load model parameters from file
  // NOTE: Ts, Tw, and TsTw read only forward data
  // ctrl_pts, weights, and coefs are either forward or backward
  // tone mapping is always backward
  // This is due to the the camera model format
  Ts        = get_Ts       (cam_model_path);
  Tw        = get_Tw       (cam_model_path, wb_index);
  TsTw      = get_TsTw     (cam_model_path, wb_index);
  ctrl_pts  = get_ctrl_pts (cam_model_path, num_ctrl_pts, 0);
  weights   = get_weights  (cam_model_path, num_ctrl_pts, 0);
  coefs     = get_coefs    (cam_model_path, num_ctrl_pts, 0);
  rev_tone  = get_rev_tone (cam_model_path);

  // Take the transpose of the color map and white balance transform for later use
  vector<vector<float>> TsTw_tran = transpose_mat (TsTw);

  // take the inverse of TsTw_tran
  TsTw_tran = inv_3x3mat(TsTw_tran);

  using namespace Halide;
  using namespace Halide::Tools;
  using namespace cv;

  // Convert control points to a Halide image
  int width  = ctrl_pts[0].size();
  int length = ctrl_pts.size();
  Image<float> ctrl_pts_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      ctrl_pts_h(x,y) = ctrl_pts[y][x];
    }
  }
  // Convert weights to a Halide image
  width  = weights[0].size();
  length = weights.size();
  Image<float> weights_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      weights_h(x,y) = weights[y][x];
    }
  }
  // Convert the reverse tone mapping function to a Halide image
  width  = 3;
  length = 256;
  Image<float> rev_tone_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_tone_h(x,y) = rev_tone[y][x];
    }
  }
 
  ///////////////////////////////////////////////////////////////////////////////////////
  // Establish IO
  char val, label;
  fstream infile(in_data_path);
  //fstream infile("/work/mark/datasets/cifar-10/cifar-10-batches-bin/test_batch.bin");
  if(!infile) {
    printf("Input file not found: %s\n", in_data_path);
    return 1;
  }  

  fstream outfile;
  outfile.open(out_data_path,fstream::out);

  // Declare image handle variables
  Var x, y, c;

  // Define input image 

  ///////////////////////////////////////////////////////////////////////////////////////
  // Process Images
 

  for (int i=0; i<1; i++) { //i<10000

    int width  = 32;
    int height = 32;

    // Define input data
    unsigned char* input_data = new unsigned char[3 * width * height];
    buffer_t input_buffer = make_char_buffer( input_data );
    Mat input_cv(height,width,CV_8UC3,input_data);
    Image<uint8_t> input_halide(input_buffer);


    // Read in label
    infile.read(&val,1);
    label = val;
    
    for (int c=0; c<3; c++) { 
      for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
          infile.read(&val,1);
          //in_data[(c*32*32) + (y*32) + x] = (unsigned char)val;
          input_data[(c) + (y*3*width) + (x*3)] = (uint8_t)val;
          //input(x,y,c) = (unsigned char)val;
        }
      }
    }

    

    ///////////////////////////////////////////////////////////////////////////////////////
    // Halide Funcs for camera pipeline

    // Cast input to float and scale according to its 8 bit input format
    Func scale   = make_scale(&input_halide);
    Func descale = make_descale(&scale);


    ImageParam input_test{UInt(8), 3, "input_test"};
    Func nothing("nothing");
      nothing(x,y,c) = input_test(x,y,c);

    ///////////////////////////////////////////////////////////////////////////////////////
    // Realization, data buffers, and OpenCV functions


    // Define output data
    unsigned char* output_data = new unsigned char[3 * width * height];
    buffer_t output_buffer = make_char_buffer( output_data );
    Mat output_cv(height,width,CV_8UC3,output_data);
    Image<uint8_t> output_halide(output_buffer);



    // Realization
    //Image<uint8_t> output;
    // backward pipeline
    
    
    scale.output_buffer().set_stride(0,3).set_stride(1,3*32).set_stride(2,1);
    descale.output_buffer().set_stride(0,3).set_stride(1,3*32).set_stride(2,1);
    nothing.output_buffer().set_stride(0,3).set_stride(1,3*32).set_stride(2,1);

    nothing(&input_buffer, &output_buffer);

/*
    output_halide = nothing.realize(input_halide.width(), 
                             input_halide.height(), 
                             input_halide.channels());
*/

//    output_halide = descale.realize(input_halide.width(), 
//                             input_halide.height(), 
//                             input_halide.channels());

//    delete[] input_data;

    ////////////////////////////////////////////////////////////////////////
    // Save the output
//    save_image(input_halide, "input_halide.png");
//    save_image(output_halide, "output_halide.png");
//
//    imwrite("input_cv.png",input_cv);
//    imwrite("output_cv.png",output_cv);

    // Read in label
    val = label;
    outfile.write(&val,1);
    
    for (int c=0; c<3; c++) { 
      for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
          val = input_data[(c) + (y*3*width) + (x*3)];
          outfile.write(&val,1);
        }
      }
    }

  }

  return 0;
}

