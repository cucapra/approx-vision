
#include "Halide.h"
#include "halide_image_io.h"
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace Halide;
using namespace cv;

///////////////////////////////////////////////////////////////////////////////////////
// Conversion functions

Mat Image2Mat( Image<float> *InImage );

Image<float> Mat2Image( Mat *InMat );

Func make_Image2Func ( Image<float> *InImage );

Func make_Image2Func ( Image<uint8_t> *InImage );

///////////////////////////////////////////////////////////////////////////////////////
// OpenCV Funcs for camera pipeline

void OpenCV_renoise ( Mat *InMat );

void OpenCV_remosaic (Mat *InMat );

void OpenCV_gaussian_blur ( Mat *InMat );

void OpenCV_lloydmax_requant( Mat *InMat );

///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img );

Func make_scale( Func *in_func );

Func make_descale( Func *in_func );

Func make_requant( Image<uint8_t> *in_img, int num_bits );

Func make_requant( Func *in_func, int num_bits );

Func make_demosaic_subsample( Func *in_func );

Func make_demosaic_nn( Func *in_func );

Func make_demosaic_interp( Func *in_func );

Func make_qrtr_res_binning( Func *in_func );

Func make_rev_tone_map( Func *in_func, Image<float> *rev_tone_h );

Func make_tone_map( Func *in_func, Image<float> *rev_tone_h );

Func make_pwl_tone_map( Func *in_func );

Func make_rbf_ctrl_pts( Func *in_func,
                        int num_ctrl_pts,
                        Image<float> *ctrl_pts_h,
                        Image<float> *weights_h );

Func make_rbf_biases( Func *in_func,
                      Func *rbf_ctrl_pts,
                      vector<vector<float>> *coefs );

Func make_transform( Func *in_func, 
                     vector<vector<float>> *TsTw_tran );

Image<float> gaussian_blur(Image<float> *in);
