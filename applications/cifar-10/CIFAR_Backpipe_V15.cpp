
#include "Halide.h"
#include "../common/ImgPipeConfig.h"
#include "../common/LoadCamModel.h"
#include "../common/MatrixOps.h"
#include <stdio.h>
#include <math.h>
#include "halide_image_io.h"

// This pipeline reverses tone mapping, and then applies a logarithmic tone mapping
// with a "gamma" of 0.1

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

  // Pre-computation for logarithmic tone mapping
  float gamma = 0.1;
  float A     = 255/log2f(float(255+1)*gamma);

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
 
  // Per image
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

    // Cast input to float and scale according to its 8 bit input format
    Func scale("scale");
      scale(x,y,c) = cast<float>(input(x,y,c))/256;

    // FUNCS /////////////////////////////////////////////////////////////////////

    // Backward tone mapping
    Func rev_tonemap("rev_tonemap");
      Expr rev_tone_idx = cast<uint8_t>(scale(x,y,c) * 256.0f);
      rev_tonemap(x,y,c) = rev_tone_h(c,rev_tone_idx);

    // Forward tone mapping
    Func log_tonemap("tonemap");
      //Expr rescaled = cast<float>(rev_tonemap(x,y,c))*256.0f;
      log_tonemap(x,y,c) = cast<uint8_t>( max( min ( 
                             (A * ( log( ( (rev_tonemap(x,y,c)*256.0f) +1) * gamma ) ))
                                               , 255), 0) );

    // Common scheduling
    rev_tonemap.reorder(c,x,y).bound(c,0,3).unroll(c);
    log_tonemap.reorder(c,x,y).bound(c,0,3).unroll(c);

    ///////////////////////////////////////////////////////////////////////////////////////
    // CPU Schedule
    rev_tonemap.compile_jit();
    log_tonemap.compile_jit();

    // Realization
    Image<uint8_t> output;
    // backward pipeline
    output = log_tonemap.realize(input.width(), 
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

