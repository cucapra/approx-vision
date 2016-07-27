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
#include "clock.h"

int main(int argc, char **argv) {

  // Inform user of usage method
  if ( argc != 3 )
  {
      printf("usage: \n./convert path/to/in/image out/image/dir\n");
      return -1;
  }

  // Input image (path and name)
  const char * in_img_path = argv[1];

  // Output image (just path)
  const char * out_path    = argv[2];

  ///////////////////////////////////////////////////////////////////////////////////////
  // Import and format model data

  using namespace std;  

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
  ctrl_pts  = get_ctrl_pts (cam_model_path, num_ctrl_pts, false);
  weights   = get_weights  (cam_model_path, num_ctrl_pts, false);
  coefs     = get_coefs    (cam_model_path, num_ctrl_pts, false);
  rev_tone  = get_rev_tone (cam_model_path);

  // Take the transpose of the color map and white balance transform for later use
  vector<vector<float>> TsTw_tran = transpose_mat (TsTw);

  // If we are performing a backward implementation of the pipeline, 
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
  // Import and format input image

  // Declare image handle variables
  Var x, y, c;

  // Load input image 
  Image<uint8_t> input = load_image(in_img_path);

  ///////////////////////////////////////////////////////////////////////////////////////
  // Halide Funcs for camera pipeline

  // Cast input to float and scale according to its 8 bit input format
  Func scale("scale");
    scale(x,y,c) = cast<float>(input(x,y,c))/256;

  // FORWARD FUNCS //////////////////////////////////////////////////////////////////////
 
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

  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
    // Initialization with all zero
    rbf_ctrl_pts(x,y,c) = cast<float>(0);
    // Index to iterate with
    RDom idx(0,num_ctrl_pts);
    // Loop code
    // Subtract the vectors 
    Expr red_sub   = transform(x,y,0) - ctrl_pts_h(0,idx);
    Expr green_sub = transform(x,y,1) - ctrl_pts_h(1,idx);
    Expr blue_sub  = transform(x,y,2) - ctrl_pts_h(2,idx);
    // Take the L2 norm to get the distance
    Expr dist      = sqrt( red_sub*red_sub + green_sub*green_sub + blue_sub*blue_sub );
    // Update persistant loop variables
    rbf_ctrl_pts(x,y,c) = select( c == 0, rbf_ctrl_pts(x,y,c) + (weights_h(0,idx) * dist),
                                  c == 1, rbf_ctrl_pts(x,y,c) + (weights_h(1,idx) * dist),
                                          rbf_ctrl_pts(x,y,c) + (weights_h(2,idx) * dist));

  // Add on the biases for the RBF
  Func rbf_biases("rbf_biases");
    rbf_biases(x,y,c) = max( select( 
      c == 0, rbf_ctrl_pts(x,y,0) + coefs[0][0] + coefs[1][0]*transform(x,y,0) +
        coefs[2][0]*transform(x,y,1) + coefs[3][0]*transform(x,y,2),
      c == 1, rbf_ctrl_pts(x,y,1) + coefs[0][1] + coefs[1][1]*transform(x,y,0) +
        coefs[2][1]*transform(x,y,1) + coefs[3][1]*transform(x,y,2),
              rbf_ctrl_pts(x,y,2) + coefs[0][2] + coefs[1][2]*transform(x,y,0) +
        coefs[2][2]*transform(x,y,1) + coefs[3][2]*transform(x,y,2))
                            , 0);

  // Forward tone mapping
  Func tonemap("tonemap");
    RDom idx2(0,256);
    // Theres a lot in this one line! Functionality wise it finds the entry in 
    // the reverse tone mapping function which is closest to the value found by
    // gamut mapping. The output is then cast to uint8 for output. Effectively 
    // it reverses the reverse tone mapping function.
    tonemap(x,y,c) = cast<uint8_t>(argmin( abs( rev_tone_h(c,idx2) - rbf_biases(x,y,c) ) )[0]);


  // BACKWARD FUNCS /////////////////////////////////////////////////////////////////////

  // Backward tone mapping
  Func rev_tonemap("rev_tonemap");
    Expr rev_tone_idx = cast<uint8_t>(scale(x,y,c) * 256.0f);
    rev_tonemap(x,y,c) = rev_tone_h(c,rev_tone_idx) ;

  // Weighted radial basis function for gamut mapping
  Func rev_rbf_ctrl_pts("rev_rbf_ctrl_pts");
    // Initialization with all zero
    rev_rbf_ctrl_pts(x,y,c) = cast<float>(0);
    // Index to iterate with
    RDom revidx(0,num_ctrl_pts);
    // Loop code
    // Subtract the vectors 
    Expr revred_sub   = rev_tonemap(x,y,0) - ctrl_pts_h(0,revidx);
    Expr revgreen_sub = rev_tonemap(x,y,1) - ctrl_pts_h(1,revidx);
    Expr revblue_sub  = rev_tonemap(x,y,2) - ctrl_pts_h(2,revidx);
    // Take the L2 norm to get the distance
    Expr revdist      = sqrt( revred_sub*revred_sub + 
                              revgreen_sub*revgreen_sub + 
                              revblue_sub*revblue_sub );
    // Update persistant loop variables
    rev_rbf_ctrl_pts(x,y,c) = select( c == 0, rev_rbf_ctrl_pts(x,y,c) + (weights_h(0,revidx) * revdist),
                                      c == 1, rev_rbf_ctrl_pts(x,y,c) + (weights_h(1,revidx) * revdist),
                                              rev_rbf_ctrl_pts(x,y,c) + (weights_h(2,revidx) * revdist));

  // Add on the biases for the RBF
  Func rev_rbf_biases("rev_rbf_biases");
    rev_rbf_biases(x,y,c) = max( select( 
      c == 0, rev_rbf_ctrl_pts(x,y,0) + coefs[0][0] + coefs[1][0]*rev_tonemap(x,y,0) +
        coefs[2][0]*rev_tonemap(x,y,1) + coefs[3][0]*rev_tonemap(x,y,2),
      c == 1, rev_rbf_ctrl_pts(x,y,1) + coefs[0][1] + coefs[1][1]*rev_tonemap(x,y,0) +
        coefs[2][1]*rev_tonemap(x,y,1) + coefs[3][1]*rev_tonemap(x,y,2),
              rev_rbf_ctrl_pts(x,y,2) + coefs[0][2] + coefs[1][2]*rev_tonemap(x,y,0) +
        coefs[2][2]*rev_tonemap(x,y,1) + coefs[3][2]*rev_tonemap(x,y,2))
                            , 0);

  // Reverse color map and white balance transform
  Func rev_transform("rev_transform");
    rev_transform(x,y,c) = cast<uint8_t>( 256.0f * max( select(
      // Perform matrix multiplication, set min of 0
      c == 0, rev_rbf_biases(x,y,0)*TsTw_tran[0][0]
            + rev_rbf_biases(x,y,1)*TsTw_tran[1][0]
            + rev_rbf_biases(x,y,2)*TsTw_tran[2][0],
      c == 1, rev_rbf_biases(x,y,0)*TsTw_tran[0][1]
            + rev_rbf_biases(x,y,1)*TsTw_tran[1][1]
            + rev_rbf_biases(x,y,2)*TsTw_tran[2][1],
              rev_rbf_biases(x,y,0)*TsTw_tran[0][2]
            + rev_rbf_biases(x,y,1)*TsTw_tran[1][2]
            + rev_rbf_biases(x,y,2)*TsTw_tran[2][2])
                                                        , 0) );

  ////////////////////////////////////////////////////////////////////////
  // Scheduling

  // Loop over color (c) first, unroll this loop
  transform.reorder(c,x,y).bound(c,0,3).unroll(c);
  rbf_ctrl_pts.reorder(c,x,y).bound(c,0,3).unroll(c);
  rbf_biases.reorder(c,x,y).bound(c,0,3).unroll(c);
  tonemap.reorder(c,x,y).bound(c,0,3).unroll(c);

  rev_transform.reorder(c,x,y).bound(c,0,3).unroll(c);
  rev_rbf_ctrl_pts.reorder(c,x,y).bound(c,0,3).unroll(c);
  rev_rbf_biases.reorder(c,x,y).bound(c,0,3).unroll(c);
  rev_tonemap.reorder(c,x,y).bound(c,0,3).unroll(c);

  // Go pixel by pixel, store intermediate values for later use
  transform.store_root().compute_at(tonemap,x);
  rbf_ctrl_pts.store_root().compute_at(tonemap,x);
  rbf_biases.store_root().compute_at(tonemap,x);

  rev_tonemap.store_root().compute_at(rev_transform,x);
  rev_rbf_ctrl_pts.store_root().compute_at(rev_transform,x);
  rev_rbf_biases.store_root().compute_at(rev_transform,x);

  // Use the just in time compiler
  tonemap.compile_jit();
  rev_tonemap.compile_jit();

  ////////////////////////////////////////////////////////////////////////
  // Realization (actual computation)

  double t1, t2;
  t1 = current_time();

  Image<uint8_t> output;

  // backward pipeline
  output = rev_transform.realize(input.width(), 
                                 input.height(), 
                                 input.channels());

  t2 = current_time();

  printf("Processing time = %1.4f milliseconds\n",t2-t1);

  ////////////////////////////////////////////////////////////////////////
  // Save the output
  save_image(output, (std::string(out_path)+"output.png").c_str());

  return 0;
}

