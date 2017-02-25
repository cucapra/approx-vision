
#include <stdio.h>
#include <math.h>

#include "../common/pipe_stages.h"
#include "../common/ImgPipeConfig.h"
#include "../common/LoadCamModel.h"
#include "../common/MatrixOps.h"

using namespace std;

// Camera pipeline stages
enum PipelineStageRev { // reverse
  RevToneMap,
  RevGamutMap,
  RevTransform
};

enum PipelineStageCV { // vision reverse
  Renoise,
  Remosaic,
  GaussianBlurCV,
  LloydRequant
};

enum PipelineStageFwd { // forward
  Requant1, // linear requantize
  Requant2,
  Requant3,
  Requant4,
  Requant5,
  Requant6,
  Requant7,

  Transform,
  GamutMap,
  ToneMap,

  DemosSubSample, // subsample demosaic
  DemosNN,        // nearest neighbor demosaic
  DemosInterp,    // bilinear interpolated demosaic
  QrtrResBinning, // quarter resolution pixel binning
  PwlToneMap      // piecewise linear tone map
};

int run_image_pipeline( char* in_img_path,
                        char* out_img_path, 
                        enum PipelineStageRev rev_stages[],  // reverse stages
                        enum PipelineStageCV cv_stages[],    // CV stages
                        enum PipelineStageFwd fwd_stages[]   // forward stages
                        );

Func run_image_pipeline_rev(Func *in_func, 
                            PipelineStageRev rev_stages[],
                            Image<float> *rev_tone_h, // rev tone map
                            int num_ctrl_pts,         // rbf ctrl pts
                            Image<float> *ctrl_pts_h, // rbf ctrl pts
                            Image<float> *weights_h,  // rbf ctrl pts
                            vector<vector<float>> *coefs, // rbf biases
                            vector<vector<float>> *TsTw_tran_inv // inv transform 
                            );

void run_image_pipeline_cv( Mat *InMat, PipelineStageCV cv_stages[] );

Func run_image_pipeline_fwd(Func *in_func, 
                            PipelineStageFwd fwd_stages[],
                            Image<float> *tone_h,     // tone map
                            int num_ctrl_pts,         // rbf ctrl pts
                            Image<float> *ctrl_pts_h, // rbf ctrl pts
                            Image<float> *weights_h,  // rbf ctrl pts
                            vector<vector<float>> *coefs, // rbf biases
                            vector<vector<float>> *TsTw_tran // transform 
                            );
