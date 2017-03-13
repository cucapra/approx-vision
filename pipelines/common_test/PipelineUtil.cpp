#include "PipelineUtil.h"
#include "/approx-vision/pipelines/common_test/CameraModel.h"

// Pipeline Utility 

int run_image_pipeline( char* in_img_path, 
                        char* out_img_path,
                        CameraModel camera_model,
                        enum PipelineStageRev  rev_stages[],
                        enum PipelineStageCV   cv_stages[],
                        enum PipelineStageFwd  fwd_stages[],
                        int num_stages[] ) {

  /////////////////////////////////////////////////////////////////////////////
  //                        Import and format model data
  /////////////////////////////////////////////////////////////////////////////

  int num_ctrl_pts                    = camera_model.get_num_ctrl_pts();

  vector<vector<float>> rev_coefs     = camera_model.get_cam_rev_coefs();
  vector<vector<float>> coefs         = camera_model.get_cam_coefs();
  vector<vector<float>> TsTw_tran     = camera_model.get_tstw_tran();
  vector<vector<float>> TsTw_tran_inv = camera_model.get_tstw_tran_inv();

  Image<float> rev_ctrl_pts_h         = camera_model.get_rev_ctrl_pts_h();
  Image<float> rev_weights_h          = camera_model.get_rev_weights_h();
  Image<float> ctrl_pts_h             = camera_model.get_ctrl_pts_h();
  Image<float> weights_h              = camera_model.get_weights_h();
  Image<float> rev_tone_h             = camera_model.get_rev_tone_h();

  /////////////////////////////////////////////////////////////////////////////
  //                    Import and format input image
  /////////////////////////////////////////////////////////////////////////////

  // Load input image 
  Image<uint8_t> input                = load_image(in_img_path);
  int width                           = input.width();
  int height                          = input.height();

  /////////////////////////////////////////////////////////////////////////////
  //                          Camera Pipeline
  /////////////////////////////////////////////////////////////////////////////

  Func lastFunc                       = make_Image2Func( &input );

  // 1. reverse pipelines
  lastFunc                            = run_image_pipeline_rev(&lastFunc, 
                                                               rev_stages,
                                                               num_stages[0],
                                                               &rev_tone_h,
                                                               num_ctrl_pts,
                                                               &rev_ctrl_pts_h,
                                                               &rev_weights_h,
                                                               &rev_coefs,
                                                               &TsTw_tran_inv
                                                              );

  cout << "1\n";
  // 2. cv pipelines
  Image<float> opencv_in_image        = lastFunc.realize(width, height, 3);
  cout << "2\n";

  Mat opencv_in_mat                   = Image2Mat(&opencv_in_image);
  cout << "3\n";

  run_image_pipeline_cv(&opencv_in_mat, cv_stages, num_stages[1]);
  cout << "4\n";

  Image<float> opencv_out             = Mat2Image(&opencv_in_mat);
  cout << "5\n";

  // clamp if needed 
  Func Image2Func;
  bool clamp_needed = false;

  for (int i = 0; i < num_stages[2]; i++) { // check if stage requires clamping
    PipelineStageFwd stage = fwd_stages[i];
    if (stage == DemosSubSample || stage == DemosNN || stage == DemosInterp) {
      clamp_needed = true;
      Func clamped("clamped");
      Image2Func = BoundaryConditions::repeat_edge(opencv_out);
      debug_print("clamping image...");
      break;
    }
  }

  if (!clamp_needed) {
    Image2Func = make_Image2Func(&opencv_out);
  } 
  cout << "6\n";

  // 3. forward pipelines
  vector<int> qrtr_bin_factor = { 1 };

  lastFunc                            = run_image_pipeline_fwd(&Image2Func,
                                                              fwd_stages,
                                                              num_stages[2],
                                                              qrtr_bin_factor,
                                                              &rev_tone_h,
                                                              num_ctrl_pts,
                                                              &ctrl_pts_h,
                                                              &weights_h,
                                                              &coefs,
                                                              &TsTw_tran
                                                             );
  debug_print("qrtr_bin_factor: " + to_string(qrtr_bin_factor[0]));

  /////////////////////////////////////////////////////////////////////////////
  //                              Scheduling
  /////////////////////////////////////////////////////////////////////////////

  // Use JIT compiler
  lastFunc.compile_jit();
  Image<uint8_t> output         = lastFunc.realize( width / qrtr_bin_factor[0],
                                                    height/ qrtr_bin_factor[0],
                                                    3 );

  /////////////////////////////////////////////////////////////////////////////
  //                            Save the output
  /////////////////////////////////////////////////////////////////////////////
  save_image( output, ( std::string(out_img_path) + "output.png" ).c_str() );

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
    debug_print("rev stage: " + to_string(stage));

    switch( stage ) {
      case RevScale: {
        out_func                = make_scale( &out_func );
        break;
      }

      case RevDescale: {
        out_func                = make_descale( &out_func );
        break;
      }

      case RevRequant1: case RevRequant2: case RevRequant3: case RevRequant4: 
      case RevRequant5: case RevRequant6: case RevRequant7: {
        int num_bits            = stage;
        out_func                = make_requant(&out_func, num_bits);
        debug_print("requant num bits: " + to_string(num_bits));
        break;
      }

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
    debug_print("cv stage:  " + to_string(stage));

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
                            vector<int> &qrtr_bin_factor,
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
    debug_print("fwd stage: " + to_string(stage));

    switch( stage ) {
      case Scale: {
        out_func            = make_scale( &out_func );
        break;
      }

      case Descale: {
        out_func            = make_descale( &out_func );
        break;
      }

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
        debug_print(num_bits);
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
        debug_print(to_string(qrtr_bin_factor[0]));
        qrtr_bin_factor[0]  = qrtr_bin_factor[0] * 2;
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
