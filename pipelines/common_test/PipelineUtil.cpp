#include "PipelineUtil.h"

// Pipeline Utility 

int run_image_pipeline( char* in_img_path, 
                        char* out_img_path, 
                        enum PipelineStageRev  rev_stages[],
                        enum PipelineStageCV   cv_stages[],
                        enum PipelineStageFwd  fwd_stages[],
                        int num_stages[] ) {

  /////////////////////////////////////////////////////////////////////////////
  //                        Import and format model data
  /////////////////////////////////////////////////////////////////////////////
  
  // Declare model parameters
  vector<vector<float>> Ts, Tw, TsTw;
  vector<vector<float>> rev_ctrl_pts, rev_weights, rev_coefs;
  vector<vector<float>> ctrl_pts, weights, coefs;
  vector<vector<float>> rev_tone;

  // Load model parameters from file
  // NOTE: Ts, Tw, and TsTw read only forward data
  // ctrl_pts, weights, and coefs are either forward or backward
  // tone mapping is always backward
  // This is due to the the camera model format
  Ts            = get_Ts       (cam_model_path);
  Tw            = get_Tw       (cam_model_path, wb_index);
  TsTw          = get_TsTw     (cam_model_path, wb_index);
  rev_ctrl_pts  = get_ctrl_pts (cam_model_path, num_ctrl_pts, 0);
  rev_weights   = get_weights  (cam_model_path, num_ctrl_pts, 0);
  rev_coefs     = get_coefs    (cam_model_path, num_ctrl_pts, 0);
  ctrl_pts      = get_ctrl_pts (cam_model_path, num_ctrl_pts, 1);
  weights       = get_weights  (cam_model_path, num_ctrl_pts, 1);
  coefs         = get_coefs    (cam_model_path, num_ctrl_pts, 1);
  rev_tone      = get_rev_tone (cam_model_path);

  // Take the transpose of the color map and white balance transform for later use
  vector<vector<float>> TsTw_tran     = transpose_mat(TsTw);

  // Create an inverse of TsTw_tran
  vector<vector<float>> TsTw_tran_inv = inv_3x3mat(TsTw_tran);

  using namespace Halide;
  using namespace Halide::Tools;
  using namespace cv;

  // Convert backward control points to a Halide image
  int width  = rev_ctrl_pts[0].size();
  int length = rev_ctrl_pts.size();
  Image<float> rev_ctrl_pts_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_ctrl_pts_h(x,y) = rev_ctrl_pts[y][x];
    }
  }

  // Convert backward weights to a Halide image
  width  = rev_weights[0].size();
  length = rev_weights.size();
  Image<float> rev_weights_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_weights_h(x,y) = rev_weights[y][x];
    }
  }

  // Convert control points to a Halide image
  width  = ctrl_pts[0].size();
  length = ctrl_pts.size();
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
 
  /////////////////////////////////////////////////////////////////////////////
  //                    Import and format input image
  /////////////////////////////////////////////////////////////////////////////

  // Load input image 
  Image<uint8_t> input          = load_image(in_img_path);
  width                         = input.width();
  int height                    = input.height();

  /////////////////////////////////////////////////////////////////////////////
  //                          Camera Pipeline
  /////////////////////////////////////////////////////////////////////////////

  // Scale to 0-1 range and represent in floating point
  Func scale                    = make_scale( &input );
  Func lastFunc                 = scale;

  // 1. reverse pipelines
  lastFunc = run_image_pipeline_rev( &lastFunc, 
                                     rev_stages,
                                     num_stages[0],
                                     &rev_tone_h,
                                     num_ctrl_pts,
                                     &rev_ctrl_pts_h,
                                     &rev_weights_h,
                                     &rev_coefs,
                                     &TsTw_tran_inv
                                    );

  // 2. cv pipelines
  Image<float> opencv_in_image  = lastFunc.realize(width, height, 3);
  Mat opencv_in_mat             = Image2Mat(&opencv_in_image);

  run_image_pipeline_cv(&opencv_in_mat, cv_stages, num_stages[1]);

  Image<float> opencv_out       = Mat2Image(&opencv_in_mat);
  Func Image2Func               = make_Image2Func(&opencv_out);

  // 3. forward pipelines
  lastFunc  = run_image_pipeline_fwd( &Image2Func,
                                      fwd_stages,
                                      num_stages[2],
                                      &rev_tone_h, // reuse rev tone
                                      num_ctrl_pts,         
                                      &ctrl_pts_h,
                                      &weights_h,
                                      &coefs,
                                      &TsTw_tran
                                     );

  // Scale back to 0-255 and represent in 8 bit fixed point
  Func descale                  = make_descale(&lastFunc);


  /////////////////////////////////////////////////////////////////////////////
  //                              Scheduling
  /////////////////////////////////////////////////////////////////////////////

  // Use JIT compiler
  descale.compile_jit();
  Image<uint8_t> output         = descale.realize(width,height,3);

  /////////////////////////////////////////////////////////////////////////////
  //                            Save the output
  /////////////////////////////////////////////////////////////////////////////
  save_image(output, (std::string(out_img_path)+"output.png").c_str());

  return 0;
}



