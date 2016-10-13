
#include "Halide.h"
#include "halide_image_io.h"
#include <opencv2/opencv.hpp>

using namespace Halide;
using namespace cv;

///////////////////////////////////////////////////////////////////////////////////////
// Conversion functions

Mat Image2Mat( Image<float> InImage );

Image<float> Mat2Image( Mat InMat );

///////////////////////////////////////////////////////////////////////////////////////
// OpenCV Funcs for camera pipeline


///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img );

Func make_descale( Func *in_func );

Func make_rev_tone_map( Func *in_func, Image<float> *rev_tone_h );

Func make_tone_map( Func *in_func, Image<float> *rev_tone_h );

Func make_rbf_ctrl_pts( Func *in_func,
                        int num_ctrl_pts,
                        Image<float> *ctrl_pts_h,
                        Image<float> *weights_h );

Func make_rbf_ctrl_pts_( Func *in_func,
                        int num_ctrl_pts,
                        Image<float> *ctrl_pts_h,
                        Image<float> *weights_h );



Func make_rbf_biases( Func *in_func,
                      Func *rbf_ctrl_pts,
                      vector<vector<float>> *coefs );

Func make_transform( Func *in_func, 
                     vector<vector<float>> *TsTw_tran );
