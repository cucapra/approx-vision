
#include "pipe_stages.h"

using namespace Halide;
using namespace cv;

///////////////////////////////////////////////////////////////////////////////////////
// Conversion functions

Mat Image2Mat( Image<float> InImage ) {
  Mat OutMat(InImage.height(),InImage.width(),CV_32FC3);
  vector<Mat> three_channels;
  split(OutMat, three_channels);

  // Convert from planar RGB memory storage to interleaved BGR memory storage
  for (int y=0; y<InImage.height(); y++) {
    for (int x=0; x<InImage.width(); x++) {
      // Blue channel
      three_channels[0].at<float>(y,x) = InImage(x,y,2);
      // Green channel
      three_channels[1].at<float>(y,x) = InImage(x,y,1);
      // Red channel
      three_channels[2].at<float>(y,x) = InImage(x,y,0);
    }
  }

  merge(three_channels, OutMat);

  return OutMat;
}

Image<float> Mat2Image( Mat InMat ) {
  Image<float> OutImage(InMat.cols,InMat.rows,3);
  vector<Mat> three_channels;
  split(InMat, three_channels);

  // Convert from interleaved BGR memory storage to planar RGB memory storage
  for (int y=0; y<InMat.rows; y++) {
    for (int x=0; x<InMat.cols; x++) {
      // Blue channel
      OutImage(x,y,2) = three_channels[0].at<float>(y,x);
      // Green channel
      OutImage(x,y,1) = three_channels[1].at<float>(y,x);
      // Red channel
      OutImage(x,y,0) = three_channels[2].at<float>(y,x);
    }
  }

  return OutImage;
}

///////////////////////////////////////////////////////////////////////////////////////
// OpenCV Funcs for camera pipeline



///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img ) {
  Var x, y, c;
  // Cast input to float and scale from 8 bit 0-255 range to 0-1 range
  Func scale("scale");
    scale(x,y,c) = cast<float>( (*in_img)(x,y,c) ) / 256;
  return scale;
}

Func make_descale( Func *in_func ) {
  Var x, y, c;
  // de-scale from 0-1 range to 0-255 range, and cast to 8 bit 
  Func descale("descale");
    descale(x,y,c) = cast<uint8_t>( (*in_func)(x,y,c) * 256 );
  return descale;
}

Func make_rev_tone_map( Func *in_func, 
                       Image<float> *rev_tone_h ) {
  Var x, y, c;
  // Backward tone mapping
  Func rev_tone_map("rev_tone_map");
    Expr rev_tone_idx = cast<uint8_t>((*in_func)(x,y,c) * 256.0f);
    rev_tone_map(x,y,c) = (*rev_tone_h)(c,rev_tone_idx) ;
  return rev_tone_map;
}

Func make_tone_map( Func *in_func,
                    Image<float> *rev_tone_h ) {
  Var x, y, c;
  // Forward tone mapping
  Func tone_map("tone_map");
    RDom idx2(0,256);
    // Theres a lot in this one line! This line finds the entry in 
    // the reverse tone mapping function which is closest to this Func's
    // input. It then scales back down to the 0-1 range expected as the 
    // output for every stage. Effectively it reverses the reverse 
    // tone mapping function.
    tone_map(x,y,c) = (argmin( abs( (*rev_tone_h)(c,idx2) 
                                 - (*in_func)(x,y,c) ) )[0])/256.0f;
  return tone_map;
}

Func make_rbf_ctrl_pts( Func *in_func, 
                        int num_ctrl_pts,
                        Image<float> *ctrl_pts_h, 
                        Image<float> *weights_h ) {
  Var x, y, c;
  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
    // Initialization with all zero
    rbf_ctrl_pts(x,y,c) = cast<float>(0);
    // Index to iterate with
    RDom idx(0,num_ctrl_pts);
    // Loop code
    // Subtract the vectors 
    Expr red_sub   = (*in_func)(x,y,0) - (*ctrl_pts_h)(0,idx);
    Expr green_sub = (*in_func)(x,y,1) - (*ctrl_pts_h)(1,idx);
    Expr blue_sub  = (*in_func)(x,y,2) - (*ctrl_pts_h)(2,idx);
    // Take the L2 norm to get the distance
    Expr dist      = sqrt( red_sub*red_sub + 
                              green_sub*green_sub + 
                              blue_sub*blue_sub );
    // Update persistant loop variables
    rbf_ctrl_pts(x,y,c) = select( c == 0, rbf_ctrl_pts(x,y,c) +
                                    ( (*weights_h)(0,idx) * dist),
                                  c == 1, rbf_ctrl_pts(x,y,c) + 
                                    ( (*weights_h)(1,idx) * dist),
                                          rbf_ctrl_pts(x,y,c) + 
                                    ( (*weights_h)(2,idx) * dist));
  return rbf_ctrl_pts;
}

