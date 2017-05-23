#include "PipelineUtil.h"
#include "/approx-vision/pipelines/CLI/core/CameraModel.h"

// Pipeline Utility 

int run_image_pipeline( char* in_img_path, 
                        char* out_img_path,
                        CameraModel camera_model,
                        enum PipelineStage stages[],
                        int num_stages ) {

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
  // Image<float> input                = load_image(in_img_path);

  int width                           = input.width();
  int height                          = input.height();


  /////////////////////////////////////////////////////////////////////////////
  //                          Camera Pipeline
  /////////////////////////////////////////////////////////////////////////////

  Func resultFunc                       = make_Image2Func( &input );

  vector<int> qrtr_bin_factor = { 1 };

  // run stages
  resultFunc = run_image_pipeline_stage(&resultFunc,
                                        stages,
                                        num_stages,
                                        width,
                                        height,
                                        num_ctrl_pts,
                                        &rev_ctrl_pts_h,
                                        &rev_tone_h,
                                        &rev_weights_h,
                                        &rev_coefs,
                                        &TsTw_tran_inv,
                                        &ctrl_pts_h,
                                        qrtr_bin_factor,
                                        &rev_tone_h,
                                        &weights_h,
                                        &coefs,
                                        &TsTw_tran);

  debug_print("qrtr_bin_factor: " + to_string(qrtr_bin_factor[0]));

  /////////////////////////////////////////////////////////////////////////////
  //                              Scheduling
  /////////////////////////////////////////////////////////////////////////////

  // Use JIT compiler
  resultFunc.compile_jit();
  Image<uint8_t> output         = resultFunc.realize( width / qrtr_bin_factor[0],
                                                      height/ qrtr_bin_factor[0],
                                                      3 );

  /////////////////////////////////////////////////////////////////////////////
  //                            Save the output
  /////////////////////////////////////////////////////////////////////////////
  save_image( output, (std::string(out_img_path) ).c_str() );

  return 0;
}



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
                            ) {

  Func out_func = *in_func;

  // process stage
  for (int i = 0; i < num_stages; i++) {
    PipelineStage stage = stages[i];
    debug_print("stage: " + to_string(stage));

    switch( stage ) {

      case Scale: {
        debug_print("Scale");
        out_func                = make_scale( &out_func );
        break;
      }

      case Requant1: case Requant2: case Requant3: case Requant4: 
      case Requant5: case Requant6: case Requant7: {
        int num_bits            = stage;
        out_func                = make_requant(&out_func, num_bits);
        debug_print("requant num bits: " + to_string(num_bits));
        break;
      }

      case RevToneMap: {
        debug_print("RevToneMap");
        out_func                = make_rev_tone_map(&out_func, rev_tone_h);
        out_func.compute_root();
        break;
      } 

      case RevGamutMap: {
        debug_print("RevGamutMap");
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
        debug_print("RevTransform");
        out_func                = make_transform(&out_func, TsTw_tran_inv);
        out_func.compute_root();
        break;
      }

      // cv module
      case Renoise: case Remosaic: case GaussianBlurCV: case LloydRequant: {
        Image<float> opencv_in_image        = out_func.realize(width, height, 3);
        Mat opencv_in_mat                   = Image2Mat(&opencv_in_image);

        switch ( stage ) {
          case Renoise: {
            debug_print("Renoise");
            OpenCV_renoise(&opencv_in_mat);
            break;
          }

          case Remosaic: {
            debug_print("Remosaic");
            OpenCV_remosaic(&opencv_in_mat);
            break;
          }

          case GaussianBlurCV: {
            debug_print("GaussianBlurCV");
            OpenCV_gaussian_blur(&opencv_in_mat);
            break;
          }

          case LloydRequant: {
            debug_print("LloydRequant");
            OpenCV_lloydmax_requant(&opencv_in_mat);
            break;
          } 
        }

        Image<float> opencv_out = Mat2Image(&opencv_in_mat);
        out_func                = make_Image2Func   ( &opencv_out );
        break;
      }

      case Descale: {
        debug_print("Descale");
        out_func            = make_descale( &out_func );
        break;
      }

      case GamutMap: {
        debug_print("GamutMap");
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
        debug_print("Transform");
        out_func            = make_transform(&out_func, TsTw_tran);
        out_func.compute_root();
        break;
      }

      case ToneMap: {
        debug_print("ToneMap");
        out_func            = make_tone_map(&out_func, tone_h);
        out_func.compute_root();
        break;
      } 

      // requires clamping
      case DemosSubSample: case DemosNN: case DemosInterp: {
        // realize image
        Image<float> image_to_clamp   = out_func.realize(width, height, 3);

        // repeat edges
        Func clamped("clamped");
        Func clamped_func = BoundaryConditions::repeat_edge(image_to_clamp);
        debug_print("clamping image...");
        switch (stage) {
          case DemosSubSample: {
            debug_print("DemosSubSample");
            out_func            = make_demosaic_subsample(&clamped_func);
            break;
          }

          case DemosNN: {
            debug_print("DemosNN");
            out_func            = make_demosaic_nn(&clamped_func);
            break;
          }

          case DemosInterp: {
            debug_print("DemosInterp");
            out_func            = make_demosaic_interp(&clamped_func);
            break;
          }
        }
        break;
      }

      // changes qrtr_bin_factor implicitly
      case QrtrResBinning: {
        debug_print("QrtrResBinning");
        out_func            = make_qrtr_res_binning( &out_func );
        debug_print(to_string(qrtr_bin_factor[0]));
        qrtr_bin_factor[0]  = qrtr_bin_factor[0] * 2;
        break;
      }

      case PwlToneMap: {
        debug_print("PwlToneMap");
        out_func            = make_pwl_tone_map( &out_func );
        break;
      }
    }
  }

  return out_func;
}
