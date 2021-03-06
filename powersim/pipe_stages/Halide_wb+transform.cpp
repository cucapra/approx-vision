/////////////////////////////////////////////////////////////////////////////////////////
// Image Processing Pipeline
//
// This is a Halide implementation of a pre-learned image 
// processing model. A description of the model can be found in
// "A New In-Camera Imaging Model for Color Computer Vision 
// and its Application" by Seon Joo Kim, Hai Ting Lin, 
// Michael Brown, et al. Code for learning a new model can 
// be found at the original project page. This particular 
// implementation was written by Mark Buckler.
//
// Original Project Page:
// http://www.comp.nus.edu.sg/~brown/radiometric_calibration/
//
// Model Format Readme:
// http://www.comp.nus.edu.sg/~brown/radiometric_calibration/datasets/Model_param/readme.pdf
//
/////////////////////////////////////////////////////////////////////////////////////////

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

  // Take the transpose of the color map and white balance transform for later use
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
    scale(x,y,c) = cast<float>(input(x,y,c))/256;


  // Color map and white balance transform
  Func transform("transform");
    transform(x,y,c) = select(
      // Perform matrix multiplication, set min of 0
      c == 0, scale(x,y,0)*TsTw_tran[0][0]
            + scale(x,y,1)*TsTw_tran[1][0]
            + scale(x,y,2)*TsTw_tran[2][0],
      c == 1, scale(x,y,0)*TsTw_tran[0][1]
            + scale(x,y,1)*TsTw_tran[1][1]
            + scale(x,y,2)*TsTw_tran[2][1],
              scale(x,y,0)*TsTw_tran[0][2]
            + scale(x,y,1)*TsTw_tran[1][2]
            + scale(x,y,2)*TsTw_tran[2][2]);

/*
  // Color map and white balance transform
  Func transform("transform");
    transform(x,y,c) = select(
      // Perform matrix multiplication, set min of 0
      c == 0, scale(x,y,0)*TsTw_tran[0][0],
      c == 1, scale(x,y,0)*TsTw_tran[0][1],
              scale(x,y,0)*TsTw_tran[0][2]);
*/
//  transform.trace_stores();

  Func descale("descale");
    descale(x,y,c) = cast<uint8_t>(transform(x,y,c)*256);
//    descale(x,y,c) = cast<uint8_t>(transform(x,y,c));


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