Func make_rbf_ctrl_pts_( Func *in_func, 
                        int num_ctrl_pts,
                        Image<float> *ctrl_pts_h, 
                        Image<float> *weights_h ) {
  Var x, y, c;
  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
    // Initialization with all zero
    rbf_ctrl_pts(x,y,c) = cast<float>(0);
    // Index to iterate with
    RDom idx_(0,num_ctrl_pts);
    // Loop code
    // Subtract the vectors 
    Expr red_sub   = (*in_func)(x,y,0) - (*ctrl_pts_h)(0,idx_);
    Expr green_sub = (*in_func)(x,y,1) - (*ctrl_pts_h)(1,idx_);
    Expr blue_sub  = (*in_func)(x,y,2) - (*ctrl_pts_h)(2,idx_);
    // Take the L2 norm to get the distance
    Expr dist      = sqrt( red_sub*red_sub + 
                              green_sub*green_sub + 
                              blue_sub*blue_sub );
    // Update persistant loop variables
    rbf_ctrl_pts(x,y,c) = select( c == 0, rbf_ctrl_pts(x,y,c) +
                                    ( (*weights_h)(0,idx_) * dist),
                                  c == 1, rbf_ctrl_pts(x,y,c) + 
                                    ( (*weights_h)(1,idx_) * dist),
                                          rbf_ctrl_pts(x,y,c) + 
                                    ( (*weights_h)(2,idx_) * dist));
  return rbf_ctrl_pts;
}


Func make_rbf_biases( Func *in_func, 
                      Func *rbf_ctrl_pts, 
                      vector<vector<float>> *coefs ) {
  Var x, y, c;
  // Add on the biases for the RBF
  Func rbf_biases("rbf_biases");
    rbf_biases(x,y,c) = max( select( 
      c == 0, (*rbf_ctrl_pts)(x,y,0)     + (*coefs)[0][0] + (*coefs)[1][0]*(*in_func)(x,y,0) +
        (*coefs)[2][0]*(*in_func)(x,y,1) + (*coefs)[3][0]*(*in_func)(x,y,2),
      c == 1, (*rbf_ctrl_pts)(x,y,1)     + (*coefs)[0][1] + (*coefs)[1][1]*(*in_func)(x,y,0) +
        (*coefs)[2][1]*(*in_func)(x,y,1) + (*coefs)[3][1]*(*in_func)(x,y,2),
              (*rbf_ctrl_pts)(x,y,2)     + (*coefs)[0][2] + (*coefs)[1][2]*(*in_func)(x,y,0) +
        (*coefs)[2][2]*(*in_func)(x,y,1) + (*coefs)[3][2]*(*in_func)(x,y,2))
                            , 0);
  return rbf_biases;
}

Func make_transform( Func *in_func, 
                     vector<vector<float>> *TsTw_tran ) {
  Var x, y, c;
  // Reverse color map and white balance transform
  Func transform("transform");
    transform(x,y,c) = max( select(
      // Perform matrix multiplication, set min of 0
      c == 0, (*in_func)(x,y,0)*(*TsTw_tran)[0][0]
            + (*in_func)(x,y,1)*(*TsTw_tran)[1][0]
            + (*in_func)(x,y,2)*(*TsTw_tran)[2][0],
      c == 1, (*in_func)(x,y,0)*(*TsTw_tran)[0][1]
            + (*in_func)(x,y,1)*(*TsTw_tran)[1][1]
            + (*in_func)(x,y,2)*(*TsTw_tran)[2][1],
              (*in_func)(x,y,0)*(*TsTw_tran)[0][2]
            + (*in_func)(x,y,1)*(*TsTw_tran)[1][2]
            + (*in_func)(x,y,2)*(*TsTw_tran)[2][2])
                                                      , 0);
  return transform;
}

