 

#include "Halide.h"
#include "ImgPipeConfig.h"
#include "LoadCamModel.h"
#include "MatrixOps.h"
#include <stdio.h>
#include <math.h>
#include "halide_image_io.h"


int main(int argc, char **argv) {

  ///////////////////////////////////////////////////////////////////////////////////////
  // Import and format model data

  using namespace std;  

  // Read in data for forward pipeline
  bool direction = true;

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
  ctrl_pts  = get_ctrl_pts (cam_model_path, num_ctrl_pts, direction);
  weights   = get_weights  (cam_model_path, num_ctrl_pts, direction);
  coefs     = get_coefs    (cam_model_path, num_ctrl_pts, direction);
  rev_tone  = get_rev_tone (cam_model_path);

  // Take the transpose of the color map and white balance scale for later use
  vector<vector<float>> TsTw_tran = transpose_mat (TsTw);

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
  // Import and format input image

  // Declare image handle variables
  Var x, y, c;

  // Load input image 
  Image<uint8_t> input;
  input = load_image(image_3C);

  // Cast input to float and scale according to its 8 bit input format
  Func scale("scale");
    scale(x,y,c) = (input(x,y,c));

  // Fake LUT (used for both gamut_lut and tone_lut)
  // The actual functionality is backward tone mapping
  Func lut_func("lut_func");
    Expr idx = cast<uint8_t>(scale(x,y,c));
    lut_func(x,y,c) = rev_tone_h(c,idx) ;

  //lut_func.trace_stores();

  Func descale("descale");
    descale(x,y,c) = cast<uint8_t>(lut_func(x,y,c)*256);

  ////////////////////////////////////////////////////////////////////////
  // Realization (actual computation)


  Image<uint8_t> output;
  output = descale.realize(input.width(), 
                           input.height(), 
                           input.channels());


  ////////////////////////////////////////////////////////////////////////
  // Save the output

  //save_image(output, "out.png");

  return 0;
}

