
#include "Halide.h"
#include "../common/ImgPipeConfig.h"
#include "../common/LoadCamModel.h"
#include "../common/MatrixOps.h"
#include <stdio.h>
#include <math.h>
#include "halide_image_io.h"

// This pipeline further quantizes the pixel values

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
  if(!infile) {
    printf("Input file not found: %s\n", in_data_path);
    return 1;
  }  

  fstream outfile;
  outfile.open(out_data_path,fstream::out);

  // Declare image handle variables
  Var x, y, c;

  // Define input image 
  Image<uint8_t> input(32,32,3);

  ///////////////////////////////////////////////////////////////////////////////////////
  // Process Images
 

  for (int i=0; i<10000; i++) { //i<10000

    // print status
    //printf("test - Image num: %u\n",i);

    // Read in label
    infile.read(&val,1);
    label = val;
    //printf("label: %u\n",(unsigned char)label);
    
    for (int c=0; c<3; c++) { 
      for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
          infile.read(&val,1);
          input(x,y,c) = (unsigned char)val;
        }
      }
    }

    //save_image(input, "input.png");

    ///////////////////////////////////////////////////////////////////////////////////////
    // Halide Funcs for camera pipeline

    int num_bits  = 6;
    int scale_val = pow(2,(8-num_bits));

    // Shift the data to the right to remove LSB(s)
    Func right_shift("right_shift");
      right_shift(x,y,c) = input(x,y,c) / scale_val;

    // Shift the data back to the left to rescale
    Func left_shift("left_shift");
      left_shift(x,y,c) = right_shift(x,y,c) * scale_val;

    ///////////////////////////////////////////////////////////////////////////////////////
    // CPU Schedule
    left_shift.compile_jit();


    // Realization
    Image<uint8_t> output;
    // backward pipeline
    output = left_shift.realize(input.width(), 
                             input.height(), 
                             input.channels());

    ////////////////////////////////////////////////////////////////////////
    // Save the output

    //save_image(output, "output.png");

    // Read in label
    val = label;
    outfile.write(&val,1);
    //infile.read(&val,1);
    //label = val;
    
    for (int c=0; c<3; c++) { 
      for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
          val = output(x,y,c);
          outfile.write(&val,1);
        }
      }
    }
  }

  return 0;
}

