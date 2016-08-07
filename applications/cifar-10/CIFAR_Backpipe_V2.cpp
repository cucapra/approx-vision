
#include "Halide.h"
#include "../common/ImgPipeConfig.h"
#include "../common/LoadCamModel.h"
#include "../common/MatrixOps.h"
#include <stdio.h>
#include <math.h>
#include "halide_image_io.h"

// This pipeline only performs reverse tone mapping and gamut mapping
// The result is as if only color transformation was performed

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
  Image<uint8_t> input(32,32,3);

  ///////////////////////////////////////////////////////////////////////////////////////
  // Process Images
 

  for (int i=0; i<10000; i++) { //i<10000

    // print status
    printf("test_batch_V2 - Image num: %u\n",i);

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
      rev_rbf_biases(x,y,c) = cast<uint8_t>( min( max( 256*select( 
        c == 0, rev_rbf_ctrl_pts(x,y,0) + coefs[0][0] + coefs[1][0]*rev_tonemap(x,y,0) +
          coefs[2][0]*rev_tonemap(x,y,1) + coefs[3][0]*rev_tonemap(x,y,2),
        c == 1, rev_rbf_ctrl_pts(x,y,1) + coefs[0][1] + coefs[1][1]*rev_tonemap(x,y,0) +
          coefs[2][1]*rev_tonemap(x,y,1) + coefs[3][1]*rev_tonemap(x,y,2),
                rev_rbf_ctrl_pts(x,y,2) + coefs[0][2] + coefs[1][2]*rev_tonemap(x,y,0) +
          coefs[2][2]*rev_tonemap(x,y,1) + coefs[3][2]*rev_tonemap(x,y,2))
                              , 0), 255 ));


  /*
    Func normalize("normalize");
      normalize(x,y,c) = cast<uint8_t>( min( select(
                           c == 1, rev_transform(x,y,c),
                                   rev_transform(x,y,c) * 2), 255) );
  */


    // Common scheduling
    rev_rbf_ctrl_pts.reorder(c,x,y).bound(c,0,3).unroll(c);
    rev_rbf_biases.reorder(c,x,y).bound(c,0,3).unroll(c);
    rev_tonemap.reorder(c,x,y).bound(c,0,3).unroll(c);


    rev_tonemap.compute_root();
    rev_rbf_ctrl_pts.compute_root();
    rev_rbf_biases.compute_root();

    ///////////////////////////////////////////////////////////////////////////////////////
    // CPU Schedule

    Var yo, yi;

    rev_rbf_ctrl_pts.vectorize(x, 8);
    rev_rbf_biases.vectorize(x, 8);
    rev_tonemap.vectorize(x, 8);

    rev_rbf_ctrl_pts.split(y,yo,yi,8).parallel(yo);
    rev_rbf_biases.split(y,yo,yi,8).parallel(yo);
    rev_tonemap.split(y,yo,yi,8).parallel(yo);

    rev_rbf_ctrl_pts.compile_jit();
    rev_rbf_biases.compile_jit();
    rev_tonemap.compile_jit();



    // Realization
    Image<uint8_t> output;
    // backward pipeline
    output = rev_rbf_biases.realize(input.width(), 
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