Func run_image_pipeline_rev(Func *in_func, 
                            PipelineStageRev rev_stages[], 
                            int num_stages,
                            Image<float> *rev_tone_h, 
                            int num_ctrl_pts, 
                            Image<float> *rev_ctrl_pts_h, 
                            Image<float> *rev_weights_h, 
                            vector<vector<float>> *rev_coefs, 
                            vector<vector<float>> *TsTw_tran_inv ) {

  Func out_func = *in_func;

  for (int i = 0; i < num_stages; i++) {
    PipelineStageRev stage = rev_stages[i];
    debug_print(stage);

    switch( stage ) {
      case RevToneMap: {
        out_func                = make_rev_tone_map(&out_func, rev_tone_h);
        out_func.compute_root();
        break;
      } 

      case RevGamutMap: {
        Func rev_gamut_map_ctrl = make_rbf_ctrl_pts(&out_func, 
                                                    num_ctrl_pts, 
                                                    rev_ctrl_pts_h, 
                                                    rev_weights_h );

        out_func                = make_rbf_biases(  &out_func, 
                                                    &rev_gamut_map_ctrl, 
                                                    rev_coefs);
        rev_gamut_map_ctrl.compute_root();
        break;
      }

      case RevTransform: {
        out_func                = make_transform(&out_func, TsTw_tran_inv);
        out_func.compute_root();
        break;
      }
    }
  }

  return out_func;
}

void run_image_pipeline_cv( Mat *InMat, PipelineStageCV cv_stages[], int num_stages ) {

  using namespace Halide;
  using namespace Halide::Tools;
  using namespace cv;

  for (int i = 0; i < num_stages; i++) {
    PipelineStageCV stage = cv_stages[i];
    debug_print(stage);

    switch( stage ) {
      case Renoise: {
        OpenCV_renoise(InMat);
        break;
      }

      case Remosaic: {
        OpenCV_remosaic(InMat);
        break;
      }

      case GaussianBlurCV: {
        OpenCV_gaussian_blur(InMat);
        break;
      }

      case LloydRequant: {
        OpenCV_lloydmax_requant(InMat);
        break;
      }
    }
  }
}

Func run_image_pipeline_fwd(Func *in_func, 
                            PipelineStageFwd fwd_stages[], 
                            int num_stages,
                            Image<float> *tone_h, 
                            int num_ctrl_pts, 
                            Image<float> *ctrl_pts_h, 
                            Image<float> *weights_h, 
                            vector<vector<float>> *coefs, 
                            vector<vector<float>> *TsTw_tran ) {

  using namespace Halide;
  using namespace Halide::Tools;
  using namespace cv;

  Func out_func = *in_func;

  for (int i = 0; i < num_stages; i++) {
    PipelineStageFwd stage = fwd_stages[i];
    debug_print(stage);

    switch( stage ) {
      case ToneMap: {
        out_func            = make_tone_map(&out_func, tone_h);
        out_func.compute_root();
        break;
      } 

      case GamutMap: {
        Func gamut_map_ctrl = make_rbf_ctrl_pts(&out_func, 
                                                num_ctrl_pts, 
                                                ctrl_pts_h, 
                                                weights_h);

        out_func            = make_rbf_biases ( &out_func, 
                                                &gamut_map_ctrl, 
                                                coefs);
        gamut_map_ctrl.compute_root();
        break;
      }

      case Transform: {
        out_func            = make_transform(&out_func, TsTw_tran);
        out_func.compute_root();
        break;
      }

      case Requant1: case Requant2: case Requant3: case Requant4: 
      case Requant5: case Requant6: case Requant7: {
        int num_bits        = stage;
        out_func            = make_requant(&out_func, num_bits);
        break;
      }

      case DemosSubSample: {
        out_func            = make_demosaic_subsample(&out_func);
        break;
      }

      case DemosNN: {
        out_func            = make_demosaic_nn(&out_func);
        break;
      }

      case DemosInterp: {
        out_func            = make_demosaic_interp( &out_func );
        break;
      }

      case QrtrResBinning: {
        out_func            = make_qrtr_res_binning( &out_func );
        break;
      }

      case PwlToneMap: {
        out_func            = make_pwl_tone_map( &out_func );
        break;
      }
    }
  }

  return out_func;
}
