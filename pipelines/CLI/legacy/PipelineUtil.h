
#include <stdio.h>
#include <math.h>

#include "/approx-vision/pipelines/common/pipe_stages.h"
#include "/approx-vision/pipelines/common/ImgPipeConfig.h"
#include "/approx-vision/pipelines/common/LoadCamModel.h"
#include "/approx-vision/pipelines/common/MatrixOps.h"

#include "/approx-vision/pipelines/CLI/CameraModel.h"

using namespace std;

// uncomment for debug prints
// #define _DEBUG_MODE

#ifdef _DEBUG_MODE
#define debug_print(x) cout << (x) << endl;
#else
#define debug_print(x)
#endif

/* Pipeline Stages */
enum PipelineStage {
  Scale,  

  Requant1,       // linear requantize, 
  Requant2,       // relies on the fact enum val is 1, 2, 3, ... here
  Requant3,
  Requant4,
  Requant5,
  Requant6,
  Requant7,

  RevToneMap,
  RevGamutMap,
  RevTransform,

  Renoise,
  Remosaic,
  GaussianBlurCV,
  LloydRequant,

  Descale,

  GamutMap,
  Transform,
  ToneMap,

  DemosSubSample, // subsample demosaic
  DemosNN,        // nearest neighbor demosaic
  DemosInterp,    // bilinear interpolated demosaic
  QrtrResBinning, // quarter resolution pixel binning
  PwlToneMap      // piecewise linear tone map
};

/**
  Runs image file through specified reverse, cv, forward pipeline stages
  and outputs to image file

  in_img_path   : full path to input image file
  out_img_path  : full path to output image file
  rev_stages    : list of reverse image pipeline stages to run
  cv_stages     : list of openCV pipeline stages to run
  fwd_stages    : list of forward image pipeline stages to run
  
  Returns 0 if successful
**/
int run_image_pipeline(     char* in_img_path,
                            char* out_img_path, 
                            CameraModel cam_model,
                            enum PipelineStage stages[], 
                            int num_stages
                            );

Func run_image_pipeline_stage(Func *in_func,
                            PipelineStage stages[],
                            int num_stages,
                            int width,
                            int height,
                            int num_ctrl_pts,         // rbf ctrl pts
                            Image<float> *rev_ctrl_pts_h, // rbf ctrl pts
                            Image<float> *rev_tone_h, // rev tone map
                            Image<float> *rev_weights_h,  // rbf ctrl pts
                            vector<vector<float>> *rev_coefs, // rbf biases
                            vector<vector<float>> *TsTw_tran_inv, // inv transform 
                            Image<float> *ctrl_pts_h,
                            vector<int> &qrtr_bin_factor, // qrtr binning factor
                            Image<float> *tone_h,     // tone map
                            Image<float> *weights_h,  // rbf ctrl pts
                            vector<vector<float>> *coefs, // rbf biases
                            vector<vector<float>> *TsTw_tran // transform 
);


/**
  Runs input Halide pipeline stage object through reverse pipeline stages
  and returns resulting Halide pipeline stage object

  in_func       : input Halide pipeline stage object
  rev_stages    : list of reverse image pipeline stages to run
  num_stages    : length of rev_stages
  rev_tone_h    : reverse tone mapping for camera model
  num_ctrl_pts  : number of reverse control points for camera model
  ctrl_pts_h    : reverse control points for camera model
  weights_h     : reverse weights for camera model
  coefs         : reverse coefficients for camera model
  TsTw_tran_inv : reverse white balance + color space transform for camera model

  Returns resulting Halide pipeline stage object
**/
// Func run_image_pipeline_rev(Func *in_func, 
//                             PipelineStageRev rev_stages[],
//                             int num_stages,
//                             Image<float> *rev_tone_h, // rev tone map
//                             int num_ctrl_pts,         // rbf ctrl pts
//                             Image<float> *ctrl_pts_h, // rbf ctrl pts
//                             Image<float> *weights_h,  // rbf ctrl pts
//                             vector<vector<float>> *coefs, // rbf biases
//                             vector<vector<float>> *TsTw_tran_inv // inv transform 
//                             );

/**
  Runs matrix representatation of image through openCV pipeline stages in place.

  InMat         : matrix representation of input image
  cv_stages     : list of openCV stages to run
  num_stages    : length of cv_stages

**/
// void run_image_pipeline_cv( Mat *InMat, 
//                             PipelineStageCV cv_stages[], 
//                             int num_stages 
//                             );

/**
  Runs input Halide pipeline stage object through forward pipeline stages
  and returns resulting Halide pipeline stage object

  in_func       : input Halide pipeline stage object
  fwd_stages    : list of forward image pipeline stages to run
  num_stages    : length of fwd_stages
  tone_h        : tone mapping for camera model (same as reverse direction)
  num_ctrl_pts  : number of forward control points for camera model
  ctrl_pts_h    : forward control points for camera model
  weights_h     : forward weights for camera model
  coefs         : forward coefficients for camera model
  TsTw_tran     : forward white balance + color space transform for camera model

  Returns resulting Halide pipeline stage object
**/
// Func run_image_pipeline_fwd(Func *in_func, 
//                             PipelineStageFwd fwd_stages[],
//                             int num_stages,
//                             vector<int> &qrtr_bin_factor, // qrtr binning factor
//                             Image<float> *tone_h,     // tone map
//                             int num_ctrl_pts,         // rbf ctrl pts
//                             Image<float> *ctrl_pts_h, // rbf ctrl pts
//                             Image<float> *weights_h,  // rbf ctrl pts
//                             vector<vector<float>> *coefs, // rbf biases
//                             vector<vector<float>> *TsTw_tran // transform 
//                             );
