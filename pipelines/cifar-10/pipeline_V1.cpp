
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

    // Define input data
    unsigned char* in_data = new unsigned char[3 * 32 * 32];

    // Read in label
    infile.read(&val,1);
    label = val;
    //printf("label: %u\n",(unsigned char)label);
    
    for (int c=0; c<3; c++) { 
      for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
          infile.read(&val,1);
          //in_data[(c*32*32) + (y*32) + x] = (unsigned char)val;
          in_data[(c) + (y*3*32) + (x*3)] = (uint8_t)val;
          //input(x,y,c) = (unsigned char)val;
        }
      }
    }

    Mat cv_image(32,32,CV_8UC3,in_data);
    imwrite("cv_image.png",cv_image);

/*
    // Construct data buffer
    buffer_t input_buf  = {0};
    // Connect to image data
    input_buf.host      = in_data;
    // 8 bit image
    input_buf.elem_size = 1;
    // Set dimension sizes
    input_buf.extent[0] = 32; //x: width
    input_buf.extent[1] = 32; //y: height
    input_buf.extent[2] = 3;  //c: num colors
    // Set dimension strides for interleaved
    input_buf.stride[0] = 3;    
    input_buf.stride[1] = 3*32; 
    input_buf.stride[2] = 1;  
*/


    buffer_t buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.host = cv_image.data;
    buffer.elem_size = cv_image.elemSize1();
    buffer.extent[0] = cv_image.cols;
    buffer.extent[1] = cv_image.rows;
    buffer.extent[2] = cv_image.channels();
    buffer.stride[0] = cv_image.step1(1);
    buffer.stride[1] = cv_image.step1(0);
    buffer.stride[2] = 1;

    Image<uint8_t> input(buffer);


    ///////////////////////////////////////////////////////////////////////////////////////
    // Halide Funcs for camera pipeline

    // Cast input to float and scale according to its 8 bit input format
    Func scale   = make_scale(&input);
    Func descale = make_descale(&scale);

    // Realization
    Image<uint8_t> output;
    // backward pipeline
    output = descale.realize(input.width(), 
                             input.height(), 
                             input.channels());


    ////////////////////////////////////////////////////////////////////////
    // Save the output
    save_image(input, "input.png");
    save_image(output, "output.png");

    // Read in label
    val = label;
    outfile.write(&val,1);
    
    for (int c=0; c<3; c++) { 
      for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
          val = in_data[(c) + (y*3*32) + (x*3)];
          outfile.write(&val,1);
        }
      }
    }



  }

  return 0;
}

